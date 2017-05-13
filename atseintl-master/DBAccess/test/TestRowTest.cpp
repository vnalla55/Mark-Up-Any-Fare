#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "DBAccess/test/TestRow.h"

using namespace std;

namespace tse
{
class TestRowTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TestRowTest);
  CPPUNIT_TEST(testInt);
  CPPUNIT_TEST(testString);
  CPPUNIT_TEST(testLongInt);
  CPPUNIT_TEST(testLongLong);
  CPPUNIT_TEST(testDate);
  CPPUNIT_TEST(testChar);
  CPPUNIT_TEST(testNull);
  CPPUNIT_TEST_SUITE_END();

public:
  static const int FIRST_COL = 0;
  static const int SECOND_COL = 1;

  TestRow row;

  void testInt()
  {
    int x(10);
    row.set(FIRST_COL, x);
    CPPUNIT_ASSERT_EQUAL(x, row.getInt(FIRST_COL));
  }

  void testString()
  {
    string s("hey");
    row.set(FIRST_COL, s);
    CPPUNIT_ASSERT_EQUAL(s.c_str(), row.getString(FIRST_COL));
  }

  void testLongInt()
  {
    long int x(9);
    row.set(FIRST_COL, x);
    CPPUNIT_ASSERT_EQUAL(x, row.getLong(FIRST_COL));
  }

  void testLongLong()
  {
    long long x(8);
    row.set(FIRST_COL, x);
    CPPUNIT_ASSERT_EQUAL(x, row.getLongLong(FIRST_COL));
  }

  void testChar()
  {
    char c('x');
    row.set(FIRST_COL, c);
    CPPUNIT_ASSERT_EQUAL(c, row.getChar(FIRST_COL));
  }

  void testDate()
  {
    DateTime date(2012, Feb, 29);
    DateTime date2(2014, Mar, 29);
    row.set(FIRST_COL, date);
    CPPUNIT_ASSERT_EQUAL(date, row.getDate(FIRST_COL));
    row.set(SECOND_COL, date2);
    CPPUNIT_ASSERT_EQUAL(date2, row.getDate(SECOND_COL));
  }

  void testNull()
  {
    CPPUNIT_ASSERT(!row.isNull(FIRST_COL));
    row.setNull(FIRST_COL);
    CPPUNIT_ASSERT(row.isNull(FIRST_COL));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestRowTest);
};
