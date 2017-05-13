#include "Common/CurrencyRoundingUtil.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/Money.h"
#include "Common/StopWatch.h"
#include "Currency/test/MockDataHandle.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/StaticObjectPool.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <iostream>
#include <string>

#include <time.h>

using namespace std;

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CurrencyRoundingUtilRoundTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyRoundingUtilRoundTest);
  CPPUNIT_TEST(testRoundUp);
  CPPUNIT_TEST(testRoundDown);
  CPPUNIT_TEST(testRoundNearest);
  CPPUNIT_TEST(testRoundAll);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _mockDataHandle = _memHandle.create<CurrencyDataHandle>(); }

  void tearDown() { _memHandle.clear(); }

  void testRoundUp()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      PricingTrx trx;

      CurrencyRoundingUtil roundingUtil;

      double pesosAmt1 = 56.83;
      double pesosAmt2 = 56.83;
      CurrencyCode mxn("MXN");
      Money mexicanPesos(pesosAmt1, mxn);
      PricingRequest request;
      trx.setRequest(&request);
      trx.getRequest()->ticketingDT() = DateTime::localTime();

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Round Up ");
      LOG4CXX_DEBUG(_logger, "Mexican Pesos Rounding Rule      : UP");
      LOG4CXX_DEBUG(_logger, "Mexican Pesos Rounding Factor    : 1");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Original unrounded amount: " << mexicanPesos.value());

      bool roundRC = roundingUtil.round(mexicanPesos, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << mexicanPesos.value());

      roundRC = roundingUtil.round(pesosAmt2, mxn, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << pesosAmt2);

      double usdAmt1 = 1.089;
      double usdAmt2 = 1.089;

      CurrencyCode usdCode("USD");

      Money usd(usdAmt1, "USD");

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Round Up ");
      LOG4CXX_DEBUG(_logger, "USD Rounding Rule      : UP");
      LOG4CXX_DEBUG(_logger, "USD Rounding Factor    : .01");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Original unrounded amount: " << usd.value());

      roundRC = roundingUtil.round(usd, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << usd.value());

      roundRC = roundingUtil.round(usdAmt2, usdCode, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << usdAmt2);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testRoundDown()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      CurrencyRoundingUtil roundingUtil;
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);
      trx.getRequest()->ticketingDT() = DateTime::localTime();

      double amount = 56.83;

      Money brazilianReals(amount, "BRL");

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Round Down ");
      LOG4CXX_DEBUG(_logger, "Brazilian Real Rounding Rule      : DOWN");
      LOG4CXX_DEBUG(_logger, "Brazilian Real Rounding Factor    : .01");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Original unrounded amount: " << brazilianReals.value());

      bool roundRC = roundingUtil.round(brazilianReals, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << brazilianReals.value());

      double sgdAmount = 56.21;
      CurrencyCode sgd("SGD");

      LOG4CXX_DEBUG(_logger, "Original unrounded SGD amount: " << sgdAmount);

      roundRC = roundingUtil.round(sgdAmount, sgd, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded sgd amount: " << sgdAmount);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testRoundNearest()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyRoundingUtil roundingUtil;
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);
      trx.getRequest()->ticketingDT() = DateTime::localTime();

      double amount = 56.83;
      Money canadianDollars(amount, "CAD");

      DataHandle dataHandle;

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing Round Nearest ");
      LOG4CXX_DEBUG(_logger, "Canadian Dollars Rounding Rule      : Nearest");
      LOG4CXX_DEBUG(_logger, "Canadian Dollars Rounding Factor    : 1");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");

      LOG4CXX_DEBUG(_logger, "Original unrounded amount: " << canadianDollars.value());

      bool roundRC = roundingUtil.round(canadianDollars, trx);

      CPPUNIT_ASSERT(roundRC != false);

      LOG4CXX_DEBUG(_logger, "Rounded amount: " << canadianDollars.value());

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testRoundAll()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      CurrencyRoundingUtil roundingUtil;
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);
      trx.getRequest()->ticketingDT() = DateTime::localTime();

      Money usd2(100.09, "USD");
      Money usd("USD");

      bool convertRC = roundingUtil.round(usd2, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      convertRC = roundingUtil.round(usd, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != true);

      convertRC = roundingUtil.round(usd2, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      convertRC = roundingUtil.round(usd, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != true);

      convertRC = roundingUtil.round(usd2, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != false);

      convertRC = roundingUtil.round(usd, trx);

      LOG4CXX_DEBUG(_logger, "Rounded amount : " << usd2.value());

      CPPUNIT_ASSERT(convertRC != true);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

private:
  CurrencyDataHandle* _mockDataHandle;
  TestMemHandle _memHandle;
  static log4cxx::LoggerPtr _logger;
};

log4cxx::LoggerPtr
CurrencyRoundingUtilRoundTest::_logger(
    log4cxx::Logger::getLogger("atseintl.Currency.test.CurrencyRoundingUtilRoundTest"));

CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyRoundingUtilRoundTest);
};
