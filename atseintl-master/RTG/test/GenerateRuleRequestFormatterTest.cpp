#include "RTG/GenerateRuleRequestFormatter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class GenerateRuleRequestFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GenerateRuleRequestFormatterTest);
  CPPUNIT_TEST(testOWRTIndicatorOneWayMayBeDoubled);
  CPPUNIT_TEST(testOWRTIndicatorOneWayMayNotBeDoubled);
  CPPUNIT_TEST(testOWRTIndicatorRoundTrip);
  CPPUNIT_TEST(testOWRTIndicatorAllWays);
  CPPUNIT_TEST(testOWRTIndicatorOther);
  CPPUNIT_TEST_SUITE_END();

  GenerateRuleRequestFormatter* _formatter;
  TestMemHandle _memHandle;

public:
  void setUp() { _formatter = _memHandle.insert(new GenerateRuleRequestFormatter(false)); }

  void tearDown() { _memHandle.clear(); }

  void testOWRTIndicatorOneWayMayBeDoubled()
  {
    std::string expected = "<DIS P04=\"X\" B40=\"JRT\" B70=\"ADT\"/>";
    XMLConstruct construct(50);
    std::string fareBasis("JRT");
    Indicator owrt = ONE_WAY_MAY_BE_DOUBLED;
    const PaxTypeCode paxTypeCode = "ADT";
    _formatter->addDISType(fareBasis, owrt, paxTypeCode, construct);
    std::string xmlRequest = construct.getXMLData();
    CPPUNIT_ASSERT_EQUAL(expected, xmlRequest);
  }

  void testOWRTIndicatorOneWayMayNotBeDoubled()
  {
    std::string expected = "<DIS P04=\"O\" B40=\"JRT\" B70=\"ADT\"/>";
    XMLConstruct construct(50);
    std::string fareBasis("JRT");
    Indicator owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    const PaxTypeCode paxTypeCode = "ADT";
    _formatter->addDISType(fareBasis, owrt, paxTypeCode, construct);
    std::string xmlRequest = construct.getXMLData();
    CPPUNIT_ASSERT_EQUAL(expected, xmlRequest);
  }

  void testOWRTIndicatorRoundTrip()
  {
    std::string expected = "<DIS P04=\"R\" B40=\"JRT\" B70=\"ADT\"/>";
    XMLConstruct construct(50);
    std::string fareBasis("JRT");
    Indicator owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    const PaxTypeCode paxTypeCode = "ADT";
    _formatter->addDISType(fareBasis, owrt, paxTypeCode, construct);
    std::string xmlRequest = construct.getXMLData();
    CPPUNIT_ASSERT_EQUAL(expected, xmlRequest);
  }

  void testOWRTIndicatorAllWays()
  {
    std::string expected = "<DIS P04=\"H\" B40=\"JRT\" B70=\"ADT\"/>";
    XMLConstruct construct(50);
    std::string fareBasis("JRT");
    Indicator owrt = ALL_WAYS;
    const PaxTypeCode paxTypeCode = "ADT";
    _formatter->addDISType(fareBasis, owrt, paxTypeCode, construct);
    std::string xmlRequest = construct.getXMLData();
    CPPUNIT_ASSERT_EQUAL(expected, xmlRequest);
  }

  void testOWRTIndicatorOther()
  {
    std::string expected = "<DIS P04=\"H\" B40=\"JRT\" B70=\"ADT\"/>";
    XMLConstruct construct(50);
    std::string fareBasis("JRT");
    Indicator owrt = '9';
    const PaxTypeCode paxTypeCode = "ADT";
    _formatter->addDISType(fareBasis, owrt, paxTypeCode, construct);
    std::string xmlRequest = construct.getXMLData();
    CPPUNIT_ASSERT_EQUAL(expected, xmlRequest);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(GenerateRuleRequestFormatterTest);
}
