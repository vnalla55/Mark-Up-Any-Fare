#!/usr/bin/perl -w

use strict;

# -------------------------------------------------------
# Author: Mohammad Hossan
# Copyright: Sabre March 2011
#
# SID: SOP ID of a Leg
# Q14: Leg ID ( 0, 1, 2)
# Q15: SOP ID
# Q1K: Flight ID
#
# ITN has list of SID [one for each leg, mostly 2]
# SID has {Q14, Q15}, Q14 and Q15 combination is unique SOP/flight
# of the Itinerary
#
# ITN - (Q14) -> Leg - (Q15) -> SOP - (Q1K) -> AAF
# AAF -> SGI/FLI
#
# -------------------------------------------------------

use XML::Parser;

my $paramCount = @ARGV;
if($paramCount < 1)
{
   print "Usage: $0 MipXmlFile <outfile>\n";
   exit;
}

my $xmlFile = shift @ARGV;
my $outFileName = "PricingReqFromMip.xml";
my $parm2nd = shift @ARGV;
if($parm2nd)
{
  $outFileName = $parm2nd;
}

open(inFile, "$xmlFile") or die "Can't open $xmlFile";

open (outFile, ">$outFileName") or die " Cannot open file: $outFileName\n";
print "Output Goes to File: $outFileName\n";



my $parser = new XML::Parser( Style => 'Tree' );
 

my $trx = "";
my $itins = 0;
my %fliHash = ();
my %idsHash = ();
my %legHash = ();
my %legBKKHash = ();

my $trxCount="10000001";
my $commonPart = "";

my $start = 0;
while(<inFile>)
{
  #if((/ShoppingRequest N06/) || (/ShoppingRequest D70/))
  if((/ShoppingRequest/))
  {
    if($trx)
    {
      #print outFile "$trx\n";
      $trx .= $_;
      $commonPart = "$trxCount<PricingRequest>";
      my $tree = $parser->parse("$trx");
      scanParsedArray("", $tree);
      $trx="";
      $start=0;
      ++$trxCount;
    }
    else
    {
     $start=1;
     $trx .= $_;
    }
    
  }
  elsif($start)
  {
     $trx .= $_;
  }
}
if($trx)
{
      $commonPart = "$trxCount<PricingRequest>";
      my $tree = $parser->parse("$trx");
      scanParsedArray("", $tree);
}

#foreach my $k (sort keys %fliHash) 
#{
#  print " $k: $fliHash{$k} \n";
#}

#my $sz = keys %legHash;
#print " Number of Leg: $sz\n";

#foreach my $k (sort keys %legHash) 
#{
#
#  my $hash = $legHash{$k};
#  $sz =  keys %$hash;
#  print " # of SOP in Leg $k:  $sz\n";
#
#  foreach my $l (sort keys %$hash)
#  {
#    #print " Q15: $l\n";
#    my $arr = $hash->{$l};
#    print "Q15: $l   Q1K list: @$arr \n";
#    foreach my $m (@$arr)
#    {
#      #print "Q15: $l  Q1K: $m \n";
#    }
#  }
#}

#--------------------------------------------------------------------------

sub scanParsedArray
{
  my ($inset,$current) = @_;
  $inset .= "  ";
  my $k = 0;
  my $j = 1;
  while($k<@$current)
  {
   my $type = ref $$current[$k]; 

   if(! $type)
   {
     my $info = $$current[$k];
     #print ("$inset$j: $info\n");

     if($info eq "AGI")
     {
       ++$k;
       buildElement("AGI", $$current[$k]);
     }
     elsif($info eq "BIL")
     {
       ++$k;
       buildElement("BIL", $$current[$k]);
     }
     elsif($info eq "PRO")
     {
       ++$k;
       buildElement("PRO", $$current[$k]);
     }
     elsif($info eq "PXI")
     {
       ++$k;
       buildElement_PXI($$current[$k]);
     }
     elsif($info eq "AAF")
     {
       ++$k;
       buildFlightSegments($$current[$k]);
     }
     elsif($info eq "LEG")
     {
       ++$k;
       getLeg($$current[$k]);
     }
     elsif($info eq "ITN")
     {
       ++$k;
       buildItin($$current[$k]);
       #last; # <<== to create request for only one Itinerary
     }
     ++$j;
   }
   else
   {
     #print ("$inset$k: $type\n");
     if ($type eq "ARRAY") 
     {
       scanParsedArray($inset,$$current[$k]);
     } 
     elsif ($type eq "HASH") 
     {
       #displayNameValue($inset,$$current[$k]);
     }
   } 
   ++$k;
 }
}

#-----------------------------------------------

sub buildElement 
{
  my ($tag, $arr) = @_;

  $commonPart .= "\<$tag ";
  getNameValue($arr);

  if($tag eq "AGI")
  {
    if($commonPart !~ /C40/)
    {
      $commonPart .= "C40=\"USD\" ";
    }
    elsif($commonPart =~ /C40=\"\"/)
    {
      $commonPart =~ s/C40=\"\"/C40=\"USD\"/;
    }
  }
  if($tag eq "PRO")
  {
    if($commonPart !~ /P51/ && $commonPart !~ /P52/ && $commonPart !~ /P1V/)
    {
      $commonPart .= "P52=\"T\" ";
    }
  }

  $commonPart .= "/>";
}

#----------------------------------------------

sub getNameValue 
{
 my ($arr) = @_;
 my $hash = $$arr[0];
 foreach my $k (sort keys %$hash) 
 {
     my $info = $hash->{$k};
     $commonPart .= "$k=\"$info\" ";
     #print ("$k=\"$info\" ");
 }
}

#-----------------------------------------------

sub buildElement_PXI 
{
  my ($arr) = @_;

  $commonPart .= "\<PXI ";
  getNameValue_PXI($arr);

  $commonPart .= "/>";
}

#----------------------------------------------

sub getNameValue_PXI 
{
 my ($arr) = @_;
 my $hash = $$arr[0];
 foreach my $k (sort keys %$hash) 
 {
     my $info = $hash->{$k};
     #print ("$k=\"$info\" ");
     if($info =~ /\D\d\d/)
     {
       my $paxType = $info;
       my $age = $info;
       $paxType =~ s/\d\d/NN/; 
       $age =~ s/\D//; 

       $commonPart .= "$k=\"$paxType\" ";
       $commonPart .= "Q0T=\"$age\" ";
     }
     else
     {
       $commonPart .= "$k=\"$info\" ";
     }
 }
}
#-------------------------------------------

sub buildFlightSegments 
{
  my ($arr) = @_;

  my $q1k = "Q1K";

  my $fliAttr = "";
  getFLtAttibuteValue(\$fliAttr, \$q1k, $arr);

  my ($sec,$min,$hour,$mday,$mon,$year) = localtime();
  $year += 1900;
  $mon += 1;
  my $d00 = sprintf("%d-%02d-%02d", $year, $mon, $mday);

  my $fli =  $fliAttr . " D00=\"$d00\"" . " N03=\"A\" BB0=\"OK\" />";

  my $ids = "\<IDS ". $fliAttr . " BB0=\"OK\" />";

  #print "$q1k: $fli\n";

  $fliHash{$q1k} = $fli;
  $idsHash{$q1k} = $ids;
}

#-------------------------------------------

sub getFLtAttibuteValue 
{
  my ($nvList, $q1k, $arr) = @_;

  my $eleTag = "";
  foreach my $i (@$arr)
  {
    my $type = ref $i;
    if(!$type)
    {
      $eleTag = $i;
      next;
    }
    else
    {
      if($eleTag eq "HSP")
      {
         #print "prev tag: $eleTag\n";
         #print "Not taking: HSP\n";
	 $eleTag= "";
         next;
      }

      if($type eq "ARRAY") 
      {
        getFLtAttibuteValue($nvList, $q1k, $i);
      }
      elsif ($type eq "HASH") 
      {
        my $hash = $i;
        foreach my $k (sort keys %$hash) 
        {
           my $info = $hash->{$k};
	   if($$q1k eq $k)
	   {
             $$q1k=$info;
             #$$nvList .= "$k=\"$info\" ";
	   }
	   else
	   {
	     if($k eq "D31" || $k eq "D32")
	     {
	       my $hh = substr($info, 0, 2);
	       my $mm = substr($info, 2, 2);
	       my $numMinutes = $hh * 60  + $mm;
	       my $mmmm = sprintf("%04d", $numMinutes);
               $$nvList .= "$k=\"$mmmm\" ";
	     }
	     elsif($k eq "P0Z")
	     {
               $$nvList .= "P2X=\"$info\" ";
	     }
	     elsif ($k eq "S05")
	     {
	       # don't take it
	     }
	     else
	     {
               $$nvList .= "$k=\"$info\" ";
	     }
	   }
        }
      }
    }
  }
}
#-------------------------------------------

sub buildItin 
{

  my ($arr) = @_;

  my $res = "<RES>";
  my $seg = "";
  my $k = 0;
  my $q0cCount=1;
  while($k<@$arr)
  {
     my $type = ref $$arr[$k]; 
     if(! $type)
     {
       my $tag = $$arr[$k];
       if($tag eq "SID")
       {
         ++$k;
	 my $q14;
	 my $q15;
	 my @bkkArr = ();
         readSID($$arr[$k], \$q14, \$q15, \@bkkArr);
	 my $q1k;
	 my $q1kArr = $legHash{$q14}->{$q15};
	 #print "@$q1kArr\n";
	 my $segCount=1;
         foreach my $i (@$q1kArr)
	 {
            $seg .= sprintf("<SGI Q0C=\"%02d\"><FLI Q0C=\"%02d\" ", $q0cCount, $q0cCount);
	    if(@bkkArr)
	    {
	      $seg .= "B30=\"$bkkArr[$segCount-1]\" ";
	    }
	    else
	    {
	      $seg .= "B30=\"$legBKKHash{$q14}\" ";
	    }
	    $seg .= $fliHash{$i};
	    $seg .= "\</SGI>";
	    ++$segCount;
	    ++$q0cCount;
	 }
         foreach my $i (@$q1kArr)
	 {
	    $res .= $idsHash{$i};
	 }

       }
     }
     ++$k;
  }
  my $itin = $commonPart;
  $itin .= $seg;
  $itin .= $res;
  $itin .= "\</RES>";
  $itin .= "\</PricingRequest>";
  print outFile "$itin\n";

}

#---------------------------------

sub readSID
{
  my ($arr, $q14, $q15, $bkkArr) = @_;
  my $k = 0;
  while($k<@$arr)
  {
    my $type = ref $$arr[$k]; 
    if(!$type)
    {
      my $tag = $$arr[$k];
      #print " $tag\n";
      if($tag eq "BKK")
      {
        ++$k;
	readBKK($$arr[$k], $bkkArr);
      }
    }
    else
    {
      if( $type eq "HASH")
      {
          $$q14 = $$arr[$k]->{'Q14'};
          $$q15 = $$arr[$k]->{'Q15'};
          #print "Leg ID: $$q14\n";
          #print "SPO ID: $$q15\n";
      }
    }
    ++$k;
  }

}
#---------------------------------

sub readBKK
{ 
  my ($arr, $bkkArr) = @_;
  foreach my $i (@$arr)
  { 
    my $type = ref $i;
    if(!$type)
    {
      #print " $i\n";
      next;
    }
    else
    {
      if( $type eq "HASH")
      {
	  if(exists $i->{'B30'})
	  {
	    push @$bkkArr, ($i->{'B30'});
            #print "B30: $i->{'B30'}\n";
	  }
	  elsif (exists $i->{'B31'})
	  {
	    push @$bkkArr, ($i->{'B31'});
            #print "B31: $i->{'B31'}\n";
	  }
      }
    }
  }
  #print "BKK : @$bkkArr\n";

}


#-------------------------------------------

sub getLeg 
{

  # this method builds: hash of hash of array
  # Leg { LegID -> { Q15 -> [ Q1K, Q1K, ..], Q15 -> [Q1K, Q1K, ..] } , LegID -> {.....} }

  my ($arr) = @_;

  my $legID = 0;
  my $legBKK = 'X';
  my %legSop = ();
  foreach my $i (@$arr)
  {
    my $type = ref $i;
    if(!$type)
    {
      #print " $i\n";
      next;
    }
    else
    {
      if( $type eq "HASH")
      {
          $legID = $i->{'Q14'};
          $legBKK = $i->{'B31'};
          #print "Leg BKK: $legBKK\n";
          #print "Leg ID: $legID\n";
      }
      else
      {
         getLegSop(\%legSop, $i);
      }
    }
  }
  $legBKKHash{$legID} = $legBKK;
  $legHash{$legID} = \%legSop;

}

#-------------------------------------------
sub getLegSop
{
  my ($legSop, $arr) = @_;

  my $sopID = 0;
  my @a = ();
  foreach my $i (@$arr)
  {
    my $type = ref $i;
    if(!$type)
    {
      #print " $i\n";
      next;
    }
    else
    {
      if( $type eq "HASH")
      {
        $sopID = $i->{'Q15'};
        #print "sopID: $sopID\n";
      }
      elsif( $type eq "ARRAY")
      {
         foreach my $j (@$i)
	 {
	   my $t = ref $j;
	   if(! $t)
	   {
	     #print "$t\n";
	   }
	   elsif(exists($j->{'Q1K'}))
	   {
	     #print "pushing Q1K : $j->{'Q1K'} \n";
	     push @a, ($j->{'Q1K'});
	   }
	 }
      }
    }
  }

  if(@a)
  {
    #print "After Push: @a\n";
    $legSop->{$sopID} = \@a;
  }


}

#-------------------------------------------
sub displayNameValue 
{
 my ($inset,$current) = @_;
 $inset .= "  ";
 my $k;
 foreach $k (sort keys %$current) 
 {
   my $type = ref $$current{$k}; 
   if ($type)
   {
     #print ("$inset$k: $type\n");
     if ($type eq "ARRAY") 
     {
       scanParsedArray($inset,$$current{$k});
     } 
     elsif ($type eq "HASH") 
     {
       displayNameValue($inset,$$current{$k});
     }
   } 
   else 
   {
     my $info = $$current{$k};
     #print ("$inset$k: $info\n");
   }
 }
}

