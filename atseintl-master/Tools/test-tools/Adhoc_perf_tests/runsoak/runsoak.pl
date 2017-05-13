#!/usr/bin/perl -w

use lib "/opt/atseintl/shopperftest/lib/perl";

use IO::Socket;
use AppConsoleCommands;
use ReportGen;
use ProdServers;
use FileHandle;
use Net::SCP qw(scp iscp);
scp($source, $destination);
use warnings;
use threads;
use Thread::Queue;

my $mipDir = "/opt/atseintl/shopping";
my $isDir  = "/opt/atseintl/shoppingis";
my $templateConfigDir = "/opt/atseintl/shopperftest/cfgtemplates/";
my $pltoolexe = "/opt/atseintl/shopperftest/supportbin/pltraffictool3";
my $targetReportRootDir = "/opt/atseintl/shopreports/";
my $errorDescriptionsFile = "/opt/atseintl/shopperftest/error_descriptions.txt";
my $ncmFile = "/opt/atseintl/shopperftest/ncm.csv";
my $maxConcurrentRequests = 48; # default for G6 hardware
my $configScriptDir = "./Config";
my $configsComparision = "";

our $getTPSFunc = "getTPS";

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
use constant IS_GPOOL  => 10;
use constant MIP_GPOOL => 11;
use constant IS_HPOOL  => 12;
use constant MIP_HPOOL => 13;

use constant INFO  => 0;
use constant ERROR => 1;

$numArgs = $#ARGV + 1;
if($numArgs == 2)
{
   $serverType = $ARGV[0];
   $configFile = $ARGV[1];
}
elsif($numArgs == 3)
{
   $serverType = $ARGV[0];
   $configFile = $ARGV[1];
   if($ARGV[2] eq "G2") {
      $maxConcurrentRequests = 15;
      $getTPSFunc = "getTPS_G2";
   }
}
else
{
   print "\nrunsoak.pl - invalid arguments\n";
   print "   ex. runsoak.pl <serverType> <test-config-file> [G2]\n";
   print "       where serverType = 0..9\n";
   print "                 0  => IS ALL\n";
   print "                 1  => MIP ALL\n";
   print "                 2  => IS General/Common Pool\n";
   print "                 3  => MIP General/Common Pool\n";
   print "                 4  => IS Expedia 200 Options\n";
   print "                 5  => MIP Expedia 200 Options\n";
   print "                 6  => IS Travelocity Domestic\n";
   print "                 7  => MIP Travelocity Domestic\n";
   print "                 8  => IS D Pool (EMEA)\n";
   print "                 9  => MIP D Pool (EMEA)\n";
   print "                 10 => IS G Pool (eTraveli)\n";
   print "                 11 => MIP G Pool (eTraveli)\n";
   print "                 12 => IS Travelocity International\n";
   print "                 13 => MIP Travelocity International\n";
   print "             configFile = test configuration file\n";
   exit;
}

my $statsSleepSeconds = 2 * 60; 
my $reportSleepSeconds = 5 * 60; 

my( $id, $title, $controlServer, $testServer, $port, $acPort,
    $controlBaseline, $testBaseline, $maxDuration,
    $instrDbServer, $reportDirBase, $testType, $filters )
    = parseConfigFile($serverType, $configFile);

$configsComparision = &getNodeConfigsComparision($configScriptDir, $controlServer, $testServer, $port);

$controlStatsFile = $controlServer . ".csv";
$testStatsFile = $testServer . ".csv";
$controlDatabase = getAppDBInfo($controlServer, $acPort);
$testDatabase = getAppDBInfo($testServer, $acPort);

$testDate = getLocalDate();
$startTime = getLocalTime();
$reportDir = $reportDirBase . $id;
$reportFile = $id . ".html";

my $prdServers = new ProdServers;
$prdServers->initServers( $ncmFile );

my $thr1 = threads->create(\&sendTraffic,
    $serverType, $id, $controlServer, $testServer, $port, $maxDuration);

my $thr2 = threads->create(\&getStats,
    $controlServer, $acPort, $controlStatsFile,
    $testServer, $acPort, $testStatsFile,
    $maxDuration, $statsSleepSeconds);

generateReport($title, $testDate, $reportDir, $reportFile,
        $controlServer, $controlBaseline, $controlDatabase, $controlStatsFile,
        $testServer, $testBaseline, $testDatabase, $testStatsFile,
        $startTime, $instrDbServer, $testType, $maxDuration,
        $reportSleepSeconds, $acPort, $configsComparision);

# may not need to wait for other threads to finish becaue report thread
# already finished processing and there is no need to wait for request
# processing thread to complete.
#$thr1->join();
#$thr2->join();

# cleanup files
unlink($controlStatsFile);
unlink($testStatsFile);
unlink($id . "_pltool.cfg");
unlink($id . "_tserequest.log");

###################################
# subroutine to send traffic
###################################
sub sendTraffic
{
    my ($serverType, $testId, $controlServer, $testServer, $port,
        $maxDuration) = @_;
    my $logFile = $testId . "_tserequest.log";
    my $pltoolConfigFile = $testId . "_pltool.cfg";

    # setup test duration
    my $teststarttime = time();
    my $testendtime = $teststarttime + $maxDuration;
    my $testcurrenttime = time();

    while( $maxDuration == 0 || $testcurrenttime < $testendtime )
    {
        unlink($logFile);
        preparePLToolConfigFile( $serverType, $pltoolConfigFile, $logFile,
            $port, $controlServer, $testServer,
            $testcurrenttime - $teststarttime );
        my $server = $prdServers->getServerName( $serverType );
        print "Getting tserequest.log file from server [$server]\n";
        ($rc) = getRequestLog($serverType, $server, $logFile);
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
sub getNodeConfigsComparision
{
    my ($pathToScript, $node1, $node2, $port) = @_;

    my $cmd = "sh " . $pathToScript . "/CompareNodeConfigs.sh shopping " . $node1 . ":" . $port . " " . $node2 . ":" . $port;

    $result = `$cmd`;

    return $result;
}
################################################################################
# getRequestLog NODETYPE, SERVER
################################################################################
sub getRequestLog
{
    my ($nodeType, $server, $resultRequestFile) = @_;
    my $localLogFileName = $resultRequestFile . ".orig";
    my $socketLog = "tserequest.log";
    my $directory = "";
    my $error = 0;
    my $userid = "ATSESTAT";
   
    if ($nodeType == MIP_ALL || $nodeType == MIP || $nodeType == MIP_EXP ||
        $nodeType == MIP_TVLY || $nodeType == MIP_DPOOL ||
        $nodeType == MIP_GPOOL || $nodeType == MIP_HPOOL)
    {
        $directory = $mipDir . "/";
    }
    elsif ($nodeType == IS_ALL || $nodeType == IS || $nodeType == IS_EXP ||
           $nodeType == IS_TVLY || $nodeType == IS_DPOOL ||
           $nodeType == IS_GPOOL || $nodeType == IS_HPOOL)
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

    if( $filters eq "sol" ) {
         prepareSOLRequests( $localLogFileName, $resultRequestFile );
    }
    if( $filters eq "nosol" ) {
         prepareNoSOLRequests( $localLogFileName, $resultRequestFile );
    }
    elsif( $filters eq "intl" ) {
         prepareIntlRequests( $localLogFileName, $resultRequestFile );
    }
    elsif( $filters eq "no2ndwash" ) {
         prepareNo2ndWashRequests( $localLogFileName, $resultRequestFile );
    }
    elsif( $filters eq "tst" ) {
         prepareTSTRequests( $localLogFileName, $resultRequestFile );
    }
    elsif( $filters eq "nofamily" ) {
         prepareNoFamilyRequests( $localLogFileName, $resultRequestFile );
    }
    elsif( $filters eq "simple" ) {
         prepareSimpleRequests( $localLogFileName, $resultRequestFile );
    }
    else {
        prepareRequests( $localLogFileName, $resultRequestFile );
    }
    unlink( $localLogFileName );

    return($error);
}

sub prepareSOLRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $validreq = 0;
    my $legs = 0;
    my $hasdia = 0;
    my $hasdiv = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        $inreq = 1 if( $line =~ /^<ShoppingRequest / );
        #$line =~ s/<PRO /<DIV\/><PRO /;
        #$validreq = 1 if( $line =~ / C21="S[DI]SXX[0-9]/ );
        #$hasdia = 1 if( $line =~ /<DIA / );
        $hasdiv = 1 if( $line =~ /<DIV / );
        #$legs++ if( $line =~ /<LEG / );
        $reqbuf .= $line if( $inreq == 1 );
        if( $line =~ /<\/ShoppingRequest/ )
        {
            if( ($inreq == 1) && ($hasdiv == 1) )
            {
                $outputData .= $reqbuf;
            }
            $inreq = 0;
            $reqbuf = "";
            $validreq = 0;
            $legs = 0;
            $hasdiv = 0;
        }
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareNoSOLRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $validreq = 0;
    my $legs = 0;
    my $hasdia = 0;
    my $hasdiv = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        $inreq = 1 if( $line =~ /^<ShoppingRequest / );
        #$line =~ s/<PRO /<DIV\/><PRO /;
        #$validreq = 1 if( $line =~ / C21="S[DI]SXX[0-9]/ );
        #$hasdia = 1 if( $line =~ /<DIA / );
        $hasdiv = 1 if( $line =~ /<DIV / );
        #$legs++ if( $line =~ /<LEG / );
        $reqbuf .= $line if( $inreq == 1 );
        if( $line =~ /<\/ShoppingRequest/ )
        {
            if( ($inreq == 1) && ($hasdiv == 0) )
            {
                $outputData .= $reqbuf;
            }
            $inreq = 0;
            $reqbuf = "";
            $validreq = 0;
            $legs = 0;
            $hasdiv = 0;
        }
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareIntlRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $validreq = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        $inreq = 1 if( $line =~ /^<ShoppingRequest / );
        $validreq = 1 if( $line =~ / C21="SI/ );
        $reqbuf .= $line if( $inreq == 1 );
        if( $line =~ /<\/ShoppingRequest/ )
        {
            if( ($inreq == 1) && ($validreq == 1) )
            {
                $outputData .= $reqbuf;
            }
            $inreq = 0;
            $reqbuf = "";
            $validreq = 0;
        }
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareNo2ndWashRequests
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
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;
        $line =~ s/PCA="T"//;
        $line =~ s/PBM="T"//;

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareTSTRequests
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
        $line =~ s/<ShoppingRequest /<ShoppingRequest TST="T" /;
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareNoFamilyRequests
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
        $line =~ s/Q5Q="[0-9]+"//;
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub prepareSimpleRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $validreq = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        $inreq = 1 if( $line =~ /^<ShoppingRequest / );
        $validreq = 1 if( $line =~ / C21="S[DI]SXX[0-9]/ );
        $reqbuf .= $line if( $inreq == 1 );
        if( $line =~ /<\/ShoppingRequest/ )
        {
            if( ($inreq == 1) && ($validreq == 1) )
            {
                $outputData .= $reqbuf;
            }
            $inreq = 0;
            $reqbuf = "";
            $validreq = 0;
        }
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
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
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub preparePLToolConfigFile
{
    my ( $serverType, $pltoolConfigFile, $logFile, $port,
         $controlServer, $testServer, $processedTime ) = @_;
    my $templateConfigFile =
        $templateConfigDir . getTemplateConfigFile( $serverType );
    my $tps = &{$getTPSFunc}( $serverType, $processedTime );

    my $outputData = "";
    
    open INPUT, "<$templateConfigFile"
        or die "could not open config file [$templateConfigFile] for reading";

    while( my $line = <INPUT> )
    {
        $line =~ s/<REQUEST_LOG_FILE>/$logFile/;
        $line =~ s/<TPS>/$tps/;
        $line =~ s/<PORT>/$port/;
        $line =~ s/<CONTROL_SERVER>/$controlServer/;
        $line =~ s/<TEST_SERVER>/$testServer/;
        $line =~ s/<MAX_CONCURRENT_REQUESTS>/$maxConcurrentRequests/;
        
        $outputData .= $line;
    }
    close INPUT;

    #remove the output file if it exists
    unlink( $pltoolConfigFile );
    open OUTPUT, ">$pltoolConfigFile"
        or die "could not open '$pltoolConfigFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

sub getTPS_G2
{
    my ($nodeType, $processedTime) = @_;
    my $tps = "0.75";
    $tps = "1.50" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                      $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                      $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    if( $processedTime > 60*60 ) {
        $tps = "2.00";
        $tps = "3.50" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                          $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                          $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    }
    return( $tps );
}

sub getTPS
{
    my ($nodeType, $processedTime) = @_;
    my $tps = "0.50";
    $tps = "1.00" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                      $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                      $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    if( $processedTime > 60*45 ) {
        $tps = "1.00";
        $tps = "2.00" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                          $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                          $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    }
    if( $processedTime > 60*90 ) {
        $tps = "1.50";
        $tps = "4.00" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                          $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                          $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    }
    if( $processedTime > 60*180 ) {
        $tps = "2.00";
        $tps = "5.00" if ($nodeType == MIP_ALL || $nodeType == MIP ||
                          $nodeType == MIP_EXP || $nodeType == MIP_TVLY ||
                          $nodeType == MIP_DPOOL || $nodeType == MIP_GPOOL ||
                      $nodeType == MIP_HPOOL);
    }
    return( $tps );
}

sub getTemplateConfigFile
{
    my ($nodeType) = @_;
    my $templateConfigFile = "";

    if ($nodeType == MIP_ALL || $nodeType == MIP || $nodeType == MIP_EXP ||
        $nodeType == MIP_TVLY || $nodeType == MIP_DPOOL ||
        $nodeType == MIP_GPOOL || $nodeType == MIP_HPOOL)
    {
        $templateConfigFile = "mip.cfg";
    }
    elsif ($nodeType == IS_ALL || $nodeType == IS || $nodeType == IS_EXP ||
           $nodeType == IS_TVLY || $nodeType == IS_DPOOL ||
           $nodeType == IS_GPOOL || $nodeType == IS_HPOOL)
    {
        $templateConfigFile = "is.cfg";
    }

    return $templateConfigFile;
}

sub parseConfigFile
{
    my ($serverType, $configFile) = @_;
    my $title = "";
    my $id = "";
    my $controlServer = "";
    my $testServer = "";
    my $port = 53601;
    my $acPort = 5001;
    my $controlBaseline = "";
    my $testBaseline = "";
    my $maxDuration = 0;
    my $instrDbServer = "";
    my $testType = "adhoc";
    my $filters = "none";
    my $reportDirBase = $targetReportRootDir . $testType . "/";

    open INPUT, "<$configFile"
        or die "could not open config file [$configFile] for reading";

    while( my $line = <INPUT> )
    {
        if( $line =~ m/^ID=(.*?)$/ ) {
            $id = $1;
        }
        if( $line =~ m/^Title=(.*?)$/ ) {
            $title = $1;
        }
        if( $line =~ m/^ControlServer=(.*?)$/ ) {
            $controlServer = $1;
        }
        if( $line =~ m/^TestServer=(.*?)$/ ) {
            $testServer = $1;
        }
        if( $line =~ m/^Port=(.*?)$/ ) {
            $port = $1;
        }
        if( $line =~ m/^AppConsolePort=(.*?)$/ ) {
            $acPort = $1;
        }
        if( $line =~ m/^ControlBaseline=(.*?)$/ ) {
            $controlBaseline = $1;
        }
        if( $line =~ m/^TestBaseline=(.*?)$/ ) {
            $testBaseline = $1;
        }
        if( $line =~ m/^MaxDuration=(.*?)$/ ) {
            $maxDuration = $1 * 60;
        }
        if( $line =~ m/^InstrDbServer=(.*?)$/ ) {
            $instrDbServer = $1;
        }
        if( $line =~ m/^TestType=(.*?)$/ ) {
            $testType = $1;
            if( $testType eq "daily" or $testType eq "weekly") {
                $reportDirBase = $targetReportRootDir . "cpt/" . getLocalDate() . "/";
            }
        }
        if( $line =~ m/^Filters=(.*?)$/ ) {
            $filters = $1;
        }
    }

    if(    $id eq "" || $title eq ""
        || $controlServer eq "" || $testServer eq ""
        || $controlBaseline eq "" || $testBaseline eq ""
        || $instrDbServer eq "" )
    {
        printf STDERR "Insufficient parameters in the config file";
        printf STDERR ", paremeter values are ==> ";
        printf STDERR "id=[%s],title=[%s]", $id, $title;
        printf STDERR ",ControlServer=[%s]", $controlServer;
        printf STDERR ",TestServer=[%s]", $testServer;
        printf STDERR ",ControlBaseline=[%s]", $controlBaseline;
        printf STDERR ",TestBaseline=[%s]", $testBaseline;
        printf STDERR ",InstrDbServer=[%s]", $instrDbServer;
        printf STDERR "\n";
        exit;
    }
    
    close INPUT;

    return( $id, $title, $controlServer, $testServer, $port, $acPort,
            $controlBaseline, $testBaseline, $maxDuration, $instrDbServer, $reportDirBase, $testType, $filters );
}

sub getStats
{
    my($ctrl_host, $ctrl_port, $ctrl_statsFile,
       $test_host, $test_port, $test_statsFile,
       $maxDuration, $sleepSeconds) = @_;

    my $ctrl_totalTrxCount     = 0, $test_totalTrxCount     = 0;
    my $ctrl_totalErrCount     = 0, $test_totalErrCount     = 0;
    my $ctrl_totalCPU          = 0, $test_totalCPU          = 0;
    my $ctrl_totalElapsed      = 0, $test_totalElapsed      = 0;
    my $ctrl_virtualMemory     = 0, $test_virtualMemory     = 0;
    my $ctrl_resMemory         = 0, $test_resMemory         = 0;
    my $ctrl_totalDBCount      = 0, $test_totalDBCount      = 0;
    my $ctrl_totalDBElapsed    = 0, $test_totalDBElapsed    = 0;
    my $ctrl_totalItins        = 0, $test_totalItins        = 0;
    my $ctrl_totalResponseSize = 0, $test_totalResponseSize = 0;
    my ($ctrl_dbHost, $test_dbHost);

    #print header to stats files
    printStatsHeader($ctrl_statsFile);
    printStatsHeader($test_statsFile);

    my $acCommands = new AppConsoleCommands;

    my $duration = 0; #counter to increment the duration
    while($maxDuration == 0 || $duration <= $maxDuration)
    {
        my ($ctrl_trxCount, $ctrl_cpu, $ctrl_elapsed, $ctrl_memory,
            $ctrl_dbCount, $ctrl_dbElapsed, $ctrl_dbHost, $ctrl_itins,
            $ctrl_responseSize, $ctrl_errCount, $ctrl_rmemory )
            = $acCommands->getStats($ctrl_host, $ctrl_port);
        my ($test_trxCount, $test_cpu, $test_elapsed, $test_memory,
            $test_dbCount, $test_dbElapsed, $test_dbHost, $test_itins,
            $test_responseSize, $test_errCount, $test_rmemory )
            = $acCommands->getStats($test_host, $test_port);
        if($ctrl_trxCount > $ctrl_totalTrxCount)
        {
            calculateAndPrintStatsData(
                $ctrl_totalTrxCount, $ctrl_totalErrCount, $ctrl_totalCPU,
                $ctrl_totalElapsed, $ctrl_virtualMemory, $ctrl_totalDBCount,
                $ctrl_totalDBElapsed, $ctrl_totalItins, $ctrl_totalResponseSize,
                $ctrl_resMemory,
                $ctrl_trxCount, $ctrl_errCount, $ctrl_cpu,
                $ctrl_elapsed, $ctrl_memory, $ctrl_dbCount,
                $ctrl_dbElapsed, $ctrl_itins, $ctrl_responseSize,
                $ctrl_rmemory,
                $ctrl_statsFile );
            calculateAndPrintStatsData(
                $test_totalTrxCount, $test_totalErrCount, $test_totalCPU,
                $test_totalElapsed, $test_virtualMemory, $test_totalDBCount,
                $test_totalDBElapsed, $test_totalItins, $test_totalResponseSize,
                $test_resMemory,
                $test_trxCount, $test_errCount, $test_cpu,
                $test_elapsed, $test_memory, $test_dbCount,
                $test_dbElapsed, $test_itins, $test_responseSize,
                $test_rmemory,
                $test_statsFile );

            # set current sample counters to the global variables
            $ctrl_totalTrxCount = $ctrl_trxCount;
            $ctrl_totalErrCount = $ctrl_errCount;
            $ctrl_totalCPU = $ctrl_cpu;
            $ctrl_totalElapsed = $ctrl_elapsed;
            $ctrl_totalDBCount = $ctrl_dbCount;
            $ctrl_totalDBElapsed = $ctrl_dbElapsed;
            $ctrl_virtualMemory = $ctrl_memory;
            $ctrl_resMemory = $ctrl_rmemory;
            $ctrl_totalResponseSize = $ctrl_responseSize;
            $ctrl_totalItins = $ctrl_itins;

            $test_totalTrxCount = $test_trxCount;
            $test_totalErrCount = $test_errCount;
            $test_totalCPU = $test_cpu;
            $test_totalElapsed = $test_elapsed;
            $test_totalDBCount = $test_dbCount;
            $test_totalDBElapsed = $test_dbElapsed;
            $test_virtualMemory = $test_memory;
            $test_resMemory = $test_rmemory;
            $test_totalResponseSize = $test_responseSize;
            $test_totalItins = $test_itins;
        }
        sleep($sleepSeconds);
        $duration += $sleepSeconds;
    }
}

sub calculateAndPrintStatsData
{
    my ($totalTrxCount, $totalErrCount, $totalCPU,
        $totalElapsed, $virtualMemory, $totalDBCount,
        $totalDBElapsed, $totalItins, $totalResponseSize, $resMemory,
        $trxCount, $errCount, $cpu,
        $elapsed, $memory, $dbCount,
        $dbElapsed, $itins, $responseSize, $rmemory,
        $statsFile ) = @_;

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
            
        if($trxCountLS gt 0)
        {
            my $avgCpuLS = $cpuLS/$trxCountLS;
            my $avgElapsedLS = $elapsedLS/$trxCountLS;
            my $avgDbElapsedLS = $dbElapsedLS/$trxCountLS;
            my $avgResponseSizeLS = $responseSizeLS/$trxCountLS;
            my $avgItinsLS = $itinsLS/$trxCountLS;

            my $avgCpu = $cpu/$trxCount;
            my $avgElapsed = $elapsed/$trxCount;
            my $avgDbCount = $dbCount/$trxCount;
            my $avgDbElapsed = $dbElapsed/$trxCount;
            my $avgItins = $itins/$trxCount;

            printStatsData($trxCount, $avgCpu, $avgElapsed,
                  $avgDbCount, $avgDbElapsed, $avgItins,
                  $trxCountLS, $avgCpuLS, $avgElapsedLS, $avgDbElapsedLS,
                  $dbCountLS, $avgResponseSizeLS, $avgItinsLS, $memory,
                  $rmemory, $statsFile);
        }
    }
}

# Prints Column Headers
sub printStatsHeader
{
    my($statsFile) = @_;
    my $fh = new FileHandle;
    $fh->open(">$statsFile")
        or die "could not open '$statsFile' for writing";
    print $fh "Time";                             #1st Column
    print $fh ",Total Trx";                       #2nd
    print $fh ",Avg CPU";                         #3rd
    print $fh ",Avg Elapsed";                     #4th
    print $fh ",Avg DB Read Count";               #5th
    print $fh ",Avg DB Elapsed";                  #6th
    print $fh ",Avg Itins";                       #7th
    print $fh ",Virtual Memory";                  #8th
    print $fh ",Resident Memory";                 #9th
    print $fh ",Last Sample Trx";                 #10th
    print $fh ",Last Sample Avg CPU";             #11th
    print $fh ",Last Sample Avg Elapsed";         #12th
    print $fh ",Last Sample Avg Response Size";   #13th
    print $fh ",Last Sample Avg Itins";           #14th
    print $fh "\n";
    $fh->close();
}

# Prints the data
sub printStatsData
{
    my ($totalTrxCount, $avgCpu, $avgElapsed,
        $avgDbCount, $avgDbElapsed, $avgItins,
        $trxCountLS, $avgCpuLS, $avgElapsedLS, $avgDbElapsedLS,
        $dbCountLS, $avgResponseSizeLS, $avgItinsLS, $virtualMemory, $resMemory,
        $statsFile) = @_;
    my $localTime = getLocalTime();
    my $gb = 1024 * 1024 * 1024;
    my $vmemoryGB = $virtualMemory / $gb;
    my $rmemoryGB = $resMemory / $gb;

    my $fh = new FileHandle;
    $fh->open(">>$statsFile")
        or die "could not open '$statsFile' for writing";

    print $fh $localTime;
    print $fh "," . $totalTrxCount;
    print $fh "," . $avgCpu;
    print $fh "," . $avgElapsed;
    print $fh "," . $avgDbCount;
    print $fh "," . $avgDbElapsed;
    print $fh "," . $avgItins;
    print $fh "," . $vmemoryGB;
    print $fh "," . $rmemoryGB;
    print $fh "," . $trxCountLS;
    print $fh "," . $avgCpuLS;
    print $fh "," . $avgElapsedLS;
    print $fh "," . $avgResponseSizeLS;
    print $fh "," . $avgItinsLS;
    print $fh "\n";
    $fh->close();
}

sub generateReport
{
    my ($title, $testDate, $reportDir, $reportFile,
        $controlServer, $controlBaseline, $controlDatabase, $controlStatsFile,
        $testServer, $testBaseline, $testDatabase, $testStatsFile,
        $startTime, $instrDbServer, $testType, $maxDuration, $sleepSeconds,
        $acPort, $configsComparision) = @_;
  
    my $reportGen = new ReportGen;
    $reportGen->initErrorDescriptions($errorDescriptionsFile);

    unlink( $reportDir );
    system("rm -fr $reportDir");
    system("mkdir -p $reportDir");
    #mkdir( $reportDir );

    my $duration = 0; #counter to increment the duration
    while($maxDuration == 0 || $duration <= $maxDuration)
    {
        sleep($sleepSeconds);

        my $endTime = getLocalTime();
        $reportGen->genReportHTML( $title, $testDate, $reportDir, $reportFile,
          $controlServer, $controlBaseline, $controlDatabase, $controlStatsFile,
          $testServer, $testBaseline, $testDatabase, $testStatsFile,
          $startTime, $endTime, $instrDbServer, $testType, $acPort, $configsComparision );

        $duration += $sleepSeconds;
    }
}

sub getAppDBInfo
{
    my ($host, $port) = @_;
    
    my $acCommands = new AppConsoleCommands;
    return $acCommands->getDBInfo($host, $port);
}

sub getLocalTime
{
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

   $year += 1900;
   $mon  += 1;

   my $localTime = sprintf("%4d-%02d-%02d %02d:%02d:%02d",
                           $year,$mon,$mday,$hour,$min,$sec);
   return($localTime);
}

sub getLocalDate
{
   my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

   $year += 1900;
   $mon  += 1;

   my $localDate = sprintf("%4d-%02d-%02d", $year,$mon,$mday);
   return($localDate);
}

