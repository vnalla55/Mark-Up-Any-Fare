#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxSP60.h"
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
class TaxSP60Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP60Test);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxSP60);
  CPPUNIT_TEST_SUITE_END();

public:
  void testConstructor()
  {
    try { TaxSP60Test taxOrchestratorTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxSP60()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    //   trx.requestType() = TAX_DISPLAY;
    trx.getRequest()->ticketingDT() = time(0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("KUL");
    origin.nation() = std::string("MY");

    destination.loc() = std::string("HGK");
    destination.nation() = std::string("HG");
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
    taxCodeReg.loc1() = tse::LocCode("MY");

    taxCodeReg.loc1ExclInd() = tse::Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = tse::LocCode("MY");

    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);

    TaxSP60 taxSP60;

    taxSP60.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    Loc origin2;
    Loc destination2;

    origin2.loc() = std::string("HGK");
    origin2.nation() = std::string("HG");

    destination2.loc() = std::string("KUL");
    destination2.nation() = std::string("MY");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination2;

    AirSeg travelSeg2;

    travelSeg2.origin() = &origin2;
    travelSeg2.destination() = &destination2;
    travelSeg2.departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(&travelSeg2);

    Loc origin3;
    Loc destination3;

    origin3.loc() = std::string("KUL");
    origin3.nation() = std::string("MY");

    destination3.loc() = std::string("BKI");
    destination3.nation() = std::string("MY");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination3;

    AirSeg travelSeg3;

    travelSeg3.origin() = &origin3;
    travelSeg3.destination() = &destination3;
    travelSeg3.departureDT() = time(0);

    farePath.itin()->travelSeg().push_back(&travelSeg3);

    travelSegIndex = 2;

    taxSP60.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);

    Loc origin4;
    Loc destination4;

    origin4.loc() = std::string("BKI");
    origin4.nation() = std::string("MY");

    destination4.loc() = std::string("NRT");
    destination4.nation() = std::string("JP");
    trx.getRequest()->ticketingAgent()->agentLocation() = &destination4;

    AirSeg travelSeg4;

    travelSeg4.origin() = &origin4;
    travelSeg4.destination() = &destination4;
    travelSeg4.departureDT() = time(0);

    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    travelSegIndex = 1;

    farePath.itin()->travelSeg().push_back(&travelSeg4);

    taxSP60.validateTransit(trx, taxResponse, taxCodeReg, travelSegIndex);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP60Test);
}
