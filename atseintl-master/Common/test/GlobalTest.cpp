#include "test/include/CppUnitHelperMacros.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/MetricsMan.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class TseServer
{
public:
  static tse::ConfigMan* getConfig() { return Global::_configMan; }
  static tse::MetricsMan* getMetrics() { return Global::_metricsMan; }

  static void setConfig(tse::ConfigMan* configMan) { Global::_configMan = configMan; }
  static void setMetrics(tse::MetricsMan* metricsMan) { Global::_metricsMan = metricsMan; }
};

class GlobalTest : public CppUnit::TestFixture
{
public:
  void tearDown() { _memHandle.clear(); }
private:
  CPPUNIT_TEST_SUITE(GlobalTest);
  CPPUNIT_TEST(testConfig);
  CPPUNIT_TEST(testMetrics);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

  void testConfig()
  {
    // Test set
    ConfigMan* dummy(0);;
    _memHandle.get(dummy);
    TseServer::setConfig(dummy);
    CPPUNIT_ASSERT(&Global::config() == dummy);
  }
  void testMetrics()
  {
    // Test set
    MetricsMan* dummy(0);
    _memHandle.get(dummy);

    TseServer::setMetrics(dummy);
    CPPUNIT_ASSERT(&Global::metrics() == dummy);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GlobalTest);
}
