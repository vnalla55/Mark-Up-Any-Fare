#!/usr/bin/perl -s
use DBI;
use XML::SemanticDiff;
use threads;
use Thread::Semaphore;

print "usage of this tool is documented here:\nhttp://wiki.sabre.com/confluence/display/AirServices/Differential+testing\n";

$h1 or die "Please provide hostname and port.\n";
$h2 or die "Please provide hostname and port.\n";
$day or $infile or die "Please provide day.\n";
$instrdb or $infile or die "Please provide instrumentation database hostname and port or input file with xml requests (-instrdb, -infile).\n";
$reqtag or die "Please provide request tag.\n";
$restag or die "Please provide response tag.\n";
($host, $port) = split(":", $h1);
($host2, $port2) = split(":", $h2);
$s and ($s1, $s2) = split(":", $s);
$requestTag = $reqtag;
$responseTag = $restag;
if($^O eq "MSWin32") {
  $hammer = "hammer.pl";
  $dbiString = "DBI:ODBC:$instrdb";
}
else {
  $hammerpath = "/vobs/atseintl/ClientSocket/test";
  $hammer = "-I$hammerpath $hammerpath/hammer.pl";
  ($ins_host, $ins_port) = split(":", $instrdb);
  if(($ins_host =~ "pinhpp75") or ($ins_host =~ "pinhpp76")) {
    die "Don't use production Instrumentation Database.\n";
  }
  $dbiString = "DBI:mysql:ATSEHYB2;host=$ins_host;port=$ins_port"; 
}
$prefix or $prefix="x";
$prefix .= $day;
$limit or $limit = 100000;
$offset or $offset = 0;
$servicename or $servicename = "INTLWPI1";
$timeout or $timeout = 30;

my $reqlasti : shared = 0;
my @requestStore : shared = ();
my $response1 : shared = "";
my $semaphore = new Thread::Semaphore; 
$SIG{ALRM} = sub { };

sub sendRequest {

  my ($host, $port, $trd) = @_;
  
 sendr: 
  my $response = "";
  eval {
    local $SIG{ALRM} = sub { die "alarm\n" };
    alarm $timeout;
    $response = `perl $hammer -h $host -p $port --print $prefix-temp.xml 2>$prefix-err-$trd.tmp`;
    alarm 0;
  };
  if($@ eq "alarm\n") {
    return "<a>timeout $timeout s <host>$host</host><port>$port</port></a>";
  }
  if (not ($response =~ $responseTag)) {
    print "no response from $host:$port\n";
    open ERRM, "<", "$prefix-err-$trd.tmp";
    while (<ERRM>) {
      print;
    }
    close ERRM;
    sleep (5);
    goto sendr;
  }
  return $response;
}

sub sendRequestThread {

  my ($host, $port, $trd) = @_;
  $response1 = sendRequest($host, $port, $trd);
}

sub readRequests {

  my $request = "";
  $sqlcommand = "SELECT INPUTCMD FROM XMLREQUEST$day WHERE SERVICENAME = '$servicename' AND INPUTCMD LIKE '<" . $requestTag . "%'" . " LIMIT $limit OFFSET $offset";
  $dbh = DBI->connect($dbiString, 'hybfunc') || die "Could not connect to database: $DBI::errstr";
  $dbh->{LongReadLen} = 64000;
  $sth = $dbh->prepare($sqlcommand, {"mysql_use_result" => 1});
  $sth->execute();
  while ($request = $sth->fetchrow() and $reqlasti < $limit) {
    
    if ($transform and $request =~ m/(<PricingRequest.*?<\/PricingRequest>)/) {
      open  XML, ">", "$prefix-temp1.xml";
      print XML $request;
      close XML;
      
      if($^O eq "MSWin32") {
        $request = `msxsl.exe $prefix-temp1.xml PricingRequestToAirTaxRQ.xsl`;
      }
      else {
        $request = `xsltproc PricingRequestToAirTaxRQ.xsl $prefix-temp1.xml`;
      }
    }
    
    $semaphore->down; 
    if (not $m or ($request =~ $m)) {
      push @requestStore, $request;
    }
    $reqlasti++;
    $semaphore->up;
    #sleep(1);
  }
}

$xmldiff = XML::SemanticDiff->new();

if (not $infile) {
  $thr = threads->new(\&readRequests);
  $thr->detach;
}
else {
  open INFILE, "<", $infile;
}

if ($saverequest) {
  open REQU, ">", $prefix . "_requ.txt";
}

if ($savebadreq) {
  open BREQU, ">", $prefix . "_bad_requ.txt";
}

open BAD_OUT, ">", $prefix . "_bad.txt";
if($savegood) {
  open GOOD_OUT, ">", $prefix . "_good.txt";
}

$reqpos = 0;

if ($infile)
{
  while ($offset and $request = <INFILE>)
  {
    $offset--;
  }
}

while (((not $infile) and ($reqpos < $limit)) or ($infile and ($request = <INFILE>))) {

  if (not $infile) {
    $request = 0; 
    while(not $request) {
      $semaphore->down;
      if($reqlasti > $reqpos) {
        $request = $requestStore[$reqpos];
        $reqpos++;
      }
      $semaphore->up;
    }
  }
  else {
    $reqpos++;
  }
  
  $output = "";
  
  
  if ($s) {
    $request =~ s/$s1/$s2/g;
  }

  if($infile) {
    if($request =~ m/(<$requestTag.*)/) {
      $request = $1;
    }
    else {
      next;
    }
  }
  
  open  XML, ">", "$prefix-temp.xml";
  print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  print XML $request;
  close XML;

  if($saverequest) {
    print REQU "$request\n";
  }
  
  $output .= "\n\n--[$reqpos]--\n";
    
  $thr1 = threads->new(\&sendRequestThread, $host, $port, "tr1");
  $response2 = sendRequest($host2, $port2, "tr2");
  $thr1->join;
   
  $diffs = "";
  foreach my $change ($xmldiff->compare($response1, $response2)) {
    if($include) {
      if ($change->{message} =~ "^Attribute '$include'") {
        $diffs .= "$change->{message} in context $change->{context}\n";
      }
    }
    else {
      if((not $exclude) or ($change->{message} !~ "^Attribute '$exclude'")) {
        $diffs .= "$change->{message} in context $change->{context}\n";
      }
    }
  }
  $output .= $diffs;
  $output .= "\n$request\n\n$h1:\n$response1\n$h2:\n$response2\n";
      
  if($diffs ne "") {
    print "<$reqpos>";
    print BAD_OUT $output;
    if($savebadreq) {
      print BREQU "$request\n";
    }
  }
  else {
    print "[$reqpos]";
    if($savegood) {
      print GOOD_OUT $output; 
    }
  }
}
close BAD_OUT;
if($savegood) {
  close GOOD_OUT;
}
if($infile) {
  close INFILE;
}
if ($saverequest) {
  close REQU;
}
if ($savebadreq) {
  close BREQU;
}

