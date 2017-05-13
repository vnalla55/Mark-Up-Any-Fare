//----------------------------------------------------------------------------
//  Copyright Sabre 2004,2009
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
#include <fstream>
#include "Rules/SecSurchargeAppl.h"
#include "DBAccess/SectorSurcharge.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DBAccess/Loc.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/Fare.h"
#include "DataModel/PaxType.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "Common/TseConsts.h"
#include "Rules/RuleConst.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/SurchargeData.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestFareUsageFactory.h"
#include "test/testdata/TestPaxTypeFactory.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/CppUnitHelperMacros.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "DBAccess/NUCInfo.h"

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    if (currency == "USD")
    {
      NUCInfo* ret = _memHandle.create<NUCInfo>();
      ret->_cur = "USD";
      ret->_nucFactor = 1;
      ret->_roundingFactor = 1;
      ret->_nucFactorNodec = 8;
      ret->_roundingRule = NEAREST;

      return ret;
    }
    return DataHandleMock::getNUCFirst(currency, carrier, date);
  }
};
}
class SecSurchargeApplTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SecSurchargeApplTest);
  CPPUNIT_TEST(testSurcharge1);
  CPPUNIT_TEST(testSurcharge2);
  CPPUNIT_TEST(testSurcharge3);
  CPPUNIT_TEST(testSurcharge4);
  CPPUNIT_TEST(testSurcharge5);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx _trx;
  PricingRequest _request;
  PaxType* _paxTypeAdult;
  PaxType* _paxTypeChild;
  AirSeg* _airSeg0;
  AirSeg* _airSeg1;
  FareUsage* _fu;
  SectorSurcharge _surchInfo;
  LocKey _posLoc;
  Agent _agent;
  Loc _agentLoc;
  DCFactory* _factory;
  DiagCollector* _diag;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown() { _memHandle.clear(); }
  void buildBaseData()
  {
    _trx.setRequest(&_request);

    // Borrow xml files created for accompanied travel test
    _paxTypeAdult = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeAdult.xml");
    _paxTypeChild = TestPaxTypeFactory::create("AccompaniedTravelData/PaxTypeChild.xml");
    _airSeg0 = TestAirSegFactory::create("AccompaniedTravelData/AirSegDFWJFK.xml");
    _airSeg1 = TestAirSegFactory::create("AccompaniedTravelData/AirSegJFKDFW.xml");
    _fu = TestFareUsageFactory::create("AccompaniedTravelData/FareUsage0.xml");

    _trx.travelSeg().push_back(_airSeg0);
    _trx.travelSeg().push_back(_airSeg1);
    _trx.paxType().push_back(_paxTypeAdult);
    _trx.paxType().push_back(_paxTypeChild);

    // Setup Diagnostics
    _trx.diagnostic().diagnosticType() = Diagnostic416;
    _trx.diagnostic().activate();
    _factory = DCFactory::instance();
    CPPUNIT_ASSERT(_factory != 0);
    _diag = _factory->create(_trx);
    _diag->enable(Diagnostic416);

    // Rule Item Info
    _surchInfo.userApplType() = SecSurchargeAppl::NOT_APPLY; // not checked
    _surchInfo.firstTvlDate() = DateTime(2004, 6, 20);
    _surchInfo.startTime() = 0;
    _surchInfo.lastTvlDate() = DateTime(2004, 10, 10);
    _surchInfo.stopTime() = 1440;
    _surchInfo.exclPsgType() = SecSurchargeAppl::NOT_APPLY;
    _surchInfo.surchargeType() = 'F';
    _surchInfo.directionalInd() = FROM;
    _surchInfo.loc1().locType() = LOCTYPE_AIRPORT;
    _surchInfo.loc1().loc() = "DFW";
    _surchInfo.loc2().locType() = LOCTYPE_NONE;
    _surchInfo.surchargeAmt1() = 0;
    _surchInfo.surchargeAmt2() = 0;
    _surchInfo.surchargePercent() = 5;
    _surchInfo.surchargeAppl() = SecSurchargeAppl::NOT_APPLY;
    // not require direction of fare component
    _surchInfo.tktgCxrs().clear();

    _posLoc.locType() = LOCTYPE_NATION;
    _posLoc.loc() = "US";
    _surchInfo.posLoc() = _posLoc;

    _surchInfo.poiLoc().locType() = LOCTYPE_NONE;

    //--------------------------------------------------------
    // Sale/Ticketing Request
    //--------------------------------------------------------

    _agentLoc.nation() = "US";
    _agentLoc.loc() = "DFW";

    _agent.agentLocation() = &_agentLoc;

    _request.ticketingAgent() = &_agent;
  }

  void testSurcharge1()
  {
    buildBaseData();
    SecSurchargeAppl secSurchAppl;

    //--------------------------------------------------------
    // Should PASS, get surcharged
    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() != 0);
    _fu->surchargeData().clear();
  }

  void testSurcharge2()
  {
    buildBaseData();
    CarrierCode carAA = "AA";
    CarrierCode carCA = "CA";

    _surchInfo.tktgCxrs().push_back(carAA);
    _surchInfo.tktgCxrs().push_back(carCA);

    //--------------------------------------------------------
    // Inclusive Carriers
    _surchInfo.exceptTktgCarrier() = SecSurchargeAppl::NOT_APPLY;

    SecSurchargeAppl secSurchAppl;

    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() != 0);
    _fu->surchargeData().clear();
  }

  void testSurcharge3()
  {
    buildBaseData();

    CarrierCode carAA = "AA";
    CarrierCode carCA = "CA";

    _surchInfo.tktgCxrs().push_back(carAA);
    _surchInfo.tktgCxrs().push_back(carCA);

    //--------------------------------------------------------
    // Exception Carriers
    _surchInfo.exceptTktgCarrier() = YES;

    SecSurchargeAppl secSurchAppl;

    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() == 0);
    _fu->surchargeData().clear();
  }

  void testSurcharge4()
  {
    buildBaseData();

    CarrierCode carAA = "AA";
    CarrierCode carCA = "CA";

    _surchInfo.tktgCxrs().push_back(carAA);
    _surchInfo.tktgCxrs().push_back(carCA);

    //--------------------------------------------------------
    // Agent Location
    _surchInfo.exceptTktgCarrier() = SecSurchargeAppl::NOT_APPLY;
    _agentLoc.nation() = "CN";

    SecSurchargeAppl secSurchAppl;

    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() == 0);
    _fu->surchargeData().clear();
  }

  void testSurcharge5()
  {
    buildBaseData();
    MoneyAmount amountNuc = (*_fu->surchargeData().begin())->amountNuc();

    CarrierCode carAA = "AA";
    CarrierCode carCA = "CA";

    _surchInfo.tktgCxrs().push_back(carAA);
    _surchInfo.tktgCxrs().push_back(carCA);

    _surchInfo.exceptTktgCarrier() = SecSurchargeAppl::NOT_APPLY;
    //--------------------------------------------------------
    // Cat12 surcharge already exist with same amount
    _agentLoc.nation() = "US";

    SurchargeData cat12Surch;
    cat12Surch.surchargeType() = _surchInfo.surchargeType();
    cat12Surch.carrier() = _airSeg0->carrier();
    cat12Surch.brdAirport() = _airSeg0->origAirport();
    cat12Surch.offAirport() = _airSeg0->destAirport();
    cat12Surch.amountNuc() = amountNuc;

    _fu->surchargeData().push_back(&cat12Surch);

    SecSurchargeAppl secSurchAppl;

    CPPUNIT_ASSERT(_fu->surchargeData().size() == 1);
    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() == 1);

    //--------------------------------------------------------
    // Cat12 surcharge already exist with different amount, should apply
    cat12Surch.amountNuc() = amountNuc / 2;
    CPPUNIT_ASSERT(_fu->surchargeData().size() == 1);
    CPPUNIT_ASSERT_NO_THROW(secSurchAppl.validate(_trx, "AA", _surchInfo, *_airSeg0, *_fu));
    CPPUNIT_ASSERT(_fu->surchargeData().size() == 2);

    _diag->flushMsg();
    std::string str = _trx.diagnostic().toString();
    // std::cout << str << std::endl;

    _fu->surchargeData().clear();
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(SecSurchargeApplTest);
} // tse
