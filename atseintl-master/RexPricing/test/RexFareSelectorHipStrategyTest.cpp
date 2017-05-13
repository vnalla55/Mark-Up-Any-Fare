#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/RexPricingTrx.h"
#include "RexPricing/RexFareSelectorHipStrategy.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/MockPaxTypeFareWrapper.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class RexFareSelectorHipStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorHipStrategyTest);

  CPPUNIT_TEST(testSelect_Pass);
  CPPUNIT_TEST(testSelect_Fail);

  CPPUNIT_TEST_SUITE_END();

private:
  static const MoneyAmount FAREAMT;

  RexPricingTrx* _trx;
  RexFareSelectorHipStrategy* _selector;
  FareCompInfo* _fc;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _trx = _memH.create<RexPricingTrx>();
    _selector = _memH.create<RexFareSelectorHipStrategy>(*_trx);
    _fc = _memH.create<MockFareCompInfo>(FAREAMT);
  }

  void tearDown() { _memH.clear(); }

  void testSelect_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.1, FAREAMT * 2.0, FAREAMT * 0.7,
                                       FAREAMT * 0.75 };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);
    expect += fares[2], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_Fail()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.1, FAREAMT * 2.0, FAREAMT * 1.9, FAREAMT * 1.5 };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }
};

const MoneyAmount RexFareSelectorHipStrategyTest::FAREAMT = 200.13;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorHipStrategyTest);

} // tse
