############################################### Field.pm Test

package Gen::test::FieldTest;
require Gen::Field;
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

sub test_Field_constructor_simple
{
  my $field = Gen::Field->new('int foo');

  assertStringEquals($field->name, 'foo');
  assertStringEquals($field->shortName, 'foo');
  assertStringEquals($field->type, Gen::Type->new('int'));
  assertStringEquals($field->nameWithType, 'int foo');

  assertWriterEquals($field->declaration, <<LINES
int foo;
LINES
  );
  assertWriterEquals($field->definition, <<LINES
int foo;

LINES
  );
}

sub test_Field_constructor_params
{
  my $field = Gen::Field->new({
    name => 'foo',
    modifiers => ['static'],
    section => 'private',
    type => 'long',
    initializer => '42L'
  });

  assert($field->hasModifier('static'));
  assert(!$field->hasModifier('mutable'));

  assertWriterEquals($field->declaration, <<LINES
static
long foo /* = 42L */;
LINES
  );
  assertWriterEquals($field->definition, <<LINES
/* private static */
long foo = 42L;

LINES
  );
}

sub test_Field_constructor_setters
{
  my $field = Gen::Field->new();
  $field
    ->setNameType('long foo')
    ->setSection('private')
    ->addModifier('static')
    ->setInitializer('42L');

  assertWriterEquals($field->declaration, <<LINES
static
long foo /* = 42L */;
LINES
  );
  assertWriterEquals($field->definition, <<LINES
/* private static */
long foo = 42L;

LINES
  );
}

sub test_Field_nameComplex
{
  my $field = Gen::Field->new('int Foo::Bar::value');

  assertStringEquals($field->name, 'Foo::Bar::value');
  assertStringEquals($field->shortName, 'value');
  assertStringEquals($field->nameWithType, 'int Foo::Bar::value');
  assertStringEquals($field->shortNameWithType, 'int value');

  assertWriterEquals($field->declaration, <<LINES
int value;
LINES
  );
  assertWriterEquals($field->definition, <<LINES
int Foo::Bar::value;

LINES
  );
}

sub test_Field_setSkipCallback
{
  my $field = Gen::Field->new('int foo');
  $field->setSkipCallback(sub { return 1; });

  assertWriterEquals($field->declaration, '');
  assertWriterEquals($field->definition, '');
}

registerTest('Gen::test::FieldTest');
