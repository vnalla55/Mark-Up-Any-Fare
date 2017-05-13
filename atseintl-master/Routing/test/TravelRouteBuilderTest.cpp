#include "test/include/CppUnitHelperMacros.h"
#include "Routing/TravelRouteBuilder.h"
#include "DataModel/FareMarket.h"
#include <vector>
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/TravelRoute.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "test/include/MockGlobal.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/Loc.h"
#include <iostream>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
namespace
{
class MockTravelRouteBuilder : public TravelRouteBuilder
{
public:
  MockTravelRouteBuilder() {};
  virtual ~MockTravelRouteBuilder() {};
  bool validateFlightTracking()
  {
    // std::cout<< "  From Derived Class " <<std::endl;
    return true;
  }
};
class MyDataHandle : public DataHandleMock
{
public:
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "")
      return "";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }

  const std::vector<AirlineAllianceCarrierInfo*>&
  getAirlineAllianceCarrier(const CarrierCode& carrierCode)
  {
    std::vector<AirlineAllianceCarrierInfo*>* allVec =
        _memHandle.create<std::vector<AirlineAllianceCarrierInfo*>>();
    if (carrierCode == "AA")
    {
      allVec->push_back(_memHandle.create<AirlineAllianceCarrierInfo>());
      allVec->front()->genericAllianceCode() = "*O";
    }
    return *allVec;
  }

protected:
  TestMemHandle _memHandle;
};
}

class TravelRouteBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TravelRouteBuilderTest);
  CPPUNIT_TEST(testWithOutHiddenCity);
  CPPUNIT_TEST(testWithHiddenCity);
  CPPUNIT_TEST(testWithSurfaceSegments);
  CPPUNIT_TEST(testWithBadData);
  CPPUNIT_TEST(testBuildTravelRoute);
  CPPUNIT_TEST(testUpdateCityCarrierWhenGenericFound);
  CPPUNIT_TEST(testUpdateCityCarrierWhenGenericNotFound);

  CPPUNIT_TEST(testBuildTravelRoutes_noHiddenPoints);
  CPPUNIT_TEST(testBuildTravelRoutes_HiddenPoints);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _trBuilder = _memHandle.create<TravelRouteBuilder>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(true);
  }
  void tearDown() { _memHandle.clear(); }

  void testWithOutHiddenCity()
  {
    TravelRoute tvlRoute;
    AirSeg aS1, aS2, aS3;
    // Loc loc1, loc2, loc3, loc4;
    // loc1.loc()= "LAX";
    // loc2.loc()= "BOS";
    // loc3.loc()= "NYC";
    // loc4.loc()= "SJO";

    LocCode loc1, loc2, loc3, loc4;
    loc1 = "LAX";
    loc2 = "BOS";
    loc3 = "NYC";
    loc4 = "SJO";

    aS1.carrier() = "AA";
    aS2.carrier() = "UA";
    aS3.carrier() = "DL";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    Loc lc1, lc2, lc3, lc4;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    // aS1.origin()=&loc1;
    // aS2.origin()=&loc2;
    // aS3.origin()=&loc3;
    // aS1.destination()=&loc2;
    // aS2.destination()=&loc3;
    // aS3.destination()=&loc4;
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlRoute.flightTrackingCxr() = false;
    tvlRoute.doNotApplyDRV() = false;
    tvlRoute.terminalPoints() = false;
    bool rc = _trBuilder->fillCityCarrier(tvlSegs, tvlRoute);

    std::vector<TravelRoute::CityCarrier>::iterator itr = tvlRoute.travelRoute().begin();

    /* std::cout<< "\n\t -------- Route of Travel ------------"<<std::endl;
     std::cout<<"\n\t"<<(*itr).boardCity().loc();
    for(; itr != tvlRoute.travelRoute().end(); itr++)
    {
     std::cout<< " - " << (*itr).carrier(); std::cout<< " - " ;
     std::cout<<(*itr).offCity().loc();
    } */
    CPPUNIT_ASSERT(rc);
  }

  void testWithSurfaceSegments()
  {
    TravelRoute tvlRoute;
    AirSeg aS1, aS3;
    SurfaceSeg aS2;
    LocCode loc1, loc2, loc3, loc4;

    loc1 = "LAX";
    loc2 = "BOS";
    loc3 = "NYC";
    loc4 = "SJO";

    // loc1.loc()= "LAX";
    // loc2.loc()= "BOS";
    // loc3.loc()= "NYC";
    // loc4.loc()= "SJO";

    aS1.carrier() = "AA";
    aS3.carrier() = "DL";

    // aS1.origin()=&loc1;
    // aS2.origin()=&loc2;
    // aS3.origin()=&loc3;
    // aS1.destination()=&loc2;
    // aS2.destination()=&loc3;
    // aS3.destination()=&loc4;

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;

    Loc lc1, lc2, lc3, lc4;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "CR";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);

    tvlRoute.flightTrackingCxr() = false;
    tvlRoute.doNotApplyDRV() = false;
    tvlRoute.terminalPoints() = false;
    bool rc = _trBuilder->fillCityCarrier(tvlSegs, tvlRoute);

    std::vector<TravelRoute::CityCarrier>::iterator itr = tvlRoute.travelRoute().begin();

    /* std::cout<< "\n\t -------- Route of Travel ------------"<<std::endl;
     std::cout<<"\n\t"<<(*itr).boardCity().loc();
    for(; itr != tvlRoute.travelRoute().end(); itr++)
    {
     std::cout<< " - " << (*itr).carrier(); std::cout<< " - " ;
     std::cout<<(*itr).offCity().loc();
    } */
    CPPUNIT_ASSERT(rc);
  }

  void testWithHiddenCity()
  {
    TravelRoute tvlRoute;
    AirSeg aS1, aS2, aS3;
    Loc loc5;
    LocCode loc1, loc2, loc3, loc4;
    loc1 = "PHX";
    loc2 = "LGD";
    loc3 = "JFK";
    loc4 = "CHI";

    loc5.loc() = "MIA";

    aS1.carrier() = "AA";
    aS2.carrier() = "UA";
    aS3.carrier() = "DL";
    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    Loc lc1, lc2, lc3, lc4;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    aS2.hiddenStops().push_back(&loc5);
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlRoute.flightTrackingCxr() = true;
    tvlRoute.doNotApplyDRV() = false;
    tvlRoute.terminalPoints() = false;
    bool rc = _trBuilder->fillCityCarrier(tvlSegs, tvlRoute);

    std::vector<TravelRoute::CityCarrier>::iterator itr = tvlRoute.travelRoute().begin();

    /* std::cout<< "\n\t -------- Route of Travel ------------"<<std::endl;
     std::cout<<"\n\t"<<(*itr).boardCity().loc();
    for(; itr != tvlRoute.travelRoute().end(); itr++)
    {
     std::cout<< " - " << (*itr).carrier(); std::cout<< " - " ;
     std::cout<<(*itr).offCity().loc();
    } */
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(aS2.hiddenStops().size() == 1);
    CPPUNIT_ASSERT(tvlRoute.travelRoute().size() == (tvlSegs.size() + aS2.hiddenStops().size()));
  }

  void testWithBadData()
  {
    TravelRoute tvlRoute;
    std::vector<TravelSeg*> tvlSegs;
    tvlRoute.flightTrackingCxr() = false;
    tvlRoute.doNotApplyDRV() = false;
    tvlRoute.terminalPoints() = false;
    bool rc = _trBuilder->fillCityCarrier(tvlSegs, tvlRoute);
    CPPUNIT_ASSERT(!rc);
  }

  void testBuildTravelRoute()
  {
    TravelRoute tvlRoute;
    AirSeg aS1, aS2, aS3;
    LocCode loc1, loc2, loc3, loc4;

    loc1 = "LAX";
    loc2 = "BOS";
    loc3 = "NYC";
    loc4 = "SJO";

    aS1.carrier() = "AA";
    aS2.carrier() = "UA";
    aS3.carrier() = "DL";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;

    Loc lc1, lc2, lc3, lc4;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    aS1.carrier() = "AA";
    aS2.carrier() = "UA";
    aS3.carrier() = "DL";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;

    bool rc = _trBuilder->buildTravelRoute(*_trx, fm, tvlRoute);
    CPPUNIT_ASSERT(rc);
  }

  void testBuildTravelRoutes_noHiddenPoints()
  {
    Loc lc1, lc2, lc3, lc4;

    AirSeg aS1, aS2, aS3;
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);

    FareMarket fm;
    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;

    TravelRoute tvlRoute;
    TravelRoute travelRouteTktOnly;
    tvlRoute.travelRouteTktOnly() = &travelRouteTktOnly;

    // CPPUNIT_ASSERT(_trBuilder.buildTravelRoutes(TKTPTS_TKTONLY, trx, fm, tvlRoute));
    CPPUNIT_ASSERT(_trBuilder->buildTravelRoutes(*_trx, fm, tvlRoute));
    CPPUNIT_ASSERT(tvlRoute.travelRouteTktOnly() == 0);
  }

  void testBuildTravelRoutes_HiddenPoints()
  {
    Loc lc1, lc2, lc3, lc4, lc5;

    AirSeg aS1, aS2, aS3;
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    aS2.hiddenStops().push_back(&lc5);

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);

    FareMarket fm;
    fm.setFltTrkIndicator(true);
    fm.travelSeg() = tvlSegs;

    TravelRoute tvlRoute;
    TravelRoute travelRouteTktOnly;
    tvlRoute.travelRouteTktOnly() = &travelRouteTktOnly;

    CPPUNIT_ASSERT(_trBuilder->buildTravelRoutes(*_trx, fm, tvlRoute));
    // CPPUNIT_ASSERT(_trBuilder.buildTravelRoutes(TKTPTS_ANY, trx, fm, tvlRoute));
    CPPUNIT_ASSERT(aS2.hiddenStops().size() == 1);
    CPPUNIT_ASSERT(tvlRoute.travelRouteTktOnly() != 0);
    CPPUNIT_ASSERT(tvlRoute.travelRouteTktOnly()->travelRoute().size() !=
                   tvlRoute.travelRoute().size());
  }

  void testUpdateCityCarrierWhenGenericFound()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;

    cityCarrier.carrier() = "AA";
    tvlRoute.travelRoute().push_back(cityCarrier);

    _trBuilder->updateCityCarrier(*_trx, tvlRoute);
    GenericAllianceCode gac = "*O";

    CPPUNIT_ASSERT_EQUAL(gac, tvlRoute.travelRoute().front().genericAllianceCode());
  }

  void testUpdateCityCarrierWhenGenericNotFound()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;

    cityCarrier.carrier() = "UA";
    tvlRoute.travelRoute().push_back(cityCarrier);

    _trBuilder->updateCityCarrier(*_trx, tvlRoute);

    CPPUNIT_ASSERT(tvlRoute.travelRoute().front().genericAllianceCode().empty());
  }

private:
  TestMemHandle _memHandle;
  TravelRouteBuilder* _trBuilder;
  PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TravelRouteBuilderTest);
}

