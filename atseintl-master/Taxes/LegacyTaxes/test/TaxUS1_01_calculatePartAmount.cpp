#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
#include "test/include/TestFallbackUtil.h"

namespace tse
{

namespace
{
class TaxUS1_01Mock : public TaxUS1_01
{

  uint32_t _milesCount;

public:
  TaxUS1_01Mock() : TaxUS1_01(), _milesCount(6) {}

  bool substractPartialFare(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg,
                            const MoneyAmount& totalFareAmount,
                            MoneyAmount& taxableAmount,
                            FareUsage& fareUsage,
                            const std::vector<TravelSeg*>& travelSegsEx) override
  {
    taxableAmount -= 25;
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

  uint32_t calculateMiles(PricingTrx& trx,
                          TaxResponse& taxResponse,
                          std::vector<TravelSeg*>& travelSegs,
                          const Loc& origin,
                          const Loc& destination) override
  {
    ++_milesCount;
    return _milesCount * 100;
  }

  virtual void convertCurrency(MoneyAmount& taxableAmount,
                               PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg) override
  {
  }
};
}

class TaxUS1_01_calculatePartAmountTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUS1_01_calculatePartAmountTest);
  CPPUNIT_TEST(calculatePartAmountTest_wholeFare);
  CPPUNIT_TEST(calculatePartAmountTest_partFare);
  CPPUNIT_TEST(calculatePartAmountTest_wholeFare3segs);
  CPPUNIT_TEST(calculatePartAmountTest_partFare3segs);
  CPPUNIT_TEST(calculatePartAmountTest_partFareMiles);
  CPPUNIT_TEST_SUITE_END();

  std::string xmlPath;
  Itin itin;
  PricingTrx trx;
  Agent agent;
  PricingRequest request;
  DateTime dt;
  TaxResponse response;
  FarePath farePath;
  TaxCodeReg taxCodeReg;
  PricingOptions options;
  FareUsage fareUsage;
  PaxTypeFare paxTypeFare;

public:
  void setUp()
  {
    xmlPath = "/vobs/atseintl/Taxes/LegacyTaxes/test/testdata/";
    agent.currencyCodeAgent() = "USD";
    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.getRequest()->ticketingDT() = dt.localTime();
    response.farePath() = &farePath;
    farePath.itin() = &itin;
    trx.setOptions(&options);
    options.currencyOverride() = "";
    fareUsage.paxTypeFare() = &paxTypeFare;
    if (paxTypeFare.fare())
      paxTypeFare.fare()->nucFareAmount() = 0;
    fareUsage.surchargeAmt() = 100;
  }

  void calculatePartAmountTest_wholeFare()
  {
    TaxUS1_01Mock tax;
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    fareUsage.travelSeg() = itin.travelSeg();
    MoneyAmount res = tax.calculatePartAmount(trx, response, taxCodeReg, 0, 1, 1, fareUsage);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(100), res);
  }

  void calculatePartAmountTest_partFare()
  {
    TaxUS1_01Mock tax;
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_YYC_DFW_YYC_1.xml"));
    fareUsage.travelSeg() = itin.travelSeg();
    MoneyAmount res = tax.calculatePartAmount(trx, response, taxCodeReg, 0, 0, 1, fareUsage);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(50), res);
  }

  void calculatePartAmountTest_partFare3segs()
  {
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    TaxUS1_01Mock tax;
    fareUsage.travelSeg() = itin.travelSeg();
    MoneyAmount res = tax.calculatePartAmount(trx, response, taxCodeReg, 2, 2, 2, fareUsage);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(50), res);
  }

  void calculatePartAmountTest_wholeFare3segs()
  {
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    Itin partItin;
    partItin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    TaxUS1_01Mock tax;
    fareUsage.travelSeg() = partItin.travelSeg();
    MoneyAmount res = tax.calculatePartAmount(trx, response, taxCodeReg, 2, 2, 2, fareUsage);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(100), res);
  }

  void calculatePartAmountTest_partFareMiles()
  {
    itin.travelSeg().clear();
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_0.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_1.xml"));
    itin.travelSeg().push_back(TestAirSegFactory::create(xmlPath + "0_FRA_CDG_DFW_ORD_2.xml"));
    TaxUS1_01Mock tax;
    fareUsage.travelSeg() = itin.travelSeg();
    MoneyAmount res = tax.calculatePartAmount(trx, response, taxCodeReg, 1, 1, 2, fareUsage);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(87.5), res);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxUS1_01_calculatePartAmountTest);
};
