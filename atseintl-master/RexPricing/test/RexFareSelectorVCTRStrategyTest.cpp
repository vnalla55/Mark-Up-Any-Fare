#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "Common/VCTR.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexBaseRequest.h"
#include "RexPricing/RexFareSelectorVCTRStrategy.h"
#include "RexPricing/test/MockFareCompInfo.h"
#include "RexPricing/test/MockPaxTypeFareWrapper.h"
#include "RexPricing/test/RexFareSelectorStrategyTestUtils.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
using boost::assign::operator+=;

class RexFareSelectorVCTRStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorVCTRStrategyTest);

  CPPUNIT_TEST(testSelect_OneExactMatch_Pass);
  CPPUNIT_TEST(testSelect_OneExactMatch_SecondStepTcr);
  CPPUNIT_TEST(testSelect_OneNotExactMatch_Pass);
  CPPUNIT_TEST(testSelect_TwoExactMatch_Pass);
  CPPUNIT_TEST(testSelect_TwoNotExactMatch_Pass);
  CPPUNIT_TEST(testSelect_ExactMatchPrefered_Pass);
  CPPUNIT_TEST(testSelect_Fail);
  CPPUNIT_TEST(testSelect_Empty_Fail);

  CPPUNIT_TEST_SUITE_END();

private:
  static const MoneyAmount FAREAMT;

  RexPricingTrx* _trx;
  ExcItin* _excItin;
  VCTR _vctr;
  FareCompInfo* _fc;
  RexFareSelectorVCTRStrategy* _selector;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _excItin = _memH.create<ExcItin>();
    _excItin->calculationCurrencyNoDec() = 2;

    _trx = _memH.create<RexPricingTrx>();
    _trx->exchangeItin().push_back(_excItin);
    _trx->setRequest(_memH.create<RexBaseRequest>());
    _trx->getRequest()->ticketingAgent() = _memH.create<Agent>();
    _trx->getRequest()->ticketingAgent()->tvlAgencyPCC() = "A47";

    _vctr = VCTR("ATP", "AA", 100, "500", 0);
    _fc = _memH.create<MockFareCompInfo>(FAREAMT);
    _fc->VCTR() = _vctr;
    _selector = _memH.create<RexFareSelectorVCTRStrategy>(*_trx);
  }

  void tearDown() { _memH.clear(); }

  void testSelect_OneExactMatch_Pass()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 200, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect(1, fares[3]), preSelected(fares, fares + 5);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_OneExactMatch_SecondStepTcr()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 200, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 94, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect(1, fares[3]), preSelected(fares, fares + 5);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_OneNotExactMatch_Pass()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 200, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, 0.95 * FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect(1, fares[3]), preSelected(fares, fares + 5);

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_TwoExactMatch_Pass()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 5);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_TwoNotExactMatch_Pass()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, 1.05 * FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, 0.95 * FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 5);
    expect += fares[1], fares[3];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_ExactMatchPrefered_Pass()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(_vctr, 1.05 * FAREAMT), MockPaxTypeFareWrapper(_vctr, FAREAMT),
      MockPaxTypeFareWrapper(_vctr, 0.95 * FAREAMT), MockPaxTypeFareWrapper(_vctr, FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 5);
    expect += fares[2], fares[4];

    CPPUNIT_ASSERT(_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_Fail()
  {
    MockPaxTypeFareWrapper fares[] = {
      MockPaxTypeFareWrapper(VCTR("ATP", "LH", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 200, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("ATP", "AA", 100, "600", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "500", 0), FAREAMT),
      MockPaxTypeFareWrapper(VCTR("SITA", "AA", 100, "600", 0), FAREAMT)
    };

    std::vector<PaxTypeFareWrapper> result, expect, preSelected(fares, fares + 5);

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }

  void testSelect_Empty_Fail()
  {
    std::vector<PaxTypeFareWrapper> result, expect, preSelected;

    CPPUNIT_ASSERT(!_selector->select(*_fc, preSelected.begin(), preSelected.end(), result));
    CPPUNIT_ASSERT_EQUAL(expect, result);
  }
};

const MoneyAmount RexFareSelectorVCTRStrategyTest::FAREAMT = 200.13;

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorVCTRStrategyTest);

} // tse
