//
// Copyright Sabre 2012-04-23
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "FareCalc/FareAmountsConverter.h"
#include "DataModel/RexPricingTrx.h"
#include "FareCalc/CalcTotals.h"
#include "DBAccess/FareCalcConfig.h"
#include "DataModel/FarePath.h"

namespace tse
{
namespace FareCalc
{

class FareAmountsConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareAmountsConverterTest);
  CPPUNIT_TEST(testDefineConversionDateReturnTicketinDateWhenNotARtransaction);
  CPPUNIT_TEST(testDefineConversionDateReturnTicketinDateWhenARtransactionNoExchangeReissue);
  CPPUNIT_TEST(
      testDefineConversionDateReturnNewItinROEDateWhenARtransactionApplyExchangeReissueNoSecondRoe);
  CPPUNIT_TEST(
      testDefineConversionDateReturnSecondROEDateWhenARtransactionApplyExchangeReissueYESSecondRoe);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  FarePath* _farePath;
  FareCalcConfig* _fcConfig;
  CalcTotals* _calcTotals;
  FareAmountsConverter* _converter;
  CurrencyCode _currencyCode;
  CurrencyNoDec _currencyNoDec;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _farePath = _memHandle.create<FarePath>();
    _fcConfig = _memHandle.create<FareCalcConfig>();
    _calcTotals = _memHandle.create<CalcTotals>();
    _converter = _memHandle.insert(new FareAmountsConverter(
        _trx, _farePath, _fcConfig, _calcTotals, _currencyCode, _currencyNoDec));
  }

  void tearDown() { _memHandle.clear(); }

  void testDefineConversionDateReturnTicketinDateWhenNotARtransaction()
  {
    RexPricingTrx rexTrx;
    _request->ticketingDT() = DateTime(2010, 3, 12);
    rexTrx.setRequest(_request);
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    CPPUNIT_ASSERT_EQUAL(rexTrx.ticketingDate(),
                         _converter->defineConversionDate(rexTrx, *_farePath));
  }

  void testDefineConversionDateReturnTicketinDateWhenARtransactionNoExchangeReissue()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    _request->ticketingDT() = DateTime(2010, 3, 12);
    rexTrx.setRequest(_request);
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    CPPUNIT_ASSERT_EQUAL(rexTrx.ticketingDate(),
                         _converter->defineConversionDate(rexTrx, *_farePath));
  }

  void
  testDefineConversionDateReturnNewItinROEDateWhenARtransactionApplyExchangeReissueNoSecondRoe()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    _request->ticketingDT() = DateTime(2010, 3, 12);
    rexTrx.setRequest(_request);
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    rexTrx.newItinROEConversionDate() = DateTime(2010, 3, 10);
    rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 11);
    _farePath->useSecondRoeDate() = false;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinROEConversionDate(),
                         _converter->defineConversionDate(rexTrx, *_farePath));
  }

  void
  testDefineConversionDateReturnSecondROEDateWhenARtransactionApplyExchangeReissueYESSecondRoe()
  {
    RexPricingTrx rexTrx;
    rexTrx.trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    rexTrx.setRexPrimaryProcessType('A');
    _request->ticketingDT() = DateTime(2010, 3, 12);
    rexTrx.setRequest(_request);
    rexTrx.setOriginalTktIssueDT() = DateTime(2010, 3, 9);
    rexTrx.newItinROEConversionDate() = DateTime(2010, 3, 10);
    rexTrx.newItinSecondROEConversionDate() = DateTime(2010, 3, 11);
    _farePath->useSecondRoeDate() = true;
    CPPUNIT_ASSERT_EQUAL(rexTrx.newItinSecondROEConversionDate(),
                         _converter->defineConversionDate(rexTrx, *_farePath));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareAmountsConverterTest);
}
}
