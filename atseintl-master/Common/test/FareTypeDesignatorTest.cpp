#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/FareTypeDesignator.h"

namespace tse
{

class FareTypeDesignatorTest : public CppUnit::TestFixture
{
public:
  CPPUNIT_TEST_SUITE(FareTypeDesignatorTest);
  CPPUNIT_TEST(testSetFareTypeDesignator);
  CPPUNIT_TEST_SUITE_END();

  void testSetFareTypeDesignator()
  {
    FareTypeDesignator f;
    f.setFareTypeDesignator(35); // old value for economy, use new values
    CPPUNIT_ASSERT(f.isFTDEconomy());
    CPPUNIT_ASSERT_EQUAL(f.fareTypeDesig(), 35); // conversion to new value
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareTypeDesignatorTest);
}
