#include "DataModel/CurrencyTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Billing.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class CurrencyTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyTrxTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {
    PricingRequest request;
    Billing billing;

    CurrencyTrx currencyTrx;

    currencyTrx.setRequest(&request);
    currencyTrx.billing() = &billing;
    currencyTrx.amount() = 123.00;
    currencyTrx.baseCountry() = "USA";
    currencyTrx.sourceCurrency() = "USD";
    currencyTrx.targetCurrency() = "GBP";
    currencyTrx.setRequest(&request);

    CPPUNIT_ASSERT(currencyTrx.getRequest());
    CPPUNIT_ASSERT(currencyTrx.billing() == &billing);
    CPPUNIT_ASSERT(currencyTrx.amount() == 123.00);
    CPPUNIT_ASSERT(currencyTrx.baseCountry() == "USA");
    CPPUNIT_ASSERT(currencyTrx.sourceCurrency() == "USD");
    CPPUNIT_ASSERT(currencyTrx.targetCurrency() == "GBP");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyTrxTest);
}
