#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Fares/DiscountPricing.h"

#include "DBAccess/BankerSellRate.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/NUCInfo.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

#include "DBAccess/DiskCache.h"

#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"
#include "test/DBAccessMock/DataHandleMock.h"

#define FARE1 333.33
#define FARE2 111.11

namespace tse
{
FALLBACKVALUE_DECL(azPlusUp);

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


class MyDataHandleMock : public DataHandleMock
{
public:
  const std::vector<BankerSellRate*>&
  getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
  {
      std::vector<BankerSellRate*>* rates = _memHandle.create<std::vector<tse::BankerSellRate*>>();
      if (primeCur == CurrencyCode("EUR") && cur == CurrencyCode("USD"))
        rates->push_back(getBSR(primeCur, cur, 1.0877, 4));
      else if (primeCur == CurrencyCode("USD") && cur == CurrencyCode("EUR"))
        rates->push_back(getBSR(primeCur, cur, 0.9194, 4));
      return *rates;
  }

  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    if (currency == "USD")
      return createNUC(currency, carrier, 1.000, 0.01, 4, 2, NEAREST);
    if (currency == "EUR")
      return createNUC(currency, carrier, 0.9194, 0.01, 4, 2, NEAREST);
    else
      return DataHandleMock::getNUCFirst(currency, carrier, date);
  }

protected:
  BankerSellRate* getBSR(const CurrencyCode& primeCur,
                         const CurrencyCode& cur,
                         const ExchRate& rate,
                         const CurrencyNoDec& noDec,
                         const Indicator& rateType = 'B',
                         const std::string& agentSine = "FXR")
  {
    BankerSellRate* sellRate = _memHandle.create<BankerSellRate>();
    sellRate->primeCur() = primeCur;
    sellRate->cur() = cur;
    sellRate->rate() = rate;
    sellRate->rateNodec() = noDec;
    sellRate->rateType() = rateType;
    sellRate->agentSine() = agentSine;
    return sellRate;
  }

  NUCInfo* createNUC(const CurrencyCode& cur,
                     CarrierCode carrier,
                     CurrencyFactor nucFactor,
                     CurrencyFactor rndFactor,
                     CurrencyNoDec nucNodec,
                     CurrencyNoDec rndNodec,
                     RoundingRule rule)
  {
    NUCInfo* ret = _memHandle.create<NUCInfo>();
    ret->_cur = cur;
    ret->_carrier = carrier;
    ret->_nucFactor = nucFactor;
    ret->_roundingFactor = rndFactor;
    ret->_nucFactorNodec = nucNodec;
    ret->_roundingFactorNodec = rndNodec;
    ret->_roundingRule = rule;
    return ret;
  }

  TestMemHandle _memHandle;
};

class DiscountPricingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiscountPricingTest);
  CPPUNIT_TEST(testApplyFareDiscount_NegativeDiscPercent_Old);
  CPPUNIT_TEST(testApplyFareDiscount_NegativeDiscPercent);
  CPPUNIT_TEST(testApplyFareDiscount_DiscPercentMoreThan100);
  CPPUNIT_TEST(testApplyFareDiscount_RT);
  CPPUNIT_TEST(testApplyFareDiscount_DiscEntryDA);
  CPPUNIT_TEST(testApplyFareDiscount_NUC);
  CPPUNIT_TEST(testApplyFareDiscount_NotNUC);
  CPPUNIT_TEST(testApplyFareDiscount_DiscPercent100_BeyondTotalNUCAmount);
  CPPUNIT_TEST(testApplyFareDiscount_DiscPercent100_NotBeyondTotalNUCAmount);

  CPPUNIT_TEST(testProcessDAorPA_discount);
  CPPUNIT_TEST(testProcessDAorPA_plusUp);
  CPPUNIT_TEST(testProcessDAorPA_discountOver100Percent);
  CPPUNIT_TEST(testProcessDAorPA_empty);
  CPPUNIT_TEST(testProcessDA_discount);
  CPPUNIT_TEST(testProcessDA_plusUp);
  CPPUNIT_TEST(testProcessDA_discountOver100Percent);
  CPPUNIT_TEST(testProcessDA_empty);

  CPPUNIT_TEST(testProcessDPorPP_discount);
  CPPUNIT_TEST(testProcessDPorPP_plusUp);
  CPPUNIT_TEST(testProcessDPorPP_discountOver100Percent);
  CPPUNIT_TEST(testProcessDPorPP_empty);
  CPPUNIT_TEST(testProcessDP_discount);
  CPPUNIT_TEST(testProcessDP_plusUp);
  CPPUNIT_TEST(testProcessDP_discountOver100Percent);
  CPPUNIT_TEST(testProcessDP_empty);

  CPPUNIT_TEST(testCalculateDiffAmount_withoutCurrencyCode);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_withoutCurrencyCode);
  CPPUNIT_TEST(testCalculateDiffAmount_noSaleLocation);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_noSaleLocation);
  CPPUNIT_TEST(testCalculateDiffAmount_currencyInAAA);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_currencyInAAA);
  CPPUNIT_TEST(testCalculateDiffAmount_noCurrencyConversion);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_noCurrencyConversion);
  CPPUNIT_TEST(testCalculateDiffAmount_USD_NUC);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_USD_NUC);
  CPPUNIT_TEST(testCalculateDiffAmount_EUR_NUC);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_EUR_NUC);
  CPPUNIT_TEST(testCalculateDiffAmount_EUR_USD);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_EUR_USD);
  CPPUNIT_TEST(testCalculateDiffAmount_badCurrency);
  CPPUNIT_TEST(testCalcApplicableDiscAmount_badCurrency);
  CPPUNIT_TEST(testApplyFareDiscount_totalAmountIsZero);

  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memHandle;
  MyDataHandleMock* _dataHandle;
  PricingTrx* _trx;
  PricingRequest* _pricingRequest;
  FareUsage* _fareUsage1;
  FareUsage* _fareUsage2;
  FarePath* _farePath;
  Fare* _fare1;
  Fare* _fare2;
  FareInfo* _fareInfo;
  PaxTypeFare* _ptf1;
  PaxTypeFare* _ptf2;
  PricingUnit* _pricingUnit;
  TravelSeg* _travelSeg;
  FareMarket* _fareMarket;
  DiscountPricing* _discountPricing;
  std::vector<DiscountAmount>* _discountAmounts;
  std::map<int16_t, Percent>* _discountPercentages;
  Loc* _agentLocation;

public:
  void setUp()
  {
    _memHandle.create<SpecificTestConfigInitializer>();
    _dataHandle = _memHandle.create<MyDataHandleMock>();
    _trx = _memHandle.create<PricingTrx>();
    _pricingRequest = _memHandle.create<PricingRequest>();
    _trx->setRequest(_pricingRequest);
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _agentLocation = _memHandle.create<Loc>();
    _agentLocation->nation() = NATION_ALL;
    _trx->getRequest()->ticketingAgent()->agentLocation() = _agentLocation;

    _fareUsage1 = _memHandle.create<FareUsage>();
    _fareUsage2 = _memHandle.create<FareUsage>();
    _farePath = _memHandle.create<FarePath>();

    _pricingUnit = _memHandle.create<PricingUnit>();
    _pricingUnit->fareUsage().push_back(_fareUsage1);
    _pricingUnit->fareUsage().push_back(_fareUsage2);
    _farePath->pricingUnit().push_back(_pricingUnit);

    _travelSeg = _memHandle.create<AirSeg>();
    _travelSeg->segmentOrder() = 1;
    _fareUsage1->travelSeg().push_back(_travelSeg);
    _fareUsage2->travelSeg().push_back(_travelSeg);

    _fareMarket = _memHandle.create<FareMarket>();
    _fareMarket->travelSeg().push_back(_travelSeg);

    _ptf1 = _memHandle.create<PaxTypeFare>();
    _ptf2 = _memHandle.create<PaxTypeFare>();
    _ptf1->fareMarket() = _fareMarket;
    _ptf2->fareMarket() = _fareMarket;
    _fareUsage1->paxTypeFare() = _ptf1;
    _fareUsage2->paxTypeFare() = _ptf2;

    _fare1 = _memHandle.create<Fare>();
    _fare1->nucFareAmount() = FARE2;
    _fare1->nucOriginalFareAmount() = FARE2;
    _fare2 = _memHandle.create<Fare>();
    _fare2->nucFareAmount() = FARE1;
    _fare2->nucOriginalFareAmount() = FARE1;

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->owrt() = ONE_WAY_MAY_BE_DOUBLED;
    _fare1->setFareInfo(_fareInfo);
    _fare2->setFareInfo(_fareInfo);

    _ptf1->setFare(_fare1);
    _ptf2->setFare(_fare2);

    _farePath->calculationCurrency() = NUC;
    _farePath->setTotalNUCAmount(_fare1->nucFareAmount() + _fare2->nucFareAmount());
    _farePath->baseFareCurrency() = "USD";

    _discountPricing = _memHandle.create<DiscountPricing>(*_trx, *_farePath);
    _discountAmounts = _memHandle.create<std::vector<DiscountAmount>>();
    _discountPercentages = _memHandle.create<std::map<int16_t, Percent>>();
  }

  void tearDown() { _memHandle.clear(); }

  void applyFareDiscount(const Percent& discPercent)
  {
    DiscountPricing::ApplyFareDiscount discount(*_farePath, discPercent, DiscountPricing(*_trx, *_farePath));
    discount(_fareUsage1);
  }

  // TESTS

  void testApplyFareDiscount_NegativeDiscPercent_Old()
  {
    fallback::value::azPlusUp.set(true);

    applyFareDiscount(-1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_NegativeDiscPercent()
  {
    applyFareDiscount(-1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2 + FARE2 / 100, _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_DiscPercentMoreThan100()
  {
    applyFareDiscount(101);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_RT()
  {
    _fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _fare1->nucOriginalFareAmount() *= 2;
    applyFareDiscount(10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 - 11.111), _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_DiscEntryDA()
  {
    DiscountPricing::ApplyFareDiscount discount(*_farePath, 10, 100, DiscountPricing(*_trx, *_farePath));
    _fareUsage1->minFarePlusUpAmt() = 10;
    discount(_fareUsage1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        (FARE1 + FARE2 - 10 / 10 - 11.111), _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_NUC()
  {
    applyFareDiscount(10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        (FARE1 + FARE2 - 11.111 - 0.005), _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_NotNUC()
  {
    _farePath->calculationCurrency() = "USD";
    applyFareDiscount(10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 - 11.111), _farePath->getTotalNUCAmount(), 0.01);
  }

  void testApplyFareDiscount_DiscPercent100_BeyondTotalNUCAmount()
  {
    _farePath->setTotalNUCAmount(_fare1->nucFareAmount() - EPSILON / 2);
    applyFareDiscount(100);
    CPPUNIT_ASSERT_EQUAL(0.00, _farePath->getTotalNUCAmount());
  }

  void testApplyFareDiscount_DiscPercent100_NotBeyondTotalNUCAmount()
  {
    applyFareDiscount(100);
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 - FARE2), _farePath->getTotalNUCAmount(), 0.01);
  }

  void testProcessDAorPA_discount()
  {
    _discountAmounts->push_back(DiscountAmount(100.00, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 - 100.00), _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(75.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 - 25.00, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 - 75.00, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDAorPA_plusUp()
  {
    _discountAmounts->push_back(DiscountAmount(-1000, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 + 1000.00), _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-250.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-750.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 +  250.00, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + 750.00, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDAorPA_discountOver100Percent()
  {
    _discountAmounts->push_back(DiscountAmount(1000, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
    CPPUNIT_ASSERT_THROW(_discountPricing->process(), tse::ErrorResponseException);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDAorPA_empty()
  {
    _trx->getRequest()->setDiscountAmountsNew(*_discountAmounts);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDA_discount()
  {
    _discountAmounts->push_back(DiscountAmount(100.00, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmounts(*_discountAmounts);
    _discountPricing->processOld();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 - 100.00), _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(75.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 - 25.00, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 - 75.00, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDA_plusUp()
  {
    _discountAmounts->push_back(DiscountAmount(-1000, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmounts(*_discountAmounts);
    _discountPricing->processOld();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2 + 0.00), _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDA_discountOver100Percent()
  {
    _discountAmounts->push_back(DiscountAmount(1000, "USD", 1, 1));
    _trx->getRequest()->setDiscountAmounts(*_discountAmounts);
    CPPUNIT_ASSERT_THROW(_discountPricing->processOld(), tse::ErrorResponseException);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDA_empty()
  {
    _discountPricing->processOld();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDPorPP_discount()
  {
    _discountPercentages->insert( {1, 50.0} );
    _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2) / 2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 / 2, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 / 2, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 * 0.5, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 * 0.5, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDPorPP_plusUp()
  {
    _discountPercentages->insert( {1, -50.0} );
    _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2) * 1.5, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-FARE2 * 0.5, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-FARE1 * 0.5, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 * 1.5, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 * 1.5, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDPorPP_discountOver100Percent()
  {
    _discountPercentages->insert( {1, 101.0} );
    _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
    CPPUNIT_ASSERT_THROW(_discountPricing->process(), ErrorResponseException);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDPorPP_empty()
  {
    _trx->getRequest()->setDiscountPercentagesNew(*_discountPercentages);
    _discountPricing->process();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2), _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDP_discount()
  {
    _discountPercentages->insert( {1, 50.0} );
    _trx->getRequest()->setDiscountPercentages(*_discountPercentages);
    _discountPricing->processOld();
    CPPUNIT_ASSERT_DOUBLES_EQUAL((FARE1 + FARE2) / 2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 / 2, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 / 2, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2 * 0.5, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 * 0.5, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDP_plusUp()
  {
    _discountPercentages->insert( {1, -50.0} );
    _trx->getRequest()->setDiscountPercentages(*_discountPercentages);
    _discountPricing->processOld();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDP_discountOver100Percent()
  {
    _discountPercentages->insert( {1, 101.0} );
    _trx->getRequest()->setDiscountPercentages(*_discountPercentages);
    CPPUNIT_ASSERT_THROW(_discountPricing->processOld(), ErrorResponseException);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testProcessDP_empty()
  {
    _discountPricing->processOld();
    _trx->getRequest()->setDiscountPercentages(*_discountPercentages);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1 + FARE2, _farePath->getTotalNUCAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage1->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.00, _fareUsage2->getDiscAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE2, _fareUsage1->totalFareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FARE1, _fareUsage2->totalFareAmount(), 0.01);
  }

  void testCalculateDiffAmount_withoutCurrencyCode()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(0.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_withoutCurrencyCode()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(0.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_noSaleLocation()
  {
    _agentLocation = nullptr;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(0.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_noSaleLocation()
  {
    _agentLocation = nullptr;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(0.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_currencyInAAA()
  {
    _agentLocation->nation() = UNITED_STATES;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(-100.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_currencyInAAA()
  {
    _agentLocation->nation() = UNITED_STATES;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(100.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_noCurrencyConversion()
  {
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "USD";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(-100.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "USD", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_noCurrencyConversion()
  {
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "USD";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(100.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "USD", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_USD_NUC()
  {
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(-100.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "USD", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_USD_NUC()
  {
    _agentLocation->nation() = GERMANY;
    _farePath->baseFareCurrency() = "USD";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(100.00),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "USD", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_EUR_USD()
  {
    _agentLocation->nation() = GERMANY;
    _farePath->baseFareCurrency() = "EUR";
    _farePath->calculationCurrency() = "USD";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(-91.94), _discountPricing->calculateDiffAmount(DiscountAmount(100, "USD", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_EUR_USD()
  {
    _agentLocation->nation() = GERMANY;
    _farePath->baseFareCurrency() = "EUR";
    _farePath->calculationCurrency() = "USD";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(91.94),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "USD", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_EUR_NUC()
  {
    _agentLocation->nation() = GERMANY;
    _farePath->baseFareCurrency() = "EUR";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(-100.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "USD", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_EUR_NUC()
  {
    _agentLocation->nation() = GERMANY;
    _farePath->baseFareCurrency() = "EUR";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(100.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "USD", 1, 1)),
        0.01);
  }

  void testCalculateDiffAmount_badCurrency()
  {
    _farePath->baseFareCurrency() = "";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(MoneyAmount(0.0), _discountPricing->calculateDiffAmount(DiscountAmount(100, "USD", 1, 1)), 0.01);
  }

  void testCalcApplicableDiscAmount_badCurrency()
  {
    _farePath->baseFareCurrency() = "";
    _farePath->calculationCurrency() = "NUC";
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        MoneyAmount(0.0),
        _discountPricing->calcApplicableDiscAmount(DiscountAmount(100, "USD", 1, 1)),
        0.01);
  }

  void testApplyFareDiscount_totalAmountIsZero()
  {
    MoneyAmount discountAmount = 100.0;
    MoneyAmount totalAmount = 0.0;
    _farePath->setTotalNUCAmount(0.0);
    Fare fare;
    FareInfo fareInfo;
    fareInfo.owrt() = ALL_WAYS;
    fare.setFareInfo(&fareInfo);
    _fareUsage1->paxTypeFare()->setFare(&fare);
    _fareUsage1->setDiscAmount(discountAmount);

    DiscountPricing::ApplyFareDiscount applyFareDiscount(*_farePath, discountAmount, totalAmount, *_discountPricing);
    applyFareDiscount(_fareUsage1);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, _farePath->getTotalNUCAmount(), 0.01);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DiscountPricingTest);
}
