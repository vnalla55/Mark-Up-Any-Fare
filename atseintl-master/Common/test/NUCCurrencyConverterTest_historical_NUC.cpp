#include "test/include/CppUnitHelperMacros.h"
#include "Common/CurrencyConversionFacade.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "Common/Money.h"

#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace tse;

class MockNUCCurrencyConverter_historical : public NUCCurrencyConverter
{
public:
  bool convert(CurrencyConversionRequest& ccRequest,
               CurrencyCollectionResults* results,
               CurrencyConversionCache* cache = 0)
  {
    _requestedDate = ccRequest.ticketDate();
    return true;
  }

  DateTime _requestedDate;
};

class TestableCurrencyConversionFacade : public CurrencyConversionFacade
{
  MockNUCCurrencyConverter_historical mockNUCCurrencyConverter;

public:
  MockNUCCurrencyConverter_historical& nucConverter() override { return mockNUCCurrencyConverter; }
};

class NUCCurrencyConverterTest_historical_NUC : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(NUCCurrencyConverterTest_historical_NUC);
  CPPUNIT_TEST(testPricingDateUsage);
  CPPUNIT_TEST(testRexDateUsage);
  CPPUNIT_TEST_SUITE_END();

  TestableCurrencyConversionFacade* _ccf;
  PricingRequest _request;
  Money _target;
  Money _source;
  DateTime _today;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();
    _ccf = _memH.create<TestableCurrencyConversionFacade>();

  }

  void tearDown() { _memH.clear(); }

  NUCCurrencyConverterTest_historical_NUC()
    : _target("EUR"), _source(100, "NUC"), _today(DateTime::localTime())
  {
    _request.ticketingDT() = _today;
  }

protected:
  void testPricingDateUsage()
  {
    PricingTrx trx;
    trx.setRequest(&_request);

    _ccf->convertCalc(_target, _source, trx);

    CPPUNIT_ASSERT_EQUAL(_today, _ccf->nucConverter()._requestedDate);
  }

  void testRexDateUsage()
  {
    DateTime oldDate = DateTime(2007, 4, 15);

    RexPricingTrx rexTrx;
    rexTrx.setOriginalTktIssueDT() = oldDate;

    rexTrx.setRequest(&_request);

    _ccf->convertCalc(_target, _source, rexTrx);

    CPPUNIT_ASSERT_EQUAL(oldDate, _ccf->nucConverter()._requestedDate);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NUCCurrencyConverterTest_historical_NUC);
