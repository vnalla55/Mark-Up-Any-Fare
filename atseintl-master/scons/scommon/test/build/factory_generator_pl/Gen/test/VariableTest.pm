############################################### Variable.pm Test

package Gen::test::VariableTest;
require Gen::Variable;

use strict;
use warnings;
use Gen::test::Test;

sub test_LocalVariable_constructor_simple
{
  my $variable = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('int')
  });

  assertStringEquals($variable->name, 'foo');
  assertStringEquals($variable->type->fullName, 'int');
  assert($variable->gettable);
  assert($variable->settable);
  assert($variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, 'foo');
  assertStringEquals($variable->getExpressionReference, 'foo');
  assertStringEquals($variable->getExpressionPointer, '&foo');
  assertStringEquals($variable->setInstruction('7'), 'foo = 7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo = *ptr');
  assertStringEquals($variable->declaration, 'int foo');
}

sub test_LocalVariable_constructor_setters
{
  my $variable = Gen::LocalVariable->new({
    name => 'foo',
    type => undef
  });
  $variable
    ->setName('bar')
    ->setType(Gen::Type->new('long')->pointer);

  assertStringEquals($variable->name, 'bar');
  assertStringEquals($variable->type->fullName, 'long*');
  assert($variable->gettable);
  assert($variable->settable);
  assert($variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'barPostfix');
  assertStringEquals($variable->getExpression, 'bar');
  assertStringEquals($variable->getExpressionReference, '*bar');
  assertStringEquals($variable->getExpressionPointer, 'bar');
  assertStringEquals($variable->setInstruction('ptr'), 'bar = ptr');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'bar = ptr');
  assertStringEquals($variable->declaration, 'long* bar');
}

sub test_CastedVariable_constructor_simple
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('int')
  });
  my $variable = Gen::CastedVariable->new({
    variable => $base,
    type => Gen::Type->new('unsigned')
  });

  assertEquals($variable->variable, $base);
  assertStringEquals($variable->name, 'foo');
  assertStringEquals($variable->type->fullName, 'unsigned');
  assert($variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, '(unsigned)foo');
  assertStringEquals($variable->setInstruction('7'), 'foo = (int)7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo = (int)*ptr');
}

sub test_CastedVariable_constructor_setters
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('int')
  });
  my $variable = Gen::CastedVariable->new({
    variable => undef,
    type => Gen::Type->new('unsigned')
  });
  $variable
    ->setVariable($base)
    ->setType(Gen::Type->new('long'));

  assertEquals($variable->variable, $base);
  assertStringEquals($variable->name, 'foo');
  assertStringEquals($variable->type->fullName, 'long');
  assert($variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, '(long)foo');
  assertStringEquals($variable->setInstruction('7'), 'foo = (int)7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo = (int)*ptr');
}

sub test_CastedVariable_pointer
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('int*')
  });
  my $variable = Gen::CastedVariable->new({
    variable => $base,
    type => Gen::Type->new('unsigned*')
  });

  assertEquals($variable->variable, $base);
  assertStringEquals($variable->name, 'foo');
  assertStringEquals($variable->type->fullName, 'unsigned*');
  assert($variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, '(unsigned*)foo');
  assertStringEquals($variable->getExpressionReference, '*(unsigned*)foo');
  assertStringEquals($variable->getExpressionPointer, '(unsigned*)foo');
  assertStringEquals($variable->setInstruction('ptr'), 'foo = (int*)ptr');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo = (int*)ptr');
}

sub test_Container_constructor_simple
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('std::vector<int>')
  });
  my $variable = Gen::Container->new($base);

  assertEquals($variable->container, $base);
  assertStringEquals($variable->name, 'fooContainer');
  assertStringEquals($variable->type->fullName, 'int');
  assert(!$variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->beginFunction, 'foo.begin()');
  assertStringEquals($variable->endFunction, 'foo.end()');
  assertStringEquals($variable->setInstruction('7'), 'foo.push_back(7)');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo.push_back(*ptr)');
}

sub test_Container_constructor_setters
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('std::vector<int>')
  });
  my $variable = Gen::Container->new();
  $variable->setContainer($base);

  assertEquals($variable->container, $base);
  assertStringEquals($variable->name, 'fooContainer');
  assertStringEquals($variable->type->fullName, 'int');
  assert(!$variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->beginFunction, 'foo.begin()');
  assertStringEquals($variable->endFunction, 'foo.end()');
  assertStringEquals($variable->setInstruction('7'), 'foo.push_back(7)');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo.push_back(*ptr)');
}

sub test_Container_pointer
{
  my $base = Gen::LocalVariable->new({
    name => 'foo',
    type => Gen::Type->new('std::vector<int*>*')
  });
  my $variable = Gen::Container->new($base);

  assertEquals($variable->container, $base);
  assertStringEquals($variable->name, 'fooContainer');
  assertStringEquals($variable->type->fullName, 'int*');
  assert(!$variable->gettable);
  assert($variable->settable);
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->beginFunction, 'foo->begin()');
  assertStringEquals($variable->endFunction, 'foo->end()');
  assertStringEquals($variable->setInstruction('ptr'), 'foo->push_back(ptr)');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'foo->push_back(ptr)');
}

sub test_Accessor_constructor_simple
{
  my $variable = Gen::Accessor->new({
    name => 'foo()',
    type => Gen::Type->new('int'),
    accessorType => $Gen::Accessor::ACCESSOR_INIT
  });

  assertEquals($variable->accessorType, $Gen::Accessor::ACCESSOR_INIT);
  assertStringEquals($variable->name, 'foo()');
  assertStringEquals($variable->type->fullName, 'int');
  assert($variable->gettable);
  assert($variable->settable);
  assert($variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, 'item->foo()');
  assertStringEquals($variable->getExpressionReference, 'item->foo()');
  assertStringEquals($variable->getExpressionPointer, '&item->foo()');
  assertStringEquals($variable->setInstruction('7'), 'item->foo() = 7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'item->foo() = *ptr');
}

sub test_Accessor_constructor_setters
{
  my $variable = Gen::Accessor->new({
    name => 'foo()',
    type => Gen::Type->new('int'),
    accessorType => $Gen::Accessor::ACCESSOR_INIT
  });
  $variable
    ->setName('bar()')
    ->setType(Gen::Type->new('long'))
    ->setAccessorType($Gen::Accessor::ACCESSOR_CONSTRUCT)
    ->setPrefix('item.');

  assertEquals($variable->accessorType, $Gen::Accessor::ACCESSOR_CONSTRUCT);
  assertStringEquals($variable->name, 'bar()');
  assertStringEquals($variable->type->fullName, 'long');
  assert($variable->gettable);
  assert($variable->settable);
  assert($variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'barPostfix');
  assertStringEquals($variable->getExpression, 'item.bar()');
  assertStringEquals($variable->getExpressionReference, 'item.bar()');
  assertStringEquals($variable->getExpressionPointer, '&item.bar()');
  assertStringEquals($variable->setInstruction('7'), 'item.bar() = 7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'item.bar() = *ptr');
}

sub test_Accessor_complex_postfixName
{
  my $variable = Gen::Accessor->new({
    name => 'foo[7]',
    type => Gen::Type->new('int')
  });

  assertStringEquals($variable->postfixName('Postfix'), 'foo_7_Postfix');
}

sub test_Accessor_getAccessor
{
  my $variable = Gen::Accessor->new({
    name => 'getFoo',
    type => Gen::Type->new('int')
  });

  assertStringEquals($variable->name, 'getFoo');
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, 'item->getFoo()');
  assertStringEquals($variable->setInstruction('7'), 'item->setFoo(7)');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'item->setFoo(*ptr)');
}

sub test_Accessor_setAccessor
{
  my $variable = Gen::Accessor->new({
    name => 'setFoo',
    type => Gen::Type->new('int')
  });

  assertStringEquals($variable->name, 'setFoo');
  assert(!$variable->isReference);
  assertStringEquals($variable->postfixName('Postfix'), 'fooPostfix');
  assertStringEquals($variable->getExpression, 'item->foo()');
  assertStringEquals($variable->setInstruction('7'), 'item->setFoo(7)');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'item->setFoo(*ptr)');
}

sub test_Accessor_clone
{
  my $base = Gen::Accessor->new({
    name => '_foo',
    type => Gen::Type->new('int')
  });
  my $variable = $base->clone(Gen::Type->new('long'));
  $base
    ->setName('getBar')
    ->setType(Gen::Type->new('std::vector<string>'));

  assertStringEquals($variable->name, '_foo');
  assertStringEquals($variable->type->fullName, 'long');
  assertStringEquals($variable->postfixName('Postfix'), '_fooPostfix');
  assertStringEquals($variable->getExpression, 'item->_foo');
  assertStringEquals($variable->setInstruction('7'), 'item->_foo = 7');
  assertStringEquals($variable->setInstructionPointer('ptr'), 'item->_foo = *ptr');
}

registerTest('Gen::test::VariableTest');
