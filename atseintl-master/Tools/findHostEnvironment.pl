#!/usr/bin/perl
use strict vars;
use warnings;

if (! -r "groupmembers.pl" ) {
  print "Importing AppConsole data...";
  system "./appconsoleimport.pl > /dev/null";
  print " done.\n";
}

if (-M "groupmembers.pl" > 1) {
  print "Importing AppConsole data...";
  system "./appconsoleimport.pl > /dev/null";
  print " done.\n";
}
require "groupmembers.pl";

while ($_ = shift @ARGV)
{
   print hostEnv($_), "\n";
}
