#include "test/include/CppUnitHelperMacros.h"

#include "Common/DateTime.h"

using namespace std;

namespace tse
{

class DateTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(DateTest);
  CPPUNIT_TEST(testBasicGetters);
  CPPUNIT_TEST(testIsLeapYearShouldBeTrue);
  CPPUNIT_TEST(testIsLeapYearShouldBeFalse);
  CPPUNIT_TEST(testIsBetween1);
  CPPUNIT_TEST(testConstructors);
  CPPUNIT_TEST(testBaseDateConstructor);
  CPPUNIT_TEST(testOpenDateConstructor);
  CPPUNIT_TEST(testFromMilitaryTimeDateConstructor);
  CPPUNIT_TEST(testStringConversionsShouldBeCorrectlyFormatted);
  CPPUNIT_TEST(testIsFutureDateShouldBeTrue);
  CPPUNIT_TEST(testIsFutureDateShouldBeFalse);
  CPPUNIT_TEST(testEarlierShouldAlwaysReturnSecondDateWhenFirstDateIsOpenDate);
  CPPUNIT_TEST(testEarlierShouldReturnFirstDateWhenSecondDateIsOpenDate);
  CPPUNIT_TEST(testEarlierShouldReturnEarlierDateIfNeitherSetToOpenDate);
  CPPUNIT_TEST(testLaterShouldAlwaysReturnSecondDateWhenFirstDateIsOpenDate);
  CPPUNIT_TEST(testLaterShouldReturnFirstDateWhenSecondDateIsOpenDate);
  CPPUNIT_TEST(testLaterShouldReturnEarlierDateIfNeitherSetToOpenDate);
  CPPUNIT_TEST(testDateOptimization);

  CPPUNIT_TEST(testConvertDate_empty);
  CPPUNIT_TEST(testConvertDate_wrongInput);
  CPPUNIT_TEST(testConvertDate);

  CPPUNIT_TEST_SUITE_END();

  void testBasicGetters()
  {
    DateTime myDate(2014, 3, 4, 2, 5, 7);
    CPPUNIT_ASSERT_MESSAGE("Error getting Year", myDate.year() == 2014);
    CPPUNIT_ASSERT_MESSAGE("Error getting Month", myDate.month() == 3);
    CPPUNIT_ASSERT_MESSAGE("Error getting Day", myDate.day() == 4);
    CPPUNIT_ASSERT_MESSAGE("Error getting Hour", myDate.hours() == 2);
    CPPUNIT_ASSERT_MESSAGE("Error getting Minute", myDate.minutes() == 5);
    CPPUNIT_ASSERT_MESSAGE("Error getting Second", myDate.seconds() == 7);
  }

  void testIsLeapYearShouldBeTrue()
  {
    bool result = DateTime::isLeapYear(2000);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2000", result);

    result = DateTime::isLeapYear(2004);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2004", result);

    result = DateTime::isLeapYear(2008);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2008", result);

    result = DateTime::isLeapYear(2012);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2012", result);

    result = DateTime::isLeapYear(2048);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2012", result);
  }

  void testIsLeapYearShouldBeFalse()
  {
    bool result = DateTime::isLeapYear(2001);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2001", !result);

    result = DateTime::isLeapYear(2002);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2002", !result);

    result = DateTime::isLeapYear(2003);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2003", !result);

    result = DateTime::isLeapYear(2005);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2005", !result);

    result = DateTime::isLeapYear(2006);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2006", !result);

    result = DateTime::isLeapYear(2007);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2007", !result);

    result = DateTime::isLeapYear(2009);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2009", !result);

    result = DateTime::isLeapYear(2010);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2010", !result);

    result = DateTime::isLeapYear(2049);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsLeapYear1-2011", !result);
  }

  void testIsBetween1()
  {
    DateTime myDate(2004, 10, 10);

    bool result = myDate.isBetween(2004, 1, 1, 2004, 12, 1);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1", result);

    result = myDate.isBetween(0, 1, 1, 0, 12, 1);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1 with 0 year", result);

    result = myDate.isBetween(2004, 11, 1, 2004, 12, 1);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1", !result);

    result = myDate.isBetween(0, 11, 1, 0, 12, 1);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1 with 0 year", !result);

    DateTime myDate2(2004, 8, 10);
    result = myDate2.isBetween(0, 4, 1, 0, 5, 31);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-2 with 0 year", !result);

    DateTime myDate3(2005, 1, 10);
    result = myDate3.isBetween(0, 12, 1, 0, 3, 15);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-3 with 0 year", result);

    DateTime myDate4(2005, 5, 10);
    result = myDate4.isBetween(0, 3, 1, 0, 6, 15);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-4 with 0 year", result);

    DateTime myDate5(2005, 5, 10);
    result = myDate5.isBetween(0, 12, 1, 0, 3, 15);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-5 with 0 year", !result);

    DateTime myDate6(2005, 5, 10);
    result = myDate6.isBetween(0, 5, 1, 0, 1, 15);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-6 with 0 year", result);

    DateTime myDate7(2005, 1, 10);
    result = myDate7.isBetween(0, 5, 12, 0, 1, 15);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-7 with 0 year", result);

    DateTime myDate8(2005, 5, 10);
    result = myDate8.isBetween(0, 1, 0, 0, 6, 0);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-8 with 0 year", result);

    DateTime myDate9(2005, 11, 9);
    result = myDate9.isBetween(0, 8, 15, 0, 6, 30);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-9 with 0 year", result);

    DateTime myDate10(2006, 1, 24);
    result = myDate10.isBetween(0, 12, 29, 0, 12, 9);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-9 with 0 year", result);

    DateTime myDate11(2006, 12, 24);
    result = myDate11.isBetween(0, 12, 29, 0, 12, 9);
    CPPUNIT_ASSERT_MESSAGE("Error in testIsBetween1-9 with 0 year", !result);
  }

  void testBaseDateConstructor()
  {
    // Default date should be Jan 1, 1980
    DateTime baseDate;
    CPPUNIT_ASSERT_EQUAL((short)1980, (short)baseDate.year());
    CPPUNIT_ASSERT_EQUAL((short)1, (short)baseDate.month());
    CPPUNIT_ASSERT_EQUAL((short)1, (short)baseDate.day());
    CPPUNIT_ASSERT_EQUAL((short)0, (short)baseDate.hours());
    CPPUNIT_ASSERT_EQUAL((short)0, (short)baseDate.minutes());
    CPPUNIT_ASSERT_EQUAL((short)0, (short)baseDate.seconds());
  }

  void testOpenDateConstructor()
  {
    // Open date should be Jan 1, 1966
    DateTime openDate = DateTime::openDate();
    CPPUNIT_ASSERT_EQUAL((short)1966, (short)openDate.year());
    CPPUNIT_ASSERT_EQUAL((short)1, (short)openDate.month());
    CPPUNIT_ASSERT_EQUAL((short)1, (short)openDate.day());
  }

  void testFromMilitaryTimeDateConstructor()
  {
    DateTime fromMTpm = DateTime::fromMilitaryTime(1530);
    CPPUNIT_ASSERT_EQUAL((string) "3:30PM", fromMTpm.timeToString(HHMM_AMPM, ":"));

    DateTime fromMTam = DateTime::fromMilitaryTime(1130);
    CPPUNIT_ASSERT_EQUAL((string) "11:30AM", fromMTam.timeToString(HHMM_AMPM, ":"));
  }

  void testConstructors()
  {
    DateTime now = DateTime::localTime();
    sleep(2); // sleep for two seconds to ensure the time has changed
    DateTime today = DateTime::localTime();

    CPPUNIT_ASSERT_MESSAGE("Local time values are not correct",
                           now.get64BitRep() < today.get64BitRep());

    DateTime fromMinutes = DateTime::fromMinutes(1400);
    CPPUNIT_ASSERT_MESSAGE("Expected 11:20PM",
                           fromMinutes.timeToString(HHMM_AMPM, ":") == "11:20PM");

    int64_t int64Time = 211984689600000000LL;
    DateTime intTime;
    intTime.set64BitRep(int64Time);
    CPPUNIT_ASSERT_EQUAL(int64Time, intTime.get64BitRep());
    CPPUNIT_ASSERT_EQUAL((string) "2005-Jun-05 00:00:00", intTime.toSimpleString());

    DateTime currDate = DateTime::localTime();
    int64Time = currDate.get64BitRep();
    int64Time = currDate.get64BitRepDateOnly();
    currDate.set64BitRep(int64Time);

    CPPUNIT_ASSERT(true);
  }

  void testStringConversionsShouldBeCorrectlyFormatted()
  {
    DateTime defaultDate;
    DateTime testDate(2014, 3, 4, 2, 5, 7);

    CPPUNIT_ASSERT_EQUAL((std::string) "N/A", defaultDate.toSimpleString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-Mar-04 02:05:07", testDate.toSimpleString());

    CPPUNIT_ASSERT_EQUAL((std::string) "20140304T020507", testDate.toIsoString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-03-04T02:05:07", testDate.toIsoExtendedString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-Mar-04", testDate.dateToSimpleString());

    CPPUNIT_ASSERT_EQUAL((std::string) "20140304", testDate.dateToIsoString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-03-04", testDate.dateToIsoExtendedString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-03-04", testDate.dateToSqlString());

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-03-04", testDate.dateToString(YYYYMMDD, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-Mar-04", testDate.dateToString(YYYYMmmDD, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "2014-MAR-04", testDate.dateToString(YYYYMMMDD, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "03/04/2014", testDate.dateToString(MMDDYYYY, "/"));

    CPPUNIT_ASSERT_EQUAL((std::string) "03/04/14", testDate.dateToString(MMDDYY, "/"));

    CPPUNIT_ASSERT_EQUAL((std::string) "Mar-04-2014", testDate.dateToString(MmmDDYYYY, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "MAR-04-2014", testDate.dateToString(MMMDDYYYY, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "MAR-04-14", testDate.dateToString(MMMDDYY, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "04.03.2014", testDate.dateToString(DDMMYYYY, "."));

    CPPUNIT_ASSERT_EQUAL((std::string) "04-Mar-2014", testDate.dateToString(DDMmmYYYY, "-"));

    CPPUNIT_ASSERT_EQUAL((std::string) "04MAR2014", testDate.dateToString(DDMMMYYYY, ""));

    CPPUNIT_ASSERT_EQUAL((std::string) "04MAR14", testDate.dateToString(DDMMMYY, ""));

    CPPUNIT_ASSERT_EQUAL((std::string) "04MAR", testDate.dateToString(DDMMM, ""));

    CPPUNIT_ASSERT_EQUAL((std::string) "04MR", testDate.dateToString(DDMM_FD, ""));

    CPPUNIT_ASSERT_EQUAL((std::string) "02:05:07", testDate.timeToString(HHMMSS, ":"));

    CPPUNIT_ASSERT_EQUAL((std::string) "02:05", testDate.timeToString(HHMM, ":"));

    CPPUNIT_ASSERT_EQUAL((std::string) "2:05AM", testDate.timeToString(HHMM_AMPM, ":"));

    DateTime testDatePM(2014, 3, 4, 15, 30, 44);
    CPPUNIT_ASSERT_EQUAL((std::string) "3:30PM", testDatePM.timeToString(HHMM_AMPM, ":"));

    DateTime testDateNoon(2004, 3, 4, 12, 0, 0);
    CPPUNIT_ASSERT_EQUAL((std::string) "12:00PM", testDateNoon.timeToString(HHMM_AMPM, ":"));

    DateTime testDateMidnight(2004, 3, 4, 0, 0, 0);
    CPPUNIT_ASSERT_EQUAL((std::string) "12:00AM", testDateMidnight.timeToString(HHMM_AMPM, ":"));
  }

  // Only uses date portion --- not time
  void testIsFutureDateShouldBeTrue()
  {
    DateTime currentDate = DateTime::localTime();

    DateTime futureDate = currentDate.addYears(1);

    CPPUNIT_ASSERT_MESSAGE("Expected date to be in the future by one year",
                           futureDate.isFutureDate());

    futureDate = currentDate.addMonths(1);
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in the future by one month",
                           futureDate.isFutureDate());

    futureDate = currentDate.addWeeks(1);
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in the future by one week",
                           futureDate.isFutureDate());

    // Add Days to make sure the test works if the date changes in the middle of the test
    futureDate = currentDate.addDays(2);
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in the future by two days",
                           futureDate.isFutureDate());
  }

  // Validate the isFutureDate returns false on past dates and today.
  // Only uses date portion --- not time
  void testIsFutureDateShouldBeFalse()
  {
    DateTime currentDate = DateTime::localTime();
    DateTime futureDate = DateTime::localTime();

    // Test with last year
    futureDate.set64BitRep(currentDate.get64BitRep() - (tse::SECONDS_PER_DAY * 365));
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in past by one year", !(futureDate.isFutureDate()));

    // Test with last month
    futureDate.set64BitRep(currentDate.get64BitRep() - (tse::SECONDS_PER_DAY * 30));
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in past by one month",
                           !(futureDate.isFutureDate()));

    // Test with last week
    futureDate.set64BitRep(currentDate.get64BitRep() - (tse::SECONDS_PER_DAY * 7));
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in past by one week", !(futureDate.isFutureDate()));

    // Test with yesterday
    futureDate.set64BitRep(currentDate.get64BitRep() - (tse::SECONDS_PER_DAY));
    CPPUNIT_ASSERT_MESSAGE("Expected date to be in past by one day", !(futureDate.isFutureDate()));

    // This test may fail if ran a few secconds before midnight
    futureDate = currentDate.addSeconds(1);
    CPPUNIT_ASSERT_MESSAGE("Expected date to be today", !(futureDate.isFutureDate()));
  }

  void testEarlierShouldAlwaysReturnSecondDateWhenFirstDateIsOpenDate()
  {
    DateTime firstDate = DateTime::openDate();
    DateTime secondDate = DateTime::openDate();

    CPPUNIT_ASSERT(DateTime::earlier(firstDate, secondDate).isOpenDate());

    secondDate = DateTime::openDate().addMonths(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    secondDate.set64BitRep(DateTime::openDate().get64BitRep() - (tse::SECONDS_PER_DAY * 30));
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());
  }

  // This tests assumes the first date is not an open date
  // otherwise it would be covered by the test above
  void testEarlierShouldReturnFirstDateWhenSecondDateIsOpenDate()
  {
    DateTime firstDate = DateTime::openDate();
    DateTime secondDate = DateTime::openDate();

    CPPUNIT_ASSERT(DateTime::earlier(firstDate, secondDate).isOpenDate());

    firstDate = DateTime::openDate().addMonths(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    firstDate.set64BitRep(DateTime::openDate().get64BitRep() - (tse::SECONDS_PER_DAY * 30));
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());
  }

  void testEarlierShouldReturnEarlierDateIfNeitherSetToOpenDate()
  {
    DateTime currentDate = DateTime::localTime();
    DateTime firstDate = DateTime::localTime();
    DateTime secondDate = currentDate.addSeconds(5);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    firstDate = currentDate.addDays(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    secondDate = currentDate.addWeeks(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    firstDate = currentDate.addMonths(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());

    secondDate = currentDate.addYears(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::earlier(firstDate, secondDate).get64BitRep());
  }

  void testLaterShouldAlwaysReturnSecondDateWhenFirstDateIsOpenDate()
  {
    DateTime firstDate = DateTime::openDate();
    DateTime secondDate = DateTime::openDate();

    CPPUNIT_ASSERT(DateTime::later(firstDate, secondDate).isOpenDate());

    secondDate = DateTime::openDate().addMonths(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    secondDate.set64BitRep(DateTime::openDate().get64BitRep() - (tse::SECONDS_PER_DAY * 30));
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());
  }

  void testLaterShouldReturnFirstDateWhenSecondDateIsOpenDate()
  {
    DateTime firstDate = DateTime::openDate();
    DateTime secondDate = DateTime::openDate();

    CPPUNIT_ASSERT(DateTime::later(firstDate, secondDate).isOpenDate());

    firstDate = DateTime::openDate().addMonths(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    firstDate.set64BitRep(DateTime::openDate().get64BitRep() - (tse::SECONDS_PER_DAY * 30));
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());
  }

  void testLaterShouldReturnEarlierDateIfNeitherSetToOpenDate()
  {
    DateTime currentDate = DateTime::localTime();
    DateTime firstDate = DateTime::localTime();
    DateTime secondDate = currentDate.addSeconds(5);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    firstDate = currentDate.addDays(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    secondDate = currentDate.addWeeks(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    firstDate = currentDate.addMonths(1);
    CPPUNIT_ASSERT_EQUAL(firstDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());

    secondDate = currentDate.addYears(1);
    CPPUNIT_ASSERT_EQUAL(secondDate.get64BitRep(),
                         DateTime::later(firstDate, secondDate).get64BitRep());
  }

  void addDay(DateTime& dt) const
  {
    boost::gregorian::day_iterator it(dt.boost::posix_time::ptime::date());
    ++it;
    dt = boost::posix_time::ptime(*it,
                                  boost::posix_time::time_duration(dt.time_of_day().hours(),
                                                                   dt.time_of_day().minutes(),
                                                                   dt.time_of_day().seconds()));
  }

  void subtractDay(DateTime& dt) const
  {
    boost::gregorian::day_iterator it(dt.boost::posix_time::ptime::date());
    --it;
    dt = boost::posix_time::ptime(*it,
                                  boost::posix_time::time_duration(dt.time_of_day().hours(),
                                                                   dt.time_of_day().minutes(),
                                                                   dt.time_of_day().seconds()));
  }

  void testDateOptimization()
  {
    DateTime date_time_pastb(boost::gregorian::date(2012, 06, 07),
                             boost::posix_time::time_duration(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(date_time_pastb.date(), date_time_pastb.boost::posix_time::ptime::date());
    DateTime date_time_paste(boost::gregorian::date(2012, 06, 07),
                             boost::posix_time::time_duration(23, 59, 59));
    CPPUNIT_ASSERT_EQUAL(date_time_paste.date(), date_time_paste.boost::posix_time::ptime::date());
    DateTime date_timeb(boost::gregorian::day_clock::local_day(),
                        boost::posix_time::time_duration(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(date_timeb.date(), date_timeb.boost::posix_time::ptime::date());
    DateTime date_timee(boost::gregorian::day_clock::local_day(),
                        boost::posix_time::time_duration(23, 59, 59));
    CPPUNIT_ASSERT_EQUAL(date_timee.date(), date_timee.boost::posix_time::ptime::date());
    DateTime empty;
    CPPUNIT_ASSERT_EQUAL(empty.date(), empty.boost::posix_time::ptime::date());
    DateTime max_date(boost::gregorian::date(9999, 12, 31));
    CPPUNIT_ASSERT_EQUAL(max_date.boost::posix_time::ptime::date(), max_date.date());
    DateTime min_date(boost::gregorian::date(1400, 1, 1));
    CPPUNIT_ASSERT_EQUAL(min_date.boost::posix_time::ptime::date(), min_date.date());
    DateTime incr_dateb(date_timeb);
    DateTime decr_dateb(date_timeb);
    DateTime incr_datee(date_timee);
    DateTime decr_datee(date_timee);
    for (int i = 0; i < 100000; ++i)
    {
      addDay(incr_dateb);
      CPPUNIT_ASSERT_EQUAL(incr_dateb.boost::posix_time::ptime::date(), incr_dateb.date());
      subtractDay(decr_dateb);
      CPPUNIT_ASSERT_EQUAL(decr_dateb.boost::posix_time::ptime::date(), decr_dateb.date());
      addDay(incr_datee);
      CPPUNIT_ASSERT_EQUAL(incr_datee.boost::posix_time::ptime::date(), incr_datee.date());
      subtractDay(decr_datee);
      CPPUNIT_ASSERT_EQUAL(decr_datee.boost::posix_time::ptime::date(), decr_datee.date());
    }
  }

  void testConvertDate_empty()
  {
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(), DateTime::convertDate(""));
  }

  void testConvertDate_wrongInput()
  {
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(), DateTime::convertDate("abc"));
    CPPUNIT_ASSERT_EQUAL(DateTime::emptyDate(), DateTime::convertDate("20140522"));
  }

  void testConvertDate()
  {
    CPPUNIT_ASSERT_EQUAL(DateTime(2014, 05, 22), DateTime::convertDate("2014-05-22"));
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(DateTest);
};
