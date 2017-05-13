#ifndef FARE_MARKET_RULE_CONTROLLER_TEST
#define FARE_MARKET_RULE_CONTROLLER_TEST

#include "test/include/CppUnitHelperMacros.h"
#include "Rules/FareMarketRuleController.h"
#include "Rules/RuleConst.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleController.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
class MockFareMarketRuleController : public FareMarketRuleController
{
public:
  // using less dependency constructor, so we can easly create obj
  MockFareMarketRuleController(CategoryPhase phase, const std::vector<uint16_t>& categories)
    : FareMarketRuleController(phase, categories) {};
};
}

struct MockFareMarketDataAccess : public RuleControllerDataAccess
{
  MockFareMarketDataAccess(PaxTypeFare& ptFare) : _paxTypeFare(ptFare) {}

  Itin* itin() { return 0; }
  PaxTypeFare& paxTypeFare() const { return _paxTypeFare; }
  PricingTrx& trx() { return _trx; }

protected:
  PaxTypeFare& _paxTypeFare;
  PricingTrx _trx;
};

class FareMarketRuleControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareMarketRuleControllerTest);
  CPPUNIT_TEST(testCallCategoryRuleItemSet_Refund);
  CPPUNIT_TEST(testCallCategoryRuleItemSet_Rex);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _cats.clear();
    _fareMarketRuleController = _memHandle.create<MockFareMarketRuleController>
                                                 (VolunExcPrevalidation, _cats);
  }

  void tearDown() { _memHandle.clear(); }

  void testCallCategoryRuleItemSet_Refund()
  {
    CategoryRuleInfo ruleInfo;
    PaxTypeFare ptf;
    MockFareMarketDataAccess da(ptf);
    CategoryRuleItemSet catRuleIS;
    const std::vector<CategoryRuleItemInfoSet*> crInfo;
    RuleProcessingData rpData;

    // test core settings
    ptf.setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE, false);
    ruleInfo.categoryNumber() = RuleConst::VOLUNTARY_REFUNDS_RULE;

    Record3ReturnTypes ret = _fareMarketRuleController->callCategoryRuleItemSet(
        catRuleIS, ruleInfo, crInfo, da, rpData, true, true, true);

    CPPUNIT_ASSERT_EQUAL(SOFTPASS, ret);
    CPPUNIT_ASSERT(ptf.isCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE));
  }

  void testCallCategoryRuleItemSet_Rex()
  {
    CategoryRuleInfo ruleInfo;
    PaxTypeFare ptf;
    MockFareMarketDataAccess da(ptf);
    CategoryRuleItemSet catRuleIS;
    const std::vector<CategoryRuleItemInfoSet*> crInfo;
    RuleProcessingData rpData;

    // test core settings
    ptf.setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, false);
    ruleInfo.categoryNumber() = RuleConst::VOLUNTARY_EXCHANGE_RULE;

    Record3ReturnTypes ret = _fareMarketRuleController->callCategoryRuleItemSet(
        catRuleIS, ruleInfo, crInfo, da, rpData, true, true, true);

    CPPUNIT_ASSERT_EQUAL(SOFTPASS, ret);
    CPPUNIT_ASSERT(ptf.isCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE));
  }

private:
  TestMemHandle _memHandle;
  FareMarketRuleController* _fareMarketRuleController;
  std::vector<uint16_t> _cats;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareMarketRuleControllerTest);
} // end of tse namespace
#endif // FARE_MARKET_RULE_CONTROLLER_TEST
