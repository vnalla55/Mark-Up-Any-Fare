############################################### FactorySectionWrite.pm Test

package Gen::test::FactorySectionWriteTest;
require Gen::FactorySectionWrite;
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

sub test_FactorySectionWrite_empty
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});

  assertWriterEquals($section, '');
}

sub test_FactorySectionWrite_simple
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    '',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  element->SetAttribute("foo", item->foo());
}
LINES
  );
}

sub test_FactorySectionWrite_two
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});

  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    '',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'bar()', type => Gen::Type->new('string')}),
    undef,
    '',
    'bar'
  );
  $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"bar\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  element->SetAttribute("foo", item->foo());
  element->SetAttribute("bar", item->bar());
}
LINES
  );
}

sub test_FactorySectionWrite_directWriter
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    '',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");
  $section->directWriter->writeCode('direct();');

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  element->SetAttribute("foo", item->foo());
  direct();
}
LINES
  );
}

sub test_FactorySectionWrite_loopWriter
{
  my $section = Gen::FactorySectionWrite->new({name => 'Foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    '',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");
  $section->loopWriter->writeCode('loop();');

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Foo"));

  element->SetAttribute("foo", item->foo());

  loop();
}
LINES
  );
}

sub test_FactorySectionWrite_section
{
  my $section = Gen::FactorySectionWrite->new({name => 'Foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_root_section
{
  my $section = Gen::FactorySectionWrite->new({});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

  element->SetAttribute("foo", item->foo());
}
LINES
  );
}

sub test_FactorySectionWrite_getSection
{
  my $section = Gen::FactorySectionWrite->new({name => 'Foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int'), incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");
  my $subsection = $section->getSection('Other');
  $subsection->directWriter->writeLine("return;");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", item->foo());
  }
  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Other"));

    return;
  }
}
LINES
  );
}

sub test_FactorySectionWrite_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => 'Foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int*'), incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", *item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_const_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => 'Foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('const int*'), incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", *item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_vector
{
  my $section = Gen::FactorySectionWrite->new({name => ''});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<int>'),
      incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  std::vector<int>::const_iterator
    it1 = item->foo().begin(),
    end = item->foo().end();
  for (; it1 != end; ++it1)
  {
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", *it1);
  }
}
LINES
  );
}

sub test_FactorySectionWrite_vector_const_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => ''});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<const int*>'),
      incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  std::vector<const int*>::const_iterator
    it1 = item->foo().begin(),
    end = item->foo().end();
  for (; it1 != end; ++it1)
  {
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", **it1);
  }
}
LINES
  );
}

sub test_FactorySectionWrite_vector_vector
{
  my $section = Gen::FactorySectionWrite->new({name => ''});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<std::vector<int>*>'),
      incomplete => 0}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  std::vector<std::vector<int>*>::const_iterator
    it1 = item->foo().begin(),
    end = item->foo().end();
  for (; it1 != end; ++it1)
  {
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    {
      std::vector<int>::const_iterator
        it2 = (*it1)->begin(),
        end = (*it1)->end();
      for (; it2 != end; ++it2)
      {
        TiXmlElement* rootElement = element;
        TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
          "Element"));

        element->SetAttribute("foo", *it2);
      }
    }
  }
}
LINES
  );
}

sub test_FactorySectionWrite_vector_const_vector
{
  my $section = Gen::FactorySectionWrite->new({name => ''});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', incomplete => 0, type =>
      Gen::Type->new('std::vector<const std::vector<int>*>')}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  std::vector<const std::vector<int>*>::const_iterator
    it1 = item->foo().begin(),
    end = item->foo().end();
  for (; it1 != end; ++it1)
  {
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    {
      std::vector<int>::const_iterator
        it2 = (*it1)->begin(),
        end = (*it1)->end();
      for (; it2 != end; ++it2)
      {
        TiXmlElement* rootElement = element;
        TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
          "Element"));

        element->SetAttribute("foo", *it2);
      }
    }
  }
}
LINES
  );
}

sub test_FactorySectionWrite_vector_list_set
{
  my $section = Gen::FactorySectionWrite->new({name => ''});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', incomplete => 0, type =>
      Gen::Type->new('std::vector<std::list<std::set<string>*>*>*')}),
    undef,
    'Section',
    'foo'
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  std::vector<std::list<std::set<std::string>*>*>::const_iterator
    it1 = item->foo()->begin(),
    end = item->foo()->end();
  for (; it1 != end; ++it1)
  {
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    {
      std::list<std::set<std::string>*>::const_iterator
        it2 = (*it1)->begin(),
        end = (*it1)->end();
      for (; it2 != end; ++it2)
      {
        TiXmlElement* rootElement = element;
        TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
          "Element"));

        {
          std::set<std::string>::const_iterator
            it3 = (*it2)->begin(),
            end = (*it2)->end();
          for (; it3 != end; ++it3)
          {
            TiXmlElement* rootElement = element;
            TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
              "Element"));

            element->SetAttribute("foo", *it3);
          }
        }
      }
    }
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", (unsigned)item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('int*')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", *(unsigned*)item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType_const_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('const int*')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    TiXmlElement* rootElement = element;
    TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

    element->SetAttribute("foo", *(const unsigned*)item->foo());
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType_vector
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<int>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    std::vector<int>::const_iterator
      it1 = item->foo().begin(),
      end = item->foo().end();
    for (; it1 != end; ++it1)
    {
      TiXmlElement* rootElement = element;
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

      element->SetAttribute("foo", (unsigned)*it1);
    }
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType_vector_const_pointer
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<const int*>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    std::vector<const int*>::const_iterator
      it1 = item->foo().begin(),
      end = item->foo().end();
    for (; it1 != end; ++it1)
    {
      TiXmlElement* rootElement = element;
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

      element->SetAttribute("foo", *(const unsigned*)*it1);
    }
  }
}
LINES
  );
}

sub test_FactorySectionWrite_outputType_vector2
{
  my $section = Gen::FactorySectionWrite->new({name => 'foo'});
  my ($variable, $writer, $rootElement) = $section->prepareWrite(
    Gen::Accessor->new({name => 'foo()', type => Gen::Type->new('std::vector<std::vector<int>*>')}),
    Gen::Type->new('unsigned'),
    'Section',
    'foo',
    1
  );
  my $getter = $variable->getExpressionReference();
  $writer->writeCode("$rootElement->SetAttribute(\"foo\", $getter);");

  assertWriterEquals($section, <<LINES
{
  TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("foo"));

  {
    std::vector<std::vector<int>*>::const_iterator
      it1 = item->foo().begin(),
      end = item->foo().end();
    for (; it1 != end; ++it1)
    {
      TiXmlElement* rootElement = element;
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Section"));

      {
        std::vector<int>::const_iterator
          it2 = (*it1)->begin(),
          end = (*it1)->end();
        for (; it2 != end; ++it2)
        {
          TiXmlElement* rootElement = element;
          TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
            "Element"));

          element->SetAttribute("foo", (unsigned)*it2);
        }
      }
    }
  }
}
LINES
  );
}

registerTest('Gen::test::FactorySectionWriteTest');
