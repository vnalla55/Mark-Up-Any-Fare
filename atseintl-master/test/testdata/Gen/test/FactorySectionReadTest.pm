############################################### FactorySectionRead.pm Test

package Gen::test::FactorySectionReadTest;
require Gen::FactorySectionRead;
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

sub test_FactorySectionRead_empty
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});

  assertWriterEquals($section, '');
}

sub test_FactorySectionRead_simple
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  item->foo() = 42;
}
LINES
  );
}

sub test_FactorySectionRead_deferAll
{
  my $section = Gen::FactorySectionRead->new({name => 'foo', deferAllSets => 1});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
int fooLocal;

if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  fooLocal = 42;
}

item->foo() = fooLocal;
LINES
  );
}

sub test_FactorySectionRead_deferAll_omitFinalSet
{
  my $section = Gen::FactorySectionRead->new({name => 'foo', deferAllSets => 1, omitFinalSet => 1});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
int fooLocal;

if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  fooLocal = 42;
}
LINES
  );
}

sub test_FactorySectionRead_directWriter
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');
  $section->directWriter->writeCode('direct();');

  assertWriterEquals($section, <<LINES
if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  item->foo() = 42;
}
direct();
LINES
  );
}

sub test_FactorySectionRead_loopWriter
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');
  $section->loopWriter->writeCode('loop();');

  assertWriterEquals($section, <<LINES
if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  item->foo() = 42;
}

TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  loop();
}
LINES
  );
}

sub test_FactorySectionRead_getSection
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');
  my $subsection = $section->getSection('Other');
  $subsection->directWriter->writeLine("return;");

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      item->foo() = 42;
    }
  }
  else if (elementName == "Other")
  {
    return;
  }
}
LINES
  );
}

sub test_FactorySectionRead_getAccessor
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  my $v = $section->getAccessor('foo()');
  assertEquals($variable, $v);
}

sub test_FactorySectionRead_two
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});

  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'getBar', type => Gen::Type->new('string')}),
    undef,
    '',
    'bar',
    1
  );
  $writer->writeCode('populate('.$variable->getExpressionPointer().');');

  assertWriterEquals($section, <<LINES
if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  item->foo() = 42;
}
if (TestXMLHelper::AttributeExists(rootElement, "bar"))
{
  std::string barLocal;
  populate(&barLocal);
  item->setBar(barLocal);
}
LINES
  );
}

sub test_FactorySectionRead_two_sameAccessor
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});

  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    '',
    'bar',
    1
  );
  $writer->writeCode($variable->setInstruction('1337').';');

  assertWriterEquals($section, <<LINES
if (TestXMLHelper::AttributeExists(rootElement, "foo"))
{
  item->foo() = 42;
}
if (TestXMLHelper::AttributeExists(rootElement, "bar"))
{
  item->foo() = 1337;
}
LINES
  );
}

sub test_FactorySectionRead_section
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      item->foo() = 42;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_getter
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'getFoo', type => Gen::Type->new('int')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int fooLocal;
      fooLocal = 42;
      item->setFoo(fooLocal);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_getter_noReference
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'getFoo', type => Gen::Type->new('int')}),
    undef,
    'Section',
    'foo',
    0
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      item->setFoo(42);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_pointer
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int*')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('ptr').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int* fooLocal = new int;
      fooLocal = ptr;
      item->foo() = fooLocal;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_const_pointer
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('const int*')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('ptr').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int* fooLocal = new int;
      fooLocal = ptr;
      item->foo() = fooLocal;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_vector
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<int>')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int fooLocal;
      fooLocal = 42;
      item->foo().push_back(fooLocal);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_vector_const_pointer
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<const int*>')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('ptr').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int* fooLocal = new int;
      fooLocal = ptr;
      item->foo().push_back(fooLocal);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_vector_getter
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'getFoo', type => Gen::Type->new('std::vector<int>')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
std::vector<int> fooLocal;

TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      int fooLocalLocal;
      fooLocalLocal = 42;
      fooLocal.push_back(fooLocalLocal);
    }
  }
}

item->setFoo(fooLocal);
LINES
  );
}

sub test_FactorySectionRead_vector_vector
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<std::vector<int>*>')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    std::vector<int>* fooLocal = new std::vector<int>;

    TiXmlNode* child = 0;
    while ((child = element->IterateChildren(child)))
    {
      TiXmlElement* element = (TiXmlElement*)child;
      const std::string elementName = element->Value();

      if (false);
      else if (elementName == "Element")
      {
        if (TestXMLHelper::AttributeExists(element, "foo"))
        {
          int fooLocalLocal;
          fooLocalLocal = 42;
          fooLocal->push_back(fooLocalLocal);
        }
      }
    }

    item->foo().push_back(fooLocal);
  }
}
LINES
  );
}

sub test_FactorySectionRead_vector_list_set
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type =>
      Gen::Type->new('std::vector<std::list<std::set<string>*>*>*')}),
    undef,
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  # FIXME fooLocalLocalLocalLocal is not a very descriptive name...
  assertWriterEquals($section, <<LINES
std::vector<std::list<std::set<std::string>*>*>* fooLocal = new std::vector<std::list<std::set<std::
  string>*>*>;

TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    std::list<std::set<std::string>*>* fooLocalLocal = new std::list<std::set<std::string>*>;

    TiXmlNode* child = 0;
    while ((child = element->IterateChildren(child)))
    {
      TiXmlElement* element = (TiXmlElement*)child;
      const std::string elementName = element->Value();

      if (false);
      else if (elementName == "Element")
      {
        std::set<std::string>* fooLocalLocalLocal = new std::set<std::string>;

        TiXmlNode* child = 0;
        while ((child = element->IterateChildren(child)))
        {
          TiXmlElement* element = (TiXmlElement*)child;
          const std::string elementName = element->Value();

          if (false);
          else if (elementName == "Element")
          {
            if (TestXMLHelper::AttributeExists(element, "foo"))
            {
              std::string fooLocalLocalLocalLocal;
              fooLocalLocalLocalLocal = 42;
              fooLocalLocalLocal->insert(fooLocalLocalLocalLocal);
            }
          }
        }

        fooLocalLocal->push_back(fooLocalLocalLocal);
      }
    }

    fooLocal->push_back(fooLocalLocal);
  }
}

item->foo() = fooLocal;
LINES
  );
}

sub test_FactorySectionRead_inputType
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      unsigned fooLocal;
      fooLocal = 42;
      item->foo() = (int)fooLocal;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_inputType_pointer
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int*')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('ptr').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      unsigned* fooLocal = new unsigned;
      fooLocal = ptr;
      item->foo() = (int*)fooLocal;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_inputType_const_pointer
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('const int*')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('ptr').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      unsigned* fooLocal = new unsigned;
      fooLocal = ptr;
      item->foo() = (const int*)fooLocal;
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_inputType_vector
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<int>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      unsigned fooLocal;
      fooLocal = 42;
      item->foo().push_back((int)fooLocal);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_inputType_vector_const
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<const int>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    if (TestXMLHelper::AttributeExists(element, "foo"))
    {
      unsigned fooLocal;
      fooLocal = 42;
      item->foo().push_back((const int)fooLocal);
    }
  }
}
LINES
  );
}

sub test_FactorySectionRead_inputType_vector2
{
  my $section = Gen::FactorySectionRead->new({name => 'foo'});
  my ($variable, $writer) = $section->prepareRead(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<std::vector<int>*>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  $writer->writeCode($variable->setInstruction('42').';');

  assertWriterEquals($section, <<LINES
TiXmlNode* child = 0;
while ((child = rootElement->IterateChildren(child)))
{
  TiXmlElement* element = (TiXmlElement*)child;
  const std::string elementName = element->Value();

  if (false);
  else if (elementName == "Section")
  {
    std::vector<int>* fooLocal = new std::vector<int>;

    TiXmlNode* child = 0;
    while ((child = element->IterateChildren(child)))
    {
      TiXmlElement* element = (TiXmlElement*)child;
      const std::string elementName = element->Value();

      if (false);
      else if (elementName == "Element")
      {
        if (TestXMLHelper::AttributeExists(element, "foo"))
        {
          unsigned fooLocalLocal;
          fooLocalLocal = 42;
          fooLocal->push_back((int)fooLocalLocal);
        }
      }
    }

    item->foo().push_back(fooLocal);
  }
}
LINES
  );
}

registerTest('Gen::test::FactorySectionReadTest');
