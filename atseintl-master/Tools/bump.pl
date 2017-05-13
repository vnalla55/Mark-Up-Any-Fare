#!/bin/env perl

use strict;
use warnings;

use threads;
use threads::shared;

use Carp;
use Getopt::Long;
use Pod::Usage;
use File::Basename;
use Cwd 'abs_path';

my $VOB_DIR;
BEGIN { $VOB_DIR=abs_path(dirname(__FILE__) . '/..'); }
use lib "$VOB_DIR/Tools/PerlModules";
use lib './PerlModules'; # for dev/local copy

use Sabre::Bump;
use Sabre::ShoppingProto;

our $opt_host           = 'pifhli102';
our $opt_port           = 53601;
our $opt_target;
our $opt_req_file       = 'tserequest.log';
our $opt_bps            = 10;                 #TODO make fractional
our $opt_max_requests   = 100;
our $opt_transaction_id = 9999;
our $opt_write_response = 0;
our $opt_write_header   = 0;
our $opt_help           = 0;
our $opt_immediately    = 0;
our $opt_timeout        = 0;
our $opt_impl           = 2;

our $usage = <<ENDUSAGE;

bump.pl [options]

  Options:
  
    --host            bump host
    --port            usually, 53601
    
      or
    --target          host:port
    
    --req-file        file to read reaquests from, default is "tserequest.log" in current directory
    
    --bps             
    --tps             amount of requests to be sent per second, --tps 0 means to send 
                      the next request immediately after finishing receiving response from the 
                      previous one
    
    --max-requests    stop sending requests after reaching specific requests number
    
    --tid
    --transaction-id  transaction id
    
    --write-response  write response to STDOUT
    --write-header    write response header to STDERR
    
     -i
    --immediately     skip wait 5 sec before start
    
    --timeout         is used to warm up
    
    --help            usage info 

ENDUSAGE

# globals
our @requests_arr :shared; # array of strings, shared for less memory footprint

sub target() { return $opt_target || "$opt_host:$opt_port"; }

sub parse_tserequestlog_into_requests_arr($) {
  
  my $tserequestlog_path = shift;
  my @result;

  open(TSEREQUEST_LOG, "<", $tserequestlog_path) 
    or 
    die "$tserequestlog_path: Oops - $!\n";
    
  print STDERR "parsing $tserequestlog_path...\n";
    
  my $file_content;
  read(TSEREQUEST_LOG, $file_content, -s TSEREQUEST_LOG) == -s TSEREQUEST_LOG
    or
    confess "Oops: incomplete read for $tserequestlog_path";
    
  @requests_arr = ($file_content =~ m{(<ShoppingRequest .+?</ShoppingRequest>)+}sg)
    or 
    confess "Oops: cannot parse $tserequestlog_path";
}

{
my $req_iter :shared = 0;
  
# thread-safe
sub bump_foo() {
  
  # executed once on first call
  do {
    lock(@requests_arr);
    parse_tserequestlog_into_requests_arr($opt_req_file) unless( @requests_arr );
  };

  my $sess = Sabre::ShoppingProto->new( target(), 
                                      { -write_header => $opt_write_header });
  
  my $req = do{ lock($req_iter); 
                $requests_arr[ $req_iter++ % @requests_arr ] };
  $sess->setRequestBody($req,
                        -client_tid => $opt_transaction_id,
                        -timeout => $opt_timeout);
  
  my    $response = $sess->send();
  print $response if $opt_write_response;
}}

if( !GetOptions( qw/host=s port=i 
                    target=s
                    req-file=s 
                    bps|tps=i 
                    max-requests=i
                    transaction-id|tid=i
                    write-response!
                    write-header!
                    help|?|usage
                    immediately
                    timeout=i
                    impl=i/ )
    ||
    $opt_help )
{
  print $usage;
  exit;
}

#parse_tserequestlog_into_requests_arr($opt_req_file);

printf STDERR <<EOM
TARGET         : %s
REQUEST FILE   : %s
TPS            : %s
MAX REQUESTS   : %s
TRANSACTION ID : %s
WRITE RESPONSE : %s
WRITE HEADERS  : %s
%s
EOM
  , target()
  , $opt_req_file
  , $opt_bps
  , $opt_max_requests
  , $opt_transaction_id
  , $opt_write_response ? 'YES' : 'NO'
  , $opt_write_header   ? 'YES' : 'NO'
  , $opt_timeout        ? "TIMEOUT        : $opt_timeout\n" : ''
  ;
if( !$opt_immediately ) {
  print STDERR "bump will start in a few sec, press CTRL+C to cancel...\n";
  sleep(5);
}  

bump('::bump_foo', $opt_bps, $opt_max_requests, $opt_impl);
print STDERR "finished.\n";


