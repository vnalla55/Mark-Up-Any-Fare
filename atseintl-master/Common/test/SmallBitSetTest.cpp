#include "test/include/CppUnitHelperMacros.h"

#include "Common/SmallBitSet.h"

using namespace tse;

namespace tse
{

class SmallBitSetTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SmallBitSetTest);
  CPPUNIT_TEST(testCopyConstructor);
  CPPUNIT_TEST(testIsSet);
  CPPUNIT_TEST(testIsSetNegative);
  CPPUNIT_TEST(testIsAllSet);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST(testSetOr);
  CPPUNIT_TEST(testSetFalseShouldClearBit);
  CPPUNIT_TEST(testCombineOr);
  CPPUNIT_TEST(testCombineAnd);
  CPPUNIT_TEST_SUITE_END();

  static const char BIT0 = 0;
  static const char BIT1 = 1;
  static const char BIT2 = 2;
  static const char BIT3 = 4;
  static const char BIT4 = 8;
  static const char BIT5 = 16;
  static const char BIT6 = 32;
  static const char BIT7 = 64;
  static const char BIT8 = 128;

public:
  void testCopyConstructor()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);
    SmallBitSet<char, char> set2(set1);

    CPPUNIT_ASSERT(!set1.isNull());
    CPPUNIT_ASSERT_EQUAL(set1.value(), set2.value());
  }

  void testIsSet()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);

    CPPUNIT_ASSERT(set1.isSet(BIT2));
  }

  void testIsSetNegative()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);
    CPPUNIT_ASSERT(!set1.isSet(BIT3));
  }

  void testIsAllSet()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);
    CPPUNIT_ASSERT(set1.isAllSet(BIT2 | BIT4));
  }

  void testClear()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT3 | BIT4);
    CPPUNIT_ASSERT(set1.isAllSet(BIT2 | BIT3 | BIT4));

    set1.clear(BIT3);

    CPPUNIT_ASSERT(!set1.isSet(BIT3));
    CPPUNIT_ASSERT(set1.isAllSet(BIT2 | BIT4));
  }

  void testSetOr()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);

    set1.set(BIT8, true);

    CPPUNIT_ASSERT(set1.isAllSet(BIT2 | BIT4 | BIT8));
  }

  void testSetFalseShouldClearBit()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);

    set1.set(BIT4, false);
    CPPUNIT_ASSERT(set1.isAllSet(BIT2));
  }

  void testCombineOr()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);
    SmallBitSet<char, char> set2(BIT4 | BIT8);

    set1.combine(set2, true);

    CPPUNIT_ASSERT(set1.isAllSet(BIT2 | BIT4 | BIT8));
  }

  void testCombineAnd()
  {
    SmallBitSet<char, char> set1(BIT2 | BIT4);
    SmallBitSet<char, char> set2(BIT4 | BIT8);

    set1.combine(set2, true);

    CPPUNIT_ASSERT(set1.isAllSet(BIT4));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SmallBitSetTest);
}
