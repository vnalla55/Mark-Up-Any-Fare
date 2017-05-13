#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/TaxDL.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

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
#include "Diagnostic/Diag804Collector.h"

#include "Common/TseCodeTypes.h"

#include "test/include/MockTseServer.h"

#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

using namespace std;

namespace tse
{

class TaxDLTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDLTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxDL);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxDLTest pfcAbsorptionTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxDL()
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

    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBUE.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegCOR_AEP.xml");
    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_AEP_USH.xml");

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.setTotalNUCAmount(500.00);

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);
    farePath.itin()->travelSeg().push_back(airSeg2);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegStartIndex = 0;
    uint16_t travelSegEndIndex = 0;

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
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

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);

    fareUsage.travelSeg().push_back(airSeg);
    fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_DL.xml");

    TaxDL taxDL;

    taxDL.validateTripTypes(trx, taxResponse, *taxCodeReg, travelSegStartIndex, travelSegEndIndex);
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDLTest);
}
