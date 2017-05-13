#include "test/include/CppUnitHelperMacros.h"
#include <rapidxml_wrapper.hpp>
#include "TestServer/Xform/XmlTagsList.h"
#include "TestServer/Xform/XmlTagsFactory.h"
#include "TestServer/Xform/NaturalXmlTagsList.h"
#include "TestServer/Xform/Xml2TagsList.h"
#include "TestServer/Xform/SelectTagsList.h"

namespace tax
{

class SelectTagsListTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SelectTagsListTest);
  CPPUNIT_TEST(goodXml2Names);
  CPPUNIT_TEST(goodNaturalNames);
  CPPUNIT_TEST(emptyFile);
  CPPUNIT_TEST(unknownRootName);
  CPPUNIT_TEST(notAnXmlFile);
  CPPUNIT_TEST_SUITE_END();

public:

  SelectTagsListTest()
  {
    _xmlTagsFactory.registerList(new NaturalXmlTagsList);
    _xmlTagsFactory.registerList(new Xml2TagsList);
  }

  void goodXml2Names()
  {
    std::string xml = "<TAX><XXX/></TAX>";
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xml[0]);
    const XmlTagsList& tags = selectTagsList(parsedRequest, _xmlTagsFactory);
    CPPUNIT_ASSERT_EQUAL((std::string)"DI1", tags.getTagName<InputDiagnosticCommand>());
  }

  void goodNaturalNames()
  {
    std::string xml = "<TaxRq><Content/></TaxRq>";
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xml[0]);
    const XmlTagsList& tags = selectTagsList(parsedRequest, _xmlTagsFactory);
    CPPUNIT_ASSERT_EQUAL((std::string)"Diagnostic", tags.getTagName<InputDiagnosticCommand>());
  }

  void emptyFile()
  {
    std::string xml = "";
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xml[0]);
    CPPUNIT_ASSERT_THROW(selectTagsList(parsedRequest, _xmlTagsFactory), std::domain_error);
  }

  void unknownRootName()
  {
    std::string xml = "<BadRoot><Content/></BadRoot>";
    rapidxml::xml_document<> parsedRequest;
    parsedRequest.parse<0>(&xml[0]);
    CPPUNIT_ASSERT_THROW(selectTagsList(parsedRequest, _xmlTagsFactory), std::domain_error);
  }

  void notAnXmlFile()
  {
    std::string xml = "@@@$$$###";
    rapidxml::xml_document<> parsedRequest;
    CPPUNIT_ASSERT_THROW(parsedRequest.parse<0>(&xml[0]), rapidxml::parse_error);
  }

private:
  XmlTagsFactory _xmlTagsFactory;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SelectTagsListTest);

} // namespace Tax
