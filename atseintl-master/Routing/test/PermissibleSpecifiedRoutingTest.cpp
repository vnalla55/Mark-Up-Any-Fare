#include <iostream>
#include <vector>
#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "Common/TseEnums.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingOptions.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageRoute.h"
#include "Common/DateTime.h"
#include "Routing/TravelRoute.h"
#include "DataModel/PricingTrx.h"
#include "Routing/PermissibleSpecifiedRouting.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "Common/TseUtil.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class PermissibleSpecifiedRoutingTest : public CppUnit::TestFixture
{
  // Add some functionality to DataHandlerMock
  class PSRDataHandleMock : public DataHandleMock
  {
  public:
    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "ROM")
        return "ROM";
      if (locCode == "RIO")
        return "RIO";
      if (locCode == "SAO")
        return "SAO";
      if (locCode == "NYC")
        return "NYC";
      if (locCode == "DUB")
        return "DUB";
      if (locCode == "OSL")
        return "OSL";
      if (locCode == "FRA")
        return "FRA";
      if (locCode == "YTO")
        return "YTO";
      if (locCode == "ASU")
        return "ASU";
      if (locCode == "STO")
        return "STO";
      if (locCode == "MEX")
        return "MEX";
      if (locCode == "SCL")
        return "SCL";
      if (locCode == "PAR")
        return "PAR";
      if (locCode == "DEN")
        return "DEN";
      if (locCode == "CHI")
        return "CHI";

      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }

    const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                          const CarrierCode& carrier,
                                          Indicator area1,
                                          Indicator area2,
                                          const DateTime& date)
    {
      static std::vector<TpdPsr*> ret;
      return ret;
    }
  };

  CPPUNIT_TEST_SUITE(PermissibleSpecifiedRoutingTest);

  CPPUNIT_TEST(testMileageRoute);
  CPPUNIT_TEST(testThruMktCxrsA);
  CPPUNIT_TEST(testThruMktCxrsB);
  CPPUNIT_TEST(testThruMktCxrsC);
  CPPUNIT_TEST(testViaMktSameCarrierA);
  CPPUNIT_TEST(testViaMktSameCarrierB);
  CPPUNIT_TEST(testViaMktSameCarrierC);
  CPPUNIT_TEST(testViaCxrLocsA);
  CPPUNIT_TEST(testViaCxrLocsB);
  CPPUNIT_TEST(testViaCxrLocsC);
  CPPUNIT_TEST(testViaCxrLocExceptionsA);
  CPPUNIT_TEST(testViaCxrLocExceptionsB);
  CPPUNIT_TEST(testViaCxrLocExceptionsC);
  CPPUNIT_TEST(testViaCxrLocExceptionsD);
  CPPUNIT_TEST(testViaGeoLocs);
  CPPUNIT_TEST(testViaGeoLocsA);
  CPPUNIT_TEST(testViaGeoLocsB);
  CPPUNIT_TEST(testViaGeoLocsC);
  CPPUNIT_TEST(testViaGeoLocsD);
  CPPUNIT_TEST(testViaGeoLocsE);
  CPPUNIT_TEST(testViaGeoLocsF);
  CPPUNIT_TEST(testViaGeoLocsG);
  CPPUNIT_TEST(testViaGeoLocsH);
  CPPUNIT_TEST(testViaGeoLocsI);
  CPPUNIT_TEST(testViaGeoLocsJ);
  CPPUNIT_TEST(testViaGeoLocsK);
  CPPUNIT_TEST(testViaGeoLocsL);
  CPPUNIT_TEST(testViaGeoLocsM);
  CPPUNIT_TEST(testViaGeoLocsN);
  CPPUNIT_TEST(testViaGeoLocsO);
  CPPUNIT_TEST(testViaGeoLocsP);
  CPPUNIT_TEST(testViaGeoLocsQ);
  CPPUNIT_TEST(testViaGeoLocsR);
  CPPUNIT_TEST(testViaGeoLocsS);
  CPPUNIT_TEST(testViaGeoLocsT);
  CPPUNIT_TEST(testViaGeoLocsU);
  CPPUNIT_TEST(testViaGeoLocsV);
  CPPUNIT_TEST(testViaGeoLocsW);
  CPPUNIT_TEST(testViaGeoLocsX);
  CPPUNIT_TEST(testViaGeoLocsY);
  CPPUNIT_TEST(testViaGeoLocsZ);
  CPPUNIT_TEST(testViaGeoLocsAA);
  CPPUNIT_TEST(testViaGeoLocsBB);
  CPPUNIT_TEST(testViaGeoLocsCC);
  CPPUNIT_TEST(testReverseTrip);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  PSRDataHandleMock* _psrMock;
  DataHandle* _dataHandle;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _psrMock = _memHandle.create<PSRDataHandleMock>();
    _dataHandle = _memHandle.create<DataHandle>();
    _trx->setRequest(_request);
  }

  void tearDown() { _memHandle.clear(); }

  // i.e. LAX-NYC
  void makeGeoLocSeg(const LocCode& loc1,
                     const Indicator& indicator,
                     const LocCode& loc2,
                     std::vector<TpdPsrViaGeoLoc*>& geoLocV,
                     int setNo = 1,
                     const LocTypeCode& locType = 'C')
  {
    geoLocV.push_back(makeGeoLoc(loc1, 1, ' ', setNo, locType));
    geoLocV.push_back(makeGeoLoc(loc2, 2, indicator, setNo, locType));
  }

  // i.e. LAX-NYC-LON
  void makeGeoLocSeg(const LocCode& loc1,
                     const Indicator& indicator1,
                     const LocCode& loc2,
                     const Indicator& indicator2,
                     const LocCode& loc3,
                     std::vector<TpdPsrViaGeoLoc*>& geoLocV,
                     int setNo = 1,
                     const LocTypeCode& locType = 'C')
  {
    makeGeoLocSeg(loc1, indicator1, loc2, geoLocV, setNo, locType);
    geoLocV.push_back(makeGeoLoc(loc3, 3, indicator2, setNo, locType));
  }

  // i.e. LAX-NYC-LON-KRK
  void makeGeoLocSeg(const LocCode& loc1,
                     const Indicator& indicator1,
                     const LocCode& loc2,
                     const Indicator& indicator2,
                     const LocCode& loc3,
                     const Indicator& indicator3,
                     const LocCode& loc4,
                     std::vector<TpdPsrViaGeoLoc*>& geoLocV,
                     int setNo = 1,
                     const LocTypeCode& locType = 'C')
  {
    makeGeoLocSeg(loc1, indicator1, loc2, indicator2, loc3, geoLocV, setNo, locType);
    geoLocV.push_back(makeGeoLoc(loc4, 4, indicator3, setNo, locType));
  }

  // i.e. LAX-NYC-LON-KRK-AAA
  void makeGeoLocSeg(const LocCode& loc1,
                     const Indicator& indicator1,
                     const LocCode& loc2,
                     const Indicator& indicator2,
                     const LocCode& loc3,
                     const Indicator& indicator3,
                     const LocCode& loc4,
                     const Indicator& indicator4,
                     const LocCode& loc5,
                     std::vector<TpdPsrViaGeoLoc*>& geoLocV,
                     int setNo = 1,
                     const LocTypeCode& locType = 'C')
  {
    makeGeoLocSeg(
        loc1, indicator1, loc2, indicator2, loc3, indicator3, loc4, geoLocV, setNo, locType);
    geoLocV.push_back(makeGeoLoc(loc5, 5, indicator4, setNo, locType));
  }

  TpdPsrViaGeoLoc* makeGeoLoc(const LocCode& loc,
                              int orderNo,
                              const Indicator& indicator = ' ',
                              int setNo = 1,
                              const LocTypeCode& locType = 'C')
  {
    TpdPsrViaGeoLoc* _getLoc = new TpdPsrViaGeoLoc();
    _getLoc->setNo() = setNo;
    _getLoc->orderNo() = orderNo;
    _getLoc->loc().locType() = locType;
    _getLoc->loc().loc() = loc;
    _getLoc->relationalInd() = indicator;
    _getLoc->stopoverNotAllowed() = NO;
    return _getLoc;
  }

  TpdPsrViaCxrLoc* makeCxrLoc(const LocCode& loc1,
                              const LocCode& loc2,
                              const CarrierCode& carrier,
                              const LocTypeCode& locType1 = 'C',
                              const LocTypeCode& locType2 = 'C')
  {
    TpdPsrViaCxrLoc* viaCxrLoc = new TpdPsrViaCxrLoc();
    viaCxrLoc->viaCarrier() = carrier;
    viaCxrLoc->loc1().locType() = locType1;
    viaCxrLoc->loc1().loc() = loc1;
    viaCxrLoc->loc2().locType() = locType1;
    viaCxrLoc->loc2().loc() = loc2;
    return viaCxrLoc;
  }

  TpdPsrViaExcept* makeExpect(const LocCode& loc1, const LocCode& loc2, const CarrierCode& carrier)
  {
    TpdPsrViaExcept* viaExcepts = new TpdPsrViaExcept();
    viaExcepts->viaCarrier() = carrier;
    viaExcepts->loc1().locType() = 'C';
    viaExcepts->loc1().loc() = loc1;
    viaExcepts->loc2().locType() = 'C';
    viaExcepts->loc2().loc() = loc2;
    return viaExcepts;
  }

  TpdPsr& buildPSR(const LocCode& loc1,
                   const LocCode& loc2,
                   const CarrierCode& carrier,
                   const LocTypeCode& locType = 'N',
                   const Indicator& area2 = '1',
                   int stopoverCnt = 1,
                   const Indicator& thruMktCarrierExc = NO,
                   const Indicator& thruViaMktSameCxr = NO)
  {
    TpdPsr* psr;
    psr = _memHandle.create<TpdPsr>();

    psr->applInd() = 'P';
    psr->carrier() = carrier;
    psr->area1() = '1';
    psr->area2() = area2;
    strToGlobalDirection(psr->globalDir(), "WH");
    psr->isiCode() = "";
    psr->fareTypeAppl() = ' ';
    psr->psrHip() = ' ';
    psr->loc1().locType() = locType;
    psr->loc1().loc() = loc1;
    psr->loc2().locType() = 'C';
    psr->loc2().loc() = loc2;
    psr->thisCarrierRestr() = NO;
    psr->thruViaLocRestr() = NO;
    psr->stopoverCnt() = stopoverCnt;
    psr->thruViaMktSameCxr() = thruViaMktSameCxr;
    psr->tpdThruViaMktOnlyInd() = NO;
    psr->thruMktCarrierExcept() = thruMktCarrierExc;

    return (*psr);
  }

  AirSeg* makeAirSeg(const LocCode& boardMultiCity,
                     const LocCode& offMultiCity,
                     const CarrierCode& carrier,
                     LocCode origAirport = "",
                     LocCode destAirport = "")
  {
    const Loc* loc1 = _dataHandle->getLoc(boardMultiCity, DateTime::localTime());
    const Loc* loc2 = _dataHandle->getLoc(offMultiCity, DateTime::localTime());

    if ("" == origAirport)
      origAirport = boardMultiCity;
    if ("" == destAirport)
      destAirport = offMultiCity;

    AirSeg* as = _memHandle.create<AirSeg>();
    as->origin() = const_cast<Loc*>(loc1);
    as->destination() = const_cast<Loc*>(loc2);
    as->boardMultiCity() = boardMultiCity;
    as->offMultiCity() = offMultiCity;
    as->origAirport() = origAirport;
    as->destAirport() = destAirport;
    as->carrier() = carrier;
    return as;
  }

  TravelRoute&
  makeTravelRoute(const CarrierCode& govCxr, const GlobalDirection& globalDir = GlobalDirection::WH)
  {
    TravelRoute* tvlRoute = _memHandle.create<TravelRoute>();
    tvlRoute->govCxr() = govCxr;
    tvlRoute->geoTravelType() = GeoTravelType::International;
    tvlRoute->globalDir() = globalDir;
    tvlRoute->travelDate() = DateTime::localTime();
    return (*tvlRoute);
  }

  bool buildMileageRoute(TravelRoute& tvlRoute, MileageRoute& mRoute, bool setStopoverFlag = false)
  {
    DateTime ticketingDT = DateTime::localTime();
    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(*_trx, tvlRoute, mRoute, *_dataHandle, ticketingDT);

    if (true == setStopoverFlag)
    {
      MileageRouteItems::iterator it = mRoute.mileageRouteItems().begin();
      for (; it != mRoute.mileageRouteItems().end(); it++)
      {
        it->isStopover() = false;
      }
    }
    return rc;
  }

  void makeAirSegs_ROM_LON_MIA_BUE_SCL(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("ROM", "LON", "AA", "ROM", "LHR"));
    tvlSegs.push_back(makeAirSeg("LON", "MIA", "AA", "LHR", "MIA"));
    tvlSegs.push_back(makeAirSeg("MIA", "BUE", "AA"));
    tvlSegs.push_back(makeAirSeg("BUE", "SCL", "AA"));
  }

  void makeAirSegs_AAA_RIO_SAO_NYC(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("AAA", "RIO", "AA"));
    tvlSegs.push_back(makeAirSeg("RIO", "SAO", "AA"));
    tvlSegs.push_back(makeAirSeg("SAO", "NYC", "AA", "SAO", "LGA"));
  }

  void makeAirSegs_LON_DUB_AAA_OSL(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("LON", "DUB", "EI", "LHR", "DUB"));
    tvlSegs.push_back(makeAirSeg("DUB", "AAA", "EI"));
    tvlSegs.push_back(makeAirSeg("AAA", "OSL", "EI"));
  }

  void makeAirSegs_KRK_YTO_RIO_SAO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "YTO", "AC"));
    tvlSegs.push_back(makeAirSeg("YTO", "RIO", "RG"));
    tvlSegs.push_back(makeAirSeg("RIO", "SAO", "RG"));
    tvlSegs.push_back(makeAirSeg("SAO", "ASU", "RG"));
  }

  void makeAirSegs_KRK_YTO_RIO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "YTO", "AC"));
    tvlSegs.push_back(makeAirSeg("YTO", "RIO", "RG"));
    tvlSegs.push_back(makeAirSeg("RIO", "ASU", "RG"));
  }

  void makeAirSegs_KRK_RIO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "RIO", "AC"));
    tvlSegs.push_back(makeAirSeg("RIO", "ASU", "RG"));
  }

  void makeAirSegs_KRK_SAO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "SAO", "AC"));
    tvlSegs.push_back(makeAirSeg("SAO", "ASU", "RG"));
  }

  void makeAirSegs_KRK_YTO_SAO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "YTO", "AC"));
    tvlSegs.push_back(makeAirSeg("YTO", "SAO", "RG"));
    tvlSegs.push_back(makeAirSeg("SAO", "ASU", "RG"));
  }

  void makeAirSegs_KRK_YTO_ASU(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("KRK", "YTO", "AC"));
    tvlSegs.push_back(makeAirSeg("YTO", "ASU", "RG"));
  }

  void makeAirSegs_LON_FRA_PAR_OSL(std::vector<TravelSeg*>& tvlSegs)
  {
    tvlSegs.push_back(makeAirSeg("LON", "FRA", "LH", "LHR", "FRA"));
    tvlSegs.push_back(makeAirSeg("FRA", "PAR", "LH", "FRA", "ORY"));
    tvlSegs.push_back(makeAirSeg("PAR", "OSL", "LH", "ORY", "OSL"));
  }

  void setUpAndBuildRoute(MileageRoute& mRoute)
  {
    TravelRoute& tvlRoute = makeTravelRoute("AA", GlobalDirection::AT);
    makeAirSegs_ROM_LON_MIA_BUE_SCL(tvlRoute.mileageTravelRoute());
    buildMileageRoute(tvlRoute, mRoute);
  }

  void addCarrierCode(TpdPsr& psr)
  {
    psr.carrier() = "";
    psr.thruMktCxrs().push_back(CarrierCode("BA"));
    psr.thruMktCxrs().push_back(CarrierCode("CO"));
    psr.thruMktCxrs().push_back(CarrierCode("AA"));
  }

  //******************************************************************
  //      Tests implementation
  //******************************************************************

  // Test function to validate through market carriers or exceptions
  void testMileageRoute()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("JL", GlobalDirection::EH);

    // create AirSegs
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("TYO", "AAA", "JL"));
    tvlSegs.push_back(makeAirSeg("AAA", "BBB", "TG"));
    tvlSegs.push_back(makeAirSeg("BBB", "KRK", "CX"));
    tvlSegs.push_back(makeAirSeg("KRK", "KRK", "PK", "LHR", "LHR"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    buildMileageRoute(tvlRoute, mRoute);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.apply(mRoute);
    CPPUNIT_ASSERT(!rc);
  }

  // Test function to validate through market carriers or exceptions
  void testThruMktCxrsA()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2');
    setUpAndBuildRoute(mRoute);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processThruMktCxrs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testThruMktCxrsB()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2');
    setUpAndBuildRoute(mRoute);

    addCarrierCode(psr);
    psr.thruMktCarrierExcept() = NO;

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processThruMktCxrs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testThruMktCxrsC()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2');
    setUpAndBuildRoute(mRoute);

    addCarrierCode(psr);
    psr.thruMktCarrierExcept() = YES;

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processThruMktCxrs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // Test function to validate through via Market Carrier Restrictions
  void testViaMktSameCarrierA()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 1, NO, YES);
    setUpAndBuildRoute(mRoute);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaMktSameCarrier(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testViaMktSameCarrierB()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 1, NO, YES);
    setUpAndBuildRoute(mRoute);
    addCarrierCode(psr);

    psr.thruMktCarrierExcept() = NO;
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaMktSameCarrier(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testViaMktSameCarrierC()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 1, NO, YES);
    setUpAndBuildRoute(mRoute);
    addCarrierCode(psr);

    psr.thruMktCarrierExcept() = YES;
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaMktSameCarrier(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // Test function to validate through via Carrier Locs
  void testViaCxrLocExceptionsA()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaExcepts().push_back(makeExpect("LON", "MIA", "BA"));

    // Test True condition
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocExceptions(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testViaCxrLocExceptionsB()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaExcepts().push_back(makeExpect("LON", "MIA", "AA"));

    // Test Nation Location match - expect true
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocExceptions(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  void testViaCxrLocExceptionsC()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaExcepts().push_back(makeExpect("BUE", "SCL", "LC"));
    psr.viaExcepts().push_back(makeExpect("LON", "MIA", "AA"));

    // Test Nation Location match - expect false
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocExceptions(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  void testViaCxrLocExceptionsD()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaExcepts().push_back(makeExpect("SCL", "BUE", "LC"));
    psr.viaExcepts().push_back(makeExpect("LON", "MIA", "AA"));

    // Test Nation Location match - expect false
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocExceptions(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // Test function to validate through via Carrier Locs
  void testViaCxrLocsA()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaCxrLocs().push_back(makeCxrLoc("LON", "MIA", "AA"));

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testViaCxrLocsB()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaCxrLocs().push_back(makeCxrLoc("US", "UK", "AA", 'N', 'N'));

    // Test Nation Location match - expect true
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testViaCxrLocsC()
  {
    MileageRoute mRoute;
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', 0);
    setUpAndBuildRoute(mRoute);

    psr.viaCxrLocs().push_back(makeCxrLoc("GB", "US", "TW", 'N', 'N'));
    psr.viaCxrLocs().push_back(makeCxrLoc("BUE", "SCL", "AA"));

    // Test Nation Location match - expect false
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaCxrLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // Test function to validate through via Geo Locs
  // TravelRoute:  ROM-LON-MIA-BUE-SCL
  // GeoLocs:      MIA/SJU/BNA
  // Expected:     FAIL
  void testViaGeoLocs()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("AA", GlobalDirection::AT);
    TpdPsr& psr = buildPSR("ROM", "SCL", "AA", 'C', '2', -1);

    makeAirSegs_ROM_LON_MIA_BUE_SCL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "MIA", VIAGEOLOCREL_OR, "SJU", VIAGEOLOCREL_OR, "BNA", psr.viaGeoLocs()); // MIA/SJU/BNA
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // GeoLocs Test Case A
  // TravelRoute:  AAA-RIO-SAO-NYC
  // GeoLocs:      RIO/SAO
  // Expected:     FAIL
  void testViaGeoLocsA()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("AA");
    TpdPsr& psr = buildPSR("AAA", "NYC", "AA", 'C');

    makeAirSegs_AAA_RIO_SAO_NYC(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // GeoLocs Test Case B
  // TravelRoute:  AAA-RIO-SAO-NYC
  // GeoLocs:      RIO/SAO
  //               RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsB()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("AA");
    TpdPsr& psr = buildPSR("AAA", "NYC", "AA", 'C');

    makeAirSegs_AAA_RIO_SAO_NYC(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // RIO/SAO
    makeGeoLocSeg("RIO", VIAGEOLOCREL_ANDOR, "SAO", psr.viaGeoLocs(), 2); // RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case C
  // validate Geo Locs with multiple sets
  // TravelRoute:  AAA-RIO-SAO-NYC
  // GeoLocs:      SAO & SFO
  //               RIO & SAO
  // Expected:     PASS
  void testViaGeoLocsC()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("AA");
    TpdPsr& psr = buildPSR("AAA", "NYC", "AA", 'C');

    // create AirSegs AAA-RIO-SAO-NYC
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("AAA", "RIO", "AA"));
    tvlSegs.push_back(makeAirSeg("RIO", "SAO", "AA"));
    tvlSegs.push_back(makeAirSeg("SAO", "NYC", "AA", "SAO", "JFK"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    makeGeoLocSeg("SAO", VIAGEOLOCREL_AND, "SFO", psr.viaGeoLocs()); // SAO & SFO
    makeGeoLocSeg("RIO", VIAGEOLOCREL_AND, "SAO", psr.viaGeoLocs(), 2); // RIO & SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case D
  // validate Geo Locs with multiple sets
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA
  //               DE-FR
  // Expected:     PASS
  void testViaGeoLocsD()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg("DUB", VIAGEOLOCREL_AND, "AAA", psr.viaGeoLocs(), 2); // DUB & AAA
    makeGeoLocSeg("DE", VIAGEOLOCREL_ANDOR, "FR", psr.viaGeoLocs(), 3); // DE-FR
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case E
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA & VIE
  //               DE-FR
  // Expected:     FAIL
  void testViaGeoLocsE()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN;
    makeGeoLocSeg(
        "DUB", VIAGEOLOCREL_AND, "AAA", VIAGEOLOCREL_AND, "VIE", psr.viaGeoLocs(), 2); // DUB & AAA
                                                                                       // & VIE
    makeGeoLocSeg("DE", VIAGEOLOCREL_ANDOR, "FR", psr.viaGeoLocs(), 3, 'N'); // DE - FR
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // GeoLocs Test Case F
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA-VIE
  // Expected:     PASS
  void testViaGeoLocsF()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());

    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg(
        "DUB", VIAGEOLOCREL_AND, "AAA", VIAGEOLOCREL_ANDOR, "VIE", psr.viaGeoLocs(), 2); // DUB &
                                                                                         // AAA-VIE
    makeGeoLocSeg("DE", VIAGEOLOCREL_ANDOR, "FR", psr.viaGeoLocs(), 3, 'N'); // DE - FR
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case G
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & VIE/LON/AAA
  // Expected:     PASS
  void testViaGeoLocsG()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg("DUB",
                  VIAGEOLOCREL_AND,
                  "VIE",
                  VIAGEOLOCREL_OR,
                  "LON",
                  VIAGEOLOCREL_OR,
                  "AAA",
                  psr.viaGeoLocs(),
                  2); // DUB & VIE/LON/AAA
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case H
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA/LON/VIE & AUS
  // Expected:     FAIL
  void testViaGeoLocsH()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg("DUB",
                  VIAGEOLOCREL_AND,
                  "AAA",
                  VIAGEOLOCREL_OR,
                  "LON",
                  VIAGEOLOCREL_OR,
                  "VIE",
                  VIAGEOLOCREL_AND,
                  "ASU",
                  psr.viaGeoLocs(),
                  2); // DUB & AAA/LON/VIE & AUS
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // GeoLocs Test Case I
  // TravelRoute:  LON-DUB-AAA-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA/LON/VIE
  // Expected:     PASS
  void testViaGeoLocsI()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("EI", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_DUB_AAA_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg("DUB",
                  VIAGEOLOCREL_ANDOR,
                  "AAA",
                  VIAGEOLOCREL_OR,
                  "LON",
                  VIAGEOLOCREL_OR,
                  "VIE",
                  VIAGEOLOCREL_ANDOR,
                  "ASU",
                  psr.viaGeoLocs(),
                  2); // DUB-AAA/LON/VIE-AUS
    makeGeoLocSeg("DUB",
                  VIAGEOLOCREL_AND,
                  "AAA",
                  VIAGEOLOCREL_OR,
                  "LON",
                  VIAGEOLOCREL_OR,
                  "VIE",
                  psr.viaGeoLocs(),
                  3); // DUB & AAA/LON/VIE
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case J
  // TravelRoute:  LON-FRA-PAR-OSL
  // GeoLocs:      LON/MAN
  //               DUB & AAA/LON
  //               DE - FR
  // Expected:     PASS
  void testViaGeoLocsJ()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("LH", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("LON", "OSL", "AA", 'C');

    makeAirSegs_LON_FRA_PAR_OSL(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("LON", VIAGEOLOCREL_OR, "MAN", psr.viaGeoLocs()); // LON/MAN
    makeGeoLocSeg(
        "DUB", VIAGEOLOCREL_ANDOR, "AAA", VIAGEOLOCREL_OR, "LON", psr.viaGeoLocs(), 2); // DUB &
                                                                                        // AAA/LON
    makeGeoLocSeg("DE", VIAGEOLOCREL_ANDOR, "FR", psr.viaGeoLocs(), 3, 'N'); // DE - FR
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  void testReverseTrip()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("LH", GlobalDirection::EH);

    makeAirSegs_LON_FRA_PAR_OSL(tvlRoute.mileageTravelRoute());
    buildMileageRoute(tvlRoute, mRoute, true);

    MileageRoute reverseMileageRoute;
    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    exclusion.reverseRoute(mRoute, reverseMileageRoute);
    CPPUNIT_ASSERT_EQUAL(3, (int)mRoute.mileageRouteItems().size());
  }

  // GeoLocs Test Case K
  // TravelRoute:  KRK-YTO-RIO-SAO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     FAIL
  void testViaGeoLocsK()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "AA");

    makeAirSegs_KRK_YTO_RIO_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(!rc);
  }

  // GeoLocs Test Case L
  // TravelRoute:  KRK-YTO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     PASS
  void testViaGeoLocsL()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case M
  // TravelRoute:  KRK-YTO-RIO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     PASS
  void testViaGeoLocsM()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "AA");

    makeAirSegs_KRK_YTO_RIO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case N
  // TravelRoute:  KRK-YTO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsN()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case O
  // TravelRoute:  KRK-YTO-RIO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsO()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_RIO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case P
  // TravelRoute:  KRK-YTO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsP()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case Q
  // TravelRoute:  DEN-CHI-ASU
  // GeoLocs:      CHI/DFW/BNA-NYC-BUE
  // Expected:     PASS
  void testViaGeoLocsQ()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("UA");
    TpdPsr& psr = buildPSR("US", "ASU", "UA");

    // create AirSegs DEN-CHI-ASU
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("DEN", "CHI", "UA", "DEN", "ORD"));
    tvlSegs.push_back(makeAirSeg("CHI", "ASU", "UA", "ORD", "ASU"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    // CHI/DFW/BNA-NYC-BUE
    makeGeoLocSeg("CHI",
                  VIAGEOLOCREL_OR,
                  "DFW",
                  VIAGEOLOCREL_OR,
                  "BNA",
                  VIAGEOLOCREL_ANDOR,
                  "NYC",
                  VIAGEOLOCREL_ANDOR,
                  "BUE",
                  psr.viaGeoLocs());
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case R
  // TravelRoute:  DEN-CHI-NYC-ASU
  // GeoLocs:      CHI/DFW/BNA-NYC-BUE
  // Expected:     PASS
  void testViaGeoLocsR()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("UA");
    TpdPsr& psr = buildPSR("US", "ASU", "UA");

    // create AirSegs DEN-CHI-NYC-ASU
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("DEN", "CHI", "UA", "DEN", "ORD"));
    tvlSegs.push_back(makeAirSeg("CHI", "NYC", "UA", "ORD", "JFK"));
    tvlSegs.push_back(makeAirSeg("NYC", "ASU", "UA", "JFK", "ASU"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    // CHI/DFW/BNA-NYC-BUE
    makeGeoLocSeg("CHI",
                  VIAGEOLOCREL_OR,
                  "DFW",
                  VIAGEOLOCREL_OR,
                  "BNA",
                  VIAGEOLOCREL_ANDOR,
                  "NYC",
                  VIAGEOLOCREL_ANDOR,
                  "BUE",
                  psr.viaGeoLocs());
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case S
  // TravelRoute:  DEN-CHI-BUE-ASU
  // GeoLocs:      CHI/DFW/BNA-NYC-BUE
  // Expected:     PASS
  void testViaGeoLocsS()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("UA");
    TpdPsr& psr = buildPSR("US", "ASU", "UA");

    // create AirSegs DEN-CHI-BUE-ASU
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("DEN", "CHI", "UA", "DEN", "ORD"));
    tvlSegs.push_back(makeAirSeg("CHI", "BUE", "UA", "ORD", "BUE"));
    tvlSegs.push_back(makeAirSeg("BUE", "ASU", "UA"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    // CHI/DFW/BNA-NYC-BUE
    makeGeoLocSeg("CHI",
                  VIAGEOLOCREL_OR,
                  "DFW",
                  VIAGEOLOCREL_OR,
                  "BNA",
                  VIAGEOLOCREL_ANDOR,
                  "NYC",
                  VIAGEOLOCREL_ANDOR,
                  "BUE",
                  psr.viaGeoLocs());
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case T
  // TravelRoute:  DEN-CHI-NYC-BUE-ASU
  // GeoLocs:      CHI/DFW/BNA-NYC-BUE
  // Expected:     PASS
  void testViaGeoLocsT()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("UA");
    TpdPsr& psr = buildPSR("US", "ASU", "UA");

    // create AirSegs DEN-CHI-NYC-BUE-ASU
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("DEN", "CHI", "UA", "DEN", "ORD"));
    tvlSegs.push_back(makeAirSeg("CHI", "NYC", "UA", "ORD", "JFK"));
    tvlSegs.push_back(makeAirSeg("NYC", "BUE", "UA", "JFK", "BUE"));
    tvlSegs.push_back(makeAirSeg("BUE", "ASU", "UA"));
    tvlRoute.mileageTravelRoute() = tvlSegs;
    // CHI/DFW/BNA-NYC-BUE
    makeGeoLocSeg("CHI",
                  VIAGEOLOCREL_OR,
                  "DFW",
                  VIAGEOLOCREL_OR,
                  "BNA",
                  VIAGEOLOCREL_ANDOR,
                  "NYC",
                  VIAGEOLOCREL_ANDOR,
                  "BUE",
                  psr.viaGeoLocs());
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case U
  // TravelRoute:  KRK-RIO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsU()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_RIO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case V
  // TravelRoute:  KRK-SAO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsV()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case W
  // TravelRoute:  KRK-YTO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsW()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case X
  // TravelRoute:  KRK-YTO-SAO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     PASS
  void testViaGeoLocsX()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case Y
  // TravelRoute:  KRK-YTO-SAO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     PASS
  void testViaGeoLocsY()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_YTO_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case Z
  // TravelRoute:  KRK-YTO-RIO-SAO-ASU
  // GeoLocs:      YTO-RIO-SAO
  // Expected:     FAIL
  void testViaGeoLocsZ()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "AA");

    makeAirSegs_KRK_YTO_RIO_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg("YTO",
                  VIAGEOLOCREL_ANDOR,
                  "RIO",
                  VIAGEOLOCREL_ANDOR,
                  "SAO",
                  psr.viaGeoLocs()); // YTO-RIO-SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case AA
  // TravelRoute:  KRK-RIO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     PASS
  void testViaGeoLocsAA()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_RIO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case BB
  // TravelRoute:  KRK-SAO-ASU
  // GeoLocs:      YTO-RIO/SAO
  // Expected:     PASS
  void testViaGeoLocsBB()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("RG");
    TpdPsr& psr = buildPSR("CA", "ASU", "RG");

    makeAirSegs_KRK_SAO_ASU(tvlRoute.mileageTravelRoute());
    makeGeoLocSeg(
        "YTO", VIAGEOLOCREL_ANDOR, "RIO", VIAGEOLOCREL_OR, "SAO", psr.viaGeoLocs()); // YTO-RIO/SAO
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }

  // GeoLocs Test Case CC
  // TravelRoute:  KRK-LO-MIA-LO-ARN-SK-MEX
  // GeoLocs:      MIA-CPH/OSL-STO
  // Expected:     PASS
  void testViaGeoLocsCC()
  {
    MileageRoute mRoute;
    TravelRoute& tvlRoute = makeTravelRoute("LO", GlobalDirection::EH);
    TpdPsr& psr = buildPSR("KRK", "MEX", "RG");

    // create AirSegs KRK-LO-MIA-LO-ARN-SK-MEX
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(makeAirSeg("KRK", "MIA", "LO"));
    tvlSegs.push_back(makeAirSeg("MIA", "STO", "LO", "MIA", "ARN"));
    tvlSegs.push_back(makeAirSeg("STO", "MEX", "SK", "ARN", "MEX"));
    tvlRoute.mileageTravelRoute() = tvlSegs;

    // MIA-CPH/OSL-STO
    makeGeoLocSeg("MIA",
                  VIAGEOLOCREL_ANDOR,
                  "CPH",
                  VIAGEOLOCREL_OR,
                  "OSL",
                  VIAGEOLOCREL_ANDOR,
                  "STO",
                  psr.viaGeoLocs());
    buildMileageRoute(tvlRoute, mRoute, true);

    const PermissibleSpecifiedRouting& exclusion(
        tse::Singleton<PermissibleSpecifiedRouting>::instance());
    bool rc = exclusion.processViaGeoLocs(mRoute, psr);
    CPPUNIT_ASSERT(rc);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(PermissibleSpecifiedRoutingTest);
}
