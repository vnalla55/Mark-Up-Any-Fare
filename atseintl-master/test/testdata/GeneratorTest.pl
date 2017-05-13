#!/usr/bin/perl

use strict;
use warnings;
use File::Basename qw(basename dirname);

use Gen::test::Test;

if (@ARGV == 0)
{
  print "Usage:\n  GeneratorTest.pl <test-name>+\n";
  exit(1);
}

if (@ARGV == 1 && $ARGV[0] =~ /^\s*all\s*$/i)
{
  my $dir = dirname(__FILE__);
  open my $fh, "find $dir/Gen/test -name *Test.pm|";
  while (my $path = <$fh>)
  {
    chomp $path;
    if (basename($path) =~ /^.+Test\.pm$/)
    {
      require $path;
    }
  }
  close $fh;
}
else
{
  foreach my $name (@ARGV)
  {
    require "Gen/test/${name}Test.pm";
  }
}

exit(runTests() ? 0 : 127);
