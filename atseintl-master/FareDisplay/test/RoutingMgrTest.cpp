#include <map>
#include <string>

#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "DataModel/AirSeg.h"
#include "DBAccess/Routing.h"
#include "FareDisplay/RoutingMgr.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DBAccess/Mileage.h"

#include "Common/Config/ConfigMan.h"
#include "test/include/TestConfigInitializer.h"

//--------------------------------------------------------------------------------
// CPPUNIT_ASSERT_EQUAL() UTILS
//--------------------------------------------------------------------------------
namespace
{

inline std::ostream& operator<<(std::ostream& os, const tse::RtgSeq2PaxTypeFareMap& m)
{
  for (tse::RtgSeq2PaxTypeFareMap::const_iterator it = m.begin(), itEnd = m.end(); it != itEnd;
       ++it)
    os << it->first << " " << it->second << "\n";
  return os;
}

//--------------------------------------------------------------------------------

struct StructVCTRRR
{
  tse::VendorCode vendor;
  tse::CarrierCode carrier;
  tse::TariffNumber routingTariff;
  tse::RoutingNumber routingNumber;
  tse::RoutingNumber addOnRouting1;
  tse::RoutingNumber addOnRouting2;
};

inline bool operator==(const StructVCTRRR& vctrrr1, const StructVCTRRR& vctrrr2)
{
  return (vctrrr1.vendor == vctrrr2.vendor && vctrrr1.carrier == vctrrr2.carrier &&
          vctrrr1.routingTariff == vctrrr2.routingTariff &&
          vctrrr1.routingNumber == vctrrr2.routingNumber &&
          vctrrr1.addOnRouting1 == vctrrr2.addOnRouting1 &&
          vctrrr1.addOnRouting2 == vctrrr2.addOnRouting2);
}

inline std::ostream& operator<<(std::ostream& os, const StructVCTRRR& s)
{
  os << "\nvendor        = " << s.vendor;
  os << "\ncarrier       = " << s.carrier;
  os << "\nroutingTariff = " << s.routingTariff;
  os << "\nroutingNumber = " << s.routingNumber;
  os << "\naddOnRouting1 = " << s.addOnRouting1;
  os << "\naddOnRouting2 = " << s.addOnRouting2;
  os << std::endl;
  return os;
}

//--------------------------------------------------------------------------------

struct StructRRR
{
  const tse::Routing* routing;
  const tse::Routing* origAddOnRouting;
  const tse::Routing* destAddOnRouting;
};

inline bool operator==(const StructRRR& ppp1, const StructRRR& ppp2)
{
  return (ppp1.routing == ppp2.routing && ppp1.origAddOnRouting == ppp2.origAddOnRouting &&
          ppp1.destAddOnRouting == ppp2.destAddOnRouting);
}

inline std::ostream& operator<<(std::ostream& os, const StructRRR& ppp)
{
  os << "\nrouting          = " << ppp.routing;
  os << "\norigAddOnRouting = " << ppp.origAddOnRouting;
  os << "\ndestAddOnRouting = " << ppp.destAddOnRouting;
  os << std::endl;
  return os;
}

//--------------------------------------------------------------------------------

typedef tse::RestrictionInfos::iterator RestInfoIt;

class RestInfoCopyStatuses
{
public:
  RestInfoCopyStatuses(const RestInfoIt iterators[4])
  {
    for (int index = 0; index < 4; index++)
    {
      const RestInfoIt& it = iterators[index];
      std::string& status = _statuses[index];

      if (it == _itEnd)
        status = "not copied";
      else if (it->second.processed() && it->second.valid())
        status = "copied";
      else
        status = "BAD";
    }
  }
  RestInfoCopyStatuses(const std::string statuses[4])
  {
    for (int index = 0; index < 4; index++)
      _statuses[index] = statuses[index];
  }

  static void initialize(RestInfoIt itEnd) { _itEnd = itEnd; }
  std::string getStatus(int index) const { return _statuses[index]; }

  inline bool operator==(const RestInfoCopyStatuses& other) const
  {
    for (int index = 0; index < 4; index++)
      if (_statuses[index] != other._statuses[index])
        return false;
    return true;
  }

private:
  static RestInfoIt _itEnd;
  std::string _statuses[4];

}; // class RestInfoCopyStatuses

RestInfoIt RestInfoCopyStatuses::_itEnd;

inline std::ostream& operator<<(std::ostream& os, const RestInfoCopyStatuses& d)
{
  os << std::endl;
  for (int index = 0; index < 4; index++)
    os << index << ": " << d.getStatus(index) << std::endl;
  os << std::endl;
  return os;
}

} // namespace (anonymous)

namespace tse
{

class RoutingMgrTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(RoutingMgrTest);
  CPPUNIT_TEST(testBuildTravelRouteAndMap);
  CPPUNIT_TEST(testBuildUniqueRoutings);
  CPPUNIT_TEST(testBuildRtgKey1);
  CPPUNIT_TEST(testBuildRtgKey2);
  CPPUNIT_TEST(testProcessRoutingWithRestrictions);
  CPPUNIT_TEST(testProcessRoutingForMileageRouting1);
  CPPUNIT_TEST(testProcessRoutingForMileageRouting2);
  CPPUNIT_TEST(testProcessRoutingWithRoutingMaps);
  CPPUNIT_TEST_SUITE_END();

public:
  class OurDataHandleMock : public DataHandleMock
  {
  public:
    OurDataHandleMock() {}
    ~OurDataHandleMock() {}

    const Mileage* getMileage(const LocCode& origin,
                              const LocCode& dest,
                              Indicator mileageType,
                              const GlobalDirection globalDir,
                              const DateTime& date)
    {
      static Mileage m;
      Mileage::dummyData(m);
      return &m;
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

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", "2013-06-16", "PRICING_SVC");

    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _fdTrx->setOptions(_memHandle.create<FareDisplayOptions>());

    _fdResponse = _memHandle.create<FareDisplayResponse>();
    _rtgmgr = _memHandle.insert(new RoutingMgr(*_fdTrx));

    _memHandle.create<OurDataHandleMock>();

    _fareInfo1 = _memHandle.create<FareInfo>();
    _fareInfo2 = _memHandle.create<FareInfo>();
    _fareInfo3 = _memHandle.create<FareInfo>();
    _fareInfo4 = _memHandle.create<FareInfo>();
    _ptFare1 = _memHandle.create<PaxTypeFare>();
    _ptFare2 = _memHandle.create<PaxTypeFare>();
    _ptFare3 = _memHandle.create<PaxTypeFare>();
    _ptFare4 = _memHandle.create<PaxTypeFare>();
    _fare1 = _memHandle.create<Fare>();
    _fare2 = _memHandle.create<Fare>();
    _fare3 = _memHandle.create<Fare>();
    _fare4 = _memHandle.create<Fare>();
    _fareMarket = _memHandle.create<FareMarket>();

    _ptFare1->fareMarket() = _fareMarket;
    _ptFare1->setFare(_fare1);
    _ptFare2->setFare(_fare2);
    _ptFare3->setFare(_fare3);
    _ptFare4->setFare(_fare4);

    _fdTrx->fdResponse() = _fdResponse;

    _tvlRoute = _memHandle.create<TravelRoute>();
    _tvlRoute->travelRoute().push_back(TravelRoute::CityCarrier());

    // airseg
    Loc* loc1 = _memHandle.create<Loc>();
    Loc* loc2 = _memHandle.create<Loc>();
    loc1->loc() = "DFW";
    loc2->loc() = "LON";
    _seg1 = _memHandle.create<AirSeg>();
    _seg1->origin() = loc1;
    _seg1->destination() = loc2;
    _seg1->pnrSegment() = 1;
    _seg1->segmentOrder() = 1;
    _seg1->stopOver() = false;
    _seg1->carrier() = "BA";
    _seg1->departureDT() = DateTime::localTime();
    _seg1->arrivalDT() = DateTime::localTime();
    _seg1->origAirport() = "DFW";
    _seg1->destAirport() = "LON";
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  //--------------------------------------------------------------------------------
  // UNIT TESTS
  //--------------------------------------------------------------------------------

  void testBuildTravelRouteAndMap()
  {
    _tcrInfo.routingTariff1() = 27;

    _fareMarket->travelSeg().push_back(_seg1);
    _fareMarket->governingCarrier() = "BA";

    _fareInfo1->vendor() = "ATP";
    _fareInfo1->routingNumber() = "1100";

    _fare1->setTariffCrossRefInfo(&_tcrInfo);
    _fare1->setFareInfo(_fareInfo1);

    _fdTrx->allPaxTypeFare().push_back(_ptFare1);
    _fareMarket->allPaxTypeFare().push_back(_ptFare1);
    _ptFare1->setIsShoppingFare();

    FareDisplayRequest request;
    _fdTrx->setRequest(&request);
    _fdTrx->fareMarket().push_back(_fareMarket);

    CPPUNIT_ASSERT(_rtgmgr->buildTravelRouteAndMap());
  }

  void testBuildUniqueRoutings()
  {
    _fareInfo1->routingNumber() = "1100";
    _fareInfo2->routingNumber() = "2";
    _fareInfo3->routingNumber() = "3";
    _fareInfo4->routingNumber() = "4";

    _fare1->setFareInfo(_fareInfo1);
    _fare2->setFareInfo(_fareInfo2);
    _fare3->setFareInfo(_fareInfo3);
    _fare4->setFareInfo(_fareInfo4);

    _fdTrx->allPaxTypeFare().push_back(_ptFare1);
    _fdTrx->allPaxTypeFare().push_back(_ptFare2);
    _fdTrx->allPaxTypeFare().push_back(_ptFare3);
    _fdTrx->allPaxTypeFare().push_back(_ptFare4);

    _fareMarket->allPaxTypeFare().push_back(_ptFare1);
    _fareMarket->allPaxTypeFare().push_back(_ptFare2);
    _fareMarket->allPaxTypeFare().push_back(_ptFare3);
    _fareMarket->allPaxTypeFare().push_back(_ptFare4);

    _ptFare1->setIsShoppingFare();
    _ptFare2->setIsShoppingFare();
    _ptFare3->setIsShoppingFare();
    _ptFare4->setIsShoppingFare();

    FareDisplayRequest request;
    _fdTrx->setRequest(&request);

    _fdResponse->uniqueRoutings().insert(
        std::map<std::string, PaxTypeFare*>::value_type(_ptFare1->routingNumber(), _ptFare1));
    _fdResponse->uniqueRoutings().insert(
        std::map<std::string, PaxTypeFare*>::value_type(_ptFare2->routingNumber(), _ptFare2));
    _fdResponse->uniqueRoutings().insert(
        std::map<std::string, PaxTypeFare*>::value_type(_ptFare3->routingNumber(), _ptFare3));
    _fdResponse->uniqueRoutings().insert(
        std::map<std::string, PaxTypeFare*>::value_type(_ptFare4->routingNumber(), _ptFare4));
    RtgSeq2PaxTypeFareMap orgMap(_fdResponse->uniqueRoutings());

    _rtgmgr->buildUniqueRoutings();

    CPPUNIT_ASSERT_EQUAL(orgMap, _fdResponse->uniqueRoutings());
  }

  void commonInitForTestBuildRtgKey()
  {
    _fareMarket->travelSeg().push_back(_seg1);

    _fareInfo1->vendor() = "ATP";
    _fare1->initialize(Fare::FS_Domestic, _fareInfo1, *_fareMarket, &_tcrInfo);
    _fareMarket->governingCarrier() = "AA";
    _tcrInfo.routingTariff1() = 200;
    _fareInfo1->routingNumber() = "1100";

    _routing.vendor() = "SITA";
    _routing.carrier() = "BA";
    _routing.routing() = "1122";
    _routing.routingTariff() = 100;

    _origAddOnRouting.routing() = "2233";
    _destAddOnRouting.routing() = "3344";
  }

  void testBuildRtgKey1()
  {
    commonInitForTestBuildRtgKey();
    _rtgmgr->buildRtgKey(*_ptFare1, *_fareMarket, 0, 0, 0, _rtgKey);

    StructVCTRRR vctr1 = { _ptFare1->vendor(),            _fareMarket->governingCarrier(),
                           _ptFare1->tcrRoutingTariff1(), _ptFare1->routingNumber(),
                           RoutingNumber(), // \_ dummy values
                           RoutingNumber() // /
    };
    StructVCTRRR vctr2 = { _rtgKey.vendor(),        _rtgKey.carrier(), _rtgKey.routingTariff(),
                           _rtgKey.routingNumber(), RoutingNumber(), // \_ dummy values
                           RoutingNumber() // /
    };
    CPPUNIT_ASSERT_EQUAL(vctr1, vctr2);
  }

  void testBuildRtgKey2()
  {
    commonInitForTestBuildRtgKey();
    _rtgmgr->buildRtgKey(
        *_ptFare1, *_fareMarket, &_routing, &_origAddOnRouting, &_destAddOnRouting, _rtgKey);

    StructVCTRRR vctrrr1 = { _routing.vendor(),           _routing.carrier(),
                             _routing.routingTariff(),    _routing.routing(),
                             _origAddOnRouting.routing(), _destAddOnRouting.routing() };
    StructVCTRRR vctrrr2 = { _rtgKey.vendor(),        _rtgKey.carrier(),
                             _rtgKey.routingTariff(), _rtgKey.routingNumber(),
                             _rtgKey.addOnRouting1(), _rtgKey.addOnRouting2() };
    CPPUNIT_ASSERT_EQUAL(vctrrr1, vctrrr2);
  }

  void testProcessRoutingWithRestrictions()
  {
    RoutingRestriction* pr1 = new RoutingRestriction; // \.
    RoutingRestriction* pr2 = new RoutingRestriction; //  \_ deleted inside _routing.~Routing()
    RoutingRestriction* pr3 = new RoutingRestriction; //  /
    RoutingRestriction* pr4 = new RoutingRestriction; // /

    ConstructedFareInfo constructedFareInfo;
    _fare1->initialize(
        Fare::FS_ConstructedFare, _fareInfo1, *_fareMarket, &_tcrInfo, &constructedFareInfo);
    _fare1->status().set(Fare::FS_ConstructedFare);

    _rtgKey.routingNumber() = "1100";

    pr1->restriction() = ROUTING_RESTRICTION_3; // should not be copied
    pr2->restriction() = ROUTING_RESTRICTION_4;
    pr3->restriction() = ROUTING_RESTRICTION_17; // should not be copied
    pr4->restriction() = ROUTING_RESTRICTION_5;

    _routing.rests().push_back(pr1);
    _routing.rests().push_back(pr2);
    _origAddOnRouting.rests().push_back(pr3);
    _destAddOnRouting.rests().push_back(pr4);

    CPPUNIT_ASSERT(_rtgmgr->processRouting(_rtgKey.routingNumber(),
                                           *_tvlRoute,
                                           _ptFare1,
                                           &_routing,
                                           &_origAddOnRouting,
                                           &_destAddOnRouting));

    _rtgInfo = _fdResponse->uniqueRoutingMap()[_rtgKey.routingNumber()];

    StructRRR routingResults = { _rtgInfo->routing(), _rtgInfo->origAddOnRouting(),
                                 _rtgInfo->destAddOnRouting() };
    StructRRR routingExpected = { &_routing, &_origAddOnRouting, &_destAddOnRouting };
    CPPUNIT_ASSERT_EQUAL(routingExpected, routingResults);

    // check if correct restrictions have been copied
    RestrictionInfos* infos = _rtgInfo->restrictions();
    RestInfoCopyStatuses::initialize(infos->end());
    RestInfoIt resultIterators[] = { infos->find(pr1), infos->find(pr2), infos->find(pr3),
                                     infos->find(pr4) };
    std::string expectedStatuses[] = { "not copied", "copied", "not copied", "copied" };
    CPPUNIT_ASSERT_EQUAL(RestInfoCopyStatuses(expectedStatuses),
                         RestInfoCopyStatuses(resultIterators));
  }

  void testProcessRoutingForMileageRouting1()
  {
    _fare1->initialize(Fare::FS_Domestic, _fareInfo1, *_fareMarket, &_tcrInfo);

    _rtgKey.routingNumber() = "1100";
    CPPUNIT_ASSERT(!_rtgmgr->processRouting(_rtgKey.routingNumber(), *_tvlRoute, _ptFare1, 0, 0, 0));
  }

  void testProcessRoutingForMileageRouting2()
  {
    _fare1->initialize(Fare::FS_Domestic, _fareInfo1, *_fareMarket, &_tcrInfo);

    _rtgKey.routingNumber() = MILEAGE_ROUTING;
    _fareInfo1->routingNumber() = MILEAGE_ROUTING;

    _fdTrx->travelSeg().push_back(_seg1);
    CPPUNIT_ASSERT(_rtgmgr->processRouting(_rtgKey.routingNumber(), *_tvlRoute, _ptFare1, 0, 0, 0));

    _rtgInfo = _fdResponse->uniqueRoutingMap()[MILEAGE_ROUTING];
    CPPUNIT_ASSERT(_rtgInfo->mileageInfo() != 0);
  }

  void testProcessRoutingWithRoutingMaps()
  {
    ConstructedFareInfo constructedFareInfo;
    RoutingMap* rmap = new RoutingMap; // deleted inside _routing.~Routing()

    _origAddOnRouting.rmaps().push_back(rmap);

    _fare1->initialize(Fare::FS_ReversedDirection,
                       &(constructedFareInfo.fareInfo()),
                       *_fareMarket,
                       &_tcrInfo,
                       &constructedFareInfo);
    _fare1->status().set(Fare::FS_ConstructedFare);
    _rtgKey.routingNumber() = "1100";

    _fdTrx->travelSeg().push_back(_seg1);

    _fdTrx->travelSeg().front()->offMultiCity() = _seg1->origin()->loc();
    _fdTrx->travelSeg().front()->boardMultiCity() = _seg1->destination()->loc();

    FareDisplayRequest request;
    request.ticketingDT() = DateTime(2014, 6, 16); // flag active
    _fdTrx->setRequest(&request);

    CPPUNIT_ASSERT(_rtgmgr->processRouting(_rtgKey.routingNumber(),
                                           *_tvlRoute,
                                           _ptFare1,
                                           &_routing,
                                           &_origAddOnRouting,
                                           &_destAddOnRouting));

    _rtgInfo = _fdResponse->uniqueRoutingMap()[_rtgKey.routingNumber()];

    // check if addons are swapped
    StructRRR rrrExpected = { 0, &_destAddOnRouting, &_origAddOnRouting };
    StructRRR rrrResult = { 0, _rtgInfo->origAddOnRouting(), _rtgInfo->destAddOnRouting() };
    CPPUNIT_ASSERT_EQUAL(rrrExpected, rrrResult);
  }

protected:
  TestMemHandle _memHandle;

  FareDisplayTrx* _fdTrx;
  FareDisplayResponse* _fdResponse;
  RoutingMgr* _rtgmgr;
  FareInfo* _fareInfo1;
  FareInfo* _fareInfo2;
  FareInfo* _fareInfo3;
  FareInfo* _fareInfo4;
  PaxTypeFare* _ptFare1;
  PaxTypeFare* _ptFare2;
  PaxTypeFare* _ptFare3;
  PaxTypeFare* _ptFare4;
  Fare* _fare1;
  Fare* _fare2;
  Fare* _fare3;
  Fare* _fare4;
  FareMarket* _fareMarket;
  AirSeg* _seg1;
  Routing _routing;
  Routing _origAddOnRouting;
  Routing _destAddOnRouting;
  RtgKey _rtgKey;
  TariffCrossRefInfo _tcrInfo;
  RoutingInfo* _rtgInfo;
  TravelRoute* _tvlRoute;
}; // class RoutingMgrTest

CPPUNIT_TEST_SUITE_REGISTRATION(RoutingMgrTest);

} // namespace tse
