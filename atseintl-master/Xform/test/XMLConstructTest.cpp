#include <iostream>
#include <fstream>
#include "Common/XMLConstruct.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{
class XMLConstructTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(XMLConstructTest);
  CPPUNIT_TEST(testOpenElement);
  CPPUNIT_TEST(testAddAttribute);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testOpenElement()
  //---------------------------------------------------------------------
  void testOpenElement()
  {
    XMLConstruct xml;
    xml.openElement("Foo");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.openElement("Bar");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.openElement("Foz");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.openElement("Baz");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(xml.isWellFormed());
    // std::cerr << xml.getXMLData() << std::endl;
    CPPUNIT_ASSERT(xml.getXMLData() == "<Foo><Bar><Foz/><Baz/></Bar></Foo>");
  }

  //---------------------------------------------------------------------
  // testOpenElement()
  //---------------------------------------------------------------------
  void testAddAttribute()
  {
    XMLConstruct xml;
    xml.openElement("Foo");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.addAttribute("Foz", "foz");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.openElement("Bar");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.addAttribute("Baz", "baz");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.addAttribute("Boz", "boz");
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(!xml.isWellFormed());
    xml.closeElement();
    CPPUNIT_ASSERT(xml.isWellFormed());
    // std::cerr << xml.getXMLData() << std::endl;
    CPPUNIT_ASSERT(xml.getXMLData() == "<Foo Foz=\"foz\"><Bar Baz=\"baz\" Boz=\"boz\"/></Foo>");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(XMLConstructTest);
}
