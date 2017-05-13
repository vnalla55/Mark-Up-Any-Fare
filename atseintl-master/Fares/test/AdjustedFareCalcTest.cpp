#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Fares/AdjustedFareCalc.h"
#include "Rules/RuleConst.h"
#include "Common/Money.h"
#include "Common/CurrencyConversionFacade.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/FareInfo.h"
#include "test/include/MockGlobal.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
namespace
{
  class MockCurrencyConversionFacade : public CurrencyConversionFacade
  {
   public:

    bool convertCalc(
      Money& target,
      const Money& source,
      PricingTrx& trx,
      bool useInternationalRounding = false,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::OTHER,
      bool reciprocalRate = false,
      CurrencyCollectionResults* results = nullptr,
      CurrencyConversionCache* cache = nullptr,
      bool roundFare = true)
    {
      target = source;
      return true;
    }
  };
}

class AdjustedFareCalcTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AdjustedFareCalcTest);

  CPPUNIT_TEST(test_calcMoney_getFromPTF);
  CPPUNIT_TEST(test_calcMoney_fareAmount);
  CPPUNIT_TEST(test_calcMoney_setFareAmount);
  CPPUNIT_TEST(test_calcMoney_setCurrency);
  CPPUNIT_TEST(test_calcMoney_setRT);
  CPPUNIT_TEST(test_calcMoney_pickAmt);
  CPPUNIT_TEST(test_calcMoney_doPercent);
  CPPUNIT_TEST(test_calcMoney_doAdd);
  CPPUNIT_TEST(test_calcMoney_doMinus);
  CPPUNIT_TEST(test_calcMoney_getFromSpec);

  CPPUNIT_TEST_SUITE_END();

public:

  void test_calcMoney_getFromPTF()
  {
    // Set to non-round trip
    _fareInfo->owrt() = ONE_WAY_MAYNOT_BE_DOUBLED;
    _calcMoney->getFromPTF(*_paxTypeFare, true);
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), _paxTypeFare->fareAmount());

    // set back to round trip
    _fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _calcMoney->getFromPTF(*_paxTypeFare, true);
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), _paxTypeFare->originalFareAmount());
  }

  void test_calcMoney_fareAmount()
  {
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), 0.0);
  }

  void test_calcMoney_setFareAmount()
  {
    MoneyAmount fareAmount = 100.0;
    _calcMoney->setFareAmount(fareAmount);
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), fareAmount);
  }

  void test_calcMoney_setCurrency()
  {
    CurrencyCode cc = "USD";
    _calcMoney->setCurrency(cc);
    CPPUNIT_ASSERT_EQUAL(_calcMoney->code(), cc);
  }

  void test_calcMoney_setRT()
  {
    _calcMoney->setRT(true);
    CPPUNIT_ASSERT_EQUAL(_calcMoney->_isRT, true);
  }

  void test_calcMoney_pickAmt()
  {
    Money native("USD");
    Money nuc("GBP");

    int rc = _calcMoney->pickAmt(native, nuc, 111.1, "USD", 222.2, "GBP");
    CPPUNIT_ASSERT_EQUAL(rc, 1);
  }

  void test_calcMoney_doPercent()
  {
    MoneyAmount fareAmount = 1000.0;
    _calcMoney->setFareAmount(fareAmount);
    _calcMoney->doPercent(120.0); // +20 percent
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), 1200.0);
  }

  void test_calcMoney_doAdd()
  {
    MoneyAmount fareAmount = 100.0;
    _calcMoney->setFareAmount(fareAmount);
    _calcMoney->doAdd(10.0, "USD", 20.0, "GBP");
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), 110.0);
  }

  void test_calcMoney_doMinus()
  {
    MoneyAmount fareAmount = 100.0;
    _calcMoney->setFareAmount(fareAmount);
    _calcMoney->doMinus(15.0, "USD", 20.0, "GBP");
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), 85.0);
  }

  void test_calcMoney_getFromSpec()
  {
    MoneyAmount fareAmount = 100.0;
    _calcMoney->setFareAmount(fareAmount);
    _calcMoney->getFromSpec(75.0, "USD", 80.0, "GBP");
    CPPUNIT_ASSERT_EQUAL(_calcMoney->value(), 75.0);
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    DateTime now = DateTime::localTime();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->ticketingDT() = now;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(req);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    // DATABASE INFO for FareRetailerCalcDetailInfo
    _frCalcDetInfo = _memHandle.create<FareRetailerCalcDetailInfo>();
    _frCalcDetInfo->fareCalcInd() = 'C'; // RuleConst::NF_CALC_PERCENT
    _frCalcDetInfo->percent1() = 10.0;
    _frCalcDetInfo->percent2() = 20.0;
    _frCalcDetInfo->amount1() = 200.0;
    _frCalcDetInfo->amountCurrency1() = "USD";
    _frCalcDetInfo->amount2() = 300.0;
    _frCalcDetInfo->amountCurrency2() = "EUR";
    _frCalcDetInfo->percentNoDec1() = 10;
    _frCalcDetInfo->percentNoDec2() = 20;
    _frCalcDetInfo->amountNoDec1() = 200;
    _frCalcDetInfo->amountNoDec2() = 300;

    // FARE
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fare = _memHandle.create<Fare>();
    _fareInfo = _memHandle.create<FareInfo>();

    _fareInfo->carrier() = "AA";
    _fareInfo->owrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    _fareInfo->originalFareAmount() = 100.0;
    _fareInfo->fareAmount() = 50.0;
    _fare->setFareInfo(_fareInfo);

    _paxTypeFare->setFare(_fare);
    _paxTypeFare->nucOriginalFareAmount() = 300.0;
    _paxTypeFare->nucFareAmount() = 150.0;

    Itin* itin = _memHandle.create<Itin>();
    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->departureDT() = now;
    itin->travelSeg().push_back(travelSeg);
    itin->calculationCurrency() = "USD";
    itin->useInternationalRounding() = true;

    _trx->travelSeg().push_back(travelSeg);

    _calcMoney = _memHandle.insert(new CalcMoney(*_trx, *itin));
    _calcMoney->_ccf.reset(new MockCurrencyConversionFacade());

    _adjFareCalc = _memHandle.insert(new AdjustedFareCalc(*_trx, *itin));
    _adjFareCalc->load(*_frCalcDetInfo);

    _curMatch = "GBP";
    _curConvert = "EUR";
    _sellingCur = "USD";
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;

  PricingTrx* _trx;
  PricingOptions* _options;
  CurrencyCode _curMatch;
  CurrencyCode _curConvert;
  CurrencyCode _sellingCur;
  Fare* _fare;
  FareInfo* _fareInfo;
  PaxTypeFare* _paxTypeFare;
  AdjustedFareCalc* _adjFareCalc;
  FareRetailerCalcDetailInfo* _frCalcDetInfo;
  CalcMoney* _calcMoney;
};
CPPUNIT_TEST_SUITE_REGISTRATION(AdjustedFareCalcTest);
}
