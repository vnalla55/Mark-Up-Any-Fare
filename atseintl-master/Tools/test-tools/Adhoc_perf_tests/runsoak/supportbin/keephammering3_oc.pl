#!/usr/bin/perl

use lib "/opt/atseintl/shopperftest/lib/perl";

use IO::Socket;
use Net::SCP qw(scp iscp);
scp($source, $destination);
use warnings;
use ProdServers;

my $mipDir = "/opt/atseintl/shopping";
my $isDir  = "/opt/atseintl/shoppingis";

my $pltoolexe = "/opt/support/scripts/shopping/bin/pltraffictool3";

use constant IS_ALL    => 0;
use constant MIP_ALL   => 1;
use constant IS        => 2;
use constant MIP       => 3;
use constant IS_EXP    => 4;
use constant MIP_EXP   => 5;
use constant IS_TVLY   => 6;
use constant MIP_TVLY  => 7;
use constant IS_DPOOL  => 8;
use constant MIP_DPOOL => 9;

use constant INFO  => 0;
use constant ERROR => 1;

$numArgs = $#ARGV + 1;

if($numArgs == 4)
{
   $serverType = $ARGV[0];
   $pltoolConfigFile = $ARGV[1];
   $logfile = $ARGV[2];
   $duration = $ARGV[3] * 60;
}
elsif($numArgs == 3)
{
   $serverType = $ARGV[0];
   $pltoolConfigFile = $ARGV[1];
   $logfile = $ARGV[2];
   $duration = 0;
}
else
{
   print "\nkeephammering.pl - invalid arguments\n";
   print "   ex. keephammering.pl <serverType> <pltoolConfigFile> <logFile> [<duration>]\n";
   print "       where serverType = 0..5\n";
   print "                 0 => IS ALL\n";
   print "                 1 => MIP ALL\n";
   print "                 2 => IS General/Common Pool\n";
   print "                 3 => MIP General/Common Pool\n";
   print "                 4 => IS Expedia 200 Options\n";
   print "                 5 => MIP Expedia 200 Options\n";
   print "                 6 => IS Travelocity 200 Options\n";
   print "                 7 => MIP Travelocity 200 Options\n";
   print "                 8 => IS D Pool (EMEA)          \n";
   print "                 9 => MIP D Pool (EMEA)          \n";
   print "             pltoolConfigFile = config file that will be supplied to pltraffictool\n";
   print "             logFile = requests log file that is set in the pltooConfigFile\n";
   print "             duration = number of minutes to run\n";
   exit;
}

my $prdServers = new ProdServers;
$prdServers->initServers( "/opt/atseintl/shopperftest/ncm.csv" );

# setup test duration
my $teststarttime = time();
my $testendtime = $teststarttime + $duration;
my $testcurrenttime = time();

while( $duration == 0 || $testcurrenttime < $testendtime )
{
   my $server = $prdServers->getServerName( $serverType );
   unlink($logfile);
   print "Getting tserequest.log file from server [$server]\n";
   ($rc) = getSocketLog($serverType, $server, $logfile);
   if ($rc == 0)
   {
       print "Got log file \n";
       $cmd = "$pltoolexe $pltoolConfigFile";
       print "Executing = $cmd\n";
       system("$cmd");
   }
   removeCoreFiles(".");
   $testcurrenttime = time();
}

################################################################################
#
# removeCoreFiles DIRNAME
#
################################################################################
sub removeCoreFiles
{
    my ($dirname) = (@_);
    my @fileslist;

    if (opendir(DIR, $dirname))
    {
        @fileslist = grep /core.*/, readdir(DIR);
        closedir(DIR);
    }

    unlink(@fileslist);
}

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
# getSocketLog NODETYPE, SERVER
#
################################################################################
sub getSocketLog
{
    my ($nodeType, $server, $resultRequestFile) = @_;
    my $localLogFileName = $resultRequestFile . ".orig";
    my $socketLog = "tserequest.log";
    my $directory = "";
    my $error = 0;
    my $userid = "ATSESTAT";
   
    if ($nodeType == MIP_ALL || $nodeType == MIP || $nodeType == MIP_EXP ||
        $nodeType == MIP_TVLY || $nodeType == MIP_DPOOL)
    {
        $directory = $mipDir . "/";
    }
    elsif ($nodeType == IS_ALL || $nodeType == IS || $nodeType == IS_EXP ||
           $nodeType == IS_TVLY || $nodeType == IS_DPOOL)
    {
        $directory = $isDir . "/";
    }
    else
    {
        return(1);
    }

    output(INFO, "Connecting to [$server]"); 
  
    $scp = Net::SCP->new( $server, $userid ) or $error = 1;
    if ($error)
    {
        output(ERROR, "Failure in connecting to [$server]!"); 
        return($error);
    }

    my $requestLog = $directory . "tserequest.log.1";

    output(INFO, "Getting Request Log [$requestLog] from [$server]");

    $scp->get($requestLog, $localLogFileName) or $error = 1;
    if ($error)
    {
        $error = 0;

        output(ERROR, "Failure to get log file [$requestLog]!");

        $requestLog = $directory . "tserequest.log";
        output(INFO, "Getting Log [$requestLog] from [$server]");

        $scp->get($requestLog, $localLogFileName) or $error = 1;
        if ($error)
        {
            output(ERROR, "Failure to get log file [$requestLog]!");
            $scp->quit;
            return($error);
        }
    }

    $scp->quit;

    $fileSize = -s $localLogFileName;
    output(INFO, "Socket Log[$localLogFileName], $fileSize bytes");

    prepareRequests( $localLogFileName, $resultRequestFile );
    unlink( $localLogFileName );

    return($error);
}

sub prepareRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $count = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        # presently tserequest.log for shopping requests do not log first
        # 8 characters due to a bug in the application. To overcome that bug
        # following substitution is added. After that bug is fixed, this
        # substitution can be removed.
        $line =~ s/ gRequest / <ShoppingRequest /;
        #$line =~ s/D70="[0-9][0-9]"//;
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;
        $line =~ s/<PRO /<PRO SEY="T" /;

        if( $line =~ m/<\/ShoppingRequest>/ ) {
            $outputData .= "<RFG S01=\"AE\" />\n";
        }

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

