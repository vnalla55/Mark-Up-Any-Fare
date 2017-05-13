#include "DataModel/TaxOverride.h"
#include "Common/TseConsts.h"
#include "DBAccess/Loc.h"
#include "test/include/CppUnitHelperMacros.h"

using namespace std;

namespace tse
{
class TaxOverrideTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOverrideTest);
  CPPUNIT_TEST(testSetGet);
  CPPUNIT_TEST_SUITE_END();

public:
  //---------------------------------------------------------------------
  // testSet()
  //---------------------------------------------------------------------
  void testSetGet()
  {

    TaxOverride taxOverride;

    taxOverride.taxAmt() = 999;
    taxOverride.taxCode() = "ABC";

    CPPUNIT_ASSERT(taxOverride.taxAmt() == 999);
    CPPUNIT_ASSERT(taxOverride.taxCode() == "ABC");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxOverrideTest);
}
