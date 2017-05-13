#include "RTG/RTGController.h"
#include "Common/XMLChString.h"
#include <xercesc/sax2/Attributes.hpp>
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FBDisplay.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{

class TestRTGController : public RTGController
{
public:
  TestRTGController(FareDisplayTrx& trx) : RTGController(trx) {}
  RuleTextMap& ruleTextMap() { return _ruleTextMap; }
};

class Category27ResponseTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Category27ResponseTest);
  CPPUNIT_TEST(testGeneralRule);
  CPPUNIT_TEST(testFareRule);
  CPPUNIT_TEST(testGeneralFareRule);
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

  TestRTGController* _controller;
  FareDisplayTrx* _trx;
  FBDisplay* _fbDisplay;

  std::string _response;

  static const std::string _generalRule;
  static const std::string _fareRule;

  static const std::string RESP1;
  static const std::string RESP2;
  static const std::string RESP3;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _trx->setRequest(_memHandle.create<FareDisplayRequest>());
    _trx->getRequest()->ticketingDT() = DateTime::localTime().nextDay();
    _controller = _memHandle.insert(new TestRTGController(*_trx));
    _fbDisplay = _memHandle.create<FBDisplay>();
  }

  void tearDown() { _memHandle.clear(); }

  void testGeneralRule()
  {
    _controller->ruleTextMap().insert(
        std::make_pair(std::make_pair(27, GeneralRule), _generalRule));

    _controller->updateFareDisplayRuleTextMap(*_fbDisplay, false, false);
    _response = _fbDisplay->ruleTextMap()[27];

    CPPUNIT_ASSERT_EQUAL(_response, RESP1);
  }

  void testFareRule()
  {
    _controller->ruleTextMap().insert(std::make_pair(std::make_pair(27, FareRule), _fareRule));

    _controller->updateFareDisplayRuleTextMap(*_fbDisplay, false, false);
    _response = _fbDisplay->ruleTextMap()[27];

    CPPUNIT_ASSERT_EQUAL(_response, RESP2);
  }

  void testGeneralFareRule()
  {
    _controller->ruleTextMap().insert(std::make_pair(std::make_pair(27, FareRule), _fareRule));
    _controller->ruleTextMap().insert(
        std::make_pair(std::make_pair(27, GeneralRule), _generalRule));

    _controller->updateFareDisplayRuleTextMap(*_fbDisplay, false, false);
    _response = _fbDisplay->ruleTextMap()[27];

    CPPUNIT_ASSERT_EQUAL(_response, RESP3);
  }
};
const std::string Category27ResponseTest::_generalRule = "GENERAL RULE TEXT";
const std::string Category27ResponseTest::_fareRule = "FARE RULE TEXT";

const std::string Category27ResponseTest::RESP1 = "\n   NOTE - THE FOLLOWING TEXT IS INFORMATIONAL "
                                                  "AND NOT VALIDATED\n   FOR AUTOPRICING.GENERAL "
                                                  "RULE TEXT";
const std::string Category27ResponseTest::RESP2 = "FARE RULE TEXT";

const std::string Category27ResponseTest::RESP3 =
    "   FARE RULE\nFARE RULE TEXT\n \n   GENERAL RULE\n   NOTE - THE FOLLOWING TEXT IS "
    "INFORMATIONAL AND NOT VALIDATED\n   FOR AUTOPRICING.GENERAL RULE TEXT";

CPPUNIT_TEST_SUITE_REGISTRATION(Category27ResponseTest);
}
