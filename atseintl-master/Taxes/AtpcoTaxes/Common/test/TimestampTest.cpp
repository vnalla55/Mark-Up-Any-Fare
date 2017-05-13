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
#include "Common/Timestamp.h"

using namespace tax::type;
using namespace std;

namespace tax
{
class DateTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DateTest);

  CPPUNIT_TEST(testValidDatesCreation);
  CPPUNIT_TEST(testInvalidDatesCreation);
  CPPUNIT_TEST(testSpecialDatesCreation);
  CPPUNIT_TEST(testMonthLastDates);
  CPPUNIT_TEST(testSeasonalDatesCreation);
  CPPUNIT_TEST(testCompareSpecificDates);
  CPPUNIT_TEST(testCompareSpecificDatesToSeasonal);
  CPPUNIT_TEST(testCompareSeasonalDates);
  CPPUNIT_TEST(testCompareWithSpecialDates);
  CPPUNIT_TEST(testIsBetween);
  CPPUNIT_TEST(testInBetweenForSeasonalDates);
  CPPUNIT_TEST(testDateParsing);
  CPPUNIT_TEST(testDateOutput);
  CPPUNIT_TEST(testSeasonalDateParsing);
  CPPUNIT_TEST(testSeasonalDateOutput);
  CPPUNIT_TEST(testSpecialDateParsing);
  CPPUNIT_TEST(testSpecialDateOutput);
  CPPUNIT_TEST(testAdvance);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testValidDatesCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Date firstDate(2010, 11, 3));

    Date firstDate(2010, 11, 3);
    CPPUNIT_ASSERT_EQUAL(uint16_t(2010), firstDate.year());
    CPPUNIT_ASSERT_EQUAL(uint16_t(11), firstDate.month());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), firstDate.day());
  }

  void testInvalidDatesCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Date minDate(1799, 11, 3));
    Date minDate(1799, 11, 3);
    CPPUNIT_ASSERT_EQUAL(Date::neg_infinity(), minDate);

    CPPUNIT_ASSERT_NO_THROW(Date maxDate(3000, 11, 3));
    Date maxDate(10000, 11, 3);
    CPPUNIT_ASSERT_EQUAL(Date::pos_infinity(), maxDate);

    CPPUNIT_ASSERT_THROW(Date firstDate(2010, 0, 3), out_of_range);
    CPPUNIT_ASSERT_THROW(Date firstDate(2010, 13, 3), out_of_range);
    CPPUNIT_ASSERT_THROW(Date firstDate(2010, 11, 0), out_of_range);
    CPPUNIT_ASSERT_THROW(Date firstDate(2010, 11, 33), out_of_range);
  }

  void testSpecialDatesCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Date maxDate(10000, 11, 3));
    Date maxDate(10000, 11, 3);
    CPPUNIT_ASSERT_EQUAL(uint16_t(65535), maxDate.year());
    CPPUNIT_ASSERT_EQUAL(uint16_t(255), maxDate.month());
    CPPUNIT_ASSERT_EQUAL(uint16_t(255), maxDate.day());
    CPPUNIT_ASSERT_EQUAL(Date::pos_infinity(), maxDate);
  }

  void testMonthLastDates()
  {
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 1, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 1, 32), out_of_range);

    CPPUNIT_ASSERT_NO_THROW(Date date(2009, 2, 28));
    CPPUNIT_ASSERT_THROW(Date date(2009, 2, 29), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 2, 28));
    CPPUNIT_ASSERT_THROW(Date date(2010, 2, 29), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2011, 2, 28));
    CPPUNIT_ASSERT_THROW(Date date(2011, 2, 29), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2012, 2, 29));
    CPPUNIT_ASSERT_THROW(Date date(2012, 2, 30), out_of_range);

    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 3, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 3, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 4, 30));
    CPPUNIT_ASSERT_THROW(Date date(2010, 4, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 5, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 5, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 6, 30));
    CPPUNIT_ASSERT_THROW(Date date(2010, 6, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 7, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 7, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 8, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 8, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 9, 30));
    CPPUNIT_ASSERT_THROW(Date date(2010, 9, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 10, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 10, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 11, 30));
    CPPUNIT_ASSERT_THROW(Date date(2010, 11, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2010, 12, 31));
    CPPUNIT_ASSERT_THROW(Date date(2010, 12, 32), out_of_range);
  }

  void testSeasonalDatesCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Date seasDate(11, 3));
    Date seasDate(11, 3);
    CPPUNIT_ASSERT_EQUAL(uint16_t(0), seasDate.year());
    CPPUNIT_ASSERT_EQUAL(uint16_t(11), seasDate.month());
    CPPUNIT_ASSERT_EQUAL(uint16_t(3), seasDate.day());

    CPPUNIT_ASSERT_THROW(Date firstDate(0, 3), out_of_range);
    CPPUNIT_ASSERT_THROW(Date firstDate(13, 3), out_of_range);
    CPPUNIT_ASSERT_THROW(Date firstDate(11, 0), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(1, 31));
    CPPUNIT_ASSERT_THROW(Date date(1, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(2, 29));
    CPPUNIT_ASSERT_THROW(Date date(2, 30), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(3, 31));
    CPPUNIT_ASSERT_THROW(Date date(3, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(4, 30));
    CPPUNIT_ASSERT_THROW(Date date(4, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(5, 31));
    CPPUNIT_ASSERT_THROW(Date date(5, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(6, 30));
    CPPUNIT_ASSERT_THROW(Date date(6, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(7, 31));
    CPPUNIT_ASSERT_THROW(Date date(7, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(8, 31));
    CPPUNIT_ASSERT_THROW(Date date(8, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(9, 30));
    CPPUNIT_ASSERT_THROW(Date date(9, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(10, 31));
    CPPUNIT_ASSERT_THROW(Date date(10, 32), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(11, 30));
    CPPUNIT_ASSERT_THROW(Date date(11, 31), out_of_range);
    CPPUNIT_ASSERT_NO_THROW(Date date(12, 31));
    CPPUNIT_ASSERT_THROW(Date date(12, 32), out_of_range);
  }

  void testCompareSpecificDates()
  {
    Date d1(2005, 11, 3);
    Date d2(2005, 11, 3);
    Date d3(2004, 12, 5);

    CPPUNIT_ASSERT_EQUAL(d1, d2);
    CPPUNIT_ASSERT_EQUAL(true, d1 <= d2);
    CPPUNIT_ASSERT_EQUAL(true, d2 <= d1);
    CPPUNIT_ASSERT_EQUAL(true, d3 <= d1);
    CPPUNIT_ASSERT_EQUAL(false, d1 <= d3);

    CPPUNIT_ASSERT_EQUAL(false, d1 < d2);
    CPPUNIT_ASSERT_EQUAL(true, d3 < d1);
    CPPUNIT_ASSERT_EQUAL(false, d1 < d3);

    CPPUNIT_ASSERT_EQUAL(true, d1 == d2);
    CPPUNIT_ASSERT_EQUAL(false, d1 == d3);
  }

  void testCompareSpecificDatesToSeasonal()
  {
    Date d1(2005, 4, 3);
    Date d2(2005, 11, 3);
    Date s1(4, 3);
    Date s2(8, 5);

    CPPUNIT_ASSERT_EQUAL(true, d1 <= s1);
    CPPUNIT_ASSERT_EQUAL(true, d1 <= s2);
    CPPUNIT_ASSERT_EQUAL(true, s1 <= d1);
    CPPUNIT_ASSERT_EQUAL(false, s2 <= d1);

    CPPUNIT_ASSERT_EQUAL(false, d1 < s1);
    CPPUNIT_ASSERT_EQUAL(false, s1 < d1);
    CPPUNIT_ASSERT_EQUAL(false, d2 < s2);

    CPPUNIT_ASSERT_EQUAL(true, d1 == s1);
    CPPUNIT_ASSERT_EQUAL(false, d2 == s2);
  }

  void testCompareSeasonalDates()
  {
    Date d1(11, 3);
    Date d2(11, 3);
    Date d3(12, 5);
    CPPUNIT_ASSERT_EQUAL(d1, d2);
    CPPUNIT_ASSERT_EQUAL(true, d1 <= d2);
    CPPUNIT_ASSERT_EQUAL(true, d2 <= d1);
    CPPUNIT_ASSERT_EQUAL(false, d3 <= d1);
    CPPUNIT_ASSERT_EQUAL(true, d1 <= d3);

    CPPUNIT_ASSERT_EQUAL(false, d1 < d2);
    CPPUNIT_ASSERT_EQUAL(false, d3 < d1);
    CPPUNIT_ASSERT_EQUAL(true, d1 < d3);

    CPPUNIT_ASSERT_EQUAL(true, d1 == d2);
    CPPUNIT_ASSERT_EQUAL(false, d1 == d3);
  }

  void testCompareWithSpecialDates()
  {
    Date d1(11, 3);
    Date d2(2011, 11, 3);
    Date pi = Date::pos_infinity();
    Date ni = Date::neg_infinity();
    Date id = Date::invalid_date();
    Date bl = Date::blank_date();

    CPPUNIT_ASSERT_EQUAL(true, pi == pi);
    CPPUNIT_ASSERT_EQUAL(true, ni == ni);
    CPPUNIT_ASSERT_EQUAL(true, id == id);
    CPPUNIT_ASSERT_EQUAL(true, bl == bl);

    CPPUNIT_ASSERT_EQUAL(false, d1 == pi);
    CPPUNIT_ASSERT_EQUAL(false, d2 == pi);
    CPPUNIT_ASSERT_EQUAL(false, d1 == ni);
    CPPUNIT_ASSERT_EQUAL(false, d2 == ni);
    CPPUNIT_ASSERT_EQUAL(false, d1 == id);
    CPPUNIT_ASSERT_EQUAL(false, d2 == id);
    CPPUNIT_ASSERT_EQUAL(false, d1 == bl);
    CPPUNIT_ASSERT_EQUAL(false, d2 == bl);

    CPPUNIT_ASSERT_EQUAL(true, d1 <= pi);
    CPPUNIT_ASSERT_EQUAL(true, d2 <= pi);
    CPPUNIT_ASSERT_EQUAL(false, pi <= d1);
    CPPUNIT_ASSERT_EQUAL(false, pi <= d2);
    CPPUNIT_ASSERT_EQUAL(false, d1 <= ni);
    CPPUNIT_ASSERT_EQUAL(false, d2 <= ni);
    CPPUNIT_ASSERT_EQUAL(true, ni <= d1);
    CPPUNIT_ASSERT_EQUAL(true, ni <= d2);

    CPPUNIT_ASSERT_EQUAL(true, d1 < pi);
    CPPUNIT_ASSERT_EQUAL(true, d2 < pi);
    CPPUNIT_ASSERT_EQUAL(false, pi < d1);
    CPPUNIT_ASSERT_EQUAL(false, pi < d2);
    CPPUNIT_ASSERT_EQUAL(false, d1 < ni);
    CPPUNIT_ASSERT_EQUAL(false, d2 < ni);
    CPPUNIT_ASSERT_EQUAL(true, ni < d1);
    CPPUNIT_ASSERT_EQUAL(true, ni < d2);

    CPPUNIT_ASSERT_THROW(d1 < id, domain_error);
    CPPUNIT_ASSERT_THROW(d2 < id, domain_error);
    CPPUNIT_ASSERT_THROW(id < d1, domain_error);
    CPPUNIT_ASSERT_THROW(id < d2, domain_error);
    CPPUNIT_ASSERT_THROW(d1 <= id, domain_error);
    CPPUNIT_ASSERT_THROW(d2 <= id, domain_error);
    CPPUNIT_ASSERT_THROW(id <= d1, domain_error);
    CPPUNIT_ASSERT_THROW(id <= d2, domain_error);

    CPPUNIT_ASSERT_THROW(d1 < bl, domain_error);
    CPPUNIT_ASSERT_THROW(d2 < bl, domain_error);
    CPPUNIT_ASSERT_THROW(bl < d1, domain_error);
    CPPUNIT_ASSERT_THROW(bl < d2, domain_error);
    CPPUNIT_ASSERT_THROW(d1 <= bl, domain_error);
    CPPUNIT_ASSERT_THROW(d2 <= bl, domain_error);
    CPPUNIT_ASSERT_THROW(bl <= d1, domain_error);
    CPPUNIT_ASSERT_THROW(bl <= d2, domain_error);

    CPPUNIT_ASSERT_EQUAL(false, id == d1);
    CPPUNIT_ASSERT_EQUAL(false, id == d2);
    CPPUNIT_ASSERT_EQUAL(false, d1 == id);
    CPPUNIT_ASSERT_EQUAL(false, d2 == id);
  }

  void testIsBetween()
  {
    Date blank = Date::blank_date();
    Date low = Date(2014, 6, 1);
    Date high = Date(2014, 6, 3);

    Date x = Date(2014, 6, 2);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(blank, blank));
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low,   blank));
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(blank, high));
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low,   high));

    x = Date(2014, 5, 31);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, blank));
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 6, 4);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(blank, high));
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low,   high));
  }

  void testInBetweenForSeasonalDates()
  {
    // Between 15th March  and 15th September same year
    Date low = Date(3, 15);
    Date high = Date(9, 15);

    Date x = Date(2014, 1, 1);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 3, 14);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 3, 15);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2014, 6, 1);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2014, 9, 15);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2014, 9, 16);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 12, 12);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    // Between 15th September and 15th March next year
    low = Date(9, 15);
    high = Date(3, 15);

    x = Date(2014, 6, 1);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 9, 14);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2014, 9, 15);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2014, 12, 30);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2015, 3, 15);
    CPPUNIT_ASSERT_EQUAL(true, x.is_between(low, high));

    x = Date(2015, 3, 16);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));

    x = Date(2015, 6, 1);
    CPPUNIT_ASSERT_EQUAL(false, x.is_between(low, high));
  }

  void testDateParsing()
  {
    Date expected(2012, 6, 18);
    Date parsed;
    istringstream input("2012-06-18");
    input >> parsed;
    CPPUNIT_ASSERT_EQUAL(expected, parsed);
  }

  void testDateOutput()
  {
    string expected("2012-06-04");
    Date date(2012, 6, 4);
    ostringstream output;
    output << date;
    CPPUNIT_ASSERT_EQUAL(expected, output.str());
  }

  void testSeasonalDateParsing()
  {
    Date expected(11, 5);
    Date parsed;
    istringstream input("00/11/05");
    input >> parsed;
    CPPUNIT_ASSERT_EQUAL(expected, parsed);
  }

  void testSeasonalDateOutput()
  {
    string expected("****-06-04");
    Date date(6, 4);
    ostringstream output;
    output << date;
    CPPUNIT_ASSERT_EQUAL(expected, output.str());
  }

  void testSpecialDateParsing()
  {
    Date parsed;
    istringstream input("1792/11/03 12921/03/01");
    Date expected = Date::neg_infinity();
    input >> parsed;
    CPPUNIT_ASSERT_EQUAL(expected, parsed);

    expected = Date::pos_infinity();
    input >> parsed;
    CPPUNIT_ASSERT_EQUAL(expected, parsed);
  }

  void testSpecialDateOutput()
  {
    string expected("1799-12-31");
    ostringstream output;
    output << Date::neg_infinity();
    CPPUNIT_ASSERT_EQUAL(expected, output.str());

    output.str("");
    expected = "9999-12-31";
    output << Date::pos_infinity();
    CPPUNIT_ASSERT_EQUAL(expected, output.str());

    output.str("");
    expected = "???\?-?\?-??";
    output << Date::invalid_date();
    CPPUNIT_ASSERT_EQUAL(expected, output.str());

    output.str("");
    expected = "    -  -  ";
    output << Date::blank_date();
    CPPUNIT_ASSERT_EQUAL(expected, output.str());
  }

  void testAdvance()
  {
    Date d1(2012, 1, 10);
    Date d2(2012, 2, 19);
    Date d3(2012, 3, 10);
    Date d4(2013, 2, 19);
    Date d5(2013, 3, 11);
    Date s1(3, 11);
    CPPUNIT_ASSERT_EQUAL(d2, d1.advance(40));
    CPPUNIT_ASSERT_EQUAL(d3, d2.advance(20));
    CPPUNIT_ASSERT_EQUAL(d4, d3.advance(346));
    CPPUNIT_ASSERT_EQUAL(d5, d4.advance(20));
    CPPUNIT_ASSERT_EQUAL(d1, d5.advance(-426));
    CPPUNIT_ASSERT_EQUAL(d1, d1.advance(75).advance(-75));
    CPPUNIT_ASSERT_THROW(s1.advance(0), logic_error);
    CPPUNIT_ASSERT_EQUAL(Date::pos_infinity().advance(-1), Date::pos_infinity());
    CPPUNIT_ASSERT_EQUAL(Date::neg_infinity().advance(10), Date::neg_infinity());
    CPPUNIT_ASSERT_EQUAL(Date::invalid_date().advance(10), Date::invalid_date());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(DateTest);

class TimeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TimeTest);

  CPPUNIT_TEST(testValidTimesCreation);
  CPPUNIT_TEST(testInvalidTimesCreation);

  CPPUNIT_TEST(testTimeDifference);
  CPPUNIT_TEST(testTimeShifts);

  CPPUNIT_TEST(testTimeInput);
  CPPUNIT_TEST(testInvalidTimeInput);
  CPPUNIT_TEST(testTimeOutput);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testValidTimesCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Time firstTime(14, 40));
    Time firstTime(14, 40);
    CPPUNIT_ASSERT_EQUAL((int16_t)14, firstTime.hour());
    CPPUNIT_ASSERT_EQUAL((int16_t)40, firstTime.min());
  }

  void testInvalidTimesCreation()
  {
    CPPUNIT_ASSERT_THROW(Time firstTime(24, 40), out_of_range);
    CPPUNIT_ASSERT_THROW(Time firstTime(-1, 40), out_of_range);
    CPPUNIT_ASSERT_THROW(Time firstTime(14, -2), out_of_range);
    CPPUNIT_ASSERT_THROW(Time firstTime(11, 60), out_of_range);
  }

  void testTimeDifference()
  {
    Time t1(11, 50);
    Time t2(11, 0);
    Time t3(23, 10);

    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), t1 - t1);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), t2 - t2);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(0), t3 - t3);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(50), t1 - t2);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(-50), t2 - t1);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(730), t3 - t2);
  }
  void testTimeShifts()
  {
    Time t1(11, 50);
    const int shift = 75;
    Time t2(t1 + shift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(13), t2.hour());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(5), t2.min());
    Time t3(t1 - shift);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(10), t3.hour());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(35), t3.min());
  }
  void testTimeInput()
  {
    Time t;
    string time("11:40");
    istringstream stream(time);
    CPPUNIT_ASSERT_NO_THROW(stream >> t);
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(11), t.hour());
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(40), t.min());
  }
  void testInvalidTimeInput()
  {
    Time t;
    string time("11/70");
    istringstream stream(time);
    CPPUNIT_ASSERT_THROW(stream >> t, out_of_range);

    time = "33:50";
    stream.str(time);
    CPPUNIT_ASSERT_THROW(stream >> t, out_of_range);

    time = "11-50";
    stream.str(time);
    CPPUNIT_ASSERT_THROW(stream >> t, out_of_range);
  }
  void testTimeOutput()
  {
    ostringstream stream;
    Time t(18, 39);
    stream << t;
    CPPUNIT_ASSERT_EQUAL(string("18:39"), stream.str());

    stream.str("");
    t = Time(0, 0);
    stream << t;
    CPPUNIT_ASSERT_EQUAL(string("00:00"), stream.str());

    stream.str("");
    t = Time(1, 3);
    stream << t;
    CPPUNIT_ASSERT_EQUAL(string("01:03"), stream.str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);

class TimestampTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TimestampTest);

  CPPUNIT_TEST(testNoArgsCreation);
  CPPUNIT_TEST(testValidArgsCreation);
  CPPUNIT_TEST(testInvalidArgsCreation);
  CPPUNIT_TEST(testAssignment);

  CPPUNIT_TEST(testTimestampDifference);

  CPPUNIT_TEST(testTimestampInput);
  CPPUNIT_TEST(testInvalidTimestampInput);
  CPPUNIT_TEST(testTimestampOutput);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void testNoArgsCreation()
  {
    Timestamp stamp;
    CPPUNIT_ASSERT_EQUAL(Date::blank_date(), stamp.date());
    CPPUNIT_ASSERT_EQUAL(Time::blank_time(), stamp.time());
  }

  void testValidArgsCreation()
  {
    CPPUNIT_ASSERT_NO_THROW(Timestamp ts(Date(2013, 8, 22), Time(14, 40)));
    Timestamp ts(Date(2013, 8, 22), Time(14, 40));
    CPPUNIT_ASSERT_EQUAL((uint16_t)2013, ts.year());
    CPPUNIT_ASSERT_EQUAL((uint16_t)8, ts.month());
    CPPUNIT_ASSERT_EQUAL((uint16_t)22, ts.day());
    CPPUNIT_ASSERT_EQUAL((int16_t)14, ts.hour());
    CPPUNIT_ASSERT_EQUAL((int16_t)40, ts.min());
  }

  void testInvalidArgsCreation()
  {
    CPPUNIT_ASSERT_THROW(Timestamp ts(Date::invalid_date(), Time(14, 40)), out_of_range);
    CPPUNIT_ASSERT_THROW(Timestamp ts(Date::pos_infinity(), Time(14, 40)), out_of_range);
    CPPUNIT_ASSERT_THROW(Timestamp ts(Date::neg_infinity(), Time(14, 40)), out_of_range);
    CPPUNIT_ASSERT_THROW(Timestamp ts(Date::blank_date(), Time(14, 40)), out_of_range);
    CPPUNIT_ASSERT_THROW(Timestamp ts(Date(2013, 8, 22), Time::blank_time()), out_of_range);
  }

  void testAssignment()
  {
    Timestamp ts1(Date(2013, 8, 17), Time(17, 44));
    Timestamp ts2(Date(2015, 3, 11), Time(14, 47));
    CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT_EQUAL(ts1, ts2));
    ts1 = ts2;
    CPPUNIT_ASSERT_EQUAL(ts1, ts2);
  }

  void testTimestampDifference()
  {
    Time t1(11, 50);
    Time t2(11, 0);
    Time t3(23, 10);

    Date d1(2013, 8, 11);
    Date d2(2013, 8, 21);

    Timestamp ts1(d1, t1);
    Timestamp ts2(d1, t2);
    Timestamp ts3(d1, t3);
    Timestamp ts4(d2, t1);
    Timestamp ts5(d2, t2);
    Timestamp ts6(d2, t3);

    CPPUNIT_ASSERT_EQUAL(0, ts1 - ts1);
    CPPUNIT_ASSERT_EQUAL(0, ts2 - ts2);
    CPPUNIT_ASSERT_EQUAL(0, ts4 - ts4);
    CPPUNIT_ASSERT_EQUAL(int32_t(t2 - t1), ts2 - ts1);
    CPPUNIT_ASSERT_EQUAL(24 * 60 * (d2 - d1), ts6 - ts3);
    CPPUNIT_ASSERT_EQUAL(24 * 60 * (d1 - d2) + (t1 - t2), ts1 - ts5);
  }
  void testTimestampInput()
  {
    Timestamp ts;
    string timestamp("2013-08-17 11:40");
    istringstream stream(timestamp);
    CPPUNIT_ASSERT_NO_THROW(stream >> ts);
    CPPUNIT_ASSERT_EQUAL(Time(11, 40), ts.time());
    CPPUNIT_ASSERT_EQUAL(Date(2013, 8, 17), ts.date());
  }
  void testInvalidTimestampInput()
  {
    Timestamp ts;
    string timestamp("2013-11-18 11/70");
    istringstream stream(timestamp);
    CPPUNIT_ASSERT_THROW(stream >> ts, out_of_range);

    timestamp = "2013-11-18x11:30";
    stream.str(timestamp);
    CPPUNIT_ASSERT_THROW(stream >> ts, out_of_range);

    timestamp = "2013-13-18 11:30";
    stream.str(timestamp);
    CPPUNIT_ASSERT_THROW(stream >> ts, out_of_range);
  }
  void testTimestampOutput()
  {
    ostringstream stream;
    Time time(18, 39);
    Date date(2013, 8, 11);
    Timestamp ts(date, time);
    stream << ts;
    CPPUNIT_ASSERT_EQUAL(string("2013-08-11 18:39"), stream.str());

    stream.str("");
    ts.time() = Time(0, 0);
    stream << ts;
    CPPUNIT_ASSERT_EQUAL(string("2013-08-11 00:00"), stream.str());

    stream.str("");
    ts.time() = Time(1, 3);
    stream << ts;
    CPPUNIT_ASSERT_EQUAL(string("2013-08-11 01:03"), stream.str());

    stream.str("");
    ts.date() = Date(2016, 2, 29);
    stream << ts;
    CPPUNIT_ASSERT_EQUAL(string("2016-02-29 01:03"), stream.str());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TimeTest);
}
