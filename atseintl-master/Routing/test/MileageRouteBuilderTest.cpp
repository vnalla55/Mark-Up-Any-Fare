#include <vector>

#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/DateTime.h"
#include "Common/FareMarketUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/TravelRoute.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using boost::assign::operator+=;

class MileageRouteBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageRouteBuilderTest);
  CPPUNIT_TEST(testbuildMileageRouteforMileageService);
  CPPUNIT_TEST(testbuildMileageRouteforMileageValidator);
  CPPUNIT_TEST(test_PL_11726);
  CPPUNIT_TEST(testsetOccurrences);
  CPPUNIT_TEST(testIsDirectFromRouteBeginReturnTrueWhenSetInRouteBuilder);
  CPPUNIT_TEST(testIsDirectFromRouteBeginReturnFalseWhenNotSetInRouteBuilder);
  CPPUNIT_TEST(testbuildWNMileageRoute);

  CPPUNIT_TEST(testSetCRSMultiHost_RequestNullCrsEmpty);
  CPPUNIT_TEST(testSetCRSMultiHost_RequestNullMultiHostEmpy);
  CPPUNIT_TEST(testSetCRSMultiHost_TicketingAgentNullCrsEmpty);
  CPPUNIT_TEST(testSetCRSMultiHost_TicketingAgentNullMultiHostEmpty);
  CPPUNIT_TEST(testSetCRSMultiHost_VendorCrsCodeNotEmpty);
  CPPUNIT_TEST(testSetCRSMultiHost_tvlAgencyPCCSize4);
  CPPUNIT_TEST(testSetCRSMultiHost_MultiHost);
  CPPUNIT_TEST(testSetCRSMultiHost_AxessMultihostId);
  CPPUNIT_TEST(testSetCRSMultiHost_AbacusMultihostId);
  CPPUNIT_TEST(testSetCRSMultiHost_InfiniMultihostId);
  CPPUNIT_TEST(testSetCRSMultiHost_SabreMultihostId);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();

    _pricingTrx = _memHandle.create<PricingTrx>();
    _pricingTrx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _agent = _memHandle.create<Agent>();
    _builder = _memHandle.create<MileageRouteBuilder>();
    _mRoute = _memHandle.create<MileageRoute>();
  }

  void tearDown() { _memHandle.clear(); }
  void setItinerary(MileageRoute& mileageRoute)
  {
    MileageRouteItem* item1 = _memHandle.create<MileageRouteItem>();
    Loc* city1 = _memHandle.create<Loc>();
    city1->loc() = "TYO";
    Loc* city2 = _memHandle.create<Loc>();
    city2->loc() = "JKT";
    item1->city1() = city1;
    item1->city2() = city2;

    MileageRouteItem* item2 = _memHandle.create<MileageRouteItem>();
    city1 = _memHandle.create<Loc>();
    city1->loc() = "JKT";
    city2 = _memHandle.create<Loc>();
    city2->loc() = "SUB";
    item2->city1() = city1;
    item2->city2() = city2;

    MileageRouteItem* item3 = _memHandle.create<MileageRouteItem>();
    city1 = _memHandle.create<Loc>();
    city1->loc() = "SUB";
    city2 = _memHandle.create<Loc>();
    city2->loc() = "JKT";
    item3->city1() = city1;
    item3->city2() = city2;

    MileageRouteItem* item4 = _memHandle.create<MileageRouteItem>();
    city1 = _memHandle.create<Loc>();
    city1->loc() = "JKT";
    city2 = _memHandle.create<Loc>();
    city2->loc() = "DPS";
    item4->city1() = city1;
    item4->city2() = city2;

    mileageRoute.mileageRouteItems() += *item1, *item2, *item3, *item4;
  }

  AirSeg* mkAirSeg(DataHandle& dh, const Loc* orig, const Loc* dest, const CarrierCode& carrier)
  {
    AirSeg* seg = dh.create<AirSeg>();
    DateTime tvlDate = seg->departureDT() = DateTime::localTime();
    std::vector<TravelSeg*> tvlSeg;
    seg->origin() = orig;
    seg->destination() = dest;
    seg->carrier() = carrier;

    tvlSeg.push_back(seg);
    GeoTravelType geoTvlType = TravelSegUtil::getGeoTravelType(tvlSeg, _pricingTrx->dataHandle());
    // std::cout << "GeoTravelType: " << geoTvlType << std::endl;

    seg->boardMultiCity() = FareMarketUtil::getMultiCity(carrier, orig->loc(), geoTvlType, tvlDate);
    seg->offMultiCity() = FareMarketUtil::getMultiCity(carrier, dest->loc(), geoTvlType, tvlDate);

    return seg;
  }

  SurfaceSeg* mkSurfaceSeg(DataHandle& dh, const Loc* orig, const Loc* dest)
  {
    SurfaceSeg* seg = dh.create<SurfaceSeg>();

    seg->departureDT() = DateTime::localTime();
    seg->origin() = orig;
    seg->destination() = dest;
    seg->boardMultiCity() = orig->city();
    seg->offMultiCity() = dest->city();

    return seg;
  }

  CarrierCode getGovCxr(const std::vector<TravelSeg*>& tvlSeg)
  {
    GoverningCarrier govCxr;
    std::set<CarrierCode> govCxrSet;
    govCxr.getGoverningCarrier(tvlSeg, govCxrSet);
    if (!govCxrSet.empty())
      return *govCxrSet.begin();
    return "";
  }

  GlobalDirection getGD(DataHandle& dataHandle, DateTime& tvlDate, std::vector<TravelSeg*>& tvlSeg)
  {
    GlobalDirection gd;
    GlobalDirectionFinderV2Adapter::getGlobalDirection(_pricingTrx, tvlDate, tvlSeg, gd);

    std::string gdStr;
    bool rc = globalDirectionToStr(gdStr, gd);
    CPPUNIT_ASSERT(rc);

    TravelRoute tvlRoute;
    tvlRoute.mileageTravelRoute() = tvlSeg;
    tvlRoute.globalDir() = gd;
    tvlRoute.govCxr() = getGovCxr(tvlSeg);
    tvlRoute.travelDate() = tvlDate;

    DateTime ticketingDT = DateTime::localTime();

    MileageRouteBuilder mBuilder;
    MileageRoute mRoute;
    rc = mBuilder.buildMileageRoute(*_pricingTrx, tvlRoute, mRoute, dataHandle, ticketingDT);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == tvlSeg.size());

    /*MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
      MileageRouteItems::iterator end(mRoute.mileageRouteItems().end());
      for(; itr!= end; ++itr)
      {
      std::string MPMglobalDirectionStr;
      std::string TPMglobalDirectionStr;
      globalDirectionToStr(TPMglobalDirectionStr, itr->tpmGlobalDirection());
      globalDirectionToStr(MPMglobalDirectionStr, itr->mpmGlobalDirection());

      std::cout << itr->city1()->loc() << itr->city2()->loc() << "\n"
      << " MPM GlobalDir "   << MPMglobalDirectionStr << "\n"
      << " TPM GlobalDir "   << TPMglobalDirectionStr << "\n"
      << " Segment Carrier " << itr->segmentCarrier() << "\n"
      << "------------------------------"
      <<std::endl;
      }

      std::cout << "GlobalDirection: " << gdStr << "\n\n" << std::endl;*/

    return gd;
  }

  typedef std::vector<std::pair<MileageRouteItem*, const MileageTrx::MileageItem*> >
  MileageRouteItemVec;

  void addRouteItems(MileageRouteItemVec& routeItems, int num)
  {
    for (int i = 0; i < num; i++)
      routeItems.push_back(MileageRouteItemVec::value_type(
          _memHandle.insert(new MileageRouteItem), _memHandle.insert(new MileageTrx::MileageItem)));
  }

  void setRouteItemsOrigAndDestCityOrAirport(MileageRouteItemVec& routeItems,
                                             const char** orig,
                                             const char** dest,
                                             int num)
  {
    for (int i = 0; i < num; i++)
    {
      Loc* loc = _memHandle.insert(new Loc);
      loc->loc() = LocCode(orig[i]);
      routeItems[i].first->origCityOrAirport() = loc;
      routeItems[i].first->multiTransportOrigin() = loc;
      loc = _memHandle.insert(new Loc);
      loc->loc() = LocCode(dest[i]);
      routeItems[i].first->destCityOrAirport() = loc;
      routeItems[i].first->multiTransportDestination() = loc;
    }
  }

  void setRouteItemsIsSurface(MileageRouteItemVec& routeItems, bool values[], int num)
  {
    for (int i = 0; i < num; i++)
      routeItems[i].first->isSurface() = values[i];
  }

  //////////////////////////////////////////////////////////////////////

  void testbuildMileageRouteforMileageService()
  {
    //  origin in Japan, destination in Europe with a non-stop segment between Japan and China, and
    // between China and Europe
    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("TYO", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("BJS", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("SHA", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("FRA", DateTime::localTime());
    const Loc* loc5 = dataHandle.getLoc("AMS", DateTime::localTime());
    MileageTrx trx;
    MileageTrx::MileageItem item1, item2, item3, item4, item5;
    item1.cityLoc = loc1;
    item2.cityLoc = loc2;
    item3.cityLoc = loc3;
    item4.cityLoc = loc4;
    item5.cityLoc = loc5;

    trx.items().push_back(&item1);
    trx.items().push_back(&item2);
    trx.items().push_back(&item3);
    trx.items().push_back(&item4);
    trx.items().push_back(&item5);
    trx.inputDT() = DateTime::localTime();
    MileageRoute mRoute;
    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(trx, mRoute, 0);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == 4);
    /*MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
      MileageRouteItems::iterator end(mRoute.mileageRouteItems().end());
      for(; itr!= end; ++itr)
      {
      std::cout<< " " <<std::endl;
      std::cout<< itr->city1()->loc() <<std::endl;
      std::cout<< itr->city2()->loc() <<std::endl;
      std::string MPMglobalDirectionStr;
      std::string TPMglobalDirectionStr;
      globalDirectionToStr(TPMglobalDirectionStr, itr->tpmGlobalDirection());
      globalDirectionToStr(MPMglobalDirectionStr, itr->mpmGlobalDirection());
      std::cout<< " MPM GlobalDir " << MPMglobalDirectionStr << std::endl;
      std::cout<< " TPM GlobalDir " << TPMglobalDirectionStr << std::endl;
      std::cout<<"------------------------------"<<std::endl;
      }*/
  }

  void testbuildMileageRouteforMileageValidator()
  {
    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("JFK", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("DFW", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("MAN", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("FRA", DateTime::localTime());
    MileageRoute mRoute;
    AirSeg as1, as2;
    SurfaceSeg ss;
    TravelRoute tvlRoute;
    // create AirSegs

    as1.origin() = const_cast<Loc*>(loc1);
    as1.destination() = const_cast<Loc*>(loc2);
    as1.boardMultiCity() = "NYC";
    as1.offMultiCity() = "DFW";
    as1.carrier() = "AA";

    as2.origin() = const_cast<Loc*>(loc2);
    as2.destination() = const_cast<Loc*>(loc3);
    as2.boardMultiCity() = "DFW";
    as2.offMultiCity() = "LON";
    as2.carrier() = "BA";

    ss.origin() = const_cast<Loc*>(loc3);
    ss.destination() = const_cast<Loc*>(loc4);
    ss.boardMultiCity() = "LON";
    ss.offMultiCity() = "FRA";
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&as1);
    tvlSegs.push_back(&as2);
    tvlSegs.push_back(&ss);
    tvlRoute.mileageTravelRoute() = tvlSegs;

    tvlRoute.globalDir() = GlobalDirection::AT;
    tvlRoute.govCxr() = "BA";
    tvlRoute.travelDate() = DateTime::localTime();

    DateTime ticketingDT = DateTime::localTime();

    PricingRequest pricingRequest;
    pricingRequest.ticketingDT() = DateTime::localTime();
    _pricingTrx->setRequest(&pricingRequest);

    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(*_pricingTrx, tvlRoute, mRoute, dataHandle, ticketingDT);
    // bool rc = mBuilder.newBuildMileageRoute(*_pricingTrx, tvlRoute, mRoute,
    // dataHandle,ticketingDT); //after Activation of TPD project
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == 3);
    /*MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
      MileageRouteItems::iterator end(mRoute.mileageRouteItems().end());
      for(; itr!= end; ++itr)
      {
      std::cout<< " " <<std::endl;
      std::cout<< itr->city1()->loc() <<std::endl;
      std::cout<< itr->city2()->loc() <<std::endl;
      std::string MPMglobalDirectionStr;
      std::string TPMglobalDirectionStr;
      globalDirectionToStr(TPMglobalDirectionStr, itr->tpmGlobalDirection());
      globalDirectionToStr(MPMglobalDirectionStr, itr->mpmGlobalDirection());
      std::cout<< " MPM GlobalDir "   << MPMglobalDirectionStr << std::endl;
      std::cout<< " TPM GlobalDir "   << TPMglobalDirectionStr << std::endl;
      std::cout<< " Segment Carrier " << itr->segmentCarrier() << std::endl;
      std::cout<<"------------------------------"<<std::endl;
      }*/
  }

  void test_PL_11726()
  {
    DataHandle dataHandle;

    DateTime tvlDate = DateTime::localTime();
    const Loc* loc1 = dataHandle.getLoc("RUH", tvlDate);
    const Loc* loc2 = dataHandle.getLoc("CAI", tvlDate);
    const Loc* loc3 = dataHandle.getLoc("MIL", tvlDate);

    const Loc* loc4 = dataHandle.getLoc("MIA", tvlDate);

    AirSeg* s1 = mkAirSeg(dataHandle, loc1, loc2, "MS");
    AirSeg* s2 = mkAirSeg(dataHandle, loc2, loc3, "AZ");
    SurfaceSeg* s3 = mkSurfaceSeg(dataHandle, loc3, loc4);

    PricingRequest pricingRequest;
    pricingRequest.ticketingDT() = DateTime::localTime();
    _pricingTrx->setRequest(&pricingRequest);

    std::vector<TravelSeg*> tvlSeg;
    tvlSeg.push_back(s1);
    getGD(dataHandle, tvlDate, tvlSeg);

    tvlSeg.push_back(s2);
    getGD(dataHandle, tvlDate, tvlSeg);

    tvlSeg.push_back(s3);
    GlobalDirection gd = getGD(dataHandle, tvlDate, tvlSeg);
    CPPUNIT_ASSERT(gd == GlobalDirection::AT);
  }

  void testIsDirectFromRouteBeginReturnTrueWhenSetInRouteBuilder()
  {
    bool isDirectFromRouteBegin = true;
    MileageRouteItem item;
    MileageRouteBuilder mBuilder;
    mBuilder.setDirectService(&item, isDirectFromRouteBegin);

    CPPUNIT_ASSERT(item.isDirectFromRouteBegin());
  }

  void testIsDirectFromRouteBeginReturnFalseWhenNotSetInRouteBuilder()
  {
    bool isDirectFromRouteBegin = false;
    MileageRouteItem item;
    MileageRouteBuilder mBuilder;
    mBuilder.setDirectService(&item, isDirectFromRouteBegin);
    CPPUNIT_ASSERT(!item.isDirectFromRouteBegin());
  }

  void testsetOccurrences()
  {
    MileageRoute mileageRoute;
    setItinerary(mileageRoute);
    MileageRouteBuilder mBuilder;
    mBuilder.setOccurrences(mileageRoute);
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[0].isFirstOccurrenceFromRouteBegin());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[0].isLastOccurrenceToRouteEnd());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[2].isFirstOccurrenceFromRouteBegin());
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[2].isLastOccurrenceToRouteEnd());
  }
  void testbuildWNMileageRoute()
  {
    MileageRoute mileageRoute;
    setItinerary(mileageRoute);
    MileageRouteBuilder mBuilder;
    mBuilder.buildWNMileageRoute(mileageRoute);
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[0].isFirstOccurrenceFromRouteBegin());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[0].isLastOccurrenceToRouteEnd());
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[0].isDirectFromRouteBegin());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[0].isDirectToRouteEnd());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[2].isFirstOccurrenceFromRouteBegin());
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[2].isLastOccurrenceToRouteEnd());
    CPPUNIT_ASSERT(!mileageRoute.mileageRouteItems()[2].isDirectFromRouteBegin());
    CPPUNIT_ASSERT(mileageRoute.mileageRouteItems()[2].isDirectToRouteEnd());
  }

  void testSetCRSMultiHost_RequestNullCrsEmpty()
  {
    _builder->setCRSMultiHost(*_mRoute, 0);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _mRoute->crs());
  }

  void testSetCRSMultiHost_RequestNullMultiHostEmpy()
  {
    _builder->setCRSMultiHost(*_mRoute, 0);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _mRoute->multiHost());
  }

  void testSetCRSMultiHost_TicketingAgentNullCrsEmpty()
  {
    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _mRoute->crs());
  }

  void testSetCRSMultiHost_TicketingAgentNullMultiHostEmpty()
  {
    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(""), _mRoute->multiHost());
  }

  void testSetCRSMultiHost_VendorCrsCodeNotEmpty()
  {
    _agent->vendorCrsCode() = AXESS_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(AXESS_USER), _mRoute->crs());
  }

  void testSetCRSMultiHost_tvlAgencyPCCSize4()
  {
    _agent->tvlAgencyPCC() = "ABCD";
    _agent->cxrCode() = ABACUS_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(ABACUS_USER), _mRoute->crs());
  }

  void testSetCRSMultiHost_MultiHost()
  {
    _agent->tvlAgencyPCC() = "ABC";
    _agent->cxrCode() = ABACUS_MULTIHOST_ID;
    _agent->hostCarrier() = "LH";
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string("LH"), _mRoute->multiHost());
  }

  void testSetCRSMultiHost_AxessMultihostId()
  {
    _agent->vendorCrsCode() = AXESS_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(AXESS_USER), _mRoute->crs());
  }

  void testSetCRSMultiHost_AbacusMultihostId()
  {
    _agent->vendorCrsCode() = ABACUS_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(ABACUS_USER), _mRoute->crs());
  }

  void testSetCRSMultiHost_InfiniMultihostId()
  {
    _agent->vendorCrsCode() = INFINI_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(INFINI_USER), _mRoute->crs());
  }

  void testSetCRSMultiHost_SabreMultihostId()
  {
    _agent->vendorCrsCode() = SABRE_MULTIHOST_ID;
    _request->ticketingAgent() = _agent;

    _builder->setCRSMultiHost(*_mRoute, _request);
    CPPUNIT_ASSERT_EQUAL(std::string(SABRE_USER), _mRoute->crs());
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    MultiTransport* getMC(LocCode city, LocCode loc)
    {
      MultiTransport* ret = _memHandle.create<MultiTransport>();
      ret->multitranscity() = city;
      ret->multitransLoc() = loc;
      return ret;
    }

  public:
    const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                            const CarrierCode& carrierCode,
                                            GeoTravelType tvlType,
                                            const DateTime& tvlDate)
    {
      if (locCode == "CAI")
        return "CAI";
      else if (locCode == "FRA")
        return "FRA";
      else if (locCode == "JFK")
        return "NYC";
      else if (locCode == "MAN")
        return "MAN";
      else if (locCode == "MIL")
        return "MIL";
      else if (locCode == "RUH")
        return "RUH";

      return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
    }

    const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                              const CarrierCode& carrierCode,
                                                              GeoTravelType tvlType,
                                                              const DateTime& tvlDate)
    {
      std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
      if (locCode == "AMS")
        return ret;
      else if (locCode == "BJS")
        return ret;
      else if (locCode == "FRA")
        return ret;
      else if (locCode == "SHA")
      {
        ret += getMC("SHA", "SHA");
        return ret;
      }
      else if (locCode == "TYO")
        return ret;

      //////////////////////////////
      //////////////////////////////////
      return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
    }
  };

  PricingTrx* _pricingTrx;
  PricingRequest* _request;
  Agent* _agent;
  TestMemHandle _memHandle;
  MileageRouteBuilder* _builder;
  MileageRoute* _mRoute;
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageRouteBuilderTest);

} // tse
