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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <time.h>

#include "TseEnums.h"

namespace tse {

/**
 * @defgroup BoostTypedefs
 *
 * Bring boost typedefs and enums into the tse namespace
 *
 * @{
 */
typedef boost::gregorian::greg_year      Year;
typedef boost::gregorian::greg_month     Month;
typedef boost::gregorian::greg_day       Day;
typedef boost::gregorian::greg_weekday   DayOfWeek;
typedef boost::posix_time::time_duration TimeDuration;
typedef boost::posix_time::hours         Hours;
typedef boost::posix_time::minutes       Minutes;
typedef boost::posix_time::seconds       Seconds;

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

static const std::string NO_DATE_STRING = "N/A";

static const std::string MONTHS_UPPER_CASE[] =
  {"NNN", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

static const std::string SHORT_MONTHS_UPPER_CASE[] =
  {"NN", "JA", "FE", "MR", "AP", "MY", "JN", "JL", "AU", "SE", "OC", "NO", "DE"};

static const std::string SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[] =
  {"NN", "JA", "FE", "MR", "AP", "MY", "JN", "JL", "AU", "SE", "OC", "NV", "DE"};

static const std::string WEEKDAYS_UPPER_CASE[] =
{"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

static const uint32_t SECONDS_PER_MINUTE = 60;
static const uint32_t SECONDS_PER_HOUR   = 3600;
static const uint32_t SECONDS_PER_DAY    = 86400;
static const uint32_t MINUTES_PER_HOUR   = 60;
static const uint32_t HOURS_PER_DAY      = 24;
static const int64_t MICROSECONDS_PER_HOUR = SECONDS_PER_HOUR * 1000000LL;

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
public:

/**
 * @defgroup Constructors
 *
 * @{
 */

/**
 * @brief Default constructor.
 *
 * Creates object that contains local time.
 */
  //DateTime() : ptime(boost::gregorian::date(1980, 1, 1),
	//	           boost::posix_time::time_duration(0,0,0))
  DateTime() : ptime(emptyDate())
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of time_t value.
 *
 * @param time_t t
 */
  DateTime(time_t t) : ptime(boost::posix_time::from_time_t(t))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of boost::posix_time::ptime object.
 *
 * @param const boost::posix_time::ptime& t
 *
 */
  DateTime(const boost::posix_time::ptime& t) : ptime(t)
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of Year, Month, Day values.
 * Time portion of DateTime set up to zeroes
 *
 * @param Year year
 * @param Month month
 * @param Day day
 */
  DateTime(Year newYear, Month newMonth, Day newDay)
	  : ptime(boost::gregorian::date(newYear, newMonth, newDay),
		  boost::posix_time::time_duration(0,0,0))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of DateTime object plus time duration
 *
 * @param DateTime& t
 * @param boost::posix_time::time_duration td
 */
  DateTime(DateTime& t, boost::posix_time::time_duration td)
	  : ptime(t+td)
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of boost::gregorian::date object
 * plus time duration
 *
 * @param boost::gregorian::date d
 * @param boost::posix_time::time_duration td
 */
  DateTime(boost::gregorian::date d, boost::posix_time::time_duration td)
	  : ptime(d, td)
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of boost::gregorian::date object.
 * Time portion of DateTime set up to zeroes
 *
 * @param boost::gregorian::date d
 */
  DateTime(boost::gregorian::date dt)
	  : ptime(dt, boost::posix_time::time_duration(0, 0, 0))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of boost::gregorian::date object and
 * time integer values.
 *
 * @param boost::gregorian::date d
 * @param int32_t hrs
 * @param int32_t mins
 * @param int32_t secs
 */
  DateTime(boost::gregorian::date dt,
           int32_t                hrs,
           int32_t                mins,
           int32_t                secs)
	  : ptime(dt, boost::posix_time::time_duration(hrs, mins, secs))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of date and time integer values
 *
 * @param Year year
 * @param Month month
 * @param Day day
 * @param int32_t hrs
 * @param int32_t mins
 * @param int32_t secs
 */
  DateTime(Year newYear,
           Month newMonth,
	         Day newDay,
	         int32_t hrs,
	         int32_t mins,
	         int32_t secs)
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
			  boost::posix_time::time_duration(hrs, mins, secs,fsecs))
	{
	}

/**
 * @brief Constructor.
 *
 * Creates object out of string
 * Exapmle of string format : "2004-11-07 23:13:44.000"
 *
 * @param std::string& str
 */
  DateTime(std::string& str) : ptime(boost::posix_time::time_from_string(str))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates object out of string date and total minutes integer
 *
 * Date could be represented with strings "2004/11/07" or "2004-Nov-7"
 * Total minutes is an integer in the range between 0 and 1440
 *
 * @param std::string& d
 * @param int32_t      mins
 */
  DateTime(const std::string& d, int32_t mins)
	  : ptime(boost::gregorian::date(boost::gregorian::from_string(d)),
	          boost::posix_time::time_duration((mins/60), (mins%60), 0))
  {
  }

/**
 * @brief Constructor.
 *
 * Creates special value DateTime object
 *
 * boost supports folowing special values:
 * neg_infin
 * pos_infin
 * not_a_date_time
 * max_date
 * min_date
 *
 * Please consult with boost documentation on proper use of
 * the special values.
 *
 * @param boost::date_time::special_values sv
 */
  DateTime(boost::date_time::special_values sv) : ptime(boost::gregorian::date(sv))
  {
  }

/**
 * @brief Copy Constructor.
 *
 * Creates copy of DateTime object.
 *
 * @param const DateTime& t
 */
  DateTime(const DateTime& t) : ptime(t)
  {
  }

/**
 * @brief DateTime Constructor.
 *
 * Creates object out of todays date and military time
 *
 * Military time is an integer in the range between 0 and 2400
 *
 * @return DateTime
 *
 * @param int32_t      militaryTime
 */
  static DateTime
  fromMilitaryTime(int32_t militaryTime)
  {
    return DateTime(boost::posix_time::second_clock::local_time().date(),
                    boost::posix_time::time_duration((militaryTime/100), (militaryTime%100), 0) );
  }

/**
 * @brief DateTime Constructor.
 *
 * Creates object out of todays date and total minutes integer value
 *
 * Total minutes is an integer in the range between 0 and 1440
 *
 * @return DateTime
 *
 * @param int32_t      mins
 *
 */
  static DateTime
  fromMinutes(int32_t mins)
  {
    return DateTime(boost::posix_time::second_clock::local_time().date(),
                    boost::posix_time::time_duration((mins/60), (mins%60), 0) );
  }
/**
 * @brief DateTime Constructor.
 *
 * Creates object that holds local time
 *
 * @return DateTime
 *
 */
  static DateTime
  localTime();

/**
 * @brief DateTime Constructor.
 *
 * Creates object that holds empty date (1980-1-1)
 * This date is used by Sabre content team to repalce
 * empty dates in ATPCO documents.
 *
 * @return DateTime
 *
 */
static const DateTime& emptyDate();

/**
 * @brief DateTime Constructor.
 *
 * Creates object that holds Sabre birthday (1966-1-1)
 * This date is used for segments with unspecified date
 * @return DateTime
 *
 */
  static const DateTime&
  openDate();

/** @} */

/**
 * @defgroup Accessors
 *
 * @{
 */

  Year
  year() const
  {
    return date().year();
  }

  Month
  month() const
  {
    return date().month();
  }

  Day
  day() const
  {
    return date().day();
  }

  int32_t
  hours() const
  {
    return time_of_day().hours();
  }

  int32_t
  minutes() const
  {
    return time_of_day().minutes();
  }

  int32_t
  seconds() const
  {
    return time_of_day().seconds();
  }

  void
  set64BitRep(int64_t nRep)
  {
      time_ = boost::posix_time::int64_time_rep(nRep + (12 * MICROSECONDS_PER_HOUR));
  }

  void setIntRep (int64_t nRep)
  {
      time_ = boost::posix_time::int64_time_rep(nRep);
  }

  int64_t
  get64BitRep() const
  {
    return (time_.get_rep().as_number() - (12 * MICROSECONDS_PER_HOUR));
  }

  int64_t getIntRep () const
  {
    return time_.get_rep().as_number();
  }

  int64_t
  get64BitRepDateOnly() const
  {
    return ((time_.get_rep().as_number() / (24 * MICROSECONDS_PER_HOUR)) *
        (24 * MICROSECONDS_PER_HOUR) - (12 * MICROSECONDS_PER_HOUR));
  }

/**
 * Returns number of total seconds out of time offset
 * in the day
 */
  uint64_t
  totalSeconds() const
  {
    return time_of_day().total_seconds();
  }

/**
 * Returns number of total minutes out of time offset
 * in the day
 */
  uint64_t
  totalMinutes() const
  {
      return time_of_day().hours()*60 + time_of_day().minutes(); //lint !e647
  }

  long fractionalSeconds() const
  {
      	return time_of_day().fractional_seconds();
  }

/**
 * Returns DayOfWeek enumeration value
 */
  DayOfWeek
  dayOfWeek() const
  {
    return (date().day_of_week());
  }
/** @} */

/**
 * @defgroup Helpers
 *
 * @{
 */

  static int64_t
  diffTime(const DateTime& t1, const DateTime& t2)
  {
    return ( (t1 - t2).total_seconds() );
  }

  static int16_t
  endOfMonthDay(Year y, Month m)
  {
    return ( boost::gregorian::gregorian_calendar::end_of_month_day(y, m) );
  }

/**
 * Returns true if value of the object is between t1 and t2
 *
 */
  bool
  isBetween(const DateTime& t1, const DateTime& t2) const
  {
    return ((*this >= t1) && (*this <= t2));
  }

/**
 * Returns true if value of the object is between two dates
 *
 * @param const int& fromYear
 * @param const int& fromMonth
 * @param const int& fromDay
 * @param const int& toYear
 * @param const int& toMonth
 * @param const int& toDay
 *
 */
  bool
  isBetween(const int32_t& fromYear, const int32_t& fromMonth, const int32_t& fromDay,
            const int32_t& toYear, const int32_t& toMonth, const int32_t& toDay) const
  {
      int32_t localFromDay = fromDay;
      int32_t localToDay = toDay;

      // if no year only compare month and day
      if(fromYear == 0 && toYear == 0)
      {
          const int32_t thisMonth = this->month();
          const int32_t thisDay = this->day();

          if ( (fromMonth == toMonth) && (fromDay == toDay) )
          {
              if (fromDay == 0)
              {
                  // any day in this month
                  return (thisMonth == fromMonth);
              }
              else
              {
                  // exact date
                  return ((thisMonth == fromMonth) && (thisDay == fromDay));
              }
          }
          else if ( fromMonth < toMonth ||
              (fromMonth == toMonth && (toDay == 0 || fromDay < toDay)) )
          {
              // from to to same year
              // no leap year check, should use toDay = 0 if want end of Febrary
              // included
              if (thisMonth < fromMonth)
                  return false;
              else if (thisMonth == fromMonth)
              {
                  if ((fromDay != 0) && (thisDay < fromDay))
                      return false;
              }

              if (thisMonth > toMonth)
                  return false;
              else if (thisMonth == toMonth)
              {
                  if ((toDay != 0) && (thisDay > toDay))
                      return false;
              }
              return true;
          }
          else
          {
              // from last year to this year, or
              // from this year to next year
              if (thisMonth > fromMonth)
                  return true;
              else if (thisMonth == fromMonth)
              {
                  if (thisDay >= fromDay)
                      return true;
              }

              if (thisMonth < toMonth)
                  return true;
              else if (thisMonth == toMonth)
              {
                  if (toDay == 0 || thisDay <= toDay)
                      return true;
              }
              return false;
          }
      }
      else
      {
          /* @TODO I think the DBAccess should give us correct dates
          */
          int32_t adjustedFromYear = 0;
          int32_t adjustedToYear = 0;
          if(fromYear < 2000)
              adjustedFromYear = fromYear + 2000;
          else
              adjustedFromYear = fromYear;

          if(toYear < 2000)
              adjustedToYear = toYear + 2000;
          else
              adjustedToYear = toYear;

          // if the day params are 0 it is for entire month
          if(fromDay == 0)
          {
              localFromDay = 1;
          }
          else
          {
              localFromDay = fromDay;
          }

          if(toDay == 0)
          {
              localToDay = endOfMonthDay(adjustedToYear, toMonth);
          }
          else
          {
              localToDay = toDay;
          }


          // make adjustment for leap year, this should only apply if year was 0 form data base
          if(!isLeapYear(adjustedFromYear))
          {
              if(fromMonth == 2)
              {
                  if(localFromDay > 28)
                  {
                      localFromDay = 28;
                  }
              }
          }

          // make adjustment for leap year
          if(!isLeapYear(adjustedToYear))
          {
              if(toMonth == 2)
              {
                  if(localToDay > 28)
                  {
                      localToDay = 28;
                  }
              }
          }

          DateTime fromDate(adjustedFromYear, fromMonth, localFromDay);
          DateTime toDate(adjustedToYear, toMonth, localToDay);
          // create a date object with no time from this date object
          DateTime thisDate(this->year(), this->month(), this->day());

          return (thisDate.isBetween(fromDate, toDate));
      }
  }

  bool isBetweenF (int fromYear, int fromMonth, int fromDay,
                   int toYear, int toMonth, int toDay) const
  {
      int32_t localFromDay = fromDay;
      int32_t localToDay = toDay;

      // if no year only compare month and day
      if(fromYear == 0 && toYear == 0)
      {
          const int32_t thisMonth = this->month();
          const int32_t thisDay = this->day();

          if ( (fromMonth == toMonth) && (fromDay == toDay) )
          {
              if (fromDay == 0)
              {
                  // any day in this month
                  return (thisMonth == fromMonth);
              }
              else
              {
                  // exact date
                  return ((thisMonth == fromMonth) && (thisDay == fromDay));
              }
          }
          else if ( fromMonth < toMonth ||
              (fromMonth == toMonth && (toDay == 0 || fromDay < toDay)) )
          {
              // from to to same year
              // no leap year check, should use toDay = 0 if want end of Febrary
              // included
              if (thisMonth < fromMonth)
                  return false;
              else if (thisMonth == fromMonth)
              {
                  if ((fromDay != 0) && (thisDay < fromDay))
                      return false;
              }

              if (thisMonth > toMonth)
                  return false;
              else if (thisMonth == toMonth)
              {
                  if ((toDay != 0) && (thisDay > toDay))
                      return false;
              }
              return true;
          }
          else
          {
              // from last year to this year, or
              // from this year to next year
              if (thisMonth > fromMonth)
                  return true;
              else if (thisMonth == fromMonth)
              {
                  if (thisDay >= fromDay)
                      return true;
              }

              if (thisMonth < toMonth)
                  return true;
              else if (thisMonth == toMonth)
              {
                  if (toDay == 0 || thisDay <= toDay)
                      return true;
              }
              return false;
          }
      }
      else
      {
          /* @TODO I think the DBAccess should give us correct dates
          */
          int32_t adjustedFromYear = 0;
          int32_t adjustedToYear = 0;
          if(fromYear < 2000)
              adjustedFromYear = fromYear + 2000;
          else
              adjustedFromYear = fromYear;

          if(toYear < 2000)
              adjustedToYear = toYear + 2000;
          else
              adjustedToYear = toYear;

          // if the day params are 0 it is for entire month
          if(fromDay == 0)
          {
              localFromDay = 1;
          }
          else
          {
              localFromDay = fromDay;
          }

          if(toDay == 0)
          {
              localToDay = endOfMonthDay(adjustedToYear, toMonth);
          }
          else
          {
              localToDay = toDay;
          }


          // make adjustment for leap year, this should only apply if year was 0 form data base
          if(!isLeapYear(adjustedFromYear))
          {
              if(fromMonth == 2)
              {
                  if(localFromDay > 28)
                  {
                      localFromDay = 28;
                  }
              }
          }

          // make adjustment for leap year
          if(!isLeapYear(adjustedToYear))
          {
              if(toMonth == 2)
              {
                  if(localToDay > 28)
                  {
                      localToDay = 28;
                  }
              }
          }

          if (year() < adjustedFromYear || year() > adjustedToYear)
          {
            return false;
          }
          if (adjustedFromYear == year())
          {
            if (month() < fromMonth)
            {
              return false;
            }
            if (fromMonth == month())
            {
              if (day() < localFromDay)
              {
                return false;
              }
            }
          }
          if (adjustedToYear == year())
          {
            if (month() > toMonth)
            {
              return false;
            }
            if (month() == toMonth)
            {
              if (day() > localToDay)
              {
                return false;
              }
            }
          }
          return true;
      }
  }

  static bool
  isLeapYear(const int32_t aYear)
  {
      //if year is divisible by 4 and either not divisable by 100 or divisable by 400 it's a leap year
      // return ((year % 4 == 0) && ((year % 100 != 0)||(year % 400 == 0)));
      return ( boost::gregorian::gregorian_calendar::is_leap_year(aYear) );
  }

  static bool
  isLeapYear(const DateTime& dt)
  {
      return ( boost::gregorian::gregorian_calendar::is_leap_year(dt.year()) );
  }

  bool
  isLeapYear() const
  {
    return ( boost::gregorian::gregorian_calendar::is_leap_year(date().year()) );
  }

  bool
  isMatchingDayOfWeek(const DayOfWeek& dow) const
  {
    return ((date().day_of_week() == dow ));
  }

/**
 * Returns true if DateTime object holds empty date
 * 1980-1-1
 * This date is used by Sabre content team to repalce
 * empty dates in ATPCO documents.
 */
  bool
  isEmptyDate() const
  {
    return( (*this == emptyDate()) );
  }

/**
 * Returns true if DateTime object holds empty date
 * 1966-1-1
 * This date is used for open segment define
 */
  bool
  isOpenDate() const
  {
    return( (*this == openDate()) );
  }

/**
 * Returns true if DateTime object
 * is one of the infinities.
 */
  bool isInfinity() const { return date().is_infinity(); }

/**
 * Returns true if DateTime object
 * is positive infinities.
 */
  bool isPosInfinity() const { return date().is_pos_infinity(); }

/**
 * Returns true if DateTime object
 * is negative infinities.
 */
  bool isNegInfinity() const { return date().is_neg_infinity(); }

/**
 * Returns true if DateTime object
 * is not a date.
 */
  bool isNotADate() const { return date().is_not_a_date(); }

/**
 * Returns true if DateTime object is not emptyDate and
 * is not Sabre Birthday and
 * is not one of the infinities.
 */
  bool
  isValid() const
  {
    return ( !(isEmptyDate() || isOpenDate() || isInfinity()) );
  }

/** @} */

/**
 * @defgroup Changers
 *
 * functions that take current DateTime object and
 * return new changed object
 *
 * @{
 */
  DateTime
  addSeconds(int numberOfSecs) const
  {
    return *this + boost::posix_time::seconds(numberOfSecs);
  }

  DateTime
  subtractSeconds(int numberOfSecs) const
  {
    return *this - boost::posix_time::seconds(numberOfSecs);
  }

  DateTime
  addDays(uint32_t numberOfDays) const
  {
    boost::gregorian::day_iterator it(this->date());

    for (uint32_t i = 0; i < numberOfDays; ++i)
    {
      ++it;
    }
    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

  DateTime
  nextDay() const
  {
    boost::gregorian::day_iterator it(this->date());
    ++it;
    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

  DateTime
  subtractDays(uint32_t numberOfDays) const
  {
    boost::gregorian::day_iterator it(this->date());

    for (uint32_t i = 0; i < numberOfDays; ++i)
    {
      --it;
    }
    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

  DateTime
  addWeeks(int32_t numberOfWeeks) const
  {
    boost::gregorian::week_iterator it(this->date());

    for (int32_t i = 0; i < numberOfWeeks; ++i)
    {
      ++it;
    }

    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }


  DateTime
  addMonths(int32_t numberOfMonths) const
  {
    boost::gregorian::month_iterator it(this->date());

    for (int32_t i = 0; i < numberOfMonths; ++i)
    {
      ++it;
    }

    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

  DateTime
  subtractMonths(int32_t numberOfMonths) const
  {
    boost::gregorian::month_iterator it(this->date());

    for (int32_t i = 0; i < numberOfMonths; ++i)
    {
      --it;
    }

    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

  DateTime
  addYears(int32_t numberOfYears) const
  {
    boost::gregorian::year_iterator it(this->date());

    for (int32_t i = 0; i < numberOfYears; ++i)
    {
      ++it;
    }

    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );

  }


/**
 * Returns new DateTime object which represent DateTime of future
 * day of week.
 *
 * @param Weekdays dayOfWeek
 * @param int32_t number - number of weeks
 *
 */
  DateTime
  getFutureDayOfWeek(Weekdays aDayOfWeek, int32_t number = 1)
  {
    boost::date_time::first_kday_after<boost::gregorian::date> firstKDayAfter(aDayOfWeek);
    boost::gregorian::date futureDate = firstKDayAfter.get_date(this->date());

    boost::gregorian::week_iterator it(futureDate);

    for (int32_t i = 1; i < number; ++i)
    {
      ++it;
    }

    return ( ptime(*it,
                   boost::posix_time::time_duration(hours(),
                                                    minutes(),
                                                    seconds())) );
  }

/**
 * Checks if this is a future date or not .
 *
 * @return bool - true, yes it is a future date,else false
 */
  bool
  isFutureDate()
  {
  DateTime currentDate = DateTime::localTime();

     if (date().year()  >  currentDate.year())
        return true;
     else if (date().year()  <  currentDate.year())
        return false;
     else if (date().month() >  currentDate.month())
        return true;
     else if (date().month() <  currentDate.month())
        return false;
     else if (date().day()   >  currentDate.day())
        return true;
     else if (date().day()   <  currentDate.day())
        return false;

     return false;
  }

/**
 * Returns new DateTime object which represent DateTime of future
 * day of week.
 *
 * @param Weekdays dayOfWeek
 * @param int32_t number - number of weeks
 *
 * @see   DateTime::getFutureDayOfWeek(Weekdays, int32_t)

 * @deprecated
 */
  DateTime
  getFutureDate(Weekdays aDayOfWeek, int32_t number = 1)
  {
    return(getFutureDayOfWeek(aDayOfWeek, number) );
  }

/** @} */

/**
 * @defgroup to string converters
 *
 * functions that cnvert current DateTime object
 * into the string
 *
 * @{
 */
  std::string
  toSimpleString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return  boost::posix_time::to_simple_string(*this);
  }

  std::string
  toIsoString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return boost::posix_time::to_iso_string(*this);
  }

  std::string
  toIsoExtendedString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return boost::posix_time::to_iso_extended_string(*this);
  }

  std::string
  dateToSimpleString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return boost::gregorian::to_simple_string(date());
  }

  std::string
  dateToIsoString() const
  {
      if ( !isValid() )
          return NO_DATE_STRING;

      return boost::gregorian::to_iso_string(date());
  }

  std::string
  dateToIsoExtendedString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return boost::gregorian::to_iso_extended_string(date());
  }

  std::string
  dateToSqlString() const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    return boost::gregorian::to_sql_string(date());
  }

  std::string
  timeToSimpleString() const
  {
    return  boost::posix_time::to_simple_string(time_of_day());
  }

/**
 * Converts date part ofDateTime object to string
 *
 * @param DateFormat format
 * @param char separator
 */
  std::string
  dateToString(DateFormat format, const char* separator ) const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    std::ostringstream ssDate;
    std::ostringstream ssYear;
    std::ostringstream ssMonth;
    std::ostringstream ssDay;

    ssYear << date().year();
    ssMonth << std::setw(2) << std::setfill('0') << ((unsigned short) date().month());
    ssDay << std::setw(2) << std::setfill('0') << date().day();

    uint32_t monthNum = date().month();

    switch (format)
    {
      case YYYYMMDD :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssYear.str()
                 << ssMonth.str()
                 << ssDay.str();
        }
        else
        {
          ssDate << ssYear.str() << separator
                 << ssMonth.str() << separator
                 << ssDay.str();
        }
        break;

      case YYYYMmmDD :
        if (separator == 0 || *separator == '\0')
        {
          ssDate <<  ssYear.str()
                 << date().month().as_short_string()
                 << ssDay.str();
        }
        else
        {
          ssDate <<  ssYear.str() << separator
                 << date().month().as_short_string() << separator
                 << ssDay.str();
        }
        break;

      case YYYYMMMDD :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssYear.str()
                 << MONTHS_UPPER_CASE[monthNum]
                 << ssDay.str();
        }
        else
        {
          ssDate << ssYear.str() << separator
                 << MONTHS_UPPER_CASE[monthNum] << separator
                 << ssDay.str();
        }
        break;

      case MMDDYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssMonth.str()
                 << ssDay.str()
                 << ssYear.str();
        }
        else
        {
          ssDate << ssMonth.str() << separator
                 << ssDay.str() << separator
                 << ssYear.str();
        }
        break;

      case MMDDYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssMonth.str()
                 << ssDay.str()
                 << ssYear.str().substr(2,2);
        }
        else
        {
          ssDate << ssMonth.str() << separator
                 << ssDay.str() << separator
                 << ssYear.str().substr(2,2);
        }
        break;

      case MmmDDYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << date().month().as_short_string()
                 << ssDay.str()
                 << ssYear.str();
        }
        else
        {
          ssDate << date().month().as_short_string() << separator
                 << ssDay.str() << separator
                 << ssYear.str();
        }
        break;

      case MMMDDYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << MONTHS_UPPER_CASE[monthNum]
                 << ssDay.str()
                 << ssYear.str();
        }
        else
        {
          ssDate << MONTHS_UPPER_CASE[monthNum] << separator
                 << ssDay.str() << separator
                 << ssYear.str();
        }
        break;

      case MMMDDYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << MONTHS_UPPER_CASE[monthNum]
                 << ssDay.str()
                 << ssYear.str().substr(2,2);
        }
        else
        {
          ssDate << MONTHS_UPPER_CASE[monthNum] << separator
                 << ssDay.str() << separator
                 << ssYear.str().substr(2,2);
        }
        break;

      case DDMMYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssDay.str()
                 << ssMonth.str()
                 << ssYear.str();
        }
        else
        {
          ssDate << ssDay.str() << separator
                 << ssMonth.str() << separator
                 << ssYear.str();
        }
        break;

      case DDMmmYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssDay.str()
                 << date().month().as_short_string()
                 << ssYear.str();
        }
        else
        {
          ssDate << ssDay.str() << separator
                 << date().month().as_short_string() << separator
                 << ssYear.str();
        }
        break;

      case DDMMMYYYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssDay.str()
                 << MONTHS_UPPER_CASE[monthNum]
                 << ssYear.str();
        }
        else
        {
          ssDate << ssDay.str() << separator
                 << MONTHS_UPPER_CASE[monthNum] << separator
                 << ssYear.str();
        }
        break;

      case DDMMMYY :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssDay.str()
                 << MONTHS_UPPER_CASE[monthNum]
                 << ssYear.str().substr(2,2);
        }
        else
        {
          ssDate << ssDay.str() << separator
                 << MONTHS_UPPER_CASE[monthNum] << separator
                 << ssYear.str().substr(2,2);
        }
        break;

      case DDMMM :
        if (separator == 0 || *separator == '\0')
        {
          ssDate << ssDay.str()
                 << MONTHS_UPPER_CASE[monthNum];
        }
        else
        {
          ssDate << ssDay.str() << separator
                 << MONTHS_UPPER_CASE[monthNum];
        }
        break;

      case DDMMY :
        ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE[monthNum]
               << ssYear.str().substr(3,1);
        break;

      case DDMM :
        ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE[monthNum];
        break;

      case DDMM_FD :
        ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[monthNum];
        break;

      default :
        ssDate << boost::posix_time::to_simple_string(*this);
    }

    return( ssDate.str() );
  }

/**
 * Converts time part of DateTime object to string
 *
 * @param TimeFormat format
 * @param char separator
 */
  std::string
  timeToString(TimeFormat format, const char* separator ) const
  {
    if ( !isValid() )
      return NO_DATE_STRING;

    std::ostringstream ssTime;
    std::ostringstream ssHours;
    std::ostringstream ssMinutes;

    uint32_t hrs = time_of_day().hours();

    ssMinutes << std::setw(2) << std::setfill('0') << time_of_day().minutes();

    switch (format)
    {
      case HHMMSS :
        {
          ssHours << std::setw(2) << std::setfill('0') << hrs;

          std::ostringstream ssSeconds;
          ssSeconds << std::setw(2) << std::setfill('0') << time_of_day().seconds();

          ssTime << ssHours.str() << separator
                 << ssMinutes.str() << separator
                 << ssSeconds.str();
          break;
        }

      case HHMM :
        {
          ssHours << std::setw(2) << std::setfill('0') << hrs;
          ssTime << ssHours.str() << separator
                 << ssMinutes.str();
          break;
        }
      case HHMM_AMPM :
        {
          std::string ampm = "AM";

          if(hrs >= 12)
          {
            hrs = hrs - 12;
            ampm = "PM";
          }

          if (hrs == 0)
            hrs=12;

          ssHours << hrs;

          ssTime << ssHours.str() << separator
                 << ssMinutes.str() << ampm;
          break;
        }
      default :
        {
          ssHours << std::setw(2) << std::setfill('0') << hrs;

          std::ostringstream ssSeconds;
          ssSeconds << std::setw(2) << std::setfill('0') << time_of_day().seconds();

          ssTime << ssHours.str() << ':'
                 << ssMinutes.str() << ':'
                 << ssSeconds.str();
          break;
        }
    }

    return( ssTime.str() );
  }
/** @} */

  static DateTime
  earlier( const DateTime& dt1, const DateTime& dt2 )
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

  static DateTime
  later( const DateTime& dt1, const DateTime& dt2 )
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
      if(!dt1.isValid())
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
  static uint32_t invalidDOW()
  {
      return 8;
  }

/**
 * Returns true if value of the object is between two dates
 *
 * @param const int& fromYear
 * @param const int& fromMonth
 * @param const int& fromDay
 * @param const int& toYear
 * @param const int& toMonth
 * @param const int& toDay
 *
 */
  bool
  isRangeInBetween(const int32_t& fromYear, const int32_t& fromMonth, const int32_t& fromDay,
            const int32_t& toYear, const int32_t& toMonth, const int32_t& toDay,
            const DateTime& latestDate) const
  {
      int32_t localFromDay = fromDay;
      int32_t localToDay = toDay;

      // if no year only compare month and day
      if(fromYear == 0 && toYear == 0)
      {
          int32_t adjustedFromYear = this->year();
          int32_t adjustedToYear = this->year();
          // we crossed over into the next year
/*          if(fromMonth > toMonth || (fromMonth == toMonth && fromDay > toDay))
          {
              if(fromMonth > this->month() && toMonth >= this->month())
              {
                  adjustedFromYear -=1;
              }
              else if(fromMonth <= this->month() && toMonth < this->month())
              {
                  adjustedToYear += 1;
              }
          }
*/
          if (fromMonth < toMonth)
          {
             if (toMonth < this->month())
             {
                adjustedFromYear++;
                adjustedToYear++;
             }
          }
          else if ( fromMonth == toMonth)
          {
             if (fromDay > toDay)
             {
                if (fromMonth > this->month())
                   adjustedFromYear--;
                else if (fromMonth == this->month() &&
                         (toDay == 0 || toDay >= this->day()))
                   adjustedFromYear--;
                else
                   adjustedToYear++;
             }
             else if (toMonth < this->month())
             {
                adjustedFromYear++;
                adjustedToYear++;
             }
             else if (toMonth == this->month() && toDay != 0 && toDay < this->day())
             {
                adjustedFromYear++;
                adjustedToYear++;
             }
          }
          else
          {
             if(fromMonth > this->month() && toMonth >= this->month())
             {
                adjustedFromYear--;
             }
             else
             {
                adjustedToYear++;
             }
          }

          // if the day params are 0 it is for entire month
          if(fromDay == 0)
          {
              localFromDay = 1;
          }
          else
          {
              localFromDay = fromDay;
          }

          if(toDay == 0)
          {
              localToDay = endOfMonthDay(adjustedToYear, toMonth);
          }
          else
          {
              localToDay = toDay;
          }

          // make adjustment for leap year
          if(!isLeapYear(adjustedFromYear))
          {
              if(fromMonth == 2)
              {
                  if(localFromDay > 28)
                  {
                      localFromDay = 28;
                  }
              }
          }

          // make adjustment for leap year
          if(!isLeapYear(adjustedToYear))
          {
              if(toMonth == 2)
              {
                  if(localToDay > 28)
                  {
                      localToDay = 28;
                  }
              }
          }

          DateTime fromDate(adjustedFromYear, fromMonth, localFromDay);
          DateTime toDate(adjustedToYear, toMonth, localToDay);
          // create a date object with no time from this date object
          DateTime thisDate(this->year(), this->month(), this->day());
          DateTime lastDate(latestDate.year(), latestDate.month(), latestDate.day());
          return (thisDate.isRangeInBetween(fromDate, toDate, lastDate));

      }
      else
      {
          // @TODO I think the DBAccess should give us correct dates
          int32_t adjustedFromYear = 0;
          int32_t adjustedToYear = 0;
          if(fromYear < 2000)
              adjustedFromYear = fromYear + 2000;
          else
              adjustedFromYear = fromYear;

          if(toYear < 2000)
              adjustedToYear = toYear + 2000;
          else
              adjustedToYear = toYear;

          // if the day params are 0 it is for entire month
          if(fromDay == 0)
          {
              localFromDay = 1;
          }
          else
          {
              localFromDay = fromDay;
          }

          if(toDay == 0)
          {
              localToDay = endOfMonthDay(adjustedToYear, toMonth);
          }
          else
          {
              localToDay = toDay;
          }


          // make adjustment for leap year, this should only apply if year was 0 form data base
          if(!isLeapYear(adjustedFromYear))
          {
              if(fromMonth == 2)
              {
                  if(localFromDay > 28)
                  {
                      localFromDay = 28;
                  }
              }
          }

          // make adjustment for leap year
          if(!isLeapYear(adjustedToYear))
          {
              if(toMonth == 2)
              {
                  if(localToDay > 28)
                  {
                      localToDay = 28;
                  }
              }
          }

          DateTime fromDate(adjustedFromYear, fromMonth, localFromDay);
          DateTime toDate(adjustedToYear, toMonth, localToDay);
          // create a date object with no time from this date object
          DateTime thisDate(this->year(), this->month(), this->day());
          DateTime lastDate(latestDate.year(), latestDate.month(), latestDate.day());
          return (thisDate.isRangeInBetween(fromDate, toDate, lastDate));
      }
  }

  bool
  isRangeInBetween(const DateTime& fromDate, const DateTime& toDate, const DateTime& lastDate) const
  {
    if (*this <= toDate && lastDate >= fromDate)
    {
       return true;
    }
    return false;
  }

}; // class DateTime

} // namespace tse

