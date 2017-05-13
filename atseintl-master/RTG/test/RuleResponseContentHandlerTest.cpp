#include "RTG/RTGController.h"
#include "Common/XMLChString.h"
#include "DataModel/FareDisplayTrx.h"
#include <xercesc/sax2/Attributes.hpp>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class RuleResponseContentHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RuleResponseContentHandlerTest);
  CPPUNIT_TEST(testResponse);
  CPPUNIT_TEST(testTooLongResponse);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

  RTGController* _controller;
  FareDisplayTrx* _trx;
  std::string _response;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _controller = _memHandle.insert(new RTGController(*_trx));
  }

  void tearDown() { _memHandle.clear(); }
  void testTooLongResponse()
  {
    std::string xmlRequest = "<GenerateBundledRuleResponse><GenerateRuleResponse "
                             "S31="
                             "\"0123456789012345678901234567890123456789012345678901234567890123\" "
                             "Q3Y=\"-1\" /></GenerateBundledRuleResponse>";
    std::string response;
    bool result = _controller->parse(xmlRequest, response);
    CPPUNIT_ASSERT_EQUAL(true, result);
    std::string expected = "RTG SERVER ERROR";
    CPPUNIT_ASSERT_EQUAL(expected, response);
  }

  void testResponse()
  {

    std::string xmlRequest = "<GenerateBundledRuleResponse><GenerateRuleResponse S31=\"RULE DATA "
                             "INCOMPLETE\" Q3Y=\"-1\" /></GenerateBundledRuleResponse>";
    std::string response;

    bool result = _controller->parse(xmlRequest, response);
    CPPUNIT_ASSERT_EQUAL(true, result);
    std::string expected = "RULE DATA INCOMPLETE";
    CPPUNIT_ASSERT_EQUAL(expected, response);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(RuleResponseContentHandlerTest);
}
