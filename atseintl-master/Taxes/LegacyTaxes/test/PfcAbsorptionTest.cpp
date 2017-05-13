#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/PfcAbsorption.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diagnostic.h"
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
#include <unistd.h>
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<PfcAbsorb*>&
  getPfcAbsorb(const LocCode& pfcAirport, const CarrierCode& localCarrier, const DateTime& date)
  {
    if (localCarrier == "AA")
      return *_memHandle.create<std::vector<PfcAbsorb*> >();
    return DataHandleMock::getPfcAbsorb(pfcAirport, localCarrier, date);
  }
};
}
class PfcAbsorptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PfcAbsorptionTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testPfcAbsorption);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _memHandle.create<TestConfigInitializer>();
  }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { PfcAbsorptionTest pfcAbsorptionTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testPfcAbsorption()
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
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegJFK_DFW.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_X_HNL.xml");

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

    // uint16_t travelSegStartIndex = 0;
    // uint16_t travelSegEndIndex = 0;

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    diagroot->activate();
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
    fareInfo._fareTariff = 85;
    fareInfo._ruleNumber = "9000";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    fareUsage.travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml");

    CurrencyCode absorbCurrency = "USD";
    MoneyAmount absorbAmount = 4.50;
    uint8_t locType = 1;

    TaxItem taxItem;

    Tax tax;

    taxItem.buildTaxItem(trx, tax, taxResponse, *taxCodeReg2);

    taxItem.taxAmount() = 100.00;

    taxResponse.taxItemVector().push_back(&taxItem);

    PfcAbsorption pfcAbsorption;

    pfcAbsorption.pfcAbsorptionApplied(
        trx, taxResponse, *airSeg2, absorbCurrency, absorbAmount, locType);

    locType = 4;
    pfcAbsorption.pfcAbsorptionApplied(
        trx, taxResponse, *airSeg2, absorbCurrency, absorbAmount, locType);

    //   AirSeg* airSeg =
    // TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml");

    //   TaxCodeReg* taxCodeReg2 =
    // TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeXV.xml");
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PfcAbsorptionTest);
}
