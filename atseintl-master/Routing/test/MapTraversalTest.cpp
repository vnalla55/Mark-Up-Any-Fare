//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TrxUtil.h"
#include "Common/DateTime.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/ATPResNationZones.h"
#include "Routing/MapTraversal.h"
#include "Routing/SpecifiedRouting.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace
{
class MyDataHandle : public tse::DataHandleMock
{
public:
  MyDataHandle(tse::TestMemHandle& memHandle) : _memHandle(memHandle) {}

  const std::vector<tse::ATPResNationZones*>&
  getATPResNationZones(const tse::NationCode& key)
  {
    if(key == "NO")
    {
      std::vector<tse::ATPResNationZones*>* resZones = _memHandle.create<std::vector<tse::ATPResNationZones*> >();
      tse::ATPResNationZones* zone = _memHandle.create<tse::ATPResNationZones>();
      zone->zones().push_back("0000210");
      zone->zones().push_back("0000212");

      resZones->push_back(zone);

      return *resZones;
    }
    return DataHandleMock::getATPResNationZones(key);
  }
private:
  tse::TestMemHandle& _memHandle;
};
}

namespace tse
{
class MapTraversalTest : public CppUnit::TestFixture
{
  class MockRouting : public Routing
  {
    friend class MapTraversalTest;
  };
  class MockSpecifiedRouting : public SpecifiedRouting
  {
    friend class MapTraversalTest;
  };

  CPPUNIT_TEST_SUITE(MapTraversalTest);

  CPPUNIT_TEST(testExecuteMaxResSize);
  CPPUNIT_TEST(testExecutePassAll);
  CPPUNIT_TEST(testExecutePassAllNextMap);
  CPPUNIT_TEST(testExecuteMissingCity);

  CPPUNIT_TEST(testTrimToOrig);
  CPPUNIT_TEST(testJumpToMapAirline);
  CPPUNIT_TEST(testJumpToMapEntryExit);
  CPPUNIT_TEST(testMatchCity);

  CPPUNIT_TEST(testOnMatchFailWhenOnLastSegment);
  CPPUNIT_TEST(testOnMatchFailWhenJustAfterLastSegment);
  CPPUNIT_TEST(testOnMatchFailWhenOnFirstSegment);
  CPPUNIT_TEST(testOnMatchFailWhenOnGateway);

  CPPUNIT_TEST(testGetNodeAndResZonesIntersection);

  CPPUNIT_TEST_SUITE_END();

public:
  MapTraversalTest() {}

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _memHandle.insert(new MyDataHandle(_memHandle));
    _trx = _memHandle.create<PricingTrx>();
    _route = _memHandle.create<std::vector<TravelRoute::CityCarrier> >();
    _missCity = _memHandle.create<int>();
    _missCarrier = _memHandle.create<bool>();
    _mapTraversal = new MapTraversal(false, *_route, *_missCity, *_missCarrier);
  }
  void tearDown()
  {
    _memHandle.clear();
    delete _mapTraversal;
  }

  void testExecuteMaxResSize()
  {
    std::vector<TravelRoute::CityCarrier> route;
    TravelRoute::CityCarrier cityCarrier;
    route.push_back(cityCarrier);

    MapTraversal mapTraversal(false, *_route, *_missCity, *_missCarrier, 1);
    mapTraversal._result.insert("TEST");

    MockSpecifiedRouting map;
    CPPUNIT_ASSERT(mapTraversal.execute(*_trx, 0, &map));
  }

  void testExecutePassAll()
  {
    TravelRoute::CityCarrier cityCarrier;
    _route->push_back(cityCarrier);


    MockSpecifiedRouting map;
    map._passAll = true;

    std::vector<std::string> result;
    CPPUNIT_ASSERT(_mapTraversal->execute(*_trx, &result, &map));
  }

  void testExecutePassAllNextMap()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.boardCity().loc() = "AAA";
    cityCarrier.offCity().loc() = "BBB";
    _route->push_back(cityCarrier);

    MockSpecifiedRouting map1, map2;
    map1._passAll = true;
    map2._passAll = true;

    std::vector<std::string> result;
    CPPUNIT_ASSERT(_mapTraversal->execute(*_trx, &result, &map1, &map2));
    CPPUNIT_ASSERT(result[0] == "AAA-BBB");
  }

  void testExecuteMissingCity()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.boardCity().loc() = "AAA";
    cityCarrier.offCity().loc() = "BBB";
    _route->push_back(cityCarrier);

    NodeCode nodeCode;
    MapNode mapNode;

    MockSpecifiedRouting map;
    map._city2node["AAA"].insert(&mapNode);

    CPPUNIT_ASSERT(!_mapTraversal->execute(*_trx, 0, &map));
  }

  void testTrimToOrig()
  {
    TravelRoute::City city;
    city.loc() = "ABC";
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.boardCity() = city;

    _route->push_back(cityCarrier);

    string strTrim, str = "TST-ABC-ABC";
    _mapTraversal->trimToOrig(str, strTrim);

    CPPUNIT_ASSERT(str == "ABC-ABC");
    CPPUNIT_ASSERT(strTrim == "ABC-TST");
  }

  void testJumpToMapAirline()
  {
    MockSpecifiedRouting map;
    MapNode node;
    node.type() = MapNode::AIRLINE;
    std::vector<TravelRoute::CityCarrier>::const_iterator iter;

    MapTraversal::SpecRoutingDef specMaps(*_trx, &map, &map, 0);

    CPPUNIT_ASSERT(!_mapTraversal->jumpToMap(&node, 0, iter, specMaps));
  }

  void testJumpToMapEntryExit()
  {
    MockRouting rtg;
    rtg._commonPointInd = SpecifiedRouting::ENTRY_EXIT;
    MockSpecifiedRouting map;
    map._routing = &rtg;

    MapNode node;
    node.type() = MapNode::CITY;
    node.tag() = MapNode::CITY;
    std::vector<TravelRoute::CityCarrier>::const_iterator iter;

    MapTraversal::SpecRoutingDef specMaps(*_trx, &map, &map, 0);

    CPPUNIT_ASSERT(!_mapTraversal->jumpToMap(&node, 0, iter, specMaps));
  }

  void testMatchCity()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.offCity().loc() = "AAA";
    _route->push_back(cityCarrier);

    const std::vector<TravelRoute::CityCarrier>::const_iterator city = _route->begin();
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    MapNode node;
    node.code().insert("AAA");

    CPPUNIT_ASSERT(_mapTraversal->matchNode(0, &node, 0, city, nextCity, false) == MapTraversal::MATCH_OK);
  }

  void setup2USAnd1MXTlvRoute(std::vector<TravelRoute::CityCarrier>& travelRoute)
  {
    TravelRoute::CityCarrier segLASORD;
    segLASORD.boardNation() = "US";
    segLASORD.offNation() = "US";
    travelRoute.push_back(segLASORD);

    TravelRoute::CityCarrier segORDCLT;
    segORDCLT.boardNation() = "US";
    segORDCLT.offNation() = "US";
    travelRoute.push_back(segORDCLT);

    TravelRoute::CityCarrier segCLTCUN;
    segCLTCUN.boardNation() = "US";
    segCLTCUN.offNation() = "MX";
    travelRoute.push_back(segCLTCUN);
  }

  void testOnMatchFailWhenOnLastSegment()
  {
    *_missCity = -1;

    MapTraversal::MatchReturn matchReturn = MapTraversal::MATCH_FAIL_END;
    std::vector<TravelRoute::CityCarrier> travelRoute;
    _mapTraversal->_travelRoute = &travelRoute;

    setup2USAnd1MXTlvRoute(travelRoute);

    _mapTraversal->onMatchFail(_mapTraversal->_travelRoute->end() - 1, matchReturn);

    CPPUNIT_ASSERT(*_missCity == -1);
  }

  void testOnMatchFailWhenJustAfterLastSegment()
  {
    *_missCity = -1;

    MapTraversal::MatchReturn matchReturn = MapTraversal::MATCH_FAIL_END;
    std::vector<TravelRoute::CityCarrier> travelRoute;
    _mapTraversal->_travelRoute = &travelRoute;

    setup2USAnd1MXTlvRoute(travelRoute);

    _mapTraversal->onMatchFail(_mapTraversal->_travelRoute->end(), matchReturn);

    CPPUNIT_ASSERT(*_missCity == -1);
  }

  void testOnMatchFailWhenOnFirstSegment()
  {
    *_missCity = -1;

    MapTraversal::MatchReturn matchReturn = MapTraversal::MATCH_FAIL_END;
    std::vector<TravelRoute::CityCarrier> travelRoute;
    _mapTraversal->_travelRoute = &travelRoute;

    setup2USAnd1MXTlvRoute(travelRoute);

    _mapTraversal->onMatchFail(_mapTraversal->_travelRoute->begin(), matchReturn);

    CPPUNIT_ASSERT(*_missCity == 0);
  }

  void testOnMatchFailWhenOnGateway()
  {
    *_missCity = -1;

    MapTraversal::MatchReturn matchReturn = MapTraversal::MATCH_FAIL_END;
    std::vector<TravelRoute::CityCarrier> travelRoute;
    _mapTraversal->_travelRoute = &travelRoute;

    setup2USAnd1MXTlvRoute(travelRoute);

    _mapTraversal->onMatchFail(_mapTraversal->_travelRoute->begin() + 1, matchReturn);

    CPPUNIT_ASSERT(*_missCity == -1);
  }

  void testGetNodeAndResZonesIntersection()
  {
    MapNode frontNode;
    MapNode backNode;
    MapNode boardZones;
    MapNode offZones;
    NationCode nation = "NO";

    NodeCode europe("210");
    NodeCode scandinavia("212");

    _mapTraversal->_path.push_back(&frontNode);
    _mapTraversal->_path.push_back(&backNode);
    frontNode.code().insert(europe);
    backNode.code().insert(europe);
    backNode.code().insert(scandinavia);

    _mapTraversal->getNodeAndResZonesIntersection(boardZones, offZones, _trx->dataHandle(), nation);

    CPPUNIT_ASSERT_EQUAL(size_t(1), boardZones.code().size());
    CPPUNIT_ASSERT_EQUAL(europe, *boardZones.code().begin());
    CPPUNIT_ASSERT_EQUAL(size_t(2), offZones.code().size());
    CPPUNIT_ASSERT_EQUAL(europe, *offZones.code().begin());
    CPPUNIT_ASSERT_EQUAL(scandinavia, *(++(offZones.code().begin())));
  }

private:
  PricingTrx* _trx;
  std::vector<TravelRoute::CityCarrier>* _route;
  MapTraversal* _mapTraversal;
  int* _missCity;
  bool* _missCarrier;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MapTraversalTest);
}
