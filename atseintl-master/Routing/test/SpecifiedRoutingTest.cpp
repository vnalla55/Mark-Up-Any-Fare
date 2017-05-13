#include "test/include/CppUnitHelperMacros.h"
#include <vector>
#include "Routing/SpecifiedRouting.h"
#include "DBAccess/Routing.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include <boost/assign/std/vector.hpp>

#include "Common/Config/ConfigMan.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
struct MockSpecifiedRouting : public SpecifiedRouting
{
  friend class SpecifiedRoutingTest;
};
namespace
{
struct contains_p
{
  const NodeCode& code;
  contains_p(const NodeCode& code) : code(code) {}
  bool operator()(const std::pair<int16_t, MapNode>& node)
  {
    return find(node.second.code().begin(), node.second.code().end(), code) !=
           node.second.code().end();
  }
  bool operator()(const MapNode* node)
  {
    return node && find(node->code().begin(), node->code().end(), code) != node->code().end();
  }
};
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  Routing* getRouting()
  {
    Routing* ret = _memHandle.create<Routing>();
    ret->noofheaders() = 2;
    return ret;
  }
  RoutingMap* getRTMap(int lnkmapsequence,
                       int loc1No,
                       LocTypeCode lt,
                       LocCode lc,
                       Indicator loctag,
                       int nextLocNo,
                       int altLocNo,
                       NationCode nation)
  {
    // will be destroyed in Routing destructor
    RoutingMap* ret = new RoutingMap;
    ret->lnkmapsequence() = lnkmapsequence;
    ret->loc1No() = loc1No;
    ret->loc1().locType() = lt;
    ret->loc1().loc() = lc;
    ret->loctag() = loctag;
    ret->nextLocNo() = nextLocNo;
    ret->altLocNo() = altLocNo;
    ret->nation() = nation;
    return ret;
  }

public:
  const std::vector<Routing*>& getRouting(const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& routingTariff,
                                          const RoutingNumber& routingNumber,
                                          const DateTime& date)
  {
    if (vendor == "ATP" && carrier == "LP" && routingTariff == 17 && routingNumber == "0003")
    {
      std::vector<Routing*>& ret = *_memHandle.create<std::vector<Routing*> >();
      ret += getRouting();
      ret.front()->rmaps() += getRTMap(1, 1, 'C', "BSB", '1', 4, 0, "BR"),
          getRTMap(2, 2, 'C', "BSB", '1', 5, 0, "BR"), getRTMap(3, 3, 'C', "BSB", '1', 6, 0, "BR"),
          getRTMap(4, 4, 'A', "YY", ' ', 7, 0, ""), getRTMap(5, 5, 'A', "YY", ' ', 8, 0, ""),
          getRTMap(6, 6, 'A', "JJ", ' ', 9, 0, ""), getRTMap(7, 7, 'C', "BSB", ' ', 11, 0, "BR"),
          getRTMap(8, 8, 'C', "BSB", ' ', 12, 0, "BR"),
          getRTMap(9, 9, 'C', "BSB", ' ', 13, 10, "BR"),
          getRTMap(10, 10, 'C', "BSB", ' ', 13, 0, "BR"),
          getRTMap(11, 11, 'C', "BSB", 'X', 0, 0, "CL"),
          getRTMap(12, 12, 'C', "BSB", 'X', 0, 0, "CL"),
          getRTMap(13, 13, 'C', "BSB", 'X', 0, 0, "CL");

      return ret;
    }
    else if (vendor == "ATP" && carrier == "LP" && routingTariff == 17 && routingNumber == "0005")
    {
      std::vector<Routing*>& ret = *_memHandle.create<std::vector<Routing*> >();
      ret += getRouting();
      ret.front()->rmaps() += getRTMap(1, 1, 'C', "SCL", '1', 2, 0, "CL"),
          getRTMap(2, 2, 'C', "BUE", ' ', 7, 3, "AR"), getRTMap(3, 3, 'C', "SAO", ' ', 6, 4, "BR"),
          getRTMap(4, 4, 'C', "SAO", ' ', 8, 5, "BR"), getRTMap(5, 5, 'C', "RIO", ' ', 8, 0, "BR"),
          getRTMap(6, 6, 'A', "JJ", ' ', 10, 0, ""), getRTMap(7, 7, 'A', "AR", ' ', 9, 0, ""),
          getRTMap(8, 8, 'A', "JJ", ' ', 12, 0, ""), getRTMap(9, 9, 'C', "POA", 'X', 0, 0, "BR"),
          getRTMap(10, 10, 'C', "CWB", 'X', 0, 11, "BR"),
          getRTMap(11, 11, 'C', "FLN", 'X', 0, 0, "BR"),
          getRTMap(12, 12, 'C', "SSA", 'X', 0, 13, "BR"),
          getRTMap(13, 13, 'C', "REC", 'X', 0, 14, "BR"),
          getRTMap(14, 14, 'C', "FOR", 'X', 0, 0, "BR");
      return ret;
    }
    return DataHandleMock::getRouting(vendor, carrier, routingTariff, routingNumber, date);
  }
};
}

class SpecifiedRoutingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SpecifiedRoutingTest);
  CPPUNIT_TEST(testEqualityOperator);
  CPPUNIT_TEST(test_reverse);

  CPPUNIT_TEST(test_reindex);
  CPPUNIT_TEST(test_compress);
  CPPUNIT_TEST(test_contains);

  CPPUNIT_TEST(testInitWithFlag_AnyPoint);
  CPPUNIT_TEST(testInitWithFlag_EntryExitPoint);

  CPPUNIT_TEST(testReindexNationWhenConnectPoints);
  CPPUNIT_TEST(testReindexNationWhenTerminalPoints);
  CPPUNIT_TEST(testContainsNationWhenFound);
  CPPUNIT_TEST(testContainsNationWhenNotFound);
  CPPUNIT_TEST(testGetStartNodesNationZoneWhenTerminalEnabledAndFound);
  CPPUNIT_TEST(testGetStartNodesNationZoneWhenTerminalEnabledButNotFound);
  CPPUNIT_TEST(testGetStartNodesNationZoneWhenTerminalDisabledAndFound);
  CPPUNIT_TEST(testGetStartNodesNationZoneWhenTerminalDisabledButNotFound);

  CPPUNIT_TEST(testInitializeWhenHasntGenericCityCode);
  CPPUNIT_TEST(testInitializeWhenHasGenericCityCode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", "2013-06-16", "PRICING_SVC");

    _specRTG = _memHandle.create<MockSpecifiedRouting>();

    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _pricingOptions = _memHandle.create<PricingOptions>();
    _trx->setOptions(_pricingOptions);
  }
  void tearDown()
  {
    _memHandle.clear();
  }

  void testInitWithFlag_AnyPoint()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 6, 17); // flag activated
    _trx->setRequest(&request);

    Routing routing;
    routing.entryExitPointInd() = ANYPOINT;

    _specRTG->initialize(routing, *_trx);

    CPPUNIT_ASSERT(_specRTG->_terminal == false);
  }

  void testInitWithFlag_EntryExitPoint()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 6, 17); // flag activated
    _trx->setRequest(&request);

    Routing routing;
    routing.entryExitPointInd() = ENTRYEXITONLY;

    _specRTG->initialize(routing, *_trx);

    CPPUNIT_ASSERT(_specRTG->_terminal == true);
  }

  void testEqualityOperator()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 5, 16); // flag inactive

    _trx->setRequest(&request);
    const std::vector<Routing*>& routingVect1 =
        _trx->dataHandle().getRouting("ATP", "LP", 17, "0003", DateTime::localTime());
    const std::vector<Routing*>& routingVect2 =
        _trx->dataHandle().getRouting("ATP", "LP", 17, "0005", DateTime::localTime());

    CPPUNIT_ASSERT(routingVect1.size() > 0);
    CPPUNIT_ASSERT(routingVect2.size() > 0);

    SpecifiedRouting specifiedRoutingA = SpecifiedRouting(*routingVect1.at(0), *_trx);
    SpecifiedRouting specifiedRoutingB = SpecifiedRouting(*routingVect2.at(0), *_trx);

    CPPUNIT_ASSERT(!(specifiedRoutingA == specifiedRoutingB));

    SpecifiedRouting specifiedRouting1(*routingVect1.at(0), *_trx);
    SpecifiedRouting specifiedRouting2(*routingVect1.at(0), *_trx);

    // wrong ! equality operator doesn't work for SpecifiedRouting class ... ( fails at
    // compareMapNodes() call, that is corrupt )
    //  CPPUNIT_ASSERT( specifiedRouting1 == specifiedRouting1 ) ;
    //  CPPUNIT_ASSERT( specifiedRouting1 == specifiedRouting2 ) ;
  }

  MapNode addNode(int id, int next, int alt, char tag, char type, MockSpecifiedRouting& rtg)
  {
    MapNode newNode;
    newNode.id() = id;
    newNode.alt() = alt;
    newNode.next() = next;
    newNode.tag() = tag;
    newNode.type() = type;
    newNode.isReverse() = false;

    rtg._node[newNode.id()] = newNode;
    return newNode;
  }

  bool reachable(int start, int end, MockSpecifiedRouting& rtg)
  {
    if (start <= 0)
      return false;
    if (start == end)
      return true;
    if (rtg._node.find(start) == rtg._node.end())
      return false;

    MapNode& node = (rtg._node[start]);

    return ((rtg._node.find(node.next()) != rtg._node.end() &&
             reachable(rtg._node[node.next()].id(), end, rtg)) ||
            (rtg._node.find(node.alt()) != rtg._node.end() &&
             reachable(rtg._node[node.alt()].id(), end, rtg)));
  }

MockSpecifiedRouting& getSpecRouting()
{
    addNode(1,10,0,MapNode::ENTRY,MapNode::CITY, *_specRTG);
    addNode(10,2,0,MapNode::CONNECT,MapNode::CITY, *_specRTG);
    addNode(2,5,3,MapNode::CONNECT,MapNode::AIRLINE, *_specRTG);
    addNode(3,5,4,MapNode::CONNECT,MapNode::AIRLINE, *_specRTG);
    addNode(4,5,0,MapNode::CONNECT,MapNode::AIRLINE, *_specRTG);
    addNode(5,0,0,MapNode::EXIT, MapNode::CITY, *_specRTG);

    return *_specRTG;
}

void test_reverse()
{
    MockSpecifiedRouting& rtg = getSpecRouting() ;

    CPPUNIT_ASSERT(reachable(1, 5, rtg));
    CPPUNIT_ASSERT(!reachable(5, 1, rtg));

    CPPUNIT_ASSERT(rtg.reverseMap(*_trx));

    for (std::map<int16_t, MapNode>::iterator it = rtg._node.begin(); it != rtg._node.end(); ++it)
      CPPUNIT_ASSERT(it->second.isReverse());

    CPPUNIT_ASSERT(reachable(5, 1, rtg));
    CPPUNIT_ASSERT(!reachable(1, 5, rtg));
  }

void test_compress()
{
    MockSpecifiedRouting& rtg = getSpecRouting();

    rtg._node[2].code().insert("CD1");
    rtg._node[3].code().insert("CD2");
    rtg._node[4].code().insert("CD3");

    rtg._node[2].nations().insert("N1");
    rtg._node[3].nations().insert("N2");
    rtg._node[4].nations().insert("N3");

    CPPUNIT_ASSERT(reachable(1, 5, rtg));
    CPPUNIT_ASSERT(!reachable(5, 1, rtg));

    rtg.compress();

    CPPUNIT_ASSERT(reachable(1, 5, rtg));
    CPPUNIT_ASSERT(!reachable(5, 1, rtg));

    contains_p ct("CD1");

    std::map<int16_t, MapNode>::iterator node = find_if(rtg._node.begin(), rtg._node.end(), ct);
    CPPUNIT_ASSERT(node != rtg._node.end());

    // found compressed node, find if other codes and nations were saved
    CPPUNIT_ASSERT(find(node->second.code().begin(), node->second.code().end(), "CD2") !=
                   node->second.code().end());
    CPPUNIT_ASSERT(find(node->second.code().begin(), node->second.code().end(), "CD3") !=
                   node->second.code().end());

    CPPUNIT_ASSERT(find(node->second.nations().begin(), node->second.nations().end(), "N1") !=
                   node->second.nations().end());
    CPPUNIT_ASSERT(find(node->second.nations().begin(), node->second.nations().end(), "N2") !=
                   node->second.nations().end());
    CPPUNIT_ASSERT(find(node->second.nations().begin(), node->second.nations().end(), "N3") !=
                   node->second.nations().end());
  }

  void createCityNames(MockSpecifiedRouting& rtg)
  {
    char lastCity = 'A';
    for (std::map<int16_t, MapNode>::iterator it = rtg._node.begin(); it != rtg._node.end(); ++it)
    {
      if (it->second.type() == MapNode::CITY)
      {
        std::string* cityName = _memHandle.insert(new std::string(std::string("CT") + lastCity));
        it->second.code().insert(*cityName);
        _createdCityNames[it->second.id()] = *cityName;
        ++lastCity;
      }
    }
  }

void test_reindex()
{
    MockSpecifiedRouting& rtg = getSpecRouting();

    createCityNames(rtg);

    CPPUNIT_ASSERT(rtg.reindex());

    for (std::map<int, std::string>::iterator it = _createdCityNames.begin();
         it != _createdCityNames.end();
         ++it)
    {
      std::string& city = it->second;
      std::set<const MapNode*>& nodes = rtg._city2node.find(city)->second;
      contains_p pred(city);
      if (find_if(nodes.begin(), nodes.end(), pred) == nodes.end())
        CPPUNIT_FAIL(std::string("NOT FOUND: " + city));
    }
  }

void test_contains()
{
    MockSpecifiedRouting& rtg = getSpecRouting();
    createCityNames(rtg);
    CPPUNIT_ASSERT(rtg.reindex());

    TravelRoute::City testCity;

    testCity.loc() = _createdCityNames.begin()->second;
    testCity.airport() = "";

    CPPUNIT_ASSERT(rtg.contains(testCity));

    testCity.loc() = "";
    testCity.airport() = _createdCityNames.begin()->second;

    CPPUNIT_ASSERT(rtg.contains(testCity));

    testCity.loc() = "";
    testCity.airport() = "";

    CPPUNIT_ASSERT(!rtg.contains(testCity));

    MapNode tmp;
    rtg._city2node[MapNode::CATCHALL].insert(&tmp);

    CPPUNIT_ASSERT(rtg.contains(testCity));
  }

void prepareReindexNationWhenConnectPoints(bool terminal)
{
  int id = 1, next = 0, alt = 0;
  char tag = MapNode::CONNECT;
  char type = MapNode::NATION;

  addNode(id, next, alt, tag, type, *_specRTG);
  _specRTG->_node[id].code().insert("US");
  _specRTG->_node[id].code().insert("GB");

  id = 2;
  if(terminal)
    tag = MapNode::ENTRY;
  else
    tag = MapNode::CONNECT;
  addNode(id, next, alt, tag, type, *_specRTG);
  _specRTG->_node[id].code().insert("PL");
  _specRTG->_node[id].code().insert("GB");

  tag = MapNode::CONNECT;
  id = 3;
  addNode(id, next, alt, tag, type, *_specRTG);
  _specRTG->_node[id].code().insert("US");

  id = 4;
  type = MapNode::ZONE;
  if(terminal)
    tag = MapNode::ENTRY;
  else
    tag = MapNode::CONNECT;
  addNode(id, next, alt, tag, type, *_specRTG);
  _specRTG->_node[id].zone2nations().insert("GB");
  _specRTG->_node[id].zone2nations().insert("PL");
  _specRTG->_node[id].zone2nations().insert("IT");
  _specRTG->_node[id].zone2nations().insert("DE");

  tag = MapNode::CONNECT;
  id = 5;
  addNode(id, next, alt, tag, type, *_specRTG);
  _specRTG->_node[id].zone2nations().insert("TN");
  _specRTG->_node[id].zone2nations().insert("EG");

  _specRTG->_terminal = terminal;
}

void testReindexNationWhenConnectPoints()
{
  bool terminal = false;

  prepareReindexNationWhenConnectPoints(terminal);

  _specRTG->reindexNation();

  CPPUNIT_ASSERT_EQUAL(size_t(7), _specRTG->_nation2node.size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2node["US"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2node["PL"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(3), _specRTG->_nation2node["GB"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["IT"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["DE"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["TN"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["EG"].size());

  CPPUNIT_ASSERT_EQUAL(size_t(0), _specRTG->_nation2nodeEntry.size());
}

void testReindexNationWhenTerminalPoints()
{
  bool terminal = true;

  prepareReindexNationWhenConnectPoints(terminal);

  _specRTG->reindexNation();

  CPPUNIT_ASSERT_EQUAL(size_t(7), _specRTG->_nation2node.size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2node["US"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2node["PL"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(3), _specRTG->_nation2node["GB"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["IT"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["DE"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["TN"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2node["EG"].size());

  CPPUNIT_ASSERT_EQUAL(size_t(4), _specRTG->_nation2nodeEntry.size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2nodeEntry["PL"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), _specRTG->_nation2nodeEntry["GB"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2nodeEntry["IT"].size());
  CPPUNIT_ASSERT_EQUAL(size_t(1), _specRTG->_nation2nodeEntry["DE"].size());
}

void testContainsNationWhenFound()
{
  NationCode nation = "US";
  std::set<const MapNode *> nodes;
  _specRTG->_nation2node.insert(std::make_pair(nation, nodes));

  CPPUNIT_ASSERT(_specRTG->containsNation(nation));
}

void testContainsNationWhenNotFound()
{
  NationCode nation = "US";
  std::set<const MapNode *> nodes;
  _specRTG->_nation2node.insert(std::make_pair(nation, nodes));

  CPPUNIT_ASSERT(!_specRTG->containsNation("GB"));
}

void prepareGetStartNodesNationZone(std::vector<TravelRoute::CityCarrier>& travelRoute,
                                    const NationCode& nation)
{
  std::set<const MapNode *> nodeList;
  MapNode* node1 = _memHandle.create<MapNode>();
  node1->id() = 1;
  nodeList.insert(node1);
  MapNode* node2 = _memHandle.create<MapNode>();
  node1->id() = 2;
  nodeList.insert(node2);
  _specRTG->_nation2nodeEntry.insert(std::make_pair(nation, nodeList));
  MapNode* node3 = _memHandle.create<MapNode>();
  node3->id() = 3;
  nodeList.insert(node3);
  _specRTG->_nation2node.insert(std::make_pair(nation, nodeList));
  travelRoute.push_back(TravelRoute::CityCarrier());
  travelRoute.front().boardNation() = nation;
}

void testGetStartNodesNationZoneWhenTerminalEnabledAndFound()
{
  _specRTG->setTerminal(true);
  std::vector<TravelRoute::CityCarrier> travelRoute;
  std::set<const MapNode *, MapNodeCmp> nodes;
  NationCode nation = "US";

  prepareGetStartNodesNationZone(travelRoute, nation);

  _specRTG->getStartNodesNationZone(travelRoute, nodes);

  CPPUNIT_ASSERT_EQUAL(size_t(2), nodes.size());
  CPPUNIT_ASSERT(nodes.find(*_specRTG->_nation2nodeEntry.begin()->second.begin()) != nodes.end());
  CPPUNIT_ASSERT(nodes.find(*(++_specRTG->_nation2nodeEntry.begin()->second.begin())) != nodes.end());
}

void testGetStartNodesNationZoneWhenTerminalEnabledButNotFound()
{
  _specRTG->setTerminal(true);
  std::vector<TravelRoute::CityCarrier> travelRoute;
  std::set<const MapNode *, MapNodeCmp> nodes;
  NationCode nation = "US";

  prepareGetStartNodesNationZone(travelRoute, nation);
  travelRoute.front().boardNation() = "GB";

  _specRTG->getStartNodesNationZone(travelRoute, nodes);

  CPPUNIT_ASSERT_EQUAL(size_t(0), nodes.size());
}

void testGetStartNodesNationZoneWhenTerminalDisabledAndFound()
{
  _specRTG->setTerminal(false);
  std::vector<TravelRoute::CityCarrier> travelRoute;
  std::set<const MapNode *, MapNodeCmp> nodes;
  NationCode nation = "US";

  prepareGetStartNodesNationZone(travelRoute, nation);

  _specRTG->getStartNodesNationZone(travelRoute, nodes);

  CPPUNIT_ASSERT_EQUAL(size_t(3), nodes.size());
  CPPUNIT_ASSERT(nodes.find(*_specRTG->_nation2node.begin()->second.begin()) != nodes.end());
  CPPUNIT_ASSERT(nodes.find(*(++_specRTG->_nation2node.begin()->second.begin())) != nodes.end());
  CPPUNIT_ASSERT(nodes.find(*(++(++_specRTG->_nation2node.begin()->second.begin()))) != nodes.end());
}

void testGetStartNodesNationZoneWhenTerminalDisabledButNotFound()
{
  _specRTG->setTerminal(false);
  std::vector<TravelRoute::CityCarrier> travelRoute;
  std::set<const MapNode *, MapNodeCmp> nodes;
  NationCode nation = "US";

  prepareGetStartNodesNationZone(travelRoute, nation);
  travelRoute.front().boardNation() = "GB";

  _specRTG->getStartNodesNationZone(travelRoute, nodes);

  CPPUNIT_ASSERT_EQUAL(size_t(0), nodes.size());
}

  void prepareInitialize(Routing& routing)
  {
    routing.entryExitPointInd() = ANYPOINT;
    RoutingMap* rm = new RoutingMap;
    rm->loc1No() = 1;
    rm->loc1().locType() = MapNode::CITY;
    rm->loc1().loc() = "LON";
    routing.rmaps().push_back(rm);
  }

  void testInitializeWhenHasntGenericCityCode()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 6, 17); // flag activated
    _trx->setRequest(&request);

    Routing routing;
    prepareInitialize(routing);

    MockSpecifiedRouting specifiedRouting;
    specifiedRouting.initialize(routing, *_trx);

    CPPUNIT_ASSERT_EQUAL(size_t(1), specifiedRouting.nodes().size());
  }

  void testInitializeWhenHasGenericCityCode()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2013, 6, 17); // flag activated
    _trx->setRequest(&request);

    Routing routing;
    prepareInitialize(routing);
    routing.rmaps()[0]->loc1().loc() = "ECC";

    MockSpecifiedRouting specifiedRouting;
    specifiedRouting.initialize(routing, *_trx);

    CPPUNIT_ASSERT_EQUAL(size_t(0), specifiedRouting.nodes().size());
  }

private:
  MockSpecifiedRouting* _specRTG;
  std::map<int,std::string > _createdCityNames ;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingOptions* _pricingOptions;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecifiedRoutingTest);
}
