#!/usr/bin/perl


###################################################################
#Method: socket_client.pl
#Description: This script can be used to send xml reqests
#to atsev2 server or v2 RTG server using 1 or more clients.
#Options also include, starting line number in a request xml and 
#number of requests to send. A log file can also be specified
# 
#This works for any socket service that takes xml as input.
#Has tested on atsev2 TSE server and RTG server.
#For TSE requests each  xml requests should be preceded by a 8 character
#string 
#
#socket_client --help for arguments for the script.
#
#Authors: Daryl Champagne and Hitha Alex
#Copyright: Sabre 2006
####################################################################

use Getopt::Long;
use IO::Socket;
use POSIX ":sys_wait_h";

$print_to_logfile = 0; # make it 0, if no log file  output is required, else 1
$print_to_stdout = 1; # make it 0, if no std output is required, else 1
$log_file = ""; 
$host = "";
$port = "";
$hpfile = "";
$xmlfile = "";
$noClients = 0;
$startReqNo=0;
$numReqs=0;
$verbose = 0;
$brief = 0;
$help = 0;


sub processOptions
{
my $rc = GetOptions("verbose" => \$verbose,
              "brief" => \$brief,
              "greenscreen" => \$greenscreen,
		      "start:i" => \$startReqNo,
		      "numreqs:i"=> \$numReqs,	
		      "clients:i"=> \$noClients,
		      "log=s"=> \$log_file,		
              "help" => \$help);
  if (!$rc)
  {
    &usage();
    exit 1;
  }

  if ($help)
  {
    &usage();
    exit 0;
  }
  if ($#ARGV+1 !=  2)
  {
    print STDERR "Error: wrong number of arguments.\n";
    &usage();
    exit 1;
  }

  if ($verbose)
  {
    print "Verbosity turned on.\n";
  }

  if ($greenscreen)
  {
    $brief = 1;
  }

  if($log_file)
  {
    $print_to_logfile = 1;
    #open the log file
    open (LOG, ">$log_file");
    print_both("Writing to log file $log_file\n");
  }

  $hpfile = $ARGV[0];
  $xmlfile = $ARGV[1];
  if(!$noClients)
  {
     $noClients = 1;
  }
  if ($verbose)
  {
    print_both("Host/Port file: $hpfile\n");
    print_both("XML file: $xmlfile\n");
    print_both("No of clients: $noClients\n");
  }
}

sub usage
{
  print <<EOF

Usage: $0 <hpfile> <xmlfile> [OPTIONS]

<hpfile>                   host:port string or filename containing socket host:port 
                           string to use to connect to server
<xmlfile>                  Filename containing XML requests to send to server. 
                           Each request should be on a line by itself.
--verbose, -v              Display verbose messages
--brief, -b                Display brief output 
--greenscreen, -g          Display even more brief output (SabreView output)
--help, -h                 Display usage
--start=<num>, -s=<num>    Start processing requests at request number <num>
--numreqs=<num>, -n=<num>  Only process <num> requests and then exit
--clients=<num>, -c=<num>  Total number of clients to send requests to the server 
                           at the same time. Default = 1
--log=<logfilename>        Write all std outputs also to a log file <logfilename>
EOF
}

sub decodeSocketIor
{
 if(! -e $hpfile)
 {
 	#Assume its a host:port string
	($host, $port) = split /:/, $hpfile
 }
 else
 {
  	open IORFILE, "<$hpfile" or die "Failed to open $hpfile: $!";
  	my @data = <IORFILE>;
  	close IORFILE;

  	if ($data > 1)
  	{
    	die "Invalid Socket IOR file format.";
  	}

  	($host, $port) = split /:/, $data[0];
  }
}

sub sendMsg
{
	my ($request,$host,$port) = @_;

	if($verbose) {
		print "Connecting to $host:$port\n";
	}
	my $sock = new IO::Socket::INET(PeerAddr => $host, PeerPort => $port, Proto => 'tcp' ) or \
    die "Failed to create socket: $!";

  # First 12 bytes is header
	my $header1 = pack('N', 16);
	my $header2 = pack('N', length $request);
	my $data = $header1 . $header2 . "RESP" . "0001". "0000" .  $request;

	print $sock $data;
	return $sock;
}

sub recvResp
{
	my ($sock) = @_;

  print_both("\tWaiting for response...\n");
  my @response = <$sock> or die "Failed to receive response: $!";
	close $sock;
  # Remove header
	$response[0] = substr $response[0], 12;

	my $result = join //, @response;
	return substr $result, 0, ((length $result) -1);
}

sub pause
{
    print "Press return to continue...";
    <STDIN>;
}
sub timestamp
{
@Month_name = ( "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" );
( $sec, $min, $hour, $day, $month, $year ) = ( localtime ) [ 0, 1, 2, 3, 4, 5 ];
  $year = $year +1900;
  return "$hour:$min:$sec $day @Month_name->[$Month] $year";
  #printf  "%02d:%02d:%02d %02d %s %04d\n", 
  #$hour, $min, $sec, $day, @Month_name->[$month], $year+1900 ;
}

&processOptions();


# Open and read XML file
open DATAFILE, "<$xmlfile" or die "Failed to open $xmlfile: $!";
@data = <DATAFILE>;
close DATAFILE;

# Get hostname and port from specified IOR file
&decodeSocketIor();

#Fork as many clients as requested

for ($no = 0 ; $no < $noClients ; $no++) 
{
   $clientID = $no + 1;
   print_both("Spawning client $clientID\n");
   if($pid = fork)
   { 
   }
   elsif($pid == 0)
   {
      #Send each line of the data file and receive the response
      $i = 0;
      $reqs = 0;
      $lines = scalar(@data);
      # For Average Elapsed Time Calculations
      $elapsedsum = 0.0;
      $maxtime = 0.0;
      foreach $line (@data) 
      {
         chomp $line;
	 $line = modifyRequest($line);
         $i++; 

	# If the starting number was specified skip until we get to it.
  	if ($startReqNo != 0 && $i < $startReqNo)
        {
           next;
        }
  	if ($numReqs != 0 && $reqs >= $numReqs)
        {
           last;
        }
        $reqs++;
        if ($verbose)
         {
            print_both("Processing request: $line\n");
         }
	 $timestamp = &timestamp;
	 $starttime = time;
         my $sock = &sendMsg($line,$host,$port);
	 print_both("Client $clientID : $timestamp : Send request #$i of $lines\n");
         my $response = &recvResp($sock);
	 $timestamp = &timestamp;
	 $endtime = time;
         $transtime = $endtime - $starttime;
         $elapsedsum += $transtime;
         if($maxtime < $transtime)
         {
            $maxtime = $transtime;
         }
	 $elapsedtimeclock = &elapsedTime($transtime);
	 print_both("Client $clientID : $timestamp : Received response #$i of $lines, Elapsed Time: $elapsedtimeclock\n");
         if ($verbose)
         {
            $response = &formatResponse($response);		
            print_both("Response: $response\n");
         }

         if ($brief)
         {
            $response = &formatBriefResponse($response);

            my $t = $greenscreen;
            $greenscreen = 0; # enable printing
            print_both("\n$response\n");
            $greenscreen = $t;
         }

      }

      $averageE = $elapsedsum/$reqs;

      print_both("\n****Client $clientID :\n\t\tNo of requests: $reqs\n\t\tAverage Elapsed Time: $averageE seconds\n\t\tMax Elapsed Time: $maxtime seconds\n\n");

      exit;
   }

   else
   {
     die "Can't fork: $!\n";
   }
   
}# End Fork
do 
{ 
      sleep(1);
      $kid = waitpid(-1,&WNOHANG);
} until $kid == -1;

sub print_both
{
    return if $greenscreen;

    if($print_to_stdout)
    {
    	print STDOUT @_;
    }
    if($print_to_logfile) 
    {    
     	print LOG @_;
    }
}

sub elapsedTime
{
   my @times = @_;
   $eSeconds = $times[0];
   $eHours = quotient ($eSeconds, 3600);
   $eSeconds = $eSeconds % 3600;
   $eMinutes = quotient ($eSeconds, 60);
   $eSeconds = $eSeconds % 60;
   return "$eHours:$eMinutes:$eSeconds";
	
}
# integer division: compute $n div $d (so 4 div 2 is 2, 5 div 2 is also 2)

# parameters are $n then $d
sub quotient {
    my $n = shift; my $d = shift;
    my $r = $n; my $q = 0;
    while ($r >= $d) { 
	$r = $r - $d;
	$q = $q + 1;
    }
    return $q;
}

# Subsitute any text before the starting XML tag "<" with
# dummy 8 character text "TransNum"
sub modifyRequest {
        my $goodRequest = shift;
        my $startIndex = index($goodRequest, '<');
        my $fragment = substr($goodRequest, 0, $startIndex);
        $goodRequest =~ s/^$fragment/TransNum/;
        return $goodRequest;
}
sub formatResponse {
	my $resp = shift;
	$resp =~ s/>/>\n/g;
	return $resp;
}

sub formatBriefResponse {
    my $resp = shift;
    my @lines = split (/>/, $resp);

    my $ret;

    foreach my $line (@lines)
    {
        $ret .= "$line\n" if ($line =~ s/^<MSG\s.*S18=\"(.*)\".*/\1/);
    }

    return $ret;
}

