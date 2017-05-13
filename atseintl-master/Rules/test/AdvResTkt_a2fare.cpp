#include <cppunit/TestFixture.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Rules/AdvanceResTkt.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"
#include "Rules/RuleUtil.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/PricingTrx.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"
namespace tse
{

class AdvResTkt_a2fare : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(AdvResTkt_a2fare);

  CPPUNIT_TEST(testGetEligibleTktDatesWithFC_ExpectsOriginalTktDate);
  CPPUNIT_TEST(testGetEligibleTktDatesWithFC_ExpectsReissueDate);
  CPPUNIT_TEST(testGetEligibleTktDatesWithPU_ExpectsOriginalTktDate);
  CPPUNIT_TEST(testGetEligibleTktDatesWithPU_ExpectsReissueDate);
  CPPUNIT_TEST(testResBeforeDeparture);
  CPPUNIT_TEST(testResBeforeDeparture_ReissueBeforeOriginalTkt);
  CPPUNIT_TEST_SUITE_END();

  AdvanceResTkt* advResTkt;
  PricingUnit* pu1;
  FareUsage* fu1;
  AirSeg* airSeg0;
  AirSeg* airSeg1;
  FareMarket* fm1;
  Itin* itin;
  AirSeg* airSeg2;
  ExchangePricingTrx* trx;
  ExcItin* excItin;
  PricingOptions* options;
  FareCompInfo* fComp1;
  PricingRequest* request;
  PricingTrx* pricingTrx;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    advResTkt = _memHandle.create<AdvanceResTkt>();
    airSeg1 = createAirSegmentDFWJFK();
    fu1 = _memHandle.create<FareUsage>();
    fu1->inbound() = false;
    fu1->travelSeg().push_back(airSeg1);

    itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(airSeg1);
    excItin = _memHandle.create<ExcItin>();
    excItin->setItinIndex(0);
    excItin->travelSeg().push_back(airSeg1);
    excItin->stopOverChange() = false;

    fm1 = _memHandle.create<FareMarket>();
    fm1->direction() = FMDirection::OUTBOUND;
    fm1->travelSeg().push_back(airSeg1);
    fComp1 = _memHandle.create<FareCompInfo>();
    fComp1->fareMarket() = fm1;

    trx = _memHandle.create<ExchangePricingTrx>();
    trx->newItin().push_back(itin);
    trx->exchangeItin().push_back(excItin);
    options = _memHandle.create<PricingOptions>();
    trx->setOptions(options);
    trx->getOptions()->AdvancePurchaseOption() = ' ';
    request = _memHandle.create<PricingRequest>();
    trx->setRequest(request);
    trx->getRequest()->ticketingDT() = DateTime::localTime();
    trx->setOriginalTktIssueDT() = DateTime::localTime().subtractDays(7);

    pricingTrx = _memHandle.create<PricingTrx>();
    pricingTrx->setRequest(request);
    pricingTrx->getRequest()->ticketingDT() = DateTime::localTime();
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue(
        "INFINI_CAT05_OVERRIDE_BKG_ACTIVATION_DATE", "2013-06-18", "PRICING_SVC", true);
  }

  void tearDown() { _memHandle.clear(); }

protected:
  AirSeg* createAirSegmentDFWJFK()
  {
    AirSeg* segment = _memHandle.create<AirSeg>();
    segment->carrier() = CarrierCode("AA");
    segment->setMarketingCarrierCode(CarrierCode("AA"));
    segment->setOperatingCarrierCode(CarrierCode("AA"));
    segment->flightNumber() = FlightNumber(222);
    segment->marketingFlightNumber() = FlightNumber(222);
    segment->operatingFlightNumber() = FlightNumber(222);
    segment->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    segment->origAirport() = "DFW";
    segment->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    segment->origAirport() = "JFK";
    segment->setBookingCode("Y");
    segment->bookingDT() = DateTime::localTime().subtractDays(7);
    segment->departureDT() = DateTime::localTime();
    segment->arrivalDT() = segment->departureDT().addDays(1);
    segment->changeStatus() = TravelSeg::UNCHANGED;
    return segment;
  }

  void testGetEligibleTktDatesWithFC_ExpectsOriginalTktDate()
  {
    DateTime originalTktDate = trx->originalTktIssueDT();
    DateTime actuals[AdvanceResTkt::MAX_NUM_ELIGIBLE_TKTDT];

    advResTkt->getEligibleTktDates(*trx, *fm1, pu1, actuals);
    CPPUNIT_ASSERT_EQUAL(originalTktDate, actuals[0]);
  }

  void testGetEligibleTktDatesWithFC_ExpectsReissueDate()
  {
    fm1->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;
    DateTime current = trx->getRequest()->ticketingDT();
    DateTime actuals[AdvanceResTkt::MAX_NUM_ELIGIBLE_TKTDT];

    advResTkt->getEligibleTktDates(*trx, *fm1, 0, actuals);
    CPPUNIT_ASSERT_EQUAL(current, actuals[0]);
  }

  void testGetEligibleTktDatesWithPU_ExpectsOriginalTktDate()
  {
    pu1 = _memHandle.create<PricingUnit>();
    pu1->fareUsage().push_back(fu1);

    trx->exchangeItin().front()->fareComponent().push_back(fComp1);
    fu1->travelSeg().front()->changeStatus() = TravelSeg::UNCHANGED;
    DateTime originalTktDate = trx->originalTktIssueDT();
    DateTime actuals[AdvanceResTkt::MAX_NUM_ELIGIBLE_TKTDT];

    advResTkt->getEligibleTktDates(*trx, *fm1, pu1, actuals);
    CPPUNIT_ASSERT_EQUAL(originalTktDate, actuals[0]);
  }

  void testGetEligibleTktDatesWithPU_ExpectsReissueDate()
  {
    pu1 = _memHandle.create<PricingUnit>();
    pu1->fareUsage().push_back(fu1);

    trx->exchangeItin().front()->fareComponent().push_back(fComp1);
    fu1->travelSeg().front()->changeStatus() = TravelSeg::CHANGED;
    DateTime current = trx->getRequest()->ticketingDT();

    DateTime actuals[AdvanceResTkt::MAX_NUM_ELIGIBLE_TKTDT];

    advResTkt->getEligibleTktDates(*trx, *fm1, pu1, actuals);
    CPPUNIT_ASSERT_EQUAL(current, actuals[0]);
  }

  void testResBeforeDeparture()
  {

    AdvResTktInfo advResTktInfo;
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "011";
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.lastResTod() = 0;
    advResTktInfo.permitted() = ' ';

    DateTime bookingDT = DateTime::localTime().subtractDays(4);
    DateTime tktDT = DateTime::localTime().subtractDays(2);
    DateTime adjustedDateResRestriction = DateTime::localTime();
    DateTime refStartDT = DateTime::localTime().addDays(5);
    DateTime refEndDT = refStartDT;
    DateTime refOrigDT = refStartDT;
    DiagManager diag(*pricingTrx);

    bool mayPassAfterRebook = false;
    bool displayWqWarning = false;

    Record3ReturnTypes result = advResTkt->validateResTkt(bookingDT,
                                                          tktDT,
                                                          true,
                                                          false,
                                                          advResTktInfo,
                                                          false,
                                                          false,
                                                          refOrigDT,
                                                          refStartDT,
                                                          refEndDT,
                                                          pu1 = 0,
                                                          diag,
                                                          mayPassAfterRebook,
                                                          false,
                                                          false,
                                                          displayWqWarning,
                                                          adjustedDateResRestriction,
                                                          *pricingTrx);

    CPPUNIT_ASSERT_EQUAL(result, PASS);
  }

  void testResBeforeDeparture_ReissueBeforeOriginalTkt()
  {
    AdvResTktInfo advResTktInfo;
    advResTktInfo.firstResUnit() = "Dd";
    advResTktInfo.firstResPeriod() = "014";
    advResTktInfo.lastResUnit() = "";
    advResTktInfo.lastResPeriod() = "";
    advResTktInfo.lastResTod() = 0;
    advResTktInfo.permitted() = ' ';

    DateTime bookingDT = DateTime::localTime().subtractDays(4);
    DateTime tktDT = DateTime::localTime();
    DateTime adjustedDateResRestriction = DateTime::localTime().subtractDays(4);
    DateTime refStartDT = DateTime::localTime().addDays(5);
    DateTime refEndDT = refStartDT;
    DateTime refOrigDT = refStartDT;
    DiagManager diag(*pricingTrx);

    bool mayPassAfterRebook = false;
    bool displayWqWarning = false;
    Record3ReturnTypes result = advResTkt->validateResTkt(bookingDT,
                                                          tktDT,
                                                          true,
                                                          false,
                                                          advResTktInfo,
                                                          false,
                                                          false,
                                                          refOrigDT,
                                                          refStartDT,
                                                          refEndDT,
                                                          pu1 = 0,
                                                          diag,
                                                          mayPassAfterRebook,
                                                          false,
                                                          false,
                                                          displayWqWarning,
                                                          adjustedDateResRestriction,
                                                          *pricingTrx);

    CPPUNIT_ASSERT_EQUAL(result, PASS);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(AdvResTkt_a2fare);

}; // namespace
