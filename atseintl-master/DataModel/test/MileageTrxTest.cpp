#include "DataModel/MileageTrx.h"
#include "DataModel/PricingRequest.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "DataModel/Fare.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class MileageTrxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageTrxTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST(testIsMileage);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {
    PricingRequest request;
    MileageTrx mileageTrx;
    mileageTrx.setRequest(&request);

    CPPUNIT_ASSERT_EQUAL(&request, mileageTrx.getRequest());
  }

  void testIsMileage()
  {
    Fare fare;

    CPPUNIT_ASSERT_MESSAGE("Error testing isRouting", !fare.isRouting());

    fare.setIsRouting();
    CPPUNIT_ASSERT_MESSAGE("Error testing isRouting", fare.isRouting());

    fare.setIsRouting(false);
    CPPUNIT_ASSERT_MESSAGE("Error testing isRouting", !fare.isRouting());

    return;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MileageTrxTest);
}
