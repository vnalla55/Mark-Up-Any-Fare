############################################### Util.pm Test

package Gen::test::UtilTest;
use Gen::Util qw(error substituteString substituteUnaryFunction extractParens extractArgument
                 wrapIn splitTypename isFactoryType);

use strict;
use warnings;
use Scalar::Util qw(reftype);
use Gen::test::Test;

##################################### Error helpers

sub test_error_works
{
  assertError(sub { error('message123'); }, 're', qr/message123/);
}

sub test_errorHandler_works
{
  my $newHandler = sub {};

  my $oldHandler = Gen::Util::registerErrorHandler($newHandler);
  my $returnedHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler($oldHandler);

  assertEquals($returnedHandler, $newHandler);
}

sub test_registerErrorHandler_returnsPreviousHandler
{
  my $newHandler = sub {};

  my $oldHandler = Gen::Util::registerErrorHandler($newHandler);
  assert((reftype($oldHandler) || '') eq 'CODE');

  my $returnedHandler = Gen::Util::registerErrorHandler($oldHandler);
  assertEquals($returnedHandler, $newHandler);
}

sub test_registerErrorHandler_works
{
  my $var = '';

  my $oldHandler = Gen::Util::registerErrorHandler(sub { $var = $_[0]; });
  error('message');
  Gen::Util::registerErrorHandler($oldHandler);

  assertStringEquals($var, 'message');
}

##################################### Substitution helpers

sub test_substituteString_noop
{
  my $str = substituteString('just a string', 0, 'error');

  assertStringEquals($str, 'just a string');
}

sub test_substituteString_noNr
{
  my $str = substituteString('just a string $1', 0, 'error');

  assertStringEquals($str, 'just a string $1');
}

sub test_substituteString_works
{
  my $str = substituteString('just $1 string $0', 1, 'a');

  assertStringEquals($str, 'just a string $0');
}

sub test_substituteUnaryFunction_noop
{
  my $str = substituteUnaryFunction('just a string', 'func', sub { return 'error'; });

  assertStringEquals($str, 'just a string');
}

sub test_substituteUnaryFunction_noFunc
{
  my $str = substituteUnaryFunction('just a string $func2(-)', 'func', sub { return 'error'; });

  assertStringEquals($str, 'just a string $func2(-)');
}

sub test_substituteUnaryFunction_works
{
  my $str = substituteUnaryFunction('just $func2(A) string $func(-)', 'func2', sub { return lc($_[0]); });

  assertStringEquals($str, 'just a string $func(-)');
}

##################################### String helpers

sub test_extractParens_simple
{
  my ($inside, $outside) = extractParens('( test )');

  assertStringEquals($inside, ' test ');
  assertStringEquals($outside, '');
}

sub test_extractParens_simpleWithOutside
{
  my ($inside, $outside) = extractParens('( test ) string');

  assertStringEquals($inside, ' test ');
  assertStringEquals($outside, ' string');
}

sub test_extractParens_template
{
  my ($inside, $outside) = extractParens('<std::string, int>');

  assertStringEquals($inside, 'std::string, int');
  assertStringEquals($outside, '');
}

sub test_extractParens_recursive
{
  my ($inside, $outside) = extractParens('(a(test)b)c');

  assertStringEquals($inside, 'a(test)b');
  assertStringEquals($outside, 'c');
}

sub test_extractParens_recursiveTemplate
{
  my ($inside, $outside) = extractParens('(<test>)');

  assertStringEquals($inside, '<test>');
  assertStringEquals($outside, '');
}

sub test_extractParens_double
{
  my ($inside, $outside) = extractParens('(test)(test2)');

  assertStringEquals($inside, 'test');
  assertStringEquals($outside, '(test2)');
}

sub test_extractParens_tooMany_ok
{
  my ($inside, $outside) = extractParens('(a))');

  assertStringEquals($inside, 'a');
  assertStringEquals($outside, ')');
}

sub test_extractParens_noEnd_error
{
  assertError(sub { extractParens('(test'); });
}

sub test_extractParens_wrongEnd_error
{
  assertError(sub { extractParens('(test>'); });
}

sub test_extractParens_tooFew_error
{
  assertError(sub { extractParens('(a(b)'); });
}

sub test_extractParens_noMatching_error
{
  assertError(sub { extractParens('(aaa<bbb)ccc>ddd'); });
}

sub test_extractArgument_noop
{
  my ($arg, $rest) = extractArgument('test');

  assertStringEquals($arg, 'test');
  assertStringEquals($rest, '');
}

sub test_extractArgument_simple
{
  my ($arg, $rest) = extractArgument('test, test2');

  assertStringEquals($arg, 'test');
  assertStringEquals($rest, ' test2');
}

sub test_extractArgument_many
{
  my ($arg, $rest) = extractArgument('test1, test2, test3');

  assertStringEquals($arg, 'test1');
  assertStringEquals($rest, ' test2, test3');

  ($arg, $rest) = extractArgument($rest);

  assertStringEquals($arg, ' test2');
  assertStringEquals($rest, ' test3');

  ($arg, $rest) = extractArgument($rest);

  assertStringEquals($arg, ' test3');
  assertStringEquals($rest, '');
}

sub test_extractArgument_complicated1
{
  my ($arg, $rest) = extractArgument('(a, b), c');

  assertStringEquals($arg, '(a, b)');
  assertStringEquals($rest, ' c');
}

sub test_extractArgument_complicated2
{
  my ($arg, $rest) = extractArgument('std::map<int, int>, double');

  assertStringEquals($arg, 'std::map<int, int>');
  assertStringEquals($rest, ' double');
}

sub test_wrapIn_noop
{
  my $str = wrapIn(0, 'error', 'test', 'error');

  assertStringEquals($str, 'test');
}

sub test_wrapIn_works
{
  my $str = wrapIn(1, '(', 'test', ')');

  assertStringEquals($str, '(test)');
}

##################################### Type helpers

sub test_splitTypename_noop
{
  my ($outerType, $actualType) = splitTypename('test');

  assertStringEquals($outerType, '');
  assertStringEquals($actualType, 'test');
}

sub test_splitTypename_two
{
  my ($outerType, $actualType) = splitTypename('std::test');

  assertStringEquals($outerType, 'std');
  assertStringEquals($actualType, 'test');
}

sub test_splitTypename_more
{
  my ($outerType, $actualType) = splitTypename('std::test::string');

  assertStringEquals($outerType, 'std::test');
  assertStringEquals($actualType, 'string');
}

sub test_isFactoryType_int
{
  my $result = isFactoryType('int');

  assert(!$result);
}

sub test_isFactoryType_long
{
  my $result = isFactoryType('long');

  assert(!$result);
}

sub test_isFactoryType_string
{
  my $result = isFactoryType('string');

  assert(!$result);
}

sub test_isFactoryType_TaxCode
{
  my $result = isFactoryType('TaxCode');

  assert(!$result);
}

sub test_isFactoryType_Code7
{
  my $result = isFactoryType('Code<7>');

  assert(!$result);
}

sub test_isFactoryType_FareMarket
{
  my $result = isFactoryType('FareMarket');

  assert($result);
}

sub test_isFactoryType_DateTime
{
  my $result = isFactoryType('DateTime');

  assert($result);
}

sub test_isFactoryType_PaxTypeFare
{
  my $result = isFactoryType('PaxTypeFare');

  assert($result);
}

registerTest('Gen::test::UtilTest');
