#!/usr/bin/perl
use IO::Socket;

local $| = 1;

$host = 'piclp400';
$port = '5001';

$SLEEPSECS = 60;

#%servers = ('ttfhlp100','51023','ttfhlp101','51023');
#%servers = ('plab037','5001','plab038','5001','plab039','5001','plab040','5001');

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
      printf STDOUT "%s,%s", $log_time , $line;
   }
   else
   {
      printf STDERR "%s, ERROR! %s\n", $log_time, $line;
   }

   return;
}

################################################################################
#
################################################################################

$HEADER_TEMPLATE = "N N A4 A4 A4";
$FULL_HEADER_LEN = 20;
$HEADER_LEN      = 16;
$HEADER_VER      = "0001";
$HEADER_REV      = "0000";

$CMD_SSTATS      = 'RFSH';
$CMD_CSTATS      = 'TATS';
$CMD_ELAPSED     = 'LAPS';

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

      return $recvMesg;
   }
   else
   {
      return undef;
   }
}

################################################################################
#
################################################################################

$numArgs = $#ARGV + 1;

if($numArgs == 2)
{
   $host = $ARGV[0];
   $port = $ARGV[1];
}
else
{
   print "\ngetstats.pl - invalid arguments\n";
   print "   ex. getstats.pl [host] [port]\n";
   print "       where host = hostname to connect to\n";
   print "             port = port number to connect to.\n";
   exit;
}

$prevCPU = 0;
$prevTRX = 0;
$prevFC = 0;
$prevFV = 0;
$prevPO = 0;
$prevSS = 0;
$prevIA = 0;
$prevTX = 0;
$prevCA = 0;
$prevCU = 0;
$prevMI = 0;
$prevIN = 0;
$prevFD = 0;
$prevFS = 0;
$totalTRX = 0;
$totalCPU = 0;
$freemem  = 0;
$totalTRX = 0;
$avgCPU   = 0;
$lastTRXSample = 0;
$lastCPUSample = 0;

print "Time,Free Memory,Total TRX,Avg CPU,TRX Last Sample,Avg CPU Last Sample,Fares Collection Last Sample,Fares Validation Last Sample,Pricing Last Sample,Shopping Last Sample,Itin Analyzer Last Sample,Tax Last Sample,Fare Calc Last Sample,Currency Last Sample,Mileage Last Sample,Internal Last Sample,Fare Display Last Sample,Fare Selector Last Sample";

$message = sendReceive($host, $port, $CMD_CSTATS, "");
if ($message)
{
   $str = $message;

   while ($str =~ m/(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|/)
   {
      print(",$1");

      $str = $';
   }
   print "\n";
}
else
{
   output(ERROR, "Unable to send to $host:$port!");
}

while(1)
{
#stats=<STATS ST="1195086104" CP="211.97" IT="1122" DB="0" SC="0" CT="4" L1="3.88623" L2="3.76074" L3="3.45459" FM="21182080" ><SP NM="TRX" OK="90" ER="66" ET="605.84" RQ="1196660" RS="6174638" /><SP NM="AS2" OK="61" ER="0" ET="22.4303" RQ="142152" RS="136403" /><SP NM="DSS" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="BAG" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="BIL" OK="156" ER="0" ET="0.11493" RQ="55661" RS="2808" /><SP NM="REQ" OK="156" ER="0" ET="0.542953" RQ="1970831" RS="2808" /><SP NM="RTG" OK="0" ER="0" ET="0" RQ="0" RS="0" /></STATS>

   $stats = sendReceive($host, $port, $CMD_SSTATS, "");
   if ($stats =~ m/^<STATS.*?CP="(.*?)".*? FM="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)"/)
   {
#print "$stats\n";
#    os << "<STATS"
#       << " ST=\""<<_acStats->startTime<<"\""
#       << " CP=\""<<_acStats->totalCPUTime<<"\""
#       << " IT=\""<<_acStats->totalItins<<"\""
#       << " DB=\""<<_acStats->dbErrorCount<<"\""
#       << " SC=\""<<_acStats->socketClose<<"\""
#       << " CT=\""<<_acStats->concurrentTrx<<"\""
#       << " L1=\""<<_acStats->loadLast1Min<<"\""
#       << " L2=\""<<_acStats->loadLast5Min<<"\""
#       << " L3=\""<<_acStats->loadLast15Min<<"\""
#       << " FM=\""<<_acStats->freeMemKB<<"\"";
#
#    os<<"<SP"
#      <<" NM=\""<<name                <<"\""
#      <<" OK=\""<<sp.totalOkCount     <<"\""
#      <<" ER=\""<<sp.totalErrorCount  <<"\""
#      <<" ET=\""<<sp.totalElapsedTime <<"\""
#      <<" RQ=\""<<sp.totalReqSize     <<"\""
#      <<" RS=\""<<sp.totalRspSize     <<"\""
#      <<" />";

#      $totalCPU = $1;
#      $freemem  = $2; 
#      $totalTRX = $3 + $4;
#
#      if ($totalTRX == 0)
#      {
#         $totalTRX = 1;
#      }
#
#      $avgCPU   = $totalCPU / $totalTRX;
#
#      $lastTRXSample = $totalTRX - $prevTRX;
#      $lastCPUSample = ($totalCPU - $prevCPU) / $lastTRXSample;
# 
#      output(INFO, "$freemem,$totalTRX,$avgCPU,$lastTRXSample,$lastCPUSample");

#      $prevTRX = $totalTRX;
#      $prevCPU = $totalCPU;

#      $message = sendReceive($host, $port, $CMD_CSTATS, "");
#      if ($message)
#      {
#         $str = $message;
#   
#         while ($str =~ m/(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|/)
#         {
#            print(",$3");
#   
#            $str = $';
#         }
#         print "\n";
#      }
#      else
#      {
#         output(ERROR, "Unable to send to $host:$port!");
#      }

#    oss << "<ELAPSED"
#        << " FC=\"" << _acStats->faresCollectionServiceElapsed  << "\""
#        << " FV=\"" << _acStats->faresValidationServiceElapsed  << "\""
#        << " PO=\"" << _acStats->pricingServiceElapsed          << "\""
#        << " SS=\"" << _acStats->shoppingServiceElapsed         << "\""
#        << " IA=\"" << _acStats->itinAnalyzerServiceElapsed     << "\""
#        << " TX=\"" << _acStats->taxServiceElapsed              << "\""
#        << " CA=\"" << _acStats->fareCalcServiceElapsed         << "\""
#        << " CU=\"" << _acStats->currencyServiceElapsed         << "\""
#        << " MI=\"" << _acStats->mileageServiceElapsed          << "\""
#        << " IN=\"" << _acStats->internalServiceElapsed         << "\""
#        << " FD=\"" << _acStats->fareDisplayServiceElapsed      << "\""
#        << " FS=\"" << _acStats->fareSelectorServiceElapsed     << "\""
#        << " />";
#
      $elapsedstats = sendReceive($host, $port, $CMD_ELAPSED, "");
      if ($elapsedstats =~ m/^<ELAPSED.*? FC="(.*?)".*? FV="(.*?)".*? PO="(.*?)".*? SS="(.*?)".*? IA="(.*?)".*? TX="(.*?)".*? CA="(.*?)".*? CU="(.*?)".*? MI="(.*?)".*? IN="(.*?)".*? FD="(.*?)".*? FS="(.*?)"/)
      {
$stats = $stats . $elapsedstats;
#         $FC = $1;
#         $FV = $2;
#         $PO = $3;
#         $SS = $4;
#         $IA = $5;
#         $TX = $6;
#         $CA = $7;
#         $CU = $8;
#         $MI = $9;
#         $IN = $10;
#         $FD = $11;
#         $FS = $12;
#
#         $lastFC = $FC - $prevFC;
#         $lastFV = $FV - $prevFV;
#         $lastPO = $PO - $prevPO;
#         $lastSS = $SS - $prevSS;
#         $lastIA = $IA - $prevIA;
#         $lastTX = $TX - $prevTX;
#         $lastCA = $CA - $prevCA;
#         $lastCU = $CU - $prevCU;
#         $lastMI = $MI - $prevMI;
#         $lastIN = $IN - $prevIN;
#         $lastFD = $FD - $prevFD;
#         $lastFS = $FS - $prevFS;
#
#         $lastFCavg = $lastFC / $lastTRXSample;
#         $lastFVavg = $lastFV / $lastTRXSample;
#         $lastPOavg = $lastPO / $lastTRXSample;
#         $lastSSavg = $lastSS / $lastTRXSample;
#         $lastIAavg = $lastIA / $lastTRXSample;
#         $lastTXavg = $lastTX / $lastTRXSample;
#         $lastCAavg = $lastCA / $lastTRXSample;
#         $lastCUavg = $lastCU / $lastTRXSample;
#         $lastMIavg = $lastMI / $lastTRXSample;
#         $lastINavg = $lastIN / $lastTRXSample;
#         $lastFDavg = $lastFD / $lastTRXSample;
#         $lastFSavg = $lastFS / $lastTRXSample;
#
#         print ",$lastFCavg,$lastFVavg,$lastPOavg,$lastSSavg,$lastIAavg,$lastTXavg,$lastCAavg,$lastCUavg,$lastMIavg,$lastINavg,$lastFDavg,$lastFSavg";
#
#         $prevFC = $FC;
#         $prevFV = $FV;
#         $prevPO = $PO;
#         $prevSS = $SS;
#         $prevIA = $IA;
#         $prevTX = $TX;
#         $prevCA = $CA;
#         $prevCU = $CU;
#         $prevMI = $MI;
#         $prevIN = $IN;
#         $prevFD = $FD;
#         $prevFS = $FS;
      }
      else
      {
         output(ERROR, "Unable to send to $host:$port!");
      }

output(INFO, $stats);

      $message = sendReceive($host, $port, $CMD_CSTATS, "");
      if ($message)
      {
         $str = $message;

         while ($str =~ m/(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|(\S*?)\|/)
         {
            print(",$3");

            $str = $';
         }
         print "\n";
      }
      else
      {
         output(ERROR, "Unable to send to $host:$port!");
      }
   }
   else
   {
      output(ERROR, "Unable to send to $host:$port!");
   }

   sleep($SLEEPSECS);
}

