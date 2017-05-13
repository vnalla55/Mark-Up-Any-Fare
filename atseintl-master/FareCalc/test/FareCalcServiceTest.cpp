//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#include "Server/TseServer.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/MetricsTrx.h"
#include "FareCalc/FareCalcService.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/FarePath.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag853Collector.h"
#include "DataModel/Trx.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Server/TseServer.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "Diagnostic/Diag854Collector.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Agent.h"
#include "DataModel/Itin.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"

#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestPricingRequestFactory.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/FareCalcConfigText.h"
#include "test/include/MockTseServer.h"

#include "FareCalc/CalcTotals.h"
//***

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  FareCalcConfigText _fct;

public:
  const FareCalcConfigText& getMsgText(const Indicator userApplType,
                                       const UserApplCode& userAppl,
                                       const PseudoCityCode& pseudoCity)
  {
    _fct.fccTextMap().insert(
        std::make_pair(static_cast<FareCalcConfigText::TextAppl>(1), "*RO RBD TICKETING OVERRIDE"));
    _fct.fccTextMap().insert(std::make_pair(static_cast<FareCalcConfigText::TextAppl>(2),
                                            "*NO COMBINABLE FARES FOR CLASS USED"));
    _fct.fccTextMap().insert(
        std::make_pair(static_cast<FareCalcConfigText::TextAppl>(3), "VERIFY BOOKING CLASS"));
    _fct.fccTextMap().insert(std::make_pair(static_cast<FareCalcConfigText::TextAppl>(4),
                                            "REBOOK OPTION OF CHOICE BEFORE STORING FARE"));
    _fct.fccTextMap().insert(
        std::make_pair(static_cast<FareCalcConfigText::TextAppl>(5), "APPLICABLE BOOKING CLASS"));
    return _fct;
  }
};
}

class FareCalcServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCalcServiceTest);
  CPPUNIT_TEST(testInit);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  MockTseServer* _mockServer;

public:
  void testInit()
  {
    FareCalcService fareCalcService("FARECALC_SVC", *_mockServer);
    CPPUNIT_ASSERT_NO_THROW(fareCalcService.initialize());
  }

  // This is just a basic test ... most logic should be validated
  // in each sub class.
  void testProcess()
  {
    MyDataHandle mdh;
    FareCalcService fareCalcService("FARECALC_SVC", *_mockServer);
    fareCalcService.initialize();

    Diagnostic diagroot(Diagnostic854);
    diagroot.activate();
    Diag854Collector diag(diagroot);
    diag.enable(Diagnostic854);
    CPPUNIT_ASSERT(diag.isActive());

    PricingTrx* trx = TestPricingTrxFactory::create("farecalcdata/trx2.xml");
    trx->setRequest(TestPricingRequestFactory::create("farecalcdata/pricingRequest.xml", true));
    // Must set the trx type manually
    trx->setTrxType(PricingTrx::MIP_TRX);
    trx->diagnostic().diagnosticType() = Diagnostic854;
    trx->diagnostic().activate();

    Agent* pAgent = _memHandle.insert(new Agent);
    Billing* pBilling = _memHandle.insert(new Billing);

    trx->getRequest()->ticketingAgent() = pAgent;
    trx->billing() = pBilling;

    // Set Number of passengers to 2; only entry used.
    trx->paxType().front()->number() = 2;

    bool retCode;
    CPPUNIT_ASSERT_NO_THROW(retCode = fareCalcService.process(*trx));
    CPPUNIT_ASSERT_EQUAL(true, retCode);
  }

  //-----------------------------------------------------------------
  // setUp()
  //-----------------------------------------------------------------
  void setUp()
  {
    _mockServer = _memHandle.create<MockTseServer>();
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCalcServiceTest);
} // tse
