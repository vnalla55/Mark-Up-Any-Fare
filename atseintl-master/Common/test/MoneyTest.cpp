#include "test/include/CppUnitHelperMacros.h"

#include "Common/Money.h"
#include <iostream>

using namespace tse;
using namespace std;

namespace tse
{

class MoneyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MoneyTest);
  CPPUNIT_TEST(testToStringNucAppendZeros);
  CPPUNIT_TEST(testToStringNucRoundUp);
  CPPUNIT_TEST(testToStringNucRoundDown);
  CPPUNIT_TEST(testToStringCurrNoDec0);
  CPPUNIT_TEST(testToStringCurrNoDec2);
  CPPUNIT_TEST(testToString_NUC);
  CPPUNIT_TEST(testToString_JPY_NoDec);
  CPPUNIT_TEST_SUITE_END();

public:
  //-------------------------------------------------------------------
  // testToStringNuc()
  //-------------------------------------------------------------------
  void testToStringNucAppendZeros()
  {
    Money nuc1(123, "NUC");
    CPPUNIT_ASSERT_EQUAL(std::string("NUC123.00"), nuc1.toStringNuc());
  }

  void testToStringNucRoundUp()
  {
    Money nuc2(123.456, "NUC");
    CPPUNIT_ASSERT_EQUAL(std::string("NUC123.46"), nuc2.toStringNuc());
  }

  void testToStringNucRoundDown()
  {
    Money nuc3(123.321, "NUC");
    CPPUNIT_ASSERT_EQUAL(std::string("NUC123.32"), nuc3.toStringNuc());
  }

  void testToStringCurrNoDec0()
  {
    Money curr1(123.45, "JPY");
    CPPUNIT_ASSERT_EQUAL(std::string("JPY123"), curr1.toStringCurr(CurrencyNoDec(0)));
  }

  void testToStringCurrNoDec2()
  {
    Money curr2(123.456, "CAD");
    CPPUNIT_ASSERT_EQUAL(std::string("CAD123.46"), curr2.toStringCurr(CurrencyNoDec(2)));
  }

  void testToString_NUC()
  {
    Money curr1(123.4, "NUC");
    CPPUNIT_ASSERT_EQUAL(std::string("NUC123.40"), curr1.toString(CurrencyNoDec(0)));
  }

  void testToString_JPY_NoDec()
  {
    Money curr1(123.45, "JPY");
    CPPUNIT_ASSERT_EQUAL(std::string("JPY123"), curr1.toString(CurrencyNoDec(0)));
    CPPUNIT_ASSERT_EQUAL(std::string("JPY123.5"), curr1.toString(CurrencyNoDec(1)));
    CPPUNIT_ASSERT_EQUAL(std::string("JPY123.45"), curr1.toString(CurrencyNoDec(2)));
    CPPUNIT_ASSERT_EQUAL(std::string("JPY123.450"), curr1.toString(CurrencyNoDec(3)));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(MoneyTest);
}
