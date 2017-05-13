#include "Common/RoutingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Routing.h"
#include "Routing/TravelRoute.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <iostream>

namespace tse
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  char getVendorType(const VendorCode& vendor)
  {
    if (vendor == "PUB")
      return 'P';
    else if (vendor == "NPUB")
      return ' ';
    return DataHandleMock::getVendorType(vendor);
  }
  TariffCrossRefInfo* getCRI(int trf1, std::string trf1desc, int trf2, std::string trf2desc)
  {
    TariffCrossRefInfo* ret = _memHandle.create<TariffCrossRefInfo>();
    ret->routingTariff1() = trf1;
    ret->routingTariff2() = trf2;
    ret->routingTariff1Code() = trf1desc;
    ret->routingTariff2Code() = trf2desc;
    return ret;
  }
  const std::vector<TariffCrossRefInfo*>& getTariffXRef(const VendorCode& vendor,
                                                        const CarrierCode& carrier,
                                                        const RecordScope& crossRefType)
  {
    if (vendor == "ABC" && carrier == "XX")
    {
      std::vector<TariffCrossRefInfo*>* ret =
          _memHandle.create<std::vector<TariffCrossRefInfo*> >();
      ret->push_back(getCRI(1, "T001", 2, "T002"));
      ret->push_back(getCRI(3, "T003", 4, "T004"));
      ret->push_back(getCRI(5, "T005", 6, "T006"));
      return *ret;
    }
    return DataHandleMock::getTariffXRef(vendor, carrier, crossRefType);
  }
};

class RoutingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RoutingUtilTest);
  CPPUNIT_TEST(testLocMatchesOrigin);
  CPPUNIT_TEST(testLocMatchesOriginDomestic);
  CPPUNIT_TEST(testLocNotMatchesOriginDomestic);
  CPPUNIT_TEST(testLocMatchesDestination);
  CPPUNIT_TEST(testLocMatchesDestinationDomestic);
  CPPUNIT_TEST(testLocNotMatchesDestinationDomestic);
  CPPUNIT_TEST(testLocMatchesTvlSeg);
  CPPUNIT_TEST(testLocNotMatchesTvlSeg);
  CPPUNIT_TEST(testLocMatchesTvlSegDomestic);
  CPPUNIT_TEST(testLocNotMatchesTvlSegDomestic);
  CPPUNIT_TEST(testGetRoutingTypeMileage);
  CPPUNIT_TEST(testGetRoutingTypeUnknown);
  CPPUNIT_TEST(testGetRoutingTypeMaps);
  CPPUNIT_TEST(testGetRoutingTypeRests12);
  CPPUNIT_TEST(testGetRoutingTypeRests16);
  CPPUNIT_TEST(testGetRoutingTypeRests);
  CPPUNIT_TEST(testGetRoutingTypeRestsMixed);
  CPPUNIT_TEST(testGetRoutingTypeRestsAndMaps);
  CPPUNIT_TEST(testGetVendorNoChange);
  CPPUNIT_TEST(testGetVendorPOFO);
  CPPUNIT_TEST(testGetVendorFMS);
  CPPUNIT_TEST(testGetVendor_MatchC25ResoultVendor);
  CPPUNIT_TEST(testGetVendor_NoC25ResoultVendor_Pub);
  CPPUNIT_TEST(testGetVendor_NoC25ResoultVendor_NotPub);

  CPPUNIT_TEST(testProcessBaseTravelRouteMileage);
  CPPUNIT_TEST(testProcessBaseTravelRouteEmptyRmaps);
  CPPUNIT_TEST(testProcessBaseTravelRoute);
  CPPUNIT_TEST(testUpdateRoutingInfoTariffCode);
  CPPUNIT_TEST(testUpdateRoutingInfoMarkets);
  CPPUNIT_TEST(testUpdateRoutingInfoSingleDestination);
  CPPUNIT_TEST(testUpdateRoutingInfoSingleOrigin);
  CPPUNIT_TEST(testUpdateRoutingInfoDoubleEnded);
  CPPUNIT_TEST(testUpdateRoutingInfoSingleDestinationReversed);
  CPPUNIT_TEST(testUpdateRoutingInfoSingleOriginReversed);
  CPPUNIT_TEST(testUpdateRoutingInfoDoubleEndedReversed);
  CPPUNIT_TEST(testUpdateRoutingInfoRouting);
  CPPUNIT_TEST(testUpdateRoutingInfoNoRouting);
  CPPUNIT_TEST(testGetStaticRoutingMileageHeaderData);

  CPPUNIT_TEST(testFillRoutingTrfDesc_MatchTrf1);
  CPPUNIT_TEST(testFillRoutingTrfDesc_MatchTrf2);
  CPPUNIT_TEST(testFillRoutingTrfDesc_NoFBR);
  CPPUNIT_TEST(testFillRoutingTrfDesc_NoC25ResRouting);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();
    _tariffCrossRef = _memHandle.create<TariffCrossRefInfo>();

    _fare->setFareInfo(_fareInfo);
    _fare->setTariffCrossRefInfo(_tariffCrossRef);
    _paxTypeFare->setFare(_fare);
  }

  void tearDown() { _memHandle.clear(); }

  void prepareTravelRoute(TravelRoute& tvlRoute, const NationCode& nation, bool origin)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->nation() = nation;

    AirSeg* seg = _memHandle.create<AirSeg>();
    tvlRoute.mileageTravelRoute().push_back(seg);

    if (origin)
    {
      seg->origin() = loc;
      tvlRoute.originNation() = nation;
    }
    else
    {
      seg->destination() = loc;
      tvlRoute.destinationNation() = nation;
    }
  }

  void testLocMatchesOrigin()
  {
    Loc loc;
    loc.nation() = "PL";

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, "PL", true);

    CPPUNIT_ASSERT(RoutingUtil::locMatchesOrigin(&loc, tvlRoute));
  }

  void testLocMatchesOriginDomestic()
  {
    Loc loc;
    loc.nation() = UNITED_STATES;

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, CANADA, true);

    CPPUNIT_ASSERT(RoutingUtil::locMatchesOrigin(&loc, tvlRoute));
  }

  void testLocNotMatchesOriginDomestic()
  {
    Loc loc;
    loc.nation() = "PL";

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, CANADA, true);

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesOrigin(&loc, tvlRoute));
  }

  void testLocMatchesDestination()
  {
    Loc loc;
    loc.nation() = "PL";

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, "PL", false);

    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(&loc, tvlRoute));
  }

  void testLocMatchesDestinationDomestic()
  {
    Loc loc;
    loc.nation() = UNITED_STATES;

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, CANADA, false);

    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(&loc, tvlRoute));
  }

  void testLocNotMatchesDestinationDomestic()
  {
    Loc loc;
    loc.nation() = "PL";

    TravelRoute tvlRoute;
    prepareTravelRoute(tvlRoute, CANADA, false);

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesDestination(&loc, tvlRoute));
  }

  void testLocMatchesTvlSeg()
  {
    Loc loc, origin, destination;
    loc.nation() = "PL";
    origin.nation() = "PL";
    destination.nation() = "PL";

    AirSeg seg;
    seg.origin() = &origin;
    seg.destination() = &destination;

    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(&loc, seg));
  }

  void testLocNotMatchesTvlSeg()
  {
    Loc loc, origin, destination;
    loc.nation() = "PL";
    origin.nation() = "PL";
    destination.nation() = "DE";

    AirSeg seg;
    seg.origin() = &origin;
    seg.destination() = &destination;

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesTvlSeg(&loc, seg));

    origin.nation() = "DE";
    destination.nation() = "PL";

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesTvlSeg(&loc, seg));
  }

  void testLocMatchesTvlSegDomestic()
  {
    Loc loc, origin, destination;
    loc.nation() = UNITED_STATES;
    origin.nation() = CANADA;
    destination.nation() = CANADA;

    AirSeg seg;
    seg.origin() = &origin;
    seg.destination() = &destination;

    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(&loc, seg));
  }

  void testLocNotMatchesTvlSegDomestic()
  {
    Loc loc, origin, destination;
    loc.nation() = UNITED_STATES;
    origin.nation() = "PL";
    destination.nation() = CANADA;

    AirSeg seg;
    seg.origin() = &origin;
    seg.destination() = &destination;

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesTvlSeg(&loc, seg));
    origin.nation() = CANADA;
    destination.nation() = "PL";

    CPPUNIT_ASSERT(!RoutingUtil::locMatchesTvlSeg(&loc, seg));
  }

  void testGetRoutingTypeMileage()
  {
    Routing routing;
    routing.routing() = MILEAGE_ROUTING;

    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == MILEAGE_FARE);
  }

  void testGetRoutingTypeUnknown()
  {
    Routing routing;
    routing.routing() = "1234";

    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == UNKNOWN_ROUTING_FARE_TYPE);
  }

  void testGetRoutingTypeMaps()
  {
    Routing routing;
    routing.routing() = "1234";

    RoutingMap* map = new RoutingMap;
    routing.rmaps().push_back(map);

    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == ROUTING_FARE);
  }

  void testGetRoutingTypeRests12()
  {
    Routing routing;
    routing.routing() = "1234";

    RoutingRestriction* rest = new RoutingRestriction;
    rest->restriction() = RTW_ROUTING_RESTRICTION_12;

    routing.rests().push_back(rest);
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == MILEAGE_FARE);
  }

  void testGetRoutingTypeRests16()
  {
    Routing routing;
    routing.routing() = "1234";

    RoutingRestriction* rest = new RoutingRestriction;
    rest->restriction() = MILEAGE_RESTRICTION_16;

    routing.rests().push_back(rest);
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == MILEAGE_FARE);
  }

  void testGetRoutingTypeRests()
  {
    Routing routing;
    routing.routing() = "1234";

    RoutingRestriction* rest = new RoutingRestriction;
    rest->restriction() = ROUTING_RESTRICTION_1;

    routing.rests().push_back(rest);
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == ROUTING_FARE);
  }

  void testGetRoutingTypeRestsMixed()
  {
    Routing routing;
    routing.routing() = "1234";

    RoutingRestriction* rest1 = new RoutingRestriction;
    RoutingRestriction* rest2 = new RoutingRestriction;
    routing.rests().push_back(rest1);
    routing.rests().push_back(rest2);

    rest1->restriction() = ROUTING_RESTRICTION_1;
    rest2->restriction() = ROUTING_RESTRICTION_2;
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == ROUTING_FARE);

    rest1->restriction() = RTW_ROUTING_RESTRICTION_12;
    rest2->restriction() = ROUTING_RESTRICTION_2;
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == MILEAGE_FARE);

    rest1->restriction() = ROUTING_RESTRICTION_1;
    rest2->restriction() = MILEAGE_RESTRICTION_16;
    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == MILEAGE_FARE);
  }

  void testGetRoutingTypeRestsAndMaps()
  {
    Routing routing;
    routing.routing() = "1234";

    routing.rests().push_back(new RoutingRestriction);
    routing.rmaps().push_back(new RoutingMap);

    CPPUNIT_ASSERT(RoutingUtil::getRoutingType(&routing) == ROUTING_FARE);
  }

  void testGetVendorNoChange()
  {
    PaxTypeFare paxTypeFare;
    DataHandle dataHandle;
    VendorCode vendor = "123";

    RoutingUtil::getVendor(&paxTypeFare, dataHandle, vendor);

    CPPUNIT_ASSERT(vendor == "123");
  }

  class VendorPaxTypeFare : public PaxTypeFare
  {
  public:
    VendorPaxTypeFare(const char* vendor) : _vendor(vendor) {}
    virtual const VendorCode& vendor() const { return _vendor; }

  private:
    VendorCode _vendor;
  };

  void testGetVendorPOFO()
  {
    VendorCode vendor;
    VendorPaxTypeFare paxTypeFare("POFO");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    RoutingUtil::getVendor(&paxTypeFare, _dh, vendor);

    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), vendor);
  }
  void testGetVendorFMS()
  {
    VendorCode vendor;
    VendorPaxTypeFare paxTypeFare("FMS");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    RoutingUtil::getVendor(&paxTypeFare, _dh, vendor);

    CPPUNIT_ASSERT_EQUAL(VendorCode(ATPCO_VENDOR_CODE), vendor);
  }
  void testGetVendor_MatchC25ResoultVendor()
  {
    VendorCode vendor;
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "C25V");
    RoutingUtil::getVendor(&paxTypeFare, _dh, vendor);

    CPPUNIT_ASSERT_EQUAL(VendorCode("C25V"), vendor);
  }
  void testGetVendor_NoC25ResoultVendor_Pub()
  {
    MyDataHandle mdh;
    VendorCode vendor = "PUB";
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "");
    RoutingUtil::getVendor(&paxTypeFare, _dh, vendor);

    CPPUNIT_ASSERT_EQUAL(VendorCode("PUB"), vendor);
  }
  void testGetVendor_NoC25ResoultVendor_NotPub()
  {
    MyDataHandle mdh;
    VendorCode vendor = "NPUB";
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "");
    RoutingUtil::getVendor(&paxTypeFare, _dh, vendor);

    CPPUNIT_ASSERT_EQUAL(VendorCode("ATP"), vendor);
  }

  void testProcessBaseTravelRouteMileage()
  {
    RoutingInfo rtgInfo;
    Routing origAddOnRtg, destAddOnRtg;

    rtgInfo.origAddOnRouting() = &origAddOnRtg;
    rtgInfo.destAddOnRouting() = &destAddOnRtg;

    origAddOnRtg.routing() = "1234";
    destAddOnRtg.routing() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(RoutingUtil::processBaseTravelRoute(rtgInfo));

    origAddOnRtg.routing() = MILEAGE_ROUTING;
    destAddOnRtg.routing() = "1234";
    CPPUNIT_ASSERT(RoutingUtil::processBaseTravelRoute(rtgInfo));
  }

  void testProcessBaseTravelRouteEmptyRmaps()
  {
    RoutingInfo rtgInfo;
    Routing origAddOnRtg, destAddOnRtg;

    rtgInfo.origAddOnRouting() = &origAddOnRtg;
    rtgInfo.destAddOnRouting() = &destAddOnRtg;

    origAddOnRtg.routing() = "1234";
    destAddOnRtg.routing() = "5678";
    CPPUNIT_ASSERT(RoutingUtil::processBaseTravelRoute(rtgInfo));

    origAddOnRtg.rmaps().push_back(new RoutingMap);
    CPPUNIT_ASSERT(RoutingUtil::processBaseTravelRoute(rtgInfo));

    origAddOnRtg.rmaps().clear();
    destAddOnRtg.rmaps().push_back(new RoutingMap);
    CPPUNIT_ASSERT(RoutingUtil::processBaseTravelRoute(rtgInfo));
  }

  void testProcessBaseTravelRoute()
  {
    RoutingInfo rtgInfo;
    Routing origAddOnRtg, destAddOnRtg;

    rtgInfo.origAddOnRouting() = &origAddOnRtg;
    rtgInfo.destAddOnRouting() = &destAddOnRtg;

    origAddOnRtg.routing() = "1234";
    destAddOnRtg.routing() = "5678";

    origAddOnRtg.rmaps().push_back(new RoutingMap);
    destAddOnRtg.rmaps().push_back(new RoutingMap);

    CPPUNIT_ASSERT(!RoutingUtil::processBaseTravelRoute(rtgInfo));
  }

  void prepareUpdatedPaxTypeFare(PaxTypeFare& paxTypeFare,
                                 Fare::FareState fareState,
                                 ConstructedFareInfo::ConstructionType type)
  {
    FareMarket* fareMarket = _memHandle.create<FareMarket>();

    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->market1() = "MKT1";
    fareInfo->market2() = "MKT2";

    ConstructedFareInfo* constructedFareInfo = _memHandle.create<ConstructedFareInfo>();
    constructedFareInfo->constructionType() = type;
    constructedFareInfo->gateway1() = "GTW1";
    constructedFareInfo->gateway2() = "GTW2";

    TariffCrossRefInfo* tariff = _memHandle.create<TariffCrossRefInfo>();
    tariff->routingTariff1Code() = "1234";
    tariff->routingTariff1() = 1234;
    tariff->routingTariff2Code() = "5678";
    tariff->routingTariff2() = 5678;

    Fare* fare = _memHandle.create<Fare>();
    fare->initialize(fareState, fareInfo, *fareMarket, tariff, constructedFareInfo);

    PaxType* paxType = _memHandle.create<PaxType>();

    paxTypeFare.initialize(fare, paxType, fareMarket);
  }

  void relaseUpdatedPaxTypeFare(PaxTypeFare& paxTypeFare) { paxTypeFare.initialize(0, 0, 0); }

  void testUpdateRoutingInfoTariffCode()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_International, ConstructedFareInfo::SINGLE_DESTINATION);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.tcrAddonTariff1Code() == "1234");
    CPPUNIT_ASSERT(routingInfo.tcrAddonTariff2Code() == "1234");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoMarkets()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_ConstructedFare, ConstructedFareInfo::SINGLE_DESTINATION);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.market1() == "GTW1");
    CPPUNIT_ASSERT(routingInfo.market2() == "GTW2");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoSingleDestination()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_ConstructedFare, ConstructedFareInfo::SINGLE_DESTINATION);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.destAddOnGateway() == "GTW2");
    CPPUNIT_ASSERT(routingInfo.destAddOnInterior() == "MKT2");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoSingleOrigin()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_ConstructedFare, ConstructedFareInfo::SINGLE_ORIGIN);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.origAddOnGateway() == "GTW1");
    CPPUNIT_ASSERT(routingInfo.origAddOnInterior() == "MKT1");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoDoubleEnded()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_ConstructedFare, ConstructedFareInfo::DOUBLE_ENDED);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.origAddOnGateway() == "GTW1");
    CPPUNIT_ASSERT(routingInfo.origAddOnInterior() == "MKT1");

    CPPUNIT_ASSERT(routingInfo.destAddOnGateway() == "GTW2");
    CPPUNIT_ASSERT(routingInfo.destAddOnInterior() == "MKT2");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoSingleDestinationReversed()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare,
        (Fare::FareState)(Fare::FS_ConstructedFare | Fare::FS_ReversedDirection),
        ConstructedFareInfo::SINGLE_DESTINATION);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.origAddOnGateway() == "GTW2");
    CPPUNIT_ASSERT(routingInfo.origAddOnInterior() == "MKT2");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoSingleOriginReversed()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare,
        (Fare::FareState)(Fare::FS_ConstructedFare | Fare::FS_ReversedDirection),
        ConstructedFareInfo::SINGLE_ORIGIN);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.destAddOnGateway() == "GTW1");
    CPPUNIT_ASSERT(routingInfo.destAddOnInterior() == "MKT1");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoDoubleEndedReversed()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare,
        (Fare::FareState)(Fare::FS_ConstructedFare | Fare::FS_ReversedDirection),
        ConstructedFareInfo::DOUBLE_ENDED);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.origAddOnGateway() == "GTW2");
    CPPUNIT_ASSERT(routingInfo.origAddOnInterior() == "MKT2");

    CPPUNIT_ASSERT(routingInfo.destAddOnGateway() == "GTW1");
    CPPUNIT_ASSERT(routingInfo.destAddOnInterior() == "MKT1");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoRouting()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_International, ConstructedFareInfo::SINGLE_DESTINATION);

    Routing routing;
    routing.routingTariff() = 1234;

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, &routing, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.routingTariff() == 1234);
    CPPUNIT_ASSERT(routingInfo.tcrRoutingTariffCode() == "1234");

    routing.routingTariff() = 0000;
    RoutingInfo routingInfo1;
    RoutingUtil::updateRoutingInfo(paxTypeFare, &routing, routingInfo1, true, false);

    CPPUNIT_ASSERT(routingInfo1.routingTariff() == 5678);
    CPPUNIT_ASSERT(routingInfo1.tcrRoutingTariffCode() == "5678");

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testUpdateRoutingInfoNoRouting()
  {
    PaxTypeFare paxTypeFare;
    prepareUpdatedPaxTypeFare(
        paxTypeFare, Fare::FS_International, ConstructedFareInfo::SINGLE_DESTINATION);

    RoutingInfo routingInfo;
    RoutingUtil::updateRoutingInfo(paxTypeFare, 0, routingInfo, true, false);

    CPPUNIT_ASSERT(routingInfo.tcrRoutingTariffCode() == "1234");
    CPPUNIT_ASSERT(routingInfo.routingTariff() == 1234);

    relaseUpdatedPaxTypeFare(paxTypeFare);
  }

  void testGetStaticRoutingMileageHeaderData()
  {
    _fareInfo->vendor() = Vendor::SITA;
    CarrierCode govCxr = "AA";
    _tariffCrossRef->routingTariff1() = 3;
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, true);

    const Routing* routing =
        RoutingUtil::getStaticRoutingMileageHeaderData(*_trx, govCxr, *_paxTypeFare);

    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->vendor(), routing->vendor());
    CPPUNIT_ASSERT_EQUAL(govCxr, routing->carrier());
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->tcrRoutingTariff1(), routing->routingTariff());
    CPPUNIT_ASSERT_EQUAL(MILEAGE_ROUTING, routing->routing());
  }
  void setFBRItem(PaxTypeFare* ptf, VendorCode vendor, int rtgTarif = 0)
  {
    PaxTypeFareRuleData* p = _memHandle.create<PaxTypeFareRuleData>();
    FareByRuleItemInfo* fbi = _memHandle.create<FareByRuleItemInfo>();
    fbi->resultRoutingVendor() = vendor;
    fbi->resultRoutingTariff() = rtgTarif;
    if (rtgTarif != 0)
      fbi->resultRouting() = "RTG";
    p->ruleItemInfo() = fbi;
    ptf->setRuleData(25, _dh, p, true);
  }
  Routing* createRouting(VendorCode vendor, CarrierCode cxr)
  {
    Routing* ret = _memHandle.create<Routing>();
    ret->vendor() = vendor;
    ret->carrier() = cxr;
    ;
    return ret;
  }
  void testFillRoutingTrfDesc_MatchTrf1()
  {
    MyDataHandle mdh;
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "AAA", 3);
    Routing* routing = createRouting("ABC", "XX");
    RoutingInfo routingInfo;
    routingInfo.routingTariff() = 3;
    RoutingUtil::fillRoutingTrfDesc(paxTypeFare, routing, routingInfo);
    CPPUNIT_ASSERT_EQUAL(TariffCode("T003"), routingInfo.tcrRoutingTariffCode());
  }
  void testFillRoutingTrfDesc_MatchTrf2()
  {
    MyDataHandle mdh;
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "AAA", 6);
    Routing* routing = createRouting("ABC", "XX");
    RoutingInfo routingInfo;
    routingInfo.routingTariff() = 6;
    RoutingUtil::fillRoutingTrfDesc(paxTypeFare, routing, routingInfo);
    CPPUNIT_ASSERT_EQUAL(TariffCode("T006"), routingInfo.tcrRoutingTariffCode());
  }
  void testFillRoutingTrfDesc_NoFBR()
  {
    MyDataHandle mdh;
    VendorPaxTypeFare paxTypeFare("5KAD");
    setFBRItem(&paxTypeFare, "AAA", 3);
    Routing* routing = createRouting("ABC", "XX");
    RoutingInfo routingInfo;
    routingInfo.routingTariff() = 3;
    RoutingUtil::fillRoutingTrfDesc(paxTypeFare, routing, routingInfo);
    CPPUNIT_ASSERT_EQUAL(TariffCode(""), routingInfo.tcrRoutingTariffCode());
  }
  void testFillRoutingTrfDesc_NoC25ResRouting()
  {
    MyDataHandle mdh;
    VendorPaxTypeFare paxTypeFare("5KAD");
    paxTypeFare.status().set(PaxTypeFare::PTF_FareByRule);
    setFBRItem(&paxTypeFare, "AAA", 0);
    Routing* routing = createRouting("ABC", "XX");
    RoutingInfo routingInfo;
    routingInfo.routingTariff() = 3;
    RoutingUtil::fillRoutingTrfDesc(paxTypeFare, routing, routingInfo);
    CPPUNIT_ASSERT_EQUAL(TariffCode(""), routingInfo.tcrRoutingTariffCode());
  }

private:
  PricingTrx* _trx;
  PaxTypeFare* _paxTypeFare;
  Fare* _fare;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tariffCrossRef;
  TestMemHandle _memHandle;
  DataHandle _dh;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RoutingUtilTest);
}
