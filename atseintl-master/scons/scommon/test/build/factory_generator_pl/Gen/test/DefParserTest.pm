############################################### DefParser.pm Test

package Gen::test::DefParserTest;
require Gen::DefParser;

use strict;
use warnings;
use Scalar::Util qw(blessed reftype looks_like_number);
use Gen::test::Test;

sub configureDefParser
{
  my ($parser) = @_;
  my @log = ();

  $parser
    ->setInitDone(sub {
      my $args = $_[1];
      push @log, ['initDone', $parser->position, $args];
    })
    ->setSpecialAggregateBegin(sub {
      my $args = $_[1];
      push @log, ['specialAggregateBegin', $parser->position, $args];
    })
    ->setSpecialAggregateEnd(sub {
      my $args = $_[1];
      push @log, ['specialAggregateEnd', $parser->position, $args];
    })
    ->setSpecialAggregateRead(sub {
      my $args = $_[1];
      push @log, ['specialAggregateRead', $parser->position, $args];
    })
    ->setSpecialAggregateWrite(sub {
      my $args = $_[1];
      push @log, ['specialAggregateWrite', $parser->position, $args];
    })
    ->setSpecialSectionBegin(sub {
      my $args = $_[1];
      push @log, ['specialSectionBegin', $parser->position, $args];
    })
    ->setSpecialSectionEnd(sub {
      my $args = $_[1];
      push @log, ['specialSectionEnd', $parser->position, $args];
    })
    ->setSpecialSectionRead(sub {
      my $args = $_[1];
      push @log, ['specialSectionRead', $parser->position, $args];
    })
    ->setSpecialSectionWrite(sub {
      my $args = $_[1];
      push @log, ['specialSectionWrite', $parser->position, $args];
    })
    ->setSpecialField(sub {
      my $args = $_[1];
      push @log, ['specialField', $parser->position, $args];
    })
    ->setSectionBegin(sub {
      my $args = $_[1];
      push @log, ['sectionBegin', $parser->position, $args];
    })
    ->setSectionEnd(sub {
      my $args = $_[1];
      push @log, ['sectionEnd', $parser->position, $args];
    })
    ->setScalar(sub {
      my $args = $_[1];
      push @log, ['scalar', $parser->position, $args];
    })
    ->setScalarAggregate(sub {
      my $args = $_[1];
      push @log, ['scalarAggregate', $parser->position, $args];
    })
    ->setScalarCompare(sub {
      my $args = $_[1];
      push @log, ['scalarCompare', $parser->position, $args];
    })
    ->setFactoryAggregate(sub {
      my $args = $_[1];
      push @log, ['factoryAggregate', $parser->position, $args];
    })
    ->setFactoryCompare(sub {
      my $args = $_[1];
      push @log, ['factoryCompare', $parser->position, $args];
    })
    ->setPreInit(sub {
      my $args = $_[1];
      push @log, ['preInit', $parser->position, $args];
    })
    ->setPostInit(sub {
      my $args = $_[1];
      push @log, ['postInit', $parser->position, $args];
    })
    ->setCheckItem(sub {
      my $args = $_[1];
      push @log, ['checkItem', $parser->position, $args];
    })
    ->setInclude(sub {
      my $args = $_[1];
      push @log, ['include', $parser->position, $args];
    })
    ->setNameValue(sub {
      my $args = $_[1];
      push @log, ['nameValue', $parser->position, $args];
    })
    ->setNameField(sub {
      my $args = $_[1];
      push @log, ['nameField', $parser->position, $args];
    });

  return \@log;
}

sub toString
{
  my ($element) = @_;
  my $type = reftype($element) || '';

  if (blessed($element))
  {
    if ($element->isa('Gen::Type'))
    {
      return 'Type('.$element->fullName.')';
    }
  }

  if ($type eq 'ARRAY')
  {
    my @array = map { toString($_) } @$element;
    return '['.join((@array <= 1 ? ', ' : ",\n  "), @array).']';
  }

  if ($type eq 'HASH')
  {
    my @keys = sort keys %$element;
    my @array = map { $_.' => '.toString($$element{$_}) } @keys;
    return '{'.join((@array <= 1 ? ', ' : ",\n  "), @array).'}';
  }

  if (!defined $element)
  {
    return 'undef';
  }

  if (looks_like_number($element))
  {
    return $element;
  }

  return "'${element}'";
}

sub logToString
{
  my ($log) = @_;
  my $result = '';

  foreach my $event (@$log)
  {
    $result .= toString($event)."\n";
  }

  return $result;
}

sub assertLogEquals
{
  my ($left, $right) = @_;

  my $leftString = logToString($left);
  my $rightString = logToString($right);

  assertStringEquals($leftString, $rightString);
}

sub test_DefParser_constructor
{
  my $parser = Gen::DefParser->new('foo');

  assertStringEquals($parser->fileName, 'foo');
}

sub test_DefParser_empty
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  assertError(sub { $parser->processFromString(''); });
}

sub test_DefParser_minimal
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:2', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }]
  ]);
}

sub test_DefParser_initDone
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo
nAmEsPaCe: foobar
Package: package/name
TypePackagE: Common DateTime
TypePackagE: Common Code
parent: Bar
outerClass: Baz
Child: foo1
Child: foo2
Constructor: field, 7
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:11', {
      className => 'Foo',
      namespace => 'foobar',
      classPackage => 'package/name',
      parentFactory => 'Bar',
      outerClass => 'Baz',
      abstract => 0,
      children => ['foo1', 'foo2'],
      constructorArgs => ['field', 7],
      typeToPackage => {'DateTime' => 'Common', 'Code' => 'Common'}
    }]
  ]);
}

sub test_DefParser_abstract_noChildren
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  assertError(sub { $parser->processFromString(<<LINES
name: Foo AbStracT
LINES
    ); });
}

sub test_DefParser_abstract_constructor
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  assertError(sub { $parser->processFromString(<<LINES
name: Foo abstract
child: Bar
constructor: a
LINES
    ); });
}

sub test_DefParser_abstract_ok
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo abstract
child: Bar
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 1,
      children => ['Bar'],
      constructorArgs => [],
      typeToPackage => {}
    }]
  ]);
}

sub test_DefParser_specialAggregate_simple
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialAggregate: Section attribute: string

endSpecial
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialAggregateBegin', 'foo:3', {
      sectionName => 'Section',
      attributeName => 'attribute',
      variable => Gen::LocalVariable->new({name => 'value', type => Gen::Type->new('string')})}],
    ['specialAggregateEnd', 'foo:5', {sectionName => 'Section'}]
  ]);
}

sub test_DefParser_specialAggregate_noLoop
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialAggregate: Section attribute: string status
  read
    item->setValue(status);
  endRead
  write
    \$write(item->getValue());
  endWrite
endSpecial
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialAggregateBegin', 'foo:3', {
      sectionName => 'Section',
      attributeName => 'attribute',
      variable => Gen::LocalVariable->new({name => 'status', type => Gen::Type->new('string')})}],
    ['specialAggregateRead', 'foo:6', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')->writeLine('item->setValue(status);')}],
    ['specialAggregateWrite', 'foo:9', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')->writeLine('$write(item->getValue());')}],
    ['specialAggregateEnd', 'foo:10', {sectionName => 'Section'}]
  ]);
}

sub test_DefParser_specialAggregate_loop
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialAggregate: Section attribute: string status
  readLoop
    if (status == "\$1")
      item->set\$1(true);
  endReadLoop
  writeLoop
    if (item->get\$1())
      \$write("\$1");
  endWriteLoop

  loopNumbers: 1:2, 4

  loop: Big, Small, Tiny
endSpecial
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialAggregateBegin', 'foo:3', {
      sectionName => 'Section',
      attributeName => 'attribute',
      variable => Gen::LocalVariable->new({name => 'status', type => Gen::Type->new('string')})}],
    ['specialAggregateRead', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "1")')
        ->writeLine('  item->set1(true);')}],
    ['specialAggregateWrite', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->get1())')
        ->writeLine('  $write("1");')}],
    ['specialAggregateRead', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "2")')
        ->writeLine('  item->set2(true);')}],
    ['specialAggregateWrite', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->get2())')
        ->writeLine('  $write("2");')}],
    ['specialAggregateRead', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "4")')
        ->writeLine('  item->set4(true);')}],
    ['specialAggregateWrite', 'foo:13', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->get4())')
        ->writeLine('  $write("4");')}],
    ['specialAggregateRead', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "Big")')
        ->writeLine('  item->setBig(true);')}],
    ['specialAggregateWrite', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getBig())')
        ->writeLine('  $write("Big");')}],
    ['specialAggregateRead', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "Small")')
        ->writeLine('  item->setSmall(true);')}],
    ['specialAggregateWrite', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getSmall())')
        ->writeLine('  $write("Small");')}],
    ['specialAggregateRead', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (status == "Tiny")')
        ->writeLine('  item->setTiny(true);')}],
    ['specialAggregateWrite', 'foo:15', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getTiny())')
        ->writeLine('  $write("Tiny");')}],
    ['specialAggregateEnd', 'foo:16', {sectionName => 'Section'}]
  ]);
}

sub test_DefParser_specialAggregate_loop_n_noLoop
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialAggregate: Section attribute: string status
  read
    if (false);
  endRead
  readLoop
    else if (status == "\$1")
      item->set\$1(true);
  endReadLoop
  writeLoop
    if (item->get\$1())
      \$write("\$1");
  endWriteLoop

  loop: Big, Small, Tiny
endSpecial
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialAggregateBegin', 'foo:3', {
      sectionName => 'Section',
      attributeName => 'attribute',
      variable => Gen::LocalVariable->new({name => 'status', type => Gen::Type->new('string')})}],
    ['specialAggregateRead', 'foo:6', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('if (false);')}],
    ['specialAggregateRead', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('else if (status == "Big")')
        ->writeLine('  item->setBig(true);')}],
    ['specialAggregateWrite', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getBig())')
        ->writeLine('  $write("Big");')}],
    ['specialAggregateRead', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('else if (status == "Small")')
        ->writeLine('  item->setSmall(true);')}],
    ['specialAggregateWrite', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getSmall())')
        ->writeLine('  $write("Small");')}],
    ['specialAggregateRead', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')
        ->writeLine('else if (status == "Tiny")')
        ->writeLine('  item->setTiny(true);')}],
    ['specialAggregateWrite', 'foo:16', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')
        ->writeLine('if (item->getTiny())')
        ->writeLine('  $write("Tiny");')}],
    ['specialAggregateEnd', 'foo:17', {sectionName => 'Section'}]
  ]);
}

sub test_DefParser_specialSection
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialSection: Section attribute: string status
  read
    item->setValue(status);
  endRead
  write
    \$write(item->getValue());
  endWrite

  scalar: value: int getNumberValue
endSpecial
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialSectionBegin', 'foo:3', {
      sectionName => 'Section',
      attributeName => 'attribute',
      variable => Gen::LocalVariable->new({name => 'status', type => Gen::Type->new('string')})}],
    ['specialSectionRead', 'foo:6', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Read')->writeLine('item->setValue(status);')}],
    ['specialSectionWrite', 'foo:9', {
      sectionName => 'Section',
      attributeName => 'attribute',
      content => Gen::Writer->new('Write')->writeLine('$write(item->getValue());')}],
    ['scalar', 'foo:11', {
      sectionName => 'Section',
      attributeName => 'value',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => 'getNumberValue',
        type => Gen::Type->new('int'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['specialSectionEnd', 'foo:12', {sectionName => 'Section'}]
  ]);
}

sub test_DefParser_specialField
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

specialField: int foo
specialPublicField: double bar = -1.0
specialPublicFieldOmitSetter: string status = ""
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['specialField', 'foo:3', {
      section => 'private',
      variable => Gen::LocalVariable->new({name => 'foo', type => Gen::Type->new('int')}),
      init => '',
      omitSetter => 0}],
    ['specialField', 'foo:4', {
      section => 'public',
      variable => Gen::LocalVariable->new({name => 'bar', type => Gen::Type->new('double')}),
      init => '-1.0',
      omitSetter => 0}],
    ['specialField', 'foo:5', {
      section => 'public',
      variable => Gen::LocalVariable->new({name => 'status', type => Gen::Type->new('string')}),
      init => '""',
      omitSetter => 1}]
  ]);
}

sub test_DefParser_sections
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

section: A
endSection

section: B
endSection
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['sectionBegin', 'foo:3', {sectionName => 'A'}],
    ['sectionEnd', 'foo:4', {sectionName => 'A'}],
    ['sectionBegin', 'foo:6', {sectionName => 'B'}],
    ['sectionEnd', 'foo:7', {sectionName => 'B'}]
  ]);
}

sub test_DefParser_scalars
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo
constructor: a, f()
scalar: foo: int f()

section: A
  scalarWithCompare: bar: string br()
endSection

section: B
  scalar: baz: uint8_t bz() Input=char
endSection
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => ['a', 'f()'],
      typeToPackage => {}
    }],
    ['scalar', 'foo:3', {
      sectionName => '',
      attributeName => 'foo',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => 'f()',
        type => Gen::Type->new('int'),
        accessorType => $Gen::Accessor::ACCESSOR_CONSTRUCT})}],
    ['sectionBegin', 'foo:5', {sectionName => 'A'}],
    ['scalar', 'foo:6', {
      sectionName => 'A',
      attributeName => 'bar',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => 'br()',
        type => Gen::Type->new('std::string'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['scalarCompare', 'foo:6', {
      accessor => Gen::Accessor->new({
        name => 'br()',
        type => Gen::Type->new('std::string'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['sectionEnd', 'foo:7', {sectionName => 'A'}],
    ['sectionBegin', 'foo:9', {sectionName => 'B'}],
    ['scalar', 'foo:10', {
      sectionName => 'B',
      attributeName => 'baz',
      inputType => Gen::Type->new('char'),
      accessor => Gen::Accessor->new({
        name => 'bz()',
        type => Gen::Type->new('uint8_t'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['sectionEnd', 'foo:11', {sectionName => 'B'}]
  ]);
}

sub test_DefParser_scalarAggregates
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo
constructor: a, f()

scalarAggregate: foo: int                             f()
scalarAggregateWithCompare: bar: int*                 _bar
scalarAggregate: baz: std::vector<const int*>         getBaz Input=double
scalarAggregate: faz: std::vector<std::vector<int>*>* setFaz
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:4', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => ['a', 'f()'],
      typeToPackage => {}
    }],
    ['scalarAggregate', 'foo:4', {
      sectionName => 'foo',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => 'f()',
        type => Gen::Type->new('int'),
        accessorType => $Gen::Accessor::ACCESSOR_CONSTRUCT})}],
    ['scalarAggregate', 'foo:5', {
      sectionName => 'bar',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => '_bar',
        type => Gen::Type->new('int*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['scalarCompare', 'foo:5', {
      accessor => Gen::Accessor->new({
        name => '_bar',
        type => Gen::Type->new('int*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['scalarAggregate', 'foo:6', {
      sectionName => 'baz',
      inputType => Gen::Type->new('double'),
      accessor => Gen::Accessor->new({
        name => 'getBaz',
        type => Gen::Type->new('std::vector<const int*>'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['scalarAggregate', 'foo:7', {
      sectionName => 'faz',
      inputType => undef,
      accessor => Gen::Accessor->new({
        name => 'setFaz',
        type => Gen::Type->new('std::vector<std::vector<int>*>*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
  ]);
}

sub test_DefParser_factoryAggregates
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo
constructor: a, f()

factoryAggregate: foo: DateTime                        f()
factoryAggregateWithCompare: bar: const FareMarket*    _bar
factoryAggregate: baz: std::vector<ZoneInfo>           getBaz     Input=ZoneInfo2
factoryAggregate: faz: std::vector<std::vector<PaxType>*>* setFaz OwnsPointer
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:4', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => ['a', 'f()'],
      typeToPackage => {}
    }],
    ['factoryAggregate', 'foo:4', {
      sectionName => 'foo',
      inputType => undef,
      willDelete => 1,
      accessor => Gen::Accessor->new({
        name => 'f()',
        type => Gen::Type->new('DateTime'),
        accessorType => $Gen::Accessor::ACCESSOR_CONSTRUCT})}],
    ['factoryAggregate', 'foo:5', {
      sectionName => 'bar',
      inputType => undef,
      willDelete => 1,
      accessor => Gen::Accessor->new({
        name => '_bar',
        type => Gen::Type->new('const FareMarket*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['factoryCompare', 'foo:5', {
      accessor => Gen::Accessor->new({
        name => '_bar',
        type => Gen::Type->new('const FareMarket*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['factoryAggregate', 'foo:6', {
      sectionName => 'baz',
      inputType => Gen::Type->new('ZoneInfo2'),
      willDelete => 1,
      accessor => Gen::Accessor->new({
        name => 'getBaz',
        type => Gen::Type->new('std::vector<ZoneInfo>'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
    ['factoryAggregate', 'foo:7', {
      sectionName => 'faz',
      inputType => undef,
      willDelete => 0,
      accessor => Gen::Accessor->new({
        name => 'setFaz',
        type => Gen::Type->new('std::vector<std::vector<PaxType>*>*'),
        accessorType => $Gen::Accessor::ACCESSOR_INIT})}],
  ]);
}

sub test_DefParser_preInit
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

initLines
  first init line
   second init line
endInitLines
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['preInit', 'foo:6', { content => Gen::Writer->new('InitLines')
      ->writeLine('first init line')
      ->writeLine(' second init line')}]
  ]);
}

sub test_DefParser_postInit
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

postInitLines
  first init line
   second init line
endPostInitLines
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['postInit', 'foo:6', { content => Gen::Writer->new('PostInitLines')
      ->writeLine('first init line')
      ->writeLine(' second init line')}]
  ]);
}

sub test_DefParser_checkItem
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

checkItemLines
  first checkItem line
    second checkItem line
endCheckItemLines
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['checkItem', 'foo:6', { content => Gen::Writer->new('CheckItemLines')
      ->writeLine('first checkItem line')
      ->writeLine('  second checkItem line')}]
  ]);
}

sub test_DefParser_include
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

extraInclude: foo.h
extraInclude: Bar/Foo.h
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['include', 'foo:3', {file => 'foo.h'}],
    ['include', 'foo:4', {file => 'Bar/Foo.h'}]
  ]);
}

sub test_DefParser_name
{
  my $parser = Gen::DefParser->new('foo');
  my $log = configureDefParser($parser);
  $parser->processFromString(<<LINES
name: Foo

nameString: "Foo_"
nameField: field
nameString: 1
LINES
  );

  assertLogEquals($log, [
    ['initDone', 'foo:3', {
      className => 'Foo',
      namespace => '',
      classPackage => '',
      parentFactory => '',
      outerClass => '',
      abstract => 0,
      children => [],
      constructorArgs => [],
      typeToPackage => {}
    }],
    ['nameValue', 'foo:3', {value => '"Foo_"'}],
    ['nameField', 'foo:4', {field => 'field'}],
    ['nameValue', 'foo:5', {value => '1'}]
  ]);
}

# TODO test deprecated stuff
# TODO add some negative tests, to test error handling

registerTest('Gen::test::DefParserTest');
