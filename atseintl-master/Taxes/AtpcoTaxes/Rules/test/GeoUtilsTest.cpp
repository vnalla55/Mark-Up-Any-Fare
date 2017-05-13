// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <stdexcept>
#include <set>
#include "test/include/CppUnitHelperMacros.h"
#include "Rules/GeoUtils.h"

using namespace std;

namespace tax
{

class GeoUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(GeoUtilsTest);
  CPPUNIT_TEST(testIsHawaii);
  CPPUNIT_TEST(testIsAlaska);
  CPPUNIT_TEST_SUITE_END();

public:
  void testIsHawaii()
  {
    CPPUNIT_ASSERT(!GeoUtils::isHawaii(type::StateProvinceCode("AA")));
    CPPUNIT_ASSERT(!GeoUtils::isHawaii(GeoUtils::ALASKA));
    CPPUNIT_ASSERT(GeoUtils::isHawaii(GeoUtils::HAWAII));
  }

  void testIsAlaska()
  {
    CPPUNIT_ASSERT(!GeoUtils::isAlaska(type::StateProvinceCode("AA")));
    CPPUNIT_ASSERT(!GeoUtils::isAlaska(GeoUtils::HAWAII));
    CPPUNIT_ASSERT(GeoUtils::isAlaska(GeoUtils::ALASKA));
  }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeoUtilsTest);
} // namespace tax
