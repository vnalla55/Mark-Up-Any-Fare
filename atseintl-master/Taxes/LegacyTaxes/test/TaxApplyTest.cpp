#include <unistd.h>
#include <time.h>

#include <string>
#include <iostream>
#include <vector>

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Response.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionTransit.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestXMLHelper.h"

using namespace std;

namespace tse
{

class TaxApplyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxApplyTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_SKIP_TEST(testTaxApply);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { TaxApply taxApply; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxApply()
  {

    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;

    //   trx.requestType() = TAX_DISPLAY;

    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJFK_DFW.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_ORD.xml");

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(500.00);

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);
    farePath.itin()->travelSeg().push_back(airSeg2);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    Diagnostic* diagroot = new Diagnostic(LegacyTaxDiagnostic24);
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg2);

    fare.nucFareAmount() = 1000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 1000.00;
    fareInfo._currency = "USD";
    fareInfo._fareTariff = 0;
    fareInfo._fareClass = "AA";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariff = 0;

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    PaxType paxType;
    paxType.paxType() = std::string("ADT");
    PaxTypeInfo pti;
    paxType.paxTypeInfo() = &pti;
    itin.farePath()[0]->paxType() = &paxType;

    paxTypeFare.paxType() = &paxType;

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);

    fareUsage.travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    TaxApply taxApply;

    taxApply.applyTax(trx, taxResponse, taxMap, *taxCodeReg, nullptr);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegYYC_DFW_AA.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_SQ.xml");

    taxApply.applyTax(trx, taxResponse, taxMap, *taxCodeReg, nullptr);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxApplyTest);
}
