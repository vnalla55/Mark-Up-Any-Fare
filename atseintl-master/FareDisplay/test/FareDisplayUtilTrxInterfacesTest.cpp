#include "test/include/CppUnitHelperMacros.h"
#include "Common/FareDisplayUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "Common/DateTime.h"
#include "test/include/MockGlobal.h"
#include "DataModel/PricingTrx.h"

using namespace std;
namespace tse
{
class FareDisplayUtilTrxInterfacesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayUtilTrxInterfacesTest);
  CPPUNIT_TEST(testGetFareDisplayTrxInterfaces);
  CPPUNIT_TEST_SUITE_END();

public:
  void testGetFareDisplayTrxInterfaces()
  {
    FareDisplayTrx fTrx;
    PricingTrx pTrx;
    FareDisplayTrx* ftrx(0);

    bool result_pass = FareDisplayUtil::getFareDisplayTrx(&fTrx, ftrx);
    CPPUNIT_ASSERT(result_pass == true);

    bool result_fail = FareDisplayUtil::getFareDisplayTrx(&pTrx, ftrx);
    CPPUNIT_ASSERT(result_fail == false);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayUtilTrxInterfacesTest);
}
