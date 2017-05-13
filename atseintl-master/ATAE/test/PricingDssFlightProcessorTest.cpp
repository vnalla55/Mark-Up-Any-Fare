//----------------------------------------------------------------------------
//
//  File   :  PricingDssFlightProcessorTest.cpp
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
#include "ATAE/PricingDssFlightKey.h"
#include "ATAE/PricingDssFlightMapBuilder.h"
#include "ATAE/PricingDssFlightProcessor.h"
#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
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

class PricingDssFlightProcessorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingDssFlightProcessorTest);

  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_1hs);
  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_2hs);
  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_2hs_inter);
  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_2hs_dn);
  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_3hs);
  CPPUNIT_TEST(testAddEquipmentTypeForHiddenStop_0hs);

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


  void testAddEquipmentTypeForHiddenStop_1hs()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "737";
    flight._equipTypeLastLeg = "777";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    airSeg.origin() = origin;

    Loc* hiddenStop1 = _memHandle.create<Loc>();
    hiddenStop1->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop1);

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)2, equipmentTypes.size());

    std::string eq = equipmentTypes[0].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);

    eq = equipmentTypes[1].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);
  }

  void testAddEquipmentTypeForHiddenStop_2hs()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "737";
    flight._equipTypeLastLeg = "777";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    airSeg.origin() = origin;

    Loc* hiddenStop1 = _memHandle.create<Loc>();
    hiddenStop1->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop1);

    Loc* hiddenStop2 = _memHandle.create<Loc>();
    hiddenStop2->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop2);

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)3, equipmentTypes.size());

    std::string eq = equipmentTypes[0].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);

    eq = equipmentTypes[1].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);

    eq = equipmentTypes[2].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);
  }

  void testAddEquipmentTypeForHiddenStop_2hs_inter()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "777";
    flight._equipTypeLastLeg = "737";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "UK";
    airSeg.origin() = origin;

    Loc* hiddenStop1 = _memHandle.create<Loc>();
    hiddenStop1->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop1);

    Loc* hiddenStop2 = _memHandle.create<Loc>();
    hiddenStop2->nation() = "IN";
    airSeg.hiddenStops().push_back(hiddenStop2);

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)3, equipmentTypes.size());

    std::string eq = equipmentTypes[0].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);

    eq = equipmentTypes[1].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);

    eq = equipmentTypes[2].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);
  }

  void testAddEquipmentTypeForHiddenStop_2hs_dn()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "737";
    flight._equipTypeLastLeg = "777";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    airSeg.origin() = origin;

    Loc* hiddenStop1 = _memHandle.create<Loc>();
    hiddenStop1->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop1);

    Loc* hiddenStop2 = _memHandle.create<Loc>();
    hiddenStop2->nation() = "UK";
    airSeg.hiddenStops().push_back(hiddenStop2);

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)3, equipmentTypes.size());

    std::string eq = equipmentTypes[0].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);

    eq = equipmentTypes[1].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);

    eq = equipmentTypes[2].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);
  }

  void testAddEquipmentTypeForHiddenStop_3hs()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "737";
    flight._equipTypeLastLeg = "777";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    airSeg.origin() = origin;

    Loc* hiddenStop1 = _memHandle.create<Loc>();
    hiddenStop1->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop1);

    Loc* hiddenStop2 = _memHandle.create<Loc>();
    hiddenStop2->nation() = "UK";
    airSeg.hiddenStops().push_back(hiddenStop2);

    Loc* hiddenStop3 = _memHandle.create<Loc>();
    hiddenStop3->nation() = "PL";
    airSeg.hiddenStops().push_back(hiddenStop3);

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)4, equipmentTypes.size());

    std::string eq = equipmentTypes[0].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("737"), eq);

    eq = equipmentTypes[1].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);

    eq = equipmentTypes[2].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);

    eq = equipmentTypes[3].c_str();
    CPPUNIT_ASSERT_EQUAL(std::string("777"), eq);
  }

  void testAddEquipmentTypeForHiddenStop_0hs()
  {
    PricingDssFlightProcessor processor(_trx);

    PricingDssFlight flight;
    flight._equipTypeFirstLeg = "737";
    flight._equipTypeLastLeg = "777";

    AirSeg airSeg;

    Loc* origin = _memHandle.create<Loc>();
    origin->nation() = "PL";
    airSeg.origin() = origin;

    processor.addEquipmentTypeForHiddenStop(flight, airSeg);
    std::vector<EquipmentType>& equipmentTypes = airSeg.equipmentTypes();
    CPPUNIT_ASSERT_EQUAL((size_t)0, equipmentTypes.size());
  }

private:

  TestMemHandle _memHandle;
  PricingTrx _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PricingDssFlightProcessorTest);

}
