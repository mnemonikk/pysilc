import time
import os
import socket
import re

import supybot.log as log
import supybot.conf as conf
import supybot.ircdb as ircdb
import supybot.drivers as drivers
import supybot.ircmsgs as ircmsgs

import silc

# TODOs
# 1. Silc nicks do not need to be unique, so there can be nick collisions.
#    IRC has nick collisions, so you cannot have two users with same nick name.
#    I presume SupyBot assumes this.

SILC_KEY_NAME = "silckey"

def strip_leading_hash(supybot_channel_name):
    return supybot_channel_name[1:]

class SupySilcClient(silc.SilcClient):
    """Supybot SILC to IRC emulator."""
    
    def __init__(self, irc, parent):
        self.nickname = conf.supybot.nick()
        self.username = conf.supybot.ident()
        self.realname = conf.supybot.user()
        
        keybase = os.path.join(conf.supybot.directories.conf(), SILC_KEY_NAME)
        pubkey = keybase + '.pub'
        privkey = keybase + '.prv'
        self.keys = None

        if os.path.exists(pubkey) and os.path.exists(privkey):
            try:
                self.keys = silc.load_key_pair(pubkey, privkey, "")
            except RuntimeError:
                self.keys = silc.load_key_pair(pubkey, privkey, None)
        else:
            drivers.log.info("SILC: Generating Keys")
            self.keys = silc.create_key_pair(pubkey, privkey, passphrase=None)
            
        silc.SilcClient.__init__(self, self.keys,
                                 self.nickname,
                                 self.username,
                                 self.realname)
        
        self.irc = irc
        self.parent = parent
        self.isconnected = False
        self.last_ping = 0
        self.users = {}    # use this to lookup nick to SilcUser objects
        self.channels = {} # use this to lookup channels to SilcChannel objects

    def _cache_user(self, user):
        self.users[user.nickname] = user
        
    def _cache_channel(self, channel):
        self.channels[channel.channel_name] = channel

    def ask_passphrase(self):
        import getpass
        return getpass.getpass('Passphrase:')

    def connected(self):
        drivers.log.info("SILC: Connected to server.")
        self.isconnected = True
        self.irc.driver = self.parent

    def disconnected(self, msg):
        drivers.log.info('SILC: Disconnected from server.')
        self.isconnected = False
        
    def command(self, success, code, command, status):
        drivers.log.info('SILC: Command: %s %s %s %s', success, code, command, status)
        
    def say(self, msg):
        drivers.log.info('SILC: Say received: %s', str([msg]))
        
    def channel_message(self, sender, channel, flags, msg):
        drivers.log.info('SILC: Channel Message: %s [%s] %s' % (sender.nickname, channel, msg))
        self._cache_channel(channel)
        self._cache_user(sender)
        
        ircemu = ':%s!%s@%s PRIVMSG #%s :%s' % (sender.nickname, sender.username,
                                               sender.hostname, channel.channel_name,
                                               msg)
        ircmsg = drivers.parseMsg(ircemu)
        self.irc.feedMsg(ircmsg)
    
    def private_message(self, sender, flags, msg):
        drivers.log.info('SILC: Private Message: [%s] %s', sender, msg)
        self._cache_user(sender)
        
        ircemu = ':%s!%s@%s PRIVMSG %s :%s' % (sender.nickname, sender.username,
                                               sender.hostname, self.username,
                                               msg)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def notify_none(self, msg):
        drivers.log.info('SILC: Notify (None): %s', msg)
        
    def notify_join(self, joiner, channel):
        self._cache_user(joiner)
        drivers.log.info('SILC: Notify (Join): %s %s', joiner, channel)

        ircemu = ':%s!n=%s@%s JOIN #%s' % (joiner.nickname, joiner.username,
                                           joiner.hostname,
                                           channel.channel_name)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def notify_invite(self, channel, channel_name, inviter):
        self._cache_channel(channel)
        self._cache_user(user)
        drivers.log.info('SILC: Notify (Invite): %s %s %s', channel, channel_name, inviter)
        
    def notify_leave(self, leaver, channel):
        self._cache_user(leaver)
        self._cache_channel(channel)
        drivers.log.info('SILC: Notify (Leave): %s %s', leaver, channel.channel_name)

        ircemu = ':%s!n=%s@%s PART #%s :' % (leaver.nickname, leaver.username,
                                           leaver.hostname,
                                           channel.channel_name)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
                                  
        
    def notify_signoff(self, user):
        self._cache_user(user)
        drivers.log.info('SILC: Notify (Signoff): %s', user)

        ircemu = ':%s!n=%s@%s QUIT :' % (leaver.nickname, leaver.username,
                                           leaver.hostname)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)        
    
    def notify_topic_set(self, changer_type, changer, channel, topic):
        self._cache_user(changer)
        self._cache_channel(channel)
        drivers.log.info('SILC: Notify (Topic Set):', channel, topic)
        if changer_type == silc.SILC_ID_CLIENT:
            ircemu = ':%s!n=%s@%s TOPIC #%s :%s' % \
                (changer.nickname, changer.username, changer.hostname,
                 channel.channel_name, topic)
            ircmsg = drivers.parseMsg(ircemu)
            if ircmsg: self.irc.feedMsg(ircmsg)
        elif changer_type == silc.SILC_ID_CHANNEL:
            ircemu = ':%s TOPIC #%s :%s' % \
                (changer.channel_name, channel.channel_name, topic)
            ircmsg = drivers.parseMsg(ircemu)
            if ircmsg: self.irc.feedMsg(ircmsg)
        
    def notify_nick_change(self, olduser, newuser):
        self._cache_user(newuser)
        drivers.log.info('SILC: Notify (Nick Change):', olduser, newuser)
        ircemu = ':%s!n=%s@%s NICK %s' % \
                 (olduser.nickname, olduser.username, olduser.hostname,
                  newuser.nickname)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def notify_cmode_change(self, type, changer, chan_mode, cipher_name,
                            hmac_name, passphrase, founder_key, 
                            channel_pubkeys, channel):
        self._cache_channel(channel)
        drivers.log.info('SILC: Notify (CMODE): for %s: %08x' % 
                         (channel, chan_mode))
    
    def notify_cumode_change(self, type, changer, chan_user_mode, user,
                            for_channel):
        self._cache_channel(for_channel)
        self._cache_user(user)
        drivers.log.info('SILC: Notify (CUMODE): for %s in %s: %08x' % 
                         (user, for_channel, chan_user_mode))
        
    def notify_motd(self, msg):
        drivers.log.info('SILC: Notify (MOTD):', msg)
        
    def notify_server_signoff(self):
        drivers.log.info('SILC: Notify (Server Signoff)')
    
    def notify_kicked(self, kicked, reason, kicker, channel):
        self._cache_user(kicked)
        self._cache_user(kicker)
        self._cache_channel(channel)
        drivers.log.info('SILC: Notify (Kick):', kicked, reason, kicker, channel)
        ircemu = ':%s!n=%s@%s KICK #%s %s' % \
                 (kicker.nickname, kicker.username, kicker.hostname,
                  channel.channel_name, kicked.nickname)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def notify_killed(self, killed, reason, killer, channel):
        self._cache_user(kicked)
        self._cache_user(kicker)
        self._cache_channel(channel)
        drivers.log.info('SILC: Notify (Killed):', killed, reason, killer, channel)
        
    def notify_error(self, type, message):
        drivers.log.info('SILC: Notify (Error):', type, message)
        
    def notify_watch(self, watched, new_nick, new_user_mode, notification, _):
        self._cache_user(watched)
        drivers.log.info('SILC: Notify (Watch):', watched)
        
    def command_reply_whois(self, user, nickname, username, realname, mode, idle):
        self._cache_user(user)
        drivers.log.info('SILC: Reply (Whois): %s mode: %x idle: %d' % (nickname, mode, idle))
        
    def command_reply_whowas(self, user, nickname, username, realname):
        self._cache_user(user)
        drivers.log.info('SILC: Reply (Whowas):', nickname)        
        
    def command_reply_nick(self, user, nickname, olduserid):
        self._cache_user(user)
        drivers.log.info('SILC: Reply (Nick):', nickname)
        
    def command_reply_list(self, channel, channel_name, channel_topic, user_count):
        if channel == None:
            drivers.log.info('SILC: Reply (List): END')
        else:
            self._cache_channel(channel)
            drivers.log.info('SILC: Reply (List):', channel_name, channel_topic)
            
    def command_reply_topic(self, channel, topic):
        self._cache_channel(channel)
        ircemu = ':%s TOPIC #%s :%s' % \
            (channel.channel_name, channel.channel_name, topic)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        drivers.log.info('SILC: Reply (Topic):', channel, topic)
        
    def command_reply_invite(self, channel, invite_list):
        pass # TODO: not implemented
        
    def command_reply_kill(self, user):
        self._cache_user(user)
        drivers.log.info('SILC: Reply (Kill):', user)
        
    def command_reply_info(self, *args):
        pass # TODO: not implemented

    def command_reply_stats(self, *args):
        pass # TODO: not implemented
        
    def command_reply_ping(self):
        drivers.log.info('SILC: Reply (Ping): PONG')
        ircemu = ':%s PONG %s :%s' % (self.remote_host(), self.remote_host(), self.last_ping)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)

    def command_reply_oper(self):
        drivers.log.info('SILC: Reply (Oper)')        
        
    def command_reply_join(self, channel, channel_name, topic, hmac_name, mode, user_limit, users):
        self._cache_channel(channel)
        myself = self.user()
        
        drivers.log.info('SILC: Reply (Join)', channel, topic, users)
        ircemu = ':%s!%s@%s JOIN :#%s' % (myself.nickname,
                                          myself.username,
                                          myself.hostname,
                                          channel_name)

        ircmsg = drivers.parseMsg(ircemu)
        self.irc.feedMsg(ircmsg)
        
        ircemu = ':%s MODE #%s %s' % (self.remote_host(), channel_name, "+ns")
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
            
        # really need names to be passed from the reply
        ircemu = ':%s 353 %s @ #%s :%s' % (self.remote_host(), 
                                    self.username,
                                    channel_name, 
                                    ' '.join([u.nickname for u in users]))
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
            
        # TODO: other list of names
        ircemu = ':%s 366 %s :End of /NAMES list' % (self.remote_host(), self.username)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
            
        # send topic as well
        ircemu = ':%s 332 RPL_TOPIC #%s :%s' % (self.remote_host(), 
                                                channel_name,
                                                topic)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def command_reply_motd(self, msg):
        drivers.log.info('SILC: Reply (MOTD):', msg)
        
    def command_reply_cmode(self, channel, mode, user_limit, founder_key, _):
        self._cache_channel(channel)
        drivers.log.info('SILC: Reply (Cmode):', channel, mode)
        
        ircemu = ':%s 324 %s #%s %s' % (self.remote_host(),
                                    self.user().nickname,
                                    channel.channel_name,
                                     "+sn")
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)

        ircemu = ':%s 329 %s #%s %d' % (self.remote_host(),
                                    self.user().nickname,
                                    channel.channel_name,
                                     time.time())
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
        
    def command_reply_cumode(self, mode, channel, user):
        self._cache_channel(channel)
        self._cache_user(user)
        drivers.log.info('SILC: Reply (CUmode):', channel, user, mode)
        
    def command_reply_kick(self, channel, user):
        self._cache_channel(channel)
        self._cache_user(user)
        drivers.log.info('SILC: Reply (Kick):', channel, user)
        
    def command_reply_ban(self, channel, banlist):
        self._cache_channel(channel)
        drivers.log.info('SILC: Reply (Ban):', channel)
        
    def command_reply_detach(self):
        drivers.log.info('SILC: Reply (Detach)')
        
    def command_reply_watch(self):
        drivers.log.info('SILC: Reply (Watch)')        
        
    def command_reply_silcoper(self):
        drivers.log.info('SILC: Reply (SilcOper)')        
        
    def command_reply_leave(self, channel):
        self._cache_channel(channel)
        drivers.log.info('SILC: Reply (Leave):', channel)        
        
    def command_reply_users(self, channel, users):
        for user in users:
            self._cache_user(user)
        drivers.log.info('SILC: Reply (Users): %s %s', channel, users)

        myself = self.user().nickname
        for user in users:
            data = {'server': self.remote_host(),
                    'channel': '#' + channel.channel_name,
                    'myself': myself,
                    'ident': user.username,
                    'host': user.hostname,
                    'nickname': user.nickname,
                    'mode': 'H@',
                    'fullname': user.realname}
            
            ircemu = ':%(server)s 352 %(myself)s %(channel)s n=%(ident)s ' % data
            ircemu += ' %(host)s %(server) %(nickname) %(mode)s ' % data
            ircemu += ':0 %(fullname)s' % data
            ircmsg = drivers.parseMsg(ircemu)
            if ircmsg: self.irc.feedMsg(ircmsg)

        ircemu = ':%s 315 %s #%s' % (self.remote_host(), myself, channel.channel_name)
        ircmsg = drivers.parseMsg(ircemu)
        if ircmsg: self.irc.feedMsg(ircmsg)
        
    def command_reply_service(self, *args):
        pass # not implemented
    
    def command_reply_failed(self, command, commandstr, errorcode, errormsg):
        # global catching failed commands and their error codes
        drivers.log.info('SILC: Reply (FAILED)! %s %s', commandstr, errormsg)


class SilcDriver(drivers.IrcDriver, drivers.ServersMixin):
    def __init__(self, irc):
        self.__parent = super(SilcDriver, self)
        self.__parent.__init__(irc)
        self.irc = irc
        self.silc = SupySilcClient(irc, self)        
        self.running = True
        self.connected = False
        self.reconnect()

    def run(self):
        if self.silc:
            self.connected = self.silc.isconnected
        else:
            self.connected = False
            
        try:
            self.silc.run_one()
            time.sleep(conf.supybot.drivers.poll())
        except:
            import traceback
            traceback.print_exc()
            raise

        self.checkIrcForMsgs()

    def reconnect(self):
        drivers.log.info('SilcDriver: Logging into server')
        host, port = self.__parent._getNextServer()
        self.silc.connect_to_server(host, port)

    def checkIrcForMsgs(self):
        # convert irc messages into silc command equivalents and send it off.
        
        if self.silc.isconnected:
            msg = self.irc.takeMsg()
            if msg:
                drivers.log.info('IRC MSG: %s', repr(msg))
                handler = 'do_' + msg.command
                handler = getattr(self, handler, None)
                if handler:
                    handler(msg)
                else:
                    drivers.log.info('!! MSG UNKNOWN: %s', msg.command)
                
    def do_PRIVMSG(self, msg):
        if msg.args[0][0] == '#':
            chan = self.silc.channels[strip_leading_hash(msg.args[0])]
            self.silc.send_channel_message(chan, msg.args[1])
        else:
            user = self.silc.users[msg.args[0]]
            self.silc.send_private_message(user, msg.args[1])


    def do_JOIN(self, msg):
        self.silc.command_call('JOIN %s' % strip_leading_hash(msg.args[0]))
        
    def do_PART(self, msg):
        self.silc.command_call('LEAVE %s' % strip_leading_hash(msg.args[0]))

    def do_NICK(self, msg):
        self.silc.command_call('NICK %s' % msg.args[0])

        ircemu =self.makeEmulatedIrcMsg('001', 'Welcome')
        ircmsg = drivers.parseMsg(ircemu)
        self.irc.feedMsg(ircmsg)
                    
    def do_PING(self, msg):
        self.silc.last_ping = msg.args[0]
        self.silc.command_call('PING %s' % self.silc.remote_host())

    def do_USER(self, msg):
        ircemu = self.makeEmulatedIrcMsg('002', 'Your host')
        ircmsg = drivers.parseMsg(ircemu)
        self.irc.feedMsg(ircmsg)

        ircemu = self.makeEmulatedIrcMsg('376', 'End MOTD')
        ircmsg = drivers.parseMsg(ircemu)
        self.irc.feedMsg(ircmsg)

    def do_MODE(self, msg):
        pass # don't know what to do with this yet

    def do_WHO(self, msg):
        if msg.args[0][0] == '#':
            self.silc.command_call('USERS %s' % strip_leading_hash(msg.args[0]))
        else:
            print '!! Command Not recognised'

    def do_TOPIC(self, msg):
        if len(msg.args) > 1:
            self.silc.command_call('TOPIC %s %s' % 
                                    (strip_leading_hash(msg.args[0]),
                                     msg.args[1]))
        else:
            self.silc.command_call('TOPIC %s' % strip_leading_hash(msg.args[0]))

    def do_QUIT(self, msg):
        self.silc.command_call('QUIT %s' % msg.args[0])

    def do_NAMES(self, msg):
        self.silc.command_call('USERS %s' % strip_leading_hash(msg.args[0]))
        
    def makeEmulatedIrcMsg(self, cmd, msg, prefix = None, username = None):
        if not prefix:
            prefix = self.silc.remote_host()
        if not username:
            username = self.silc.username
        return ':%s %s %s :%s' % (prefix, cmd, username, msg)

Driver = SilcDriver
