#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Taxes/LegacyTaxes/TaxUS2.h"
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
#include "Common/TsePrimitiveTypes.h"
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

#include <unistd.h>
#include "Common/CurrencyConversionFacade.h"
#include "Common/BSRCollectionResults.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/TaxNation.h"
#include <boost/assign/std/vector.hpp>
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/TaxSpecConfigReg.h"

#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
using namespace boost::assign;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }
  const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date)
  {
    if (nation == "US")
    {
      TaxNation* ret = _memHandle.create<TaxNation>();
      ret->roundingRule() = NEAREST;
      ret->taxRoundingOverrideInd() = 'E';
      ret->taxCollectionInd() = 'A';
      ret->taxFarequoteInd() = 'I';
      ret->taxCodeOrder() += "US1", "ZP", "US2", "YC", "XY", "XA", "AY", "YZ1";
      return ret;
    }
    else if (nation == "GB")
    {
      TaxNation* ret = _memHandle.create<TaxNation>();
      ret->roundingRule() = NEAREST;
      ret->taxRoundingOverrideInd() = ' ';
      ret->taxCollectionInd() = 'A';
      ret->taxFarequoteInd() = 'N';
      ret->taxCodeOrder() += "GB5", "GB6", "YO5", "YO6", "YO1", "YO2", "YO3", "YO4", "UB1", "UB8",
          "UB2", "UB7", "UB3", "UB4", "UB6", "UB9", "YZ2";
      return ret;
    }
    return DataHandleMock::getTaxNation(nation, date);
  }
  const std::vector<BankerSellRate*>&
  getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
  {
    if (primeCur == "EUR" && cur == "PLN")
    {
      std::vector<BankerSellRate*>& ret = *_memHandle.create<std::vector<BankerSellRate*> >();
      ret.push_back(_memHandle.create<BankerSellRate>());
      ret.front()->rate() = 4.0404;
      ret.front()->rateType() = 'B';
      ret.front()->agentSine() = "FXR";
      return ret;
    }
    else if (primeCur == "EUR" && cur == "USD")
    {
      std::vector<BankerSellRate*>& ret = *_memHandle.create<std::vector<BankerSellRate*> >();
      ret.push_back(_memHandle.create<BankerSellRate>());
      ret.front()->rate() = 1.238;
      ret.front()->rateType() = 'B';
      ret.front()->agentSine() = "FXR";
      return ret;
    }

    return DataHandleMock::getBankerSellRate(primeCur, cur, date);
  }

  std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
  {
    std::vector<TaxSpecConfigReg*>& ret = *_memHandle.create<std::vector<TaxSpecConfigReg*> >();
    ret.push_back(_memHandle.create<TaxSpecConfigReg>());
    TaxSpecConfigReg::TaxSpecConfigRegSeq* seq = new TaxSpecConfigReg::TaxSpecConfigRegSeq();
    ret.front()->taxSpecConfigName() = name;
    seq->paramName() = "HALFTAXROUND";
    if (name == "TEST_DOWN")
      seq->paramValue() = "DOWN";
    else if (name == "TEST_UP")
      seq->paramValue() = "UP";
    else
      seq->paramValue() = "NONE";
    ret.front()->seqs().push_back(seq);
    return ret;
  }
};
}
class TaxUS2Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxUS2Test);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTaxUS2);
  CPPUNIT_TEST(testCalculateHalfTaxAmountRoundUP);
  CPPUNIT_TEST(testCalculateHalfTaxAmountRoundDOWN);
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
    try { TaxUS2Test taxOrchestratorTest; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testTaxUS2()
  {
    PricingTrx trx;

    Agent agent;
    agent.currencyCodeAgent() = "USD";

    PricingRequest request;
    trx.setRequest(&request);

    trx.getRequest()->ticketingAgent() = &agent;
    DateTime dt;

    trx.getRequest()->ticketingDT() = dt.localTime();
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    request.ticketingAgent()->agentLocation() = loc;

    PricingOptions options;
    trx.setOptions(&options);

    AirSeg* airSeg =
        TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegLAX_HNL.xml");
    //   AirSeg* airSeg2 =
    // TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSeg_X_NRT_LAX.xml");

    Itin itin;
    itin.originationCurrency() = "USD";
    itin.calculationCurrency() = "USD";

    trx.itin().push_back(&itin);

    FarePath farePath;

    farePath.itin() = &itin;

    farePath.itin()->travelSeg().push_back(airSeg);
    //   farePath.itin()->travelSeg().push_back(airSeg2);

    itin.farePath().push_back(&farePath);

    TaxResponse taxResponse;
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(trx));

    taxResponse.diagCollector() = &diagCollector;

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

    TariffCrossRefInfo tariffRefInfo;
    tariffRefInfo._fareTariffCode = "TAFPBA";

    fare.initialize(Fare::FS_International, &fareInfo, fareMarket, &tariffRefInfo);

    fareUsage.paxTypeFare() = &paxTypeFare;
    fareUsage.paxTypeFare()->setFare(&fare);
    fareUsage.totalFareAmount();

    fareUsage.travelSeg().push_back(airSeg);
    //   fareUsage.travelSeg().push_back(airSeg2);

    taxResponse.farePath()->pricingUnit().push_back(&pricingUnit);
    taxResponse.farePath()->pricingUnit()[0]->fareUsage().push_back(&fareUsage);

    TaxCodeReg* taxCodeReg2 =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml");

    TaxUS2 taxUS2;

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegNRT_LAX.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegANC_LAX.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    request.ticketingAgent()->agentLocation() = loc;

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegDFW_LON.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegNRT_LAX.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    farePath.itin()->travelSeg().clear();

    airSeg = TestAirSegFactory::create("/vobs/atseintl/test/testdata/data/AirSegANC_LAX.xml");

    farePath.itin()->travelSeg().push_back(airSeg);

    taxUS2.applyUS2(trx, taxResponse, *taxCodeReg2);

    //   TaxCodeReg* taxCodeReg2 =
    // TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeXV.xml");
  }

  void testCalculateHalfTaxAmountRoundUP()
  {
    TaxResponse* taxResponse = _memHandle.create<TaxResponse>();
    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml");
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    CurrencyCode paymentCurrency = "PLN";
    taxCodeReg->taxCur() = "USD";
    taxCodeReg->taxAmt() = 16.10;
    taxCodeReg->specConfigName() = "TEST_UP";
    Agent* agent = _memHandle.create<Agent>();
    agent->currencyCodeAgent() = "USD";
    PricingRequest* request = _memHandle.create<PricingRequest>();
    trx->setRequest(request);
    trx->getRequest()->ticketingAgent() = agent;
    trx->getRequest()->ticketingDT() = DateTime::localTime();
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml");
    request->ticketingAgent()->agentLocation() = loc;
    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx->setOptions(options);

    CurrencyConversionFacade ccFacade;
    Money targetMoney(paymentCurrency);
    targetMoney.value() = 0;
    Money sourceMoney(1, "USD");
    BSRCollectionResults bsrResults;

    ccFacade.convert(targetMoney,
                     sourceMoney,
                     *trx,
                     false,
                     CurrencyConversionRequest::TAXES,
                     false,
                     &bsrResults);

    MoneyAmount expected = targetMoney.value() * 8.10;
    MoneyAmount m =
        TaxUS2::calculateHalfTaxAmount(*trx, *taxResponse, paymentCurrency, *taxCodeReg);

    CPPUNIT_ASSERT(((expected - EPSILON) <= m) && (m <= (expected + EPSILON)));
  }

  void testCalculateHalfTaxAmountRoundDOWN()
  {
    TaxResponse* taxResponse = _memHandle.create<TaxResponse>();
    TaxCodeReg* taxCodeReg =
        TestTaxCodeRegFactory::create("/vobs/atseintl/test/testdata/data/TaxCodeReg_US2.xml");
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    CurrencyCode paymentCurrency = "PLN";
    taxCodeReg->taxCur() = "USD";
    taxCodeReg->taxAmt() = 16.10;
    taxCodeReg->specConfigName() = "TEST_DOWN";
    Agent* agent = _memHandle.create<Agent>();
    agent->currencyCodeAgent() = "USD";
    PricingRequest* request = _memHandle.create<PricingRequest>();
    trx->setRequest(request);
    trx->getRequest()->ticketingAgent() = agent;
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDUB.xml");
    request->ticketingAgent()->agentLocation() = loc;
    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx->setOptions(options);

    CurrencyConversionFacade ccFacade;
    Money targetMoney(paymentCurrency);
    targetMoney.value() = 0;
    Money sourceMoney(1, "USD");
    BSRCollectionResults bsrResults;

    ccFacade.convert(targetMoney,
                     sourceMoney,
                     *trx,
                     false,
                     CurrencyConversionRequest::TAXES,
                     false,
                     &bsrResults);

    MoneyAmount expected = targetMoney.value() * 8.00;
    MoneyAmount m =
        TaxUS2::calculateHalfTaxAmount(*trx, *taxResponse, paymentCurrency, *taxCodeReg);

    CPPUNIT_ASSERT(((expected - EPSILON) <= m) && (m <= (expected + EPSILON)));
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxUS2Test);
}
