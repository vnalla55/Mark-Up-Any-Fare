#include "DataModel/Billing.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "Common/Config/ConfigMan.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/PricingTrx.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class BillingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BillingTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST(testServiceName);
  CPPUNIT_TEST(testServiceNameOutOfRange);
  CPPUNIT_TEST(testBusinessFunctionHistorical);
  CPPUNIT_TEST(testBusinessFunction);
  CPPUNIT_TEST(testBusinessFunctionOutOfRange);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {
    Billing billing;
    billing.userPseudoCityCode() = "ABC";
    billing.userStation() = "ABC";
    billing.userBranch() = "ABC";
    billing.partitionID() = "ABC";
    billing.userSetAddress() = "ABC";
    billing.serviceName() = "ABC";
    billing.aaaCity() = "ABC";
    billing.aaaSine() = "ABC";
    billing.actionCode() = "ABC";
    CPPUNIT_ASSERT(billing.userPseudoCityCode() == "ABC");
    CPPUNIT_ASSERT(billing.userStation() == "ABC");
    CPPUNIT_ASSERT(billing.userBranch() == "ABC");
    CPPUNIT_ASSERT(billing.partitionID() == "ABC");
    CPPUNIT_ASSERT(billing.userSetAddress() == "ABC");
    CPPUNIT_ASSERT(billing.serviceName() == "ABC");
    CPPUNIT_ASSERT(billing.aaaCity() == "ABC");
    CPPUNIT_ASSERT(billing.aaaSine() == "ABC");
    CPPUNIT_ASSERT(billing.actionCode() == "ABC");
    CPPUNIT_ASSERT(billing.requestPath() == UNKNOWN_PATH);
  }

  void testServiceName()
  {
    Billing billing;
    for (uint16_t s = 0; s < Billing::MAX_SVC_IND; ++s)
      CPPUNIT_ASSERT(billing.getServiceName(static_cast<Billing::Service>(s)) != 0);
  }
  void testServiceNameOutOfRange()
  {
    Billing billing;
    CPPUNIT_ASSERT(strcmp(billing.getServiceName(Billing::MAX_SVC_IND), "") == 0);
  }
  void testBusinessFunctionHistorical()
  {
    // All these so I can setup Global::config::SERVER_TYPE.HISTORICAL
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setTrxType(PricingTrx::PRICING_TRX);
    std::string serverType("HISTORICAL");
    TestConfigInitializer::setValue("SERVER_TYPE", serverType, "APPLICATION_CONSOLE");

    Billing billing;
    for (uint16_t s = 0; s < Billing::MAX_SVC_IND; ++s)
      CPPUNIT_ASSERT(billing.getBusinessFunction(static_cast<Billing::Service>(s)) != 0);
  }
  void testBusinessFunction()
  {
    // All these so I can setup Global::config::SERVER_TYPE.HISTORICAL
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setTrxType(PricingTrx::PRICING_TRX);
    std::string serverType("TAX");
    TestConfigInitializer::setValue("SERVER_TYPE", serverType, "APPLICATION_CONSOLE");

    Billing billing;
    for (uint16_t s = 0; s < Billing::MAX_SVC_IND; ++s)
      CPPUNIT_ASSERT(billing.getBusinessFunction(static_cast<Billing::Service>(s)) != 0);
  }
  // Bad Apple!
  void testBusinessFunctionOutOfRange()
  {
    // All these so I can setup Global::config::SERVER_TYPE.HISTORICAL
    PricingTrx* trx = _memHandle.create<PricingTrx>();
    trx->setTrxType(PricingTrx::PRICING_TRX);
    std::string serverType("TAX");
    TestConfigInitializer::setValue("SERVER_TYPE", serverType, "APPLICATION_CONSOLE");

    Billing billing;
    CPPUNIT_ASSERT(strcmp(billing.getBusinessFunction(Billing::MAX_SVC_IND), "") == 0);
  }
}; // end class BillingTest
CPPUNIT_TEST_SUITE_REGISTRATION(BillingTest);
} // end namespace tse
