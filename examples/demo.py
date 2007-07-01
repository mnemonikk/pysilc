import silc

import os
import pwd
import socket
import time

class SupySilcClient(silc.SilcClient):
    def __init__(self, irc = None):
        keybase = './silckey'
        pubkey = keybase + '.pub'
        privkey = keybase + '.prv'
        self.keys = None
        if os.path.exists(pubkey) and os.path.exists(privkey):
            self.keys = silc.load_key_pair(pubkey, privkey)
        else:
            self.keys = silc.create_key_pair(pubkey, privkey)
            
        silc.SilcClient.__init__(self, keys = self.keys, nickname = "bot", username = "demo")
        
        self.irc = irc
        self.isconnected = False
        self.users = {}    # use this to lookup nick to SilcUser objects
        self.channels = {} # use this to lookup channels to SilcChannel objects

    def _cache_user(self, user):
        self.users[user.nickname] = user
        
    def _cache_channel(self, channel):
        self.channels[channel.channel_name] = channel

    def running(self):
        self.connect_to_server(sys.argv[1], 706)

    def connected(self):
        print 'SILC: Connected to server.'
        self.isconnected = True
        self.command_call('JOIN #bot')

    def disconnected(self, msg):
        print 'SILC: Disconnected from server.'
        self.isconnected = False

    def command(self, success, code, command, status):
        print 'SILC: Command:', success, code, command, status
        
    def say(self, msg):
        print 'SILC: Say received: %s' % msg
        
    def channel_message(self, sender, channel, flags, msg):
        print 'SILC: Channel Message: %s [%s] %s' % (sender, channel, msg)
        self._cache_channel(channel)
        self._cache_user(sender)
        self.send_channel_message(channel, 'I totally agree with what you are saying, %s' % sender.username)
        self.send_channel_message(channel, 'Again, %s' % msg)
    
    def private_message(self, sender, flags, msg):
        print 'SILC: Private Message: [%s] %s' % (sender, msg)
        self._cache_user(sender)
        self.send_private_message(sender, 'Wow, I never knew %s' % msg)
        
    def notify_none(self, msg):
        print 'SILC: Notify (None):', msg
        
    def notify_join(self, joiner, channel):
        self._cache_user(joiner)
        print 'SILC: Notify (Join): %s %s' % (joiner, channel)
        
    def notify_invite(self, channel, channel_name, inviter):
        print 'SILC: Notify (Invite):', channel, channel_name, inviter
        
    def notify_leave(self, leaver, channel):
        self._cache_user(leaver)
        self._cache_channel(channel)
        print 'SILC: Notify (Leave): %s %s' % (leaver, channel)
        
    def notify_signoff(self, user, msg):
        self._cache_user(user)
        print 'SILC: Notify (Signoff):', user, msg
    
    def notify_topic_set(self, type, changedby, channel, topic):
        self._cache_user(changedby)
        self._cache_channel(channel)
        print 'SILC: Notify (Topic Set):', channel, topic
        
    def notify_nick_change(self, olduser, newuser):
        self._cache_user(newuser)
        print 'SILC: Notify (Nick Change):', olduser, newuser
        
    def notify_cmode_change(self, *args):
        pass # TODO: not implemented
    
    def notify_cumode_change(self, *args):
        pass # TODO: not implemented
        
    def notify_motd(self, msg):
        print 'SILC: Notify (MOTD):', msg
        
    def notify_server_signoff(self):
        print 'SILC: Notify (Server Signoff)'
    
    def notify_kicked(self, kicked, reason, kicker, channel):
        self._cache_user(kicked)
        self._cache_user(kicker)
        self._cache_channel(channel)
        print 'SILC: Notify (Kick):', kicked, reason, kicker, channel
        
    def notify_killed(self, *args):
        pass # TODO: not implemented
        
    def notify_error(self, type, message):
        print 'SILC: Notify (Error):', type, message
        
    def notify_watch(self, watched, new_nick, new_user_mode, notification, _):
        self._cache_user(watched)
        print 'SILC: Notify (Watch):', watched
        
    def command_reply_whois(self, user, nickname, username, realname, mode, idle):
        self._cache_user(user)
        print 'SILC: Reply (Whois): %s mode: %x idle: %d' % (nickname, mode, idle)
        
    def command_reply_whowas(self, user, nickname, username, realname):
        self._cache_user(user)
        print 'SILC: Reply (Whowas):', nickname
        
    def command_reply_nick(self, user, nickname, olduserid):
        self._cache_user(user)
        print 'SILC: Reply (Nick):', nickname
        
    def command_reply_list(self, channel, channel_name, channel_topic, user_count):
        if channel == None:
            print 'SILC: Reply (List): END'
        else:
            self._cache_channel(channel)
            print 'SILC: Reply (List):', channel_name, channel_topic
            
    def command_reply_topic(self, channel, topic):
        self.cache_channel(channel)
        print 'SILC: Reply (Topic):', channel, topic
        
    def command_reply_invite(self, *args):
        pass # TODO: not implemented
        
    def command_reply_kill(self, user):
        self._cache_user(user)
        print 'SILC: Reply (Kill):', user
        
    def command_reply_info(self, *args):
        pass # TODO: not implemented

    def command_reply_stats(self, *args):
        pass # TODO: not implemented
        
    def command_reply_ping(self):
        print 'SILC: Reply (Ping): PONG'

    def command_reply_oper(self):
        print 'SILC: Reply (Oper)'
        
    def command_reply_join(self, channel, channel_name, topic, hmac_name, mode, user_limit, users):
        self._cache_channel(channel)
        print 'SILC: Reply (Join)', channel, topic, users
        myself = self.user()
        self.send_channel_message(channel, 'My name is %s. I live in %s' % (myself.realname, myself.hostname))
        
    def command_reply_motd(self, msg):
        print 'SILC: Reply (MOTD):', msg
        
    def command_reply_cmode(self, channel, mode, user_limit, founder_key, _):
        self._cache_channel(channel)
        print 'SILC: Reply (Cmode):', channel, mode
        
    def command_reply_cumode(self, mode, channel, user):
        self._cache_channel(channel)
        self._cache_user(user)
        print 'SILC: Reply (CUmode):', channel, user, mode
        
    def command_reply_kick(self, channel, user):
        self._cache_channel(channel)
        self._cache_user(user)
        print 'SILC: Reply (Kick):', channel, user
        
    def command_reply_ban(self, channel, banlist):
        self._cache_channel(channel)
        print 'SILC: Reply (Ban):', channel
        
    def command_reply_detach(self):
        print 'SILC: Reply (Detach)'
        
    def command_reply_watch(self):
        print 'SILC: Reply (Watch)'        
        
    def command_reply_silcoper(self):
        print 'SILC: Reply (SilcOper)'
        
    def command_reply_leave(self, channel):
        self._cache_channel(channel)
        print 'SILC: Reply (Leave):', channel
        
    def command_reply_users(self, channel, users):
        self._cache_user(user)
        print 'SILC: Reply (Users):', type, user
        
    def command_reply_service(self, *args):
        pass # not implemented

    def command_reply_failed(self, command, commandstr, errorcode, errormsg):
        # global catching failed commands and their error codes
        print 'SILC: Reply (FAILED)!', commandstr, errormsg


if __name__ == "__main__":
    import sys
    c = SupySilcClient()
    try:
        while True:
            c.run_one()        
            time.sleep(0.1)
    except KeyboardInterrupt:
        pass
