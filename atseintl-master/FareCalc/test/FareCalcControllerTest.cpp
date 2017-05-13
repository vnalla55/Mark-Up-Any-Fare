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

#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include "Diagnostic/Diag203Collector.h"
#include "Diagnostic/Diag853Collector.h"
#include "Diagnostic/Diag980Collector.h"

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "Server/TseServer.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/Trx.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseEnums.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"

#include "FareCalc/FareCalcController.h"
#include "FareCalc/FareCalcCollector.h"

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"

#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/FareCalcConfigText.h"

#include <unistd.h>

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  std::vector<FareTypeQualifier*> _ftq;
  FareCalcConfigText _fct;

public:
  const std::vector<FareTypeQualifier*>& getFareTypeQualifier(const Indicator& userApplType,
                                                              const UserApplCode& userAppl,
                                                              const FareType& qualifier)
  {
    return _ftq;
  }
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

class FareCalcControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCalcControllerTest);
  CPPUNIT_TEST(testDiag203Process);
  CPPUNIT_TEST(testDiag853Process);
  CPPUNIT_TEST(testDiag980Process);
  CPPUNIT_TEST(testDiag980ProcessTicketing);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown()
  {
    TestFactoryManager::instance()->destroyAll();
    _memHandle.clear();
  }

  void testDiag203Process()
  {
    // Create Base data and modify
    PricingTrx* trx = TestPricingTrxFactory::create("farecalcdata/trx.xml");
    trx->diagnostic().diagnosticType() = Diagnostic203;
    trx->diagnostic().activate();

    DateTime dt;
    trx->getRequest()->ticketingDT() = dt.localTime();
    trx->getOptions()->mOverride() = 'Y';

    // Update times to current time
    AirSeg* airSeg = (AirSeg*)trx->travelSeg().front();
    airSeg->departureDT() = dt.localTime();
    airSeg->arrivalDT() = dt.localTime();
    airSeg->bookingDT() = dt.localTime();

    // Set currencies to USD and NUC amount to 500
    Itin* itin = trx->itin().front();
    itin->originationCurrency() = "USD";
    itin->calculationCurrency() = "USD";
    itin->farePath().front()->setTotalNUCAmount(500.00);

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(*trx));
    diagCollector.activate();

    FareCalcController* fcc = _memHandle.insert(new FareCalcController(*trx));

    bool retCode = false;
    CPPUNIT_ASSERT_NO_THROW(retCode = fcc->process());
    CPPUNIT_ASSERT_EQUAL(true, retCode);
    // Verify at least part of the expected data is present in the diagnostic string
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("NO MATCH FOUND") != std::string::npos);
  }

  void testDiag853Process()
  {
    PricingTrx* trx = TestPricingTrxFactory::create("farecalcdata/trx.xml");
    trx->diagnostic().diagnosticType() = Diagnostic853;
    trx->diagnostic().activate();
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(*trx));
    diagCollector.activate();

    // Set Number of passengers to 2; only entry used.
    trx->paxType().front()->number() = 2;

    FareCalcController* fcc = _memHandle.insert(new FareCalcController(*trx));

    bool retCode = false;
    CPPUNIT_ASSERT_NO_THROW(retCode = fcc->process());
    CPPUNIT_ASSERT_EQUAL(true, retCode);
    // Verify at least part of the expected data is present in the diagnostic string
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("PSEUDO-CITY:") != std::string::npos);
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("NO DATA AVAILABLE") != std::string::npos);
  }

  void testDiag980Process()
  {
    PricingTrx* trx = generatePricingTrx();

    // Setup diagnostics
    trx->diagnostic().diagnosticType() = Diagnostic980;
    trx->diagnostic().activate();

    FareCalcController* fcc = _memHandle.insert(new FareCalcController(*trx));

    bool retCode = false;
    CPPUNIT_ASSERT_NO_THROW(retCode = fcc->process());
    CPPUNIT_ASSERT_EQUAL(true, retCode);
    // Verify at least part of the expected data is present in the diagnostic string
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("JOURNEY APPLIED") != std::string::npos);
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("AA") != std::string::npos);
  };

  void testDiag980ProcessTicketing()
  {
    PricingTrx* trx = generatePricingTrx();

    // Setup diagnostics
    trx->diagnostic().diagnosticType() = Diagnostic980;
    trx->diagnostic().activate();

    // Setup ticketing
    trx->getRequest()->ticketEntry() = true;
    trx->getOptions()->ticketStock() = 206;

    FareCalcController* fcc = _memHandle.insert(new FareCalcController(*trx));

    bool retCode = false;
    CPPUNIT_ASSERT_NO_THROW(retCode = fcc->process());
    CPPUNIT_ASSERT_EQUAL(true, retCode);
    // Verify at least part of the expected data is present in the diagnostic string
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("JOURNEY APPLIED") != std::string::npos);
    CPPUNIT_ASSERT(trx->diagnostic().toString().find("AA") != std::string::npos);
  }

protected:
  PricingTrx* generatePricingTrx()
  {
    PricingTrx* trx = TestPricingTrxFactory::create("farecalcdata/trx.xml");

    DateTime dt;
    trx->getRequest()->ticketingDT() = dt.localTime();
    trx->getOptions()->mOverride() = 'Y';

    // Update times to current time
    AirSeg* airSeg = (AirSeg*)trx->travelSeg().front();
    airSeg->departureDT() = dt.localTime();
    airSeg->arrivalDT() = dt.localTime();
    airSeg->bookingDT() = dt.localTime();

    // Set currencies to USD and NUC amount to 500
    Itin* itin = trx->itin().front();
    itin->originationCurrency() = "USD";
    itin->calculationCurrency() = "USD";
    itin->farePath().front()->setTotalNUCAmount(500.00);

    Agent* pAgent = _memHandle.insert(new Agent);
    Billing* pBilling = _memHandle.insert(new Billing);

    trx->getRequest()->ticketingAgent() = pAgent;
    trx->billing() = pBilling;

    return trx;
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCalcControllerTest);
}
