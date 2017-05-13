############################################### FactoryGenerator.pm Test

package Gen::test::FactoryGeneratorTest;
require Gen::FactoryGenerator;
require Gen::Type;

use strict;
use warnings;
use Gen::test::Test;
use Scalar::Util qw(reftype);

sub assertWriterEquals
{
  my ($writer, $string) = @_;

  my $result = $writer->saveToString();
  assertStringEquals($result, $string);
}

sub assertSelectiveEquals
{
  my ($result, $selects) = @_;
  if ((reftype($selects) || '') ne 'HASH')
  {
    return assertStringEquals($result, $selects);
  }

  foreach my $key (keys %$selects)
  {
    my $select = $$selects{$key};
    my $lineCount = scalar split(/^/, $select);

    my $regexp = "\n(.*\nTest.*Factory::\Q$key\E(?:.{2,}\n|[^}]\n|\n)+\}\n)";
    assert(scalar $result =~ /$regexp/, "$key not found");

    assertStringEquals($1, $select);
  }
}

sub assertFactoryEquals
{
  my ($factory, $headerTest, $sourceTest) = @_;

  my ($header, $source) = $factory->saveToString();

  # Remove date to make result deterministic.
  $header =~ s/on \d{4}-\d{2}-\d{2} at \d{2}:\d{2}:\d{2}/on yyyy-mm-dd at hh:ii:ss/;
  $source =~ s/on \d{4}-\d{2}-\d{2} at \d{2}:\d{2}:\d{2}/on yyyy-mm-dd at hh:ii:ss/;

  assertStringEquals($header, $headerTest) if defined $headerTest;
  assertSelectiveEquals($source, $sourceTest) if defined $sourceTest;
}

sub test_FactoryGenerator_empty
{
  my $factory = Gen::FactoryGenerator->new({className => 'Foo'});

  assertFactoryEquals($factory, <<LINES
#ifndef TEST_FOO_FACTORY_H
#define TEST_FOO_FACTORY_H

/**
 *
 * This Factory is for the creation of test locations in order to facilitate unit testing.
 * Particular cities are supported (DFW, LAX, LON, for example); use of the class is by invoking the
 * static method for the location desired.
 *
 **/

#include <iostream>
#include <string>

#include "Common/TsePrimitiveTypes.h"
#include "Foo.h"

class TiXmlNode;
class TiXmlElement;

class TestFooFactory
{
public:
  static void
  clearCache();

  static Foo*
  create(const std::string& fileName, bool unique = false);

  static Foo*
  create(const std::string& fileName, int flags);

  static Foo*
  create(const std::string& fileName, TiXmlElement* rootElement, int flags);

  static Foo*
  construct(const std::string& fileName, TiXmlElement* rootElement, int flags);

  static void
  preInit(Foo* item);

  static void
  init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags);

  static void
  postInit(Foo* item);

  static void
  write(const std::string& fullPathName, const Foo& item);

  static void
  writeSubItem(const std::string& sectionName, const Foo& item, TiXmlNode* node,
               const std::string& filePrefix);

  static void
  write(const Foo& item, TiXmlNode* node, const std::string& filePrefix);

  static void
  write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix);

  static bool
  compare(const Foo* i1, const Foo* i2);

  static bool
  checkItem(const Foo& item);

private:
  TestFooFactory();
};

#endif
LINES
  , <<LINES
#include "Common/TseConsts.h"
#include "test/testdata/TestFactoryBase.h"
#include "test/testdata/TestFooFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/testdata/tinyxml/tinyxml.h"

using namespace tse;

/* public static */ void
TestFooFactory::clearCache()
{
  TestFactoryBase<TestFooFactory>::destroyAll<Foo>();
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, bool unique /* = false */)
{
  using namespace TestFactoryFlags;
  return TestFooFactory::create(fileName, unique ? UNIQUE : DEFAULT);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, "Foo", flags);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, rootElement, flags);
}

/* public static */ Foo*
TestFooFactory::construct(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
  return new Foo;
}

/* public static */ void
TestFooFactory::preInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
}

/* public static */ void
TestFooFactory::postInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::write(const std::string& fullPathName, const Foo& item)
{
  TestFactoryBase<TestFooFactory>::write<Foo>(fullPathName, item);
}

/* public static */ void
TestFooFactory::writeSubItem(const std::string& sectionName, const Foo& item, TiXmlNode* node,
                             const std::string& filePrefix)
{
  if (!checkItem(item))
    return;
  TestFactoryBase<TestFooFactory>::writeSubItem<Foo>(sectionName, item, node, filePrefix, "Foo");
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlNode* node, const std::string& filePrefix)
{
  TestFactoryBase<TestFooFactory>::write<Foo>("Foo", item, node, filePrefix);
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
}

/* public static */ bool
TestFooFactory::compare(const Foo* i1, const Foo* i2)
{
  if (i1 == i2)
    return true;
  return false;
}

/* public static */ bool
TestFooFactory::checkItem(const Foo& item)
{
  if (!&item)
    return false;
  return true;
}

LINES
  );
}

sub test_FactoryGenerator_include
{
  my $factory = Gen::FactoryGenerator->new({className => 'Foo'});
  $factory
    ->addInclude('DataModel/FareMarket.h')
    ->addInclude('zzz/Yyy.h');

  assertFactoryEquals($factory, undef, <<LINES
#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"
#include "test/testdata/TestFactoryBase.h"
#include "test/testdata/TestFooFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/testdata/tinyxml/tinyxml.h"
#include "zzz/Yyy.h"

using namespace tse;

/* public static */ void
TestFooFactory::clearCache()
{
  TestFactoryBase<TestFooFactory>::destroyAll<Foo>();
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, bool unique /* = false */)
{
  using namespace TestFactoryFlags;
  return TestFooFactory::create(fileName, unique ? UNIQUE : DEFAULT);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, "Foo", flags);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, rootElement, flags);
}

/* public static */ Foo*
TestFooFactory::construct(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
  return new Foo;
}

/* public static */ void
TestFooFactory::preInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
}

/* public static */ void
TestFooFactory::postInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::write(const std::string& fullPathName, const Foo& item)
{
  TestFactoryBase<TestFooFactory>::write<Foo>(fullPathName, item);
}

/* public static */ void
TestFooFactory::writeSubItem(const std::string& sectionName, const Foo& item, TiXmlNode* node,
                             const std::string& filePrefix)
{
  if (!checkItem(item))
    return;
  TestFactoryBase<TestFooFactory>::writeSubItem<Foo>(sectionName, item, node, filePrefix, "Foo");
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlNode* node, const std::string& filePrefix)
{
  TestFactoryBase<TestFooFactory>::write<Foo>("Foo", item, node, filePrefix);
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
}

/* public static */ bool
TestFooFactory::compare(const Foo* i1, const Foo* i2)
{
  if (i1 == i2)
    return true;
  return false;
}

/* public static */ bool
TestFooFactory::checkItem(const Foo& item)
{
  if (!&item)
    return false;
  return true;
}

LINES
  );
}

sub test_FactoryGenerator_specialField
{
  my $factory = Gen::FactoryGenerator->new({className => 'Foo'});
  $factory
    ->addSpecialField('public', Gen::LocalVariable->new(
      {name => 'foo', type => Gen::Type->new('int')}), '', 1)
    ->addSpecialField('private', Gen::LocalVariable->new(
      {name => 'bar', type => Gen::Type->new('string')}), '"default"', 0);

  assertFactoryEquals($factory, <<LINES
#ifndef TEST_FOO_FACTORY_H
#define TEST_FOO_FACTORY_H

/**
 *
 * This Factory is for the creation of test locations in order to facilitate unit testing.
 * Particular cities are supported (DFW, LAX, LON, for example); use of the class is by invoking the
 * static method for the location desired.
 *
 **/

#include <iostream>
#include <string>

#include "Common/TsePrimitiveTypes.h"
#include "Foo.h"

class TiXmlNode;
class TiXmlElement;

class TestFooFactory
{
public:
  static void
  clearCache();

  static Foo*
  create(const std::string& fileName, bool unique = false);

  static Foo*
  create(const std::string& fileName, int flags);

  static Foo*
  create(const std::string& fileName, TiXmlElement* rootElement, int flags);

  static Foo*
  construct(const std::string& fileName, TiXmlElement* rootElement, int flags);

  static void
  preInit(Foo* item);

  static void
  init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags);

  static void
  postInit(Foo* item);

  static void
  write(const std::string& fullPathName, const Foo& item);

  static void
  writeSubItem(const std::string& sectionName, const Foo& item, TiXmlNode* node,
               const std::string& filePrefix);

  static void
  write(const Foo& item, TiXmlNode* node, const std::string& filePrefix);

  static void
  write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix);

  static bool
  compare(const Foo* i1, const Foo* i2);

  static bool
  checkItem(const Foo& item);

  static int
  getFoo();

private:
  TestFooFactory();

  static std::string
  getBar();

  static void
  setBar(const std::string& bar);

  static
  int _foo;
  static
  std::string _bar /* = "default" */;
};

#endif
LINES
  , <<LINES
#include "Common/TseConsts.h"
#include "test/testdata/TestFactoryBase.h"
#include "test/testdata/TestFooFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/testdata/tinyxml/tinyxml.h"

using namespace tse;

/* public static */ void
TestFooFactory::clearCache()
{
  TestFactoryBase<TestFooFactory>::destroyAll<Foo>();
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, bool unique /* = false */)
{
  using namespace TestFactoryFlags;
  return TestFooFactory::create(fileName, unique ? UNIQUE : DEFAULT);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, "Foo", flags);
}

/* public static */ Foo*
TestFooFactory::create(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  return TestFactoryBase<TestFooFactory>::create<Foo>(fileName, rootElement, flags);
}

/* public static */ Foo*
TestFooFactory::construct(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
  return new Foo;
}

/* public static */ void
TestFooFactory::preInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
}

/* public static */ void
TestFooFactory::postInit(Foo* item)
{
}

/* public static */ void
TestFooFactory::write(const std::string& fullPathName, const Foo& item)
{
  TestFactoryBase<TestFooFactory>::write<Foo>(fullPathName, item);
}

/* public static */ void
TestFooFactory::writeSubItem(const std::string& sectionName, const Foo& item, TiXmlNode* node,
                             const std::string& filePrefix)
{
  if (!checkItem(item))
    return;
  TestFactoryBase<TestFooFactory>::writeSubItem<Foo>(sectionName, item, node, filePrefix, "Foo");
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlNode* node, const std::string& filePrefix)
{
  TestFactoryBase<TestFooFactory>::write<Foo>("Foo", item, node, filePrefix);
}

/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
}

/* public static */ bool
TestFooFactory::compare(const Foo* i1, const Foo* i2)
{
  if (i1 == i2)
    return true;
  return false;
}

/* public static */ bool
TestFooFactory::checkItem(const Foo& item)
{
  if (!&item)
    return false;
  return true;
}

/* public static */ int
TestFooFactory::getFoo()
{
  return _foo;
}

/* private static */ std::string
TestFooFactory::getBar()
{
  return _bar;
}

/* private static */ void
TestFooFactory::setBar(const std::string& bar)
{
  _bar = bar;
}

/* private static */
int TestFooFactory::_foo;

/* private static */
std::string TestFooFactory::_bar = "default";

LINES
  );
}

sub test_FactoryGenerator_init
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addAttribute('', 'bar', Gen::Accessor->new({
      name => 'bar', type => Gen::Type->new('int'),
      accessorType => $Gen::Accessor::ACCESSOR_INIT}))
    ->addAttribute('', 'baz', Gen::Accessor->new({
      name => 'baz', type => Gen::Type->new('double'),
      accessorType => $Gen::Accessor::ACCESSOR_INIT}));

  assertFactoryEquals($factory, undef, {
    'init' => <<LINES
/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;

  if (TestXMLHelper::AttributeExists(rootElement, "bar"))
  {
    TestXMLHelper::Attribute(rootElement, "bar", item->bar);
  }
  if (TestXMLHelper::AttributeExists(rootElement, "baz"))
  {
    TestXMLHelper::Attribute(rootElement, "baz", item->baz);
  }
}
LINES
    , 'write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)' => <<LINES
/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
  rootElement->SetAttribute("bar", TestXMLHelper::format(item.bar));
  rootElement->SetAttribute("baz", TestXMLHelper::format(item.baz));
}
LINES
  });
}

sub test_FactoryGenerator_construct
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo',
    constructorArgs => ['bar', 'baz']
  });
  $factory
    ->addAttribute('', 'bar', Gen::Accessor->new({
      name => 'bar', type => Gen::Type->new('int'),
      accessorType => $Gen::Accessor::ACCESSOR_CONSTRUCT}))
    ->addAttribute('', 'baz', Gen::Accessor->new({
      name => 'baz', type => Gen::Type->new('double'),
      accessorType => $Gen::Accessor::ACCESSOR_CONSTRUCT}));

  assertFactoryEquals($factory, undef, {
    'init' => <<LINES
/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
}
LINES
    , 'construct' => <<LINES
/* public static */ Foo*
TestFooFactory::construct(const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;
  int barLocal;
  double bazLocal;

  if (TestXMLHelper::AttributeExists(rootElement, "bar"))
  {
    TestXMLHelper::Attribute(rootElement, "bar", barLocal);
  }
  if (TestXMLHelper::AttributeExists(rootElement, "baz"))
  {
    TestXMLHelper::Attribute(rootElement, "baz", bazLocal);
  }

  return new Foo(
    barLocal,
    bazLocal
  );
}
LINES
    , 'write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)' => <<LINES
/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
  rootElement->SetAttribute("bar", TestXMLHelper::format(item.bar));
  rootElement->SetAttribute("baz", TestXMLHelper::format(item.baz));
}
LINES
  });
}

sub test_FactoryGenerator_vector_compare_factory
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  my $accessor = Gen::Accessor->new({
    name => 'bar()', type => Gen::Type->new('std::vector<Bar*>'),
    accessorType => $Gen::Accessor::ACCESSOR_INIT});
  $factory
    ->addFactorySection('Bar', $accessor, undef, 1)
    ->addFactoryCompare($accessor->clone());

  assertFactoryEquals($factory, undef, {
    'init' => <<LINES
/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;

  TiXmlNode* child = 0;
  while ((child = rootElement->IterateChildren(child)))
  {
    TiXmlElement* element = (TiXmlElement*)child;
    const std::string elementName = element->Value();

    if (false);
    else if (elementName == "Bar")
    {
      item->bar().push_back(TestFactoryBase<TestBarFactory>::create<Bar> (fileName, element, flags &
        UNIQUE));
    }
  }
}
LINES
    , 'write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)' => <<LINES
/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
  {
    std::vector<Bar*>::const_iterator
      it1 = item.bar().begin(),
      end = item.bar().end();
    for (; it1 != end; ++it1)
      TestBarFactory::writeSubItem("Bar", **it1, rootElement, filePrefix);
  }
}
LINES
    , 'compare' => <<LINES
/* public static */ bool
TestFooFactory::compare(const Foo* i1, const Foo* i2)
{
  if (i1 == i2)
    return true;

  {
    std::vector<Bar*>::const_iterator
      it1_1 = i1->bar().begin(), e1 = i1->bar().end(),
      it1_2 = i2->bar().begin(), e2 = i2->bar().end();
    for (; it1_1 != e1 && it1_2 != e2; ++it1_1, ++it1_2)
      if (!TestBarFactory::compare(*it1_1, *it1_2))
        return false;

    if (it1_1 != e1 || it1_2 != e2)
      return false;
  }
  return true;
}
LINES
  });
}

sub test_FactoryGenerator_vector_vector_compare_scalar
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  my $accessor = Gen::Accessor->new({
    name => 'bar()', type => Gen::Type->new('std::vector<std::vector<Bar*>>'),
    accessorType => $Gen::Accessor::ACCESSOR_INIT});
  $factory
    ->addScalarSection('Bar', $accessor, undef)
    ->addScalarCompare($accessor->clone());

  assertFactoryEquals($factory, undef, {
    'init' => <<LINES
/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;

  TiXmlNode* child = 0;
  while ((child = rootElement->IterateChildren(child)))
  {
    TiXmlElement* element = (TiXmlElement*)child;
    const std::string elementName = element->Value();

    if (false);
    else if (elementName == "Bar")
    {
      std::vector<Bar*> barLocal;

      TiXmlNode* child = 0;
      while ((child = element->IterateChildren(child)))
      {
        TiXmlElement* element = (TiXmlElement*)child;
        const std::string elementName = element->Value();

        if (false);
        else if (elementName == "Element")
        {
          if (TestXMLHelper::AttributeExists(element, "value"))
          {
            Bar* barLocalLocal = new Bar;
            TestXMLHelper::Attribute(element, "value", *barLocalLocal);
            barLocal.push_back(barLocalLocal);
          }
        }
      }

      item->bar().push_back(barLocal);
    }
  }
}
LINES
    , 'write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)' => <<LINES
/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
  {
    std::vector<std::vector<Bar*> >::const_iterator
      it1 = item.bar().begin(),
      end = item.bar().end();
    for (; it1 != end; ++it1)
    {
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("Bar"));

      {
        std::vector<Bar*>::const_iterator
          it2 = (*it1).begin(),
          end = (*it1).end();
        for (; it2 != end; ++it2)
        {
          TiXmlElement* rootElement = element;
          TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement(
            "Element"));

          element->SetAttribute("value", TestXMLHelper::format(**it2));
        }
      }
    }
  }
}
LINES
    , 'compare' => <<LINES
/* public static */ bool
TestFooFactory::compare(const Foo* i1, const Foo* i2)
{
  if (i1 == i2)
    return true;

  {
    std::vector<std::vector<Bar*> >::const_iterator
      it1_1 = i1->bar().begin(), e1 = i1->bar().end(),
      it1_2 = i2->bar().begin(), e2 = i2->bar().end();
    for (; it1_1 != e1 && it1_2 != e2; ++it1_1, ++it1_2)
      {
        std::vector<Bar*>::const_iterator
          it2_1 = (*it1_1).begin(), e1 = (*it1_1).end(),
          it2_2 = (*it1_2).begin(), e2 = (*it1_2).end();
        for (; it2_1 != e1 && it2_2 != e2; ++it2_1, ++it2_2)
          if (**it2_1 != **it2_2)
            return false;

        if (it2_1 != e1 || it2_2 != e2)
          return false;
      }

    if (it1_1 != e1 || it1_2 != e2)
      return false;
  }
  return true;
}
LINES
  });
}

sub test_FactoryGenerator_preInit
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addPreInit(Gen::Writer->new()->writeLine('line1;'))
    ->addPreInit(Gen::Writer->new()->writeLine('line2;'));

  assertFactoryEquals($factory, undef, {
    'preInit' => <<LINES
/* public static */ void
TestFooFactory::preInit(Foo* item)
{
  line1;
  line2;
}
LINES
  });
}

sub test_FactoryGenerator_postInit
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addPostInit(Gen::Writer->new()->writeLine('line1;'))
    ->addPostInit(Gen::Writer->new()->writeLine('line2;'));

  assertFactoryEquals($factory, undef, {
    'postInit' => <<LINES
/* public static */ void
TestFooFactory::postInit(Foo* item)
{
  line1;
  line2;
}
LINES
  });
}

sub test_FactoryGenerator_checkItem
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addCheckItem(Gen::Writer->new()->writeLine('line1;'))
    ->addCheckItem(Gen::Writer->new()->writeLine('line2;'));

  assertFactoryEquals($factory, undef, {
    'checkItem' => <<LINES
/* public static */ bool
TestFooFactory::checkItem(const Foo& item)
{
  if (!&item)
    return false;
  line1;
  line2;
  return true;
}
LINES
  });
}

sub test_FactoryGenerator_getName
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addNameValue('"Foo_"')
    ->addNameField('number()');

  assertFactoryEquals($factory, undef, {
    'getName' => <<LINES
/* public static */ void
TestFooFactory::getName(std::ostringstream& os, const std::string& className, const Foo& item)
{
  os << "Foo_";
  os << item.number();
}
LINES
  });
}

sub test_FactoryGenerator_specialAggregate
{
  my $factory = Gen::FactoryGenerator->new({
    className => 'Foo'
  });
  $factory
    ->addAttribute('', 'bar', Gen::Accessor->new({
      name => 'bar', type => Gen::Type->new('int'),
      accessorType => $Gen::Accessor::ACCESSOR_INIT}))
    ->addAttribute('', 'baz', Gen::Accessor->new({
      name => 'baz', type => Gen::Type->new('double'),
      accessorType => $Gen::Accessor::ACCESSOR_INIT}))
    ->addSpecial('Special', 'name', 'Aggregate', Gen::LocalVariable->new({
        name => 'name', type => Gen::Type->new('unsigned')
      }))
    ->addSpecialRead('Special', 'name', 'Aggregate', Gen::Writer->new()
        ->writeLine('if (name == "right")')
        ->writeLine('  item->setRight(true);')
      )
    ->addSpecialWrite('Special', 'name', 'Aggregate', Gen::Writer->new()
        ->writeLine('if (item->isRight())')
        ->writeLine('  $write("right");')
      );

  assertFactoryEquals($factory, undef, {
    'init' => <<LINES
/* public static */ void
TestFooFactory::init(Foo* item, const std::string& fileName, TiXmlElement* rootElement, int flags)
{
  using namespace TestFactoryFlags;

  if (TestXMLHelper::AttributeExists(rootElement, "bar"))
  {
    TestXMLHelper::Attribute(rootElement, "bar", item->bar);
  }
  if (TestXMLHelper::AttributeExists(rootElement, "baz"))
  {
    TestXMLHelper::Attribute(rootElement, "baz", item->baz);
  }

  TiXmlNode* child = 0;
  while ((child = rootElement->IterateChildren(child)))
  {
    TiXmlElement* element = (TiXmlElement*)child;
    const std::string elementName = element->Value();

    if (false);
    else if (elementName == "Special")
    {
      unsigned name;
      TestXMLHelper::Attribute(element, "name", name);

      if (name == "right")
        item->setRight(true);
    }
  }
}
LINES
    , 'write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)' => <<LINES
/* public static */ void
TestFooFactory::write(const Foo& item, TiXmlElement* rootElement, const std::string& filePrefix)
{
  rootElement->SetAttribute("bar", TestXMLHelper::format(item.bar));
  rootElement->SetAttribute("baz", TestXMLHelper::format(item.baz));

  if (item->isRight())
    do {
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(
        new TiXmlElement("Special"));
      element->SetAttribute("name", "right");
    } while (0)
;
}
LINES
  });
}

registerTest('Gen::test::FactoryGeneratorTest');
