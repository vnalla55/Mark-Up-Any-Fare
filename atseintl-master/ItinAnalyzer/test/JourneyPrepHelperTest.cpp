#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "ItinAnalyzer/JourneyPrepHelper.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using boost::assign::operator+=;
using boost::assign::list_of;

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    CarrierPreference* cxrPref = _memHandle.create<CarrierPreference>();
    cxrPref->activateJourneyShopping() = YES;
    cxrPref->activateJourneyPricing() = YES;
    return cxrPref;
  }
};
}

class JourneyPrepHelperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(JourneyPrepHelperTest);

  CPPUNIT_TEST(testAllSegsMarriedReturnFalseWhenFareMarketHasLessThan2Segs);
  CPPUNIT_TEST(testAllSegsMarriedReturnFalseWhenNotSameCarriers);
  CPPUNIT_TEST(testAllSegsMarriedReturnFalseWhenMarriageDontStartAtFirstSeg);
  CPPUNIT_TEST(testAllSegsMarriedReturnFalseWhenMarriageDontEndAtLastSeg);
  CPPUNIT_TEST(testAllSegsMarriedReturnFalseWhenAllIntermediateSegsNotPartOfMarriage);
  CPPUNIT_TEST(testAllSegsMarriedReturnTrueWhenTwoSegsFormMarriage);
  CPPUNIT_TEST(testAllSegsMarriedReturnTrueWhenThreeSegsFormMarriage);

  CPPUNIT_TEST(testDuplicateFareMarketReturnFalseWhenVectorEmpty);
  CPPUNIT_TEST(testDuplicateFareMarketReturnTrueWhenSamePointerInVector);
  CPPUNIT_TEST(testDuplicateFareMarketReturnTrueWhenSameTravelSegsVector);
  CPPUNIT_TEST(testDuplicateFareMarketReturnFalseWhenFareMarketNotFound);

  CPPUNIT_TEST(testMarkMarriedMarketsDontMarkAnyWhenNotWpnc);
  CPPUNIT_TEST(testMarkMarriedMarketsDontMarkAnyWhenNoItinInTrx);
  CPPUNIT_TEST(testMarkMarriedMarketsDontMarkAnyWhenItinPointerZero);
  CPPUNIT_TEST(testMarkMarriedMarketsPositive);
  CPPUNIT_TEST(testMarkMarriedMarketsDontMarkAnyWhenNotPricingOrMipTrx);
  CPPUNIT_TEST(testMarkMarriedMarketsDontMarkAnyWhenMIPandNotWpniCurrentItin);
  CPPUNIT_TEST(testMarkMarriedMarketsPositiveWhenMIPandWpniCurrentItin);

  CPPUNIT_TEST(testMarkFlowMarketsFindsJourneyFareMarkets);
  CPPUNIT_TEST(testMarkFlowMarketsDontFindJourneyFareMarkets);

  CPPUNIT_TEST(testInitAvailBreakSetsAvailBreaks);
  CPPUNIT_TEST(testInitAvailBreak_SoloPricingCarrier_WPNC);
  CPPUNIT_TEST(testInitAvailBreak_NonSoloPricingCarrier_WPNC);

  CPPUNIT_TEST_SUITE_END();

public:
  typedef std::vector<TravelSeg*> TravelSegs;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pricingTrx = _memHandle.create<PricingTrx>();
    _newItin = _memHandle.create<Itin>();
    _request = _memHandle.create<PricingRequest>();
    _option = _memHandle.create<PricingOptions>();
    _memHandle.create<MyDataHandle>();

    _pricingTrx->setRequest(_request);
  }

  void tearDown()
  {
    _memHandle.clear();
    // TrxUtil::interlineAvailabilityCarriers.clear();
    // TrxUtil::interlineAvailabilityCarriersConfigured = false;
  }

  bool allSegsMarried(FareMarket& fm)
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->journeyActivatedForPricing() = true;
    Itin itin;
    itin.fareMarket().push_back(&fm);
    _pricingTrx->itin().push_back(&itin);
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);

    return fm.hasAllMarriedSegs();
  }

  void testAllSegsMarriedReturnFalseWhenFareMarketHasLessThan2Segs()
  {
    FareMarket fm;
    fm.travelSeg().clear();
    CPPUNIT_ASSERT(!allSegsMarried(fm));
  }

  void testAllSegsMarriedReturnFalseWhenNotSameCarriers()
  {
    Itin* itin = prepareItinAndFareMarkets();
    AirSeg* lastAirSeg = dynamic_cast<AirSeg*>(itin->fareMarket()[0]->travelSeg()[2]);
    lastAirSeg->carrier() = "CO";
    CPPUNIT_ASSERT(!allSegsMarried(*(itin->fareMarket()[0])));
  }

  void testAllSegsMarriedReturnFalseWhenMarriageDontStartAtFirstSeg()
  {
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT(!allSegsMarried(*(itin->fareMarket()[2])));
  }

  void testAllSegsMarriedReturnFalseWhenMarriageDontEndAtLastSeg()
  {
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT(!allSegsMarried(*(itin->fareMarket()[1])));
  }

  void testAllSegsMarriedReturnFalseWhenAllIntermediateSegsNotPartOfMarriage()
  {
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT(!allSegsMarried(*(itin->fareMarket()[4])));
  }

  void testAllSegsMarriedReturnTrueWhenTwoSegsFormMarriage()
  {
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT(allSegsMarried(*(itin->fareMarket()[3])));
  }

  void testAllSegsMarriedReturnTrueWhenThreeSegsFormMarriage()
  {
    Itin* itin = prepareItinAndFareMarkets();
    CPPUNIT_ASSERT(allSegsMarried(*(itin->fareMarket()[0])));
  }

  void testDuplicateFareMarketReturnFalseWhenVectorEmpty()
  {
    std::vector<FareMarket*> marriedFm;
    FareMarket fm;
    CPPUNIT_ASSERT(!tse::iadetail::JourneyPrepHelper::duplicateFareMarket(marriedFm, &fm));
  }

  void testDuplicateFareMarketReturnTrueWhenSamePointerInVector()
  {
    Itin* itin = prepareItinAndFareMarkets();
    std::vector<FareMarket*> marriedFm;
    marriedFm += itin->fareMarket()[0];
    CPPUNIT_ASSERT(
        tse::iadetail::JourneyPrepHelper::duplicateFareMarket(marriedFm, itin->fareMarket()[0]));
  }

  void testDuplicateFareMarketReturnTrueWhenSameTravelSegsVector()
  {
    Itin* itin = prepareItinAndFareMarkets();
    std::vector<FareMarket*> marriedFm;
    marriedFm += itin->fareMarket()[0];
    FareMarket fm;
    fm.travelSeg() = itin->fareMarket()[0]->travelSeg();
    CPPUNIT_ASSERT(tse::iadetail::JourneyPrepHelper::duplicateFareMarket(marriedFm, &fm));
  }

  void testDuplicateFareMarketReturnFalseWhenFareMarketNotFound()
  {
    Itin* itin = prepareItinAndFareMarkets();
    std::vector<FareMarket*> marriedFm;
    marriedFm += itin->fareMarket()[0];
    CPPUNIT_ASSERT(
        !tse::iadetail::JourneyPrepHelper::duplicateFareMarket(marriedFm, itin->fareMarket()[1]));
  }

  void testMarkMarriedMarketsDontMarkAnyWhenNotWpnc()
  {
    preparePricingTrx();
    _pricingTrx->getRequest()->lowFareRequested() = NO;
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(!assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsDontMarkAnyWhenNoItinInTrx()
  {
    preparePricingTrx();
    Itin* itin = prepareItinAndFareMarkets();
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(!assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsDontMarkAnyWhenItinPointerZero()
  {
    preparePricingTrx();
    Itin* itin = prepareItinAndFareMarkets();
    Itin* trxItin = 0;
    _pricingTrx->itin() += trxItin;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(!assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsPositive()
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->journeyActivatedForPricing() = true;
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsDontMarkAnyWhenNotPricingOrMipTrx()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(!assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsDontMarkAnyWhenMIPandNotWpniCurrentItin()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    _pricingTrx->billing()->actionCode() = "WPNI";
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    _pricingTrx->getOptions()->journeyActivatedForShopping() = true;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(!assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkMarriedMarketsPositiveWhenMIPandWpniCurrentItin()
  {
    preparePricingTrx();
    _pricingTrx->setTrxType(PricingTrx::MIP_TRX);
    _pricingTrx->billing()->actionCode() = "WPNI.C";
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->itin() += itin;
    _pricingTrx->getOptions()->journeyActivatedForShopping() = true;
    tse::iadetail::JourneyPrepHelper::markMarriedMarkets(*_pricingTrx);
    CPPUNIT_ASSERT(assertFareMarketsWithAllMarriedSegsMarked(itin));
  }

  void testMarkFlowMarketsFindsJourneyFareMarkets()
  {
    preparePricingTrx();
    _pricingTrx->getOptions()->journeyActivatedForShopping() = true;
    Itin* itin = prepareItinAndFareMarkets();
    FareMarket* fm = itin->fareMarket()[0];
    std::map<const TravelSeg*, bool> marriedSegs;
    OAndDMarket od;
    od.initializeOD(*fm, 0, 0, 0, 0, &marriedSegs);
    itin->oAndDMarkets().push_back(&od);
    itin->segmentOAndDMarket()[fm->travelSeg().front()] = &od;
    tse::iadetail::JourneyPrepHelper::markFlowMarkets(*_pricingTrx, *itin);
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->applyJourneyLogic());
    CPPUNIT_ASSERT(itin->fareMarket()[0]->flowMarket());
  }

  void testMarkFlowMarketsDontFindJourneyFareMarkets()
  {
    preparePricingTrx();
    Itin* itin = prepareItinAndFareMarkets();
    _pricingTrx->getOptions()->journeyActivatedForShopping() = true;
    tse::iadetail::JourneyPrepHelper::markFlowMarkets(*_pricingTrx, *itin);
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->applyJourneyLogic());
    CPPUNIT_ASSERT(!itin->fareMarket()[0]->flowMarket());
  }

  void testInitAvailBreakSetsAvailBreaks()
  {
    preparePricingTrx();
    Itin* itin = prepareItinAndFareMarkets();
    FareMarket* fm = itin->fareMarket()[0];
    tse::iadetail::JourneyPrepHelper::initAvailBreak(*_pricingTrx, *fm);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
    CPPUNIT_ASSERT_EQUAL((size_t)3, fm->availBreaks().size());
  }

  void testInitAvailBreak_SoloPricingCarrier_WPNC()
  {
    _request->lowFareRequested() = YES;

    FareMarket* fm = prepareFareMarketForSoloCarrierTest('Y');

    tse::iadetail::JourneyPrepHelper::initAvailBreak(*_pricingTrx, *fm);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fm->availBreaks().size());

    // set avail breaks between airsegs with same carrier marked as solo
    CPPUNIT_ASSERT(fm->availBreaks()[0]);
    CPPUNIT_ASSERT(fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }

  void testInitAvailBreak_NonSoloPricingCarrier_WPNC()
  {
    _request->lowFareRequested() = YES;

    FareMarket* fm = prepareFareMarketForSoloCarrierTest('N');

    tse::iadetail::JourneyPrepHelper::initAvailBreak(*_pricingTrx, *fm);

    CPPUNIT_ASSERT_EQUAL(size_t(3), fm->availBreaks().size());

    // do not set avail breaks between airsegs with same carrier not marked as solo
    CPPUNIT_ASSERT(!fm->availBreaks()[0]);
    CPPUNIT_ASSERT(!fm->availBreaks()[1]);
    CPPUNIT_ASSERT(fm->availBreaks()[2]);
  }


private:
  Itin* _newItin;
  PricingRequest* _request;
  PricingOptions* _option;
  TestMemHandle _memHandle;
  PricingTrx* _pricingTrx;

  void preparePricingTrx()
  {
    _option->jpsEntered() = ' ';
    _pricingTrx->setOptions(_option);
    Agent* agent = _memHandle.create<Agent>();
    _request->ticketingAgent() = agent;
    _request->lowFareRequested() = YES;
    agent->agentFunctions() = "YFH";
    agent->agentCity() = "HDQ";
    agent->agentDuty() = "8";
    agent->airlineDept() = "HDQ";
    agent->cxrCode() = "1S";
    agent->currencyCodeAgent() = "USD";
    agent->agentLocation() = _memHandle.create<Loc>();
    _pricingTrx->setRequest(_request);

    // Need billing record
    Billing* billing = _memHandle.create<Billing>();
    billing->userPseudoCityCode() = "HDQ";
    billing->userStation() = "925";
    billing->userBranch() = "3470";
    billing->partitionID() = "AA";
    billing->userSetAddress() = "02BD09";
    billing->aaaCity() = "HDQ";
    billing->aaaSine() = "YFH";
    billing->serviceName() = "ITPRICE1";
    billing->actionCode() = "WPBET";
    _pricingTrx->billing() = billing;
  }

  Itin* prepareItinAndFareMarkets()
  {
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    AirSeg* a4 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = a4->carrier() = "AA";
    a1->marriageStatus() = 'S';
    a2->marriageStatus() = 'P';
    a3->marriageStatus() = 'E';
    a4->marriageStatus() = 'E';

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;

    FareMarket* fm1 = _memHandle.create<FareMarket>();
    fm1->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm2 = _memHandle.create<FareMarket>();
    fm2->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2;

    FareMarket* fm3 = _memHandle.create<FareMarket>();
    fm3->travelSeg() += (TravelSeg*)a2, (TravelSeg*)a3;

    FareMarket* fm4 = _memHandle.create<FareMarket>();
    fm4->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a3;

    FareMarket* fm5 = _memHandle.create<FareMarket>();
    fm5->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)a3, (TravelSeg*)a4;

    itin->fareMarket() += fm1, fm2, fm3, fm4, fm5;
    return itin;
  }

  FareMarket* prepareFareMarketForSoloCarrierTest(Indicator soloSeg)
  {
    AirSeg* airSegVY = _memHandle.create<AirSeg>();
    airSegVY->carrier() = "VY";

    CarrierPreference* cp = _memHandle.create<CarrierPreference>();
    cp->activateSoloPricing() = soloSeg;
    airSegVY->carrierPref() = cp;

    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->travelSeg() += airSegVY, airSegVY, airSegVY;

    return fm;
  }

  bool assertFareMarketsWithAllMarriedSegsMarked(Itin* itin)
  {
    std::vector<FareMarket*>::iterator fmIt = itin->fareMarket().begin();
    std::vector<FareMarket*>::iterator fmItE = itin->fareMarket().end();
    for (; fmIt != fmItE; ++fmIt)
    {
      const FareMarket& fm = *(*fmIt);
      if (fm.hasAllMarriedSegs())
        return true;
    }
    return false;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(JourneyPrepHelperTest);

} // tse
