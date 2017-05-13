//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/TseStlTypes.h"
#include "Common/XMLConstruct.h"
#include "Xform/CommonFormattingUtils.h"
#include "Xform/XMLCommonTags.h"
#include "Xform/PricingResponseXMLTags.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <string>
#include <vector>

namespace tse
{

// CPPUNIT_ASSERT_EQUAL(expected, actual)
// CPPUNIT_ASSERT(bool)
// CPPUNIT_ASSERT_THROW(cmd, exception_type)

class CommonFormattingUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CommonFormattingUtilsTest);
  CPPUNIT_TEST(testAddAttributeWithXMLWriter);
  CPPUNIT_TEST(testAddAttributeWithXMLConstruct);
  CPPUNIT_TEST(testAddAttributeWithDiagnostic);
  CPPUNIT_TEST(testAddAttributeWithVectorOfStringsConstruct);
  CPPUNIT_TEST(testFormattingWithXMLWriter);
  CPPUNIT_TEST(testFormattingWithXMLConstruct);
  CPPUNIT_TEST(testFormattingWithDiagnostic);
  CPPUNIT_TEST(testFormattingWithVectorOfStringsConstruct);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    BrandInfo* bInfo = _memHandle.create<BrandInfo>();
    bInfo->brandCode() = "BCODE";
    bInfo->brandName() = "BNAME";
    BrandProgram* bProgram = _memHandle.create<BrandProgram>();
    bProgram->brandsData().push_back(bInfo);
    bProgram->programCode() = "PCODE";
    bProgram->programName() = "PNAME";
    bProgram->programID() = "PID";
    bProgram->systemCode() = 'S';
    _qualifiedBrand = std::make_pair(bProgram, bInfo);
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testAddAttributeWithXMLWriter()
  {
    XMLWriter writer;
    XMLWriter::Node node(writer, "NODE");
    xform::addAttribute(node, "TEST_KEY", "TEST_VALUE");
    std::string xml = writer.result();
    CPPUNIT_ASSERT(xml.find("TEST_KEY=\"TEST_VALUE\"") != std::string::npos);
  }

  void testAddAttributeWithXMLConstruct()
  {
    XMLConstruct construct;
    construct.openElement("NODE");
    xform::addAttribute(construct, "TEST_KEY", "TEST_VALUE");
    construct.closeElement();
    std::string xml = construct.getXMLData();
    CPPUNIT_ASSERT(xml.find("TEST_KEY=\"TEST_VALUE\"") != std::string::npos);
  }

  void testAddAttributeWithDiagnostic()
  {
    DiagCollector* diag = _memHandle.create<DiagCollector>();
    CPPUNIT_ASSERT(diag != nullptr);
    diag->activate();
    xform::addAttribute(*diag, "TEST_KEY", "TEST_VALUE");
    std::string diagStr = diag->str();
    CPPUNIT_ASSERT(diagStr.find("TEST_KEY=TEST_VALUE") != std::string::npos);
  }

  void testAddAttributeWithVectorOfStringsConstruct()
  {
    std::vector<std::string> container;
    xform::addAttribute(container, "TEST_KEY", "TEST_VALUE");
    CPPUNIT_ASSERT_EQUAL(size_t(1), container.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TEST_KEY=TEST_VALUE"), container[0]);
  }

  void testFormattingWithDiagnostic()
  {
    DiagCollector* diag = _memHandle.create<DiagCollector>();
    CPPUNIT_ASSERT(diag != nullptr);
    diag->activate();
    xform::formatBrandProgramData(*diag, _qualifiedBrand);
    std::string diagStr = diag->str();
    CPPUNIT_ASSERT(diagStr.find("SB2=BCODE") != std::string::npos);
    CPPUNIT_ASSERT(diagStr.find("SB3=BNAME") != std::string::npos);
    CPPUNIT_ASSERT(diagStr.find("SC2=PNAME") != std::string::npos);
    CPPUNIT_ASSERT(diagStr.find("SC1=S") != std::string::npos);
    CPPUNIT_ASSERT(diagStr.find("SC3=PID") != std::string::npos);
    CPPUNIT_ASSERT(diagStr.find("SC4=PCODE") != std::string::npos);
  }

  void testFormattingWithXMLWriter()
  {
    XMLWriter writer;
    XMLWriter::Node nodeFDC(writer, "FDC");
    xform::formatBrandProgramData(nodeFDC, _qualifiedBrand);
    std::string xml = writer.result();
    CPPUNIT_ASSERT(xml.find("SB2=\"BCODE\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SB3=\"BNAME\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC2=\"PNAME\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC1=\"S\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC3=\"PID\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC4=\"PCODE\"") != std::string::npos);
  }

  void testFormattingWithXMLConstruct()
  {
    XMLConstruct construct;
    construct.openElement(xml2::FareCalcInformation);
    xform::formatBrandProgramData(construct, _qualifiedBrand);
    construct.closeElement();
    std::string xml = construct.getXMLData();
    CPPUNIT_ASSERT(xml.find("SB2=\"BCODE\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SB3=\"BNAME\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC2=\"PNAME\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC1=\"S\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC3=\"PID\"") != std::string::npos);
    CPPUNIT_ASSERT(xml.find("SC4=\"PCODE\"") != std::string::npos);
  }

  void testFormattingWithVectorOfStringsConstruct()
  {
    std::vector<std::string> construct;

    std::vector<std::string> expected;
    expected.push_back("SB2=BCODE");
    expected.push_back("SB3=BNAME");
    expected.push_back("SC2=PNAME");
    expected.push_back("SC1=S");
    expected.push_back("SC3=PID");
    expected.push_back("SC4=PCODE");

    xform::formatBrandProgramData(construct, _qualifiedBrand);
    CPPUNIT_ASSERT_EQUAL(expected.size(), construct.size());
    for (size_t index = 0; index < expected.size(); ++index)
      CPPUNIT_ASSERT_EQUAL(expected[index], construct[index]);
  }

private:
  TestMemHandle _memHandle;
  QualifiedBrand _qualifiedBrand;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CommonFormattingUtilsTest);

} // namespace tse
