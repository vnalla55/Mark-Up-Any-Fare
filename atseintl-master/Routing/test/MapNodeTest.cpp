//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Routing/MapNode.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

#include <sstream>
#include <string>

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/ZoneInfo.h"

using namespace std;

using namespace tse;

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

namespace tse
{

class MapNodeMock : public MapNode
{
  friend class MapNodeTest;

public:
  MapNodeMock() : MapNode() {}
  MapNodeMock(const MapNodeMock& node) : MapNode(node) {}
  MapNodeMock(PricingTrx& trx, const RoutingMap& routeRecord) : MapNode(trx, routeRecord) {}
};

class MapNodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MapNodeTest);

  CPPUNIT_TEST(testPrintNodeElements);
  CPPUNIT_TEST(testPrintNodeElements_Nation);
  CPPUNIT_TEST(testHasZoneInNationsWhenPass);
  CPPUNIT_TEST(testHasZoneInNationsWhenFail);
  CPPUNIT_TEST(testConstructorWhenHasZones);
  CPPUNIT_TEST(testConstructorWhenNoZones);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  MapNodeMock* _mapNode;
  PricingTrx* _trx;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _mapNode = _memHandle.create<MapNodeMock>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->getOptions()->setRtw(true);
  }

  void tearDown() { _memHandle.clear(); }

  void testPrintNodeElements()
  {
    std::stringstream stream;

    _mapNode->code().insert("ABC");
    _mapNode->code().insert("DEF");
    _mapNode->code().insert("GHI");

    _mapNode->printNodeElements(stream, "[", "]");
    CPPUNIT_ASSERT(stream.str() == "[ABC/DEF/GHI]");
  }

  void testPrintNodeElements_Nation()
  {
    std::stringstream stream;

    _mapNode->type() = NATION;

    _mapNode->code().insert("ABC");
    _mapNode->code().insert("DEF");
    _mapNode->code().insert("GHI");

    _mapNode->printNodeElements(stream, "[", "]");
    CPPUNIT_ASSERT(stream.str() == "[*ABC/*DEF/*GHI]");
  }

  void testHasZoneInNationsWhenPass()
  {
    NationCode nation = "GB";
    _mapNode->zone2nations().insert(nation);

    CPPUNIT_ASSERT(_mapNode->hasNationInZone(nation));
  }

  void testHasZoneInNationsWhenFail()
  {
    NationCode nation = "PL";
    _mapNode->zone2nations().insert("GB");

    CPPUNIT_ASSERT(!_mapNode->hasNationInZone(nation));
  }

  void testConstructorWhenHasZones()
  {
    RoutingMap routeRecord;
    routeRecord.loc1().locType() = ZONE;
    routeRecord.loc1().loc() = "210";

    MapNodeMock mapNode(*_trx, routeRecord);

    CPPUNIT_ASSERT_EQUAL(size_t(2), mapNode.zone2nations().size());
  }

  void testConstructorWhenNoZones()
  {
    RoutingMap routeRecord;
    routeRecord.loc1().locType() = ZONE;
    routeRecord.loc1().loc() = "220";

    MapNodeMock mapNode(*_trx, routeRecord);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mapNode.zone2nations().size());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MapNodeTest);
}
