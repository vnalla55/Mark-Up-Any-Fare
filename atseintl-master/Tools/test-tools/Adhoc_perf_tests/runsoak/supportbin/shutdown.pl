#!/usr/bin/perl

use IO::Socket;
use File::Basename; 

$numArgs = @ARGV;
if($numArgs < 2)
{
   print "Invalid number of arguments.\n";
   print "Usage: shutdown.pl <host> <port>\n";
   exit(1);
}

shutdownServer($ARGV[0], $ARGV[1]);


################################################################################
#
# shutdownServer
#
# Send shutdown signal to the Server
#
################################################################################
sub shutdownServer
{
   my ( $host, $port ) = @_;

   my $cmd = 'DOWN';
   my $HEADER_TEMPLATE = "N N A4 A4 A4";
   my $FULL_HEADER_LEN = 20;
   my $HEADER_LEN      = 16;
   my $HEADER_VER      = "0001";
   my $HEADER_REV      = "0000";

   my $recvMesg = "";
   my $message = "";
   my $payloadLen = 0;

   $socket = IO::Socket::INET->new( Proto => 'tcp',
                                    PeerAddr => $host,
                                    PeerPort => $port,);
   if($socket)
   {
      $message = pack($HEADER_TEMPLATE, $HEADER_LEN, $payloadLen, $cmd,
                      $HEADER_VER, $HEADER_REV);
      $socket->send($message);

      $socket->recv($header, $FULL_HEADER_LEN);
      ($headerLen, $payloadLen, $command, $version, $revision) =
          unpack($HEADER_TEMPLATE, $header);

      $toRecv = $payloadLen;

      while ($toRecv > 0)
      {
         $socket->recv($data, $toRecv);
         $recvMesg .= $data;
         $toRecv -= length($data);
      }
      close($socket);

      print STDOUT "Successfully sent shutdown command to server [$host] \n";
      return( 1 );
   }

   print STDERR "Unable to open socket to the server [$host] \n";
   return( 0 );
}
1;
