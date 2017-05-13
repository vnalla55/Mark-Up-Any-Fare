#!/usr/bin/perl -s
# scripts takes on standard input log containing tax requests,
# sends request to server given in command line parameter, 
# checks if response contains given tax code, if yes, writes this response to file filtered.xml 
use DBI;

#print "usage: [perl -s] taxMillUltra.pl -instrdb=instrdb_host:port -h1=srv_host:port -h2=srv_host2:port2 -taxcode=taxcode -offset=offset -day=day_of_month -m=filter -s=replace\n";

$h1 or die "Please provide hostname and port.\n";
$taxcode or die "Please provide tax code.\n";
$day or die "Please provide day.\n";
$instrdb or die "Please provie instrumentation database.\n";
($ins_host, $ins_port) = split(":", $instrdb);
($host, $port) = split(":", $h1);

if ($h2) {
  ($host2, $port2) = split(":", $h2);
}
if (not $offset) {
  $offset = 0;
}
if ($s) {
  ($s1, $s2) = split(":", $s);
}

$hammerpath = "/vobs/atseintl/ClientSocket/test";
$i = 1;
$curroffset = $offset;
$prefix = $taxcode . "_$day" . "_$offset";

sub sendRequest {

  my ($host, $port) = @_;
  
 sendr: 
  my $response = `perl -I$hammerpath $hammerpath/hammer.pl -h $host -p $port --print temp.xml 2>err.tmp`;
  if (not ($response =~ "<AirTaxRS")) {
    print "no response from $host:$port\n";
    print `cat err.tmp`;
    sleep (15);
    goto sendr;
  }
  my @taxAmount = ($response =~  m/Message="\s*\d+\s+($taxcode.*?)"/g);
 
  foreach(@taxAmount) {
    $_ =~ m/($taxcode)\s+\S+\s+(\S+)\s+\S+\s+\S+\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/;
    $_ = "$1 $2 $3 $4 $5 $6";
  }
  @taxAmount = sort (@taxAmount);
  #print $response . "\n\n\n\n"; 
  return @taxAmount;
}


open BAD_OUT, ">", $prefix . "bad.txt";
open GOOD_OUT, ">", $prefix . "good.txt";
open FILT, ">", $prefix . "request.xml";

$dbh = DBI->connect("DBI:mysql:ATSEHYB2;host=$ins_host;port=$ins_port", 'hybfunc') || die "Could not connect to database: $DBI::errstr";
$sth = $dbh->prepare("SELECT INPUTCMD FROM XMLREQUEST$day WHERE SERVICENAME = 'INTLWPI1' AND (INPUTCMD LIKE '<PricingRequest%' OR INPUTCMD LIKE '<AirTaxRQ%')", { "mysql_use_result" => 1} );
$sth->execute();
while ($request = $sth->fetchrow()) {

  if ($m and not ($request =~ $m)) {
    print "-";
    next;
  }
  
  if ($request =~ m/(<PricingRequest.*?<\/PricingRequest>)/) {

    open  XML, ">", "temp.xml";
    print XML $1;
    close XML;

    $request = `xsltproc PricingRequestToAirTaxRQ.xsl temp.xml`;
  }
  
  if ($request =~ m/(<AirTaxRQ.*?<\/AirTaxRQ>)/) {
  
    $request = $1;
    $output = "";
    $istaxcode = 0;
    $good = 0;

    if ($s) {
     $request =~ s/$s1/$s2/g;
    }

    $request =~ s/<POS/<Diagnostic><RequestedDiagnostic Number="817"><\/RequestedDiagnostic><\/Diagnostic><POS/;
    
    open  XML, ">", "temp.xml";
    print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print XML $request;
    close XML;

    $output .= "\n\n--[$i]--\n";
    if ($request =~ m/<Source PseudoCityCode="(.*?)"/) {
      $output .= "PCC:$1";
    }
    if ($request =~ m/<Partition ID="(.*?)"/) {
      $output .= "partition:$1";
    }
    if ($request =~ m/<AAACity Code="(.*?)"/) {
      $output .= "AAACity:$1";    
    }  
    if ($request =~ m/<AgentSine Code="(.*?)"/) {
      $output .= "AgentSine:$1";
    }
    $output .= "\n";
    
    @items = ($request =~ m/<Item.*?<\/Item>/g);
    
    foreach(@items) {
    
      $output .= "\n";
      
      if (m/SalePseudoCityCode="(.*?)"/) {
        $output .= "SalePseudoCity:$1\n";
      }

      if (m/PassengerType Quantity="(.*?)" *Code="(.*?)"/) {
	$output .= "PAX: $1 $2\n";
      }
      
      @segments = ($request =~ m/<FlightSegment.*?<\/FlightSegment>/g);
      
      foreach(@segments) {
      
        $carrier = "--";
	if (m/MarketingAirline Code="(.*?)"/) {
          $carrier = $1;
	}
	$bookcode = "-";
	if (m/ ResBookDesigCode="(.*?)"/) {
          $bookcode = $1;
	}
        $deptTme = "-";
        if (m/DepartureDateTime="(.*?)"/) {
          $deptTme = $1;
          $deptTme =~ s/T/ /;
	}
        $arrvlDte = "-";
        if (m/ArrivalDateTime="(.*?)"/) {
          $arrvlDte = $1;
          $arrvlDte =~ s/T/ /;
	}
        $deptPort = "-";
        if (m/<DepartureAirport.*?LocationCode="(.*?)"\/>/) {
          $deptPort = $1;
        }
	$arrvlPort = "-";
        if (m/<ArrivalAirport.*?LocationCode="(.*?)"\/>/) {
        $arrvlPort = $1;
	}
      
        $output .=  "$carrier $bookcode $deptPort $arrvlPort $deptTme $arrvlDte\n";
      }
    }
    
    $output .= "\n$h1:\n";
    @taxAmount = sendRequest($host, $port);
    foreach(@taxAmount) {
      $istaxcode = 1;
      $output .= "$_\n";
    }

    if ($h2) {
      $output .=  "\n$h2:\n";
      @taxAmount2 = sendRequest($host2, $port2);
      
      $numbermatch = 0;
      if (scalar(@taxAmount) == scalar(@taxAmount2)) {
        $numbermatch = 1;
      }
     
      $ind = 0;
      foreach (@taxAmount2) {
        $istaxcode = 1;
	$output .= "$_\n";
        if ($_ ne $taxAmount[$ind]) {
	  $good = 0;
	}
	else {
	  if ($numbermatch) {
            $good = 1;
	  }
	}
	$ind++;
      }
    }
    else {
      $good = 1;
    }
    
    if ($istaxcode) {    
      if ($good == 0) {
        print "<$i>";
	print BAD_OUT $output;
      }
      else {
        print "[$i]";
        print GOOD_OUT $output; 
      }
      print FILT "[$i]\n";
      print FILT "$request\n";
      $i++;
    }
    else {
      print ".";
    }
  }
}

close BAD_OUT;
close GOOD_OUT;
close FILT;


