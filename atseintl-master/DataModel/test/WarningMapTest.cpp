#include "DataModel/Fare.h"
#include "test/include/CppUnitHelperMacros.h"

#include <time.h>
#include <unistd.h>

using namespace std;

namespace tse
{

class WarningMapTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(WarningMapTest);
  CPPUNIT_TEST(testIsWarningSet);
  CPPUNIT_TEST(testIsWarningUnset);
  CPPUNIT_TEST(testIsCat5WarningSet);
  CPPUNIT_TEST(testIsCat5WarningUnset);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIsWarningSet()
  {
    Fare fare;
    WarningMap& wm = fare.warningMap();

    wm.set(WarningMap::cat2_warning_1, true);
    wm.set(WarningMap::cat2_warning_2, true);
    wm.set(WarningMap::cat4_warning);
    wm.set(WarningMap::cat11_warning);

    CPPUNIT_ASSERT(wm.isSet(WarningMap::cat2_warning_1));
    CPPUNIT_ASSERT(wm.isSet(WarningMap::cat2_warning_2));
    CPPUNIT_ASSERT(wm.isSet(WarningMap::cat4_warning));
    CPPUNIT_ASSERT(wm.isSet(WarningMap::cat11_warning));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat5_warning_1));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat5_warning_2));
  }

  void testIsWarningUnset()
  {
    Fare fare;
    WarningMap& wm = fare.warningMap();

    wm.set(WarningMap::cat2_warning_1, true);
    wm.set(WarningMap::cat2_warning_2, true);
    wm.set(WarningMap::cat2_warning_1, false);
    wm.set(WarningMap::cat2_warning_2, false);
    wm.set(WarningMap::cat4_warning, false);
    wm.set(WarningMap::cat11_warning, false);

    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat2_warning_1));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat2_warning_2));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat4_warning));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat11_warning));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat5_warning_1));
    CPPUNIT_ASSERT(!wm.isSet(WarningMap::cat5_warning_2));
  }

  void testIsCat5WarningSet()
  {
    Fare fare;
    WarningMap& wm = fare.warningMap();

    wm.setCat5WqWarning(1, true);
    wm.setCat5WqWarning(2, true);
    wm.setCat5WqWarning(3);
    wm.setCat5WqWarning(4);

    CPPUNIT_ASSERT(wm.isCat5WqWarning(1));
    CPPUNIT_ASSERT(wm.isCat5WqWarning(2));
    CPPUNIT_ASSERT(wm.isCat5WqWarning(3));
    CPPUNIT_ASSERT(wm.isCat5WqWarning(4));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(5));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(6));
  }

  void testIsCat5WarningUnset()
  {
    Fare fare;
    WarningMap& wm = fare.warningMap();

    wm.setCat5WqWarning(1, true);
    wm.setCat5WqWarning(2, true);
    wm.setCat5WqWarning(1, false);
    wm.setCat5WqWarning(2, false);
    wm.setCat5WqWarning(3, false);
    wm.setCat5WqWarning(4, false);

    CPPUNIT_ASSERT(!wm.isCat5WqWarning(1));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(2));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(3));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(4));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(5));
    CPPUNIT_ASSERT(!wm.isCat5WqWarning(6));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(WarningMapTest);
}
