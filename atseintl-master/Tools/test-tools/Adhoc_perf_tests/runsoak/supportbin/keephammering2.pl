#!/usr/bin/perl

use IO::Socket;
use Net::SCP qw(scp iscp);
scp($source, $destination);
use warnings;

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

# setup test duration
my $teststarttime = time();
my $testendtime = $teststarttime + $duration;
my $testcurrenttime = time();

while( $duration == 0 || $testcurrenttime < $testendtime )
{
   my $server = getServerName( $serverType );
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
# getServerName SERVERTYPE
#
#   Returns a node id for the specified serverType. serverType can be one of
#   the following.
#
#   0 => IS ALL
#   1 => MIP ALL
#   2 => IS General/Common Pool
#   3 => MIP General/Common Pool
#   4 => IS Expedia 200 Options
#   5 => MIP Expedia 200 Options
#   6 => IS Travelocity 200 Options
#   7 => MIP Travelocity 200 Options
#   8 => IS D Pool (EMEA)          
#   9 => MIP D Pool (EMEA)          
#
################################################################################
sub getServerName
{
    my @mip    =
    (
    "piclp412","piclp413","piclp414","piclp415","piclp416",
    "piclp424","piclp425","piclp426","piclp427","piclp428",
    "piclp429","piclp430","piclp431","piclp432","piclp433",
    "piclp437","piclp438","piclp439","piclp440","piclp441",
    "piclp442",
    "piclp479","piclp480","piclp481","piclp482","piclp483",
    "piclp484","piclp485","piclp486","piclp487","piclp488",
    "piclp489","piclp490","piclp491","piclp492","piclp493",
    "piclp494","piclp495","piclp509",
    "piclp570","piclp571","piclp572","piclp573",
    "piclp574","piclp575","piclp576","piclp577","piclp578",
    "piclp579",
    "piclp600","piclp601","piclp602","piclp603",
    "piclp624","piclp625","piclp626","piclp627","piclp628",
    "piclp629","piclp630","piclp631","piclp632","piclp633",
    "piclp654","piclp655","piclp656","piclp657","piclp658",
    "piclp659",
    "piclp685","piclp686","piclp687","piclp688",
    "piclp689","piclp690","piclp691","piclp692","piclp693",
    "piclp694","piclp695","piclp696",
    "piclp733","piclp734","piclp735","piclp736",
    "piclp738","piclp739","piclp740","piclp741","piclp742",
    "piclp743","piclp744","piclp745","piclp746",
    "pimlp400","pimlp401","pimlp402","pimlp403","pimlp404",
    "pimlp405",
    "pimlp816","pimlp817","pimlp818","pimlp819","pimlp820",
    "pimlp821","pimlp822","pimlp823","pimlp824","pimlp825",
    "pimlp826","pimlp827","pimlp828","pimlp829","pimlp830",
    "pimlp831","pimlp832","pimlp833","pimlp834"
    );

    my @mipExp200 =
    (
    "piclp417","piclp418","piclp419","piclp420","piclp421",
    "piclp422","piclp423",
    "piclp434","piclp435","piclp436",
    "piclp443","piclp444","piclp445","piclp446","piclp447",
    "piclp448","piclp449",
    "piclp450","piclp451","piclp452","piclp453","piclp454",
    "piclp455","piclp456","piclp457","piclp458","piclp459",
    "piclp460","piclp461","piclp462","piclp463","piclp464",
    "piclp465","piclp466","piclp467","piclp468","piclp469",
    "piclp470","piclp471","piclp472","piclp473","piclp474",
    "piclp475","piclp476","piclp477","piclp478"
    );

    my @mipTvly200 =
    (
    "piclp514","piclp515","piclp516","piclp517","piclp518",
    "piclp519","piclp520","piclp521","piclp522","piclp523",
    "piclp527","piclp528",
    "piclp529","piclp530","piclp531","piclp532","piclp533",
    "piclp534","piclp535","piclp536",
    "piclp585","piclp586",
    "piclp590","piclp591","piclp592","piclp593","piclp594",
    "piclp595","piclp596","piclp597","piclp598","piclp599",
    "piclp634","piclp635","piclp636","piclp637","piclp638",
    "piclp639","piclp640","piclp641","piclp642","piclp643",
    "piclp644","piclp645","piclp646","piclp647","piclp648",
    "piclp649","piclp650","piclp651","piclp652","piclp653",
    "piclp660","piclp661","piclp662","piclp663","piclp664",
    "piclp665","piclp666","piclp667","piclp668","piclp669",
    "piclp670","piclp671","piclp672","piclp673","piclp674",
    "piclp675","piclp676","piclp677","piclp678","piclp679",
    "piclp680","piclp681",
    "piclp687","piclp699","piclp700",
    "piclp701","piclp702","piclp703","piclp704","piclp705",
    "piclp706","piclp707","piclp708","piclp709","piclp710",
    "piclp711","piclp712","piclp713"
    );

    my @mipDPool =
    (
    "pimlp803","pimlp804","pimlp805","pimlp806","pimlp807",
    "pimlp808","pimlp809","pimlp810","pimlp811","pimlp812",
    "pimlp813","pimlp814","pimlp815",
    "pimlp835","pimlp836","pimlp837","pimlp838","pimlp839",
    "pimlp840","pimlp841","pimlp842","pimlp843","pimlp844",
    "pimlp845","pimlp846","pimlp847","pimlp848","pimlp849",
    "pimlp850","pimlp851","pimlp852","pimlp853"
    );

    my @is     =
    (
    "piclp206","piclp207","piclp208",
    "piclp417",
    "piclp438","piclp439","piclp440","piclp441","piclp442",
    "piclp496","piclp497","piclp498","piclp499","piclp500",
    "piclp501",
    "pimlp010","pimlp011","pimlp012",
    "pimlp035","pimlp036","pimlp037","pimlp059"
    );

    my @isExp200 =
    (
    "piclp400","piclp401","piclp402","piclp403","piclp404",
    "piclp405"
    );

    my @isTvly200 =
    (
    "piclp716","piclp717","piclp718","piclp719","piclp720",
    "piclp721","piclp722","piclp723","piclp724","piclp725",
    "piclp247"
    );

    my @isDPool =
    (
    "piclp209","piclp210","piclp211","piclp212",
    "piclp406","piclp407","piclp410","piclp411",
    "piclp556","piclp589","piclp682","piclp683","piclp684"
    );

    my ( $nodeType ) = @_;
    my $size;
    my @serverList;

    if ($nodeType == IS)
    {
        @serverList = @is;
    }
    elsif ($nodeType == MIP)
    {
        @serverList = @mip;
    }
    elsif ($nodeType == IS_EXP)
    {
        @serverList = @isExp200;
    }
    elsif ($nodeType == MIP_EXP)
    {
        @serverList = @mipExp200;
    }
    elsif ($nodeType == IS_TVLY)
    {
        @serverList = @isTvly200;
    }
    elsif ($nodeType == MIP_TVLY)
    {
        @serverList = @mipTvly200;
    }
    elsif ($nodeType == IS_DPOOL)
    {
        @serverList = @isDPool;  
    }
    elsif ($nodeType == MIP_DPOOL)
    {
        @serverList = @mipDPool;
    }
    elsif($nodeType == IS_ALL)
    {
        push(@serverList, @is);
        push(@serverList, @isExp200);
        push(@serverList, @isTvly200);
        push(@serverList, @isDPool);
    }
    elsif($nodeType == MIP_ALL)
    {
        push(@serverList, @mip);
        push(@serverList, @mipExp200);
        push(@serverList, @mipTvly200);
        push(@serverList, @mipDPool);
    }
    $size = @serverList;

    my $serverIndex = int(rand($size));
    return $serverList[$serverIndex];
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
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;

        $outputData .= $line;
    }

    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}

