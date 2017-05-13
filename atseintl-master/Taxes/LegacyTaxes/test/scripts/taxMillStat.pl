#!/usr/bin/perl -s
# scripts takes on standard input log containing tax requests, 
# sends each request to server given in command line parameter 
# and counts how many times each taxcode uccured in responses 

print "usage: [perl -s] taxMillStat.pl -h=srv_host:port\n";
$h or die "Please provide hostname and port.\n";
($host, $port) = split(":", $h);
print "host:$host port:$port\n";

$request="<AirTaxRQ";
$hammerpath="/vobs/atseintl/ClientSocket/test";
$taxcodetag="TaxCode=\".*?\"";
$taxcodecount=0;
$i=1;
%taxcodes_total = ();

@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";

foreach (<STDIN>) {

  if ($_ =~ $request)
  {
    s/.*$request/$request/;
    open  XML, ">", "temp.xml";
    print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    print XML $_;
    close XML;
    print "--[$i]--\n";
    print "$_";
    print "\n---------------------------------------------------------------------\n";
    $response =  `perl -I$hammerpath $hammerpath/hammer.pl -h $host -p $port --print temp.xml`;
    @taxcodes = ($response =~ m/$taxcodetag/g);
    foreach (@taxcodes)
    {
      s/TaxCode=//;
      s/\"//g;
      $taxcodes_total{$_}++;
      print "$_:$taxcodes_total{$_}  ";
    }
    print "\n=====================================================================\n";
    $i++;
  }
}

@timeData = localtime(time);
print "TIME $timeData[2]:$timeData[1].$timeData[0]\n";

while (($key, $value) = each(%taxcodes_total))
{
  print "$key:$value\n";
}
     
