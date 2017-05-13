#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Trx.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Server/TseServer.h"
#include "Common/TseEnums.h"
#include "DataModel/PaxType.h"
#include "Common/Vendor.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"
#include "Common/ErrorResponseException.h"
#include "Diagnostic/Diag804Collector.h"

#include "Common/TseCodeTypes.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include <unistd.h>

using namespace std;

namespace tse
{
class TaxCreateTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCreateTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testPercentageTax);
  CPPUNIT_TEST(testFixedTax);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testConstructor()
  {
    try { TaxCreateTest taxCreateTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testPercentageTax()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX.xml");

    trx.travelSeg().push_back(airSeg);

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    TaxResponse taxResponse;
    Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(airSeg);

    fare.nucFareAmount() = 1000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 1000.00;
    fareInfo._currency = "USD";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    fareUsage.travelSeg().push_back(airSeg);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml");

    Tax tax;

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);

    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_LAX_NRT.xml");

    farePath.itin()->travelSeg().push_back(airSeg2);

    fareMarket.travelSeg().push_back(airSeg2);

    fareUsage.travelSeg().push_back(airSeg2);

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);

    loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    request.ticketingAgent()->agentLocation() = loc;

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);
  }

  void testFixedTax()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegANC_LAX.xml");

    trx.travelSeg().push_back(airSeg);

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    TaxResponse taxResponse;
    Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(airSeg);

    fare.nucFareAmount() = 1000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 1000.00;
    fareInfo._currency = "USD";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    fareUsage.travelSeg().push_back(airSeg);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml");

    Tax tax;

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);

    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_LAX_NRT.xml");

    farePath.itin()->travelSeg().push_back(airSeg2);

    fareMarket.travelSeg().push_back(airSeg2);

    fareUsage.travelSeg().push_back(airSeg2);

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);

    loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    request.ticketingAgent()->agentLocation() = loc;

    tax.taxCreate(trx, taxResponse, *taxCodeReg2, travelSegStartIndex, travelSegEndIndex);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxCreateTest);
}
