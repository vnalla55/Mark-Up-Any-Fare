#!/usr/bin/perl

use strict;
use warnings;

use v5.8.8;
use Fatal qw(pipe open close);

use IO::Handle;

my $catch_prase         = 'TseServer is running';
my $run_tseserver_cmd   = "@ARGV";

our $dbg = 0;

# It looks like tseserver can not be killed because of SIGHUP,
# but we'd better play in a safe way

pipe(my $parent_rdr, my $child_wtr);
$child_wtr->autoflush(1);
$parent_rdr->autoflush(1);

my  $pid = fork();
if ($pid == 0) {
  # child

  # don't forget to detach from STDIN
  open STDIN, "< /dev/null";
  # will log tseserver output for debug
  open TSESERVER_LOG, "> tseserver.log";

  STDOUT->autoflush(1);

  open(my $pipe, "-|", "$run_tseserver_cmd 2>&1");
  my $start_ok = 0;
  while(<$pipe>) {

    print TSESERVER_LOG;
    print;

    if (/$catch_prase/) {
      $start_ok = 1;
      last;
    }
  }

  if ($start_ok) {
    print $child_wtr "TSESERVER IS RUNNING\n";
    close $child_wtr;
  } else {
    print $child_wtr "SOMETHING WENT WRONG, TRY TO LOOK INTO tseserver.log FOR DETAILS\n";
    exit 1;
  }

  # detach from STDOUT, STDERR to properly daemonize (now we can die along with process group leader only)
  open STDOUT, "> /dev/null";
  open STDERR, ">& STDOUT";

  # infinite read & log
  print TSESERVER_LOG while(<$pipe>);
  exit;

} elsif ($pid) {
  # in parent

  my $child_says = <$parent_rdr>;
  print "parent: child said: " if $dbg;

  if ($child_says eq "TSESERVER IS RUNNING\n") {
    print $child_says if $dbg;
    exit 0; # tseserver started
  } else {
    print $child_says;
    exit 1; # tseserver failed
  }
}
