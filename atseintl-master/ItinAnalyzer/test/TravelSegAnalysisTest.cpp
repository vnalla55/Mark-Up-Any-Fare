#include "test/include/CppUnitHelperMacros.h"

#include "Common/ItinUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include <iostream>
#include <vector>

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->orig() = origin;
    ret->dest() = dest;
    if (origin == "BRU" && dest == "ZRH")
    {
      ret->mileage() = 309;
      return ret;
    }
    else if (origin == "GLA" && dest == "LON")
    {
      ret->mileage() = 345;
      return ret;
    }
    else if (origin == "LON" && dest == "BRU")
    {
      ret->mileage() = 203;
      return ret;
    }
    else if (origin == "ROM" && dest == "ATH")
    {
      ret->mileage() = 667;
      return ret;
    }
    else if (origin == "ZRH" && dest == "ROM")
    {
      ret->mileage() = 435;
      return ret;
    }
    else if (origin == "AMS" && dest == "IST")
    {
      ret->mileage() = 1384;
      return ret;
    }
    else if (origin == "DUB" && dest == "AMS")
    {
      ret->mileage() = 467;
      return ret;
    }
    else if (origin == "HEL" && dest == "RIX")
    {
      ret->mileage() = 238;
      return ret;
    }
    else if (origin == "RIX" && dest == "WAW")
    {
      ret->mileage() = 350;
      return ret;
    }
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
};
}
class TravelSegAnalysisTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TravelSegAnalysisTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testgeoTravelType);
  CPPUNIT_TEST(testgeoTravelTypeItin);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _srv = _memHandle.create<MockTseServer>();
    _memHandle.create<MyDataHandle>();
  }
  void tearDown() { _memHandle.clear(); }
  void testProcess()
  {
    try
    {
      PricingTrx trx;
      Itin itn;
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testgeoTravelType()
  {
    AirSeg aS1, aS2, aS3, aS4, aS5;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("PLZ", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("CPT", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("JNB", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    if (loc1 && loc2 && loc3)
    {
      aS1.origin() = loc1;
      aS1.destination() = loc2;
      aS2.origin() = loc2;
      aS2.destination() = loc3;

      aS1.departureDT() = DateTime::localTime();
      aS2.departureDT() = DateTime::localTime();

      aS1.carrier() = "BA";
      aS2.carrier() = "BA";
      std::vector<TravelSeg*> tvlSegs;

      tvlSegs.push_back(&aS1);
      tvlSegs.push_back(&aS2);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      TravelSegAnalysis _tvlSegAnalysis;
      Boundary boundary = _tvlSegAnalysis.selectTravelBoundary(tvlSegs);
      CPPUNIT_ASSERT(boundary == Boundary::EXCEPT_USCA);
      ItinAnalyzerService itin("ITIN_SVC", *_srv);
      ItinUtil::setGeoTravelType(boundary, fm);
      CPPUNIT_ASSERT(fm.geoTravelType() == GeoTravelType::ForeignDomestic);
    }
  }

  void testgeoTravelTypeItin()
  {
    AirSeg aS1, aS2, aS3, aS4, aS5;

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("YYT", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("FSP", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("YYT", DateTime::localTime());

    //---------------------
    // create the AirSegs
    //--------------------
    if (loc1 && loc2 && loc3)
    {
      aS1.origin() = loc1;
      aS1.destination() = loc2;
      aS2.origin() = loc2;
      aS2.destination() = loc3;

      aS1.departureDT() = DateTime::localTime();
      aS2.departureDT() = DateTime::localTime();

      aS1.carrier() = "BA";
      aS2.carrier() = "BA";
      std::vector<TravelSeg*> tvlSegs;

      tvlSegs.push_back(&aS1);
      tvlSegs.push_back(&aS2);

      FareMarket fm;
      fm.travelSeg() = tvlSegs;
      TravelSegAnalysis _tvlSegAnalysis;
      Boundary boundary = _tvlSegAnalysis.selectTravelBoundary(tvlSegs);

      ItinAnalyzerService itin("ITIN_SVC", *_srv);
      ItinUtil::setGeoTravelType(boundary, fm);
      CPPUNIT_ASSERT(fm.geoTravelType() == GeoTravelType::International);
    }
  }

private:
  TestMemHandle _memHandle;
  MockTseServer* _srv;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TravelSegAnalysisTest);
}
