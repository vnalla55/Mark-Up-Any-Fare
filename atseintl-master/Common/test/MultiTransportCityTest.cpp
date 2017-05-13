//-------------------------------------------------------------------
//
//  File:        MultiTransportCityTest.h
//  Created:
//  Authors:
//
//  Updates:
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"

#include "DBAccess/DataHandle.h"
#include "Common/LocUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "EWR")
      return "NYC";
    return DataHandleMock::getMultiTransportCity(locCode);
  }
};
}
class LocUtilMultiTransportCityTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(LocUtilMultiTransportCityTest);
  CPPUNIT_TEST(testGetMultiTransportCityEWRDomestic);
  CPPUNIT_TEST(testGetMultiTransportCityEWRInternational);
  CPPUNIT_TEST(testGetMultiTransportCityXAX_AA_International);
  CPPUNIT_TEST(testGetMultiTransportCityXAX_2R_International);
  CPPUNIT_TEST(testGetMultiTransportCityIAHDomestic);
  CPPUNIT_TEST(testGetMultiTransportCityIAH_WN_Domestic);
  CPPUNIT_TEST(testGetMultiTransportCityIAH_WN_International);
  CPPUNIT_TEST(testGetMultiTransportCityIAH_CO_Domestic);
  CPPUNIT_TEST(testGetMultiTransportCityIAH_CO_International);
  CPPUNIT_TEST(testGetMultiTransportCityIAGInternational);
  CPPUNIT_TEST(testGetMultiTransportCityJFK_International);
  CPPUNIT_TEST(testGetMultiTransportCity2);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetMultiTransportCityEWRDomestic()
  {
    LocCode c = LocUtil::getMultiTransportCity("EWR", "AA", GeoTravelType::Domestic, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("EWR"), c);
  }

  void testGetMultiTransportCityEWRInternational()
  {
    LocCode c = LocUtil::getMultiTransportCity("EWR", "AA", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("EWR"), c);
  }

  void testGetMultiTransportCityXAX_AA_International()
  {
    LocCode c = LocUtil::getMultiTransportCity("XAX", "AA", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("YMQ"), c);
  }

  void testGetMultiTransportCityXAX_2R_International()
  {
    LocCode c = LocUtil::getMultiTransportCity("XAX", "2R", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("YMQ"), c);
  }

  void testGetMultiTransportCityIAHDomestic()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "", GeoTravelType::Domestic, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("HOU"), c);
  }

  void testGetMultiTransportCityIAGInternational()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("HOU"), c);
  }

  void testGetMultiTransportCityIAH_WN_Domestic()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "WN", GeoTravelType::Domestic, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("IAH"), c);
  }

  void testGetMultiTransportCityIAH_WN_International()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "WN", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("HOU"), c);
  }

  void testGetMultiTransportCityIAH_CO_Domestic()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "CO", GeoTravelType::Domestic, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("HOU"), c);
  }

  void testGetMultiTransportCityIAH_CO_International()
  {
    LocCode c = LocUtil::getMultiTransportCity("IAH", "CO", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("HOU"), c);
  }

  void testGetMultiTransportCityJFK_International()
  {
    LocCode c = LocUtil::getMultiTransportCity("JFK", "AA", GeoTravelType::International, DateTime::localTime());
    CPPUNIT_ASSERT_EQUAL(LocCode("NYC"), c);
  }

  void testGetMultiTransportCity2()
  {
    MyDataHandle mdh;
    DataHandle dataHandle;

    bool rc = LocUtil::isAirportInCity("EWR", "EWR");
    CPPUNIT_ASSERT(rc == false);

    rc = LocUtil::isAirportInCity("EWR", "EWR", "", GeoTravelType::Domestic);
    CPPUNIT_ASSERT(rc == true);

    rc = LocUtil::isAirportInCity("EWR", "NYC");
    CPPUNIT_ASSERT(rc == true);

    LocCode c = dataHandle.getMultiTransportCity("EWR");
    CPPUNIT_ASSERT_EQUAL(std::string("NYC"), static_cast<std::string>(c));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(LocUtilMultiTransportCityTest);
}
