
#include "Taxes/LegacyTaxes/TaxUS1_01.h"

#include <string>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTaxCodeRegFactory.h"
#include "test/testdata/TestClassOfServiceFactory.h"
#include "test/testdata/TestXMLHelper.h"

namespace tse
{
namespace
{
class TaxUS1_01Mock : public TaxUS1_01
{

  bool substractPartialFare(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            const MoneyAmount& totalFareAmount,
                            MoneyAmount& taxableAmount,
                            FareUsage& fareUsage,
                            const std::vector<TravelSeg*>& travelSegsEx) override
  {
    return true;
  }

  MoneyAmount calculatePercentage(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t startIndex,
                                  uint16_t endIndex) override
  {
    return MoneyAmount(0.075);
  }

  MoneyAmount calculatePartAmount(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  uint16_t startIndex,
                                  uint16_t endIndex,
                                  uint16_t fareBreakEnd,
                                  FareUsage& fareUsage) override
  {
    _fareStart = taxResponse.farePath()->itin()->segmentOrder(fareUsage.travelSeg().front()) - 1;
    ++_callCounter;
    return MoneyAmount(1000);
  }

public:
  void initializeMock()
  {
    _callCounter = 0;
    _fareStart = 99;
  }

  uint8_t _callCounter;
  uint8_t _fareStart;
};
}

class TaxUS1_01_taxCreate : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUS1_01_taxCreate);
  CPPUNIT_TEST(taxCreate_oneFareBreak);
  CPPUNIT_TEST(taxCreate_twoFareBreaks);
  CPPUNIT_TEST(taxCreate_overlapingFares);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
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
    _memHandle.create<TestConfigInitializer>();
    itin = _memHandle.create<Itin>();
    trx = _memHandle.create<PricingTrx>();
    agent = _memHandle.create<Agent>();
    request = _memHandle.create<PricingRequest>();
    dt = _memHandle.create<DateTime>();
    response = _memHandle.create<TaxResponse>();
    farePath = _memHandle.create<FarePath>();
    taxCodeReg = _memHandle.create<TaxCodeReg>();
    options = _memHandle.create<PricingOptions>();

    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    TestAirSegFactory::clearCache();
    itin->travelSeg().clear();
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin->travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    agent->currencyCodeAgent() = "USD";
    trx->setRequest(request);
    trx->getRequest()->ticketingAgent() = agent;
    trx->getRequest()->ticketingDT() = dt->localTime();
    response->farePath() = farePath;
    farePath->itin() = itin;
    trx->setOptions(options);
    options->currencyOverride() = "";
  }

  void taxCreate_oneFareBreak()
  {
    TaxUS1_01Mock tax;
    FareUsage fareUsage;
    fareUsage.travelSeg() = itin->travelSeg();
    tax._soldInUS = true;
    tax._fareBreaksFound = true;
    tax._latestEndIndex = 1;
    tax._fareBreaks.insert(std::pair<uint16_t, FareUsage*>(1, &fareUsage));
    tax.taxCreate(*trx, *response, *taxCodeReg, 0, 1);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(75), tax._taxAmount);
  }

  void taxCreate_twoFareBreaks()
  {
    TaxUS1_01Mock tax;
    FareUsage fareUsage1;
    fareUsage1.travelSeg().push_back(itin->travelSeg()[0]);
    FareUsage fareUsage2;
    fareUsage2.travelSeg().push_back(itin->travelSeg()[1]);
    tax._soldInUS = true;
    tax._fareBreaksFound = true;
    tax._latestEndIndex = 1;
    tax._fareBreaks.insert(std::pair<uint16_t, FareUsage*>(0, &fareUsage1));
    tax._fareBreaks.insert(std::pair<uint16_t, FareUsage*>(1, &fareUsage2));
    tax.taxCreate(*trx, *response, *taxCodeReg, 0, 1);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(150), tax._taxAmount);
  }

  void taxCreate_overlapingFares()
  {
    TaxUS1_01Mock tax;
    FareUsage fareUsage1;
    fareUsage1.travelSeg().push_back(itin->travelSeg()[1]);
    FareUsage fareUsage2;
    fareUsage2.travelSeg() = itin->travelSeg();
    tax._soldInUS = true;
    tax._fareBreaksFound = true;
    tax._latestEndIndex = 0;
    tax._fareBreaks.insert(std::pair<uint16_t, FareUsage*>(0, &fareUsage1));
    tax._fareBreaks.insert(std::pair<uint16_t, FareUsage*>(1, &fareUsage2));
    tax.initializeMock();
    tax.taxCreate(*trx, *response, *taxCodeReg, 0, 0);
    CPPUNIT_ASSERT_EQUAL(tax._callCounter, uint8_t(1));
    CPPUNIT_ASSERT_EQUAL(tax._fareStart, uint8_t(0));
  }

  void tearDown()
  {
    _memHandle.clear();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUS1_01_taxCreate);
}
