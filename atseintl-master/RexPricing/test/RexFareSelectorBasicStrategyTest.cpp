#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/PreSelectedFaresStore.h"
#include "RexPricing/RexFareSelectorBasicStrategy.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/MockPaxTypeFareWrapper.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

using boost::assign::operator+=;

class RexFareSelectorBasicStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorBasicStrategyTest);

  CPPUNIT_TEST(testGetTolerance_NumberDecimalPlaces0);
  CPPUNIT_TEST(testGetTolerance_NumberDecimalPlaces1);
  CPPUNIT_TEST(testGetTolerance_NumberDecimalPlaces2);
  CPPUNIT_TEST(testGetTolerance_NumberDecimalPlaces3);

  CPPUNIT_TEST(testSelect_Exact_Pass);
  CPPUNIT_TEST(testSelect_UpBorder_Pass);
  CPPUNIT_TEST(testSelect_DownBorder_Pass);

  CPPUNIT_TEST(testSelect_Fail);
  CPPUNIT_TEST(testSelect_UpBorder_Fail);
  CPPUNIT_TEST(testSelect_DownBorder_Fail);

  CPPUNIT_TEST(testSelect_MileageSurcharge_Pass);
  CPPUNIT_TEST(testSelect_MileageSurcharge_Fail);

  CPPUNIT_TEST(testSelect_EmptyVectors_Fail);

  CPPUNIT_TEST_SUITE_END();

private:
  static const MoneyAmount FAREAMT;

  RexPricingTrx* _trx;
  ExcItin* _excItin;
  RexFareSelectorBasicStrategy* _selector;
  double _tolerance;
  MockFareCompInfo* _fc;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _trx = _memH.create<RexPricingTrx>();
    _excItin = _memH.create<ExcItin>();
    _excItin->calculationCurrencyNoDec() = 2;
    _trx->exchangeItin().push_back(_excItin);

    _selector =
        _memH.create<RexFareSelectorBasicStrategy>(*_trx, *_memH.create<PreSelectedFaresStore>());
    _tolerance = _selector->getTolerance();
    _fc = _memH.create<MockFareCompInfo>(FAREAMT);
  }

  void tearDown()
  {
    _memH.clear();
  }

  void testGetTolerance_NumberDecimalPlaces0()
  {
    _excItin->calculationCurrencyNoDec() = 0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _selector->getTolerance(), EPSILON);
  }

  void testGetTolerance_NumberDecimalPlaces1()
  {
    _excItin->calculationCurrencyNoDec() = 1;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-1, _selector->getTolerance(), 1e-1 * EPSILON);
  }

  void testGetTolerance_NumberDecimalPlaces2()
  {
    _excItin->calculationCurrencyNoDec() = 2;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-2, _selector->getTolerance(), 1e-2 * EPSILON);
  }

  void testGetTolerance_NumberDecimalPlaces3()
  {
    _excItin->calculationCurrencyNoDec() = 3;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-3, _selector->getTolerance(), 1e-3 * EPSILON);
  }

  void testSelect_Exact_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5, FAREAMT };
    std::vector<PaxTypeFareWrapper> result, expect(1, fares[2]), preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_UpBorder_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5, FAREAMT + _tolerance };
    std::vector<PaxTypeFareWrapper> result, expect(1, fares[2]), preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_DownBorder_Pass()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5, FAREAMT - _tolerance };
    std::vector<PaxTypeFareWrapper> result, expect(1, fares[2]), preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_Fail()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5, FAREAMT * 0.99 };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_UpBorder_Fail()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5,
                                       FAREAMT + _tolerance + EPSILON };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_DownBorder_Fail()
  {
    MockPaxTypeFareWrapper fares[] = { FAREAMT * 1.01, FAREAMT * 1.5,
                                       FAREAMT - _tolerance - EPSILON };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_MileageSurcharge_Pass()
  {
    _fc->tktFareCalcFareAmt() = FAREAMT * 1.15;
    _fc->mileageSurchargePctg() = 15;

    MockPaxTypeFareWrapper fares[] = { FAREAMT * 0.5, FAREAMT * 2.0, FAREAMT };
    std::vector<PaxTypeFareWrapper> result, expect(1, fares[2]), preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_MileageSurcharge_Fail()
  {
    _fc->tktFareCalcFareAmt() = FAREAMT * 1.15;
    _fc->mileageSurchargePctg() = 15;

    MockPaxTypeFareWrapper fares[] = { FAREAMT * 0.5, FAREAMT * 2.0, FAREAMT * 1.1 };
    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 3);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_EmptyVectors_Fail()
  {
    std::vector<PaxTypeFareWrapper> result, expect, preSelected;

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }
};

const MoneyAmount RexFareSelectorBasicStrategyTest::FAREAMT = 200.13;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorBasicStrategyTest);

} // tse
