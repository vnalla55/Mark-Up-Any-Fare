// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/RangeUtils.h"

#include <gtest/gtest.h>
#include "test/include/CppUnitHelperMacros.h"

namespace tax
{
class RangeUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RangeUtilsTest);

  CPPUNIT_TEST(testProperRange);
  CPPUNIT_TEST(testEqual);
  CPPUNIT_TEST(testIntersection);

  CPPUNIT_TEST_SUITE_END();
public:
  void testProperRange()
  {
    ProperRange range = ProperRange(5, 2);
    ASSERT_EQ(2, range.begin);
    ASSERT_EQ(5, range.end);
    ASSERT_FALSE(range.empty);
  }

  void testEqual()
  {
    ASSERT_TRUE(Range(1, 3) == Range(1, 3));
    ASSERT_TRUE(Range(3, 2) == Range(7, 4));
    ASSERT_FALSE(Range(1, 4) == Range(2, 4));
  }

  void testLessThanOrEqual()
  {
    ASSERT_TRUE(Range(1, 3) <= Range(0, 5));
    ASSERT_TRUE(Range(4, 1) <= Range(7, 13));
    ASSERT_TRUE(Range(3, 2) <= Range(7, 4));
    ASSERT_TRUE(Range(3, 5) <= Range(3, 5));
    ASSERT_FALSE(Range(2, 5) <= Range(1, 4));
    ASSERT_FALSE(Range(1, 2) <= Range(7, 4));
  }

  void testIntersection()
  {
    ASSERT_EQ(Range(2, 4), Range(1, 4) & Range(2, 19));
    ASSERT_TRUE((Range(1, 2) & Range(4, 7)).empty);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RangeUtilsTest);
}
