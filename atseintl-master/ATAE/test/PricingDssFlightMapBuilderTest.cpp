//----------------------------------------------------------------------------
//
//  File   :  PricingDssFlightMapBuilderTest.cpp
//
//  Author :  Janusz Jagodzinski
//
//  Copyright Sabre 2015
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "ATAE/test/MockObjects.h"
#include "ATAE/PricingDssRequest.h"
#include "ATAE/PricingDssResponseHandler.h"
#include "ATAE/PricingDssFlightMapBuilder.h"
#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/DiskCache.h"

namespace tse
{

class PricingDssFlightMapBuilderTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingDssFlightMapBuilderTest);

  CPPUNIT_TEST(testPopulateFlightMap);

  CPPUNIT_TEST_SUITE_END();

public:
  //-----------------------------------------------------------------
  // setUp()
  //-----------------------------------------------------------------
  void setUp()
  {
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

  void buildFlightVector(std::vector<AirSeg*>& airSegments, const std::string& equipCode = "")
  {
    AirSeg* airSeg = _memHandle.create<AirSeg>();
    airSeg->origAirport() = "DFW";
    airSeg->destAirport() = "LGW";
    airSeg->carrier() = "BA";
    airSeg->flightNumber() = 2192;

    AirSeg* airSegSecondItin = _memHandle.create<AirSeg>();
    airSegSecondItin->origAirport() = "KRK";
    airSegSecondItin->destAirport() = "FRA";
    airSegSecondItin->carrier() = "LO";
    airSegSecondItin->flightNumber() = 2193;

    if (!equipCode.empty())
    {
      airSeg->equipmentType() = equipCode.c_str();
      airSegSecondItin->equipmentType() = equipCode.c_str();
    }
    airSegments.push_back(airSeg);
    airSegments.push_back(airSegSecondItin);
  }

  void testPopulateFlightMap()
  {
    std::vector<AirSeg*> airSegments;
    buildFlightVector(airSegments);
    CarrierSwapUtilMock carrierSwapMock;
    PricingDssFlightMapBuilder<CarrierSwapUtilMock, CurrentTimeMock>
      builder(carrierSwapMock, MethodGetFlownSchedule::NORMAL_GET_FLOWN);

    builder.populateFlightMap(airSegments.begin(), airSegments.end(), _flightMap);
    CPPUNIT_ASSERT(_flightMap.size() == 2);
    const PricingDssFlightKey & key = _flightMap.cbegin()->first;
    const std::vector<AirSeg*>& segs = _flightMap.cbegin()->second;
    CPPUNIT_ASSERT(key._carrier == "BA");
    CPPUNIT_ASSERT(key._origin == "DFW");
    CPPUNIT_ASSERT(key._destination == "LGW");
    CPPUNIT_ASSERT(key._flightNumber == 2192);

    CPPUNIT_ASSERT(segs.size() == 1);
    CPPUNIT_ASSERT(segs.front() == airSegments.front());

    PricingDssFlightMap::const_iterator it = _flightMap.cbegin();
    ++it;
    const PricingDssFlightKey & secondKey = it->first;
    const std::vector<AirSeg*>& secondSegs = it->second;

    CPPUNIT_ASSERT(secondKey._carrier == "LO");
    CPPUNIT_ASSERT(secondKey._origin == "KRK");
    CPPUNIT_ASSERT(secondKey._destination == "FRA");
    CPPUNIT_ASSERT(secondKey._flightNumber == 2193);

    CPPUNIT_ASSERT(secondSegs.size() == 1);
    CPPUNIT_ASSERT(secondSegs.front() == airSegments.at(1));

  }
 private:
  TestMemHandle _memHandle;
  PricingDssFlightMap _flightMap;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingDssFlightMapBuilderTest);

}
