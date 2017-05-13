#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "Fares/NegFareCalc.h"
#include "Rules/RuleConst.h"
#include "Common/Money.h"
#include "Fares/FareController.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/NegFareCalcInfo.h"
#include "test/include/MockGlobal.h"
#include "test/testdata/TestLocFactory.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
namespace
{
class MockFareController : public FareController
{
public:
  MockFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
    : FareController(trx, itin, fareMarket)
  {
  }

  char matchCurrency(Money& target, const bool isIntl, const Money& src1, const Money& src2)
  {
    if (target.code() == src1.code() || target.code() == src2.code() || src1.value() == 0.0)
    {
      return FareController::matchCurrency(target, isIntl, src1, src2);
    }

    MoneyAmount amt1 = src1.value() * 0.75;
    MoneyAmount amt2 = src2.value() * 1.5;

    if (amt2 < amt1)
    {
      target.value() = amt2;
      return 2;
    }
    target.value() = amt1;
    return 1;
  }
};
}

class NegFareCalcTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegFareCalcTest);
  CPPUNIT_TEST(testIsMatchFareLoc_Fail);
  CPPUNIT_TEST(testIsMatchFareLoc_Pass_BlankDirectionality);

  CPPUNIT_TEST(testIsValidCat_NetN);
  CPPUNIT_TEST(testIsValidCat_NetS);
  CPPUNIT_TEST(testIsValidCat_NetSSellR);

  CPPUNIT_TEST(testGetPrice_Cur1);
  CPPUNIT_TEST(testGetPrice_Cur2);
  CPPUNIT_TEST(testGetPrice_CurMacth);
  CPPUNIT_TEST(testGetPrice_FareIndC);
  CPPUNIT_TEST(testGetPrice_FareIndA);
  CPPUNIT_TEST(testGetPrice_FareIndM);
  CPPUNIT_TEST_SUITE_END();

public:
  //-----------------------------------------------------------------
  // test IsMatchFareLoc
  //-----------------------------------------------------------------
  void testIsMatchFareLoc_Fail() { CPPUNIT_ASSERT(!_nfCalc->isMatchFareLoc(*_trx, *_paxTypeFare)); }

  void testIsMatchFareLoc_Pass_BlankDirectionality()
  {
    _nfCalc->_directionality = ' ';
    CPPUNIT_ASSERT(_nfCalc->isMatchFareLoc(*_trx, *_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // test IsValidCat
  //-----------------------------------------------------------------
  void testIsValidCat_NetN()
  {
    // net/sell match
    _nfCalc->_netSellingInd = 'N';
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::SELLING_FARE, true));
    CPPUNIT_ASSERT(_nfCalc->isValidCat(RuleConst::SELLING_FARE, false));
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE, true));
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE_UPD, true));
  }

  void testIsValidCat_NetS()
  {
    _nfCalc->_netSellingInd = 'S';
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::SELLING_FARE, false));
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE, true));
    CPPUNIT_ASSERT(_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE, false));
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE_UPD, true));
  }

  void testIsValidCat_NetSSellR()
  {
    _nfCalc->_netSellingInd = 'S';
    _nfCalc->_selling._ind = 'R';
    CPPUNIT_ASSERT(_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE_UPD, true));
    CPPUNIT_ASSERT(!_nfCalc->isValidCat(RuleConst::NET_SUBMIT_FARE_UPD, false));
  }

  //-----------------------------------------------------------------
  // test GetPrice
  //-----------------------------------------------------------------
  void testGetPrice_Cur1()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        350.0, _nfCalc->_selling.getPrice(Money("GBP"), *_fc, true, &_curConvert), 0.01);
  }

  void testGetPrice_Cur2()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        375.0, _nfCalc->_selling.getPrice(Money("EUR"), *_fc, true, &_curConvert), 0.01);
  }

  void testGetPrice_CurMacth()
  {
    // range check
    _nfCalc->_range._ind = 'P';
    _nfCalc->_range._max._ind = 'P';
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        350.0, _nfCalc->_selling.getPrice(Money(300, _curMatch), *_fc, true), 0.01);
  }

  void testGetPrice_FareIndC()
  {
    _nfCalc->_selling._ind = 'C';
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        910.0, _nfCalc->_selling.getPrice(Money(9100, _sellingCur), *_fc, true, nullptr), 0.01);
  }

  void testGetPrice_FareIndA()
  {
    _nfCalc->_selling._ind = 'A';
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1410.0, _nfCalc->_selling.getPrice(Money(9100, _sellingCur), *_fc, true, nullptr), 0.01);
  }

  void testGetPrice_FareIndM()
  {
    _nfCalc->_selling._ind = 'M';
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        410.0, _nfCalc->_selling.getPrice(Money(9100, _sellingCur), *_fc, true, nullptr), 0.01);
  }

  //-----------------------------------------------------------------
  // setUp()
  //-----------------------------------------------------------------
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    // TRX
    _fm = _memHandle.create<FareMarket>();
    _fm->origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");
    _fm->destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _fm->boardMultiCity() = _fm->origin()->loc();
    _fm->offMultiCity() = _fm->destination()->loc();

    DateTime now = DateTime::localTime();
    PricingRequest* req = _memHandle.create<PricingRequest>();
    req->ticketingDT() = now;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(req);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);

    // DATABASE INFO
    _nfInfo = _memHandle.create<NegFareCalcInfo>();
    _nfInfo->directionality() = 'X';
    _nfInfo->netSellingInd() = 'S';
    _nfInfo->fareInd() = 'S';
    _nfInfo->sellingPercent() = 10.0;
    _nfInfo->sellingFareAmt1() = 500.0;
    _nfInfo->sellingCur1() = "USD";
    _nfInfo->sellingFareAmt2() = 350.0;
    _nfInfo->sellingCur2() = "GBP";
    _nfInfo->calcPercentMin() = 100.0;
    _nfInfo->calcPercentMax() = 150.0;
    // for testing swapped locs
    _nfInfo->loc1().locType() = LOCTYPE_CITY;
    _nfInfo->loc1().loc() = _fm->destination()->loc();
    _nfInfo->loc2().locType() = LOCTYPE_CITY;
    _nfInfo->loc2().loc() = _fm->origin()->loc();

    // FARE
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _paxTypeFare->fareMarket() = _fm;
    Fare* fare = _memHandle.create<Fare>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    fareInfo->carrier() = "AA";
    fare->setFareInfo(fareInfo);
    _paxTypeFare->setFare(fare);

    _nfCalc = _memHandle.create<NegFareCalc>();
    _nfCalc->load(_nfInfo);

    Itin* itin = _memHandle.create<Itin>();
    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->departureDT() = now;
    itin->travelSeg().push_back(travelSeg);
    _fm->travelSeg().push_back(travelSeg);
    _trx->travelSeg().push_back(travelSeg);

    _fc = _memHandle.insert(new MockFareController(*_trx, *itin, *_fm));

    _curMatch = "GBP";
    _curConvert = "EUR";
    _sellingCur = "USD";
    //       _base1 = _memHandle.insert(new Money("GBP"));
    //       _base2 = _memHandle.insert(new Money("EUR"));
  }

  //-----------------------------------------------------------------
  // tearDown()
  //-----------------------------------------------------------------
  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;

  PricingTrx* _trx;
  PricingOptions* _options;
  NegFareCalcInfo* _nfInfo;
  FareMarket* _fm;
  FareController* _fc;
  CurrencyCode _curMatch;
  CurrencyCode _curConvert;
  CurrencyCode _sellingCur;
  PaxTypeFare* _paxTypeFare;
  NegFareCalc* _nfCalc;
  //     Money           *_base1;
  //     Money           *_base2;
};
CPPUNIT_TEST_SUITE_REGISTRATION(NegFareCalcTest);
}
