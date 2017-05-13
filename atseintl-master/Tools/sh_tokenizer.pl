#!/usr/bin/perl
#------------------------------------------------------------------------------
#
#  File:    sh_tokenize.pl (PERL script)
#  Author:  John Watilo
#  Created: November 2007
#  Description:
#           This perl script is used by shell scripts to tokenize strings.
#
#           Syntax of use:
#                sh_tokenize.pl <delimiters> <string>
#
#           Tokens are returned in stdout, one line per token.
#
#           Shell Example:
#                for token in $(sh_tokenize '\s+|:|;' "one two:three four;five") ; do
#                  echo ${token}
#                done
#
#
#  Copyright (c) SABRE 2007
#
#  The copyright to the computer program(s) herein is the property of SABRE.
#  The program(s) may be used and/or copied only with the written permission
#  of SABRE or in accordance with the terms and conditions stipulated in the
#  agreement/contract under which the program(s) have been supplied.
#
#------------------------------------------------------------------------------

use strict ;
use Text::ParseWords ;

sub usage
{
  print "\n" ;
  print "Usage: $0 <delimiter> <string>" ;
  print "\n" ;
  exit 1
}

usage unless (scalar @ARGV > 1) ;

( my $delim, my $str ) = @ARGV ;

my @words = &parse_line( "[$delim]+", 0, $str ) ;

foreach (@words)
{
  print "$_\n" ;
}

exit 0 ;
