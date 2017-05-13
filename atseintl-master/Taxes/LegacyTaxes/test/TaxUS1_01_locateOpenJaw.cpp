#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxUS1_01.h"
#include "DataModel/Agent.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DBAccess/TaxCodeReg.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"
#include <string>

namespace tse
{

class LocRestrictionValidatorMock : public LocRestrictionValidator3601
{
  bool fareBreaksFound() { return true; }
  void findFarthestPoint(PricingTrx& trx, const Itin& itin, const uint16_t& startIndex) {}
  uint16_t getFarthestSegIndex() { return 1; }
};

namespace
{

class TaxUS1_01Mock : public TaxUS1_01
{
protected:
  uint32_t _milesCount;

public:
  TaxUS1_01Mock() : TaxUS1_01(), _milesCount(6) {}

  uint32_t calculateMiles(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          std::vector<TravelSeg*>& travelSegs,
                          const Loc& origin,
                          const Loc& destination) override
  {
    --_milesCount;
    return _milesCount * 100;
  }
};

class TaxUS1_01Mock2 : public TaxUS1_01Mock
{
  uint32_t calculateMiles(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          std::vector<TravelSeg*>& travelSegs,
                          const Loc& origin,
                          const Loc& destination) override
  {
    ++_milesCount;
    return _milesCount * 100;
  }
};
}

class TaxUS1_01_locateOpenJaw : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUS1_01_locateOpenJaw);
  CPPUNIT_TEST(locateOpenJaw_shouldFailOnLocations);
  CPPUNIT_TEST(locateOpenJaw_shouldPass);
  CPPUNIT_TEST(locateOpenJaw_shouldFailOnMiles);
  CPPUNIT_TEST_SUITE_END();

  std::string xmlPath;
  Itin* itin;
  PricingTrx* trx;
  Agent* agent;
  PricingRequest* request;
  DateTime* dt;
  TaxResponse* response;
  FarePath* farePath;
  TaxCodeReg* taxCodeReg;
  PricingOptions* options;

public:
  void setUp()
  {
    itin = new Itin();
    trx = new PricingTrx();
    agent = new Agent();
    request = new PricingRequest();
    dt = new DateTime();
    response = new TaxResponse();
    farePath = new FarePath();
    taxCodeReg = new TaxCodeReg();
    options = new PricingOptions();

    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    agent->currencyCodeAgent() = "USD";
    trx->setRequest(request);
    trx->getRequest()->ticketingAgent() = agent;
    trx->getRequest()->ticketingDT() = dt->localTime();
    response->farePath() = farePath;
    farePath->itin() = itin;
    trx->setOptions(options);
    options->currencyOverride() = "";
  }

  void tearDown()
  {
    delete itin;
    delete trx;
    delete agent;
    delete request;
    delete dt;
    delete response;
    delete farePath;
    delete taxCodeReg;
    delete options;
  }

  void locateOpenJaw_shouldFailOnLocations()
  {
    TaxUS1_01Mock tax;
    itin->travelSeg().clear();
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    bool res = tax.locateOpenJaw(*trx, *response);
    CPPUNIT_ASSERT(!res);
  }

  void locateOpenJaw_shouldPass()
  {
    TaxUS1_01Mock tax;
    itin->travelSeg().clear();
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_0.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_1.xml"));
    bool res = tax.locateOpenJaw(*trx, *response);
    CPPUNIT_ASSERT(res);
  }

  void locateOpenJaw_shouldFailOnMiles()
  {
    TaxUS1_01Mock2 tax;
    itin->travelSeg().clear();
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_0.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_DFW_MEX_MIA_1.xml"));
    bool res = tax.locateOpenJaw(*trx, *response);
    CPPUNIT_ASSERT(!res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUS1_01_locateOpenJaw);
};
