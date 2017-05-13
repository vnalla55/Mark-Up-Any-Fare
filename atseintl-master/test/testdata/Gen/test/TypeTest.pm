############################################### Type.pm Test

package Gen::test::TypeTest;
require Gen::Type;

use strict;
use warnings;
use Gen::test::Test;

sub test_Null
{
  my $type = Gen::Type::Null->new();

  assertStringEquals($type->fullName, '');
  assert($type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);
}

sub test_Normal
{
  my $type = Gen::Type::Normal->new('int');

  assertStringEquals($type->fullName, 'int');
  assertStringEquals($type->fullName('foo'), 'int foo');
  assert(!$type->isNull);
  assert($type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);
  assert(!$type->const);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);

  my $const = $type->withConst;

  assertEquals($const, Gen::Type::Normal->new('int', {const => 1}));
  assert($const->isNormal);
  assert($const->const);
  assertStringEquals($const->fullName, 'const int');
  assertStringEquals($const->fullName('foo'), 'const int foo');
}

sub test_Normal_string
{
  my $type = Gen::Type::Normal->new('string');

  assertStringEquals($type->fullName, 'std::string');
}

sub test_Pointer
{
  my $base = Gen::Type::Normal->new('int');
  my $type = $base->pointer();

  assertEquals($type->base, $base);
  assertStringEquals($type->fullName, 'int*');
  assertStringEquals($type->fullName('foo'), 'int* foo');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert($type->isPtrRef);
  assert($type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);
  assert(!$type->const);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);

  my $const = $type->withConst;

  assertEquals($const, Gen::Type::Pointer->new($base, {const => 1}));
  assert($const->isPointer);
  assert($const->const);
  assertStringEquals($const->fullName, 'int* const');
  assertStringEquals($const->fullName('foo'), 'int* const foo');
}

sub test_Pointer_2const
{
  my $base = Gen::Type::Normal->new('int', 1);
  my $type = $base->pointer(1);

  assert($base->const);
  assert($type->const);
  assertStringEquals($type->fullName, 'const int* const');
  assertStringEquals($type->fullName('foo'), 'const int* const foo');
  assertEquals($type->withConst, $type);

  my $withoutConst = $type->withoutConst;

  assert(!$withoutConst->const);
  assertEquals($withoutConst->base, $base);
  assertStringEquals($withoutConst->fullName, 'const int*');
  assertStringEquals($withoutConst->fullName('foo'), 'const int* foo');
}

sub test_Reference
{
  my $base = Gen::Type::Normal->new('int');
  my $type = $base->reference();

  assertEquals($type->base, $base);
  assertStringEquals($type->fullName, 'int&');
  assertStringEquals($type->fullName('foo'), 'int& foo');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert($type->isPtrRef);
  assert(!$type->isPointer);
  assert($type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);
  assert(!$type->const);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);

  my $const = $type->withConst;

  assertEquals($const, Gen::Type::Reference->new($base, {const => 1}));
  assert($const->isReference);
  assert($const->const);
  assertStringEquals($const->fullName, 'int& const');
  assertStringEquals($const->fullName('foo'), 'int& const foo');
}

sub test_Array
{
  my $base = Gen::Type::Normal->new('long');
  my $type = $base->array(42);

  assertEquals($type->base, $base);
  assertStringEquals($type->fullName, 'long[42]');
  assertStringEquals($type->fullName('foo'), 'long foo[42]');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert($type->isArray);
  assert(!$type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);
}

sub test_Array_0
{
  my $base = Gen::Type::Normal->new('long');
  my $type = $base->array(0);

  assertStringEquals($type->fullName, 'long[0]');
  assertStringEquals($type->fullName('foo'), 'long foo[0]');
}

sub test_Array_noSize
{
  my $base = Gen::Type::Normal->new('long');
  my $type = $base->array();

  assertStringEquals($type->fullName, 'long[]');
  assertStringEquals($type->fullName('foo'), 'long foo[]');
}

sub test_Function
{
  my $base = Gen::Type::Normal->new('long');
  my $type = $base->function('std::string', 'bool&');

  assertEquals($type->returnType, $base);
  assertEquals(scalar @{$type->arguments}, 2);
  assertStringEquals($type->fullName('foo'), 'long foo(std::string, bool&)');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert($type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);
}

sub test_Function_notNormalized
{
  my $base = Gen::Type::Normal->new('long');
  my $type = $base->function('std::string bar', 'bool&');

  assertEquals($type->returnType, $base);
  assertEquals(scalar @{$type->arguments}, 2);
  assertStringEquals($type->fullName('foo'), 'long foo(std::string bar, bool&)');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert($type->isFunction);
  assert(!$type->isTemplate);
  assert(!$type->isContainer);
  assert(!$type->isNormalized);
  assert(!$type->hasContainer);

  my $normalized = $type->normalized;

  assertEquals($normalized->returnType, $base);
  assertStringEquals($normalized->fullName('foo'), 'long foo(std::string, bool&)');
}

sub test_Template
{
  my $base = Gen::Type::Normal->new('std::pair');
  my $type = $base->template('int', 'long');

  assertStringEquals($type->name, 'std::pair');
  assertEquals(scalar @{$type->arguments}, 2);
  assertStringEquals($type->fullName('foo'), 'std::pair<int, long> foo');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert($type->isTemplate);
  assert(!$type->isContainer);
  assert($type->isNormalized);
  assert(!$type->hasContainer);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);

  my $const = $type->withConst;

  assertEquals($const, $base->withConst->template('int', 'long'));
  assertStringEquals($const->name, 'std::pair');
  assert($const->isTemplate);
  assert($const->const);
  assertStringEquals($const->fullName('foo'), 'const std::pair<int, long> foo');
}

sub test_Container
{
  my $base = Gen::Type::Normal->new('std::vector');
  my $type = $base->template('int');

  assertStringEquals($type->name, 'std::vector');
  assertEquals(scalar @{$type->arguments}, 1);
  assertStringEquals($type->fullName('foo'), 'std::vector<int> foo');
  assert(!$type->isNull);
  assert(!$type->isNormal);
  assert(!$type->isPtrRef);
  assert(!$type->isPointer);
  assert(!$type->isReference);
  assert(!$type->isArray);
  assert(!$type->isFunction);
  assert($type->isTemplate);
  assert($type->isContainer);
  assert($type->isNormalized);
  assert($type->hasContainer);

  assertEquals($type, $type->normalized);
  assertEquals($type, $type->withoutConst);
}

sub test_Complex
{
  my $void = Gen::Type::Normal->new('void');
  my $int = Gen::Type::Normal->new('int');
  my $long = Gen::Type::Normal->new('long');
  my $string = Gen::Type::Normal->new('string');

  my $func1 = $void->function($int->withConst->reference, $string->pointer->withConst);
  my $func2 = $long->function($long);
  my $cont = Gen::Type::Template->new('std::vector', $func2->pointer);

  my $func = $func1->pointer->function($cont->withConst->reference, $int);

  assertStringEquals($func->fullName('foo'),
    'void (* foo(const std::vector<long (*)(long)>&, int))(const int&, std::string* const)');
}

sub test_parse_void
{
  my ($type, $id) = Gen::Type::parse(' ');

  assert($type->isNull);
  assertStringEquals($id, '');
}

sub test_parse_normal
{
  my ($type, $id) = Gen::Type::parse('  int ');

  assert($type->isNormal);
  assertStringEquals($type->name, 'int');
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('  int ttest');

  assert($type->isNormal);
  assertStringEquals($type->name, 'int');
  assertStringEquals($id, 'ttest');
}

sub test_parse_pointer
{
  my ($type, $id) = Gen::Type::parse('  int * ');

  assert($type->isPointer);
  assertStringEquals($type->base->name, 'int');
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('const int * const ptest');

  assert($type->isPointer);
  assert($type->const);
  assert($type->base->const);
  assertStringEquals($type->base->name, 'int');
  assertStringEquals($id, 'ptest');
}

sub test_parse_array
{
  my ($type, $id) = Gen::Type::parse(' long [7] ');

  assert($type->isArray);
  assertStringEquals($type->base->name, 'long');
  assertEquals($type->size, 7);
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('const long foo[]');

  assert($type->isArray);
  assertEquals($type->size, -1);
  assertStringEquals($type->sizeString, '');
  assert($type->base->const);
  assertStringEquals($type->base->name, 'long');
  assertStringEquals($id, 'foo');
}

sub test_parse_function
{
  my ($type, $id) = Gen::Type::parse('void(int, std::string)');

  assert($type->isFunction);
  assertStringEquals($type->returnType->name, 'void');
  assertEquals(scalar @{$type->arguments}, 2);
  assertStringEquals($id, '');

  (my $type2, $id) = Gen::Type::parse('void bar(int, std::string)');

  assertEquals($type, $type2);
  assertStringEquals($id, 'bar');
}

sub test_parse_function_vs_grouping
{
  my ($type, $id) = Gen::Type::parse('int (*[])');

  assert($type->isArray);
  assert($type->base->isPointer);
  assert($type->base->base->isNormal);
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('int (f[])');

  assert($type->isFunction);
  assert($type->returnType->isNormal);
  assert(${$type->arguments}[0]->isArray);
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('int f(c*[])');

  assert($type->isFunction);
  assert($type->returnType->isNormal);
  assert(${$type->arguments}[0]->isArray);
  assert(${$type->arguments}[0]->base->isPointer);
  assertStringEquals($id, 'f');

  assertError(sub { Gen::Type::parse('int f(*[])'); });
}

sub test_parse_template
{
  my ($type, $id) = Gen::Type::parse('std::vector<int>');

  assert($type->isTemplate);
  assert($type->isContainer);
  assertStringEquals($type->name, 'std::vector');
  assertEquals(scalar @{$type->arguments}, 1);
  assert(!${$type->arguments}[0]->const);
  assertStringEquals($id, '');

  ($type, $id) = Gen::Type::parse('std::vector<const int> a');

  assert($type->isTemplate);
  assert($type->isContainer);
  assertStringEquals($type->name, 'std::vector');
  assertEquals(scalar @{$type->arguments}, 1);
  assert(${$type->arguments}[0]->const);
  assertStringEquals($id, 'a');
}

sub test_parse_complex
{
  my ($type, $id) = Gen::Type::parse('std::list<void (* const)(int, long a)>& bar(void (*)())');

  assert($type->isFunction);
  assert(${$type->arguments}[0]->isPointer);
  assert(${$type->arguments}[0]->base->isFunction);
  assert(${$type->arguments}[0]->base->isNormalized);
  assert($type->returnType->isReference);
  assert($type->returnType->base->isContainer);
  assert($type->returnType->base->base->isPointer);
  assert($type->returnType->base->base->const);
  assert($type->returnType->base->base->base->isFunction);
  assert(!$type->returnType->base->base->base->isNormalized);
  assert(!$type->isNormalized);
  assertStringEquals($id, 'bar');

  my $normalized = $type->normalized;
  assert($normalized != $type);
  assert($normalized->isNormalized);
  assertEquals(${$normalized->arguments}[0], ${$type->arguments}[0]);
}

sub test_parse_fullName
{
  my $type = Gen::Type->new('std::vector<int> a(int foo)');
  my $type2 = Gen::Type->new($type->fullName);

  assertEquals($type2, $type);

  $type = Gen::Type->new('std::list<void (* const)(int, long a)>& bar(void (*)())');
  $type2 = Gen::Type->new($type->fullName);

  assertEquals($type2, $type);
}

sub test_hasContainer_positive
{
  my $type = Gen::Type->new('std::vector<int>*const &');

  assert($type->hasContainer);
}

sub test_hasContainer_negative
{
  my $type = Gen::Type->new('std::basic_string<char>*const &');

  assert(!$type->hasContainer);
}

sub test_declaration_simple
{
  my ($type, $id) = Gen::Type::parse('const int foo');

  assertStringEquals($type->declaration($id), 'const int foo');
}

sub test_declaration_pointer_noInit
{
  my ($type, $id) = Gen::Type::parse('const int* foo');

  assertStringEquals($type->declaration($id), 'const int* foo');
}

sub test_declaration_pointer_init
{
  my ($type, $id) = Gen::Type::parse('int*const foo');

  assertStringEquals($type->declaration($id, 1), 'int* const foo = new int');
}

sub test_innerMostType_noop
{
  my $type = Gen::Type->new('int');

  assertEquals($type->innerMostType, $type);
}

sub test_innerMostType_one
{
  my $type = Gen::Type->new('int*');

  assertEquals($type->innerMostType, $type->base);
}

sub test_innerMostType_complex
{
  my $type = Gen::Type->new('std::map<std::string, const int*const>& *const');

  assertEquals($type->innerMostType, Gen::Type::Normal->new('int', {const => 1}));
}

sub test_insertInstruction
{
  my $type = Gen::Type->new('std::set<const int*const>');

  assertStringEquals($type->insertInstruction('foo'), 'insert(foo)');

  $type = Gen::Type->new('std::vector<long>');

  assertStringEquals($type->insertInstruction('foo'), 'push_back(foo)');
}

registerTest('Gen::test::TypeTest');
