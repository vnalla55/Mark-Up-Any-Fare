#!/usr/bin/perl

local $| = 1;

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

$prevCPU = 0;
$prevElapsed = 0;
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
$totalElapsed = 0;
$freemem  = 0;
$totalTRX = 0;
$avgCPU   = 0;
$avgElapsed = 0;
$lastTRXSample = 0;
$lastCPUSample = 0;
$lastElapsedSample = 0;

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

#2007-11-30 09:39:10,<STATS ST="1196371331" CP="48523.5" IT="67501" DB="2" SC="0" CT="0" L1="0.0649414" L2="0.0258789" L3="0.00537109" FM="20882480" ><SP NM="TRX" OK="29234" ER="960" ET="44876.8" RQ="2447022585" RS="846368981" /><SP NM="AS2" OK="19402" ER="25" ET="19880.5" RQ="47888025" RS="47968782" /><SP NM="DSS" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="BAG" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="BIL" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="REQ" OK="0" ER="0" ET="0" RQ="0" RS="0" /><SP NM="RTG" OK="0" ER="0" ET="0" RQ="0" RS="0" /></STATS>^@<ELAPSED FC="7713.01" FV="4988.53" PO="10518.5" SS="0.283936" IA="20060.1" TX="270.823" CA="34.4404" CU="0" MI="0" IN="0" FD="0" FS="0" />^@,3,379,0,0,5587,0,0,0,72589,0,0,0,257,0,3388,0,0,223,15,0,2887,14,0,0,817,19902,0,790,0,0,253,25,281,1070,16,192,815,26,4743,0,15,0,10,66,1,30,171,80,53,316,668,15,0,34,1555,9943,96,134,0,22432,175,0,0,0,2321,0,8791,216,1920006,0,324,60,28,51,4,72,296,274,44,10,65,72,35,35,0,1,3,1,0,0,0,0,0,14,0,0,0,2546,147,199877,0,0,114381,0,929,1,361,1,0,2059,26,452,157,157,1,1,84,14619,0,0,0,915,0,1069,18,433,7025,143,11731,32,0,44,0,2,50,0,515,292,2,986,299,299,225,970,225,240,5,6757,5041,184,180,0,0,7,65,1,15,2171,453,202,86,1434,1,426,397,52,1,0,73,2035,0,0,0,0,44,32,85,12326,45,7600,81,2263,2,161,0,70,13,312,3170,236,420,1035,756,49,0,24,206,27,1,568,0,1729,1006,73,467,0,0,0,72,70,1837,0


while(my $line = <STDIN>)
{
   if ($line =~ m/(.*?),<STATS.*?CP="(.*?)".*? FM="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)" ET="(.*?)".*<ELAPSED.*\/>(.{1}),(.*)/)
   {
      $time = $1;
      $totalCPU = $2;
      $freemem  = $3; 
      $totalTRX = $4 + $5;
      $totalElapsed = $6;
      $cacheStats = $8;

      if ($totalTRX == 0)
      {
         $avgCPU = 0;
         $avgElapsed = 0;
      }
      else
      {
         $avgCPU = $totalCPU / $totalTRX;
         $avgElapsed = $totalElapsed / $totalTRX;
      }

      $lastTRXSample = $totalTRX - $prevTRX;
      if ($lastTRXSample == 0)
      {
         $lastCPUSample = 0;
         $lastElapsedSample = 0;
      }
      else
      {
         $lastCPUSample = ($totalCPU - $prevCPU) / $lastTRXSample;
         $lastElapsedSample = ($totalElapsed - $prevElapsed) / $lastTRXSample;
      }
 
      #print("$time,$freemem,$totalTRX,$avgElapsed,$avgCPU,$lastTRXSample,$lastElapsedSample,$lastCPUSample,$cacheStats\n");
      print("$time,$freemem,$totalTRX,$avgElapsed,$avgCPU,$lastTRXSample,$lastElapsedSample,$lastCPUSample\n");

      $prevTRX = $totalTRX;
      $prevCPU = $totalCPU;
      $prevElapsed = $totalElapsed;

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
#      $elapsedstats = sendReceive($host, $port, $CMD_ELAPSED, "");
#      if ($elapsedstats =~ m/^<ELAPSED.*? FC="(.*?)".*? FV="(.*?)".*? PO="(.*?)".*? SS="(.*?)".*? IA="(.*?)".*? TX="(.*?)".*? CA="(.*?)".*? CU="(.*?)".*? MI="(.*?)".*? IN="(.*?)".*? FD="(.*?)".*? FS="(.*?)"/)
#      {
#$stats = $stats . $elapsedstats;
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
#      }
#      else
#      {
#         output(ERROR, "Unable to send to $host:$port!");
#      }
#
#output(INFO, $stats);
#
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
   }
   else
   {
      output(ERROR, "Unable to parse data record!");
   }
}

