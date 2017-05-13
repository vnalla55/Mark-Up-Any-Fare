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
#include "test/include/CppUnitHelperMacros.h"

#include "Rules/MathUtils.h"

namespace tax
{

class MathUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MathUtilsTest);

  CPPUNIT_TEST(testAdjustDecimalA0D0);
  CPPUNIT_TEST(testAdjustDecimalA100D0);
  CPPUNIT_TEST(testAdjustDecimalA10D1);
  CPPUNIT_TEST(testAdjustDecimalA0D1);
  CPPUNIT_TEST(testAdjustDecimalA10D2);
  CPPUNIT_TEST(testAdjustDecimalA10D20);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testAdjustDecimalA0D0()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), MathUtils::adjustDecimal(0, 0));
  }

  void testAdjustDecimalA100D0()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(100), MathUtils::adjustDecimal(100, 0));
  }

  void testAdjustDecimalA10D1()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1), MathUtils::adjustDecimal(10, 1));
  }
  void testAdjustDecimalA0D1()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), MathUtils::adjustDecimal(0, 1));
  }

  void testAdjustDecimalA10D2()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1,10), MathUtils::adjustDecimal(10, 2));
  }
  void testAdjustDecimalA10D20()
  {
    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(1,1000000000), MathUtils::adjustDecimal(1000000000, 18));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MathUtilsTest);

} // namespace tax
