#include "Fares/DRVController.h"
#include "DataModel/FareMarket.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/PaxType.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseEnums.h"
#include "Routing/TravelRoute.h"
#include "DataModel/TravelSeg.h"
#include "Common/DateTime.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Routing.h"
#include "Routing/RtgKey.h"
#include "Routing/RoutingConsts.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RoutingInfo.h"
#include "Diagnostic/Diag455Collector.h"
#include <iostream>
#include <set>
#include <vector>
#include <map>
#include "test/include/CppUnitHelperMacros.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestRoutingFactory.h"
#include "test/testdata/TestTravelRouteFactory.h"

#include "test/include/MockDataManager.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DBAccess/FltTrkCntryGrp.h"

using namespace tse;
using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
public:
  const FltTrkCntryGrp* getFltTrkCntryGrp(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "AA")
    {
      FltTrkCntryGrp* trk = _memHandle.create<FltTrkCntryGrp>();
      trk->carrier() = "CA";
      trk->flttrkApplInd() = 'E';
      trk->nations().push_back("US");
      return trk;
    }
    else if (carrier == "AZ")
    {
      FltTrkCntryGrp* trk = _memHandle.create<FltTrkCntryGrp>();
      trk->carrier() = "AZ";
      trk->flttrkApplInd() = 'E';
      trk->nations().push_back("IT");
      return trk;
    }
    return DataHandleMock::getFltTrkCntryGrp(carrier, date);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "")
      return "";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
};
}

class DRVControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DRVControllerTest);
  CPPUNIT_TEST(carrierAllowsDRV);
  CPPUNIT_TEST(buildLocalTravelRoute1A);
  CPPUNIT_TEST(buildLocalTravelRoute1C);
  CPPUNIT_TEST(buildLocalTravelRoute1D);
  CPPUNIT_TEST(buildLocalTravelRoute2A);
  CPPUNIT_TEST(buildLocalTravelRoute2B);
  CPPUNIT_TEST(buildLocalTravelRoute2C);
  CPPUNIT_TEST(buildLocalTravelRoute2D);
  CPPUNIT_TEST(findMatchingFareMarket);
  CPPUNIT_TEST(buildIntlTravelRoute1A);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenDiffNationOnThirdSegSPR135990);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenDiffCarrierOnThirdSeg);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenSameCarrierOnPrimarySeg);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenDiffCarrierOnPrimarySeg);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenDiffCarrierFirstSeg);
  CPPUNIT_TEST(testBuildLocalTravelRouteWhenDiffCarrierSecondSeg);

  CPPUNIT_TEST_SUITE_END();

private:
  tse::MockDataManager* _dm;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  // DFWLON-BA  AT
  void carrierAllowsDRV()
  {
    PricingTrx trx;
    int16_t missingCityIndex = 2; // CHI

    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "ATL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "BA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "DFW";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "LON";
    tvlRoute.destinationNation() = "UK";
    tvlRoute.travelDate() = DateTime::localTime();

    // Build Vector of Travel Segs to populate the Mileage Travel Route
    AirSeg aS1, aS2, aS3, aS4;
    LocCode loc1, loc2, loc3, loc4, loc5;
    // Loc loc5;
    loc1 = "DFW";
    loc2 = "NYC";
    loc3 = "CHI";
    loc4 = "ATL";
    loc5 = "LON";
    // loc5.loc() = "LON";
    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "BA";
    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    // aS2.hiddenStops().push_back(&loc5);

    Loc lc1, lc2, lc3, lc4, lc5;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    lc5.nation() = "UK";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);

    FareMarket fm;
    fm.travelSeg() = tvlSegs;

    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    DRVController drv(trx);
    bool ret = drv.carrierAllowsDRV(trx, missingCityIndex, tvlRoute);
    CPPUNIT_ASSERT_EQUAL(ret, true);
  }

  void addCityCarrier(std::vector<TravelRoute::CityCarrier>& travelRoute,
                      LocCode lc1,
                      NationCode nation1,
                      LocCode lc2,
                      NationCode nation2,
                      CarrierCode carrier)
  {
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = lc1;
    boardCty.isHiddenCity() = false;
    offCty.loc() = lc2;
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.boardNation() = nation1;
    cityCarrier.carrier() = carrier;
    cityCarrier.offCity() = offCty;
    cityCarrier.offNation() = nation2;
    travelRoute.push_back(cityCarrier);
  }

  void addTravelSeg(std::vector<TravelSeg*>& tvlSegs,
                    LocCode boardMultiCityLoc,
                    NationCode origNation,
                    LocCode offMultiCityLoc,
                    NationCode destNation,
                    CarrierCode carrier)
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();

    airSeg->carrier() = carrier;
    airSeg->boardMultiCity() = boardMultiCityLoc;
    airSeg->offMultiCity() = offMultiCityLoc;

    Loc* origLoc = _memHandle.create<Loc>();
    Loc* destLoc = _memHandle.create<Loc>();

    origLoc->nation() = origNation;
    destLoc->nation() = destNation;
    airSeg->origin() = origLoc;
    airSeg->destination() = destLoc;

    tvlSegs.push_back(airSeg);
  }

  void addCityCrxAndTvlSeg(std::vector<TravelRoute::CityCarrier>& travelRoute,
                           std::vector<TravelSeg*>& tvlSegs,
                           LocCode boardMultiCityLoc,
                           NationCode origNation,
                           LocCode offMultiCityLoc,
                           NationCode destNation,
                           CarrierCode carrier)
  {
    addCityCarrier(
        travelRoute, boardMultiCityLoc, origNation, offMultiCityLoc, destNation, carrier);
    addTravelSeg(tvlSegs, boardMultiCityLoc, origNation, offMultiCityLoc, destNation, carrier);
  }

  // Test true condition
  // DFWLON-BA  AT
  void buildLocalTravelRoute1A()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 1; // CHI
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "DFW", "US", "NYC", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "NYC", "US", "CHI", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "CHI", "US", "ATL", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "ATL", "US", "LON", "UK", "BA");

    tvlRoute.govCxr() = "BA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "DFW";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "LON";
    tvlRoute.destinationNation() = "UK";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;
    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(3), localTvlRoute.travelRoute().size());
  }

  // Test US-PR Market - Expect True return
  // LAXCCS-AA
  void buildLocalTravelRoute1C()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 1; // MIA
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "LAX", "US", "JFK", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "JFK", "US", "MIA", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIA", "US", "SJU", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "SJU", "US", "CCS", "VZ", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "LAX";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "CCS";
    tvlRoute.destinationNation() = "VZ";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;
    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(3), localTvlRoute.travelRoute().size());
  }

  // Test US-CA Market
  // YZZLIM-AA
  void buildLocalTravelRoute1D()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // NYC
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "YYZ", "CA", "NYC", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "NYC", "US", "RDU", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "RDU", "US", "LIM", "VZ", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "YYZ";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "LIM";
    tvlRoute.destinationNation() = "VZ";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;
    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), localTvlRoute.travelRoute().size());
  }

  // Test DRV for a flight tracking Carrier
  // HNLLON-AA
  void buildLocalTravelRoute2A()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // DFW
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "HNL", "US", "DFW", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "DFW", "US", "CHI", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "CHI", "US", "LON", "UK", "BA");

    tvlRoute.govCxr() = "BA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "HNL";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "LON";
    tvlRoute.destinationNation() = "UK";
    tvlRoute.travelDate() = DateTime::localTime();

    Loc loc5, loc6;
    loc5.loc() = "BOS";
    loc5.nation() = "US";
    loc6.loc() = "ATL";
    loc6.nation() = "US";
    tvlSegs[2]->hiddenStops().push_back(&loc5);
    tvlSegs[2]->hiddenStops().push_back(&loc6);

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(2), localTvlRoute.travelRoute().size());
  }

  // Test DRV for a flight tracking Carrier
  // LONHNL-AA
  void buildLocalTravelRoute2B()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 1; // DFW
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "LON", "UK", "CHI", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "CHI", "US", "DFW", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "DFW", "US", "HNL", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "LON";
    tvlRoute.originNation() = "UK";
    tvlRoute.destination() = "HNL";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    Loc loc5, loc6;
    loc5.loc() = "BOS";
    loc5.nation() = "US";
    loc6.loc() = "ATL";
    loc6.nation() = "US";
    tvlSegs[0]->hiddenStops().push_back(&loc5);
    tvlSegs[0]->hiddenStops().push_back(&loc6);

    tvlSegs[0]->destAirport() = tvlSegs[0]->offMultiCity();

    tvlRoute.primarySector() = tvlSegs[0];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(4), localTvlRoute.travelRoute().size());
  }

  // Test DRV for a flight tracking Carrier
  // LAXCCS-AA
  void buildLocalTravelRoute2C()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // NYC
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "LAX", "US", "NYC", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "NYC", "US", "MIA", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIA", "US", "CCS", "VZ", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "LAX";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "CCS";
    tvlRoute.destinationNation() = "VZ";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlSegs[2]->origAirport() = tvlSegs[2]->boardMultiCity();

    Loc loc5;
    loc5.loc() = "SJU";
    loc5.nation() = "PR";
    loc5.state() = "USPR";
    tvlSegs[2]->hiddenStops().push_back(&loc5);

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(3), localTvlRoute.travelRoute().size());
  }

  // Test DRV for a flight tracking Carrier
  // STLAUA-AA
  void buildLocalTravelRoute2D()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 1; // BOS
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "STL", "US", "NYC", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "NYC", "US", "BOS", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "BOS", "US", "WAS", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "WAS", "US", "AUA", "DN", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "STL";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "AUA";
    tvlRoute.destinationNation() = "DN";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlSegs[3]->origAirport() = tvlSegs[3]->boardMultiCity();

    Loc loc6;
    loc6.loc() = "MIA";
    loc6.nation() = "US";
    tvlSegs[3]->hiddenStops().push_back(&loc6);

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute localTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, localTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(4), localTvlRoute.travelRoute().size());
  }

  // Find a Matching FareMarket
  // Looking for DFWATL-AA
  void findMatchingFareMarket()
  {
    MyDataHandle mdh;
    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "ATL";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "DFW";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "ATL";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    // Build Vector of Travel Segs to populate the Mileage Travel Route
    // DFWNYC
    AirSeg* aS1 =
        TestAirSegFactory::create("/vobs/atseintl/Fares/test/data/DRVController/AirSeg0.xml");
    // NYCCHI
    AirSeg* aS2 =
        TestAirSegFactory::create("/vobs/atseintl/Fares/test/data/DRVController/AirSeg1.xml");
    // CHIATL
    AirSeg* aS3 =
        TestAirSegFactory::create("/vobs/atseintl/Fares/test/data/DRVController/AirSeg2.xml");

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(aS1);
    tvlSegs.push_back(aS2);
    tvlSegs.push_back(aS3);

    copy(tvlSegs.begin(), tvlSegs.end(), back_inserter(tvlRoute.mileageTravelRoute()));

    // Build a PricingTrx with Multiple Fare Markets
    PricingTrx trx;

    // DFW-NYC-CHI
    FareMarket* fareMarketPtr1 = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/DRVController/fareMarket1.xml");
    trx.fareMarket().push_back(fareMarketPtr1);

    // DFW-NYC
    FareMarket* fareMarketPtr2 = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/DRVController/fareMarket2.xml");
    trx.fareMarket().push_back(fareMarketPtr2);

    // DFW-NYC-CHI-ATL
    FareMarket* fareMarketPtr3 = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/DRVController/fareMarket3.xml");
    fareMarketPtr3->travelSeg().clear();
    fareMarketPtr3->travelSeg().push_back(aS1);
    fareMarketPtr3->travelSeg().push_back(aS2);
    fareMarketPtr3->travelSeg().push_back(aS3);
    trx.fareMarket().push_back(fareMarketPtr3);

    DRVController drv(trx);
    const FareMarket* fmPtr = drv.findMatchingFareMarket(trx, tvlRoute, DateTime::localTime());
    CPPUNIT_ASSERT(fmPtr != 0);
  }

  // Test true condition
  // DFW-CHI-STL-ATL-LON-MAN-VIE
  void buildIntlTravelRoute1A()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 1; // STL
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "DFW", "US", "CHI", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "CHI", "US", "STL", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "STL", "US", "ATL", "US", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "ATL", "US", "LON", "UK", "BA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "LON", "UK", "MAN", "UK", "BA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MAN", "UK", "VIE", "AT", "BA");

    tvlRoute.govCxr() = "BA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "DFW";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "VIE";
    tvlRoute.destinationNation() = "AT";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(ret, true);
    CPPUNIT_ASSERT_EQUAL(size_t(3), intlTvlRoute.travelRoute().size());
  }

  // Test true condition
  // FLR-AZ-FCO-AZ-MIL-AA-FRA-AA-DFW
  void testBuildLocalTravelRouteWhenDiffNationOnThirdSegSPR135990()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "FRA", "DE", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FRA", "DE", "DFW", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(true, ret);
    CPPUNIT_ASSERT_EQUAL(size_t(2), intlTvlRoute.travelRoute().size());
  }

  // Test fail condition
  // FLR-AZ-FCO-AZ-MIL-AA-NAP-AA-DFW
  void testBuildLocalTravelRouteWhenDiffCarrierOnThirdSeg()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "NAP", "IT", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "NAP", "IT", "DFW", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[3];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  // Test pass condition
  // FLR-AA-FCO-AA-MIL-AA-DFW - STOP in NAP between MIL and DFW
  void testBuildLocalTravelRouteWhenSameCarrierOnPrimarySeg()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "DFW", "US", "AZ");

    tvlRoute.govCxr() = "AZ";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlSegs[2]->origAirport() = tvlSegs[2]->boardMultiCity();

    Loc loc;
    loc.loc() = "NAP";
    loc.nation() = "IT";
    tvlSegs[2]->hiddenStops().push_back(&loc);

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(true, ret);
    CPPUNIT_ASSERT_EQUAL(size_t(3), intlTvlRoute.travelRoute().size());
  }

  // Test pass condition
  // FLR-AA-FCO-AA-MIL-AA-DFW - STOP in NAP between MIL and DFW
  void testBuildLocalTravelRouteWhenDiffCarrierOnPrimarySeg()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "DFW", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlSegs[2]->origAirport() = tvlSegs[2]->boardMultiCity();

    Loc loc;
    loc.loc() = "NAP";
    loc.nation() = "IT";
    tvlSegs[2]->hiddenStops().push_back(&loc);

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingOptions opt;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(&opt);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(true, ret);
    CPPUNIT_ASSERT_EQUAL(size_t(2), intlTvlRoute.travelRoute().size());
  }

  // Test fail condition
  // FLR-AA-FCO-AZ-MIL-AA-DFW
  void testBuildLocalTravelRouteWhenDiffCarrierFirstSeg()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "DFW", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  // Test fail condition
  // FLR-AZ-FCO-AA-MIL-AA-DFW
  void testBuildLocalTravelRouteWhenDiffCarrierSecondSeg()
  {
    MyDataHandle mdh;
    int16_t missingCityIndex = 0; // FCO
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;

    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FLR", "IT", "FCO", "IT", "AZ");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "FCO", "IT", "MIL", "IT", "AA");
    addCityCrxAndTvlSeg(tvlRoute.travelRoute(), tvlSegs, "MIL", "IT", "DFW", "US", "AA");

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "FLR";
    tvlRoute.originNation() = "IT";
    tvlRoute.destination() = "DFW";
    tvlRoute.destinationNation() = "US";
    tvlRoute.travelDate() = DateTime::localTime();

    tvlRoute.primarySector() = tvlSegs[2];
    tvlRoute.mileageTravelRoute() = tvlSegs;

    TravelRoute intlTvlRoute;

    PricingRequest request;
    PricingTrx trx;
    trx.setRequest(&request);
    trx.ticketingDate() = DateTime(2012, 6, 18);

    DRVInfo drvInfo;
    drvInfo.notSameCarrier() = false;
    drvInfo.missingCityOrigDest() = false;

    DRVController drv(trx);
    bool ret = drv.buildLocalTravelRoute(trx, tvlRoute, missingCityIndex, intlTvlRoute, drvInfo);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DRVControllerTest);
} // namespace
