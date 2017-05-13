//----------------------------------------------------------------------------
//	File: NegFareCalc_range.cpp
//
//	Author: Gern Blanston
//  	created:      04/18/2007
//  	description:  this is a unit test class for the routines in
//  	FareDisplayService that depend on how fares are merged
//
//  copyright sabre 2007
//
//          the copyright to the computer program(s) herein
//          is the property of sabre.
//          the program(s) may be used and/or copied only with
//          the written permission of sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Fares/NegFareCalc.h"
#include "Fares/FareController.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingOptions.h"
#include "Server/TseServer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/CppUnitHelperMacros.h"

#define IS_INTL true
#define IS_RT true
#define DEFAULT_PRICE 601.00
#define DEFAULT_CURRENCY "EUR"
#define NO_PERCENT 0

namespace tse
{
class FakeTseServer : public TseServer
{
public:
  FakeTseServer() : TseServer()
  {
    initializeGlobalConfigMan();
    initializeGlobal();
  }
  ~FakeTseServer() {};
};

class NegFareCalc_range : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegFareCalc_range);
  CPPUNIT_TEST(testRange_blank);
  CPPUNIT_TEST(testRange_percent0);
  CPPUNIT_TEST(testRange_percentGoodHi);
  CPPUNIT_TEST(testRange_percentGoodLo);
  CPPUNIT_TEST(testRange_percentBadHi);
  CPPUNIT_TEST(testRange_percentBadLo);
  CPPUNIT_TEST(testRange_percentGoodFixed1);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _tseServer = _memHandle.create<FakeTseServer>();
    _range = _memHandle.create<NegRange>();
    _price = _memHandle.create<NegPrice>();

    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();
    _trx->setOptions(_options);
    _itin = _memHandle.create<Itin>();
    _fm = _memHandle.create<FareMarket>();
    _fc = _memHandle.insert(new FareController(*_trx, *_itin, *_fm));
    _req = _memHandle.create<PricingRequest>();
    _trx->setRequest(_req);
    _req->ticketingDT() = DateTime::localTime();
  }
  void tearDown() { _memHandle.clear(); }

  void callAndAssertRange(bool expected = true)
  {
    bool actual = _range->isInRange(
        *_price, Money(DEFAULT_PRICE, DEFAULT_CURRENCY), DEFAULT_PRICE, *_fc, IS_INTL, IS_RT);
    CPPUNIT_ASSERT_EQUAL(expected, actual);
  }
  void setRangePercent(Percent min, Percent max)
  {
    _range->_ind = RuleConst::NF_RANGE_PERCENT;
    _range->_percent = min;

    _range->_max._ind = RuleConst::NF_RANGE_PERCENT;
    _range->_max._percent = max;
  }
  void setPricePercent(Percent p)
  {
    _price->_ind = RuleConst::NF_CALC_PERCENT;
    _price->_percent = p;
  }

  /**********************************************************************/

  void testRange_blank() { callAndAssertRange(); }

  void testRange_percent0()
  {
    _range->_ind = RuleConst::NF_RANGE_PERCENT;
    callAndAssertRange();
  }
  void testRange_percentGoodHi()
  {
    setRangePercent(110, 130);
    setPricePercent(130);
    callAndAssertRange();
  }
  void testRange_percentGoodLo()
  {
    setRangePercent(110, 130);
    setPricePercent(110);
    callAndAssertRange();
  }
  void testRange_percentBadHi()
  {
    setRangePercent(110, 130);
    setPricePercent(131);
    callAndAssertRange(false);
  }
  void testRange_percentBadLo()
  {
    setRangePercent(110, 130);
    setPricePercent(109);
    callAndAssertRange(false);
  }
  void testRange_percentGoodFixed1()
  {
    setRangePercent(100, 100);
    _price = _memHandle.insert(
        new NegPrice(RuleConst::NF_SPECIFIED, NO_PERCENT, DEFAULT_PRICE, DEFAULT_CURRENCY));

    callAndAssertRange();
  }

private:
  NegRange* _range;
  NegPrice* _price;

  PricingTrx* _trx;
  PricingOptions* _options;
  Itin* _itin;
  FareMarket* _fm;
  FakeTseServer* _tseServer;
  FareController* _fc;
  PricingRequest* _req;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(NegFareCalc_range);
}
