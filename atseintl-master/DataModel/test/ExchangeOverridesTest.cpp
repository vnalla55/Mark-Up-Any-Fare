#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/stack.hpp>
#include "test/include/TestMemHandle.h"

#include "DataModel/RefundPricingTrx.h"
#include "DataModel/ExchangeOverrides.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"

namespace tse
{

using namespace boost::assign;

class MockTrx : public RefundPricingTrx
{
public:
  virtual Money convertCurrency(const Money& source, const CurrencyCode& targetCurr) const
  {
    if (source.code() == "PLN" && targetCurr == NUC)
      return Money(source.value() * 0.5, targetCurr);

    if (source.code() == NUC && targetCurr == "PLN")
      return Money(source.value() * 2, targetCurr);

    return source;
  }
};

class ExchangeOverridesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ExchangeOverridesTest);

  CPPUNIT_TEST(testFillSurcharges_match);
  CPPUNIT_TEST(testFillSurcharges_noMatch);
  CPPUNIT_TEST(testFillStopovers_match);
  CPPUNIT_TEST(testFillStopovers_noMatch);
  CPPUNIT_TEST(testCreateOrderedTripChunks);
  CPPUNIT_TEST(testFillDifferentials_empty);
  CPPUNIT_TEST(testFillDifferentials_match);
  CPPUNIT_TEST(testFillDifferentials_InvMatch);
  CPPUNIT_TEST(testFillDifferentials_NoMatch);
  CPPUNIT_TEST(testFillDifferentials_SecondTimeNoMatch);
  CPPUNIT_TEST(testFillDifferentials_SecondTimeMatch);

  CPPUNIT_TEST_SUITE_END();

  ExchangeOverrides* _exOver;
  TestMemHandle _memory;
  FareUsage* _fu;
  std::stack<int16_t> _TSOrder;

public:
  void setUp()
  {
    _exOver = _memory.insert(new ExchangeOverrides(*_memory.insert(new RefundPricingTrx)));
    _fu = _memory.insert(new FareUsage);
  }

  void tearDown() { _memory.clear(); }

  template <class T>
  void setUpTvlSegments(std::vector<T*>& pluses, bool match = true)
  {
    _fu->travelSeg().push_back(_memory.insert(new AirSeg));
    _fu->travelSeg().push_back(_memory.insert(new AirSeg));
    pluses.push_back(_memory.insert(new T));
    pluses.push_back(_memory.insert(new T));

    if (match)
    {
      pluses.back()->fromExchange() = true;
      pluses.back()->amount() = 10.0;
      _fu->travelSeg().back()->origAirport() = LocCode("AAA");
      _fu->travelSeg().back()->destAirport() = LocCode("BBB");
      pluses.back()->travelSeg() = _fu->travelSeg().back();
    }

    _fu->travelSeg().push_back(_memory.insert(new AirSeg));
    pluses.push_back(_memory.insert(new T));
  }

  FarePath* createFarePath()
  {
    FarePath* fp = _memory.insert(new FarePath);
    fp->itin() = _memory.insert(new Itin);
    fp->itin()->calculationCurrency() = NUC;
    CPPUNIT_ASSERT_EQUAL(0.0, fp->getTotalNUCAmount());
    return fp;
  }

  void testFillSurcharges_match()
  {
    setUpTvlSegments(_exOver->_surchargeOverride);

    CPPUNIT_ASSERT(_fu->surchargeData().empty());
    _exOver->fillSurcharges(*_fu);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::vector<SurchargeData*>::size_type>(1),
                         _fu->surchargeData().size());
  }

  void testFillSurcharges_noMatch()
  {
    setUpTvlSegments(_exOver->_surchargeOverride, false);

    CPPUNIT_ASSERT(_fu->surchargeData().empty());
    _exOver->fillSurcharges(*_fu);
    CPPUNIT_ASSERT(_fu->surchargeData().empty());
  }

  void testFillStopovers_match()
  {
    setUpTvlSegments(_exOver->_stopoverOverride);
    FarePath* fp = createFarePath();

    CPPUNIT_ASSERT(_fu->stopoverSurcharges().empty());
    _exOver->fillStopovers(*_fu, *fp);
    CPPUNIT_ASSERT_EQUAL(static_cast<FareUsage::StopoverSurchargeMultiMap::size_type>(1),
                         _fu->stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(10.0, fp->getTotalNUCAmount());
  }

  void testFillStopovers_noMatch()
  {
    setUpTvlSegments(_exOver->_stopoverOverride, false);
    FarePath* fp = createFarePath();

    CPPUNIT_ASSERT(_fu->stopoverSurcharges().empty());
    _exOver->fillStopovers(*_fu, *fp);
    CPPUNIT_ASSERT(_fu->stopoverSurcharges().empty());
    CPPUNIT_ASSERT_EQUAL(0.0, fp->getTotalNUCAmount());
  }

  FareUsage* createFareUsage()
  {
    FareUsage* fu = _memory.insert(new FareUsage);
    fu->paxTypeFare() = _memory.insert(new PaxTypeFare);
    fu->paxTypeFare()->fareMarket() = _memory.insert(new FareMarket);
    fu->paxTypeFare()->fareMarket()->travelSeg().push_back(_memory.insert(new AirSeg));
    fu->paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder() = _TSOrder.top();
    _TSOrder.pop();
    return fu;
  }

  PricingUnit* createPricingUnit()
  {
    PricingUnit* pu = _memory.insert(new PricingUnit);
    pu->fareUsage().push_back(createFareUsage());
    pu->fareUsage().push_back(createFareUsage());
    pu->fareUsage().push_back(createFareUsage());
    return pu;
  }

  void testCreateOrderedTripChunks()
  {
    _TSOrder += 3, 5, 1, 0, 6, 2, 4, 7, 8;

    FarePath* fp = _memory.insert(new FarePath);
    fp->pricingUnit().push_back(createPricingUnit());
    fp->pricingUnit().push_back(createPricingUnit());
    fp->pricingUnit().push_back(createPricingUnit());

    CPPUNIT_ASSERT(_TSOrder.empty());

    ExchangeOverrides::OrderedTripChunks orderedTripChunks = _exOver->createOrderedTripChunks(*fp);

    ExchangeOverrides::OrderedTripChunks::iterator fui = orderedTripChunks.begin();
    for (int16_t order = 0; fui != orderedTripChunks.end(); ++fui, ++order)
      CPPUNIT_ASSERT_EQUAL(
          order, (**fui).paxTypeFare()->fareMarket()->travelSeg().front()->segmentOrder());
  }

  void testFillDifferentials_empty()
  {
    _TSOrder += 3;

    FareUsage* fu = createFareUsage();

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());
  }

  FareUsage* setUpForDiffs(LocCode origin, LocCode destination)
  {
    _TSOrder += 3;
    FareUsage* fu = createFareUsage();
    fu->paxTypeFare()->fareMarket()->travelSeg().push_back(_memory.insert(new AirSeg));
    fu->paxTypeFare()->fareMarket()->travelSeg().back()->boardMultiCity() = "AAA";
    fu->paxTypeFare()->fareMarket()->travelSeg().back()->offMultiCity() = "BBB";

    _exOver->differentialOverride().push_back(_memory.insert(new DifferentialOverride));
    _exOver->differentialOverride().front()->fromExchange() = true;
    _exOver->differentialOverride().front()->amount() = 10.0;
    _exOver->differentialOverride().front()->highDiffFareOrigin() = origin;
    _exOver->differentialOverride().front()->highDiffFareDestination() = destination;

    return fu;
  }

  void assertDiffs(FareUsage* fu, unsigned howMany)
  {
    CPPUNIT_ASSERT_EQUAL(static_cast<std::vector<DifferentialOverride*>::size_type>(howMany),
                         fu->differentialPlusUp().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, fu->differentialAmt(), EPSILON);
    CPPUNIT_ASSERT(_exOver->differentialOverride().front()->usedForRecreate());
  }

  void testFillDifferentials_match()
  {
    FareUsage* fu = setUpForDiffs("AAA", "BBB");

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    assertDiffs(fu, 1);
  }

  void testFillDifferentials_InvMatch()
  {
    FareUsage* fu = setUpForDiffs("BBB", "AAA");

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    assertDiffs(fu, 1);
  }

  void testFillDifferentials_NoMatch()
  {
    FareUsage* fu = setUpForDiffs("CCC", "AAA");

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());
  }

  void testFillDifferentials_SecondTimeNoMatch()
  {
    FareUsage* fu = setUpForDiffs("BBB", "AAA");

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));
    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    assertDiffs(fu, 1);
  }

  void testFillDifferentials_SecondTimeMatch()
  {
    FareUsage* fu = setUpForDiffs("BBB", "AAA");

    CPPUNIT_ASSERT(fu->differentialPlusUp().empty());

    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));
    _exOver->fill(*_memory.insert(new FarePath));
    _exOver->fillDifferentials(*fu, *_memory.insert(new FarePath));

    CPPUNIT_ASSERT_EQUAL(static_cast<std::vector<DifferentialOverride*>::size_type>(2),
                         fu->differentialPlusUp().size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExchangeOverridesTest);
}
