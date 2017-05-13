#!/usr/bin/perl
#------------------------------------------------------------------------------
#
#  File:    bigip.pl (PERL script)
#  Author:  Reginaldo Costa
#  Created: 08/21/2006
#  Description:
#           This perl script is able to call any of BIGIP.pm methods; It uses
#           the iControl interface to BIGIP box defined as its 1st argument.
#
#           Syntax of use:
#                bigip.pl [-v | -d] <host> <method> <parameters separated by space>
#
#           If -v option is given, diagnostics messages are writen to STDERR.
#           If -d option is given, diagnostics messages are writen to STDOUT.
#           Data is returned as a list on STDOUT if not -d.
#           Error code is returned on $? variable.
#
#           Example:
#                timeout=$(bigip.pl redball.dev.sabre.com getMonitorTimeout SNPMONITOR)
#                echo $timeout
#                20
#
#
#  Copyright (c) SABRE 2006
#
#  The copyright to the computer program(s) herein is the property of SABRE.
#  The program(s) may be used and/or copied only with the written permission
#  of SABRE or in accordance with the terms and conditions stipulated in the
#  agreement/contract under which the program(s) have been supplied.
#
#------------------------------------------------------------------------------

sub usage;

use lib qw( . ../perlpm );
use strict;
use BIGIP;

my @results = qw();
my $verbose = 0;
my $display = 0;

my $printit = \&BIGIP::printm;
if ($ARGV[0] eq '-v')
{
     $verbose = 1;
     shift;
};

if ($ARGV[0] eq '-d')
{
     $display = 1;
     $verbose = 1;
     $printit = \&BIGIP::prints;
     shift;
};

usage unless (scalar @ARGV > 1);
(my $host, my $function, my @parameters) = @ARGV;

if (UNIVERSAL::can('BIGIP',$function))
{
    my $box = new BIGIP($host);
    $box->verbose($verbose);      # Tell how verbose we are
    my $call = '$box->' . $function . '(' . "'" . join("','",@parameters) . "'" . ')';
    &$printit("iControl Call\: %s\n", $call) if $verbose;
    eval '@results = ' . $call;
    if ($verbose)
    {
        for (my $i=0; $i < scalar @results; $i++) {
             chomp(my $line = $results[$i]);
             &$printit("[%03d] - %s\n", $i, $line);
        };
        &$printit("iControl Errorcode = %s\n", $box->errorcode());
    };
    print join(' ',@results) unless $display;
    exit $box->errorcode();
}
else
{
     my $msg = "iControl Error: no method $function found on BIGIP package.\n";
     &$printit("$msg") if $verbose;
     print '';
     exit 99;
};


sub usage {

    print STDERR << "EOF!";

    $0 Help

     This perl script is able to call any of BIGIP.pm methods; It uses
     the iControl interface to BIGIP box defined as its 1st argument.

     Syntax of use:
        bigip.pl [-v | -d ] [<user>[:<pass>]@]<host> <method> <parameters>

     If -v options is given, diagnostics and results are writen to STDERR.
     If -d option is given, diagnostics messages are writen to STDOUT.
     Data is returned as a list on STDOUT, except for -d.
     Error code is returned on \$? variable.

     Example:
          timeout=\$(bigip.pl redball.dev.sabre.com getMonitorTimeout SNPMONITOR)
          echo \$timeout
          20

EOF!

exit 8
};


