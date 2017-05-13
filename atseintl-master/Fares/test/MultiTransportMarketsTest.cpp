#include "test/include/CppUnitHelperMacros.h"
#include "Common/MultiTransportMarkets.h"
#include "DBAccess/DataHandle.h"
#include <vector>
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DataHandle.h"
#include "test/include/MockGlobal.h"
#include "Common/TseEnums.h"
#include "DBAccess/MultiTransport.h"
#include <iostream>
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  MultiTransport* getMC(LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const std::vector<MultiTransport*>& getMultiTransportLocs(const LocCode& city,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    if (city == "LON")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      ret->push_back(getMC("LCY"));
      ret->push_back(getMC("LGW"));
      ret->push_back(getMC("LHR"));
      ret->push_back(getMC("LTN"));
      ret->push_back(getMC("QQP"));
      ret->push_back(getMC("QQS"));
      ret->push_back(getMC("QQU"));
      ret->push_back(getMC("QQW"));
      ret->push_back(getMC("STN"));
      ret->push_back(getMC("XQE"));
      return *ret;
    }
    else if (city == "AMS")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      return *ret;
    }
    else if (city == "NYC")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      ret->push_back(getMC("EWR"));
      ret->push_back(getMC("JFK"));
      ret->push_back(getMC("JRB"));
      ret->push_back(getMC("LGA"));
      ret->push_back(getMC("TSS"));
      return *ret;
    }
    return DataHandleMock::getMultiTransportLocs(city, carrierCode, tvlType, tvlDate);
  }
};
}

class MultiTransportMarketsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MultiTransportMarketsTest);
  CPPUNIT_TEST(testgetMarkets);
  CPPUNIT_TEST_SUITE_END();

public:
  void testgetMarkets()
  {
    MyDataHandle mdh;
    DateTime localTime = DateTime::localTime();
    MultiTransportMarkets mul("LON", "AMS", "KL", GeoTravelType::International, localTime, localTime);
    std::vector<MultiTransportMarkets::Market> markets;
    CPPUNIT_ASSERT_EQUAL(true, mul.getMarkets(markets));

    DataHandle dataHandle;
    const std::vector<MultiTransport*>& multi =
        dataHandle.getMultiTransportLocs("NYC", "KL", GeoTravelType::Domestic, localTime);
    CPPUNIT_ASSERT_EQUAL(false, multi.empty());

    std::set<LocCode> airports;
    LocCode loc = "NYC";
    CPPUNIT_ASSERT_EQUAL(true, airports.empty());
    mul.getAirports(airports, loc);
    CPPUNIT_ASSERT_EQUAL(false, airports.empty());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MultiTransportMarketsTest);
}
