#include "test/include/CppUnitHelperMacros.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

#include "DBAccess/TaxCodeReg.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

using std::string;
namespace tse
{
class TaxCodeRegTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxCodeRegTest);
  CPPUNIT_TEST(amtIntToStringTest);
  CPPUNIT_TEST(amtIntPercentageToStringTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void amtIntToStringTest()
  {
    _amtInt.amt() = 0;
    _amtInt.nodec() = 0;

    string expected("0");
    assertAmtToString(expected);

    _amtInt.amt() = 1000000000;
    _amtInt.nodec() = 1;

    expected = "100000000";
    assertAmtToString(expected);

    _amtInt.amt() = 100;
    _amtInt.nodec() = 0;

    expected = "100";
    assertAmtToString(expected);

    _amtInt.nodec() = 1;

    expected = "10";
    assertAmtToString(expected);

    _amtInt.nodec() = 2;

    expected = "1";
    assertAmtToString(expected);

    _amtInt.nodec() = 3;

    expected = "0.1";
    assertAmtToString(expected);

    _amtInt.nodec() = 7;

    expected = "0.00001";
    assertAmtToString(expected);
  }

  void amtIntPercentageToStringTest()
  {
    string expected("0");
    _amtInt.amt() = 0;
    _amtInt.nodec() = 0;
    assertAmtPercentageToString(expected);

    _amtInt.amt() = 1;
    _amtInt.nodec() = 0;

    expected = "100";
    assertAmtPercentageToString(expected);

    _amtInt.nodec() = 1;

    expected = "10";
    assertAmtPercentageToString(expected);

    _amtInt.nodec() = 2;

    expected = "1";
    assertAmtPercentageToString(expected);

    _amtInt.nodec() = 3;

    expected = "0.1";
    assertAmtPercentageToString(expected);
  }

private:
  void assertAmtPercentageToString(const string& expected)
  {
    CPPUNIT_ASSERT_EQUAL(expected, _amtInt.percentageToString());
  }

  void assertAmtToString(const string& expected)
  {
    CPPUNIT_ASSERT_EQUAL(expected, _amtInt.toString());
  }

  TaxCodeReg::AmtInt _amtInt;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxCodeRegTest);
}
