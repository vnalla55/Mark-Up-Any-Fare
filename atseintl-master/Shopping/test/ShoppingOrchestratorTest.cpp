#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockTseServer.h"
#include "DataModel/MetricsTrx.h"
#include "Shopping/ShoppingOrchestrator.h"
#include "DataModel/ShoppingTrx.h"

#include <iostream>

namespace tse
{

class ShoppingOrchestratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShoppingOrchestratorTest);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST_SUITE_END();

public:
  void testProcess()
  {
    MockTseServer server;
    ShoppingOrchestrator orch(server);

    ShoppingTrx shopping;
    const bool shoppingRes = orch.process(shopping);

    CPPUNIT_ASSERT_EQUAL(shoppingRes, true);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ShoppingOrchestratorTest);
}
