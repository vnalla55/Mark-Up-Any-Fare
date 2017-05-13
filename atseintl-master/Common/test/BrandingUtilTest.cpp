#include "test/include/CppUnitHelperMacros.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "Common/BrandingUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

using namespace std;
namespace tse
{
class BrandingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrandingUtilTest);
  CPPUNIT_TEST(testIsPopulateFareDataMapVecTrue);
  CPPUNIT_TEST(testIsPopulateFareDataMapVecFalse);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fdTrx = _memHandle.create<FareDisplayTrx>();
  }
  void tearDown()
  {
    _memHandle.clear();
  }


  void testIsPopulateFareDataMapVecTrue()
  {
    CPPUNIT_ASSERT(BrandingUtil::isPopulateFareDataMapVec(*_fdTrx));
  }

  void testIsPopulateFareDataMapVecFalse()
  {
    _fdTrx->billing() = _memHandle.create<Billing>();
    _fdTrx->billing()->requestPath() = PSS_PO_ATSE_PATH;
    CPPUNIT_ASSERT(!BrandingUtil::isPopulateFareDataMapVec(*_fdTrx));
  }

  TestMemHandle _memHandle;
  FareDisplayTrx* _fdTrx;
};
CPPUNIT_TEST_SUITE_REGISTRATION(BrandingUtilTest);
}
