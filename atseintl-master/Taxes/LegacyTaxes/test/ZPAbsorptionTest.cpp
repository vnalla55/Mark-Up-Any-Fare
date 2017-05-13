#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/ZPAbsorption.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/Tax.h"

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
#include "Server/TseServer.h"
#include "Common/TseEnums.h"
#include "DataModel/PaxType.h"
#include "Common/Vendor.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"
#include "Common/ErrorResponseException.h"

#include "Common/TseCodeTypes.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  std::vector<TaxSegAbsorb*> _ret;

public:
  const std::vector<TaxSegAbsorb*>&
  getTaxSegAbsorb(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "F9" || carrier == "AQ" || carrier == "HP")
      return _ret;
    return DataHandleMock::getTaxSegAbsorb(carrier, date);
  }
};
}
class ZPAbsorptionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ZPAbsorptionTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testZPAbsorption);
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
    try { ZPAbsorptionTest pfcAbsorptionTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testZPAbsorption()
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
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegWRL_F9.xml");
    //   AirSeg* airSeg2 =
    // TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_NRT_LAX.xml");

    Itin itin;

    trx.itin().push_back(&itin);
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    FarePath farePath;

    farePath.setTotalNUCAmount(500.00);

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);
    //   farePath.itin()->travelSeg().push_back(airSeg2);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    taxResponse.farePath() = &farePath;

    PricingUnit pricingUnit;
    FareUsage fareUsage;
    Fare fare;
    PaxTypeFare paxTypeFare;

    FareMarket fareMarket;
    fareMarket.travelSeg().push_back(airSeg);
    //   fareMarket.travelSeg().push_back(airSeg2);

    fare.nucFareAmount() = 1000.00;

    FareInfo fareInfo;
    fareInfo._fareAmount = 1000.00;
    fareInfo._currency = "USD";
    fareInfo._fareTariff = 0;
    fareInfo._fareClass = "AA";

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariff = 0;

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);

    fareUsage.travelSeg().push_back(airSeg);
    //   fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US1.xml");

    TaxItem taxItem;

    Tax tax;

    taxItem.buildTaxItem(trx, tax, taxResponse, *taxCodeReg);

    taxItem.taxAmount() = 100.00;

    taxResponse.taxItemVector().push_back(&taxItem);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_ZP.xml");

    TaxMap taxMap(trx.dataHandle());

    taxMap.initialize();

    Tax& tax2 = *taxMap.getSpecialTax(taxCodeReg2->specialProcessNo());

    TaxItem taxItem2;

    taxItem2.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem2.taxAmount() = 40.00;

    taxResponse.taxItemVector().push_back(&taxItem2);

    TaxItem taxItem3;

    taxItem3.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem3.taxAmount() = 20.00;

    taxResponse.taxItemVector().push_back(&taxItem3);

    TaxItem taxItem4;

    taxItem4.buildTaxItem(trx, tax2, taxResponse, *taxCodeReg2);

    taxItem4.taxAmount() = 10.00;

    taxResponse.taxItemVector().push_back(&taxItem4);

    ZPAbsorption ZPAbsorption;

    ZPAbsorption.applyZPAbsorption(trx, taxResponse);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegHNL_MKK_AQ.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    fareInfo._fareTariff = 773;
    fareInfo._fareClass = "YAQAP";

    ZPAbsorption.applyZPAbsorption(trx, taxResponse);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegAMA_DFW_F9.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    fareInfo._fareTariff = 0;
    fareInfo._fareClass = "";

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    ZPAbsorption.applyZPAbsorption(trx, taxResponse);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LAX_HP.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);

    fareInfo._fareTariff = 745;
    fareInfo._ruleNumber = "3600";

    ZPAbsorption.applyZPAbsorption(trx, taxResponse);

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegMKE_X_PHX_HP.xml");
    AirSeg* airSegX;
    airSegX = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegPHX_X_ONT_HP.xml");

    farePath.itin()->travelSeg().clear();
    fareUsage.travelSeg().clear();
    fareMarket.travelSeg().clear();

    farePath.itin()->travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg);
    fareMarket.travelSeg().push_back(airSeg);
    farePath.itin()->travelSeg().push_back(airSegX);
    fareUsage.travelSeg().push_back(airSegX);
    fareMarket.travelSeg().push_back(airSegX);

    fareInfo._fareTariff = 0;
    fareInfo._ruleNumber = "";

    ZPAbsorption.applyZPAbsorption(trx, taxResponse);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(ZPAbsorptionTest);
}
