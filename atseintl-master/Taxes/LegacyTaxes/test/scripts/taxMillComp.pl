#!/usr/bin/perl -s
# scripts takes on standard input log containing tax requests,
# sends request to two servers given in command line parameter, 
# if no -e or -t parameters are specified compares tax amounts of all codes in two responses,
# if -t parameter is specified compares only one taxcode
# if -e parameter is specified compares all except given taxcode
# generartes error message if amounts are different
# writes requests which caused errors to error.xml file

print "usage:  -a=host1:port1 -b=host2:port2 -t=only_taxcode -e=except_taxcode\n";
($a and $b) or die "Please provide hostname and port.\n";
($t and $e) and die "-t and -e cannot be used together.\n";
($host1, $port1) = split(":", $a);
($host2, $port2) = split(":", $b);
print "host1:$host1 port1:$port1\n";
print "host2:$host2 port2:$port2\n";
$except_taxcode = $e;
$one_taxcode = $t;

$request_tag="<AirTaxRQ";
$hammerpath="/vobs/atseintl/ClientSocket/test";
$one_taxcodetag="<Tax TaxCode=\"$one_taxcode\" Amount=\"(.*?)\".*?>";
$taxcodetag="<Tax TaxCode=\".*?\" Amount=\".*?\".*?>";
$taxcodetagx="<Tax TaxCode=\"(.*?)\" Amount=\"(.*?)\".*?>";
$taxtag="<AirTaxRS";
$i=1;

@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";
open  ERRX, ">", "error.xml";

foreach (<STDIN>) {

  if ($_ =~ $request_tag)
  {
    s/.*$request_tag/$request_tag/;
    open  XML, ">", "temp.xml";
    print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print XML $_;
    close XML;
    print "--[$i]--\n";
    $request = $_;
    s/<FlightSegment/\n<FlightSegment/g;
    print $_;
    print "\n---------------------------------------------------------------------\n";
    $response1 = `perl -I$hammerpath $hammerpath/hammer.pl -h $host1 -p $port1 --print temp.xml`;
    print $response1;
    if($response1 !~ $taxtag)
    {
      print "ERROR - no response from server\n";
    }
    print "\n---------------------------------------------------------------------\n";
    $response2 = `perl -I$hammerpath $hammerpath/hammer.pl -h $host2 -p $port2 --print temp.xml`;
    print $response2;
    if($response2 !~ $taxtag)
    {
      print "ERROR - no response from server\n";
    }
    if($one_taxcode)
    {
      $match1 = ($response1 =~ $one_taxcodetag);
      $amount1 = $1;
      $match2 = ($response2 =~ $one_taxcodetag);
      $amount2 = $1;
      if(($match1 != $match2) || ($amount1 != $amount2))
      {
        print "ERROR - $one_taxcode amounts don't match (only $one_taxcode checked)\n";
	print ERRX $request;
      }
    }
    else
    {
      @taxcodes1 = ($response1 =~ m/$taxcodetag/g);
      @taxcodes2 = ($response2 =~ m/$taxcodetag/g);
      if(scalar(@taxcodes) != scalar(@taxcodes))
      {
        print "ERROR - number of returned tax codes don't match\n";
	print ERRX $request;
      }
      foreach(@taxcodes1)
      {
        $_ =~ $taxcodetagx;
        $tax_hash1{$1} = $2;
      }
      foreach(@taxcodes2)
      {
        $_ =~ $taxcodetagx;
        if($tax_hash1{$1} != $2) 
        {
	  if($except_taxcode ne $1)
	  {
            print "ERROR - $1 amounts don't match";
	    print ERRX $request;
	  }
        }
      }
    }
    print "\n=====================================================================\n";
    $i++;
  }
}

close ERRX;
@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";


