#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/PricingTrx.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcHelper.h"

namespace tse
{
class AdjustedSellingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdjustedSellingUtilTest);
  CPPUNIT_TEST(testGetADJSellingLevelOrgMessage);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetADJSellingLevelOrgMessage()
  {
    PricingTrx trx;
    CalcTotals totals;

    std::string msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    PricingOptions options;
    trx.setOptions(&options);

    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    options.setPDOForFRRule(true);
    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    CalcTotals adjustedCalcTotals;
    totals.adjustedCalcTotal = &adjustedCalcTotals;

    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a1);

    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg == "ADJT AMT  100.00");

    options.setPDOForFRRule(false);
    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    options.setPDOForFRRule(true);
    totals.adjustedCalcTotal = 0;
    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg.empty());

    totals.adjustedCalcTotal = &adjustedCalcTotals;
    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg == "ADJT AMT  100.00");

    AdjustedSellingDiffInfo a2("A2", "G", "88.88");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a2);

    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg == "ADJT AMT  100.00/88.88A2");

    AdjustedSellingDiffInfo a3("X2", "G", "7.7");
    adjustedCalcTotals.adjustedSellingDiffInfo.push_back(a3);

    msg = AdjustedSellingUtil::getADJSellingLevelOrgMessage(trx, totals);
    CPPUNIT_ASSERT(msg == "ADJT AMT  100.00/88.88A2/7.7X2");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AdjustedSellingUtilTest);
}
