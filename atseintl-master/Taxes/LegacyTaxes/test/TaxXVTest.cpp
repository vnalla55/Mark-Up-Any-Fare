#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Server/TseServer.h"
#include "Taxes/LegacyTaxes/TaxXV.h"
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include "test/include/MockTseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include <unistd.h>

using namespace std;

namespace tse
{
class TaxXVTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxXVTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxXVcase0);
  CPPUNIT_TEST(testTaxXVcase1);
  CPPUNIT_TEST(testTaxXVcase2);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { TaxXVTest taxXVtest; }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void testTaxXVcase0()
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

    AirSeg travelSeg;

    travelSeg.origin() = &origin;
    travelSeg.destination() = &destination;
    travelSeg.departureDT() = time(0);

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegIndex = 0;

    TaxResponse taxResponse;
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    taxResponse.farePath() = &farePath;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("CA");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y');

    taxCodeReg.loc2() = tse::LocCode("US");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N');

    TaxXV taxXV;

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    //   CPPUNIT_ASSERT( rc == false );

    origin.loc() = std::string("MEX");
    origin.nation() = std::string("MX");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    //   CPPUNIT_ASSERT( rc == false );

    origin.loc() = std::string("LHR");
    origin.nation() = std::string("GB");

    destination.loc() = std::string("DFW");
    destination.nation() = std::string("US");

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
  }

  void testTaxXVcase1()
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

    AirSeg travelSeg;

    travelSeg.origin() = &origin;
    travelSeg.destination() = &destination;
    travelSeg.departureDT() = time(0);

    Itin itin;
    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(&travelSeg);

    itin.farePath().push_back(&farePath);

    uint16_t travelSegIndex = 0;

    TaxResponse taxResponse;
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    taxResponse.farePath() = &farePath;

    Loc origin2;
    Loc destination2;

    origin2.loc() = std::string("MEX");
    origin2.nation() = std::string("MX");

    destination2.loc() = std::string("LAX");
    destination2.nation() = std::string("US");

    AirSeg travelSeg2;

    travelSeg2.origin() = &origin2;
    travelSeg2.destination() = &destination2;
    travelSeg2.departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(&travelSeg2);

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("CA");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y');

    taxCodeReg.loc2() = tse::LocCode("US");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N');

    travelSegIndex = 1;

    TaxXV taxXV;

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    //   CPPUNIT_ASSERT( rc == false );

    origin.loc() = std::string("ACA");
    origin.nation() = std::string("MX");

    origin2.loc() = std::string("MEX");
    origin2.nation() = std::string("MX");

    destination2.loc() = std::string("CUN");
    destination2.nation() = std::string("MX");

    Loc origin3;
    Loc destination3;

    origin3.loc() = std::string("CUN");
    origin3.nation() = std::string("MX");

    destination3.loc() = std::string("MEX");
    destination3.nation() = std::string("MX");

    AirSeg travelSeg3;

    travelSeg3.origin() = &origin3;
    travelSeg3.destination() = &destination3;
    travelSeg3.departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(&travelSeg3);

    travelSegIndex = 2;

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    //   CPPUNIT_ASSERT( rc == false );

    origin3.loc() = std::string("CUN");
    origin3.nation() = std::string("MX");

    destination3.loc() = std::string("ACA");
    destination3.nation() = std::string("MX");

    taxXV.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    //   CPPUNIT_ASSERT( rc == false );
  }

  void testTaxXVcase2()
  {

    tse::PricingTrx trx;
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    Loc origin;
    origin.loc() = string("MEX");
    origin.nation() = string("MX");

    Loc destination;
    destination.loc() = string("CUN");
    destination.nation() = string("MX");

    AirSeg ts;
    ts.origin() = &origin;
    ts.destination() = &destination;

    Itin itin;
    itin.travelSeg().push_back(&ts);

    FarePath fp;
    fp.itin() = &itin; // Set the itinerary in the fare path.

    tse::TaxResponse taxResponse;
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

    taxResponse.farePath() = &fp;

    tse::TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = tse::LocType('N');
    taxCodeReg.loc2Type() = tse::LocType('N');
    taxCodeReg.loc1() = tse::LocCode("PE");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = tse::LocCode("PE");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

    taxCodeReg.tripType() = 'F';

    TaxXV taxXV;

    taxXV.validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    //   CPPUNIT_ASSERT( result == false );

    taxCodeReg.loc2ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

    taxXV.validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    //   CPPUNIT_ASSERT( result == false );
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxXVTest);
}
