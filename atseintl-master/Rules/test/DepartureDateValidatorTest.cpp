#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/RexPricingTrx.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DBAccess/ReissueSequence.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PaxTypeFare.h"

#include "test/include/TestMemHandle.h"
#include "Rules/DepartureDateValidator.h"

namespace tse
{
using boost::assign::operator+=;

class DepartureDateValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DepartureDateValidatorTest);

  CPPUNIT_TEST(testCreate);
  CPPUNIT_TEST(testCreate_Default);
  CPPUNIT_TEST(testAssign);
  CPPUNIT_TEST(testHasSameMeasurePoint_PermutationsEmpty);
  CPPUNIT_TEST(testHasSameMeasurePoint_TagsEmpty);
  CPPUNIT_TEST(testHasSameMeasurePoint_AdvanceTicketingNotBlank);
  CPPUNIT_TEST(testHasSameMeasurePoint_DepartureDateNotFromNewTicket);
  CPPUNIT_TEST(testHasSameMeasurePoint_DepartureDateNotForFareComponent);
  CPPUNIT_TEST(testHasSameMeasurePoint_Pass);

  CPPUNIT_TEST(testGetDepartureDate_ExcItinPhase);
  CPPUNIT_TEST(testGetDepartureDate_IsNotValid);
  CPPUNIT_TEST(testGetDepartureDate_SegmentFlown);
  CPPUNIT_TEST(testGetDepartureDate_SegmentUnFlown);
  CPPUNIT_TEST(testGetDepartureDate_FareComponentVersusJourney);
  CPPUNIT_TEST(testGetDepartureDate_PricingUnitVersusJourney);
  CPPUNIT_TEST(testGetDepartureDate_JourneyVersusFareComponent);
  CPPUNIT_TEST(testGetDepartureDate_JourneyVersusPricingUnit);
  CPPUNIT_TEST(testGetDepartureDate_JourneyVersusFareComponent_EmptyDate);
  CPPUNIT_TEST(testGetDepartureDate_JourneyVersusPricingUnit_EmptyDate);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = createRexPricingTrx();
    _val = _memH(new DepartureDateValidator(*_trx));
  }

  void tearDown() { _memH.clear(); }

  RexPricingTrx* createRexPricingTrx() { return _memH(new RexPricingTrx); }

  void testCreate() { CPPUNIT_ASSERT(!_val->isValid()); }

  void testCreate_Default()
  {
    DepartureDateValidator val;
    CPPUNIT_ASSERT_EQUAL(static_cast<const RexPricingTrx*>(0), val._trx);
    CPPUNIT_ASSERT(!val.isValid());
  }

  void testAssign()
  {
    _val->_isValid = true;
    CPPUNIT_ASSERT(_val->isValid());
    CPPUNIT_ASSERT_EQUAL(static_cast<const RexPricingTrx*>(_trx), _val->_trx);

    const RexPricingTrx* trx = createRexPricingTrx();
    _val->assign(*trx);
    CPPUNIT_ASSERT(!_val->isValid());
    CPPUNIT_ASSERT_EQUAL(trx, _val->_trx);
  }

  ProcessTagPermutation* createProcessTagPermutation(ReissueSequence* seq = 0)
  {
    ProcessTagPermutation* perm = _memH(new ProcessTagPermutation);
    if (seq)
    {
      ProcessTagInfo* info = _memH(new ProcessTagInfo);
      info->reissueSequence()->orig() = seq;
      perm->processTags() += info;
    }
    return perm;
  }

  enum Secenario
  {
    ADVANCETICKETING_BLANK = 0,
    ADVANCETICKETING_NOTBLANK,
    FOR_FARECOMPONENT,
    FOR_PRICINGUNIT,
    FOR_JOURNEY,
    NOTFROM_NEWTICKET
  };

  ReissueSequence* createReissueSequence(Secenario s = FOR_FARECOMPONENT)
  {
    ReissueSequence* seq = _memH(new ReissueSequence);
    switch (s)
    {
    case ADVANCETICKETING_BLANK:
      break;
    case ADVANCETICKETING_NOTBLANK:
      seq->departureInd() = 'L';
      break;
    case FOR_FARECOMPONENT:
      seq->toAdvResInd() = 'F';
      break;
    case FOR_PRICINGUNIT:
      seq->toAdvResInd() = ' ';
      break;
    case FOR_JOURNEY:
      seq->toAdvResInd() = 'J';
      break;
    case NOTFROM_NEWTICKET:
      seq->fromAdvResInd() = 'O';
      break;
    }
    return seq;
  }

  typedef DepartureDateValidator::Permutations Permutations;

  Permutations& createPermutations(Secenario s = FOR_FARECOMPONENT)
  {
    Permutations& perm = *_memH(new Permutations);
    for (int i = 0; i < 5; ++i)
      perm += createProcessTagPermutation(createReissueSequence(s));
    return perm;
  }

  void testHasSameMeasurePoint_PermutationsEmpty()
  {
    Permutations permutations;
    Indicator point;
    CPPUNIT_ASSERT(!_val->hasSameMeasurePoint(permutations, point));
  }

  void testHasSameMeasurePoint_TagsEmpty()
  {
    Permutations& permutations = createPermutations();
    permutations += createProcessTagPermutation();
    Indicator point;
    CPPUNIT_ASSERT(!_val->hasSameMeasurePoint(permutations, point));
  }

  void testHasSameMeasurePoint_AdvanceTicketingNotBlank()
  {
    Permutations& permutations = createPermutations();
    permutations += createProcessTagPermutation(createReissueSequence(ADVANCETICKETING_NOTBLANK));
    Indicator point;
    CPPUNIT_ASSERT(!_val->hasSameMeasurePoint(permutations, point));
  }

  void testHasSameMeasurePoint_DepartureDateNotFromNewTicket()
  {
    Permutations& permutations = createPermutations();
    permutations += createProcessTagPermutation(createReissueSequence(NOTFROM_NEWTICKET));
    Indicator point;
    CPPUNIT_ASSERT(!_val->hasSameMeasurePoint(permutations, point));
  }

  void testHasSameMeasurePoint_DepartureDateNotForFareComponent()
  {
    Permutations& permutations = createPermutations();
    permutations += createProcessTagPermutation(createReissueSequence(FOR_PRICINGUNIT));
    Indicator point;
    CPPUNIT_ASSERT(!_val->hasSameMeasurePoint(permutations, point));
  }

  void testHasSameMeasurePoint_Pass()
  {
    Permutations& permutations = createPermutations();
    permutations += createProcessTagPermutation(createReissueSequence(FOR_FARECOMPONENT));
    Indicator point;
    CPPUNIT_ASSERT(_val->hasSameMeasurePoint(permutations, point));
    CPPUNIT_ASSERT_EQUAL(point, 'F');
  }

  enum SegmentStatus
  {
    FLOWN = 0,
    UNFLOWN
  };

  AirSeg* createAirSeg(SegmentStatus s)
  {
    AirSeg* seg = _memH(new AirSeg);
    seg->unflown() = s;
    return seg;
  }

  void setupForGetDepartureDateTest()
  {
    _trx->setOriginalTktIssueDT() = DateTime(2005, 10, 15);
    _trx->currentTicketingDT() = DateTime(2006, 12, 24);
    _trx->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    _val->_isValid = true;
  }

  void testGetDepartureDate_ExcItinPhase()
  {
    setupForGetDepartureDateTest();
    _trx->trxPhase() = RexBaseTrx::REPRICE_EXCITIN_PHASE;
    AirSeg& seg = *createAirSeg(FLOWN);
    CPPUNIT_ASSERT(_val->getDepartureDate(seg).isEmptyDate());
  }

  void testGetDepartureDate_IsNotValid()
  {
    setupForGetDepartureDateTest();
    _val->_isValid = false;
    AirSeg& seg = *createAirSeg(FLOWN);
    CPPUNIT_ASSERT(_val->getDepartureDate(seg).isEmptyDate());
  }

  void testGetDepartureDate_SegmentFlown()
  {
    setupForGetDepartureDateTest();
    AirSeg& seg = *createAirSeg(FLOWN);
    CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _val->getDepartureDate(seg));
  }

  void testGetDepartureDate_SegmentUnFlown()
  {
    setupForGetDepartureDateTest();
    AirSeg& seg = *createAirSeg(UNFLOWN);
    CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _val->getDepartureDate(seg));
  }

  Itin& createItin()
  {
    Itin* itin = _memH(new Itin);
    itin->travelSeg() += createAirSeg(FLOWN);
    return *itin;
  }

  FareMarket& createFareMarket()
  {
    FareMarket* fm = _memH(new FareMarket);
    fm->travelSeg() += createAirSeg(UNFLOWN);
    return *fm;
  }

  PricingUnit& createPricingUnit()
  {
    PricingUnit* pu = _memH(new PricingUnit);
    pu->fareUsage() += _memH(new FareUsage);
    pu->fareUsage().front()->paxTypeFare() = _memH(new PaxTypeFare);
    pu->fareUsage().front()->paxTypeFare()->fareMarket() = &createFareMarket();
    return *pu;
  }

  void testGetDepartureDate_FareComponentVersusJourney()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = 'F';
    CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(),
                         _val->getDepartureDate(createFareMarket(), createItin()));
  }

  void testGetDepartureDate_PricingUnitVersusJourney()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = ' ';
    CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(),
                         _val->getDepartureDate(createPricingUnit(), createItin()));
  }

  void testGetDepartureDate_JourneyVersusFareComponent()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = 'J';
    CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(),
                         _val->getDepartureDate(createFareMarket(), createItin()));
  }

  void testGetDepartureDate_JourneyVersusPricingUnit()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = 'J';
    CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(),
                         _val->getDepartureDate(createPricingUnit(), createItin()));
  }

  void testGetDepartureDate_JourneyVersusFareComponent_EmptyDate()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = ' ';
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(),
                         _val->getDepartureDate(createFareMarket(), createItin()));
  }

  void testGetDepartureDate_JourneyVersusPricingUnit_EmptyDate()
  {
    setupForGetDepartureDateTest();
    _val->_toAdvResMeasurePoint = 'F';
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(),
                         _val->getDepartureDate(createPricingUnit(), createItin()));
  }

private:
  TestMemHandle _memH;
  DepartureDateValidator* _val;
  RexPricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DepartureDateValidatorTest);
}
