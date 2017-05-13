#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/TaxSP17.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"

#include "DataModel/PricingRequest.h"
#include "DBAccess/Loc.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxType.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "DataModel/Itin.h"
#include "DataModel/FarePath.h"
#include "DataModel/Response.h"
#include "DataModel/Itin.h"
#include "DataModel/Agent.h"
#include "Diagnostic/Diag804Collector.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include <unistd.h>

using namespace std;

namespace tse
{
class TaxYCTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxYCTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxYCcase0);
  CPPUNIT_TEST(testTaxYCcase1);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }
  void testConstructor()
  {
    try { TaxYCTest taxYCtest; }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testTaxYCcase0()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = time(0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("YYZ");
    origin.nation() = std::string("CA");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination;

    TravelSeg* travelSeg = _memHandle.create<AirSeg>();

    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;
    travelSeg->departureDT() = time(0);

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(travelSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegIndex = 0;

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("CA");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y');

    taxCodeReg.loc2() = tse::LocCode("US");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N');

    return; // Tax No longer special process.

    TaxSP17 taxYC;

    bool rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT(rc == false);

    origin.loc() = std::string("MEX");
    origin.nation() = std::string("MX");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");

    rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT(rc == false);

    origin.loc() = std::string("LHR");
    origin.nation() = std::string("GB");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");

    rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    taxCodeReg.taxCode() = "XA";

    Loc origin2;
    Loc destination2;

    origin2.loc() = std::string("YYZ");
    origin2.nation() = std::string("CA");

    destination2.loc() = std::string("DFW");
    destination2.nation() = std::string("US");

    TravelSeg* travelSeg2 = _memHandle.create<AirSeg>();

    travelSeg2->origin() = &origin2;
    travelSeg2->destination() = &destination2;
    travelSeg2->departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(travelSeg2);

    travelSegIndex = 1;

    rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
  }

  void testTaxYCcase1()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = time(0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("LHR");
    origin.nation() = std::string("GB");

    destination.loc() = std::string("MEX");
    destination.nation() = std::string("MX");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination;

    TravelSeg* travelSeg = _memHandle.create<AirSeg>();

    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;
    travelSeg->departureDT() = time(0);

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(travelSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegIndex = 0;

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;

    taxResponse.farePath() = &farePath;

    Loc origin2;
    Loc destination2;

    origin2.loc() = std::string("MEX");
    origin2.nation() = std::string("MX");

    destination2.loc() = std::string("LAX");
    destination2.nation() = std::string("US");

    TravelSeg* travelSeg2 = _memHandle.create<AirSeg>();

    travelSeg2->origin() = &origin2;
    travelSeg2->destination() = &destination2;
    travelSeg2->departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(travelSeg2);

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("CA");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y');

    taxCodeReg.loc2() = tse::LocCode("US");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N');

    return; // Tax No longer special process.

    TaxSP17 taxYC;

    bool rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT(rc == true);

    origin2.loc() = std::string("MEX");
    origin2.nation() = std::string("MX");

    destination2.loc() = std::string("ACA");
    destination2.nation() = std::string("MX");

    Loc origin3;
    Loc destination3;

    origin3.loc() = std::string("MEX");
    origin3.nation() = std::string("MX");

    destination3.loc() = std::string("LAX");
    destination3.nation() = std::string("US");

    TravelSeg* travelSeg3 = _memHandle.create<AirSeg>();

    travelSeg3->origin() = &origin3;
    travelSeg3->destination() = &destination3;
    travelSeg3->departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(travelSeg3);

    rc = taxYC.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT(rc == true);
  }
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxYCTest);
}
