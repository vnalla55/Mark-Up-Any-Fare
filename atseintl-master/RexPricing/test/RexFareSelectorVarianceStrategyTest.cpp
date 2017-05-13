#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/RexPricingTrx.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/RexFareSelectorVarianceStrategy.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/MockPaxTypeFareWrapper.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class RexFareSelectorVarianceStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorVarianceStrategyTest);

  CPPUNIT_TEST(testSelect_LowVariance_Pass);
  CPPUNIT_TEST(testSelect_UpVariance_Pass);
  CPPUNIT_TEST(testSelect_MiddleVariance_Pass);
  CPPUNIT_TEST(testSelect_MileageSurcharge_Pass);
  CPPUNIT_TEST(testSelect_EmptyVectors_Fail);

  CPPUNIT_TEST_SUITE_END();

private:
  static const MoneyAmount FAREAMT;

  PreSelectedFaresStore _store;
  RexPricingTrx* _trx;
  RexFareSelectorVarianceStrategy* _selector;
  FareCompInfo* _fc;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _trx = _memH.create<RexPricingTrx>();
    _selector = _memH.create<RexFareSelectorVarianceStrategy>(*_trx, _store);
    _fc = _memH.create<MockFareCompInfo>(FAREAMT);
  }

  void tearDown() { _memH.clear(); }

  void testSelect_LowVariance_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 2.0, FAREAMT * 1 / 1.09, FAREAMT * 0.5,
                                       FAREAMT * 1 / 1.09 };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_UpVariance_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 2.0, FAREAMT * 1.09, FAREAMT * 0.5,
                                       FAREAMT * 1.09 };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_MiddleVariance_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 2.0, FAREAMT * 0.95, FAREAMT * 0.5,
                                       FAREAMT * 1.05 };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_MileageSurcharge_Pass()
  {
    _fc->tktFareCalcFareAmt() = FAREAMT * 1.15;
    _fc->mileageSurchargePctg() = 15;
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 2.0, FAREAMT * 1.09, FAREAMT * 0.5,
                                       FAREAMT * 1 / 1.09 };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 4);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_EmptyVectors_Fail()
  {
    std::vector<PaxTypeFareWrapper> result, expect, preSelected;

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }
};

const MoneyAmount RexFareSelectorVarianceStrategyTest::FAREAMT = 200.13;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorVarianceStrategyTest);

} // tse
