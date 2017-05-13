############################################### Test Framework

package Gen::test::Test;
use strict;
use warnings;
use Exporter 'import';

use Gen::Util qw(error);

our @EXPORT = qw(runTests registerTest assert assertEquals assertStringEquals assertRegExp
                 assertIsUndef assertError);

our $showDiff = 1;

my %Colors = (
  'green' => 32,
  'red'   => 31
);

sub beginColor
{
  my ($color) = @_;

  my $c = $Colors{$color} || 0;

  return "\e[${c}m";
}

sub endColor
{
  return beginColor('');
}


my %Tests = ();
my $raisedAssert = 0;

sub runTests
{
  my @modules = keys %Tests;

  my $totalCount = 0;
  my $totalPassed = 0;

  foreach my $module (@modules)
  {
    my $testModule_ = $Tests{$module};
    my @testModule = sort { $$a[0] cmp $$b[0] } @$testModule_;

    my $testCount = @testModule;
    my $passed = 0;
    my @messages = ();

    print "Testing $module...\n\n";

    foreach my $test (@testModule)
    {
      my ($name, $sub) = @$test;
      print $module.': '.$name.': ';

      $raisedAssert = 0;
      my $oldHandler = Gen::Util::errorHandler;
      eval
      {
        $sub->();
      };
      if ($@)
      {
        print beginColor('red').($raisedAssert ? 'ASSERT' : 'ERROR').endColor()."\n";
        push @messages, [$name, $@];
      }
      else
      {
        print beginColor('green').'OK'.endColor()."\n";
        ++$passed;
      }
      Gen::Util::registerErrorHandler($oldHandler);
    }

    my $result = ($passed == $testCount ?
      beginColor('green').'PASSED'.endColor() :
      beginColor('red').'FAILED'.endColor());
    print "\n$module done: $result ($passed/$testCount passed)\n";

    if (@messages != 0)
    {
      print "\nAsserts:\n";
      foreach my $message (@messages)
      {
        print $$message[0].': '.$$message[1];
      }
      print "\n";
    }

    $totalCount += $testCount;
    $totalPassed += $passed;
  }

  my $result = ($totalPassed == $totalCount ?
      beginColor('green').'PASSED'.endColor() :
      beginColor('red').'FAILED'.endColor());
  print "\nTotal: $result ($totalPassed/$totalCount passed)\n";

  return ($totalPassed == $totalCount);
}

sub registerTest
{
  my ($module) = @_;

  my $testModule = $Tests{$module} = [];
  {
    no strict 'refs';
    foreach my $entry (keys %{"${module}::"})
    {
      if (defined &{"${module}::$entry"} && $entry =~ /^test/i)
      {
        my $test = [$entry, \&{"${module}::$entry"}];
        push @$testModule, $test;
      }
    }
  }

  return 1;
}

sub assert
{
  my ($expr, $message) = @_;
  return if $expr;

  $message = '' if !defined $message;
  $message = ": $message" if $message ne '';

  my $i = 0;
  my ($filename, $line);
  while (1)
  {
    (undef, $filename, $line) = caller($i++);
    last if $filename !~ /\/Test\.pm$/;
  }

  $raisedAssert = 1;
  error("Assertion failed$message at $filename:$line");
}

sub assertEquals
{
  my ($left, $right) = @_;
  assert($left == $right, "$left != $right");
}

sub assertStringEquals
{
  my ($left, $right) = @_;

  if ("${left}${right}" =~ /\n/ && $left ne $right && $showDiff)
  {
    my $fh;
    my $lName = "/tmp/GeneratorTest.$$.left";
    my $rName = "/tmp/GeneratorTest.$$.right";
    open $fh, '>', $lName; print $fh $left; close $fh;
    open $fh, '>', $rName; print $fh $right; close $fh;
    open $fh, "diff -u --label left $lName --label right $rName|";
    my $diff;
    while (<$fh>)
    {
      $diff .= $_;
    }
    close($fh);
    unlink($lName);
    unlink($rName);
    assert($left eq $right, "\n$diff");
  }

  my $msg = "'${left}' != '${right}'";
  $msg = "\n${left}\n          !=\n${right}" if "${left}${right}" =~ /\n/;
  assert($left eq $right, "$msg");
}

sub assertRegExp
{
  my ($string, $regExp) = @_;
  assert(scalar $string =~ m/$regExp/, "'${string}' !~ $regExp");
}

sub assertIsUndef
{
  my ($expr) = @_;
  assert(!defined $expr, (defined $expr ? "'${expr}' isn't undef" : ''));
}

sub assertError
{
  my ($func, $expectedError, $re) = @_;

  $@ = '';
  eval
  {
    $func->();
  };
  if (!defined $expectedError)
  {
    assert($@ ne '', "No error detected");
  }
  elsif ($expectedError =~ /^no$/i)
  {
    assert($@ eq '', "Error detected");
  }
  elsif ($expectedError =~ /^re$/i)
  {
    assert(scalar $@ =~ m/$re/, "Error message '$@' !~ $re");
  }
  else
  {
    assert($@ eq $expectedError, "Error message '$@' != '${expectedError}'");
  }
}

1;
