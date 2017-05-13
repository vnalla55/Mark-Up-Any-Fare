#!/usr/bin/perl -s
# tip: to see xml sended to server on Liberty Emulator
# DEBUG#CONNECTION/ATSETAX/REQ
# # - cross of L
# 
print "usage: [perl -s] taxDispl.pl -h=srv_host -p=srv_port\n";
print "host:$h port:$p\n";

while (<STDIN>) {

  chomp();
  @entry = split ("/", $_);
	
  open  XML, ">", "temp.xml";
  print XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<AirTaxDisplayRQ>\n";
  print XML "<TaxCode code=\"$entry[0]\"/>\n";
  if($#entry >= 1) {
	if ($entry[1] =~ /^[sS]\d+/){
		
		$entry[1] =~ s/s//;
		$entry[1] =~ s/S//;
        	print XML "<Sequence SequenceMatch=\"$entry[1]\"/>\n";
	}
	elsif($entry[1] =~ /[mM]/){
	        $entry[1] =~ tr/[mM]/T/;
		print XML "<menu code=\"$entry[1]\"/>\n";
	}
	else{
		print XML "<category code=\"$entry[1]\"/>\n";
	}	
  }
  if($#entry >= 2) {
        if ($entry[2] =~ /^[sS]\d+/){        
		$entry[2] =~ s/s//;
		$entry[2] =~ s/S//;
		print XML "<Sequence SequenceMatch=\"$entry[2]\"/>\n";
	}
	elsif($entry[2] =~ /\D/){
	        $entry[2] =~ tr/a-z/A-Z/;
		print XML "<carrierCode code=\"$entry[2]\"/>\n";
	}
	else{
		print XML "<category code=\"$entry[2]\"/>\n";
	}
  }
  if($#entry >= 3) {
  print "XXxxxxxxxXX3 $entry[2]\n";
  	if ($entry[3] =~ /\D/){
  		$entry[3] =~ tr/a-z/A-Z/;
		print XML "<carrierCode code=\"$entry[3]\"/>\n";
	}	
  }
  print XML "  <Source PseudoCityCode=\"23X7\">\n";
  print XML "      <TPA_Extensions>\n";
  print XML "      <UserInfo>\n";
  print XML "	   <Partition ID=\"AA\"/>\n";
  print XML "      <Service Name=\"AIRTAX_RQ\"/>\n";
  print XML "      <AAACity Code=\"DFW\"/>\n";
  print XML "      <AgentSine Code=\"AAT\"/>\n";
  print XML "      </UserInfo>\n";
  print XML "      </TPA_Extensions>\n";
  print XML "  </Source>\n";
  print XML "</AirTaxDisplayRQ>\n";
  close XML;
	
  print `perl ./hammer.pl -h $h -p $p --print temp.xml | perl outFilter.pl`;
}
