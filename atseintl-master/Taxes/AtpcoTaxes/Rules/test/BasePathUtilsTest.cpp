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

#include "Rules/BasePathUtils.h"

namespace tax
{

class BasePathUtilsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BasePathUtilsTest);

  CPPUNIT_TEST(testBaseFareAmoutEmpty);
  CPPUNIT_TEST(testBaseFareAmoutTwoFares);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testBaseFareAmoutEmpty()
  {
    FarePath farePath;

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(0), tax::BasePathUtils::baseFareAmount(farePath));
  }
  void testBaseFareAmoutTwoFares()
  {

    FarePath farePath;
    Fare fare;
    fare.amount() = 100;
    farePath.fareUsages().resize(2);
    farePath.fareUsages()[0].fare() = &fare;
    farePath.fareUsages()[1].fare() = &fare;

    CPPUNIT_ASSERT_EQUAL(type::MoneyAmount(200), tax::BasePathUtils::baseFareAmount(farePath));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BasePathUtilsTest);

} // namespace tax
