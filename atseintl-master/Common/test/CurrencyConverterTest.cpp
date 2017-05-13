#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Common/CurrencyConverter.h"

#include "Common/Money.h"

namespace tse
{
class CurrencyConverterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyConverterTest);
  CPPUNIT_TEST(testRoundUp_TruncateForUsd);
  CPPUNIT_TEST(testRoundUp_RoundUpForMxn);
  CPPUNIT_TEST(testRoundUp_ZeroAmount);
  CPPUNIT_TEST(testRoundUp_ZeroRoundingFactor);
  CPPUNIT_TEST(testRoundDown_ZeroAmount);
  CPPUNIT_TEST(testRoundDown_ZeroRoundingFactor);
  CPPUNIT_TEST(testRoundNearest_ZeroAmount);
  CPPUNIT_TEST(testRoundNearest_ZeroRoundingFactor);
  CPPUNIT_TEST(testRoundNone_ZeroAmount);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  CurrencyConverter* _converter;

public:
  void setUp() { _converter = _memHandle.create<CurrencyConverter>(); }

  void tearDown() { _memHandle.clear(); }

  // TESTS
  void testRoundUp_TruncateForUsd()
  {
    Money money(1.01, "CAD");
    _converter->roundUp(money, 1.0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, money.value(), 0.01);
  }

  void testRoundUp_RoundUpForMxn()
  {
    Money money(1.01, "MXN");
    _converter->roundUp(money, 1.0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, money.value(), 0.01);
  }
  void testRoundUp_ZeroAmount()
  {
    Money money(0.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundUp(money, 1.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, money.value(), 0.01);
  }
  void testRoundUp_ZeroRoundingFactor()
  {
    Money money(1.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundUp(money, 0.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, money.value(), 0.01);
  }
  void testRoundDown_ZeroAmount()
  {
    Money money(0.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundDown(money, 1.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, money.value(), 0.01);
  }
  void testRoundDown_ZeroRoundingFactor()
  {
    Money money(1.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundDown(money, 0.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, money.value(), 0.01);
  }
  void testRoundNearest_ZeroAmount()
  {
    Money money(0.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundNearest(money, 1.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, money.value(), 0.01);
  }
  void testRoundNearest_ZeroRoundingFactor()
  {
    Money money(1.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundNearest(money, 0.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, money.value(), 0.01);
  }
  void testRoundNone_ZeroAmount()
  {
    Money money(0.0, "PLN");
    CPPUNIT_ASSERT(_converter->roundNone(money, 1.0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, money.value(), 0.01);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyConverterTest);
}
