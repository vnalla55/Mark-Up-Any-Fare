#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/ESVOptions.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <stdint.h>
#include <vector>

#include <boost/assign/std/vector.hpp>

using namespace std;
using namespace boost::assign;

namespace tse
{
class ShoppingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingUtilTest);
  CPPUNIT_TEST(testCollectFPCxrKeys);
  CPPUNIT_TEST(testCollectFPCxrKeys_Exc);
  CPPUNIT_TEST(testCollectSopsCxrKeys);
  CPPUNIT_TEST(testCollectSopsCxrKeys_Exc);
  CPPUNIT_TEST(testCollectOnlineCxrKeys_Ow);
  CPPUNIT_TEST(testCollectOnlineCxrKeys_Rt);
  CPPUNIT_TEST(testCollectOnlineCxrKeys_RtIntl);
  CPPUNIT_TEST(testGetDummyItineraryFromCarrierIndexWithEmptyCarrierIndex);
  CPPUNIT_TEST(testGetDummyItineraryFromCarrierIndexWithMultipleCarriersInIndex);
  CPPUNIT_TEST(testAllPenaltiesWhenNoTravSegments);
  CPPUNIT_TEST(testTotalStopPenaltyWhenOneTravSegment);
  CPPUNIT_TEST(testTotalStopPenaltyWhenTwoTravSegments);
  CPPUNIT_TEST(testTotalTravDurPenaltyWhenDurationIsZero);
  CPPUNIT_TEST(testTotalTravDurPenaltyWhenDurationIsNonZero);
  CPPUNIT_TEST(testTotalDepTimeDevPenaltyWhenDeviationIsZero);
  CPPUNIT_TEST(testTotalDepTimeDevPenaltyWhenDeviationIsNonZero);
  CPPUNIT_TEST(testFindInternalSopId_AllValid);
  CPPUNIT_TEST(testFindInternalSopId_SopErased);
  CPPUNIT_TEST(testBrandingUtils);
  CPPUNIT_TEST(testGetBrandPrograms);
  CPPUNIT_TEST(testGetFareBrandProgram);
  CPPUNIT_TEST(testOnlineItinerary);
  CPPUNIT_TEST(testOnlineItineraryRef);
  CPPUNIT_TEST(testInterlineItinerary);
  CPPUNIT_TEST(testInterlineItineraryRef);
  CPPUNIT_TEST(testOnlineFareMarket);
  CPPUNIT_TEST(testOnlineFareMarketRef);
  CPPUNIT_TEST(testInterlineFareMarket);
  CPPUNIT_TEST(testInterlineFareMarketRef);
  CPPUNIT_TEST(testRemoveBrandsNotPresentInFilter);
  CPPUNIT_TEST(testHardPassForBothDirectionsFound);
  CPPUNIT_TEST(testValidHardPassExists);
  CPPUNIT_TEST(testIsMinConnectionTimeMet);
  CPPUNIT_TEST(testPositiveConnectionTime);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  ShoppingTrx* _trx;

protected:
  DataHandle dataHandle;
  Itin* itin;
  size_t id;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<ShoppingTrx>();
  }

  void tearDown() { _memHandle.clear(); }

  ShoppingUtilTest() : _trx(0), itin(0), id(0) {}

  void initItin() { itin = new Itin(); }
  void destroyItin() { delete itin; }

  ClassOfService* buildCOS(const BookingCode& bkc, uint16_t numOfSeats = 9)
  {
    ClassOfService* cos(0);
    dataHandle.get(cos);
    cos->numSeats() = numOfSeats;
    cos->cabin().setEconomyClass();
    cos->bookingCode() = bkc;

    return cos;
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
    locorig->area() = "1";
    locorig->nation() = "XX";
    locorig->subarea() = "11";
    locdest->loc() = destination;
    locdest->area() = "1";
    locdest->subarea() = "11";
    locdest->nation() = "XX";

    airSeg->origAirport() = origin;
    airSeg->origin() = locorig;
    airSeg->destAirport() = destination;
    airSeg->destination() = locdest;

    airSeg->carrier() = carrier;
    airSeg->setOperatingCarrierCode(carrier);
    airSeg->boardMultiCity() = locorig->loc();
    airSeg->offMultiCity() = locdest->loc();
    airSeg->stopOver() = stopOver;

    ClassOfService* oneEconomyClass = buildCOS("Y");
    airSeg->classOfService().push_back(oneEconomyClass);

    return airSeg;
  }

  void addOneSegmentToItin(std::string origin,
                           std::string dest,
                           DateTime departure = DateTime::localTime(),
                           DateTime arrival = DateTime::localTime())
  {

    AirSeg* travelSeg = buildSegment(origin, dest, "AA", departure, arrival);
    itin->travelSeg().push_back(travelSeg);
  }

  void addDummyItineraryToTrx(ShoppingTrx* shoppingTrx, Itin* dummyItinerary, string carrier)
  {
    ItinIndex::ItinCellInfo* itinCellInfo;
    dataHandle.get(itinCellInfo);

    itinCellInfo->flags() |= ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT;

    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey(carrier, cxrKey);

    ItinIndex::Key scheduleKey;
    ShoppingUtil::createScheduleKey(scheduleKey);

    shoppingTrx->legs().front().carrierIndex().addItinCell(
        dummyItinerary, *itinCellInfo, cxrKey, scheduleKey);

    shoppingTrx->legs().front().addSop(ShoppingTrx::SchedulingOption(dummyItinerary, true));
    ShoppingTrx::SchedulingOption& newSop = shoppingTrx->legs().front().sop().back();
    newSop.setDummy(true);
  }

  FareMarket* buildFareMarket(uint16_t legId, uint32_t cxrKey1, uint32_t cxrKey2)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->legIndex() = legId;
    fm->getApplicableSOPs() = _memHandle.create<ApplicableSOP>();
    if (cxrKey1)
      (*fm->getApplicableSOPs())[cxrKey1];
    if (cxrKey2)
      (*fm->getApplicableSOPs())[cxrKey2];
    return fm;
  }

  FarePath& buildFarePath(const std::vector<FareMarket*>& fms)
  {
    FarePath* fp = _memHandle.create<FarePath>();

    for (FareMarket* fm : fms)
    {
      PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
      ptf->fareMarket() = fm;
      FareUsage* fu = _memHandle.create<FareUsage>();
      fu->paxTypeFare() = ptf;
      PricingUnit* pu = _memHandle.create<PricingUnit>();
      pu->fareUsage().push_back(fu);
      fp->pricingUnit().push_back(pu);
    }

    return *fp;
  }

  void testCollectFPCxrKeys()
  {
    std::vector<FareMarket*> fms;
    fms += buildFareMarket(0u, 1u, 2u), buildFareMarket(0u, 2u, 3u), buildFareMarket(1u, 4u, 5u);

    shpq::CxrKeysPerLeg cxrKeysPerLeg;
    ShoppingUtil::collectFPCxrKeys(buildFarePath(fms), 2u, cxrKeysPerLeg);

    CPPUNIT_ASSERT_EQUAL(size_t(2), cxrKeysPerLeg.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), cxrKeysPerLeg[0].size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), cxrKeysPerLeg[0].front());
    CPPUNIT_ASSERT_EQUAL(size_t(2), cxrKeysPerLeg[1].size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(4), cxrKeysPerLeg[1][0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(5), cxrKeysPerLeg[1][1]);
  }

  void testCollectFPCxrKeys_Exc()
  {
    std::vector<FareMarket*> fms;
    fms += buildFareMarket(0u, 1u, 2u), buildFareMarket(0u, 3u, 4u), buildFareMarket(1u, 4u, 5u);
    FarePath& fp = buildFarePath(fms);

    shpq::CxrKeysPerLeg cxrKeysPerLeg;
    CPPUNIT_ASSERT_THROW(ShoppingUtil::collectFPCxrKeys(fp, 2u, cxrKeysPerLeg),
                         tse::ErrorResponseException);
  }

  void testCollectSopsCxrKeys()
  {
    Itin itin;
    ShoppingTrx::SchedulingOption obSop(&itin, 0), ibSop(&itin, 0);
    obSop.governingCarrier() = "AA";
    ibSop.governingCarrier() = "BB";
    _trx->legs().resize(2u);
    _trx->legs()[0].sop() += obSop;
    _trx->legs()[1].sop() += ibSop;

    shpq::CxrKeyPerLeg cxrKeyPerLeg;
    ShoppingUtil::collectSopsCxrKeys(*_trx, SopIdVec(2u, 0), cxrKeyPerLeg);

    CPPUNIT_ASSERT_EQUAL(size_t(2), cxrKeyPerLeg.size());
    ItinIndex::Key obKey, ibKey;
    ShoppingUtil::createCxrKey("AA", obKey);
    ShoppingUtil::createCxrKey("BB", ibKey);
    CPPUNIT_ASSERT_EQUAL(obKey, cxrKeyPerLeg[0]);
    CPPUNIT_ASSERT_EQUAL(ibKey, cxrKeyPerLeg[1]);
  }

  void testCollectSopsCxrKeys_Exc()
  {
    Itin itin;
    ShoppingTrx::SchedulingOption obSop(&itin, 0), ibSop(&itin, 0);
    obSop.governingCarrier() = "AA";
    ibSop.governingCarrier() = "BB";
    _trx->legs().resize(2u);
    _trx->legs()[0].sop() += obSop;
    _trx->legs()[1].sop() += ibSop;

    shpq::CxrKeyPerLeg cxrKeyPerLeg;
    CPPUNIT_ASSERT_THROW(ShoppingUtil::collectSopsCxrKeys(*_trx, SopIdVec(1u, 0), cxrKeyPerLeg),
                         tse::ErrorResponseException);
  }

  void testCollectOnlineCxrKeys_Ow()
  {
    shpq::CxrKeysPerLeg cxrKeysPerLeg;
    cxrKeysPerLeg[0] += 1u, 2u;

    shpq::CxrKeys onlineCxrs;
    bool intlAppl;

    ShoppingUtil::collectOnlineCxrKeys(cxrKeysPerLeg, onlineCxrs, intlAppl);
    CPPUNIT_ASSERT_EQUAL(size_t(2), onlineCxrs.size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), onlineCxrs[0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), onlineCxrs[1]);
    CPPUNIT_ASSERT(!intlAppl);
  }

  void testCollectOnlineCxrKeys_Rt()
  {
    shpq::CxrKeysPerLeg cxrKeysPerLeg;
    cxrKeysPerLeg[0] += 1u, 2u;
    cxrKeysPerLeg[1] += 1u, 2u, 3u;

    shpq::CxrKeys onlineCxrs;
    bool intlAppl;

    ShoppingUtil::collectOnlineCxrKeys(cxrKeysPerLeg, onlineCxrs, intlAppl);
    CPPUNIT_ASSERT_EQUAL(size_t(2), onlineCxrs.size());
    CPPUNIT_ASSERT_EQUAL(uint32_t(1), onlineCxrs[0]);
    CPPUNIT_ASSERT_EQUAL(uint32_t(2), onlineCxrs[1]);
    CPPUNIT_ASSERT(intlAppl);
  }

  void testCollectOnlineCxrKeys_RtIntl()
  {
    shpq::CxrKeysPerLeg cxrKeysPerLeg;
    cxrKeysPerLeg[0] += 1u;
    cxrKeysPerLeg[1] += 2u;

    shpq::CxrKeys onlineCxrs;
    bool intlAppl;

    ShoppingUtil::collectOnlineCxrKeys(cxrKeysPerLeg, onlineCxrs, intlAppl);
    CPPUNIT_ASSERT(onlineCxrs.empty());
    CPPUNIT_ASSERT(intlAppl);
  }

  void testGetDummyItineraryFromCarrierIndexWithEmptyCarrierIndex()
  {
    ShoppingTrx* shoppingTrx;
    dataHandle.get(shoppingTrx);

    shoppingTrx->legs().push_back(ShoppingTrx::Leg());

    ItinIndex::Key carrierKey;
    ShoppingUtil::createCxrKey("AA", carrierKey);

    Itin* dummyItin = 0;

    // Check if server wouldn't throw exception
    try
    {
      dummyItin = ShoppingUtil::getDummyItineraryFromCarrierIndex(
          shoppingTrx->legs().front().carrierIndex(), carrierKey);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    // Check if dumy itinerary for empty carrier index wasn't found
    CPPUNIT_ASSERT(NULL == dummyItin);
  }

  void prepareDataForGetDummyItineraryFromCarrierIndexWithMultipleCarriersInIndex(
      ShoppingTrx* shoppingTrx)
  {
    AirSeg* airSeg1 = buildSegment("AAA", "BBB", "AA");
    AirSeg* airSeg2 = buildSegment("BBB", "CCC", "AA");
    AirSeg* airSeg3 = buildSegment("AAA", "BBB", "BB");
    AirSeg* airSeg4 = buildSegment("BBB", "CCC", "BB");

    Itin* itineraryOb1;
    Itin* itineraryOb2;
    dataHandle.get(itineraryOb1);
    dataHandle.get(itineraryOb2);

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

    AirSeg* airSeg5 = buildSegment("AAA", "CCC", "AA");
    AirSeg* airSeg6 = buildSegment("AAA", "CCC", "BB");

    Itin* dummyItinerary1;
    Itin* dummyItinerary2;
    dataHandle.get(dummyItinerary1);
    dataHandle.get(dummyItinerary2);

    dummyItinerary1->travelSeg().push_back(airSeg5);
    dummyItinerary2->travelSeg().push_back(airSeg6);

    addDummyItineraryToTrx(shoppingTrx, dummyItinerary1, "AA");
    addDummyItineraryToTrx(shoppingTrx, dummyItinerary2, "BB");
  }

  void testGetDummyItineraryFromCarrierIndexWithMultipleCarriersInIndex()
  {
    ShoppingTrx* shoppingTrx;
    dataHandle.get(shoppingTrx);

    prepareDataForGetDummyItineraryFromCarrierIndexWithMultipleCarriersInIndex(shoppingTrx);

    ItinIndex::Key carrierKey1;
    ShoppingUtil::createCxrKey("AA", carrierKey1);

    ItinIndex::Key carrierKey2;
    ShoppingUtil::createCxrKey("BB", carrierKey2);

    Itin* dummyItin = 0;

    // Check if server wouldn't throw exception
    try
    {
      dummyItin = ShoppingUtil::getDummyItineraryFromCarrierIndex(
          shoppingTrx->legs().front().carrierIndex(), carrierKey1);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    // Check if dumy itinerary was found
    CPPUNIT_ASSERT(NULL != dummyItin);

    // Check if server wouldn't throw exception
    try
    {
      dummyItin = ShoppingUtil::getDummyItineraryFromCarrierIndex(
          shoppingTrx->legs().front().carrierIndex(), carrierKey2);
    }
    catch (...) { CPPUNIT_ASSERT(false); }

    // Check if dumy itinerary was found
    CPPUNIT_ASSERT(NULL != dummyItin);
  }

  void testAllPenaltiesWhenNoTravSegments()
  {
    int anyvalue = 37;

    int stopPenalty = ShoppingUtil::totalStopPenalty(itin, anyvalue);
    CPPUNIT_ASSERT_EQUAL(stopPenalty, 0);

    int travDurPenalty = ShoppingUtil::totalTravDurPenalty(itin, anyvalue);
    CPPUNIT_ASSERT_EQUAL(travDurPenalty, 0);

    DateTime anydate = DateTime::localTime();
    int devPenalty = ShoppingUtil::totalDepTimeDevPenalty(itin, anyvalue, anydate);
    CPPUNIT_ASSERT_EQUAL(devPenalty, 0);
  }

  void testTotalStopPenaltyWhenOneTravSegment()
  {
    initItin();
    int anyvalue = 23;
    addOneSegmentToItin("LAX", "BOS");

    int stopPenalty = ShoppingUtil::totalStopPenalty(itin, anyvalue);
    CPPUNIT_ASSERT_EQUAL(stopPenalty, 0);
    destroyItin();
  }

  void testTotalStopPenaltyWhenTwoTravSegments()
  {
    initItin();
    addOneSegmentToItin("LAX", "BOS");
    addOneSegmentToItin("BOS", "KRK");

    int stopPenalty = ShoppingUtil::totalStopPenalty(itin, ESVOptions::DEFAULT_STOP_PENALTY);
    int default_stop_penaty = ESVOptions::DEFAULT_STOP_PENALTY;

    CPPUNIT_ASSERT_EQUAL(stopPenalty, default_stop_penaty);

    int anyStopPenalty = 37;
    stopPenalty = ShoppingUtil::totalStopPenalty(itin, anyStopPenalty);
    CPPUNIT_ASSERT_EQUAL(stopPenalty, anyStopPenalty);
    destroyItin();
  }
  // travel duration penalty ------------------------------------------------------

  void testTotalTravDurPenaltyWhenDurationIsZero()
  {
    initItin();
    DateTime now = DateTime::localTime();
    DateTime afternow = now.addSeconds(SECONDS_PER_HOUR);

    addOneSegmentToItin("LAX", "BOS", now, afternow);
    addOneSegmentToItin("BOS", "KRK", afternow, now);

    int travDurPenalty =
        ShoppingUtil::totalTravDurPenalty(itin, ESVOptions::DEFAULT_TRAV_DUR_PENALTY);
    CPPUNIT_ASSERT_EQUAL(travDurPenalty, 0);

    travDurPenalty = ShoppingUtil::totalTravDurPenalty(itin, 15);
    CPPUNIT_ASSERT_EQUAL(travDurPenalty, 0);
    destroyItin();
  }

  void testTotalTravDurPenaltyWhenDurationIsNonZero()
  {
    initItin();
    DateTime departure = DateTime::localTime();
    DateTime arrival = departure.addSeconds(2 * SECONDS_PER_HOUR);

    addOneSegmentToItin("LAX", "BOS", departure, departure);
    addOneSegmentToItin("BOS", "KRK", arrival, arrival);

    int defaultTravDurPenaty = ESVOptions::DEFAULT_TRAV_DUR_PENALTY;
    int travDurPenalty = ShoppingUtil::totalTravDurPenalty(itin, defaultTravDurPenaty);

    CPPUNIT_ASSERT_EQUAL(travDurPenalty, 2 * defaultTravDurPenaty);

    int notDefaultTravDurPenalty = 18;
    travDurPenalty = ShoppingUtil::totalTravDurPenalty(itin, notDefaultTravDurPenalty);
    CPPUNIT_ASSERT_EQUAL(travDurPenalty, 2 * notDefaultTravDurPenalty);
    destroyItin();
  }

  /// Departure Time Deviation Penalty------------------------------------------------------

  void testTotalDepTimeDevPenaltyWhenDeviationIsZero()
  {
    initItin();
    DateTime reqdeparture = DateTime::localTime().addSeconds(SECONDS_PER_HOUR);
    DateTime arrival1 = reqdeparture.addSeconds(2 * SECONDS_PER_HOUR);
    DateTime departure2 = arrival1.addSeconds(1 * SECONDS_PER_HOUR);
    DateTime arrival2 = departure2.addSeconds(1 * SECONDS_PER_HOUR);

    addOneSegmentToItin("LAX", "BOS", reqdeparture, arrival1);
    addOneSegmentToItin("BOS", "DFW", departure2, arrival2);

    int anyDeviationPenalty = 89;
    int devPenalty = ShoppingUtil::totalDepTimeDevPenalty(itin, anyDeviationPenalty, reqdeparture);
    CPPUNIT_ASSERT_EQUAL(devPenalty, 0);
    destroyItin();
  }

  void testTotalDepTimeDevPenaltyWhenDeviationIsNonZero()
  {
    initItin();
    DateTime reqdeparture = DateTime::localTime().addSeconds(SECONDS_PER_HOUR);
    DateTime departure1 = reqdeparture.addSeconds(3 * SECONDS_PER_HOUR);
    DateTime arrival1 = departure1.addSeconds(2 * SECONDS_PER_HOUR);
    DateTime departure2 = arrival1.addSeconds(1 * SECONDS_PER_HOUR);
    DateTime arrival2 = departure2.addSeconds(1 * SECONDS_PER_HOUR);

    addOneSegmentToItin("LAX", "BOS", departure1, arrival1);
    addOneSegmentToItin("BOS", "DFW", departure2, arrival2);

    int devPenalty = ShoppingUtil::totalDepTimeDevPenalty(
        itin, ESVOptions::DEFAULT_DEP_TIME_DEV_PENALTY, reqdeparture);
    int defaultTimeDevPenaty = ESVOptions::DEFAULT_DEP_TIME_DEV_PENALTY;
    CPPUNIT_ASSERT_EQUAL(devPenalty, 3 * defaultTimeDevPenaty);

    int anyDeviationPenalty = 24;
    devPenalty = ShoppingUtil::totalDepTimeDevPenalty(itin, anyDeviationPenalty, reqdeparture);
    CPPUNIT_ASSERT_EQUAL(3 * anyDeviationPenalty, devPenalty);
    destroyItin();
  }

  void setupOneLegSops(ShoppingTrx& trx)
  {
    Itin* itn(0);

    trx.legs().push_back(ShoppingTrx::Leg());
    size_t leg(trx.legs().size() - 1);
    trx.schedulingOptionIndices().resize(leg + 1);
    trx.indicesToSchedulingOption().resize(leg + 1);

    uint32_t originalSopId = 0;

    AirSeg* airSeg1 = buildSegment("JFK", "DFW", "LH");
    AirSeg* airSeg2 = buildSegment("DFW", "LAX", "AA");

    dataHandle.get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "DFW", "DL");
    airSeg2 = buildSegment("DFW", "LAX", "AC");

    dataHandle.get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "WAS", "DL");
    airSeg2 = buildSegment("WAS", "LAX", "AC");

    dataHandle.get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    ++originalSopId;
    airSeg1 = buildSegment("JFK", "WAS", "AA");
    airSeg2 = buildSegment("WAS", "LAX", "AA");

    dataHandle.get(itn);
    itn->travelSeg() += airSeg1, airSeg2;

    trx.itin().push_back(itn);
    trx.schedulingOptionIndices()[leg][originalSopId] = trx.legs()[leg].sop().size();
    trx.indicesToSchedulingOption()[leg][trx.legs()[leg].sop().size()] = originalSopId;
    trx.legs()[leg].sop().push_back(ShoppingTrx::SchedulingOption(itn, originalSopId, true));

    CPPUNIT_ASSERT_EQUAL(trx.legs().size(), (size_t)1);
    CPPUNIT_ASSERT_EQUAL(trx.legs()[leg].sop().size(), (size_t)4);
  }

  void testFindInternalSopId_AllValid()
  {
    ShoppingTrx* trx(0);
    dataHandle.get(trx);

    setupOneLegSops(*trx);
    uint32_t leg = 0;

    for (size_t idx = 0; idx < trx->legs()[leg].sop().size(); ++idx)
    {
      CPPUNIT_ASSERT_EQUAL(ShoppingUtil::findInternalSopId(*trx, leg, (uint32_t)idx),
                           (uint32_t)idx);
    }
  }

  void testFindInternalSopId_SopErased()
  {
    ShoppingTrx* trx(0);
    dataHandle.get(trx);

    setupOneLegSops(*trx);
    const uint32_t leg = 0;
    ShoppingTrx::Leg& curLeg = trx->legs()[leg];
    std::vector<ShoppingTrx::SchedulingOption>::iterator item = curLeg.sop().begin();

    const size_t index = ++item - curLeg.sop().begin();
    curLeg.sop().erase(item);

    { // Update Scheduling indices
      std::map<uint32_t, uint32_t>::iterator i = trx->schedulingOptionIndices()[leg].begin();
      for (; i != trx->schedulingOptionIndices()[leg].end();)
      {
        if (i->second == index)
        {
          trx->schedulingOptionIndices()[leg].erase(i++);
          continue;
        }
        else if (i->second > index)
        {
          i->second--;
        }

        ++i;
      }
    }

    CPPUNIT_ASSERT_EQUAL(trx->legs()[leg].sop().size(), (size_t)3);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::findInternalSopId(*trx, leg, 0), (uint32_t)0);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::findInternalSopId(*trx, leg, 2), (uint32_t)1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::findInternalSopId(*trx, leg, 3), (uint32_t)2);
  }

  void testBrandingUtils()
  {
    PricingTrx trx;
    BrandProgram bProgram1, bProgram2;
    BrandInfo brand1, brand2, brand3;

    bProgram1.programID() = "15";
    bProgram2.programID() = "25";

    bProgram1.programCode() = "P15";
    bProgram2.programCode() = "P25";

    brand1.brandCode() = "AS";
    brand2.brandCode() = "BS";
    brand3.brandCode() = "CS";

    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand1));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand2));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram2, &brand3));

    trx.validBrands().push_back("AS");
    trx.validBrands().push_back("BS");
    trx.validBrands().push_back("CS");
    trx.validBrands().push_back(NO_BRAND);

    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "15", "AS"), 0);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "15", "BS"), 1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "25", "CS"), 2);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "15", "CS"), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "25", "AS"), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "25", "BS"), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "", ""), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "", "BS"), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "15", ""), -1);
    CPPUNIT_ASSERT_EQUAL(ShoppingUtil::getRequestedBrandIndex(trx, "44", "XX"), -1);

    CPPUNIT_ASSERT(ShoppingUtil::getBrandCode(trx, 0) == "AS");
    CPPUNIT_ASSERT(ShoppingUtil::getBrandCode(trx, 1) == "BS");
    CPPUNIT_ASSERT(ShoppingUtil::getBrandCode(trx, 2) == "CS");

    CPPUNIT_ASSERT(ShoppingUtil::getProgramCode(trx, 0) == "P15");
    CPPUNIT_ASSERT(ShoppingUtil::getProgramCode(trx, 1) == "P15");
    CPPUNIT_ASSERT(ShoppingUtil::getProgramCode(trx, 2) == "P25");

    CPPUNIT_ASSERT(ShoppingUtil::getProgramId(trx, 0) == "15");
    CPPUNIT_ASSERT(ShoppingUtil::getProgramId(trx, 1) == "15");
    CPPUNIT_ASSERT(ShoppingUtil::getProgramId(trx, 2) == "25");

    CPPUNIT_ASSERT(ShoppingUtil::getBrandCodeString(trx, 0) == "AS");
    CPPUNIT_ASSERT(ShoppingUtil::getBrandCodeString(trx, 1) == "BS");
    CPPUNIT_ASSERT(ShoppingUtil::getBrandCodeString(trx, 2) == "CS");
    CPPUNIT_ASSERT(ShoppingUtil::getBrandCodeString(trx, 3) == "");

    Itin itin;
    itin.brandCodes().insert("AS");
    itin.brandCodes().insert("BS");

    CPPUNIT_ASSERT(ShoppingUtil::isValidItinBrand(&itin, "AS"));
    CPPUNIT_ASSERT(ShoppingUtil::isValidItinBrand(&itin, "BS"));
    CPPUNIT_ASSERT(!ShoppingUtil::isValidItinBrand(&itin, "CS"));
    CPPUNIT_ASSERT(!ShoppingUtil::isValidItinBrand(&itin, ""));

    FarePath fp;
    fp.setBrandCode(NO_BRAND);
    CPPUNIT_ASSERT(ShoppingUtil::getFarePathBrandCode(&fp) == "");

    fp.setBrandCode("BS");
    CPPUNIT_ASSERT(ShoppingUtil::getFarePathBrandCode(&fp) == "BS");
  }

  void testGetBrandPrograms()
  {
    PricingTrx trx;
    BrandProgram bProgram1, bProgram2;
    BrandInfo brand1, brand2, brand3;

    brand1.brandCode() = "AS";
    brand2.brandCode() = "BS";

    bProgram1.programID() = "15";
    bProgram2.programID() = "25";

    // (15,AS), (15, BS), (25, BS);
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand1));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand2));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram2, &brand2));

    PaxTypeFare ptf1, ptf2;
    FareMarket fm1, fm2;

    FareInfo fareInfo;
    fareInfo.directionality() = BOTH;
    fareInfo.market1() = "SYD";
    fareInfo.market1() = "AKL";

    Fare fare;
    fare.initialize(Fare::FareState::FS_Domestic, &fareInfo, fm1, nullptr, nullptr);

    Loc origin, destination;
    origin.loc() = "SYD";
    destination.loc() = "AKL";

    fm1.origin() = &origin;
    fm1.destination() = &destination;
    fm2.origin() = &origin;
    fm2.destination() = &destination;

    fm1.brandProgramIndexVec().push_back(2); // (25, BS)
    fm1.brandProgramIndexVec().push_back(0); // (15, AS)
    fm1.brandProgramIndexVec().push_back(1); // (15, BS)

    ptf1.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS)); // (25, BS)
    ptf1.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS)); // (15, AS)
    ptf1.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS)); // (15, BS)
    ptf1.fareMarket() = &fm1;
    ptf1.setFare(&fare);

    fm2.brandProgramIndexVec().push_back(2); // (25, BS)
    fm2.brandProgramIndexVec().push_back(1); // (15, BS)
    fm2.brandProgramIndexVec().push_back(0); // (15, AS)

    ptf2.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS)); // (25, BS)
    ptf2.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS)); // (15, AS)
    ptf2.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS)); // (15, BS)
    ptf2.fareMarket() = &fm2;
    ptf2.setFare(&fare);

    FarePath fp;
    PricingUnit pu;
    FareUsage fu1, fu2;

    pu.fareUsage().push_back(&fu1);
    pu.fareUsage().push_back(&fu2);

    fu1.paxTypeFare() = &ptf1;
    fu2.paxTypeFare() = &ptf2;

    fp.pricingUnit().push_back(&pu);
    fp.setBrandCode("AS");

    std::string result = ShoppingUtil::getBrandPrograms(trx, &fp);
    CPPUNIT_ASSERT(result == "15");

    fp.setBrandCode("BS");
    result = ShoppingUtil::getBrandPrograms(trx, &fp);
    CPPUNIT_ASSERT(result == "15,25");

    fp.setBrandCode("PS");
    result = ShoppingUtil::getBrandPrograms(trx, &fp);
    CPPUNIT_ASSERT(result == "");
  }

  void testGetFareBrandProgram()
  {
    PricingTrx trx;
    BrandProgram bProgram1, bProgram2;
    BrandInfo brand1, brand2, brand3;

    FareInfo fareInfo;
    fareInfo.directionality() = BOTH;
    fareInfo.market1() = "SYD";
    fareInfo.market1() = "AKL";

    brand1.brandCode() = "AS";
    brand2.brandCode() = "BS";

    bProgram1.programID() = "15";
    bProgram2.programID() = "25";

    // (15,AS), (15, BS), (25, BS);
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand1));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram1, &brand2));
    trx.brandProgramVec().push_back(std::make_pair(&bProgram2, &brand2));

    Loc origin, destination;
    origin.loc() = "SYD";
    destination.loc() = "AKL";

    PaxTypeFare ptf;
    FareMarket fm;
    fm.brandProgramIndexVec().push_back(2); // (25, BS)
    fm.brandProgramIndexVec().push_back(0); // (15, AS)
    fm.brandProgramIndexVec().push_back(1); // (15, BS)

    fm.origin() = &origin;
    fm.destination() = &destination;

    Fare fare;
    fare.initialize(Fare::FareState::FS_Domestic, &fareInfo, fm, nullptr, nullptr);

    ptf.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS)); // (25, BS)
    ptf.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS)); // (15, AS)
    ptf.getMutableBrandStatusVec().push_back(
        std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS)); // (15, BS)
    ptf.fareMarket() = &fm;
    ptf.setFare(&fare);

    std::string result = ShoppingUtil::getFareBrandProgram(trx, "BS", &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "15");

    result = ShoppingUtil::getFareBrandProgram(trx, "AS", &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "15");

    ptf.getMutableBrandStatusVec()[0].first = PaxTypeFare::BS_HARD_PASS;
    result = ShoppingUtil::getFareBrandProgram(trx, "BS", &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "25");

    result = ShoppingUtil::getFareBrandProgram(trx, "PS", &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "");

    result = ShoppingUtil::getFareBrandProgram(trx, "", &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "");

    result = ShoppingUtil::getFareBrandProgram(trx, NO_BRAND, &ptf, Direction::BOTHWAYS);
    CPPUNIT_ASSERT(result == "");
  }

  void testOnlineItinerary()
  {
    Itin itin;
    itin.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    itin.travelSeg().push_back(buildSegment("BBB", "CCC", "AA"));
    CPPUNIT_ASSERT(ShoppingUtil::isOnlineFlight(itin));
  }

  void testOnlineItineraryRef()
  {
    Itin itin;
    itin.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    itin.travelSeg().push_back(buildSegment("BBB", "CCC", "AA"));
    CarrierCode code;
    CPPUNIT_ASSERT(ShoppingUtil::isOnlineFlightRef(itin, code));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), code);
  }

  void testInterlineItinerary()
  {
    Itin itin;
    itin.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    itin.travelSeg().push_back(buildSegment("BBB", "CCC", "DD"));
    CPPUNIT_ASSERT(!ShoppingUtil::isOnlineFlight(itin));
  }

  void testInterlineItineraryRef()
  {
    Itin itin;
    itin.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    itin.travelSeg().push_back(buildSegment("BBB", "CCC", "DD"));
    CarrierCode code;
    CPPUNIT_ASSERT(!ShoppingUtil::isOnlineFlightRef(itin, code));
    // No assumption about value of code
  }

  void testOnlineFareMarket()
  {
    FareMarket fm;
    fm.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    fm.travelSeg().push_back(buildSegment("BBB", "CCC", "AA"));
    CPPUNIT_ASSERT(ShoppingUtil::isOnlineFlight(fm));
  }

  void testOnlineFareMarketRef()
  {
    FareMarket fm;
    fm.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    fm.travelSeg().push_back(buildSegment("BBB", "CCC", "AA"));
    CarrierCode code;
    CPPUNIT_ASSERT(ShoppingUtil::isOnlineFlightRef(fm, code));
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), code);
  }

  void testInterlineFareMarket()
  {
    FareMarket fm;
    fm.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    fm.travelSeg().push_back(buildSegment("BBB", "CCC", "DD"));
    CPPUNIT_ASSERT(!ShoppingUtil::isOnlineFlight(fm));
  }

  void testInterlineFareMarketRef()
  {
    FareMarket fm;
    fm.travelSeg().push_back(buildSegment("AAA", "BBB", "AA"));
    fm.travelSeg().push_back(buildSegment("BBB", "CCC", "DD"));
    CarrierCode code;
    CPPUNIT_ASSERT(!ShoppingUtil::isOnlineFlightRef(fm, code));
    // No assumption about value of code
  }

  void testRemoveBrandsNotPresentInFilter()
  {
    BrandFilterMap filter;
    std::set<BrandCode> originalBrands {BrandCode("SV"),
                                        BrandCode("FL"),
                                        BrandCode("BZ"),
                                        BrandCode("PS"),
                                        BrandCode("PE")};

    // -- empty filter -> empty result
    std::set<BrandCode> brands = originalBrands;
    
    ShoppingUtil::removeBrandsNotPresentInFilter(brands, filter);
    CPPUNIT_ASSERT(brands == std::set<BrandCode>());
    // -- filter with brands -> set intersection returned
    brands = originalBrands;

    filter[BrandCode("BZ")];
    filter[BrandCode("X1")];
    filter[BrandCode("SV")];
    filter[BrandCode("PE")];
    filter[BrandCode("MM")];

    ShoppingUtil::removeBrandsNotPresentInFilter(brands, filter);
    std::set<BrandCode> expectedBrands {BrandCode("BZ"),
                                        BrandCode("SV"),
                                        BrandCode("PE")};

    CPPUNIT_ASSERT(brands == expectedBrands);
  }

  void testHardPassForBothDirectionsFound()
  {
    std::set<Direction> directions;
    CPPUNIT_ASSERT(ShoppingUtil::hardPassForBothDirectionsFound(directions) == false);
    directions.insert(Direction::ORIGINAL);
    CPPUNIT_ASSERT(ShoppingUtil::hardPassForBothDirectionsFound(directions) == false);
    directions.insert(Direction::REVERSED);
    CPPUNIT_ASSERT(ShoppingUtil::hardPassForBothDirectionsFound(directions) == true);
    directions.clear();
    directions.insert(Direction::BOTHWAYS);
    CPPUNIT_ASSERT(ShoppingUtil::hardPassForBothDirectionsFound(directions) == true);
  }

  void testValidHardPassExists()
  {
     FareMarket* fm = _memHandle.create<FareMarket>();
     PaxTypeFare *ptf1, *ptf2, *ptf3, *ptf4, *ptf5, *ptf6;
     ptf1 = _memHandle.create<PaxTypeFare>();
     ptf2 = _memHandle.create<PaxTypeFare>();
     ptf3 = _memHandle.create<PaxTypeFare>();
     ptf4 = _memHandle.create<PaxTypeFare>();
     ptf5 = _memHandle.create<PaxTypeFare>();
     ptf6 = _memHandle.create<PaxTypeFare>();
     fm->allPaxTypeFare().push_back(ptf1);
     fm->allPaxTypeFare().push_back(ptf2);
     fm->allPaxTypeFare().push_back(ptf3);
     fm->allPaxTypeFare().push_back(ptf4);
     fm->allPaxTypeFare().push_back(ptf5);
     fm->allPaxTypeFare().push_back(ptf6);
     ptf1->setIsShoppingFare(); // To make it pass PaxTypeFare::isValid()
     ptf2->setIsShoppingFare();
     ptf3->setIsShoppingFare();
     ptf4->setIsShoppingFare();
     ptf5->setIsShoppingFare();
     // ptf6 is kept invalid

     ptf1->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::ORIGINAL));
     ptf2->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::REVERSED));
     ptf3->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::REVERSED));
     ptf4->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::ORIGINAL));
     ptf5->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::ORIGINAL));
     // hard pass in invalid fare should not be taken into consideration:
     ptf6->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::ORIGINAL));
     // TEST WITH SOFT PASSES ONLY
     std::set<Direction> directions;
     bool result = ShoppingUtil::validHardPassExists(fm, 0, directions, 0, true, false);
     CPPUNIT_ASSERT(result == false);
     CPPUNIT_ASSERT(directions.empty());
     // TEST WITH HARD PASS IN ONE DIRECTION
     ptf1->getMutableBrandStatusVec().clear();
     ptf1->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::ORIGINAL));
     result = ShoppingUtil::validHardPassExists(fm, 0, directions, 0, true, false);
     CPPUNIT_ASSERT(result == true);
     CPPUNIT_ASSERT(directions.size() == 1);
     CPPUNIT_ASSERT(directions.find(Direction::ORIGINAL) != directions.end());
     // TEST WITH HARD PASSES IN BOTH DIRECTIONS
     ptf3->getMutableBrandStatusVec().clear();
     ptf3->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::REVERSED));
     // BOTHWAYS should be ignored as bofore this fare is found we already should have passes in both directions
     ptf4->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));
     result = ShoppingUtil::validHardPassExists(fm, 0, directions, 0, true, false);
     CPPUNIT_ASSERT(result == true);
     CPPUNIT_ASSERT(directions.size() == 2);
     CPPUNIT_ASSERT(directions.find(Direction::ORIGINAL) != directions.end());
     CPPUNIT_ASSERT(directions.find(Direction::REVERSED) != directions.end());
     // Test without directionality (function should quit after first HARD_PASS regardless of direction
     result = ShoppingUtil::validHardPassExists(fm, 0, directions, 0, false, false);
     CPPUNIT_ASSERT(result == true);
     CPPUNIT_ASSERT(directions.size() == 0);
  }

  void testRemoveSoftPassesFromFareMarket()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    PaxTypeFare *ptf1, *ptf2, *ptf3, *ptf4, *ptf5, *ptf6, *ptf7, *ptf8, *ptf9;
    ptf1 = _memHandle.create<PaxTypeFare>();
    ptf2 = _memHandle.create<PaxTypeFare>();
    ptf3 = _memHandle.create<PaxTypeFare>();
    ptf4 = _memHandle.create<PaxTypeFare>();
    ptf5 = _memHandle.create<PaxTypeFare>();
    ptf6 = _memHandle.create<PaxTypeFare>();
    ptf7 = _memHandle.create<PaxTypeFare>();
    ptf8 = _memHandle.create<PaxTypeFare>();
    ptf9 = _memHandle.create<PaxTypeFare>();

    fm->allPaxTypeFare().push_back(ptf1);
    fm->allPaxTypeFare().push_back(ptf2);
    fm->allPaxTypeFare().push_back(ptf3);
    fm->allPaxTypeFare().push_back(ptf4);
    fm->allPaxTypeFare().push_back(ptf5);
    fm->allPaxTypeFare().push_back(ptf6);
    fm->allPaxTypeFare().push_back(ptf7);
    fm->allPaxTypeFare().push_back(ptf8);
    fm->allPaxTypeFare().push_back(ptf9);
    ptf1->setIsShoppingFare(); // To make it pass PaxTypeFare::isValid()
    ptf2->setIsShoppingFare();
    ptf3->setIsShoppingFare();
    ptf4->setIsShoppingFare();
    ptf5->setIsShoppingFare();
    // ptf6 is kept invalid
    ptf7->setIsShoppingFare();
    ptf8->setIsShoppingFare();
    ptf9->setIsShoppingFare();

    ptf1->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::ORIGINAL));
    ptf2->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::REVERSED));
    ptf3->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    ptf4->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::ORIGINAL));
    ptf5->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::ORIGINAL));
    ptf6->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::REVERSED));
    ptf7->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::REVERSED));
    ptf8->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    ptf9->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));

    // remove soft passes in one direction
    ShoppingUtil::removeSoftPassesFromFareMarket(fm, 0, 0, Direction::ORIGINAL);
    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);

    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);

    // remove soft passes in the other direction as well
    ShoppingUtil::removeSoftPassesFromFareMarket(fm, 0, 0, Direction::REVERSED);
    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);

    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);

    // restore original statuses
    ptf1->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::ORIGINAL));
    ptf2->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::REVERSED));
    ptf3->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
    ptf4->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::ORIGINAL));
    ptf5->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::ORIGINAL));
    ptf6->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::REVERSED));
    ptf7->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::REVERSED));
    ptf8->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_SOFT_PASS, Direction::BOTHWAYS));
    ptf9->getMutableBrandStatusVec().push_back(std::make_pair(PaxTypeFare::BS_HARD_PASS, Direction::BOTHWAYS));
    // remove both directions at once
    ShoppingUtil::removeSoftPassesFromFareMarket(fm, 0, 0, Direction::BOTHWAYS);

    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf1->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf2->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf3->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);

    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf4->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf5->getMutableBrandStatusVec()[0].second == Direction::ORIGINAL);

    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_SOFT_PASS);
    CPPUNIT_ASSERT(ptf6->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf7->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_FAIL);
    CPPUNIT_ASSERT(ptf8->getMutableBrandStatusVec()[0].second == Direction::REVERSED);

    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].first == PaxTypeFare::BS_HARD_PASS);
    CPPUNIT_ASSERT(ptf9->getMutableBrandStatusVec()[0].second == Direction::BOTHWAYS);
  }

  void addOneSegmentToItin(Itin* requestedItin,
                           std::string origin,
                           std::string dest,
                           DateTime departure,
                           DateTime arrival)
  {

    AirSeg* travelSeg = buildSegment(origin, dest, "AA", departure, arrival);
    requestedItin->travelSeg().push_back(travelSeg);
  }

  void testIsMinConnectionTimeMet()
  {
    Itin *itin1, *itin2, *itin3, *itin4, *itin5;
    itin1 = new Itin();
    itin2 = new Itin();
    itin3 = new Itin();
    // TEST 1 : only one hour between each leg.
    DateTime now = DateTime::localTime();
    DateTime departure1 = now;
    DateTime arrival1 = now.addSeconds(1*SECONDS_PER_HOUR);
    DateTime departure2 = now.addSeconds(2*SECONDS_PER_HOUR);
    DateTime arrival2 = now.addSeconds(3*SECONDS_PER_HOUR);
    DateTime departure3 = now.addSeconds(4*SECONDS_PER_HOUR);
    DateTime arrival3 = now.addSeconds(5*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin1, "KRK", "WAW", departure1, arrival1);
    addOneSegmentToItin(itin2, "WAW", "MAD", departure2, arrival2);
    addOneSegmentToItin(itin3, "MAD", "HAV", departure3, arrival3);

    ShoppingTrx::SchedulingOption sop1(itin1, 1, true), sop2(itin2, 2, true), sop3(itin3, 3, true);
    _trx->legs().resize(3u);
    _trx->legs()[0].sop() += sop1;
    _trx->legs()[1].sop() += sop2;
    _trx->legs()[2].sop() += sop3;

    PricingOptions* pop = new PricingOptions();
    pop->setMinConnectionTimeDomestic(2*SECONDS_PER_HOUR);
    pop->setMinConnectionTimeInternational(2*SECONDS_PER_HOUR);
    _trx->setOptions(pop);

    std::vector<uint16_t> problematicLegs;
    CPPUNIT_ASSERT(!ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, false, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(2), problematicLegs.size());
    CPPUNIT_ASSERT(ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(0), problematicLegs.size());
    // Test 2: new SOP added so MCT can be met between leg 0 and 1
    itin4 = new Itin();
    DateTime departure4 = now.addSeconds(4.1*SECONDS_PER_HOUR);
    DateTime arrival4 = now.addSeconds(5.1*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin4, "WAW", "MAD", departure4, arrival4);
    ShoppingTrx::SchedulingOption sop4(itin4, 4, true);
    _trx->legs()[1].sop() += sop4;
    CPPUNIT_ASSERT(!ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, false, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(1), problematicLegs.size());
    CPPUNIT_ASSERT(ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(0), problematicLegs.size());
    // Test 3: another SOP added so MCT is met between all legs
    itin5 = new Itin();
    DateTime departure5 = now.addSeconds(6.1*SECONDS_PER_HOUR);
    DateTime arrival5 = now.addSeconds(7.1*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin5, "MAD", "HAV", departure5, arrival5);
    ShoppingTrx::SchedulingOption sop5(itin5, 5, true);
    _trx->legs()[2].sop() += sop5;
    CPPUNIT_ASSERT(ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, false, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(0), problematicLegs.size());
    CPPUNIT_ASSERT(ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(0), problematicLegs.size());
  }

  void testPositiveConnectionTime()
  {
    Itin *itin1, *itin2, *itin3, *itin4, *itin5;
    itin1 = new Itin();
    itin2 = new Itin();
    itin3 = new Itin();
    // TEST 1 : only one hour between each leg.
    DateTime now = DateTime::localTime();
    DateTime departure1 = now;
    DateTime arrival1 = now.addSeconds(1*SECONDS_PER_HOUR);
    DateTime departure2 = now.addSeconds(1*SECONDS_PER_HOUR);
    DateTime arrival2 = now.addSeconds(1*SECONDS_PER_HOUR);
    DateTime departure3 = now.addSeconds(1*SECONDS_PER_HOUR);
    DateTime arrival3 = now.addSeconds(1*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin1, "KRK", "WAW", departure1, arrival1);
    addOneSegmentToItin(itin2, "WAW", "MAD", departure2, arrival2);
    addOneSegmentToItin(itin3, "MAD", "HAV", departure3, arrival3);

    ShoppingTrx::SchedulingOption sop1(itin1, 1, true), sop2(itin2, 2, true), sop3(itin3, 3, true);
    _trx->legs().resize(3u);
    _trx->legs()[0].sop() += sop1;
    _trx->legs()[1].sop() += sop2;
    _trx->legs()[2].sop() += sop3;

    PricingOptions* pop = new PricingOptions();
    pop->setMinConnectionTimeDomestic(2*SECONDS_PER_HOUR);
    pop->setMinConnectionTimeInternational(2*SECONDS_PER_HOUR);
    _trx->setOptions(pop);

    std::vector<uint16_t> problematicLegs;
    CPPUNIT_ASSERT(!ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(2), problematicLegs.size());
    // Test 2: new SOP added so PCT can be met between leg 0 and 1
    itin4 = new Itin();
    DateTime departure4 = now.addSeconds(1*SECONDS_PER_HOUR+60);
    DateTime arrival4 = now.addSeconds(2*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin4, "WAW", "MAD", departure4, arrival4);
    ShoppingTrx::SchedulingOption sop4(itin4, 4, true);
    _trx->legs()[1].sop() += sop4;
    CPPUNIT_ASSERT(!ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(1), problematicLegs.size());
    // Test 3: another SOP added so PCT is met between all legs
    itin5 = new Itin();
    DateTime departure5 = now.addSeconds(2*SECONDS_PER_HOUR+300);
    DateTime arrival5 = now.addSeconds(3*SECONDS_PER_HOUR);
    addOneSegmentToItin(itin5, "MAD", "HAV", departure5, arrival5);
    ShoppingTrx::SchedulingOption sop5(itin5, 5, true);
    _trx->legs()[2].sop() += sop5;
    CPPUNIT_ASSERT(ShoppingUtil::isMinConnectionTimeMet(_trx->legs(), pop, true, problematicLegs));
    CPPUNIT_ASSERT_EQUAL(size_t(0), problematicLegs.size());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingUtilTest);
}
