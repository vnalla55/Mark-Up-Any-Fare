#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxAY.h"
#include "Taxes/LegacyTaxes/TaxSP18.h"
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

using namespace std;

namespace tse
{
class TaxAYTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxAYTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxAY);
  CPPUNIT_TEST(testHasTransfer);
  CPPUNIT_TEST(testHasTransferWhenForcedConx);
  CPPUNIT_TEST(testHasTransferWhenForcedStopOver);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TaxAYTest taxAYTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxAY()
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

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_AY.xml");

    TaxAY taxAY;

    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    bool result = taxAY.validateTripTypes(trx, taxResponse, *taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT(result == false);

    airSeg->hiddenStops().push_back(loc);

    bool result2 = taxAY.validateTripTypes(trx, taxResponse, *taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT(result2 == false);

    AirSeg* airSeg2 =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml");

    trx.travelSeg().push_back(airSeg2);
    farePath.itin()->travelSeg().push_back(airSeg2);
    airSeg2->hiddenStops().push_back(loc);
    startIndex = 1;

    bool result3 = taxAY.validateTripTypes(trx, taxResponse, *taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT(result3 == true);

    airSeg2->hiddenStops().clear();

    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    airSeg2->hiddenStops().push_back(loc2);

    bool result4 = taxAY.validateTripTypes(trx, taxResponse, *taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT(result4 == false);
  }

  void testHasTransfer()
  {
    GeoTravelType geoTravelType;
    std::vector<TravelSeg*> ts;
    ts.push_back(_memHandle.create<AirSeg>());
    ts.push_back(_memHandle.create<AirSeg>());
    std::vector<TravelSeg*>::iterator itsc = ts.begin();
    std::vector<TravelSeg*>::iterator itsn = itsc + 1;

    TaxSP18 tax;

    CPPUNIT_ASSERT(tax.hasTransfer(itsc, itsn, ts.end(), geoTravelType));
  }

  void testHasTransferWhenForcedConx()
  {
    GeoTravelType geoTravelType;
    std::vector<TravelSeg*> ts;
    ts.push_back(_memHandle.create<AirSeg>());
    ts.push_back(_memHandle.create<AirSeg>());
    std::vector<TravelSeg*>::iterator itsc = ts.begin();
    std::vector<TravelSeg*>::iterator itsn = itsc + 1;

    TaxSP18 tax;

    (*itsc)->forcedConx() = 'T';
    CPPUNIT_ASSERT(tax.hasTransfer(itsc, itsn, ts.end(), geoTravelType));
  }

  void testHasTransferWhenForcedStopOver()
  {
    GeoTravelType geoTravelType;
    std::vector<TravelSeg*> ts;
    ts.push_back(_memHandle.create<AirSeg>());
    ts.push_back(_memHandle.create<AirSeg>());
    std::vector<TravelSeg*>::iterator itsc = ts.begin();
    std::vector<TravelSeg*>::iterator itsn = itsc + 1;

    TaxSP18 tax;

    (*itsc)->forcedConx() = 'F';
    (*itsc)->forcedStopOver() = 'T';
    CPPUNIT_ASSERT(!tax.hasTransfer(itsc, itsn, ts.end(), geoTravelType));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxAYTest);
}
