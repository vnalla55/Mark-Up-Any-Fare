#!/usr/bin/perl -s
# scripts takes on standard input log containing tax requests,
# sends request to server given in command line parameter, 
# checks if response contains given tax code, if yes, writes this response to file filtered.xml 

print "usage: [perl -s] taxMill.pl -h=srv_host:port -t=taxcode\n";
$h or die "Please provide hostname and port.\n";
$t or die "Please provide tax code.\n";
($host, $port) = split(":", $h);
print "host:$host port:$port\n";
$taxcode=$t; 

$requesttag="<AirTaxRQ";
$hammerpath="/vobs/atseintl/ClientSocket/test";
$taxcodetag="TaxCode=\"$taxcode\"";
$taxcodecount=0;
$i=1;

open  F_XML, ">", "filtered.xml";

@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";

foreach (<STDIN>) {

  if ($_ =~ $requesttag)
  {
    s/.*$requesttag/$requesttag/;
    $request = $_;
    open  XML, ">", "temp.xml";
    print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print XML $request;
    close XML;
    print "--[$i]--\n";
    print $request;
    print "\n---------------------------------------------------------------------\n";
    $response =  `perl -I$hammerpath $hammerpath/hammer.pl -h $host -p $port --print temp.xml`;
    print $response;
    print "\n=====================================================================\n";
    $i++;
    
    if ($response =~ $taxcodetag)
    {
      $taxcodecount++;
      print F_XML "$request\n";
    }
  }
}

@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";
print "tax $taxcode aplied $taxcodecount times.\n";

close F_XML;

