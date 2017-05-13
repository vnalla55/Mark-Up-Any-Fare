//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/ZoneInfo.h"
#include "Routing/MapValidator.h"
#include "Routing/RouteStringExtraction.h"
#include "Routing/SpecifiedRouting.h"

using namespace std;

namespace tse
{
namespace
{

class MyDataHandle : public DataHandleMock
{
public:
  const ZoneInfo*
  getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
  {
    ZoneInfo* zoneInfo = _memHandle.create<ZoneInfo>();

    if(zone == "0000210")
    {
      ZoneInfo::ZoneSeg* zoneSeg1 = _memHandle.create<ZoneInfo::ZoneSeg>();

      zoneInfo->sets().resize(1);
      zoneInfo->sets()[0].resize(2);

      zoneSeg1->locType() = LOCTYPE_NATION;
      zoneSeg1->loc() = "GB";
      zoneInfo->sets()[0][0] = *zoneSeg1;

      ZoneInfo::ZoneSeg* zoneSeg2 = _memHandle.create<ZoneInfo::ZoneSeg>();

      zoneSeg2->locType() = LOCTYPE_NATION;
      zoneSeg2->loc() = "PL";
      zoneInfo->sets()[0][1] = *zoneSeg2;
    }

    return zoneInfo;
  }

protected:
  mutable TestMemHandle _memHandle;
};
}

class MapValidatorMock : public MapValidator
{
  friend class MapValidatorTest;

public:
  MapValidatorMock(const std::vector<TravelRoute::CityCarrier>& travelRoute,
                   int& city,
                   bool& carrier)
    : MapValidator(travelRoute, city, carrier)
  {
  }

  bool
  enumeratePaths(const MapNode* prevNode, const MapNode* lastValidatedNode,
                 const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                 MapTraversal::SpecRoutingDef& specMaps, bool isOverfly)
  {
    const MapNode* currNode = std::get<MapTraversal::MAP>(specMaps)->getNext(prevNode);

    return MapTraversal::enumeratePaths(currNode, prevNode, lastValidatedNode, city, specMaps,
        isOverfly);
  }

protected:
  virtual void outputPath(const PricingTrx& trx) {}
};

class RouteStringExtractionRTWMock: public RouteStringExtractionRTW
{
  friend class MapValidatorTest;

  RouteStringExtractionRTWMock(const std::vector<TravelRoute::CityCarrier>& travelRoute,
                               int& city, bool& carrier, const uint16_t rtgStrMaxResSize) :
    RouteStringExtractionRTW(travelRoute, city, carrier, rtgStrMaxResSize)
  {}
};

class MapValidatorTest: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MapValidatorTest);

  CPPUNIT_TEST(testGetStartNodesWhenTerminalEnabled);
  CPPUNIT_TEST(testGetStartNodesWhenTerminalDisabled);

  CPPUNIT_TEST(testIsOverflyPossibleWhenMapCarrierFail);
  CPPUNIT_TEST(testIsOverflyPossibleWhenMapCarrierPass);
  CPPUNIT_TEST(testIsOverflyPossibleWhenCarrierPass);
  CPPUNIT_TEST(testIsOverflyPossibleWhenEmptyNodePass);

  CPPUNIT_TEST(testCheckCarrierWhenFail);
  CPPUNIT_TEST(testCheckCarrierWhenCarrierPass);
  CPPUNIT_TEST(testCheckCarrierWhenIndustryCarrierPass);
  CPPUNIT_TEST(testCheckCarrierWhenSurfaceCarrierPass);

  CPPUNIT_TEST(testMatchNodeWhenSurfaceCarrierPass);
  CPPUNIT_TEST(testMatchNodeWhenIndustryCarrierPass);
  CPPUNIT_TEST(testMatchNodeWhenNodeCarrierPass);
  CPPUNIT_TEST(testMatchNodeWhenNodeCarrierFail);
  CPPUNIT_TEST(testMatchNodeWhenCityAndPrevAirlinePass);
  CPPUNIT_TEST(testMatchNodeWhenCityAndPrevAirlineDiffCxrPass);
  CPPUNIT_TEST(testMatchNodeWhenCityAndNoPrevAirlineFail);
  CPPUNIT_TEST(testMatchNodeWhenCityAndMapCarrierPass);
  CPPUNIT_TEST(testMatchNodeWhenCityAndSurfaceCarrierPass);
  CPPUNIT_TEST(testMatchNodeWhenCityAndNationPass);
  CPPUNIT_TEST(testMatchNodeWhenOverflyFail);
  CPPUNIT_TEST(testMatchNodeWhenOverflyPass);

  CPPUNIT_TEST(testEnumeratePathsWhenOnlyCitiesPass);
  CPPUNIT_TEST(testEnumeratePathsWhenOneCityInSameNationPass);
  CPPUNIT_TEST(testEnumeratePathsWhenOneCityInSameNationFail);
  CPPUNIT_TEST(testEnumeratePathsWhenTwoCitiesInSameNationPass);
  CPPUNIT_TEST(testEnumeratePathsWhenTwoCitiesInSameNationFail);
  CPPUNIT_TEST(testEnumeratePathsWhenOverflyPass);

  CPPUNIT_TEST(testGetStartNodesWhenZonesAndTerminalEnabled);
  CPPUNIT_TEST(testGetStartNodesWhenZonesAndTerminalDisabled);

  CPPUNIT_TEST(testMatchNodeWhenZonePass);
  CPPUNIT_TEST(testMatchNodeWhenZoneFail);

  CPPUNIT_TEST(testEnumeratePathsWhenTwoCitiesInSameZonePass);
  CPPUNIT_TEST(testEnumeratePathsWhenTwoCitiesInSameZoneFail);

  CPPUNIT_TEST(testHasPointOnMapWhenHasCityOnMap);
  CPPUNIT_TEST(testHasPointOnMapWhenHasntCityOnMap);
  CPPUNIT_TEST(testHasPointOnMapWhenHasCityOnNextMap);
  CPPUNIT_TEST(testHasPointOnMapWhenHasntCityOnNextMap);
  CPPUNIT_TEST(testHasPointOnMapWhenHasCityOnNextMap2);
  CPPUNIT_TEST(testHasPointOnMapWhenHasntCityOnNextMap2);
  CPPUNIT_TEST(testHasPointOnMapWhenHasNationOnMap);
  CPPUNIT_TEST(testHasPointOnMapWhenHasntNationOnMap);

  CPPUNIT_TEST(testCheckCarrierWhenGenericAllianceCodePass);
  CPPUNIT_TEST(testCheckCarrierWhenGenericAllianceCodeFail);

  CPPUNIT_TEST(testSetupStartingNodeWhenNation);
  CPPUNIT_TEST(testSetupStartingNodeWhenCity);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
    _tvlRoute = _memHandle.create<std::vector<TravelRoute::CityCarrier> >();
    _tvlRoute->push_back(TravelRoute::CityCarrier());
    _tvlRoute->push_back(TravelRoute::CityCarrier());
    _tvlRoute->push_back(TravelRoute::CityCarrier());
    _tvlRoute->push_back(TravelRoute::CityCarrier());
    _tvlRoute->push_back(TravelRoute::CityCarrier());

    _missCity = _memHandle.create<int>();
    _missCarrier = _memHandle.create<bool>();
    _mapValidatorMock = _memHandle.insert(new MapValidatorMock(*_tvlRoute, *_missCity, *_missCarrier));
    _routeStringExtractionRTWMock = _memHandle.insert(new RouteStringExtractionRTWMock(
          *_tvlRoute, *_missCity, *_missCarrier, 12));
    _tvlRoute->clear();

    _map = _memHandle.create<SpecifiedRouting>();
    _nextMap = _memHandle.create<SpecifiedRouting>();
    _nextMap2 = _memHandle.create<SpecifiedRouting>();
    _nodes = _memHandle.create<std::set<const MapNode*, MapNodeCmp> >();
    _nodesExtract = _memHandle.create<std::set<const MapNode*, MapNodeCmp> >();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(true);
  }

  void tearDown() { _memHandle.clear(); }

  RoutingMap& addRoutingNode(
      int id, int next, LocCode loc, LocTypeCode locType, const NationCode nation, Indicator tag)
  {
    RoutingMap& rMap = *_memHandle.create<RoutingMap>();

    rMap.loc1No() = id;
    rMap.nextLocNo() = next;
    rMap.loc1().loc() = loc;
    rMap.loc1().locType() = locType;
    rMap.nation() = nation;
    rMap.loctag() = tag;

    return rMap;
  }

  void prepareGetStartNodesWhenNation()
  {
    int id = 1;
    NationCode loc = "GB";
    Indicator tag = MapNode::ENTRY;
    LocTypeCode locType = MapNode::NATION;

    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    id = 2;
    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    id = 3;
    tag = MapNode::CONNECT;
    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    _map->reindexNation();

    TravelRoute::CityCarrier cc;
    cc.boardNation() = loc;
    _tvlRoute->push_back(cc);
  }

  void testGetStartNodesWhenTerminalEnabled()
  {
    _map->setTerminal(true);
    prepareGetStartNodesWhenNation();

    _mapValidatorMock->getStartNodes(_map, *_nodes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _nodes->size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(_map->_nation2nodeEntry.begin()->second.size(), _nodes->size());
    CPPUNIT_ASSERT(_nodes->find(*_map->_nation2nodeEntry.begin()->second.begin()) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++_map->_nation2nodeEntry.begin()->second.begin())) != _nodes->end());

    _routeStringExtractionRTWMock->getStartNodes(_map, *_nodesExtract);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _nodesExtract->size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(_map->_nation2nodeEntry.begin()->second.size(), _nodesExtract->size());
    CPPUNIT_ASSERT(_nodesExtract->find(*_map->_nation2nodeEntry.begin()->second.begin()) != _nodesExtract->end());
    CPPUNIT_ASSERT(_nodesExtract->find(*(++_map->_nation2nodeEntry.begin()->second.begin())) != _nodesExtract->end());
  }

  void testGetStartNodesWhenTerminalDisabled()
  {
    _map->setTerminal(false);
    prepareGetStartNodesWhenNation();

    _mapValidatorMock->getStartNodes(_map, *_nodes);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _nodes->size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _map->_nation2nodeEntry.size());
    CPPUNIT_ASSERT(_nodes->find(*_map->_nation2node.begin()->second.begin()) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++_map->_nation2node.begin()->second.begin())) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++(++_map->_nation2node.begin()->second.begin()))) != _nodes->end());

    _routeStringExtractionRTWMock->getStartNodes(_map, *_nodesExtract);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _nodesExtract->size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _map->_nation2nodeEntry.size());
    CPPUNIT_ASSERT(_nodesExtract->find(*_map->_nation2node.begin()->second.begin()) != _nodesExtract->end());
    CPPUNIT_ASSERT(_nodesExtract->find(*(++_map->_nation2node.begin()->second.begin())) != _nodesExtract->end());
    CPPUNIT_ASSERT(_nodesExtract->find(*(++(++_map->_nation2node.begin()->second.begin()))) != _nodesExtract->end());
  }

  void prepareMatchNode(SpecifiedRouting* map)
  {
    TravelRoute::CityCarrier cc;
    int id = 1;
    int next = id + 1;
    LocCode loc = "CHI";
    LocTypeCode locType = MapNode::CITY;
    NationCode nation = "US";
    Indicator tag = MapNode::ENTRY;
    CarrierCode carrier = "UA";

    map->_carrier = carrier;

    map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    cc.boardCity().loc() = loc;
    cc.boardNation() = nation;
    id = 2;
    next = id + 1;
    tag = MapNode::CONNECT;
    loc = carrier;
    locType = MapNode::AIRLINE;
    nation = "GB";
    cc.carrier() = carrier;
    cc.offCity().loc() = "LON";
    cc.offNation() = nation;
    _tvlRoute->push_back(cc);
    map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));
    loc = "LON";

    cc.boardCity().loc() = loc;
    cc.boardNation() = nation;
    id = 3;
    next = id + 1;
    tag = MapNode::CONNECT;
    loc = "AMS";
    locType = MapNode::CITY;
    nation = "NL";
    cc.carrier() = carrier;
    cc.offCity().loc() = loc;
    cc.offNation() = nation;
    _tvlRoute->push_back(cc);
    map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    cc.boardCity().loc() = loc;
    cc.boardNation() = nation;
    id = 4;
    next = id + 1;
    tag = MapNode::CONNECT;
    loc = "PAR";
    locType = MapNode::CITY;
    nation = "FR";
    cc.carrier() = carrier;
    cc.offCity().loc() = loc;
    cc.offNation() = nation;
    _tvlRoute->push_back(cc);
    map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));
  }

  void testIsOverflyPossibleWhenMapCarrierFail()
  {
    prepareMatchNode(_map);

    CarrierCode cityCarrier = "AA";
    CarrierCode mapCarrier = "UA";
    const MapNode& node = (++(++_map->nodes().begin()))->second;

    CPPUNIT_ASSERT(!_mapValidatorMock->isOverflyPossible(&node, cityCarrier, cityCarrier, mapCarrier));
  }

  void testIsOverflyPossibleWhenMapCarrierPass()
  {
    prepareMatchNode(_map);

    CarrierCode cityCarrier = "UA";
    CarrierCode mapCarrier = "UA";
    const MapNode& node = (++(++_map->nodes().begin()))->second;

    CPPUNIT_ASSERT(_mapValidatorMock->isOverflyPossible(&node, cityCarrier, cityCarrier, mapCarrier));
  }

  void testIsOverflyPossibleWhenCarrierPass()
  {
    prepareMatchNode(_map);

    CarrierCode cityCarrier = "UA";
    CarrierCode mapCarrier = "UA";
    const MapNode& node = (++_map->nodes().begin())->second;

    CPPUNIT_ASSERT(_mapValidatorMock->isOverflyPossible(&node, cityCarrier, cityCarrier, mapCarrier));
  }

  void testIsOverflyPossibleWhenEmptyNodePass()
  {
    const MapNode* node = 0;

    CPPUNIT_ASSERT(_mapValidatorMock->isOverflyPossible(node, "", "", ""));
  }

  void testCheckCarrierWhenFail()
  {
    prepareMatchNode(_map);

    const MapNode& node = (++_map->nodes().begin())->second;
    CarrierCode cityCarrier = "AA";

    CPPUNIT_ASSERT(!_mapValidatorMock->checkCarrier(&node, cityCarrier, cityCarrier));
  }

  void testCheckCarrierWhenCarrierPass()
  {
    prepareMatchNode(_map);

    const MapNode& node = (++_map->nodes().begin())->second;
    CarrierCode cityCarrier = "UA";

    CPPUNIT_ASSERT(_mapValidatorMock->checkCarrier(&node, cityCarrier, cityCarrier));
  }

  void testCheckCarrierWhenIndustryCarrierPass()
  {
    prepareMatchNode(_map);

    MapNode& node = (++_map->_node.begin())->second;
    node.code().insert(INDUSTRY_CARRIER);
    CarrierCode cityCarrier = "AA";

    CPPUNIT_ASSERT(_mapValidatorMock->checkCarrier(&node, cityCarrier, cityCarrier));
  }

  void testCheckCarrierWhenSurfaceCarrierPass()
  {
    prepareMatchNode(_map);

    const MapNode& node = (++_map->nodes().begin())->second;
    CarrierCode cityCarrier = SURFACE_CARRIER;

    CPPUNIT_ASSERT(_mapValidatorMock->checkCarrier(&node, cityCarrier, cityCarrier));
  }

  void testCheckCarrierWhenGenericAllianceCodePass()
  {
    prepareMatchNode(_map);

    MapNode& node  = (++_map->_node.begin())->second;
    GenericAllianceCode genericAllianceCode = "*A";
    node.code().insert(genericAllianceCode);
    CarrierCode cityCarrier = "AA";

    CPPUNIT_ASSERT(_mapValidatorMock->checkCarrier(&node, cityCarrier, genericAllianceCode));
  }

  void testCheckCarrierWhenGenericAllianceCodeFail()
  {
    prepareMatchNode(_map);

    MapNode& node  = (++_map->_node.begin())->second;
    GenericAllianceCode genericAllianceCode = "*A";
    node.code().insert("*O");
    CarrierCode cityCarrier = "AA";

    CPPUNIT_ASSERT(!_mapValidatorMock->checkCarrier(&node, cityCarrier, genericAllianceCode));
  }

  void testMatchNodeWhenSurfaceCarrierPass()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[0].carrier() = SURFACE_CARRIER;

    const MapNode& node = (++_map->nodes().begin())->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &node, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenIndustryCarrierPass()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[0].carrier() = "AA";

    MapNode& node = (++_map->_node.begin())->second;
    node.code().insert(INDUSTRY_CARRIER);

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &node, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenNodeCarrierPass()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[0].carrier() = "UA";

    const MapNode& node = (++_map->nodes().begin())->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &node, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenNodeCarrierFail()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[0].carrier() = "AA";

    const MapNode& node = (++_map->nodes().begin())->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_FAIL_CARRIER,
                         _mapValidatorMock->matchNode(_map, &node, &node, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenCityAndPrevAirlinePass()
  {
    prepareMatchNode(_map);

    const MapNode& node = (++(++_map->nodes().begin()))->second;
    const MapNode& prevNode = (++_map->nodes().begin())->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 1;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenCityAndPrevAirlineDiffCxrPass()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[1].carrier() = "AA";

    const MapNode& node = (++(++_map->nodes().begin()))->second;
    const MapNode& prevNode = (++_map->nodes().begin())->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 1;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenCityAndNoPrevAirlineFail()
  {
    prepareMatchNode(_map);

    _map->_carrier = "AA";

    const MapNode& node = (++(++(++_map->nodes().begin())))->second;
    const MapNode& prevNode = (++(++_map->nodes().begin()))->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_FAIL_CARRIER,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenCityAndMapCarrierPass()
  {
    prepareMatchNode(_map);

    const MapNode& node = (++(++(++_map->nodes().begin())))->second;
    const MapNode& prevNode = (++(++_map->nodes().begin()))->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenCityAndSurfaceCarrierPass()
  {
    prepareMatchNode(_map);

    _map->_carrier = "AA";
    (*_tvlRoute)[2].carrier() = SURFACE_CARRIER;

    const MapNode& node = (++(++(++_map->nodes().begin())))->second;
    const MapNode& prevNode = (++(++_map->nodes().begin()))->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenCityAndNationPass()
  {
    prepareMatchNode(_map);

    _map->_carrier = "AA";

    const MapNode& node = (++(++(++_map->nodes().begin())))->second;
    MapNode& prevNode = (++(++_map->_node.begin()))->second;
    prevNode.type() = MapNode::NATION;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenOverflyFail()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[2].carrier() = "AA";

    const MapNode& node = (++(++_map->nodes().begin()))->second;
    const MapNode& prevNode = (++(++_map->_node.begin()))->second;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_FAIL_CARRIER,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void testMatchNodeWhenOverflyPass()
  {
    prepareMatchNode(_map);

    (*_tvlRoute)[2].carrier() = "AA";
    (*_tvlRoute)[2].carrier() = SURFACE_CARRIER;

    const MapNode& node = (++(++_map->nodes().begin()))->second;
    const MapNode& prevNode = (++(++_map->_node.begin()))->second;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT(city == nextCity);
  }

  void prepareEnumeratePaths(CarrierCode carrier = "AA")
  {
    TravelRoute::CityCarrier cc;
    int id = 1;
    int next = id + 1;
    LocCode loc = "CHI";
    LocTypeCode locType = MapNode::CITY;
    NationCode nation = "US";
    Indicator tag = MapNode::ENTRY;

    _map->_carrier = carrier;

    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    cc.boardCity().loc() = loc;
    id = 2;
    next = id + 1;
    tag = MapNode::CONNECT;
    loc = "LON";
    nation = "GB";
    cc.carrier() = carrier;
    cc.offNation() = nation;
    cc.offCity().loc() = loc;
    _tvlRoute->push_back(cc);
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 3;
    next = id + 1;
    loc = "MAN";
    nation = "GB";
    cc.carrier() = carrier;
    cc.offNation() = nation;
    cc.offCity().loc() = loc;
    _tvlRoute->push_back(cc);
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 4;
    next = id + 1;
    loc = "PAR";
    nation = "FR";
    cc.carrier() = carrier;
    cc.offCity().loc() = loc;
    if (carrier == "AA")
    {
      locType = MapNode::CITY;
    }
    else
    {
      loc = "FR";
      locType = MapNode::NATION;
    }
    cc.offNation() = nation;
    _tvlRoute->push_back(cc);
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 5;
    next = id + 1;
    loc = "MEX";
    nation = "MX";
    locType = MapNode::CITY;
    cc.offNation() = nation;
    cc.offCity().loc() = loc;
    cc.carrier() = carrier;
    _tvlRoute->push_back(cc);
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));
    MapNode& node = (--_map->_node.end())->second;
    node.alt() = 8;

    id = 6;
    next = id + 1;
    loc = "AA";
    locType = MapNode::AIRLINE;
    cc.carrier() = carrier;
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 7;
    next = 0;
    loc = "CHI";
    nation = "US";
    locType = MapNode::CITY;
    tag = MapNode::EXIT;
    cc.carrier() = carrier;
    cc.offNation() = nation;
    cc.offCity().loc() = loc;
    _tvlRoute->push_back(cc);
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    if (carrier == "AA")
      return;

    id = 8;
    next = id + 1;
    loc = "PAR";
    nation = "FR";
    locType = MapNode::CITY;
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 9;
    next = id + 1;
    loc = "MEX";
    nation = "MX";
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    id = 10;
    next = 0;
    loc = "CHI";
    nation = "US";
    tag = MapNode::EXIT;
    _map->addNode(*_trx, addRoutingNode(id, next, loc, locType, nation, tag));

    _mapValidatorMock->_travelRoute = _tvlRoute;
    _trx->setForceNoTimeout(true);
  }

  void testEnumeratePathsWhenOnlyCitiesPass()
  {
    prepareEnumeratePaths();

    const MapNode& prevNode = _map->nodes().begin()->second;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenOneCityInSameNationPass()
  {
    prepareEnumeratePaths();

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node = (++_map->_node.begin())->second;
    node.code().insert("GB");
    node.type() = MapNode::NATION;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenOneCityInSameNationFail()
  {
    prepareEnumeratePaths();

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node = (++_map->_node.begin())->second;
    node.code().insert("PL");
    node.type() = MapNode::NATION;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(!_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenTwoCitiesInSameNationPass()
  {
    prepareEnumeratePaths();

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node1 = (++_map->_node.begin())->second;
    node1.code().insert("GB");
    node1.type() = MapNode::NATION;
    MapNode& node2 = (++(++_map->_node.begin()))->second;
    node2.code().insert("GB");
    node2.type() = MapNode::NATION;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenTwoCitiesInSameNationFail()
  {
    prepareEnumeratePaths();
    _map->_carrier = "UA";

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node1 = (++_map->_node.begin())->second;
    node1.code().insert("GB");
    node1.type() = MapNode::NATION;
    MapNode& node2 = (++(++_map->_node.begin()))->second;
    node2.code().insert("PL");
    node2.type() = MapNode::NATION;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(!_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenOverflyPass()
  {
    prepareEnumeratePaths("UA");

    const MapNode& prevNode = _map->nodes().begin()->second;
    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void prepareGetStartNodesWhenZone()
  {
    int id = 1;
    NationCode nation = "GB";
    Indicator tag = MapNode::ENTRY;
    LocCode loc = "210";
    LocTypeCode locType = MapNode::ZONE;

    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    id = 2;
    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    id = 3;
    tag = MapNode::CONNECT;
    _map->addNode(*_trx, addRoutingNode(id, 0, loc, locType, "", tag));
    _map->reindexNation();

    TravelRoute::CityCarrier cc;
    cc.boardNation() = nation;
    _tvlRoute->push_back(cc);
  }

  void testGetStartNodesWhenZonesAndTerminalEnabled()
  {
    _map->setTerminal(true);
    prepareGetStartNodesWhenZone();

    _mapValidatorMock->getStartNodes(_map, *_nodes);

    CPPUNIT_ASSERT_EQUAL(size_t(2), _nodes->size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), (++_map->_nation2node.begin())->second.size());
    CPPUNIT_ASSERT_EQUAL(_map->_nation2nodeEntry.begin()->second.size(), _nodes->size());
    CPPUNIT_ASSERT(_nodes->find(*_map->_nation2nodeEntry.begin()->second.begin()) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++_map->_nation2nodeEntry.begin()->second.begin())) !=
                   _nodes->end());
  }

  void testGetStartNodesWhenZonesAndTerminalDisabled()
  {
    _map->setTerminal(false);
    prepareGetStartNodesWhenZone();

    _mapValidatorMock->getStartNodes(_map, *_nodes);

    CPPUNIT_ASSERT_EQUAL(size_t(3), _nodes->size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _map->_nation2node.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), _map->_nation2node.begin()->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(3), (++_map->_nation2node.begin())->second.size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), _map->_nation2nodeEntry.size());
    CPPUNIT_ASSERT(_nodes->find(*_map->_nation2node.begin()->second.begin()) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++_map->_nation2node.begin()->second.begin())) != _nodes->end());
    CPPUNIT_ASSERT(_nodes->find(*(++(++_map->_nation2node.begin()->second.begin()))) !=
                   _nodes->end());
  }

  void testMatchNodeWhenZonePass()
  {
    prepareMatchNode(_map);

    _map->_carrier = "AA";

    MapNode& node = (++(++(++_map->_node.begin())))->second;
    node.type() = MapNode::ZONE;
    node.zone2nations().insert("FR");
    MapNode& prevNode = (++(++_map->_node.begin()))->second;
    prevNode.type() = MapNode::ZONE;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_OK,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
    CPPUNIT_ASSERT((city + 1) == nextCity);
  }

  void testMatchNodeWhenZoneFail()
  {
    prepareMatchNode(_map);

    _map->_carrier = "AA";

    MapNode& node = (++(++(++_map->_node.begin())))->second;
    node.type() = MapNode::ZONE;
    node.zone2nations().insert("NL");
    MapNode& prevNode = (++(++_map->_node.begin()))->second;
    prevNode.type() = MapNode::ZONE;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin() + 2;
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    CPPUNIT_ASSERT_EQUAL(MapTraversal::MATCH_FAIL_CARRIER,
                         _mapValidatorMock->matchNode(_map, &node, &prevNode, city, nextCity, false));
  }

  void testEnumeratePathsWhenTwoCitiesInSameZonePass()
  {
    prepareEnumeratePaths();

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node1 = (++_map->_node.begin())->second;
    node1.zone2nations().insert("GB");
    node1.type() = MapNode::ZONE;
    MapNode& node2 = (++(++_map->_node.begin()))->second;
    node2.zone2nations().insert("GB");
    node2.type() = MapNode::ZONE;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testEnumeratePathsWhenTwoCitiesInSameZoneFail()
  {
    prepareEnumeratePaths();
    _map->_carrier = "UA";

    const MapNode& prevNode = _map->nodes().begin()->second;

    MapNode& node1 = (++_map->_node.begin())->second;
    node1.zone2nations().insert("GB");
    node1.type() = MapNode::ZONE;
    MapNode& node2 = (++(++_map->_node.begin()))->second;
    node2.zone2nations().insert("PL");
    node2.type() = MapNode::ZONE;

    const std::vector<TravelRoute::CityCarrier>::const_iterator& city = _tvlRoute->begin();

    MapTraversal::SpecRoutingDef specMaps(*_trx, _map, 0, 0);

    CPPUNIT_ASSERT(!_mapValidatorMock->enumeratePaths(&prevNode, 0, city, specMaps, false));
  }

  void testHasPointOnMapWhenHasCityOnMap()
  {
    prepareMatchNode(_map);
    _map->reindex();

    const TravelRoute::City& city = _tvlRoute->front().boardCity();
    NationCode nation = "";

    CPPUNIT_ASSERT(_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, 0));
    CPPUNIT_ASSERT(_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, 0));
  }

  void testHasPointOnMapWhenHasntCityOnMap()
  {
    prepareMatchNode(_map);
    _map->reindex();

    TravelRoute::City city;
    city.loc() = "KRK";
    NationCode nation = "";

    CPPUNIT_ASSERT(!_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, 0));
    CPPUNIT_ASSERT(!_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, 0));
  }

  void testHasPointOnMapWhenHasCityOnNextMap()
  {
    prepareMatchNode(_nextMap);
    _nextMap->reindex();

    const TravelRoute::City& city = _tvlRoute->front().boardCity();
    NationCode nation = "";

    CPPUNIT_ASSERT(_mapValidatorMock->hasPointOnMap(city, nation, _map, _nextMap, 0));
    CPPUNIT_ASSERT(_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, _nextMap, 0));
  }

  void testHasPointOnMapWhenHasntCityOnNextMap()
  {
    prepareMatchNode(_nextMap);
    _nextMap->reindex();

    TravelRoute::City city;
    city.loc() = "KRK";
    NationCode nation = "";

    CPPUNIT_ASSERT(!_mapValidatorMock->hasPointOnMap(city, nation, _map, _nextMap, 0));
    CPPUNIT_ASSERT(!_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, _nextMap, 0));
  }

  void testHasPointOnMapWhenHasCityOnNextMap2()
  {
    prepareMatchNode(_nextMap2);
    _nextMap2->reindex();

    const TravelRoute::City& city = _tvlRoute->front().boardCity();
    NationCode nation = "";

    CPPUNIT_ASSERT(_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, _nextMap2));
    CPPUNIT_ASSERT(_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, _nextMap2));
  }

  void testHasPointOnMapWhenHasntCityOnNextMap2()
  {
    prepareMatchNode(_nextMap2);
    _nextMap2->reindex();

    TravelRoute::City city;
    city.loc() = "KRK";
    NationCode nation = "";

    CPPUNIT_ASSERT(!_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, _nextMap2));
    CPPUNIT_ASSERT(!_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, _nextMap2));
  }

  void testHasPointOnMapWhenHasNationOnMap()
  {
    prepareGetStartNodesWhenNation();

    const TravelRoute::City& city = _tvlRoute->front().boardCity();
    NationCode nation = "GB";

    CPPUNIT_ASSERT(_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, 0));
    CPPUNIT_ASSERT(_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, 0));
  }

  void testHasPointOnMapWhenHasntNationOnMap()
  {
    prepareGetStartNodesWhenNation();

    const TravelRoute::City& city = _tvlRoute->front().boardCity();
    NationCode nation = "PL";

    CPPUNIT_ASSERT(!_mapValidatorMock->hasPointOnMap(city, nation, _map, 0, 0));
    CPPUNIT_ASSERT(!_routeStringExtractionRTWMock->hasPointOnMap(city, nation, _map, 0, 0));
  }

  void prepareSetupStartingNode(MapNode& node, int16_t id, int16_t alt, Indicator type, Indicator tag,
                                const LocCode& originLoc)
  {
    node.id() = id;
    node.next() = id + 1;
    node.alt() = alt;
    node.type() = type;
    node.tag() = tag;
    node.code().insert(originLoc);
  }

  void testSetupStartingNodeWhenNation()
  {
    NodeCode originLoc = "*AU";
    MapNode currentNode;
    int16_t id = 5;
    Indicator type = MapNode::NATION;
    Indicator tag = MapNode::ENTRY;

    MapNode node;
    prepareSetupStartingNode(node, id, 3, type, tag, originLoc);

    _mapValidatorMock->setupStartingNode(currentNode, node, originLoc);

    CPPUNIT_ASSERT_EQUAL(id, currentNode.id());
    CPPUNIT_ASSERT_EQUAL(id, currentNode.next());
    CPPUNIT_ASSERT_EQUAL(int16_t(0), currentNode.alt());
    CPPUNIT_ASSERT_EQUAL(type, currentNode.type());
    CPPUNIT_ASSERT_EQUAL(tag, currentNode.tag());
    CPPUNIT_ASSERT_EQUAL(size_t(2), currentNode.code().size());
    CPPUNIT_ASSERT_EQUAL(originLoc, *currentNode.code().begin());
    CPPUNIT_ASSERT_EQUAL(INDUSTRY_CARRIER, *(++currentNode.code().begin()));
  }

  void testSetupStartingNodeWhenCity()
  {
    NodeCode originLoc = "FRA";
    MapNode currentNode;
    int16_t id = 5;
    int16_t alt = 7;
    Indicator type = MapNode::CITY;
    Indicator tag = MapNode::COMMONPOINT;

    MapNode node;
    prepareSetupStartingNode(node, id, alt, type, tag, originLoc);
    node.code().insert("BER");

    _mapValidatorMock->setupStartingNode(currentNode, node, originLoc);

    CPPUNIT_ASSERT_EQUAL(id, currentNode.id());
    CPPUNIT_ASSERT_EQUAL(int16_t(id + 1), currentNode.next());
    CPPUNIT_ASSERT_EQUAL(alt, currentNode.alt());
    CPPUNIT_ASSERT_EQUAL(type, currentNode.type());
    CPPUNIT_ASSERT_EQUAL(tag, currentNode.tag());
    CPPUNIT_ASSERT_EQUAL(size_t(1), currentNode.code().size());
    CPPUNIT_ASSERT_EQUAL(originLoc, *currentNode.code().begin());
  }

private:
  TestMemHandle _memHandle;
  std::vector<TravelRoute::CityCarrier>* _tvlRoute;
  int* _missCity;
  bool* _missCarrier;
  MapValidatorMock* _mapValidatorMock;
  RouteStringExtractionRTWMock* _routeStringExtractionRTWMock;
  SpecifiedRouting* _map;
  SpecifiedRouting* _nextMap;
  SpecifiedRouting* _nextMap2;
  std::set<const MapNode*, MapNodeCmp>* _nodes;
  std::set<const MapNode*, MapNodeCmp>* _nodesExtract;
  PricingTrx* _trx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MapValidatorTest);

} // namespace tse
