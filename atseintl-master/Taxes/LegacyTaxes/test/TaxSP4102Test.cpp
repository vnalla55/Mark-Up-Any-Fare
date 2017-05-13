#include "DataModel/AirSeg.h"
#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxSP4102.h"
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

class TaxSP4102Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP4102Test);
  CPPUNIT_TEST(checkIfTravelSegSpecialTaxEndIndexIsCorrectTest);
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
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    itin = new Itin();
    trx = new PricingTrx();
    agent = new Agent();
    request = new PricingRequest();
    dt = new DateTime();
    response = new TaxResponse();
    farePath = new FarePath();
    taxCodeReg = new TaxCodeReg();
    options = new PricingOptions();
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

  void checkIfTravelSegSpecialTaxEndIndexIsCorrectTest()
  {
    // TaxSP4101 tax; //should fail if 4101 is used
    TaxSP4102 tax;
    itin->travelSeg().clear();
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_KUL_SYD_KUL_SIN_0.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_KUL_SYD_KUL_SIN_1.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_KUL_SYD_KUL_SIN_2.xml"));
    tax.loc2ExcludeTransit(*trx, *response, *taxCodeReg, 2);
    tax.loc2ExcludeTransit(*trx, *response, *taxCodeReg, 0);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), tax._travelSegSpecialTaxEndIndex);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP4102Test);
};
