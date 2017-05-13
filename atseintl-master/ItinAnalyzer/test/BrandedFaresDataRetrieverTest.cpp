#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseConsts.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/Itin.h"
#include <iostream>
#include <vector>
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"
#include "Common/ClassOfService.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "BrandedFares/BrandProgram.h"

using namespace std;
namespace tse
{
class BrandedFaresDataRetrieverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandedFaresDataRetrieverTest);

  CPPUNIT_TEST(checkMapForBrandingGeneration);
  CPPUNIT_TEST(testGetUniqueOAndDsForBranding);
  CPPUNIT_TEST(testGetUniqueOAndDsForBrandingPerFareComponent);
  CPPUNIT_TEST(testGetPrimarySector);
  CPPUNIT_TEST(testGetItinLegs);
  CPPUNIT_TEST(testGetFareMarketOAndDAndCarrierOneWay);
  CPPUNIT_TEST(testGetFareMarketOAndDAndCarrierRoundTrip);
  CPPUNIT_TEST(testUnacceptableArunk);
  CPPUNIT_TEST(testIsTripOpenJawAndcalculateTripType);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  DataHandle dataHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  void testUnacceptableArunk()
  {
    std::vector<TravelSeg*> travelSegs;
    ArunkSeg arunkSeg1, arunkSeg2, arunkSeg3, arunkSeg4, arunkSeg5;
    Loc* orig1 = new Loc;
    Loc* dest1 = new Loc;
    Loc* orig2 = new Loc;
    Loc* dest2 = new Loc;
    Loc* orig3 = new Loc;
    Loc* dest3 = new Loc;
    Loc* orig4 = new Loc;
    Loc* dest4 = new Loc;
    Loc* orig5 = new Loc;
    Loc* dest5 = new Loc;

    orig1->nation() = UNITED_STATES;
    dest1->nation() = CANADA;
    arunkSeg1.origin() = orig1;
    arunkSeg1.destination() = dest1;

    orig2->nation() = "RU";
    dest2->nation() = "XU";
    arunkSeg2.origin() = orig2;
    arunkSeg2.destination() = dest2;

    orig3->state() = VIRGIN_ISLAND;
    dest3->state() = PUERTORICO;
    arunkSeg3.origin() = orig3;
    arunkSeg3.destination() = dest3;

    orig4->nation() = NORWAY;
    dest4->nation() = SWEDEN;
    arunkSeg4.origin() = orig4;
    arunkSeg4.destination() = dest4;

    orig5->nation() = VIRGIN_ISLAND;
    dest5->nation() = MEXICO;
    arunkSeg5.origin() = orig5;
    arunkSeg5.destination() = dest5;

    ShoppingTrx* shoppingTrx;
    dataHandle.get(shoppingTrx);
    prepareTestData(shoppingTrx);

    travelSegs.push_back(&arunkSeg2);
    bool result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == false);

    travelSegs.push_back(&arunkSeg4);
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == false);

    travelSegs.push_back(&arunkSeg3);
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == false);

    travelSegs.push_back(&arunkSeg5);
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == true);

    travelSegs.pop_back();
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == false);

    travelSegs.push_back(&arunkSeg1);
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == true);

    orig1->nation() = CANADA;
    result = IAIbfUtils::hasArunkThatSpreadsAcrossCountries(travelSegs);
    CPPUNIT_ASSERT(result == false);
  }

  void testIsTripOpenJawAndcalculateTripType()
  {
    Itin* itinOOJ; // origin open jaw
    Itin* itinTOJ; // turnaround open jaw
    Itin* itinDOJ; // double open jaw
    Itin* itinOW; // one way
    Itin* itinRT; // round trip
    Itin* itinCT; // circle trip

    dataHandle.get(itinOOJ);
    dataHandle.get(itinTOJ);
    dataHandle.get(itinDOJ);
    dataHandle.get(itinOW);
    dataHandle.get(itinRT);
    dataHandle.get(itinCT);

    ArunkSeg arunkSeg1;

    Loc* orig1 = new Loc;
    Loc* dest1 = new Loc;

    AirSeg* airSeg1 = buildSegment("JFK", "PAR", "AA");
    AirSeg* airSeg2 = buildSegment("JFK", "NCE", "AA");
    AirSeg* airSeg3 = buildSegment("NCE", "JFK", "AA");
    AirSeg* airSeg4 = buildSegment("NCE", "DFW", "AA");
    AirSeg* airSeg5 = buildSegment("DFW", "JFK", "AA");

    orig1->loc() = "PAR";
    orig1->nation() = FRANCE;
    dest1->loc() = "NCE";
    dest1->nation() = FRANCE;

    arunkSeg1.origin() = orig1;
    arunkSeg1.destination() = dest1;

    itinTOJ->travelSeg().push_back(airSeg1);
    itinTOJ->travelSeg().push_back(&arunkSeg1);
    itinTOJ->travelSeg().push_back(airSeg3);

    itinOOJ->travelSeg().push_back(airSeg2);
    itinOOJ->travelSeg().push_back(airSeg4);
    itinOOJ->geoTravelType() = GeoTravelType::International;

    itinDOJ->travelSeg().push_back(airSeg1);
    itinDOJ->travelSeg().push_back(&arunkSeg1);
    itinDOJ->travelSeg().push_back(airSeg4);
    itinDOJ->geoTravelType() = GeoTravelType::International;

    itinRT->travelSeg().push_back(airSeg2);
    itinRT->travelSeg().push_back(airSeg3);

    itinOW->travelSeg().push_back(airSeg2);

    itinCT->travelSeg().push_back(airSeg2);
    itinCT->travelSeg().push_back(airSeg4);
    itinCT->travelSeg().push_back(airSeg5);

    PricingTrx trx;
    trx.setTrxType(PricingTrx::MIP_TRX);

    DataHandle& dataHandle = trx.dataHandle();

    bool result = IAIbfUtils::isTripOpenJaw(itinTOJ, dataHandle);
    CPPUNIT_ASSERT(result == true);
    putItinAsFirstInTrx(trx, itinTOJ);
    IAIbfUtils::TripType tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::OPENJAW);

    result = IAIbfUtils::isTripOpenJaw(itinDOJ, dataHandle);
    CPPUNIT_ASSERT(result == true);
    putItinAsFirstInTrx(trx, itinDOJ);
    tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::OPENJAW);

    result = IAIbfUtils::isTripOpenJaw(itinOOJ, dataHandle);
    CPPUNIT_ASSERT(result == true);
    putItinAsFirstInTrx(trx, itinOOJ);
    tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::OPENJAW);

    result = IAIbfUtils::isTripOpenJaw(itinOW, dataHandle);
    CPPUNIT_ASSERT(result == false);
    putItinAsFirstInTrx(trx, itinOW);
    tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::ONEWAY);

    result = IAIbfUtils::isTripOpenJaw(itinRT, dataHandle);
    CPPUNIT_ASSERT(result == false);
    putItinAsFirstInTrx(trx, itinRT);
    tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::ROUNDTRIP);

    result = IAIbfUtils::isTripOpenJaw(itinCT, dataHandle);
    CPPUNIT_ASSERT(result == false);
    putItinAsFirstInTrx(trx, itinCT);
    tripType = IAIbfUtils::calculateTripType(trx);
    CPPUNIT_ASSERT(tripType == IAIbfUtils::ROUNDTRIP);

    itinOOJ->geoTravelType() =
        GeoTravelType::Domestic; // domestic flights of this type should be considered one ways
    result = IAIbfUtils::isTripOpenJaw(itinOOJ, dataHandle);
    CPPUNIT_ASSERT(result == false);
  }
  void putItinAsFirstInTrx(PricingTrx& trx, Itin* itin)
  {
    trx.itin().clear();
    trx.itin().push_back(itin);
  }

  void prepareTestData(PricingTrx& trx)
  {
    trx.setTrxType(PricingTrx::MIP_TRX);
    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    trx.setRequest(pRequest);
    Itin* itin;
    dataHandle.get(itin);
    AirSeg* airSeg1 = buildSegment("AAA", "BBB", "AA");
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "AA";
    itin->fareMarket().push_back(&fareMarket1);
    itin->travelSeg().push_back(airSeg1);

    trx.itin().push_back(itin);
  }

  void prepareTestData(ShoppingTrx* shoppingTrx)
  {
    shoppingTrx->setTrxType(PricingTrx::IS_TRX);

    PricingRequest* pRequest = _memHandle.create<PricingRequest>();
    shoppingTrx->setRequest(pRequest);
    AirSeg* airSeg1 = buildSegment("AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment("BBB", "CCC", "AA");
    AirSeg* airSeg3 = buildSegment("AAA", "BBB", "BB");
    AirSeg* airSeg4 = buildSegment("BBB", "CCC", "BB");

    Itin* itineraryOb1;
    Itin* itineraryOb2;
    dataHandle.get(itineraryOb1);
    dataHandle.get(itineraryOb2);

    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "AA";
    FareMarket fareMarket2;
    fareMarket2.governingCarrier() = "BB";
    FareMarket fareMarket3;

    itineraryOb1->fareMarket().push_back(&fareMarket1);
    itineraryOb2->fareMarket().push_back(&fareMarket2);
    itineraryOb2->fareMarket().push_back(&fareMarket3);

    itineraryOb1->travelSeg().push_back(airSeg1);
    itineraryOb1->travelSeg().push_back(airSeg2);
    itineraryOb2->travelSeg().push_back(airSeg3);
    itineraryOb2->travelSeg().push_back(airSeg4);

    shoppingTrx->legs().push_back(ShoppingTrx::Leg());

    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb1, 1, true));
    shoppingTrx->legs().front().sop().front().governingCarrier() = "AA";

    shoppingTrx->legs().front().sop().push_back(
        ShoppingTrx::SchedulingOption(itineraryOb2, 1, true));
    shoppingTrx->legs().front().sop().back().governingCarrier() = "BB";

    ItinIndex itinIndex;
    ItinIndex::ItinCellInfo itinCellInfo;
    ItinIndex::Key carrierKey;
    ShoppingUtil::createCxrKey("AA", carrierKey);
    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(itineraryOb1, scheduleKey);
    shoppingTrx->legs().front().carrierIndex().addItinCell(
        itineraryOb1, itinCellInfo, carrierKey, scheduleKey);
    itinIndex.addItinCell(itineraryOb1, itinCellInfo, carrierKey, scheduleKey);

    ShoppingUtil::createCxrKey("BB", carrierKey);
    ShoppingUtil::createScheduleKey(itineraryOb2, scheduleKey);
    shoppingTrx->legs().front().carrierIndex().addItinCell(
        itineraryOb2, itinCellInfo, carrierKey, scheduleKey);
    itinIndex.addItinCell(itineraryOb2, itinCellInfo, carrierKey, scheduleKey);

    shoppingTrx->journeyItin() = itineraryOb1;
  }

  AirSeg* buildSegment(string origin,
                       string destination,
                       string carrier,
                       DateTime departure = DateTime::localTime(),
                       DateTime arrival = DateTime::localTime(),
                       bool stopOver = false)
  {
    AirSeg* airSeg;
    dataHandle.get(airSeg);

    airSeg->departureDT() = departure;
    airSeg->arrivalDT() = arrival;

    Loc* locorig, *locdest;
    dataHandle.get(locorig);
    dataHandle.get(locdest);
    locorig->loc() = origin;
    locdest->loc() = destination;

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->origAirport() = locorig->loc();
    airSeg->destAirport() = locdest->loc();
    airSeg->stopOver() = stopOver;

    return airSeg;
  }

  void checkMapForBrandingGeneration()
  {
    ShoppingTrx* shoppingTrx;
    dataHandle.get(shoppingTrx);
    prepareTestData(shoppingTrx);

    BrandedFaresDataRetriever bfdr(*shoppingTrx, BrandRetrievalMode::PER_O_AND_D, 0);
    bfdr.buildODCWithFareMarketsMapIS();

    CPPUNIT_ASSERT(bfdr._fMsForBranding.size() == 2);

    for (FMsForBranding::iterator it = bfdr._fMsForBranding.begin();
         it != bfdr._fMsForBranding.end();
         ++it)
      std::cout << " key " << it->first << " fm count = " << it->second.size();

    IAIbfUtils::OdcTuple l1("AA", "AAA", "CCC"), l2("BB", "AAA", "CCC");

    CPPUNIT_ASSERT(bfdr._fMsForBranding[l1].size() == 1);
    CPPUNIT_ASSERT(bfdr._fMsForBranding[l2].size() == 2);
  }

  void testGetUniqueOAndDsForBrandingPerFareComponent()
  {
    PricingTrx trx;
    prepareTestData(trx);
    BrandedFaresDataRetriever bfdr(trx, BrandRetrievalMode::PER_FARE_COMPONENT, 0);

    AirSeg seg;
    FareMarket fm;
    fm.travelSeg().push_back(&seg);
    Loc originLoc, destinationLoc;
    fm.origin() = &originLoc;
    originLoc.loc() = "AUS";
    fm.destination() = &destinationLoc;
    destinationLoc.loc() = "PDX";
    fm.governingCarrier() = "AA";
    fm.direction() = FMDirection::OUTBOUND;

    Itin itin;
    itin.travelSeg().push_back(&seg);
    // in retrieving brands per fare component we should use origin and destination from FM
    // no other data should be needed.
    std::vector<OdcTuple> odcTupleVec;
    IAIbfUtils::fillOdDataForFareMarketPricing(
        &itin, &fm, IAIbfUtils::ONEWAY, BrandRetrievalMode::PER_FARE_COMPONENT, odcTupleVec, trx);
    bfdr.buildODCWithFareMarketsMapMIP();
    CPPUNIT_ASSERT_EQUAL(size_t(2), odcTupleVec.size());
    CPPUNIT_ASSERT_EQUAL(LocCode("AUS"), odcTupleVec[0].origin);
    CPPUNIT_ASSERT_EQUAL(LocCode("PDX"), odcTupleVec[0].destination);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), odcTupleVec[0].governingCarrier);
    CPPUNIT_ASSERT_EQUAL(LocCode("PDX"), odcTupleVec[1].origin);
    CPPUNIT_ASSERT_EQUAL(LocCode("AUS"), odcTupleVec[1].destination);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), odcTupleVec[1].governingCarrier);
  }

  void testGetUniqueOAndDsForBranding()
  {
    PricingTrx trx;
    prepareTestData(trx);
    BrandedFaresDataRetriever bfdr(trx, BrandRetrievalMode::PER_O_AND_D, 0);
    trx.itin().clear(); // to initialize BFDR we always need to have one itin ( to computee trip
                        // type )
    // this itin ( put there in test preparation) is not needed for most of the tests
    // so it's removed using .clear() methind here
    bfdr.buildODCWithFareMarketsMapMIP();
    CPPUNIT_ASSERT(bfdr._fMsForBranding.empty());

    Itin itin;
    itin.tripCharacteristics().set(Itin::OneWay);
    trx.itin().push_back(&itin);
    bfdr.buildODCWithFareMarketsMapMIP();
    CPPUNIT_ASSERT(bfdr._fMsForBranding.empty());

    AirSeg seg, seg2;
    seg.origAirport() = "AUS";
    seg.destAirport() = "PDX";
    seg.legId() = 0;
    seg2.origAirport() = "PDX";
    seg2.destAirport() = "AUS";
    seg2.legId() = 1;

    FareMarket fm;
    Loc originLoc, destinationLoc;
    fm.origin() = &originLoc;
    fm.destination() = &destinationLoc;
    originLoc.loc() = "AUS";
    destinationLoc.loc() = "PDX";
    fm.travelSeg().push_back(&seg);
    fm.governingCarrier() = "AA";
    fm.direction() = FMDirection::OUTBOUND;

    itin.travelSeg().push_back(&seg);
    itin.fareMarket().push_back(&fm);
    trx.fareMarket().push_back(&fm);

    bfdr.buildODCWithFareMarketsMapMIP();
    CPPUNIT_ASSERT(!bfdr._fMsForBranding.empty());

    CPPUNIT_ASSERT_EQUAL(size_t(2), bfdr._fMsForBranding.size());

    OdcTuple l("AA", "AUS", "PDX");

    CPPUNIT_ASSERT(bfdr._fMsForBranding[l].size() == 1);

    bfdr._fMsForBranding.clear();

    Itin itin2;
    itin2.travelSeg().push_back(&seg);
    itin2.fareMarket().push_back(&fm);

    trx.itin().push_back(&itin2);

    bfdr.buildODCWithFareMarketsMapMIP();

    CPPUNIT_ASSERT_EQUAL(size_t(2), bfdr._fMsForBranding.size());
    bfdr._fMsForBranding.clear();

  }

  void testGetFareMarketOAndDAndCarrierOneWay()
  {
    std::vector<OdcTuple> odcTupleVec;
    OdDataForFareMarket odData;
    const CarrierCode govCarrier = "AA";
    const IAIbfUtils::TripType tripType = IAIbfUtils::ONEWAY;
    odData.fmOD = std::make_pair("AAA", "BBB");
    odData.legOD = std::make_pair("AAA", "CCC");
    odData.tripOD = std::make_pair("AAA", "DDD");
    odData.significantLegOD = std::make_pair("AAA", "CCC");
    odData.isWithinLeg = true;

    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_O_AND_D);

    CPPUNIT_ASSERT(odcTupleVec.size() == 2);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "DDD");
    CPPUNIT_ASSERT(odcTupleVec[1].destination == "CCC");

    // If leg matches trip then we need to add only one OD pair because otherwise we'd put
    // duplicates there
    odcTupleVec.clear();
    odData.tripOD = std::make_pair("AAA", "CCC");
    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_O_AND_D);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
  }

  void testGetFareMarketOAndDAndCarrierRoundTrip()
  {
    // case 1 : fare market within leg
    std::vector<OdcTuple> odcTupleVec;
    OdDataForFareMarket odData;
    const CarrierCode govCarrier = "AA";
    const IAIbfUtils::TripType tripType = IAIbfUtils::ROUNDTRIP;
    odData.fmOD = std::make_pair("AAA", "BBB");
    odData.legOD = std::make_pair("AAA", "CCC");
    odData.tripOD = std::make_pair("AAA", "DDD");
    odData.significantLegOD = std::make_pair("AAA", "CCC");
    odData.isWithinLeg = true;

    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_O_AND_D);

    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "CCC");

    // When the same request is made with "per fare component" logic then fm's O&D should be used
    odcTupleVec.clear();
    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_FARE_COMPONENT);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "BBB");

    // case 2 : fare market extends the leg
    odcTupleVec.clear();
    odData.fmOD = std::make_pair("AAA", "CCC");
    odData.legOD = std::make_pair("AAA", "BBB");
    odData.tripOD = std::make_pair("AAA", "DDD");
    odData.significantLegOD = std::make_pair("BBB", "DDD");
    odData.isWithinLeg = false;

    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_O_AND_D);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "BBB");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "DDD");

    // When the same request is made with "per fare component" logic then fm's O&D should be used
    odcTupleVec.clear();
    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_FARE_COMPONENT);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "CCC");

    // case 3 : fare market matches multiple legs
    odcTupleVec.clear();
    odData.fmOD = std::make_pair("AAA", "DDD");
    odData.legOD = std::make_pair("AAA", "DDD");
    odData.tripOD = std::make_pair("ZZZ", "XXX");
    odData.significantLegOD = std::make_pair("BBB", "DDD");
    odData.isWithinLeg = false;

    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_O_AND_D);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "DDD");

    // When the same request is made with "per fare component" logic then fm's O&D should be used
    odcTupleVec.clear();
    IAIbfUtils::fillOdcTupleVec(
        odcTupleVec, odData, govCarrier, tripType, BrandRetrievalMode::PER_FARE_COMPONENT);
    CPPUNIT_ASSERT(odcTupleVec.size() == 1);
    CPPUNIT_ASSERT(odcTupleVec[0].origin == "AAA");
    CPPUNIT_ASSERT(odcTupleVec[0].destination == "DDD");
  }

  void testGetItinLegs()
  {
    Itin itin;

    AirSeg seg1, seg2, seg3, seg4;
    seg1.origAirport() = "AUS";
    seg1.destAirport() = "PDX";
    seg1.legId() = 0;

    seg2.origAirport() = "PDX";
    seg2.destAirport() = "SEA";
    seg2.legId() = 0;

    seg3.origAirport() = "SEA";
    seg3.destAirport() = "PDX";
    seg3.legId() = 1;

    seg4.origAirport() = "PDX";
    seg4.destAirport() = "AUS";
    seg4.legId() = 1;

    itin.travelSeg().push_back(&seg1);
    itin.travelSeg().push_back(&seg2);
    itin.travelSeg().push_back(&seg3);
    itin.travelSeg().push_back(&seg4);

    PricingTrx trx;
    prepareTestData(trx);
    BrandedFaresDataRetriever bfdr(trx, BrandRetrievalMode::PER_O_AND_D, 0);

    itinanalyzerutils::setItinLegs(&itin);
    CPPUNIT_ASSERT(itin.itinLegs().size() == 2);
    CPPUNIT_ASSERT(itin.itinLegs()[0].front()->origAirport() == "AUS");
    CPPUNIT_ASSERT(itin.itinLegs()[0].back()->destAirport() == "SEA");
    CPPUNIT_ASSERT(itin.itinLegs()[1].front()->origAirport() == "SEA");
    CPPUNIT_ASSERT(itin.itinLegs()[1].back()->destAirport() == "AUS");

    itin.travelSeg().clear();
    seg3.origAirport() = "SEA";
    seg3.destAirport() = "OGG";
    seg3.legId() = 0;

    itin.travelSeg().push_back(&seg1);
    itin.travelSeg().push_back(&seg2);
    itin.travelSeg().push_back(&seg3);

    itin.itinLegs().clear();
    itinanalyzerutils::setItinLegs(&itin);
    CPPUNIT_ASSERT(itin.itinLegs().size() == 1);
    CPPUNIT_ASSERT(itin.itinLegs()[0].front()->origAirport() == "AUS");
    CPPUNIT_ASSERT(itin.itinLegs()[0].back()->destAirport() == "OGG");

    itin.travelSeg().clear();
    seg4.origAirport() = "PDX";
    seg4.destAirport() = "AUS";

    seg2.legId() = 0;
    seg3.legId() = 1;
    seg4.legId() = 2;

    itin.travelSeg().push_back(&seg1);
    itin.travelSeg().push_back(&seg2);
    itin.travelSeg().push_back(&seg3);
    itin.travelSeg().push_back(&seg4);

    itin.itinLegs().clear();
    itinanalyzerutils::setItinLegs(&itin);

    CPPUNIT_ASSERT(itin.itinLegs().size() == 3);
    CPPUNIT_ASSERT(itin.itinLegs()[0].front()->origAirport() == "AUS");
    CPPUNIT_ASSERT(itin.itinLegs()[0].back()->destAirport() == "SEA");
    CPPUNIT_ASSERT(itin.itinLegs()[1].front()->origAirport() == "SEA");
    CPPUNIT_ASSERT(itin.itinLegs()[1].back()->destAirport() == "OGG");
    CPPUNIT_ASSERT(itin.itinLegs()[2].front()->origAirport() == "PDX");
    CPPUNIT_ASSERT(itin.itinLegs()[2].back()->destAirport() == "AUS");
  }

  void testGetPrimarySector()
  {
    FareMarket fm;
    AirSeg* seg1 = buildSegment("AUS", "DFW", "AA");
    AirSeg* seg2 = buildSegment("DFW", "SEA", "AA");
    seg1->departureDT() = DateTime::localTime();
    seg2->departureDT() = DateTime::localTime();

    PricingTrx trx;
    prepareTestData(trx);
    BrandedFaresDataRetriever bfdr(trx, BrandRetrievalMode::PER_O_AND_D, 0);

    fm.primarySector() = seg1;
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg1);

    fm.primarySector() = seg2;
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg2);

    fm.primarySector() = 0;
    fm.travelSeg().push_back(seg1);

    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg1);

    fm.travelSeg().clear();
    fm.travelSeg().push_back(seg2);
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg2);

    fm.travelSeg().push_back(seg1);
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg2);

    seg1->segmentType() = Arunk;
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg2);

    seg1->segmentType() = Air;
    seg2->segmentType() = Arunk;
    CPPUNIT_ASSERT(IAIbfUtils::getPrimarySector(&fm) == seg1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BrandedFaresDataRetrieverTest);
}
