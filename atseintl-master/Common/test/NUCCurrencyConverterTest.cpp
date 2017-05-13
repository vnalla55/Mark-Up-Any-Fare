#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/FarePath.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConverter.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/Money.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/Agent.h"
#include "DBAccess/Loc.h"
#include "DataModel/ExchangePricingTrx.h"
#include "Server/TseServer.h"

// using namespace tse;
namespace tse
{

class TseServerMock : public TseServer
{
public:
  static tse::ConfigMan* getConfig() { return Global::_configMan; }
  static void setConfig(tse::ConfigMan* configMan) { Global::_configMan = configMan; }
};

class MockNUCCurrencyConverter : public NUCCurrencyConverter
{
public:
  MockNUCCurrencyConverter() : NUCCurrencyConverter() {}

  bool getNucInfo(const tse::CarrierCode& carrier,
                  const tse::CurrencyCode& currency,
                  const tse::DateTime& ticketDate,
                  tse::ExchRate& nucFactor,
                  tse::RoundingFactor& nucRoundingFactor,
                  RoundingRule& roundingRule,
                  CurrencyNoDec& roundingFactorNoDec,
                  CurrencyNoDec& nucFactorNoDec,
                  DateTime& discontinueDate,
                  DateTime& effectiveDate,
                  CurrencyConversionCache* cache = 0)
  {
    return true;
  }
};

class NUCCurrencyConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NUCCurrencyConverterTest);
  CPPUNIT_TEST(testConvertROEoverride);
  CPPUNIT_TEST(testConvertBaseFareROEoverride);

  CPPUNIT_TEST(testDefineConversionDateReturnOriginalIssueDateWhenNotReissueExchange);
  CPPUNIT_TEST(testDefineConversionDateReturnHistoricalBsrDateForExchangeItin);
  CPPUNIT_TEST(testDefineConversionDateReturnFirstConverionDateWhenFlagForSecondNotOn);
  CPPUNIT_TEST(testDefineConversionDateReturnSecondConversionDateWhenFlagOn);
  CPPUNIT_TEST_SUITE_END();

  tse::ConfigMan _config;

public:
  void setUp() { TseServerMock::setConfig(&_config); }

  void testConvertROEoverride()
  {
    Money target("PLN");
    // 100 NUC is amount that we check for ROE override
    Money source(100, "NUC");
    DateTime ticketDate(time(NULL));
    PricingRequest request;
    DataHandle dataHandle;

    PricingTrx* trx = new ExchangePricingTrx;

    CurrencyConversionRequest ccrequest(target,
                                        source,
                                        ticketDate,
                                        request,
                                        dataHandle,
                                        true,
                                        CurrencyConversionRequest::OTHER,
                                        false,
                                        nullptr,
                                        false,
                                        true,
                                        trx);

    trx->setRequest(&request);
    // override ROE is 1.5
    trx->getRequest()->roeOverride() = 1.5;
    trx->getRequest()->roeOverrideNoDec() = 1;

    CurrencyCollectionResults results;

    MockNUCCurrencyConverter ncc;
    ncc.convert(ccrequest, &results);

    // 100 x 1.5 should be equal 150...
    CPPUNIT_ASSERT(ccrequest.target().value() == 150);
  }

  void testConvertBaseFareROEoverride()
  {
    DateTime ticketDate(time(NULL));
    PricingRequest request;
    PricingOptions options;
    Itin itin;
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.calculationCurrency() = "PLN";
    Agent agent;
    Loc loc;
    agent.agentLocation() = &loc;
    ExchangePricingTrx trx;

    trx.setRequest(&request);
    trx.getRequest()->ticketingAgent() = &agent;
    trx.setOptions(&options);
    trx.itin().push_back(&itin);

    // set override values in trx...
    trx.getRequest()->roeOverride() = 1.5;
    trx.getRequest()->roeOverrideNoDec() = 2;
    MockNUCCurrencyConverter ncc;
    MoneyAmount ma;
    CurrencyNoDec cnd;
    CurrencyCode cc = "NUC";

    // set diffrent values and send them to convertBaseFare...
    ExchRate roeRate = 3.3;
    CurrencyNoDec roeRateNoDec = 5;

    ncc.convertBaseFare(trx,
                        farePath,
                        farePath.getTotalNUCAmount(),
                        ma,
                        cnd,
                        cc,
                        roeRate,
                        roeRateNoDec,
                        ticketDate,
                        ticketDate,
                        true,
                        true);
    // see if code changed roe values...
    CPPUNIT_ASSERT(roeRate == 1.5);
    CPPUNIT_ASSERT(roeRateNoDec == 2);
  }

  void testDefineConversionDateReturnOriginalIssueDateWhenNotReissueExchange()
  {
    RexPricingTrx rexTrx;
    FarePath fp;
    NUCCurrencyConverter ncc;
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    CPPUNIT_ASSERT_EQUAL(rexTrx.originalTktIssueDT(), ncc.defineConversionDate(rexTrx, fp));
  }

  void testDefineConversionDateReturnHistoricalBsrDateForExchangeItin()
  {
    RexPricingTrx rexTrx;
    FarePath fp;
    NUCCurrencyConverter ncc;
    PricingOptions options;
    rexTrx.setOptions(&options);
    rexTrx.setAnalyzingExcItin(true);
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.setHistoricalBsrRoeDate();
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    rexTrx.previousExchangeDT() = DateTime(2010, 3, 10);
    CPPUNIT_ASSERT_EQUAL(rexTrx.getHistoricalBsrRoeDate(), ncc.defineConversionDate(rexTrx, fp));
  }

  void testDefineConversionDateReturnFirstConverionDateWhenFlagForSecondNotOn()
  {
    RexPricingTrx rexTrx;
    FarePath fp;
    fp.exchangeReissue() = true;
    NUCCurrencyConverter ncc;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.newItinROEConversionDate() = DateTime(2010, 3, 10);
    rexTrx.useSecondROEConversionDate() = false;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinROEConversionDate(), ncc.defineConversionDate(rexTrx, fp));
  }

  void testDefineConversionDateReturnSecondConversionDateWhenFlagOn()
  {
    RexPricingTrx rexTrx;
    FarePath fp;
    fp.useSecondRoeDate() = true;
    NUCCurrencyConverter ncc;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 11);
    rexTrx.useSecondROEConversionDate() = true;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinSecondROEConversionDate(),
                         ncc.defineConversionDate(rexTrx, fp));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(NUCCurrencyConverterTest);
}
