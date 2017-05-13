// Copyright Sabre 2014
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.


#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <string>

#include "FareCalc/FcDispItem.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class FcDispItemTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FcDispItemTest);
  CPPUNIT_TEST(testConvertAmount);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testConvertAmount()
  {
    using namespace std;

    MoneyAmount amount1 = 4.56789;
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 2, 0), string("4.57"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 0, 2), string("5.00"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 0, 0), string("5"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 2, 1), string("4.5"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 1, 2), string("4.60"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount1, 2, 2), string("4.57"));

    MoneyAmount amount2 = 1679.489990234375;
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount2, 2, 0), string("1679.49"));

    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(floor(amount2 * 100.0) / 100.0, 2, 2),
                         string("1679.48"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(floor(amount2 * 100.0) / 100.0, 4, 2),
                         string("1679.48"));
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(floor(amount2 * 100.0) / 100.0, 2, 4),
                         string("1679.4800"));

    MoneyAmount amount3 = 116.567999999998;
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount3, 6, 0), string("116.568"));

    MoneyAmount amount4 = 3.00;
    CPPUNIT_ASSERT_EQUAL(FcDispItem::convertAmount(amount4, 2, 0), string("3"));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(FcDispItemTest);

} // namespace tse
