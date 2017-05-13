// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
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

#include "Taxes/LegacyTaxes/TaxDisplayFormatter.h"

#include <string>
#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class TaxDisplayFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayFormatterTest);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testDefaultFmt);
  CPPUNIT_TEST(testMarginFmt);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void testCreation()
  {
    TaxDisplayFormatter* formatter = _memHandle.create<TaxDisplayFormatter>();

    CPPUNIT_ASSERT(0 != formatter);
  }

  void testDefaultFmt()
  {
    std::string s1 =
        "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBERS:\n"
        "  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";

    std::string s2 =
        "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBES: "
        "123456789012345678902345678901234567890123456789012345678902345678901234567890"
        "123456789012345678902345678901234567890";

    std::string s1_expected = "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBERS:\n"
                              "  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 2 3 4 5 6 7 8 9 0 \n"
                              "  1 2 3 4 5 6 7 8 9 0\n";

    std::string s2_expected = "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBES: \n"
                              "  1234567890123456789023456789012345678901234567890123456789\n"
                              "  0234567890123456789012345678901234567890234567890123456789\n"
                              "  0\n";
    TaxDisplayFormatter formatter;
    formatter.format(s1);
    // std::cout << std::endl << s1 <<std::endl;

    formatter.format(s2);
    // std::cout << std::endl << s2 <<std::endl;

    CPPUNIT_ASSERT_EQUAL(s1_expected, s1);
    CPPUNIT_ASSERT_EQUAL(s2_expected, s2);
  }

  void testMarginFmt()
  {
    std::string s1 =
        "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBERS:\n"
        "  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";

    std::string s2 =
        "1234567890123456789023456789012345678901234567890"
        "123456789012345678902345678901234567890123456789012345678902345678901234567890"
        "12345678901234567890234567890123456789";

    std::string s3 =
        "1234567890123456789023456789012345678901234567890"
        "123456789012345678902345678901234567890123456789012345678902345678901234567890"
        "123456789012345678902345678901234567890\n"
        "123456789012345678902345678901234567890123456789012345678902345678901234567890"
        "123456789012345678902345678901234567890123456789012345678902345678901234567890";

    std::string s1_expected = "* APPLICABLE TO THE FOLLOWING DEFAULT NUMBERS:\n"
                              "  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 2 3 4 5 6 7 8 9 0 \n"
                              "          1 2 3 4 5 6 7 8 9 0\n";

    std::string s2_expected = "123456789012345678902345678901234567890123456789012345678901\n"
                              "          23456789023456789012345678901234567890123456789023\n"
                              "          45678901234567890123456789012345678902345678901234\n"
                              "          56789\n";

    std::string s3_expected = "123456789012345678902345678901234567890123456789012345678901\n"
                              "          23456789023456789012345678901234567890123456789023\n"
                              "          45678901234567890123456789012345678902345678901234\n"
                              "          567890\n"
                              "123456789012345678902345678901234567890123456789012345678902\n"
                              "          34567890123456789012345678901234567890234567890123\n"
                              "          4567890123456789012345678902345678901234567890\n";

    TaxDisplayFormatter formatter;
    formatter.offsetWidth(10);
    formatter.format(s1);
    // std::cout << std::endl << s1 <<std::endl;

    formatter.format(s2);
    // std::cout << std::endl << s2 <<std::endl;

    formatter.format(s3);
    // std::cout << std::endl << s3 <<std::endl;

    CPPUNIT_ASSERT_EQUAL(s1_expected, s1);
    CPPUNIT_ASSERT_EQUAL(s2_expected, s2);
    CPPUNIT_ASSERT_EQUAL(s3_expected, s3);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayFormatterTest);
}
