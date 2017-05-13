############################################### Function.pm Test

package Gen::test::FunctionTest;
require Gen::Function;
require Gen::Type;

use strict;
use warnings;
use Gen::test::Test;

sub assertWriterEquals
{
  my ($writer, $string) = @_;

  my $result = $writer->saveToString();
  assertStringEquals($result, $string);
}

sub test_Function_constructor_simple
{
  my $function = Gen::Function->new('int foo()');

  assertStringEquals($function->name, 'foo');
  assertStringEquals($function->shortName, 'foo');
  assertStringEquals($function->returnType, Gen::Type->new('int'));
  assertEquals(scalar @{$function->arguments}, 0);
  assertEquals($function->signature, Gen::Type->new('int foo()'));

  assertWriterEquals($function->declaration, <<LINES
int
foo();

LINES
  );
  assertWriterEquals($function->definition, <<LINES
int
foo()
{
}

LINES
  );
}

sub test_Function_constructor_params
{
  my $function = Gen::Function->new({
    name => 'foo',
    modifiers => ['static'],
    section => 'private',
    returnType => 'void',
    arguments => [['long arg', '42L']]
  });

  assert($function->hasModifier('static'));
  assert(!$function->hasModifier('const'));

  assertWriterEquals($function->declaration, <<LINES
static void
foo(long arg = 42L);

LINES
  );
  assertWriterEquals($function->definition, <<LINES
/* private static */ void
foo(long arg /* = 42L */)
{
}

LINES
  );
}

sub test_Function_constructor_setters
{
  my $function = Gen::Function->new();
  $function
    ->setSignature('void foo(long arg = 42L)')
    ->addArgument(Gen::Type->new('string'), 's', '""')
    ->setSection('private')
    ->addModifier('static');

  assertWriterEquals($function->declaration, <<LINES
static void
foo(long arg = 42L, std::string s = "");

LINES
  );
  assertWriterEquals($function->definition, <<LINES
/* private static */ void
foo(long arg /* = 42L */, std::string s /* = "" */)
{
}

LINES
  );
}

sub test_Function_nameComplex
{
  my $function = Gen::Function->new('int Foo::Bar::value()');

  assertStringEquals($function->name, 'Foo::Bar::value');
  assertStringEquals($function->shortName, 'value');

  assertWriterEquals($function->declaration, <<LINES
int
value();

LINES
  );
  assertWriterEquals($function->definition, <<LINES
int
Foo::Bar::value()
{
}

LINES
  );
}

sub test_Function_setSkipCallback
{
  my $function = Gen::Function->new('int foo()');
  $function->setSkipCallback(sub { return 1; });

  assertWriterEquals($function->declaration, '');
  assertWriterEquals($function->definition, '');
}

sub test_Function_body
{
  my $function = Gen::Function->new('int foo()');
  $function->body->writeLine('body')->setEndNewLine(1);

  assertWriterEquals($function->declaration, <<LINES
int
foo();

LINES
  );
  assertWriterEquals($function->definition, <<LINES
int
foo()
{
  body
}

LINES
  );
}

registerTest('Gen::test::FunctionTest');
