#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "Common/MetricsUtil.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "Common/TSELatencyData.h"
#include "Common/MetricsMan.h"
#include "test/include/MockGlobal.h"

using namespace tse;
using namespace std;

namespace tse
{
class MetricsUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MetricsUtilTest);
  CPPUNIT_TEST(testHeader);
  CPPUNIT_TEST(testLineItemHeader);
  CPPUNIT_TEST(testLineItem);
  CPPUNIT_TEST(testTrxLatencyTrx);
  CPPUNIT_TEST(testTrxLatencyPricingTrx);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp()
  {
    tse::MetricsMan* metricsMan = _memHandle.create<tse::MetricsMan>();
    metricsMan->initialize(tse::MetricsUtil::MAX_METRICS_ENUM);
    tse::MockGlobal::setMetricsMan(metricsMan);
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown()
  {
    _memHandle.clear();
    tse::MockGlobal::clear();
  }

  void testHeader()
  {
    ostringstream oss;

    CPPUNIT_ASSERT(MetricsUtil::header(oss, "MetricsUtilTest"));
    CPPUNIT_ASSERT(!oss.str().empty());
  }

  void testLineItemHeader()
  {
    ostringstream oss;

    CPPUNIT_ASSERT(MetricsUtil::lineItemHeader(oss));
    CPPUNIT_ASSERT(!oss.str().empty());
  }

  void testLineItem()
  {
    ostringstream oss;

    // Test bad values
    CPPUNIT_ASSERT(!MetricsUtil::lineItem(oss, -1));
    CPPUNIT_ASSERT(!MetricsUtil::lineItem(oss, 999999));

    // Test good value
    CPPUNIT_ASSERT(MetricsUtil::lineItem(oss, MetricsUtil::FCO_DIAG));
    CPPUNIT_ASSERT(!oss.str().empty());
  }

  void testTrxLatencyTrx()
  {
    ostringstream oss;
    MultiExchangeTrx trx;
    TSELatencyData(trx, "test");
    TSELatencyData(trx, "test2");

    CPPUNIT_ASSERT(MetricsUtil::trxLatency(oss, trx));
    CPPUNIT_ASSERT(!oss.str().empty());
  }

  void testTrxLatencyPricingTrx()
  {
    ostringstream oss;
    PricingTrx trx;
    trx.setRecordMemory(true);
    TSELatencyData(trx, "test");
    TSELatencyData(trx, "test2");

    CPPUNIT_ASSERT(MetricsUtil::trxLatency(oss, trx));
    CPPUNIT_ASSERT(!oss.str().empty());
    CPPUNIT_ASSERT(oss.str().find("MEMORY MEASUREMENTS") != string::npos);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MetricsUtilTest);
}
