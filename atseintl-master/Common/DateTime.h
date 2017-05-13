/**
 * @file     DateTime.h
 * @date     05/07/2004
 * @author   Konstantin Sidorin
 *
 * @brief    contains DateTime class for Travel Shopoing Eengine.
 *
 *  Updates: 4/21/2005  J.D. Batchelor  - Added const SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY
 *
 *  Copyright Sabre 2004
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#pragma once

#include "Common/TseEnums.h"
#include "Util/BranchPrediction.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <ctime>

namespace tse
{

/**
 * @defgroup BoostTypedefs
 *
 * Bring boost typedefs and enums into the tse namespace
 *
 * @{
 */
typedef boost::gregorian::greg_year Year;
typedef boost::gregorian::greg_month Month;
typedef boost::gregorian::greg_day Day;
typedef boost::gregorian::greg_weekday DayOfWeek;
typedef boost::posix_time::time_duration TimeDuration;
typedef boost::posix_time::hours Hours;
typedef boost::posix_time::minutes Minutes;
typedef boost::posix_time::seconds Seconds;

typedef boost::gregorian::gregorian_calendar GregorianCalendar;

typedef boost::date_time::months_of_year MonthsOfYear;

using boost::date_time::months_of_year;
using boost::date_time::Jan;
using boost::date_time::Feb;
using boost::date_time::Mar;
using boost::date_time::Apr;
using boost::date_time::May;
using boost::date_time::Jun;
using boost::date_time::Jul;
using boost::date_time::Aug;
using boost::date_time::Sep;
using boost::date_time::Oct;
using boost::date_time::Nov;
using boost::date_time::Dec;
using boost::date_time::NotAMonth;
using boost::date_time::NumMonths;

typedef boost::date_time::weekdays Weekdays;

using boost::date_time::weekdays;
using boost::date_time::Sunday;
using boost::date_time::Monday;
using boost::date_time::Tuesday;
using boost::date_time::Wednesday;
using boost::date_time::Thursday;
using boost::date_time::Friday;
using boost::date_time::Saturday;

typedef boost::date_time::special_values SpecialValues;

using boost::date_time::special_values;
using boost::date_time::not_special;
using boost::date_time::neg_infin;
using boost::date_time::pos_infin;
using boost::date_time::not_a_date_time;
using boost::date_time::max_date_time;
using boost::date_time::min_date_time;

/** @} */

/**
 * @defgroup DateTimeConstants
 *
 * tse constants that related to the DateTime functionality.
 *
 * @{
 */

extern const std::string NO_DATE_STRING;
extern const std::string MONTHS_UPPER_CASE[];
extern const std::string SHORT_MONTHS_UPPER_CASE[];
extern const std::string SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[];
extern const std::string WEEKDAYS_UPPER_CASE[];

constexpr uint32_t SECONDS_PER_MINUTE = 60;
constexpr uint32_t SECONDS_PER_HOUR = 3600;
constexpr uint32_t SECONDS_PER_DAY = 86400;
constexpr uint32_t MINUTES_PER_HOUR = 60;
constexpr uint32_t HOURS_PER_DAY = 24;
constexpr int64_t MICROSECONDS_PER_HOUR = SECONDS_PER_HOUR * 1000000LL;

/** @} */

/**
 * @class DateTime DateTime.h
 *
 * @brief DateTime functionality for TSE.
 *
 * This class is just wrapper for boost date/time classes.
 * All methods are inlined, so ther is no implementation file
 * for this class.
 *
 */

class DateTime : public boost::posix_time::ptime
{
protected:
  static const boost::int64_t DAY_DURATION = static_cast<boost::int64_t>(SECONDS_PER_DAY) * 1000000;
  static const boost::gregorian::date MIN_DATE;
  static const boost::int64_t MIN_DATE_MICROSEC;
  static const boost::gregorian::date MAX_DATE;
  static const boost::int64_t MAX_DATE_MICROSEC;
  static const boost::gregorian::date POS_INFIN_DATE;
  static const boost::int64_t POS_INFIN_MICROSEC;
  static const boost::gregorian::date NEG_INFIN_DATE;
  static const boost::int64_t NEG_INFIN_MICROSEC;
  static const boost::gregorian::date NOT_A_DATE;
  static const boost::int64_t NOT_A_DATE_MICROSEC;

public:
  boost::gregorian::date date() const;
  boost::gregorian::date dateImpl() const;

  DateTime() : ptime(emptyDate_) {}

  DateTime(time_t t) : ptime(boost::posix_time::from_time_t(t)) {}

  DateTime(const boost::posix_time::ptime& t) : ptime(t) {}

  DateTime(Year newYear, Month newMonth, Day newDay)
    : ptime(boost::gregorian::date(newYear, newMonth, newDay),
            boost::posix_time::time_duration(0, 0, 0))
  {
  }

  DateTime(DateTime& t, boost::posix_time::time_duration td) : ptime(t + td) {}

  DateTime(boost::gregorian::date d, boost::posix_time::time_duration td) : ptime(d, td) {}

  DateTime(boost::gregorian::date dt) : ptime(dt, boost::posix_time::time_duration(0, 0, 0)) {}

  DateTime(boost::gregorian::date dt, int32_t hrs, int32_t mins, int32_t secs)
    : ptime(dt, boost::posix_time::time_duration(hrs, mins, secs))
  {
  }

  DateTime(Year newYear, Month newMonth, Day newDay, int32_t hrs, int32_t mins)
    : ptime(boost::gregorian::date(newYear, newMonth, newDay),
            boost::posix_time::time_duration(hrs, mins, 0))
  {
  }

  DateTime(Year newYear, Month newMonth, Day newDay, int32_t hrs, int32_t mins, int32_t secs)
    : ptime(boost::gregorian::date(newYear, newMonth, newDay),
            boost::posix_time::time_duration(hrs, mins, secs))
  {
  }

  DateTime(Year newYear,
           Month newMonth,
           Day newDay,
           int32_t hrs,
           int32_t mins,
           int32_t secs,
           int32_t fsecs)
    : ptime(boost::gregorian::date(newYear, newMonth, newDay),
            boost::posix_time::time_duration(hrs, mins, secs, fsecs))
  {
  }

  DateTime(const std::string& str) : ptime(boost::posix_time::time_from_string(str)) {}

  DateTime(const std::string& d, int32_t mins)
    : ptime(boost::gregorian::date(boost::gregorian::from_string(d)),
            boost::posix_time::time_duration((mins / 60), (mins % 60), 0))
  {
  }

  DateTime(boost::date_time::special_values sv) : ptime(boost::gregorian::date(sv)) {}

  DateTime(const DateTime& t) : ptime(t) {}

  static DateTime fromMilitaryTime(int32_t militaryTime)
  {
    return DateTime(
        boost::posix_time::second_clock::local_time().date(),
        boost::posix_time::time_duration((militaryTime / 100), (militaryTime % 100), 0));
  }

  static DateTime fromMinutes(int32_t mins)
  {
    return DateTime(boost::posix_time::second_clock::local_time().date(),
                    boost::posix_time::time_duration((mins / 60), (mins % 60), 0));
  }

  static DateTime localTime();

  /* Empty date (1980-1-1)
   * This date is used by Sabre content team to repalce
   * empty dates in ATPCO documents. */
  static const DateTime& emptyDate() { return emptyDate_; }

  /* Open date (1966-1-1)
   * This date is used for segments with unspecified date */
  static const DateTime& openDate() { return openDate_; }

  Year year() const { return date().year(); }

  Month month() const { return date().month(); }

  Day day() const { return date().day(); }

  int32_t hours() const { return time_of_day().hours(); }

  int32_t minutes() const { return time_of_day().minutes(); }

  int32_t seconds() const { return time_of_day().seconds(); }

  void set64BitRep(int64_t nRep)
  {
    time_ = boost::posix_time::int64_time_rep(nRep + (12 * MICROSECONDS_PER_HOUR));
  }

  void setIntRep(int64_t nRep) { time_ = boost::posix_time::int64_time_rep(nRep); }

  int64_t get64BitRep() const
  {
    return (time_.get_rep().as_number() - (12 * MICROSECONDS_PER_HOUR));
  }

  int64_t getIntRep() const { return time_.get_rep().as_number(); }

  int64_t get64BitRepDateOnly() const
  {
    return ((time_.get_rep().as_number() / (24 * MICROSECONDS_PER_HOUR)) *
                (24 * MICROSECONDS_PER_HOUR) -
            (12 * MICROSECONDS_PER_HOUR));
  }

  uint64_t totalSeconds() const { return static_cast<uint32_t>(time_of_day().total_seconds()); }

  uint64_t totalMinutes() const
  {
    unsigned totalMinutes =
        static_cast<uint32_t>(time_of_day().hours() * 60 + time_of_day().minutes());
    return totalMinutes;
  }

  long fractionalSeconds() const { return time_of_day().fractional_seconds(); }

  DayOfWeek dayOfWeek() const { return (date().day_of_week()); }

  static int64_t diffTime(const DateTime& t1, const DateTime& t2)
  {
    return ((t1 - t2).total_seconds());
  }

  static int16_t endOfMonthDay(Year y, Month m)
  {
    return static_cast<int16_t>(GregorianCalendar::end_of_month_day(y, m));
  }

  bool isBetween(const DateTime& t1, const DateTime& t2) const
  {
    return ((*this >= t1) && (*this <= t2));
  }

  bool isBetween(const int32_t& fromYear,
                 const int32_t& fromMonth,
                 const int32_t& fromDay,
                 const int32_t& toYear,
                 const int32_t& toMonth,
                 const int32_t& toDay) const;

  bool
  isBetweenF(int fromYear, int fromMonth, int fromDay, int toYear, int toMonth, int toDay) const;

  static bool isLeapYear(const int32_t aYear)
  {
    // if year is divisible by 4 and either not divisable by 100 or divisable by 400 it's a leap
    // year
    // return ((year % 4 == 0) && ((year % 100 != 0)||(year % 400 == 0)));
    return (GregorianCalendar::is_leap_year(static_cast<uint16_t>(aYear)));
  }

  static bool isLeapYear(const DateTime& dt)
  {
    return (GregorianCalendar::is_leap_year(dt.year()));
  }

  bool isLeapYear() const { return (GregorianCalendar::is_leap_year(date().year())); }

  bool isMatchingDayOfWeek(const DayOfWeek& dow) const { return ((date().day_of_week() == dow)); }

  bool isEmptyDate() const { return ((*this == emptyDate())); }

  bool isOpenDate() const { return (*this == openDate()); }

  bool isInfinity() const { return date().is_infinity(); }

  bool isPosInfinity() const { return date().is_pos_infinity(); }

  bool isNegInfinity() const { return date().is_neg_infinity(); }

  bool isNotADate() const { return date().is_not_a_date(); }

  bool isValid() const { return (!(isEmptyDate() || isOpenDate() || isInfinity())); }

  DateTime addSeconds(long numberOfSecs) const
  {
    return *this + boost::posix_time::seconds(numberOfSecs);
  }

  DateTime subtractSeconds(long numberOfSecs) const
  {
    return *this - boost::posix_time::seconds(numberOfSecs);
  }

  DateTime addDays(uint32_t numberOfDays) const;

  DateTime nextDay() const;

  DateTime subtractDays(uint32_t numberOfDays) const;

  DateTime addWeeks(int32_t numberOfWeeks) const;

  DateTime addMonths(int32_t numberOfMonths) const;

  DateTime subtractMonths(int32_t numberOfMonths) const;

  DateTime addYears(int32_t numberOfYears) const;

  void addMinutes(int numberOfMinutes);
  void setHistoricalIncludesTime();
  inline bool historicalIncludesTime() const { return seconds() == 1; }

  DateTime getFutureDayOfWeek(const Weekdays aDayOfWeek, int32_t number = 1) const;

  bool isFutureDate();

  DateTime getFutureDate(const Weekdays aDayOfWeek, int32_t number = 1) const
  {
    return (getFutureDayOfWeek(aDayOfWeek, number));
  }

  DateTime getOnlyDate() const
  {
    return DateTime(year(), month(), day());
  }

  std::string toSimpleString() const
  {
    if (UNLIKELY(!isValid()))
      return NO_DATE_STRING;

    return boost::posix_time::to_simple_string(*this);
  }

  std::string toIsoString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::posix_time::to_iso_string(*this);
  }

  std::string toIsoExtendedString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::posix_time::to_iso_extended_string(*this);
  }

  std::string dateToSimpleString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::gregorian::to_simple_string(date());
  }

  std::string dateToIsoString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::gregorian::to_iso_string(date());
  }

  std::string dateToIsoExtendedString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::gregorian::to_iso_extended_string(date());
  }

  std::string dateToSqlString() const
  {
    if (!isValid())
      return NO_DATE_STRING;

    return boost::gregorian::to_sql_string(date());
  }

  std::string timeToSimpleString() const
  {
    return boost::posix_time::to_simple_string(time_of_day());
  }

  std::string dateToString(DateFormat format, const char* separator) const;

  std::string timeToString(TimeFormat format, const char* separator) const;

  static DateTime earlier(const DateTime& dt1, const DateTime& dt2)
  {
    if (dt1.isOpenDate())
      return dt2;
    if (dt2.isOpenDate())
      return dt1;

    if (dt1 < dt2)
      return dt1;
    else
      return dt2;
  }

  static DateTime later(const DateTime& dt1, const DateTime& dt2)
  {
    if (dt1.isOpenDate())
      return dt2;
    if (dt2.isOpenDate())
      return dt1;

    if (dt1 > dt2)
      return dt1;
    else
      return dt2;
  }

  void setWithEarlier(const DateTime& dt1)
  {
    if (!dt1.isValid())
      return;

    if (this->isOpenDate() || (*this > dt1))
      *this = dt1;
  }

  void setWithLater(const DateTime& dt1)
  {
    if (this->isOpenDate() || (*this < dt1))
      *this = dt1;
  }

  // initialize DayOfWeek that we want to be out of range (0-6)
  static uint32_t invalidDOW() { return 8; }

  bool isRangeInBetween(const int32_t& fromYear,
                        const int32_t& fromMonth,
                        const int32_t& fromDay,
                        const int32_t& toYear,
                        const int32_t& toMonth,
                        const int32_t& toDay,
                        const DateTime& latestDate) const;

  bool
  isRangeInBetween(const DateTime& fromDate, const DateTime& toDate, const DateTime& lastDate) const
  {
    return (*this <= toDate && lastDate >= fromDate);
  }

  static DateTime convertDate(const char* inDate);

private:
  static const DateTime emptyDate_;
  static const DateTime openDate_;
}; // class DateTime
} // tse namespace
