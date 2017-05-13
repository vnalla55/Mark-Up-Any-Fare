//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include <iostream>
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include <iostream>

#include "Routing/SpecifiedRoutingCache.h"
#include "Routing/AddOnRouteExtraction.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include <boost/assign/std/vector.hpp>
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include "Common/Config/ConfigMan.h"

using namespace std;
using namespace boost::assign;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  RoutingMap* getRtgMap(int nodeNo, LocTypeCode lt, LocCode lc, int nextNode)
  {
    RoutingMap* ret = new RoutingMap;
    ret->loc1No() = nodeNo;
    ret->loc1().locType() = lt;
    ret->loc1().loc() = lc;
    ret->nextLocNo() = nextNode;
    ret->nation() = "AR";
    return ret;
  }

public:
  const std::vector<Routing*>& getRouting(const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& routingTariff,
                                          const RoutingNumber& routingNumber,
                                          const DateTime& date)
  {
    if (carrier == "LP" && routingNumber == "0003")
    {
      std::vector<Routing*>& ret = *_memHandle.create<std::vector<Routing*> >();
      Routing* rtg = _memHandle.create<Routing>();
      rtg->vendor() = vendor;
      rtg->carrier() = carrier;
      rtg->routingTariff() = routingTariff;
      rtg->routing() = routingNumber;
      rtg->rmaps() += getRtgMap(1, 'C', "COR", 2), getRtgMap(2, 'C', "ROS", 3),
          getRtgMap(3, 'C', "MDZ", 4), getRtgMap(4, 'C', "BRC", 0);
      ret += rtg;
      return ret;
    }
    return DataHandleMock::getRouting(vendor, carrier, routingTariff, routingNumber, date);
  }
  const CarrierPreference* getCarrierPreference(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "LP")
      return TestCarrierPreferenceFactory::create(
          "/vobs/atseintl/test/testdata/data/CarrierPreference/CarrierPreference_BLANK.xml");
    return DataHandleMock::getCarrierPreference(carrier, date);
  }
};
}

class AddOnRouteExtractionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AddOnRouteExtractionTest);
  CPPUNIT_TEST(testAddOn);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", "2013-06-16", "PRICING_SVC");

    _memHandle.create<MyDataHandle>();
  }
  void tearDown()
  {
    _memHandle.clear();
  }

  void testAddOn()
  {
    PricingRequest request;
    request.ticketingDT() = DateTime(2014, 6, 16); // flag active
    _trx.setRequest(&request);
    PricingOptions pricingOptions;
    _trx.setOptions(&pricingOptions);

    DataHandle dh;
    DateTime now(DateTime::localTime());
    const Routing* routing = NULL;
    const std::vector<Routing*>& routingVect =
        _trx.dataHandle().getRouting("ATP", "LP", 17, "0003", now);
    if (!routingVect.empty())
      routing = routingVect.front();

    // SpecifiedRoutingDAO& dao = SpecifiedRoutingDAO::instance();
    SpecifiedRoutingKey key(*routing, now);
    SpecifiedRouting& map = SpecifiedRoutingCache::getSpecifiedRouting(_trx, key);

    CPPUNIT_ASSERT(routing != 0);
    const Loc* bog = dh.getLoc("COR", now);
    std::vector<std::string> result;
    AddOnRouteExtraction extract(bog);
    CPPUNIT_ASSERT(extract.execute(_trx, &result, &map));
    CPPUNIT_ASSERT(!result.empty());
    /*vector<std::string>::iterator i;
    for (i = result.begin(); i != result.end(); i++)
        // LOG4CXX_INFO(_logger, *i);
        std::cout << *i << std::endl;*/
  }

  void createMap(Routing& routing,
                 int loc1No,
                 Indicator loctag,
                 int nextLocNo,
                 int altLocNo,
                 char loc1Type,
                 const char* loc1Code)
  {
    RoutingMap* map = _memHandle.create<RoutingMap>();
    map->vendor() = routing.vendor();
    map->carrier() = routing.carrier();
    map->routingTariff() = routing.routingTariff();
    map->routing() = routing.routing();
    map->effDate() = DateTime::localTime();
    map->lnkmapsequence() = loc1No;
    map->loc1No() = loc1No;
    map->loctag() = loctag;
    map->nextLocNo() = nextLocNo;
    map->altLocNo() = altLocNo;
    map->loc1().locType() = loc1Type;
    map->loc1().loc() = loc1Code;
    map->localRouting() = "";
    routing.rmaps().push_back(map);
  }

private:
  PricingTrx _trx;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AddOnRouteExtractionTest);
}
