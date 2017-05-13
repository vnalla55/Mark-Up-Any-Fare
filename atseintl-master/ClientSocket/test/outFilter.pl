
@xml =  <STDIN>; 
@disp = split ( "\"/>", $xml[0]);
$disp[0] =~ s/<.*>//;
$disp[-1] = "";
foreach (@disp) {
  s/<MSG.*=\"//;
  
  if(length() > 63){
  	print "WARNING::Next linie too width, width = ", length(), "\n";
  } 
  print "$_\n";
}	


