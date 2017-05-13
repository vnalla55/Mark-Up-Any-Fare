#!/usr/bin/perl

use IO::Socket;

local $| = 1;

# Global constats used for sending app console commands
my $HEADER_TEMPLATE = "N N A4 A4 A4";
my $FULL_HEADER_LEN = 20;
my $HEADER_LEN      = 16;
my $HEADER_VER      = "0001";
my $HEADER_REV      = "0000";
my $CMD_SSTATS      = 'RFSH';
my $CMD_CSTATS      = 'TATS';
my $CMD_ELAPSED     = 'LAPS';

# Global Variables to keep track of the previous loop
my $totalTrx = 0;
my $totalCPU = 0;
my $totalElapsed = 0;
my $virtualMemory = 0;
my $toalDBCount = 0;
my $totalDBElapsed = 0;
my $totalItins = 0;
my $totalResponseSize = 0;
my $host = "";
my $port = "";
my $sleepSeconds = 60;
my $maxDuration = 0;

$numArgs = $#ARGV + 1;
if($numArgs >= 2)
{
    $host = $ARGV[0];
    $port = $ARGV[1];
}
else
{
    print "\ngetstats2.pl - invalid arguments\n";
    print "Usage: \n";
    print "getstats.pl <host> <port> [<duration>] [<sleeptime>]\n";
    print "  where host      = hostname to connect to\n";
    print "        port      = port number to connect to\n";
    print "        duration  = num of minutes to capture stats\n";
    print "        sleeptime = num of minutes to delay between every capture\n";
    exit;
}
$maxDuration  = $ARGV[2] * 60 if($numArgs >= 3);
$sleepSeconds = $ARGV[3] * 60 if($numArgs == 4);

#print header to output
printHeader();

my $duration = 0; #counter to increment the duration
while($maxDuration == 0 || $duration <= $maxDuration)
{
    my ($trxCount, $cpu, $elapsed, $memory, $dbCount, $dbElapsed,
        $dbHost, $itins, $responseSize ) = getStats($host, $port);
    if($trxCount > $totalTrxCount)
    {
        # skip first time to compute and write
        if($totalTrxCount gt 0)
        {
            my $trxCountLS = $trxCount - $totalTrxCount;
            my $cpuLS = $cpu - $totalCPU;
            my $elapsedLS = $elapsed - $totalElapsed;
            my $dbCountLS = $dbCount - $totalDBCount;
            my $dbElapsedLS = $dbElapsed - $totalDBElapsed;
            my $responseSizeLS = $responseSize - $totalResponseSize;
            my $itinsLS = $itins - $totalItins;
            
            my $avgCpuLS = $cpuLS/$trxCountLS;
            my $avgElapsedLS = $elapsedLS/$trxCountLS;
            my $avgDbElapsedLS = $dbElapsedLS/$trxCountLS;
            my $avgResponseSizeLS = $responseSizeLS/$trxCountLS;
            my $avgItinsLS = $itinsLS/$trxCountLS;

            my $avgCpu = $cpu/$trxCount;
            my $avgElapsed = $elapsed/$trxCount;
            my $avgDbCount = $dbCount/$trxCount;
            my $avgDbElapsed = $dbElapsed/$trxCount;

            printData($trxCount, $avgCpu, $avgElapsed,
                $avgDbCount, $avgDbElapsed,
                $trxCountLS, $avgCpuLS, $avgElapsedLS, $avgDbElapsedLS,
                $dbCountLS, $avgResponseSizeLS, $avgItinsLS, $memory);
        }

        # set current sample counters to the global variables
        $totalTrxCount = $trxCount;
        $totalCPU = $cpu;
        $totalElapsed = $elapsed;
        $totalDBCount = $dbCount;
        $totalDBElapsed = $dbElapsed;
        $virtualMemory = $memory;
        $totalResponseSize = $responseSize;
        $totalItins = $itins;
    }
    sleep($sleepSeconds);
    $duration += $sleepSeconds;
}
exit(0);


# Get Stats from app console
sub getStats
{
    my ($host, $port) = @_;
    my $trxCount  = 0;
    my $cpu       = 0;
    my $elapsed   = 0;
    my $memory    = 0;
    my $dbCount   = 0;
    my $dbElapsed = 0;
    my $dbHost    = 0;
    my $itins     = 0;
    my $responseSize = 0;

    $stats = sendReceive($host, $port, $CMD_SSTATS, "");
    if ($stats =~ m/^<STATS.*?CP="(.*?)" IT="(.*?)".*? DB="(.*?)".*? DQ="(.*?)" VM="(.*?)" DH="(.*?)".*? NM="TRX" OK="(.*?)" ER="(.*?)" ET="(.*?)".*?RS="(.*?)"/)
    {
        $cpu = $1;
        $itins = $2;
        $dbCount = $3 + $4;
        $memory = $5;
        $dbHost = $6;
        $trxCount = $7 + $8;
        $elapsed = $9;
        $responseSize = $10;
    }
    $elapsedstats = sendReceive($host, $port, $CMD_ELAPSED, "");
    if ($elapsedstats =~ m/^<ELAPSED.*? DS="(.*?)"/)
    {
        $dbElapsed = $1;
    }
    return( $trxCount, $cpu, $elapsed, $memory, $dbCount, $dbElapsed,
            $dbHost, $itins, $responseSize );
}

# Method to send app console command and receive response
sub sendReceive
{
   my ($host, $port, $cmd, $payload) = @_;
   my $recvMesg = "";
   my $message  = "";
   my $data     = "";   

   $socket = IO::Socket::INET->new( Proto => 'tcp', PeerAddr => $host,
                                    PeerPort => $port,);
   if ($socket)
   {
      my $payloadLen = length($payload);
      $message = pack($HEADER_TEMPLATE, $HEADER_LEN, $payloadLen, $cmd,
                      $HEADER_VER, $HEADER_REV);
      if ($payloadLen > 0)
      {
         $message .= $payload;
      }

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
      return $recvMesg;
   }
   return undef;
}

# Prints Column Headers
sub printHeader
{
    print("Time");                             #1st Column
    print(",Total Trx");                       #2nd
    print(",Avg CPU");                         #3rd
    print(",Avg Elapsed");                     #4th
    print(",Avg DB Read Count");               #5th
    print(",Avg DB Elapsed");                  #6th
    print(",Virtual Memory");                  #7th
    print(",Last Sample Trx");                 #8th
    print(",Last Sample Avg CPU");             #9th
    print(",Last Sample Avg Elapsed");         #10th
    print(",Last Sample Avg Response Size");   #11th
    print(",Last Sample Avg Itins");           #12th
    print("\n");
}

# Prints the data
sub printData
{
    my ($totalTrxCount, $avgCpu, $avgElapsed,
        $avgDbCount, $avgDbElapsed,
        $trxCountLS, $avgCpuLS, $avgElapsedLS, $avgDbElapsedLS,
        $dbCountLS, $avgResponseSizeLS, $avgItinsLS, $virtualMemory) = @_;
    my $localTime = getLocalTime();

    print($localTime);
    print("," . $totalTrxCount);
    print("," . $avgCpu);
    print("," . $avgElapsed);
    print("," . $avgDbCount);
    print("," . $avgDbElapsed);
    print("," . $virtualMemory);
    print("," . $trxCountLS);
    print("," . $avgCpuLS);
    print("," . $avgElapsedLS);
    print("," . $avgResponseSizeLS);
    print("," . $avgItinsLS);
    print("\n");
}

sub getLocalTime
{
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

   $year += 1900;
   $mon  += 1;

   my $localTime = sprintf("%4d-%02d-%02d %02d:%02d:%02d", $year,$mon,$mday,$hour,$min,$sec);
   return($localTime);
}
