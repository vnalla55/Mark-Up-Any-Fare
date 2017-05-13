############################################### Class.pm Test

package Gen::test::ClassTest;
require Gen::Class;
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

sub test_Class_constructor_simple
{
  my $class = Gen::Class->new('Foo');

  assertStringEquals($class->name, 'Foo');
  assertWriterEquals($class->declaration, <<LINES
class Foo
{
};

LINES
  );
  assertWriterEquals($class->definition, '');
}

sub test_Class_constructor_setters
{
  my $class = Gen::Class->new('Foo');
  $class->setName('Bar');

  assertStringEquals($class->name, 'Bar');
  assertWriterEquals($class->declaration, <<LINES
class Bar
{
};

LINES
  );
  assertWriterEquals($class->definition, '');
}

sub test_Class_setSkipCallback
{
  my $class = Gen::Class->new('FooBar');
  $class->setSkipCallback(sub { return 1; });

  assertWriterEquals($class->declaration, '');
  assertWriterEquals($class->definition, '');
}

sub test_Class_Section_addFunction
{
  my $class = Gen::Class->new('Class');
  $class->public->addFunction({
    modifiers => ['const'],
    signature => 'void empty(const int& argument = 7)'
  });
  $class->public->addFunction(my $func = Gen::Function->new({
    modifiers => ['static'],
    signature => 'int withBody(int a, int& b)'
  }));
  $func->body->writeCode('return (b = a + 1);');

  assertWriterEquals($class->declaration, <<LINES
class Class
{
public:
  void
  empty(const int& argument = 7) const;

  static int
  withBody(int a, int& b);
};

LINES
  );
  assertWriterEquals($class->definition, <<LINES
/* public */ void
Class::empty(const int& argument /* = 7 */) const
{
}

/* public static */ int
Class::withBody(int a, int& b)
{
  return (b = a + 1);
}

LINES
  );
}

sub test_Class_Section_addFunction_constructor
{
  my $class = Gen::Class->new('Class');
  $class->public->addFunction(my $func = Gen::Function->new({
    arguments => ['int a']
  }));
  $func->body->writeCode('(void)a;');

  assertWriterEquals($class->declaration, <<LINES
class Class
{
public:
  Class(int a);
};

LINES
  );
  assertWriterEquals($class->definition, <<LINES
/* public */
Class::Class(int a)
{
  (void)a;
}

LINES
  );
}

sub test_Class_Section_addFunction_skipBody
{
  my $class = Gen::Class->new('Class');
  $class->private->addFunction({
    signature => 'void withoutBody()'
  }, 1);

  assertWriterEquals($class->declaration, <<LINES
class Class
{
private:
  void
  withoutBody();
};

LINES
  );
  assertWriterEquals($class->definition, '');
}

sub test_Class_Section_addField
{
  my $class = Gen::Class->new('Class');
  $class->public->addField({
    modifiers => ['static'],
    nameType => 'int foo',
    initializer => '42'
  });
  $class->public->addField(Gen::Field->new({
    nameType => 'double number'
  }));

  assertWriterEquals($class->declaration, <<LINES
class Class
{
public:
  static
  int foo /* = 42 */;
  double number;
};

LINES
  );
  assertWriterEquals($class->definition, <<LINES
/* public static */
int Class::foo = 42;

LINES
  );
}

sub test_Class_multipleSections
{
  my $class = Gen::Class->new('Class');
  $class->private->addField({
    modifiers => ['static'],
    nameType => 'int foo',
    initializer => '42'
  });
  $class->public->addFunction(my $func = Gen::Function->new({
    signature => 'void Foo()'
  }));
  $func->body->writeCode('Class::foo = 1337 + Class::Bar();');
  $class->private->addFunction($func = Gen::Function->new({
    modifiers => ['static'],
    signature => 'int Bar()'
  }));
  $func->body->writeCode('return 7;');
  $class->protected->addFunction({
    modifiers => ['virtual'],
    signature => 'void nothing()'
  });
  $class->public->addFunction({}, 1);

  assertWriterEquals($class->declaration, <<LINES
class Class
{
public:
  Class();

  void
  Foo();

protected:
  virtual void
  nothing();

private:
  static int
  Bar();

  static
  int foo /* = 42 */;
};

LINES
  );
  assertWriterEquals($class->definition, <<LINES
/* public */ void
Class::Foo()
{
  Class::foo = 1337 + Class::Bar();
}

/* protected virtual */ void
Class::nothing()
{
}

/* private static */ int
Class::Bar()
{
  return 7;
}

/* private static */
int Class::foo = 42;

LINES
  );
}

registerTest('Gen::test::ClassTest');
