#!/usr/bin/perl 

# ---------------------------------------------------------------------------------------
# Author: Mohammad Hossan
# Copyright: Sabre Holdings 2013
#
# Usage:
#      Usage: checkConsistency.sh -h host -p port -f filename [-n N] [ -c N] [-s] [-d] [-x] 
#             -h host, -p port, -f xmlFileName
#             -r N: repeat N times, -c N: Concurrent Trx
#             -s to store Response, -x store XML, -d allow Diag
#
# ---------------------------------------------------------------------------------------

use strict;
use Getopt::Std;
use IO::Socket;

use threads;
use threads::shared;
use Thread::Queue;
use XML::Parser;
use File::Basename;

use Sys::Hostname;


select(STDERR);
$| = 1;
select(STDOUT);
$| = 1;


use vars qw($opt_d $opt_h $opt_p $opt_c $opt_n $opt_f $opt_s $opt_x);
getopts('dh:p:c:n:f:sx');


my $pid=$$;
print "PID: $pid\n";


my $host = $opt_h;
if(not $host)
{
  print "No host\n";
  displayUsage();
  exit;
}
print "host: $host\n";

my $port = $opt_p;
if(not $port)
{
  print "No port\n";
  displayUsage();
  exit;
}
print "port: $port\n";

my $reqFile = $opt_f;
if(not $reqFile)
{
  print "No reqFile\n";
  exit;
}
print "reqFile: $reqFile\n";

my $loopCount = $opt_n;
if(not $loopCount)
{
  # each trx will be tried at least 2 times
  $loopCount=2;
}
print "loopCount: $loopCount\n";

my $concurrentTrx = $opt_c;
if(not $concurrentTrx)
{
  # each trx will be tried at least 2 times
  $concurrentTrx=1;
}
print "concurrentTrx: $concurrentTrx\n";

my $storeResponse = 0;
$storeResponse = 1 if $opt_s;
print "storeResponse: $storeResponse\n";

my $storeXMLResponse = 0;
$storeXMLResponse = 1 if $opt_x;
print "storeXMLResponse: $storeXMLResponse\n";

my $allowDiag = 0;
$allowDiag = 1 if $opt_d;
print "allowDiag: $allowDiag\n";


my $respFilePrefix = "Response";


my $startTime = localtime time;
print "Start Time: $startTime\n";

my ($sec,$min,$hour,$mday,$mon,$year) = localtime();
$year += 1900;
$mon += 1;
my $MM = sprintf("%02d", $mon);
my $DD = sprintf("%02d", $mday);
my $ticketingDT = $year . "-" . $MM . "-" . $DD;
print "ticketingDT = $ticketingDT\n";

my $respFileNum : shared = 0;
my $trxCount : shared = 0;
my $rDiffFileLock : shared = 0;


my $file_withoutPath = basename($reqFile);
(my $file_withoutExt =  $file_withoutPath)  =~ s/\.[^.]+$//;

my $diffTrxFile = "diff_" . "$file_withoutExt" . "_" . $pid;

#open FILE, ">$diffTrxFile" or die "Could not open $diffTrxFile: $!";
#close FILE;

my $trxQueue = new Thread::Queue;
my @elementStack = ();

# ---------- Main Loop -------------
print "Host= $host\n";
print "Port= $port\n";

  $trxCount = 0;
  my $trxThr = threads->create(\&processFile, $reqFile);

  my @thrArry;
  print "concurrentTrx = $concurrentTrx \n";
  sleep 3;

  for(my $j = 0; $j < $concurrentTrx; $j++)
  {
   print " Starting Thread # $j \n";
    my $trxThr = threads->create(\&processTrx, $host, $port);
   push @thrArry, $trxThr;
  }

  for(my $j = 0; $j < $concurrentTrx; $j++)
  {
    $thrArry[$j]->join();
  }

   $trxThr->join();


print "\nTotal trx count: $trxCount\n";
exit;



#------------------------------------------------------------------------
sub processFile
{
	my ($trxFile) = @_;

        print "File= $trxFile\n";
        open(inFile, "$trxFile") or die "Can't open $trxFile";


	my $count = 0;
        my $trxNumStart= "10000001";
	while(<inFile>) 
	{
          my $line = $_;

          if($line =~ /(\w)+/ && $line =~ /</ &&  $line =~ />/)
          {
            my $trxNum = $trxNumStart + $count;
            my @XML = split /(<.*)/, $line;
	    my $trx = "$trxNum$XML[1]";

	    removeDiag($allowDiag, \$trx); # if C10=...  is there 
	    updateTicketingDate($ticketingDT, \$trx);

	    $trxQueue->enqueue($trx);
	    if($trxQueue->pending > 200)
	    {
	      while($trxQueue->pending > 100)
	      {
	        sleep 1;
	      }
	    }
	    $count = $count +1;
	    #print "trx enqueue = $count\n";
	  } 
	}
	print "Done reading file: $trxFile, Trx Count: $count\n";

	close inFile;
	
}
#------------------------------------------------------------------------
sub processTrx
{
	my ($host,$port) = @_;

	my $trxNum = 0;
	while(my $trx = $trxQueue->dequeue_nb)
	{

	    {
	     lock($trxCount);
	     $trxCount = $trxCount + 1;
	     if($concurrentTrx < 3 )
	     {
	       print "\nSend req # $trxCount\n";
	     }
	     else
	     {
	       if($trxCount % 500 == 0)
	       {
	         my ($sec, $min, $hr) = localtime();
		 print "$hr:$min:$sec Sent $trxCount\n";
	       }
	     }
	    }
	  my @prevResp = 0;
          for(my $i=1; $i <= $loopCount; ++$i)
	  {
	    displayMsg("\nLoop count# $i\n");

	    my $start = time;

	    my $sock1;
	    my $sock2;


	    ++ $trxNum;
	    my ($sec, $min, $hr) = localtime();

	    displayMsg( "$hr:$min:$sec Send req $host:$port\n");

	    $sock1 = &sendData($trx,$host, $port);

	    my ($thr1) = threads->create(\&getResponse,$sock1, $start);

	    displayMsg("Waiting for responses...\n");
	     
	    my @trResp1 = $thr1->join();

	    my $taken = time - $start;

	    displayMsg("Received resp from $host in $trResp1[1] Sec\n");
	     
	    close $sock1;	

	    if( @prevResp && $prevResp[0])
	    {   my @S18Resp1 = (); 
	        my @S18Resp2 = ();
	  	my $diffInResp = "";
                parseXMLResponseAndCompare(\$trResp1[0], \$prevResp[0], \@S18Resp1,  \@S18Resp2,  \$diffInResp, $trx);
	        if(! $diffInResp)
	        {
	          displayMsg(" NO diff in response XML\n");

	          $diffInResp = &checkTotalInS18(\@S18Resp1,  \@S18Resp2, $trx);
	          if(! $diffInResp)
	          {
	            displayMsg(" NO diff in Total in S18 attribute\n");
	          }
	        }
		if($diffInResp)
		{
		  writeToFile_l($diffTrxFile, $trx, \$rDiffFileLock, \@S18Resp1, \@S18Resp2, \$diffInResp);
	          if($storeXMLResponse)
	          {
                     my $digNum = &getDiagNumber($trx);
	             writeToRespFile($digNum, "XML_A", $prevResp[0]);
	             writeToRespFile($digNum, "XML_B", $trResp1[0]);
	          }
	          if($storeResponse)
	          {
                    my $digNum = &getDiagNumber($trx);
	            writeToRespFile($digNum, "_A", @S18Resp1);
	            writeToRespFile($digNum, "_B", @S18Resp2);
                    {
                      lock($respFileNum);
                      ++$respFileNum;
                    }
	          }
		  last;
		}
	    }
	    @prevResp = @trResp1;
	  }  # Loop same request N times if diff found break the loop
	}

	print " Done Trx Thread \n";

}

#------------------------------------------------------------------------
sub checkTotalInS18
{
	my  ($S18Resp1, $S18Resp2, $trx) = @_;

	my $hasDiff = 0;

        my $trxType = getTrxType($trx);


	if($trxType eq 'pricing')
	{

	  my $ttlAmount1 = getTTL(@$S18Resp1);
	  my $ttlAmount2 = getTTL(@$S18Resp2);
	  if($ttlAmount1 && $ttlAmount2 && ($ttlAmount1 == $ttlAmount2) )
	  {
	    #displayMsg( " Same Amount: $ttlAmount1 - $ttlAmount2\n");
	    return $hasDiff;
	  }

	  #TOTAL SGD   2848.00
	  $ttlAmount1 = getTOTAL(@$S18Resp1);
	  $ttlAmount2 = getTOTAL(@$S18Resp2);
	  if($ttlAmount1 && $ttlAmount2 && ($ttlAmount1 eq $ttlAmount2) )
	  {
	    #displayMsg(" Same Amount 2: $ttlAmount1 - $ttlAmount2\n");
	    return $hasDiff;
	  }

	  if(sameWPASolutions($concurrentTrx, $S18Resp1, $S18Resp2) != 0)
	  {
	    displayMsg(" Same WPA solutions\n");
	    return $hasDiff;
	  }
	}


	my $size1 = @$S18Resp1;
	my $size2 = @$S18Resp2;
	if($size1 != $size2)
	{
	   displayMsg("Num of S18 attributes are not equal\n");
	   $hasDiff = 1;
	}

	return $hasDiff;
}

#------------------------------------------------------------------------
sub writeToRespFile
{
   my ($digNum, $postFix, @strings) = @_;

   my $fNum = 0;
   {
     lock($respFileNum);
     $fNum = $respFileNum;
   }


   my $respFileName = "$respFilePrefix" . "$fNum" . "_" . "$digNum" . "$postFix";
   open FILE, ">$respFileName" or die "Could not open $respFileName: $!";
   print FILE "@strings";
   close FILE;

}

#---------------------------------------------------------------------------
sub parseXMLResponseAndCompare
{  
        my ($resp1, $resp2, $S18Resp1, $S18Resp2, $diff, $trx) = @_;

        #print "calling parser\n";
        my $parser = new XML::Parser( Style => 'Tree' );
        my $tree1 = $parser->parse( $$resp1);
        my $tree2 = $parser->parse( $$resp2);
        #print "returned from parser\n";

	my $display = 1;
        compareRespArray($tree1, $tree2, $S18Resp1, $S18Resp2, $diff, \$display);

}

sub compareRespArray
{
        my ($arr1, $arr2, $S18Resp1, $S18Resp2, $diff, $display) = @_;

        my $sz1 = @$arr1;
        my $sz2 = @$arr2;

	my $i = 0;
        while($i < $sz1 && $i < $sz2)
        {
           my $type1 = ref $$arr1[$i];
           my $type2 = ref $$arr2[$i];

           if ($type1 && $type2 )
           {
              if ($type1 eq "ARRAY" && $type2 eq "ARRAY")
              {
                compareRespArray($$arr1[$i], $$arr2[$i], $S18Resp1, $S18Resp2, $diff, $display);
              }
              elsif ($type1 eq "HASH" && $type2 eq "HASH")
              {
                compareRespNameValue($$arr1[$i], $$arr2[$i], $S18Resp1, $S18Resp2, $diff, $display);
              }
	      else
	      {
                #print "0. compare RespArray  $$arr1[$i] $$arr2[$i]\n";
	      }
           }
           elsif( (! $type1) && (! $type2) )
	   {
	      my $ele = $$arr1[$i];
              my $nextArr1 = $$arr1[$i+1];
              my $nextArr2 = $$arr2[$i+1];
              my $tref1 = ref $$arr1[$i+1];
              my $tref2 = ref $$arr2[$i+1];
	      if($tref1 && $tref2)
	      {
	        push (@elementStack, $ele);
                compareRespArray($nextArr1, $nextArr2, $S18Resp1, $S18Resp2, $diff, $display);
	        pop(@elementStack);
	      }
	      else
	      {
                #print "1. compare RespArray  $nextArr1 $nextArr2\n";
	      }
	      ++$i;
	   }
           else
           {
              #print "2. compare RespArray $$arr1[$i]: $$arr2[$i]\n";
           }
	   ++$i;
        }
}

sub compareRespNameValue
{

 my ($hash1, $hash2, $S18Resp1, $S18Resp2, $diff, $display) = @_;

 my $sz1 = keys %$hash1;
 my $sz2 = keys %$hash2;

 foreach my $k (sort keys %$hash1)
 {
    #print ("$k=$hash1->{$k} : $hash2->{$k} \n");
    if($k eq "S79")                    # ignore hostname
    {
      next;
    }
    if($k eq "S66")                    # ignore Fare Calc line
    {
      next;
    }
    if($k eq "S18")                    # ignore, may come in different order
    {
      #print "$k $hash1->{$k}\n";
      push @$S18Resp1, "$hash1->{$k}\n";
      push @$S18Resp2, "$hash2->{$k}\n";
      next;
    }
    if( $hash1->{$k} ne $hash2->{$k} )
    {
       #print "Elements: @elementStack \n";
       my $nn = @elementStack;
       my $str = "";
       for(my $j = 0; $j < $nn; ++$j)
       {
         $str = $str . "<" . $elementStack[$j] . ">";
       }
       if($$display)
       {
         displayMsg( "$str\n");
         $$diff .= "$str\n";
       }

       if(length($hash1->{$k})> 10)
       {
         if($$display)
         {
	   $$diff .= "$k= $hash1->{$k} :\n";
	   $$diff .= "     $hash2->{$k} \n";
	 }

         if($$display)
         {
           displayMsg ("$k= $hash1->{$k} :\n");
           displayMsg ("     $hash2->{$k} \n");
         }
       }
       else
       {
         if($$display)
         {
	    $$diff .= "$k= $hash1->{$k} : $hash2->{$k} \n";
            displayMsg ("$k= $hash1->{$k} : $hash2->{$k} \n");
	 }
       }
     $$display = 0;
    }

 }
}



#------------------------------------------------------------------------
sub writeXMLDiffToFile_l
{
	my ($fileName, $trx, $fLock, $diff) = @_;
	lock($fLock);
	open FILE, ">>$fileName" or die "Could not open $fileName: $!";
	print FILE "$trx\n";
	if(($loopCount == 1) && ($concurrentTrx < 3))
	{
	    print FILE "--------------------------------------------------------------\n";
	    print FILE "$$diff";
	    print FILE "--------------------------------------------------------------\n";
	}
	close FILE;
}

#------------------------------------------------------------------------
sub writeToFile_l
{
	my ($fileName, $trx, $fLock, $resp1, $resp2, $diffInResp) = @_;
	lock($fLock);
	open FILE, ">>$fileName" or die "Could not open $fileName: $!";
	print FILE "$trx\n";
	if(($concurrentTrx < 2))
	{
	  if($resp1)
	  {
	    print FILE "--------------------------------------------------------------\n";
	    print FILE "Response A:\n";
	    print FILE "@$resp1";
	    print FILE "--------------------------------------------------------------\n";
	  }
	  if($resp2)
	  {
	    print FILE "--------------------------------------------------------------\n";
	    print FILE "Response B:\n";
	    print FILE "@$resp1\n";
	    print FILE "--------------------------------------------------------------\n";
	  }
	  if($diffInResp)
	  {
	    print FILE "Diff in XML Starts at:\n";
	    print FILE "$$diffInResp \n";
	    print FILE "--------------------------------------------------------------\n";
	  }
	}
	close FILE;
}

#=======================================================================

sub sendData
{
	my ($request, $host, $port) = @_;


	my $sock = new IO::Socket::INET(PeerAddr => $host, PeerPort => $port, Proto => 'tcp' ) or (print STDERR "Could not connect to $host:$port\n" and exit);

	my $command = 'RQST';
	my $header = pack('N',12 + length $request);
	my $data = "$header" . $command . "0000" . $request;
	print $sock $data;

	return $sock; 
}

#------------------------------------------------------------------------
sub writeToFile
{
        my ($fileName, $fLock, $trx) = @_;
        lock($fLock);
        open FILE, ">>$fileName" or die "Could not open $fileName: $!";
        print FILE "$trx\n";
        close FILE;
}
#------------------------------------------------------------------------

sub getResponse
{
        my ($sock, $startTime) = @_;

        my $result = '';
        while(my $line = <$sock>) 
	{
                $result .= $line;
        }

        die "socket disconnected (server crashed?)\n" unless $result;

        close $sock or print STDERR "Could not close socket\n";

        my $command = substr $result, 4, 4;

        $result = substr $result, 12;
        $result = substr $result, 0, ((length $result) - 1);
	my $timeTaken = time - $startTime;
	return ($result, $timeTaken);

}

#------------------------------------------------------------------------

sub getDiagNumber
{

  my ($trx) = @_;
  my $diagNum = 0;
  my  $str = "C10=";
   if($trx =~ 'C10=')
   {
     my @SPLIT1 = split 'C10=', $trx;
     my @SPLIT2 = split '\"', $SPLIT1[1];
     $diagNum = $SPLIT2[1];
   }
  return $diagNum;
}


#------------------------------------------------------------------------
sub getTTL
{
	my  (@response) = @_;
	my $size = @response;
	for(my $i=0; $i < $size ; $i++)
	{
	  my $str = $response[$i];
	  if($str !~  /(\d+\.?\d*|\.\d+)TTL/) 
	  {
	    next;
	  }
	  $str =~ s/(\s+)/ /g;
	  my @tokens = split / /, $str;
	  my $count = @tokens;
	  my $total =  $tokens[$count-1];
	  $total =~ s/TTL//g;
	  return $total;
	}
	return 0;
}
#------------------------------------------------------------------------
sub getTOTAL
{
	my  (@response) = @_;
	my $size = @response;
	for(my $i=0; $i < $size ; $i++)
	{
	  my $str = $response[$i];
	  if($str !~  /TOTAL(\s+)(\D{3})(\s+)(\d+\.?\d*|\.\d+)/) 
	  {
	    next;
	  }
	  return $str;
	}
	return 0;
}
#------------------------------------------------------------------------
sub sameWPASolutions
{
        my ($concurrentTrx, $response1, $response2) = @_;

	my @sol1 = &getWPASolutions(@{$response1});
	my @sol2 = &getWPASolutions(@{$response2});
	if( @sol1 == 0 || @sol2 == 0)
	{
	  return 0;
	}

	#return 0 unless @sol1 == @sol2;
	if(@sol1 != @sol2)
	{
	 if($concurrentTrx < 3 )
	 {
	   print "Diff response Length\n";
	 }
	 return 0;
	}

	for (my $i = 0 ; $i < @sol1; $i++)
	{
	  #return 0 if $sol1[$i] ne $sol2[$i];
	  if( $sol1[$i] ne $sol2[$i])
	  {
	    if($concurrentTrx < 3 )
	    {
	      print "Diff response:\n";
	      print " $sol1[$i]\n";
	      print " $sol2[$i]\n";
	    }
	    return 0;
	  }
	}

	return 1;
}

#------------------------------------------------------------------------
sub getWPASolutions
{
	my  (@response) = @_;
	my @sol = ();
	my $size = @response;
	my $i = 0;
	for(; $i < $size; $i++)
	{
	  my $line = $response[$i];
	  #if($line !~ /FARE BASIS BOOK CODE           FARE      TAX       TOTAL/) 
	  if($line !~ /FARE BASIS BOOK CODE           FARE/)
	  {
	    next;
	  }
	  last;
	}
	for(; $i < $size; $i++)
	{
	  my $line = $response[$i];
	  $line =~ s/(\s+)/ /g;
	  $line =~ s/@/ /g;
	  $line =~ s/\*/ /g;
	  my @tokens = split / /, $line;
	  my $count = @tokens;
	  if($count >= 7)
	  {
	     if($tokens[0] =~ /(\d\d)/ )
	     {
	       my $str = $tokens[0];
	       if($count == 7)
	       {
	         $str = $str . " " . $tokens[$count-1];
	       }
	       else
	       {
	         $str = $str . " " . $tokens[$count-2];
	       }
	       push @sol, $str;
	       #print " $str\n";

	     }
	  }
	}
	return @sol;

}

#------------------------------------------------------------------------
sub removeDiag
{
   my ($allowDiag, $trx) = @_;

   if(! $allowDiag)
   {
     $$trx =~ s/C10=\"...\"//g;
   }

}

#------------------------------------------------------------------------
sub updateTicketingDate
{
   my ($ticketingDT, $trx) = @_;
   $$trx =~ s/ D07=".*?" / D07=\"$ticketingDT\" /g;
}

#------------------------------------------------------------------------

sub getTrxType
{
        #  trxType: pricing/faredisplay/tax

        my ($trx) = @_;
        my $type= 'pricing';

        if($trx =~ /PricingRequest/ ||
           $trx =~ /CurrencyConversionRequest/ ||
           $trx =~ /CURRENCYCONVERSIONREQUEST/ ||
           $trx =~ /SelectionRequest/ )
        {
           $type = 'pricing';
        }
        elsif($trx =~ /FAREDISPLAYREQUEST/ )
        {
           $type = 'faredisplay';
        }
        elsif($trx =~ /AirTaxRQ/ )
        {
           $type = 'tax';
        }
        return $type;
}

#------------------------------------------------------------------------
sub displayMsg
{
  my ($msg) = @_;

  if($concurrentTrx < 3 )
  {
    print "$msg";
  }

}

#------------------------------------------------------------------------
sub getInputFileName
{
    my ($fullyQualifiedName) = @_;

}

#------------------------------------------------------------------------
sub displayUsage
{
   print " 
      Usage: checkConsistency.sh -h host -p port -f filename [-n N] [ -c N] [-s] [-d] [-x] 
             -h host, -p port
             -r N: repeat N times, -c N: Concurrent Trx
             -s to store Response, -x store XML, -d allow Diag\n";
}

