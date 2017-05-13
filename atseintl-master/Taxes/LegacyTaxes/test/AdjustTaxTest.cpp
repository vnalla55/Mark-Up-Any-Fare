#include "Taxes/LegacyTaxes/AdjustTax.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/BankerSellRate.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag804Collector.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{
class AdjustTaxTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(AdjustTaxTest);
  CPPUNIT_TEST(testPlusUp);
  CPPUNIT_TEST(testPlusUp_negative);
  CPPUNIT_TEST(testPlusUp_diffCurrency);
  CPPUNIT_TEST(testBooklet);
  CPPUNIT_TEST(testBooklet_diffCurrency);
  CPPUNIT_TEST(testMinimum_minApplies);
  CPPUNIT_TEST(testMinimum_minDoesntApply);
  CPPUNIT_TEST(testMinimum_diffCurrency);
  CPPUNIT_TEST(testMaximum_maxApplies);
  CPPUNIT_TEST(testMaximum_maxDoesntApply);
  CPPUNIT_TEST(testMaximum_diffCurrency);
  CPPUNIT_TEST_SUITE_END();

  class SpecificTestConfigInitializer : public TestConfigInitializer
  {
  public:
    SpecificTestConfigInitializer()
    {
      DiskCache::initialize(_config);
      _memHandle.create<MockDataManager>();
    }

    ~SpecificTestConfigInitializer() { _memHandle.clear(); }

  private:
    TestMemHandle _memHandle;
  };

public:

  // Do Plus Up
  void testPlusUp()
  {
    const CurrencyCode paymentCurrency = "USD";

    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 50.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(65.00, taxCalcAmt);
  }

  void testPlusUp_negative()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = -50.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(15.00, taxCalcAmt);
  }

  void testPlusUp_diffCurrency()
  {
    const CurrencyCode paymentCurrency = "GBP";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 50.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(15.00 + 50.00*MyDataHandle::RateGbp2Usd, taxCalcAmt);
  }

  // Do Booklet Coupon Per Ticket
  void testBooklet()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.occurrence() = 'B'; // TICKET_BOOKLET
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(15.00, taxCalcAmt);
  }

  void testBooklet_diffCurrency()
  {
    const CurrencyCode paymentCurrency = "GBP";
    TaxCodeReg taxCodeReg;
    taxCodeReg.occurrence() = 'B'; // TICKET_BOOKLET
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(15.00*MyDataHandle::RateGbp2Usd, taxCalcAmt);
  }

  // Do Minimum Tax
  void testMinimum_minApplies()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 20.00;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(20.00, taxCalcAmt);
  }

  // Do Minimum Tax
  void testMinimum_minDoesntApply()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 35.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 20.00;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(35.00, taxCalcAmt);
  }

  void testMinimum_diffCurrency()
  {
    const CurrencyCode paymentCurrency = "GBP";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 25.00;
    taxCodeReg.maxTax() = 0;

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(25.00*MyDataHandle::RateGbp2Usd, taxCalcAmt);
  }

  // Do Maximum Tax
  void testMaximum_maxApplies()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0.00;
    taxCodeReg.maxTax() = 10.00;
    taxCodeReg.taxType() = 'P';

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(10.00, taxCalcAmt);
  }

  // Do Maximum Tax
  void testMaximum_maxDoesntApply()
  {
    const CurrencyCode paymentCurrency = "USD";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0.00;
    taxCodeReg.maxTax() = 18.00;
    taxCodeReg.taxType() = 'P';

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(15.00, taxCalcAmt);
  }

  void testMaximum_diffCurrency()
  {
    const CurrencyCode paymentCurrency = "GBP";
    TaxCodeReg taxCodeReg;
    taxCodeReg.plusupAmt() = 0.00;
    taxCodeReg.taxAmt() = 15.00;
    taxCodeReg.taxCur() = "USD";
    taxCodeReg.minTax() = 0;
    taxCodeReg.maxTax() = 10.00;
    taxCodeReg.taxType() = 'P';

    const MoneyAmount taxAmount = taxCodeReg.taxAmt();

    MoneyAmount taxCalcAmt = AdjustTax::applyAdjust(
        *_trx, *_taxResponse, taxAmount, paymentCurrency, taxCodeReg, *_details);

    CPPUNIT_ASSERT_EQUAL(10.00*MyDataHandle::RateGbp2Usd, taxCalcAmt);
  }

  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();

    // Set up the PricingTrx
    _trx = _memHandle.create<PricingTrx>();

    _trx->setOptions(_memHandle.create<PricingOptions>());

    Loc* agentLocation = _memHandle.create<Loc>();
    agentLocation->loc() = "DFW";
    agentLocation->nation() = "US";

    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = agentLocation;

    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingAgent() = agent;

    _trx->setRequest(request);
    _trx->getRequest()->ticketingAgent() = agent;
    DateTime dt;
    _trx->getRequest()->ticketingDT() = dt.localTime();
    _trx->getRequest()->ticketingAgent()->currencyCodeAgent() = "USD";

    // Finished setting up trx.

    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = "NYC";
    origin->nation() = "US";

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = "LIM";
    destination->nation() = "PE";

    _trx->getRequest()->ticketingAgent()->agentLocation() = origin; // AAA in USA

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;

    // Done setting up travel segment

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    _trx->itin().push_back(itin);
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    diagroot->activate();
    Diag804Collector* diag804 = _memHandle.insert(new Diag804Collector(*diagroot));

    _taxResponse->diagCollector() = diag804;

    _details = _memHandle.create<CalculationDetails>();
  }

  void tearDown() { _memHandle.clear(); }

private:

  class MyDataHandle : public DataHandleMock
  {
  public:
    static constexpr double RateGbp2Usd = 0.65;

    TestMemHandle _memHandle;

    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = primeCur;
      bsr->cur() = cur;
      bsr->rateType() = 'B';
      bsr->agentSine() = "FXR";
      if (primeCur == "USD" && cur == "GBP")
      {
        bsr->rate() = RateGbp2Usd;
        bsr->rateNodec() = 2;
      }
      else
      {
        bsr->rate() = 0.1;
        bsr->rateNodec() = 1;
      }

      ret->push_back(bsr);
      return *ret;
    }
  };

  tse::PricingTrx* _trx;
  tse::TaxResponse* _taxResponse;
  tse::CalculationDetails* _details;
  TestMemHandle _memHandle;
  MyDataHandle* _mdh;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AdjustTaxTest);
}
