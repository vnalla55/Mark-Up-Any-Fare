#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "test/DBAccessMock/DataHandleMock.h"

#include "FareCalc/FareCalcCollector.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/Billing.h"
#include "DBAccess/Customer.h"
#include "Common/TrxUtil.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/NetRemitFarePath.h"
#include "FareCalc/AltFareCalculation.h"
#include "FareCalc/FcTaxInfo.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/NetFarePath.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/RuleItemInfo.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

using namespace boost::assign;


namespace tse
{

  class MyDataHandle : public DataHandleMock
  {
  private:
    TestMemHandle _memHandle;

  public:
    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = primeCur;
      bsr->cur() = cur;
      bsr->rateType() = 'B';
      bsr->agentSine() = "FXR";
      if (primeCur == "GBP" && cur == "USD")
      {
        bsr->rate() = 2;
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


class FareCalcCollectorStub : public FareCalcCollector
{
public:
  FareCalcCollectorStub() : FareCalcCollector(), _returnZeroCalcTotal(false), _total(0) {}
  ~FareCalcCollectorStub() {}

  CalcTotals*
  createCalcTotals(PricingTrx& pricingTrx, const FareCalcConfig* fcConfig, const FarePath* fp)
  {
    if (_returnZeroCalcTotal)
      return 0;
    _total = new CalcTotals();
    return _total;
  }
  bool _returnZeroCalcTotal;
  CalcTotals* _total;
};

class FareCalcCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCalcCollectorTest);

  CPPUNIT_TEST(testReorderPaxTypesReorderWhenLastPaxTypeInputOrderZero);
  CPPUNIT_TEST(testReorderPaxTypesDoNotReorderWhenLastPaxTypeInputOrderNotZero);

  CPPUNIT_TEST(testCreateNetRemitCalcTotalReturnTrueWhenNotNetRemitOrAxess);
  CPPUNIT_TEST(testCreateNetRemitCalcTotalReturnFalseWhenNetRemitCalcTotalIsZero);
  CPPUNIT_TEST(testCreateNetRemitCalcTotalReturnTrueWhenNetRemitCalcTotalNotZero);
  CPPUNIT_TEST(testCreateNetRemitCalcTotalReturnFalseWhenAxessCalcTotalIsZero);
  CPPUNIT_TEST(testCreateNetRemitCalcTotalReturnTrueWhenAxessCalcTotalNotZero);

  CPPUNIT_TEST(testGetTotalPriceCurrencyReturnEmptyWhenMapEmpty);
  CPPUNIT_TEST(testGetTotalPriceCurrencyReturnCurrencyForMatchingFarePath);
  CPPUNIT_TEST(testIsMixedBaseFareCurrencyReturnFalseWhenCalcTotalMapEmpty);
  CPPUNIT_TEST(testIsMixedBaseFareCurrencyReturnFalseWhenFarePathsNotYetProcessed);
  CPPUNIT_TEST(testIsMixedBaseFareCurrencyReturnTrueWhenDifferentCurrencyBetweenCalcTotals);
  CPPUNIT_TEST(testGetBaseFareTotalReturnZeroWhenCalcTotalMapEmpty);
  CPPUNIT_TEST(testGetBaseFareTotalReturnZeroWhenCalcTotalZeroInMap);
  CPPUNIT_TEST(testGetBaseFareTotalReturnAmountForOnePassenger);
  CPPUNIT_TEST(testGetBaseFareTotalReturnAmountForTwoPassenger);
  CPPUNIT_TEST(testGetEquivFareAmountTotalReturnTotalEquivalentAmount);
  CPPUNIT_TEST(testIsMixedEquivFareAmountCurrencyReturnFalseWhenFarePathNotProcessed);
  CPPUNIT_TEST(
      testIsMixedEquivFareAmountCurrencyReturnFalseWhenBaseCurrencyNotEqualToEquivCurrency);
  CPPUNIT_TEST(testIsMixedEquivFareAmountCurrencyReturnTrueWhenBaseCurrencyEqualToEquivCurrency);
  CPPUNIT_TEST(testFindCalcTotalsReturnCalcTotalForFarePath);
  CPPUNIT_TEST(testFindCalcTotalsReturnCalcTotalForPaxType);
  CPPUNIT_TEST(testIsFareCalcTooLongReturnFalseWhenCalcTotalFclToLongEqualFalse);
  CPPUNIT_TEST(testIsFareCalcTooLongReturnTrueWhenCalcTotalFclToLongEqualTrue);

  CPPUNIT_TEST(testGetIataSalesCode);

  CPPUNIT_TEST(testCreateNetCalcTotalReturnTrueWhenNotNetFarePath);
  CPPUNIT_TEST(testCreateNetCalcTotalReturnFalseWhenNetCalcTotalIsZero);
  CPPUNIT_TEST(testCreateNetCalcTotalReturnTrueWhenNetCalcTotalNotZero);

  CPPUNIT_TEST(testRequestedFareBasisIsTrue);
  CPPUNIT_TEST(testRequestedFareBasisIsFalse);

  CPPUNIT_TEST(testGetAdjustedSellingDifference);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoNoTax);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoMismatchTax);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoNoGSTTax);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoGSTTax);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoMultiGSTTax);
  CPPUNIT_TEST(testProcessAdjustedSellingDiffInfoMismacthedGSTTax);
  CPPUNIT_TEST(testCreateAdjustedCalcTotal);
  CPPUNIT_TEST(testCopyItemsToAdjustedFP);
  CPPUNIT_TEST(testCollectPriceByCabinMessage);
  CPPUNIT_TEST(testCollectWPNCBPriceByCabinTrailerMessage);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _fcc = new FareCalcCollector();
    _trx = new PricingTrx();
    _request = new PricingRequest();
    _request->collectOBFee() = 'T';
    _options = new PricingOptions();
    _agent = new Agent();
    _billing = new Billing();
    _trx->setRequest(_request);
    _trx->setOptions(_options);
    _request->ticketingAgent() = _agent;
    _customer = new Customer();
    _agent->agentTJR() = _customer;
    _trx->billing() = _billing;
    _totals = new CalcTotals;
    _fcTaxInfo = new FareCalc::FcTaxInfo;
    _itin = new Itin();

  }

  void tearDown()
  {
    delete _fcTaxInfo;
    delete _totals;
    delete _customer;
    delete _agent;
    delete _request;
    delete _options;
    delete _billing;
    delete _trx;
    delete _fcc;
    delete _itin;
    _memHandle.clear();
  }

  void testReorderPaxTypesReorderWhenLastPaxTypeInputOrderZero()
  {
    PaxType pax1, pax2;
    uint16_t one = 1, zero = 0;
    pax1.inputOrder() = one;
    pax2.inputOrder() = zero;
    _trx->paxType() += &pax1, &pax2;
    _fcc->reorderPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL(zero, pax1.inputOrder());
    CPPUNIT_ASSERT_EQUAL(one, pax2.inputOrder());
  }

  void testReorderPaxTypesDoNotReorderWhenLastPaxTypeInputOrderNotZero()
  {
    PaxType pax1, pax2;
    uint16_t one = 1, two = 2;
    pax1.inputOrder() = two;
    pax2.inputOrder() = one;
    _trx->paxType() += &pax1, &pax2;
    _fcc->reorderPaxTypes(*_trx);
    CPPUNIT_ASSERT_EQUAL(two, pax1.inputOrder());
    CPPUNIT_ASSERT_EQUAL(one, pax2.inputOrder());
  }

  void testCreateNetRemitCalcTotalReturnTrueWhenNotNetRemitOrAxess()
  {
    CPPUNIT_ASSERT(_fcc->createNetRemitCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
  }

  void testCreateNetRemitCalcTotalReturnFalseWhenNetRemitCalcTotalIsZero()
  {
    FareCalcCollectorStub fccStub;
    fccStub._returnZeroCalcTotal = true;
    NetRemitFarePath netRemitFp;
    _fp.netRemitFarePath() = &netRemitFp;
    CPPUNIT_ASSERT(!fccStub.createNetRemitCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
  }

  void testCreateNetRemitCalcTotalReturnTrueWhenNetRemitCalcTotalNotZero()
  {
    FareCalcCollectorStub fccStub;
    NetRemitFarePath netRemitFp;
    _fp.netRemitFarePath() = &netRemitFp;
    CPPUNIT_ASSERT(fccStub.createNetRemitCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
    delete fccStub._total;
  }

  void testCreateNetRemitCalcTotalReturnFalseWhenAxessCalcTotalIsZero()
  {
    FareCalcCollectorStub fccStub;
    fccStub._returnZeroCalcTotal = true;
    FarePath fpAxess;
    _fp.axessFarePath() = &fpAxess;
    CPPUNIT_ASSERT(!fccStub.createNetRemitCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
  }

  void testCreateNetRemitCalcTotalReturnTrueWhenAxessCalcTotalNotZero()
  {
    FareCalcCollectorStub fccStub;
    FarePath fpAxess;
    _fp.axessFarePath() = &fpAxess;
    CPPUNIT_ASSERT(fccStub.createNetRemitCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
    delete fccStub._total;
  }

  void testGetTotalPriceCurrencyReturnEmptyWhenMapEmpty()
  {
    _fcc->_calcTotalsMap.clear();
    CurrencyCode emptyCurrency = "";
    CPPUNIT_ASSERT_EQUAL(emptyCurrency, _fcc->getTotalPriceCurrency());
  }

  void testGetTotalPriceCurrencyReturnCurrencyForMatchingFarePath()
  {
    populateCalcTotalsMap();
    CurrencyCode expectedCurrency = _totals->convertedBaseFareCurrencyCode;
    CPPUNIT_ASSERT_EQUAL(expectedCurrency, _fcc->getTotalPriceCurrency());
  }

  void testIsMixedBaseFareCurrencyReturnFalseWhenCalcTotalMapEmpty()
  {
    _fcc->_calcTotalsMap.clear();
    bool forNetRemit = false;
    CPPUNIT_ASSERT(!_fcc->isMixedBaseFareCurrency(forNetRemit));
  }

  void testIsMixedBaseFareCurrencyReturnFalseWhenFarePathsNotYetProcessed()
  {
    populateCalcTotalsMap();
    _fp.processed() = false;
    bool forNetRemit = false;
    CPPUNIT_ASSERT(!_fcc->isMixedBaseFareCurrency(forNetRemit));
  }

  void testIsMixedBaseFareCurrencyReturnTrueWhenDifferentCurrencyBetweenCalcTotals()
  {
    populateCalcTotalsMap();
    FarePath anotherFp;
    anotherFp.processed() = true;
    CalcTotals anotherTotal;
    anotherTotal.convertedBaseFareCurrencyCode = "INR";
    anotherTotal.farePath = &anotherFp;
    _fcc->_calcTotalsMap[&anotherFp] = &anotherTotal;

    bool forNetRemit = false;
    CPPUNIT_ASSERT(_fcc->isMixedBaseFareCurrency(forNetRemit));
  }

  void testGetBaseFareTotalReturnZeroWhenCalcTotalMapEmpty()
  {
    _fcc->_calcTotalsMap.clear();
    CurrencyCode curr;
    CurrencyNoDec decNo;
    MoneyAmount expectedMoney = 0;
    bool forNetRemit = false;
    CPPUNIT_ASSERT_EQUAL(expectedMoney, _fcc->getBaseFareTotal(*_trx, curr, decNo, forNetRemit));
  }

  void testGetBaseFareTotalReturnZeroWhenCalcTotalZeroInMap()
  {
    populateCalcTotalsMap();
    _fcc->_calcTotalsMap[&_fp] = 0;
    CurrencyCode curr;
    CurrencyNoDec decNo;
    MoneyAmount expectedMoney = 0;
    bool forNetRemit = false;
    CPPUNIT_ASSERT_EQUAL(expectedMoney, _fcc->getBaseFareTotal(*_trx, curr, decNo, forNetRemit));
  }

  void testGetBaseFareTotalReturnAmountForOnePassenger()
  {
    populateCalcTotalsMap();
    CurrencyCode curr;
    CurrencyNoDec decNo;
    MoneyAmount expectedMoney = _totals->convertedBaseFare * _paxType.number();
    bool forNetRemit = false;
    CPPUNIT_ASSERT_EQUAL(expectedMoney, _fcc->getBaseFareTotal(*_trx, curr, decNo, forNetRemit));
    CPPUNIT_ASSERT_EQUAL(_totals->convertedBaseFareCurrencyCode, curr);
    CPPUNIT_ASSERT_EQUAL(_totals->convertedBaseFareNoDec, decNo);
  }

  void testGetBaseFareTotalReturnAmountForTwoPassenger()
  {
    populateCalcTotalsMap();
    _paxType.number() = 2;
    CurrencyCode curr;
    CurrencyNoDec decNo;
    MoneyAmount expectedMoney = _totals->convertedBaseFare * _paxType.number();
    bool forNetRemit = false;
    CPPUNIT_ASSERT_EQUAL(expectedMoney, _fcc->getBaseFareTotal(*_trx, curr, decNo, forNetRemit));
    CPPUNIT_ASSERT_EQUAL(_totals->convertedBaseFareCurrencyCode, curr);
    CPPUNIT_ASSERT_EQUAL(_totals->convertedBaseFareNoDec, decNo);
  }

  void testGetEquivFareAmountTotalReturnTotalEquivalentAmount()
  {
    populateCalcTotalsMap();
    CurrencyCode curr;
    CurrencyNoDec decNo;
    MoneyAmount expectedMoney = _totals->equivFareAmount;
    bool forNetRemit = false;
    CPPUNIT_ASSERT_EQUAL(expectedMoney, _fcc->getEquivFareAmountTotal(*_trx, curr, decNo, forNetRemit));
    CPPUNIT_ASSERT_EQUAL(_totals->equivCurrencyCode, curr);
    CPPUNIT_ASSERT_EQUAL(_totals->taxNoDec(), decNo);
  }

  void testIsMixedEquivFareAmountCurrencyReturnFalseWhenFarePathNotProcessed()
  {
    populateCalcTotalsMap();
    _fp.processed() = false;
    CPPUNIT_ASSERT(!_fcc->isMixedEquivFareAmountCurrency());
  }

  void testIsMixedEquivFareAmountCurrencyReturnFalseWhenBaseCurrencyNotEqualToEquivCurrency()
  {
    populateCalcTotalsMap();
    CPPUNIT_ASSERT(!_fcc->isMixedEquivFareAmountCurrency());
  }

  void testIsMixedEquivFareAmountCurrencyReturnTrueWhenBaseCurrencyEqualToEquivCurrency()
  {
    populateCalcTotalsMap();
    _totals->equivCurrencyCode = _totals->convertedBaseFareCurrencyCode;
    CPPUNIT_ASSERT(_fcc->isMixedEquivFareAmountCurrency());
  }

  void testFindCalcTotalsReturnCalcTotalForFarePath()
  {
    populateCalcTotalsMap();
    CPPUNIT_ASSERT_EQUAL(_totals, _fcc->findCalcTotals(&_fp));

    CalcTotals* expectedCalcTotal = 0;
    _fcc->_calcTotalsMap.clear();
    CPPUNIT_ASSERT_EQUAL(expectedCalcTotal, _fcc->findCalcTotals(&_fp));
  }

  void testFindCalcTotalsReturnCalcTotalForPaxType()
  {
    populateCalcTotalsMap();
    CPPUNIT_ASSERT_EQUAL(_totals, _fcc->findCalcTotals(&_paxType));

    CalcTotals* expectedCalcTotal = 0;
    _fcc->_calcTotalsMap.clear();
    CPPUNIT_ASSERT_EQUAL(expectedCalcTotal, _fcc->findCalcTotals(&_paxType));
  }

  void testIsFareCalcTooLongReturnFalseWhenCalcTotalFclToLongEqualFalse()
  {
    populateCalcTotalsMap();
    CPPUNIT_ASSERT(!_fcc->isFareCalcTooLong());
  }

  void testIsFareCalcTooLongReturnTrueWhenCalcTotalFclToLongEqualTrue()
  {
    populateCalcTotalsMap();
    _totals->fclToLong = true;
    CPPUNIT_ASSERT(_fcc->isFareCalcTooLong());
  }

  void testGetIataSalesCode()
  {
    std::string expected = "SITI";
    _fp.intlSaleIndicator() = Itin::SITI;
    CPPUNIT_ASSERT_EQUAL(expected, _fcc->getIataSalesCode(&_fp));

    expected = "SITO";
    _fp.intlSaleIndicator() = Itin::SITO;
    CPPUNIT_ASSERT_EQUAL(expected, _fcc->getIataSalesCode(&_fp));

    expected = "SOTI";
    _fp.intlSaleIndicator() = Itin::SOTI;
    CPPUNIT_ASSERT_EQUAL(expected, _fcc->getIataSalesCode(&_fp));

    expected = "SOTO";
    _fp.intlSaleIndicator() = Itin::SOTO;
    CPPUNIT_ASSERT_EQUAL(expected, _fcc->getIataSalesCode(&_fp));

    expected = "UNKNOWN";
    _fp.intlSaleIndicator() = Itin::UNKNOWN;
    CPPUNIT_ASSERT_EQUAL(expected, _fcc->getIataSalesCode(&_fp));
  }

  void testCreateNetCalcTotalReturnTrueWhenNotNetFarePath()
  {
    CPPUNIT_ASSERT(_fcc->createNetCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
  }

  void testCreateNetCalcTotalReturnFalseWhenNetCalcTotalIsZero()
  {
    FareCalcCollectorStub fccStub;
    fccStub._returnZeroCalcTotal = true;
    NetFarePath netFp;
    _fp.netFarePath() = &netFp;
    CPPUNIT_ASSERT(!fccStub.createNetCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
  }

  void testCreateNetCalcTotalReturnTrueWhenNetCalcTotalNotZero()
  {
    FareCalcCollectorStub fccStub;
    NetFarePath netFp;
    _fp.netFarePath() = &netFp;
    CPPUNIT_ASSERT(fccStub.createNetCalcTotal(*_trx, &_fcConfig, &_fp, _totals));
    delete fccStub._total;
  }

  void testRequestedFareBasisIsTrue()
  {
    _trx->setRfblistOfSolution(true);
    CPPUNIT_ASSERT(dynamic_cast<AltFareCalculation*>(_fcc->createFareCalculation(_trx, &_fcConfig)));
  }

  void testRequestedFareBasisIsFalse()
  {
    _trx->setRfblistOfSolution(false);
    CPPUNIT_ASSERT(!(dynamic_cast<AltFareCalculation*>(_fcc->createFareCalculation(_trx, &_fcConfig))));
  }

  void testGetAdjustedSellingDifference()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    MoneyAmount m = _fcc->getAdjustedSellingDifference(*_trx, *_totals);
    CPPUNIT_ASSERT(m == 110.11);

    adjustedCalcTotal.equivFareAmount = 100.00;
    _totals->equivFareAmount = 110.11;
    m = _fcc->getAdjustedSellingDifference(*_trx, *_totals);
    CPPUNIT_ASSERT(m == -10.11);
  }

  void testProcessAdjustedSellingDiffInfoNoTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 1);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");
  }

  void testProcessAdjustedSellingDiffInfoMismatchTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    TaxRecord t1;
    t1._gstTaxInd = false;
    t1.setTaxAmount(10);

    TaxResponse taxResponse;
    taxResponse.taxRecordVector().push_back(&t1);

    FareCalc::FcTaxInfo& fcTaxInfo = _totals->getMutableFcTaxInfo();
    fcTaxInfo.initialize(_trx, _totals, &_fcConfig, &taxResponse);

    _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 1);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");
  }

  void testProcessAdjustedSellingDiffInfoNoGSTTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    TaxRecord t1, t2;
    t1._gstTaxInd = false;
    t1.setTaxAmount(10);
    t2._gstTaxInd = false;
    t2.setTaxAmount(20);

    TaxResponse taxResponse, adjTaxResponse;
    taxResponse.taxRecordVector().push_back(&t1);
    adjTaxResponse.taxRecordVector().push_back(&t2);

    FareCalc::FcTaxInfo& fcTaxInfo = _totals->getMutableFcTaxInfo();
    FareCalc::FcTaxInfo& adjFcTaxInfo = adjustedCalcTotal.getMutableFcTaxInfo();
    fcTaxInfo.initialize(_trx, _totals, &_fcConfig, &taxResponse);
    adjFcTaxInfo.initialize(_trx, &adjustedCalcTotal, &_fcConfig, &adjTaxResponse);

   _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 1);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");
  }

  void testProcessAdjustedSellingDiffInfoGSTTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    TaxRecord t1, t2, gst1, gst2;
    t1._gstTaxInd = false;
    t1.setTaxAmount(10);
    t2._gstTaxInd = false;
    t2.setTaxAmount(20);
    gst1._gstTaxInd = true;
    gst1.setTaxAmount(15);
    gst1.setTaxCode("G12");
    gst2._gstTaxInd = true;
    gst2.setTaxAmount(22);
    gst2.setTaxCode("G12");

    TaxResponse taxResponse, adjTaxResponse;
    taxResponse.taxRecordVector().push_back(&t1);
    taxResponse.taxRecordVector().push_back(&gst1);
    adjTaxResponse.taxRecordVector().push_back(&t2);
    adjTaxResponse.taxRecordVector().push_back(&gst2);

    FareCalc::FcTaxInfo& fcTaxInfo = _totals->getMutableFcTaxInfo();
    FareCalc::FcTaxInfo& adjFcTaxInfo = adjustedCalcTotal.getMutableFcTaxInfo();
    fcTaxInfo.initialize(_trx, _totals, &_fcConfig, &taxResponse);
    adjFcTaxInfo.initialize(_trx, &adjustedCalcTotal, &_fcConfig, &adjTaxResponse);

   _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 2);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].description == "G1");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].typeCode == "G");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].amount == "7.00");
  }

  void testProcessAdjustedSellingDiffInfoMultiGSTTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    TaxRecord t1, t2, gst1, gst2, gst3, gst4;
    t1._gstTaxInd = false;
    t1.setTaxAmount(10);
    t2._gstTaxInd = false;
    t2.setTaxAmount(20);
    gst1._gstTaxInd = true;
    gst1.setTaxAmount(15);
    gst1.setTaxCode("G12");
    gst2._gstTaxInd = true;
    gst2.setTaxAmount(22);
    gst2.setTaxCode("G12");
    gst3._gstTaxInd = true;
    gst3.setTaxAmount(33.33);
    gst3.setTaxCode("ZZ5");
    gst4._gstTaxInd = true;
    gst4.setTaxAmount(3.01);
    gst4.setTaxCode("ZZ5");

    TaxResponse taxResponse, adjTaxResponse;
    taxResponse.taxRecordVector().push_back(&t1);
    taxResponse.taxRecordVector().push_back(&gst1);
    taxResponse.taxRecordVector().push_back(&gst3);
    adjTaxResponse.taxRecordVector().push_back(&t2);
    adjTaxResponse.taxRecordVector().push_back(&gst2);
    adjTaxResponse.taxRecordVector().push_back(&gst4);

    FareCalc::FcTaxInfo& fcTaxInfo = _totals->getMutableFcTaxInfo();
    FareCalc::FcTaxInfo& adjFcTaxInfo = adjustedCalcTotal.getMutableFcTaxInfo();
    fcTaxInfo.initialize(_trx, _totals, &_fcConfig, &taxResponse);
    adjFcTaxInfo.initialize(_trx, &adjustedCalcTotal, &_fcConfig, &adjTaxResponse);

   _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 3);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].description == "G1");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].typeCode == "G");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].amount == "7.00");

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[2].description == "ZZ");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[2].typeCode == "G");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[2].amount == "-30.32");
  }

  void testProcessAdjustedSellingDiffInfoMismacthedGSTTax()
  {
    CalcTotals adjustedCalcTotal;
    _totals->adjustedCalcTotal = &adjustedCalcTotal;

    adjustedCalcTotal.equivFareAmount = 220.22;
    _totals->equivFareAmount = 110.11;

    _totals->equivCurrencyCode = "USD";
    adjustedCalcTotal.equivCurrencyCode = "USD";
    FarePath adjustedFp;
    adjustedCalcTotal.farePath = &adjustedFp;
    adjustedFp.calculationCurrency() = "USD";

    TaxRecord t1, t2, gst1, gst2, gst3, gst4;
    t1._gstTaxInd = false;
    t1.setTaxAmount(10);
    t2._gstTaxInd = false;
    t2.setTaxAmount(20);
    gst1._gstTaxInd = true;
    gst1.setTaxAmount(15);
    gst1.setTaxCode("G12");
    gst2._gstTaxInd = true;
    gst2.setTaxAmount(22);
    gst2.setTaxCode("G22");
    gst3._gstTaxInd = true;
    gst3.setTaxAmount(33.33);
    gst3.setTaxCode("ZZ5");
    gst4._gstTaxInd = true;
    gst4.setTaxAmount(3.01);
    gst4.setTaxCode("ZZ5");

    TaxResponse taxResponse, adjTaxResponse;
    taxResponse.taxRecordVector().push_back(&t1);
    taxResponse.taxRecordVector().push_back(&gst1);
    taxResponse.taxRecordVector().push_back(&gst3);
    adjTaxResponse.taxRecordVector().push_back(&t2);
    adjTaxResponse.taxRecordVector().push_back(&gst2);
    adjTaxResponse.taxRecordVector().push_back(&gst4);

    FareCalc::FcTaxInfo& fcTaxInfo = _totals->getMutableFcTaxInfo();
    FareCalc::FcTaxInfo& adjFcTaxInfo = adjustedCalcTotal.getMutableFcTaxInfo();
    fcTaxInfo.initialize(_trx, _totals, &_fcConfig, &taxResponse);
    adjFcTaxInfo.initialize(_trx, &adjustedCalcTotal, &_fcConfig, &adjTaxResponse);

   _fcc->processAdjustedSellingDiffInfo(*_trx, *_totals);

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo.size() == 2);
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].description == "ADJT AMT");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].typeCode == "J");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[0].amount == "110.11");

    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].description == "ZZ");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].typeCode == "G");
    CPPUNIT_ASSERT(adjustedCalcTotal.adjustedSellingDiffInfo[1].amount == "-30.32");
  }

  void testCreateAdjustedCalcTotal()
  {
    FarePath adjustedFP;
    adjustedFP.setAdjustedSellingFarePath();

    PaxTypeFare ptf;
    FareUsage fu;
    fu.paxTypeFare() = &ptf;
    PricingUnit pu;
    pu.fareUsage().push_back(&fu);
    adjustedFP.pricingUnit().push_back(&pu);
    PaxType paxType;
    paxType.paxType() = "ADT";
    ptf.paxType() = &paxType;
    adjustedFP.paxType() = &paxType;

    _totals->farePath = &_fp;

    CPPUNIT_ASSERT(_fcc->createAdjustedCalcTotal(*_trx, &_fcConfig, adjustedFP, _totals));
    CPPUNIT_ASSERT(_totals->adjustedCalcTotal != nullptr);
  }

  void testCopyItemsToAdjustedFP()
  {
    FarePath fp, adjustedFP;
    fp.adjustedSellingFarePath() = &adjustedFP;

    fp.baggageEmbargoesResponse() = "baggageEmbargoes";
    fp.baggageResponse() = "baggageResponse";
    fp.defaultValidatingCarrier() = "VC";
    fp.commissionAmount() = 88.8;
    fp.commissionPercent() = 77.7;
    fp.brandIndex() = 999;

    _fcc->copyItemsToAdjustedFP(*_trx, &fp);
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->baggageEmbargoesResponse() == "baggageEmbargoes");
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->baggageResponse() == "baggageResponse");
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->defaultValidatingCarrier() == "VC");
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->commissionAmount() == 88.8);
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->commissionPercent() == 77.7);
    CPPUNIT_ASSERT(fp.adjustedSellingFarePath()->brandIndex() == 999);
  }

  void testCollectPriceByCabinMessage()
  {
    _trx->getRequest()->setjumpUpCabinAllowed();
    _trx->getOptions()->cabin().setBusinessClass();
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = "CO";
    a1->bookedCabin().setFirstClass();
    a2->bookedCabin().setBusinessClass();
    a3->bookedCabin().setFirstClass();
    _itin->travelSeg() += a1, a2, a3;
    _fcc->collectPriceByCabinMessage(*_trx, _itin, _totals);
    CPPUNIT_ASSERT(!_totals->fcMessage.empty());
    CPPUNIT_ASSERT(_totals->fcMessage[0].messageText().find("CABIN NOT OFFERED") != std::string::npos);
  }

  void testCollectWPNCBPriceByCabinTrailerMessage()
  {
    populateCalcTotalsMap();
    _trx->paxType() += &_paxType;

    _trx->getRequest()->setjumpUpCabinAllowed();
    _trx->getOptions()->cabin().setBusinessClass();
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    AirSeg* a3 = _memHandle.create<AirSeg>();
    a1->carrier() = a2->carrier() = a3->carrier() = "CO";
    a1->bookedCabin().setFirstClass();
    a1->setBookingCode("Y ");
    a2->bookedCabin().setBusinessClass();
    a3->bookedCabin().setFirstClass();
    _itin->travelSeg() += a1, a2, a3;

    _totals->bookingCodeRebook.resize(_itin->travelSeg().size());
    _totals->bookingCodeRebook[0] = a1->getBookingCode();

    _fcc->collectWPNCBPriceByCabinTrailerMessage(*_trx, _itin);
    CPPUNIT_ASSERT(!_totals->fcMessage.empty());
    CPPUNIT_ASSERT(_totals->fcMessage[0].messageText().find("CABIN NOT OFFERED") != std::string::npos);
  }

protected:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  PricingOptions* _options;
  Agent* _agent;
  Customer* _customer;
  Billing* _billing;
  FareCalcCollector* _fcc;
  FareCalcConfig _fcConfig;
  FarePath _fp;
  CalcTotals* _totals;
  PaxType _paxType;
  FareCalc::FcTaxInfo* _fcTaxInfo;
  Itin* _itin;

  void populateCalcTotalsMap()
  {
    _paxType.number() = 1;
    _fp.paxType() = &_paxType;
    _fp.processed() = true;
    _totals->convertedBaseFareCurrencyCode = "USD";
    _totals->convertedBaseFareNoDec = 2;
    _totals->convertedBaseFare = 500.00;

    _totals->equivCurrencyCode = "INR";
    _totals->equivFareAmount = 40000;

    _totals->getMutableFcTaxInfo() = *_fcTaxInfo;

    _totals->farePath = &_fp;
    _fcc->_calcTotalsMap[&_fp] = _totals;
  }

};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCalcCollectorTest);
}
