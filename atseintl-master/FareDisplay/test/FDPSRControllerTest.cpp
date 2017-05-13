//----------------------------------------------------------------------------
//	File: FDPSRControllerTest.cpp
//
//	Author: Partha Kumar Chakraborti
//  Created:      04/12/2005
//  Description:  This is a unit test class for FDPSRController.cpp
//
//  Copyright Sabre 2005
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

#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/Itin.h"
#include "FareDisplay/FDPSRController.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"

#include <log4cxx/propertyconfigurator.h>

using namespace boost;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  std::vector<TpdPsr*>& _ptrList;

public:
  MyDataHandle(std::vector<TpdPsr*>& ptrList) : _ptrList(ptrList) {}
  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date)
  {
    return _ptrList;
  }
};
}
class FDPSRControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDPSRControllerTest);
  CPPUNIT_TEST(testprocessThruMktCxrs);
  CPPUNIT_TEST(test_validate);
  CPPUNIT_TEST(test_validate_GI);
  CPPUNIT_TEST(test_validate_usingTroughMarketCarriers);
  CPPUNIT_TEST(testgetPSR);
  CPPUNIT_TEST(testgetPSR_2);
  CPPUNIT_TEST_SUITE_END();

public:
  // ------------------------------------------------------
  // @MethodName  setUp()
  // ------------------------------------------------------
  void setUp() { setupTestData(); }

  void setupTestData()
  {
    _trx.itin().clear();
    _trx.itin().push_back(&_itin);

    _options.routingNumber() = "7777";
    _trx.setRequest(&_fdRequest);
    _trx.setOptions(&_options);

    _loc1 = *(Loc*)_trx.dataHandle().getLoc("DFW", time(0));
    _loc2 = *(Loc*)_trx.dataHandle().getLoc("LON", time(0));

    _ts.destination() = &_loc1;
    _ts.origin() = &_loc2;
    _trx.travelSeg().clear();
    _trx.travelSeg().push_back(&_ts);

    _lk1.loc() = "DFW";
    _lk1.locType() = 'C';
    _lk2.loc() = "LON";
    _lk2.locType() = 'C';

    _lk3.loc() = "WAW";
    _lk3.locType() = 'C';
    _lk4.loc() = "KRK";
    _lk4.locType() = 'C';

    _psr1.carrier() = "BA";
    _psr1.thruMktCarrierExcept() = NO;
    _psr1.thruMktCxrs().clear();
    _psr1.thruMktCxrs().push_back("AA");
    _psr1.thruMktCxrs().push_back("BA");
    _psr1.thruMktCxrs().push_back("AI");
    _psr1.globalDir() = GlobalDirection::ZZ;
    _psr1.loc1() = _lk1;
    _psr1.loc2() = _lk2;

    _psr2.carrier() = EMPTY_STRING();
    _psr2.thruMktCarrierExcept() = NO;
    _psr2.thruMktCxrs().clear();
    _psr2.thruMktCxrs().push_back("AA");
    _psr2.thruMktCxrs().push_back("BA");
    _psr2.thruMktCxrs().push_back("AI");
    _psr2.globalDir() = GlobalDirection::ZZ;
    _psr2.loc1() = _lk3;
    _psr2.loc2() = _lk4;

    _psrList.clear();
    _psrList.push_back(&_psr1);
    _psrList.push_back(&_psr2);
  }

  // -------------------------------------------------------------------
  // @MethodName  testprocessThruMktCxrs()
  // -----------------------------------------------------------

  void testprocessThruMktCxrs()
  {
    bool iRet;
    // ----------------------------
    // Tes1:
    // Expected Result: false
    // ----------------------------
    _psr1.thruMktCarrierExcept() = YES;
    _psr1.carrier() = EMPTY_STRING();

    iRet = _fdpsrController.processThruMktCxrs("AA", _psr1);

    CPPUNIT_ASSERT(iRet == false);

    // ----------------------------
    // Tes2:
    // Expected Result: true
    // ----------------------------
    _psr1.thruMktCarrierExcept() = NO;

    iRet = _fdpsrController.processThruMktCxrs("AA", _psr1);

    CPPUNIT_ASSERT(iRet == true);

    // ----------------------------
    // Tes3:
    // Expected Result: false
    // ----------------------------
    _psr1.thruMktCarrierExcept() = NO;
    _psr1.carrier() = "BA";

    iRet = _fdpsrController.processThruMktCxrs("AA", _psr1);

    CPPUNIT_ASSERT(iRet == false);

    return;
  }

  // -------------------------------------------------------------------
  // @MethodName  testvalidate()
  // -----------------------------------------------------------

  void test_validate()
  {
    // validate 1 matching / 1 nonmatching item
    RoutingInfo routingInfo;
    _fdpsrController.validate(_trx, GlobalDirection::AT, "BA", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(1), routingInfo.fdPSR().size());

    // validate 2 matching items
    _psr2.loc1() = _lk1;
    _psr2.loc2() = _lk2;
    routingInfo.fdPSR().clear();
    _fdpsrController.validate(_trx, GlobalDirection::AT, "BA", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(2), routingInfo.fdPSR().size());
  }

  void test_validate_GI()
  {
    RoutingInfo routingInfo;
    // global dir - AT / AT
    routingInfo.fdPSR().clear();
    _psr1.globalDir() = GlobalDirection::AT;
    _fdpsrController.validate(_trx, GlobalDirection::AT, "BA", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(1), routingInfo.fdPSR().size());

    // global dir - AT /   US - validate that the item fails
    routingInfo.fdPSR().clear();
    _psr1.globalDir() = GlobalDirection::US;
    _fdpsrController.validate(_trx, GlobalDirection::AT, "BA", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(0), routingInfo.fdPSR().size());
  }

  void test_validate_usingTroughMarketCarriers()
  {
    RoutingInfo routingInfo;
    // validate that using thruMktCarrierExcept
    _psr1.globalDir() = GlobalDirection::ZZ;
    _psr1.carrier() = EMPTY_STRING();
    _psr1.thruMktCarrierExcept() = NO;
    // "BA" is on the throug market carriers list
    _fdpsrController.validate(_trx, GlobalDirection::AT, "BA", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(1), routingInfo.fdPSR().size());

    routingInfo.fdPSR().clear();
    // "LO" is not on the through market carriers list
    _fdpsrController.validate(_trx, GlobalDirection::AT, "LO", routingInfo, _psrList);
    CPPUNIT_ASSERT_EQUAL(size_t(0), routingInfo.fdPSR().size());
  }

  void testgetPSR()
  {
    MyDataHandle mdh(_psrList);
    RoutingInfo routingInfo;
    // first test - no preffered carriers in the list, and not using through carriers - return none
    _trx.preferredCarriers().insert("AF");
    _trx.preferredCarriers().insert("LO");

    // thru market carrier exemptions
    _psr1.thruMktCarrierExcept() = YES;
    _psr2.thruMktCarrierExcept() = YES;

    _fdpsrController.getPSR(_trx, GlobalDirection::AT, routingInfo);
    CPPUNIT_ASSERT_EQUAL(size_t(0), routingInfo.fdPSR().size());
  }

  void testgetPSR_2()
  {
    MyDataHandle mdh(_psrList);
    RoutingInfo routingInfo;

    _trx.preferredCarriers().insert("AF");
    _trx.preferredCarriers().insert("LO");

    // 2 matching items
    _psr2.loc1() = _lk1;
    _psr2.loc2() = _lk2;

    _psr1.carrier() = "AF";
    _psr2.carrier() = "LO";

    _fdpsrController.getPSR(_trx, GlobalDirection::AT, routingInfo);
    CPPUNIT_ASSERT_EQUAL(size_t(2), routingInfo.fdPSR().size());
  }

private:
  FDPSRController _fdpsrController;
  std::vector<TpdPsr*> _psrList;
  TpdPsr _psr1, _psr2;
  FareDisplayTrx _trx;
  FareDisplayRequest _fdRequest;
  FareDisplayOptions _options;
  Itin _itin;
  AirSeg _ts;
  Loc _loc1, _loc2;
  LocKey _lk1, _lk2, _lk3, _lk4;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FDPSRControllerTest);
}
