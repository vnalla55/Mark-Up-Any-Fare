#!/usr/bin/perl
use IO::Socket;

local $| = 1;

%serverStats = ();

################################################################################
#
# output TYPE, LINE
#
#   Writes out LINE along with the current time to either STDOUT or STDERR
#   depending on TYPE. TYPE can either be INFO or ERROR.  INFO is directed to 
#   STDOUT and ERROR is directed to STDERR.  If TYPE is neither INFO or ERROR
#   the line will be directed to STDERR.
#
#   ex.  output(INFO, "Information to output to STDOUT");
#
################################################################################

use constant INFO => 0;
use constant ERROR => 1;

sub output
{
   my ($type, $line) = @_;
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

   $year += 1900;
   $mon  += 1;

   my $log_time = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $year,$mon,$mday,$hour,$min,$sec);

   if ($type == INFO)
   {
      printf STDOUT "%s | %s\n", $log_time , $line;
   }
   else
   {
      printf STDERR "%s | ERROR! %s\n", $log_time, $line;
   }

   return;
}

################################################################################
#
################################################################################

$HEADER_TEMPLATE = "N N A4 A4 A4";
$FULL_HEADER_LEN = 20;
$HEADER_LEN      = 16;
$HEADER_VER      = '0001';
$HEADER_REV      = '0000';
$DELIM           = '|';

$DEFAULT_PORT    = '5001';

$CMD_DCFG = 'DCFG';

sub sendReceive
{
   my ($host, $port, $cmd, $payload) = @_;
   my $recvMesg = "";
   my $message = "";
   my $data    = "";   

   $socket = IO::Socket::INET->new( Proto => 'tcp', PeerAddr => $host, PeerPort => $port,);
   if ($socket)
   {
      my $payloadLen = length($payload);

      $message = pack($HEADER_TEMPLATE, $HEADER_LEN, $payloadLen, $cmd, $HEADER_VER, $HEADER_REV);
      if ($payloadLen > 0)
      {
         $message .= $payload;
      }

      $socket->send($message);

      $socket->recv($header, $FULL_HEADER_LEN);
      ($headerLen, $payloadLen, $command, $version, $revision) = unpack($HEADER_TEMPLATE, $header);

      $toRecv = $payloadLen;

      while ($toRecv > 0)
      {
         $socket->recv($data, $toRecv);
         $recvMesg .= $data;
         $toRecv -= length($data);
      }

      close($socket);
   }
   else
   {
      output(ERROR, "Unable to connect to $host:$port");
   }

   return $recvMesg;
}

################################################################################
#
################################################################################

$numArgs = $#ARGV + 1;

if($numArgs != 2)
{
   print "\ndcfg.pl - invalid arguments\n";
   print "   ex. dcfg.pl host port\n";
   print "       where host = hostname to connect to\n";
   print "             port = port number to connect to.\n";
   exit;
}

$host = $ARGV[0];
$port = $ARGV[1];
$payload = "";

$message = sendReceive($host, $port, $CMD_DCFG, $payload);
if ($message eq "")
{
   print "\nEmpty response from server!\n\n";
}
else
{
   print "\n$message\n\n";
}
