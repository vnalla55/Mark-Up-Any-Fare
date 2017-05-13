#! /usr/bin/perl
use strict;
use IO::Socket;
use xml_formatter;
use Time::HiRes qw( time );

my $host = 'piili001';
my $port = 61000;
my $async = 0;
my $print_responses = 0;
my $store_responses = 0;
my $check_responses = 0;
my $duplicates = 1;
my @pids = ();
my @requests = ();
my $warm_first = 0;
my $print_request = 0;
my $format_request = 1;
my $compress = 0;
my @expect = ();

my @args = ();

my %request_args = ();

my $options_file = $ENV{'HOME'} . "/.hammer";

if(open OPTIONS, "<$options_file") {
	my $line = <OPTIONS>;
	chomp $line;
	@args = split / /, $line;
	close OPTIONS;
}

@args = (@args,@ARGV);

while($_ = shift @args) {
	if($_ =~ /^--piili0\d\d$/) {
		($host) = $_ =~ /--(piili0\d\d)/;
		$port = 53601;
	} elsif($_ =~ /^--piclp\d\d\d$/) {
		($host) = $_ =~ /--(piclp\d\d\d)/;
		$port = 53601;
	} elsif($_ eq '--atseibld2') {
		$host = 'atseibld2';
		$port = 53601;
	} elsif($_ eq '--pinlc108') {
		$host = 'pinlc108';
		$port = 53601;
	} elsif($_ eq '--cert' or $_ eq '--cert1') {
		$host = 'piili001';
		$port = 61000;
	} elsif($_ eq '--cert2') {
		$host = 'piili002';
		$port = 61000;
	} elsif($_ eq '--cert3') {
		$host = 'pinlc108';
		$port = 61000;
	} elsif($_ eq '--cert4') {
		$host = 'piili005';
		$port = 61000;
	} elsif($_ eq '--cert5') {
		$host = 'piili006';
		$port = 61000;
	} elsif($_ eq '--testdeploy') {
		$host = 'atseibld2';
		$port = 28300;
	} elsif($_ eq '--host' or $_ eq '-h' or $_ eq '--hosts') {
		$host = shift @args;
	} elsif($_ eq '--port' or $_ eq '-p') {
		$port = shift @args;
	} elsif($_ eq '--ior' or $_ eq '-i') {
		my $filename = shift @args or die "must specify a filename after $_ option";
		open IOR, "<$filename" or die "Could not open ior file '$_': $!";
		my $line = <IOR>;
		chomp $line;
		($host,$port) = split /:/, $line;
		close IOR;
	} elsif($_ eq '--async' or $_ eq '-a') {
		$async = 1;
	} elsif($_ eq '--print') {
		$print_responses = 1;
	} elsif($_ eq '--silent') {
		$print_responses = 0;
	} elsif($_ eq '--prettyprint') {
		$print_responses = 'pretty';
	} elsif($_ eq '--printrequest') {
		$print_request = 1;
	} elsif($_ eq '--output') {
		my $filename = shift @args or die "must specify a filename after $_ option";
		open STDOUT, ">$filename" or die "Could not open file '$filename'";
	} elsif($_ eq '--store') {
		$store_responses = 1;
	} elsif($_ eq '--check') {
		$check_responses = 1;
	} elsif($_ eq '--duplicates') {
		$duplicates = shift @args;
	} elsif($_ eq '--expect') {
		push @expect, (shift @args);
	} elsif($_ eq '--help') {
		print "usage: $0 <options> <requests>
OPTIONS:
    -h, --host:  host to connect to (default: piili001)
    -p, --port:  port number to use (default: 61000)
    -i, --ior:   file which specifies the host and port to connect to in the format host:port
        --cert:
		--cert1: sends the request to the first cert server (piili001,61000)
		--cert2: sends the request to the second cert server (piili002,61000)
	-a, --async: send all requests at the same time
        --print: output responses to standard output
        --prettyprint: output responses to standard output with XML nicely formatted (this option will also check that the response is actually valid XML)
        --silent: don't output responses
        --output: sends all output to the given file, rather than to standard output
        --printrequest: output the modified, formatted version of the request that is being sent to the server
        --store: responses will be stored in files in the format <request>.response
        --check: responses will be checked to see if they match files in the format <request>.response
        --expect: regular expression to expect in responses. An error message will be printed if each response doesn't contain the specified string.
        --duplicates:  duplicate each request this many times (default: 1)
        --diag:  override the diagnostic number specified in the request
        --nodiag: a diagnostic will not be used
        --diagarg: sets the following parameter as a diagnostic argument. e.g. --diagarg SHOW_MATRIX or --diagarg SOLUTIONS=100
        --is:    send the request as an IS request
        --mip:   send the request as a MIP request
        --ismip: send the request as both an IS and MIP request (single pass)
        --solutions: override the number of solutions specified in the request
		--estimates: override the number of estimated solutions specified in the request
        --timeout: override the server timeout specified in the request
        --warm:  warms the server by sending request(s) through once before reporting results
        --legs:  makes the request have no more than the given number of legs
		--sops:  makes each leg in the request have no more than the given number of sops
		--itin: makes the request only send the given itin number
		--nitins: makes the request only send the first n itins
        --pricing: the request formatting process won't assume the request is a shopping request
		--noestimates: makes a MIP request ask for true prices rather than estimates for all itineraries
        --compress: communicate with the server using compression
		--stdin: read request from standard input
";
	} elsif($_ eq '--diag') {
		$request_args{'diag'} = shift @args;
	} elsif($_ eq '--nodiag') {
		$request_args{'nodiag'} = 1;
	} elsif($_ eq '--diagarg') {
		$request_args{'diag'} or die "--diag option must be set before --diagarg can be used";
		my $arg = shift @args or die "must specify an argument to $_";
		my ($name,$value) = split /=/, $arg;
		$request_args{'diagargs'} = {} unless $request_args{'diagargs'};
		$request_args{'diagargs'}->{$name} = $value;
	} elsif($_ eq '--is') {
		$request_args{'request_type'} = 'I';
	} elsif($_ eq '--mip') {
		$request_args{'request_type'} = 'M';
	} elsif($_ eq '--ismip') {
		$request_args{'request_type'} = 'S';
	} elsif($_ eq '--solutions') {
		$request_args{'solutions'} = shift @args;
	} elsif($_ eq '--estimates') {
		$request_args{'estimates'} = shift @args;
	} elsif($_ eq '--timeout') {
		$request_args{'timeout'} = shift @args;
	} elsif($_ eq '--warm') {
		$warm_first = 1;
	} elsif($_ eq '--legs') {
		$request_args{'legs'} = shift @args;
	} elsif($_ eq '--sops') {
		$request_args{'sops'} = shift @args;
	} elsif($_ eq '--itin') {
		$request_args{'itin'} = shift @args;
	} elsif($_ eq '--nitins') {
		$request_args{'nitins'} = shift @args;
	} elsif($_ eq '--pricing') {
		$format_request = 0;
	} elsif($_ eq '--noestimates') {
		$request_args{'noestimates'} = 1;
	} elsif($_ eq '--parentsonly') {
		$request_args{'parentsonly'} = 1;
	} elsif($_ eq '--compress') {
		$compress = 1;
	} elsif($_ eq '--noavail') {
		$request_args{'noavail'} = 1;
	} elsif(/^\-.+/ and $_ ne '--stdin') {
		print STDERR "unknown option: $_\n";
		exit;
	} else {
		push @requests, $_;
	}
}

if ( $duplicates > 1 )
{
	my @array = @requests;
	while ( $duplicates > 1 )
	{
		@requests = (@requests,@array);
		$duplicates--;
	}
}

unless(@requests) {
	print STDERR "must specify at least one request on the command line.\n";
	exit;
}

if($warm_first) {
	print STDERR "first warming the server....\n";
	foreach my $request (@requests) {
		my $sock = &send_data($request,$host,$port);
		my $response = &get_response($sock);
		close $sock;
	}

	print STDERR "server is warm. Sending requests....\n";
}

my $start_all = time;

my @bad = ();

foreach my $request (@requests) {
	print STDERR "sending '$request'...\n";
	my $pid = 0;
	$pid = fork if $async;
	die "could not fork: $!" if $pid == -1;
	if($pid == 0) {
		my $start = time;
		my $sock = &send_data($request,$host,$port);
		print STDERR "sent '$request'. Getting response...\n";
		my $response = &get_response($sock);
		if($print_responses eq 'pretty') {
			print &format_xml($response, ('output_text' => 1)) . "\n";
		} elsif($print_responses) {
			print "$response\n";
		}

		close $sock;

		if($check_responses) {
			open FILE, "<$request.response" or die "Could not find response file '$request.response': $!";
			my @lines = <FILE>;
			close FILE;

			my $contents = join /\n/, @lines;
			if($contents ne &format_xml($response)) {
				print STDERR "response from request '$request' did not match expected results\n";
				push @bad, $request;
			}
		}

		if($store_responses) {
			open FILE, ">$request.response" or die "Could not open response file: $!";
			print FILE &format_xml($response);
			close FILE;
		}

		foreach my $expect (@expect) {
			if($response !~ /$expect/) {
				print STDERR "ERROR: response from request '$request' did not contain expected string '$expect'\n";
				exit 0;
			}
		}

		my $taken = time - $start;

		print STDERR "request '$request' processed in $taken seconds\n";
		
		exit 0 if $async;
	} else {
		print STDERR "forked $pid to send '$request'\n";
		push @pids, $pid;
	}
}

foreach (@pids) {
	wait;
}

my $taken = time - $start_all;
print STDERR "all requests processed in $taken seconds\n";

if($check_responses and not $async) {
	if(@bad) {
		print STDERR scalar(@bad) . " REQUEST(S) RETURNED UNEXPECTED RESULTS: ";
		print STDERR @bad;
		print STDERR "\n";
	} else {
		print STDERR "ALL REQUESTS RETURNED EXPECTED RESULTS\n";
	}
}

sub send_data
{
	my ($request,$host,$port) = @_;

	my @request;

	if($request eq '--stdin') {
		@request = <STDIN>;
	} else {
		open REQUEST, "<$request" or die "could not open '$request': $!";
		@request = <REQUEST>;
		close REQUEST;
	}

	my $request = join('', @request);
	if($format_request and %request_args) {
		print STDERR "formatting request...\n";
		$request = &format_xml($request,%request_args);
		print STDERR "formatted request\n";
	}

	if($print_request) {
		print "request: {{{$request}}}\n";
	}

	my $sock = new IO::Socket::INET(PeerAddr => $host, PeerPort => $port, Proto => 'tcp' ) or (print STDERR "Could not connect to $host:$port\n" and exit);

	my $command = 'RQST';
	if($compress) {
		$command = 'RQDF';
		print STDERR "compressing data before sending...\n";
		$request = &zip('compress',$request);
		print STDERR "data compressed\n";
	}

	my $header = pack('N',12 + length $request);
	my $data = "$header" . $command . "0000" . $request;

	print $sock $data;

	return $sock;
}

sub get_response
{
	my ($sock) = @_;

	my $result = '';
	while(my $line = <$sock>) {
		$result .= $line;
	}

	die "socket disconnected (server crashed?)\n" unless $result;

	close $sock or print STDERR "Could not close socket\n";

	my $command = substr $result, 4, 4;

	$result = substr $result, 12;
	if($command eq 'REDF') {
		$result = &zip('decompress',$result);
	}

	return substr $result, 0, ((length $result) - 1);
}

sub zip
{
	my $fname = "/tmp/$$.data";

	my ($operation,$data) = @_;

	open FILE, ">$fname";
	print FILE $data;
	close FILE;

	my $cmd = "./compress --$operation $fname $fname";
	system($cmd);
	open FILE, "<$fname" or die "Compression failed: $!";
	
	my $out = "";
	read FILE, $out, 1000000 or die "Compression failed: $!";
	close FILE;

	system("rm $fname");
	return $out;
}

