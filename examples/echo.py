import time
import silc

class EchoClient(silc.SilcClient):

      def channel_message(self, sender, channel, flags, message):
          print message
          self.send_channel_message(channel, message)

      def private_message(self, sender, flags, message):
          print message
          self.send_private_message(sender, message)

      def running(self):
          print "* Running"
          client.connect_to_server("silc.example.com")

      def connected(self):
          print "* Connected"
          self.command_call("JOIN #cam")

      def disconnected(self, msg):
          print "* Disconnected: %s" % msg

      # catch responses to commands

      def command_reply_join(self, channel, name, topic, hmac, x, y,
          users):

          print "* Joined channel %s" % name
          self.send_channel_message(channel, "Hello!")
           
      # catch async notifications from the server

      def notify_join(self, user, channel):
          print "* A user named %s has joined the channel %s" % \
                (user.username, channel.channel_name)
          self.send_channel_message(channel, "Hello, %s" %
          user.username)

if __name__ == "__main__":
   keys = silc.create_key_pair("silc.pub", "silc.prv", passphrase = "")
   client = EchoClient(keys, "echobot", "echobot", "Echo Bot")

   while True:
       try:
           client.run_one()
           time.sleep(0.2)
       except KeyboardInterrupt:
           break
