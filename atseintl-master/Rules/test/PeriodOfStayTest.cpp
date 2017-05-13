#include "test/include/CppUnitHelperMacros.h"
#include <string>
#include "Common/DateTime.h"
#include "Rules/PeriodOfStay.h"

namespace tse
{
class PeriodOfStayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PeriodOfStayTest);
  CPPUNIT_TEST(testOneMonth);
  CPPUNIT_TEST(testOneDay);
  CPPUNIT_TEST(testOneHour);
  CPPUNIT_TEST(testOneMinute);
  CPPUNIT_TEST(testTwoMonths);
  CPPUNIT_TEST(testTwentyTwoDays);
  CPPUNIT_TEST(testHundredOneHours);
  CPPUNIT_TEST(test9HundredsOneMinutes);
  CPPUNIT_TEST_SUITE_END();

public:
  void createAndAssert(const std::string& period,
                       const std::string& unit,
                       const int intPeriod,
                       const int intUnit,
                       const std::string& stringPOS)
  {
    PeriodOfStay pos(period, unit);
    CPPUNIT_ASSERT(pos.isValid());
    CPPUNIT_ASSERT(!pos.isDayOfWeek());
    CPPUNIT_ASSERT_EQUAL(intPeriod, (int)pos);
    CPPUNIT_ASSERT_EQUAL(intUnit, pos.unit());
    CPPUNIT_ASSERT_EQUAL(stringPOS, pos.getPeriodOfStayAsString());
  }

  void testOneMonth() { createAndAssert("001", "Mb", 1, PeriodOfStay::MONTHS, "1 MONTH"); }

  void testOneDay() { createAndAssert("001", "Db", 1, PeriodOfStay::DAYS, "1 DAY"); }

  void testOneHour() { createAndAssert("001", "Hb", 1, PeriodOfStay::HOURS, "1 HOUR"); }

  void testOneMinute() { createAndAssert("001", "Nb", 1, PeriodOfStay::MINUTES, "1 MINUTE"); }

  void testTwoMonths() { createAndAssert("002", "Mb", 2, PeriodOfStay::MONTHS, "2 MONTHS"); }

  void testTwentyTwoDays() { createAndAssert("022", "Db", 22, PeriodOfStay::DAYS, "22 DAYS"); }

  void testHundredOneHours()
  {
    createAndAssert("101", "Hb", 101, PeriodOfStay::HOURS, "101 HOURS");
  }

  void test9HundredsOneMinutes()
  {
    createAndAssert("901", "Nb", 901, PeriodOfStay::MINUTES, "901 MINUTES");
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(PeriodOfStayTest);
};
