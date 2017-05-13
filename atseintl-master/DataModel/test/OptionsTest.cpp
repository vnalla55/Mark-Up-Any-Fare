#include "DataModel/PricingOptions.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class OptionsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(OptionsTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {
    PricingOptions options;

    //   options.priorityPrice() = 12;
    //   options.priorityService() = 67;
    //   options.priorityTime() = 20;
    //   options.numberOfSolutions() = 1;
    //   options.directAmount() = 466;

    //    CPPUNIT_ASSERT(options.priorityPrice() == 12);
    //    CPPUNIT_ASSERT(options.priorityService() == 67);
    //    CPPUNIT_ASSERT(options.priorityTime() == 20);
    //    CPPUNIT_ASSERT(options.numberOfSolutions() == 1);
    //   CPPUNIT_ASSERT(options.directAmount() == 466);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(OptionsTest);
}
