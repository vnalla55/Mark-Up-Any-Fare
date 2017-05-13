#include "test/include/CppUnitHelperMacros.h"
#include "Pricing/test/PricingOrchestratorTestShoppingCommon.h"

using namespace std;
namespace tse
{
class PQDiversifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PQDiversifierTest);
  CPPUNIT_TEST(testCalculateNumOfSolutionsNeededForDatePair_noSolutions);
  CPPUNIT_TEST(testCalculateNumOfSolutionsNeededForDatePair_oneOnlineSolutions);
  CPPUNIT_TEST(testCalculateNumOfSolutionsNeededForDatePair_twoOnlineFourInterlineSolutions);
  CPPUNIT_TEST(testCalculateNumOfSolutionsNeededForDatePair_noOnlineFourInterlineSolutions);
  CPPUNIT_TEST(testAllocateExtraOptions);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCalculateNumOfSolutionsNeededForDatePair_noSolutions()
  {
    results.numberInterlineOptions = 0;
    results._numberOnlineOptionsPerCarrier = 0;

    PricingOptions options;
    options.setEnableCalendarForInterlines(false);

    uint32_t result = diversifier.calculateNumOfSolutionNeededForDatePair(&options, results);

    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(result));
  }

  void testCalculateNumOfSolutionsNeededForDatePair_oneOnlineSolutions()
  {
    results.numberInterlineOptions = 0;
    results._numberOnlineOptionsPerCarrier = 1;

    PricingOptions options;
    options.setEnableCalendarForInterlines(false);

    uint32_t result = diversifier.calculateNumOfSolutionNeededForDatePair(&options, results);

    CPPUNIT_ASSERT_EQUAL(1, static_cast<int>(result));
  }

  void testCalculateNumOfSolutionsNeededForDatePair_twoOnlineFourInterlineSolutions()
  {
    results.numberInterlineOptions = 4;
    results._numberOnlineOptionsPerCarrier = 2;

    PricingOptions options;
    options.setEnableCalendarForInterlines(false);

    uint32_t result = diversifier.calculateNumOfSolutionNeededForDatePair(&options, results);

    CPPUNIT_ASSERT_EQUAL(2, static_cast<int>(result));
  }

  void testCalculateNumOfSolutionsNeededForDatePair_noOnlineFourInterlineSolutions()
  {
    results.numberInterlineOptions = 4;
    results._numberOnlineOptionsPerCarrier = 0;

    PricingOptions options;
    options.setEnableCalendarForInterlines(false);

    uint32_t result = diversifier.calculateNumOfSolutionNeededForDatePair(&options, results);

    CPPUNIT_ASSERT_EQUAL(4, static_cast<int>(result));
  }

  void testAllocateExtraOptions()
  {
    ShoppingTrx::PQDiversifierResult res;
    ShoppingTrx::CxrOnlinOptions& aa = res.cxrOnlinOptions["AA"];
    ShoppingTrx::CxrOnlinOptions& lh = res.cxrOnlinOptions["LH"];
    ShoppingTrx::CxrOnlinOptions& ba = res.cxrOnlinOptions["BA"];

    res.totalOnlineOptions = 30000;
    res._numberOnlineOptionsPerCarrier = 10000;

    aa.numberOnlineOptions = 10000;
    lh.numberOnlineOptions = 10000;
    ba.numberOnlineOptions = 10000;

    aa.maxPossibleCombinations = 10000;
    lh.maxPossibleCombinations = 55000;
    ba.maxPossibleCombinations = 45000;

    diversifier.allocateExtraOptions(res, 1000);

    CPPUNIT_ASSERT_EQUAL(31000u, res.totalOnlineOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10000u), aa.numberOnlineOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10550u), lh.numberOnlineOptions);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(10450u), ba.numberOnlineOptions);
  }

protected:
  ShoppingTrx::PQDiversifierResult results;
  PQDiversifier diversifier;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PQDiversifierTest);
}
