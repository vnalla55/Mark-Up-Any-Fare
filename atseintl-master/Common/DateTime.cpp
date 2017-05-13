#include "Common/DateTime.h"

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TSSCacheCommon.h"
#include "DataModel/Trx.h"

#include <cstdio>

namespace tse
{
const std::string NO_DATE_STRING = "N/A";

const std::string MONTHS_UPPER_CASE[] = {
    "NNN", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

const std::string SHORT_MONTHS_UPPER_CASE[] = {
    "NN", "JA", "FE", "MR", "AP", "MY", "JN", "JL", "AU", "SE", "OC", "NO", "DE"};

const std::string SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[] = {
    "NN", "JA", "FE", "MR", "AP", "MY", "JN", "JL", "AU", "SE", "OC", "NV", "DE"};

const std::string WEEKDAYS_UPPER_CASE[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

const boost::gregorian::date
DateTime::MIN_DATE(boost::date_time::min_date_time);
const boost::int64_t
DateTime::MIN_DATE_MICROSEC(DateTime(MIN_DATE).time_.get_rep().as_number());
const boost::gregorian::date
DateTime::MAX_DATE(boost::date_time::max_date_time);
const boost::int64_t
DateTime::MAX_DATE_MICROSEC(DateTime(MAX_DATE).time_.get_rep().as_number());
const boost::gregorian::date
DateTime::POS_INFIN_DATE(boost::date_time::pos_infin);
const boost::int64_t
DateTime::POS_INFIN_MICROSEC(DateTime(POS_INFIN_DATE).time_.get_rep().as_number());
const boost::gregorian::date
DateTime::NEG_INFIN_DATE(boost::date_time::neg_infin);
const boost::int64_t
DateTime::NEG_INFIN_MICROSEC(DateTime(NEG_INFIN_DATE).time_.get_rep().as_number());
const boost::gregorian::date
DateTime::NOT_A_DATE(boost::date_time::not_a_date_time);
const boost::int64_t
DateTime::NOT_A_DATE_MICROSEC(DateTime(NOT_A_DATE).time_.get_rep().as_number());

boost::gregorian::date
DateTime::dateImpl() const
{
  boost::int64_t timeAsNumber(time_.get_rep().as_number());
  if (MIN_DATE_MICROSEC < timeAsNumber && MAX_DATE_MICROSEC > timeAsNumber)
  {
    unsigned numDays(static_cast<unsigned>(timeAsNumber / DAY_DURATION));
    return boost::gregorian::date(numDays);
  }
  if (UNLIKELY(MAX_DATE_MICROSEC == timeAsNumber))
  {
    return MAX_DATE;
  }
  if (LIKELY(POS_INFIN_MICROSEC == timeAsNumber))
  {
    return POS_INFIN_DATE;
  }
  if (MIN_DATE_MICROSEC == timeAsNumber)
  {
    return MIN_DATE;
  }
  if (NEG_INFIN_MICROSEC == timeAsNumber)
  {
    return NEG_INFIN_DATE;
  }
  if (NOT_A_DATE_MICROSEC == timeAsNumber)
  {
    return NOT_A_DATE;
  }
  return boost::posix_time::ptime::date();
}

boost::gregorian::date
DateTime::date() const
{
  return tsscache::date(*this);
}

DateTime
DateTime::localTime()
{
  const Trx* const trx = TseCallableTrxTask::currentTrx();
  if (trx != nullptr)
  {
    return trx->transactionStartTime();
  }
  else
  {
    time_t t;
    time(&t);
    tm ltime;
    localtime_r(&t, &ltime);

    return DateTime(static_cast<uint16_t>(ltime.tm_year + 1900),
                    static_cast<uint16_t>(ltime.tm_mon + 1),
                    static_cast<uint16_t>(ltime.tm_mday),
                    ltime.tm_hour,
                    ltime.tm_min,
                    ltime.tm_sec);
  }
}

const DateTime DateTime::emptyDate_ =
    DateTime(boost::gregorian::date(1980, 1, 1), boost::posix_time::time_duration(0, 0, 0));

const DateTime DateTime::openDate_ =
    DateTime(boost::gregorian::date(1966, 1, 1), boost::posix_time::time_duration(0, 0, 0));
bool
DateTime::isBetween(const int32_t& fromYear,
                    const int32_t& fromMonth,
                    const int32_t& fromDay,
                    const int32_t& toYear,
                    const int32_t& toMonth,
                    const int32_t& toDay) const
{
  int32_t localFromDay = fromDay;
  int32_t localToDay = toDay;

  // if no year only compare month and day
  if (fromYear == 0 && toYear == 0)
  {
    const int32_t thisMonth = this->month();
    const int32_t thisDay = this->day();

    if ((fromMonth == toMonth) && (fromDay == toDay))
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
    else if (fromMonth < toMonth || (fromMonth == toMonth && (toDay == 0 || fromDay < toDay)))
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
    if (UNLIKELY(fromYear < 2000))
      adjustedFromYear = fromYear + 2000;
    else
      adjustedFromYear = fromYear;

    if (toYear < 2000)
      adjustedToYear = toYear + 2000;
    else
      adjustedToYear = toYear;

    // if the day params are 0 it is for entire month
    if (UNLIKELY(fromDay == 0))
    {
      localFromDay = 1;
    }
    else
    {
      localFromDay = fromDay;
    }

    if (UNLIKELY(toDay == 0))
    {
      localToDay =
          endOfMonthDay(static_cast<uint16_t>(adjustedToYear), static_cast<uint16_t>(toMonth));
    }
    else
    {
      localToDay = toDay;
    }

    // make adjustment for leap year, this should only apply if year was 0 form data base
    if (!isLeapYear(adjustedFromYear))
    {
      if (fromMonth == 2)
      {
        if (UNLIKELY(localFromDay > 28))
        {
          localFromDay = 28;
        }
      }
    }

    // make adjustment for leap year
    if (!isLeapYear(adjustedToYear))
    {
      if (toMonth == 2)
      {
        if (UNLIKELY(localToDay > 28))
        {
          localToDay = 28;
        }
      }
    }

    DateTime fromDate(static_cast<uint16_t>(adjustedFromYear),
                      static_cast<uint16_t>(fromMonth),
                      static_cast<uint16_t>(localFromDay));
    DateTime toDate(static_cast<uint16_t>(adjustedToYear),
                    static_cast<uint16_t>(toMonth),
                    static_cast<uint16_t>(localToDay));
    // create a date object with no time from this date object
    DateTime thisDate(this->year(), this->month(), this->day());

    return (thisDate.isBetween(fromDate, toDate));
  }
}

bool
DateTime::isBetweenF(int fromYear, int fromMonth, int fromDay, int toYear, int toMonth, int toDay)
    const
{
  int32_t localFromDay = fromDay;
  int32_t localToDay = toDay;

  // if no year only compare month and day
  if (fromYear == 0 && toYear == 0)
  {
    const int32_t thisMonth = this->month();
    const int32_t thisDay = this->day();

    if ((fromMonth == toMonth) && (fromDay == toDay))
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
    else if (fromMonth < toMonth || (fromMonth == toMonth && (toDay == 0 || fromDay < toDay)))
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
    if (fromYear < 2000)
      adjustedFromYear = fromYear + 2000;
    else
      adjustedFromYear = fromYear;

    if (toYear < 2000)
      adjustedToYear = toYear + 2000;
    else
      adjustedToYear = toYear;

    // if the day params are 0 it is for entire month
    if (fromDay == 0)
    {
      localFromDay = 1;
    }
    else
    {
      localFromDay = fromDay;
    }

    if (toDay == 0)
    {
      localToDay =
          endOfMonthDay(static_cast<uint16_t>(adjustedToYear), static_cast<uint16_t>(toMonth));
    }
    else
    {
      localToDay = toDay;
    }

    // make adjustment for leap year, this should only apply if year was 0 form data base
    if (!isLeapYear(adjustedFromYear))
    {
      if (fromMonth == 2)
      {
        if (localFromDay > 28)
        {
          localFromDay = 28;
        }
      }
    }

    // make adjustment for leap year
    if (!isLeapYear(adjustedToYear))
    {
      if (toMonth == 2)
      {
        if (localToDay > 28)
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

std::string
DateTime::dateToString(DateFormat format, const char* separator) const
{
  if (!isValid())
    return NO_DATE_STRING;

  std::ostringstream ssDate;
  std::ostringstream ssYear;
  std::ostringstream ssMonth;
  std::ostringstream ssDay;

  ssYear << date().year();
  ssMonth << std::setw(2) << std::setfill('0') << ((unsigned short)date().month());
  ssDay << std::setw(2) << std::setfill('0') << date().day();

  uint32_t monthNum = date().month();

  switch (format)
  {
  case YYYYMMDD:
    if (UNLIKELY(separator == nullptr || *separator == '\0'))
    {
      ssDate << ssYear.str() << ssMonth.str() << ssDay.str();
    }
    else
    {
      ssDate << ssYear.str() << separator << ssMonth.str() << separator << ssDay.str();
    }
    break;

  case YYYYMmmDD:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssYear.str() << date().month().as_short_string() << ssDay.str();
    }
    else
    {
      ssDate << ssYear.str() << separator << date().month().as_short_string() << separator
             << ssDay.str();
    }
    break;

  case YYYYMMMDD:
    if (UNLIKELY(separator == nullptr || *separator == '\0'))
    {
      ssDate << ssYear.str() << MONTHS_UPPER_CASE[monthNum] << ssDay.str();
    }
    else
    {
      ssDate << ssYear.str() << separator << MONTHS_UPPER_CASE[monthNum] << separator
             << ssDay.str();
    }
    break;

  case MMDDYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssMonth.str() << ssDay.str() << ssYear.str();
    }
    else
    {
      ssDate << ssMonth.str() << separator << ssDay.str() << separator << ssYear.str();
    }
    break;

  case MMDDYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssMonth.str() << ssDay.str() << ssYear.str().substr(2, 2);
    }
    else
    {
      ssDate << ssMonth.str() << separator << ssDay.str() << separator << ssYear.str().substr(2, 2);
    }
    break;

  case MmmDDYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << date().month().as_short_string() << ssDay.str() << ssYear.str();
    }
    else
    {
      ssDate << date().month().as_short_string() << separator << ssDay.str() << separator
             << ssYear.str();
    }
    break;

  case MMMDDYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << MONTHS_UPPER_CASE[monthNum] << ssDay.str() << ssYear.str();
    }
    else
    {
      ssDate << MONTHS_UPPER_CASE[monthNum] << separator << ssDay.str() << separator
             << ssYear.str();
    }
    break;

  case MMMDDYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << MONTHS_UPPER_CASE[monthNum] << ssDay.str() << ssYear.str().substr(2, 2);
    }
    else
    {
      ssDate << MONTHS_UPPER_CASE[monthNum] << separator << ssDay.str() << separator
             << ssYear.str().substr(2, 2);
    }
    break;

  case DDMMYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssDay.str() << ssMonth.str() << ssYear.str();
    }
    else
    {
      ssDate << ssDay.str() << separator << ssMonth.str() << separator << ssYear.str();
    }
    break;

  case DDMmmYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssDay.str() << date().month().as_short_string() << ssYear.str();
    }
    else
    {
      ssDate << ssDay.str() << separator << date().month().as_short_string() << separator
             << ssYear.str();
    }
    break;

  case DDMMMYYYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssDay.str() << MONTHS_UPPER_CASE[monthNum] << ssYear.str();
    }
    else
    {
      ssDate << ssDay.str() << separator << MONTHS_UPPER_CASE[monthNum] << separator
             << ssYear.str();
    }
    break;

  case DDMMMYY:
    if (separator == nullptr || *separator == '\0')
    {
      ssDate << ssDay.str() << MONTHS_UPPER_CASE[monthNum] << ssYear.str().substr(2, 2);
    }
    else
    {
      ssDate << ssDay.str() << separator << MONTHS_UPPER_CASE[monthNum] << separator
             << ssYear.str().substr(2, 2);
    }
    break;

  case DDMMM:
    if (LIKELY(separator == nullptr || *separator == '\0'))
    {
      ssDate << ssDay.str() << MONTHS_UPPER_CASE[monthNum];
    }
    else
    {
      ssDate << ssDay.str() << separator << MONTHS_UPPER_CASE[monthNum];
    }
    break;

  case DDMMY:
    ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE[monthNum] << ssYear.str().substr(3, 1);
    break;

  case DDMM:
    ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE[monthNum];
    break;

  case DDMM_FD:
    ssDate << ssDay.str() << SHORT_MONTHS_UPPER_CASE_FARE_DISPLAY[monthNum];
    break;

  default:
    ssDate << boost::posix_time::to_simple_string(*this);
  }

  return (ssDate.str());
}

std::string
DateTime::timeToString(TimeFormat format, const char* separator) const
{
  if (UNLIKELY(!isValid()))
    return NO_DATE_STRING;

  std::ostringstream ssTime;
  std::ostringstream ssHours;
  std::ostringstream ssMinutes;

  uint32_t hrs = time_of_day().hours();

  ssMinutes << std::setw(2) << std::setfill('0') << time_of_day().minutes();

  switch (format)
  {
  case HHMMSS:
  {
    ssHours << std::setw(2) << std::setfill('0') << hrs;

    std::ostringstream ssSeconds;
    ssSeconds << std::setw(2) << std::setfill('0') << time_of_day().seconds();

    ssTime << ssHours.str() << separator << ssMinutes.str() << separator << ssSeconds.str();
    break;
  }

  case HHMM:
  {
    ssHours << std::setw(2) << std::setfill('0') << hrs;
    ssTime << ssHours.str() << separator << ssMinutes.str();
    break;
  }
  case HHMM_AMPM:
  {
    std::string ampm = "AM";

    if (hrs >= 12)
    {
      hrs = hrs - 12;
      ampm = "PM";
    }

    if (hrs == 0)
      hrs = 12;

    ssHours << hrs;

    ssTime << ssHours.str() << separator << ssMinutes.str() << ampm;
    break;
  }
  default:
  {
    ssHours << std::setw(2) << std::setfill('0') << hrs;

    std::ostringstream ssSeconds;
    ssSeconds << std::setw(2) << std::setfill('0') << time_of_day().seconds();

    ssTime << ssHours.str() << ':' << ssMinutes.str() << ':' << ssSeconds.str();
    break;
  }
  }

  return (ssTime.str());
}

bool
DateTime::isRangeInBetween(const int32_t& fromYear,
                           const int32_t& fromMonth,
                           const int32_t& fromDay,
                           const int32_t& toYear,
                           const int32_t& toMonth,
                           const int32_t& toDay,
                           const DateTime& latestDate) const
{
  int32_t localFromDay = fromDay;
  int32_t localToDay = toDay;

  // if no year only compare month and day
  if (fromYear == 0 && toYear == 0)
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
    else if (fromMonth == toMonth)
    {
      if (fromDay > toDay)
      {
        if (fromMonth > this->month())
          adjustedFromYear--;
        else if (fromMonth == this->month() && (toDay == 0 || toDay >= this->day()))
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
      if (fromMonth > this->month() && toMonth >= this->month())
      {
        adjustedFromYear--;
      }
      else
      {
        adjustedToYear++;
      }
    }

    // if the day params are 0 it is for entire month
    if (fromDay == 0)
    {
      localFromDay = 1;
    }
    else
    {
      localFromDay = fromDay;
    }

    if (toDay == 0)
    {
      localToDay =
          endOfMonthDay(static_cast<uint16_t>(adjustedToYear), static_cast<uint16_t>(toMonth));
    }
    else
    {
      localToDay = toDay;
    }

    // make adjustment for leap year
    if (!isLeapYear(adjustedFromYear))
    {
      if (fromMonth == 2)
      {
        if (localFromDay > 28)
        {
          localFromDay = 28;
        }
      }
    }

    // make adjustment for leap year
    if (!isLeapYear(adjustedToYear))
    {
      if (toMonth == 2)
      {
        if (localToDay > 28)
        {
          localToDay = 28;
        }
      }
    }

    DateTime fromDate(static_cast<uint16_t>(adjustedFromYear),
                      static_cast<uint16_t>(fromMonth),
                      static_cast<uint16_t>(localFromDay));
    DateTime toDate(static_cast<uint16_t>(adjustedToYear),
                    static_cast<uint16_t>(toMonth),
                    static_cast<uint16_t>(localToDay));
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
    if (fromYear < 2000)
      adjustedFromYear = fromYear + 2000;
    else
      adjustedFromYear = fromYear;

    if (toYear < 2000)
      adjustedToYear = toYear + 2000;
    else
      adjustedToYear = toYear;

    // if the day params are 0 it is for entire month
    if (fromDay == 0)
    {
      localFromDay = 1;
    }
    else
    {
      localFromDay = fromDay;
    }

    if (toDay == 0)
    {
      localToDay =
          endOfMonthDay(static_cast<uint16_t>(adjustedToYear), static_cast<uint16_t>(toMonth));
    }
    else
    {
      localToDay = toDay;
    }

    // make adjustment for leap year, this should only apply if year was 0 form data base
    if (!isLeapYear(adjustedFromYear))
    {
      if (fromMonth == 2)
      {
        if (localFromDay > 28)
        {
          localFromDay = 28;
        }
      }
    }

    // make adjustment for leap year
    if (!isLeapYear(adjustedToYear))
    {
      if (toMonth == 2)
      {
        if (localToDay > 28)
        {
          localToDay = 28;
        }
      }
    }

    DateTime fromDate(static_cast<uint16_t>(adjustedFromYear),
                      static_cast<uint16_t>(fromMonth),
                      static_cast<uint16_t>(localFromDay));
    DateTime toDate(static_cast<uint16_t>(adjustedToYear),
                    static_cast<uint16_t>(toMonth),
                    static_cast<uint16_t>(localToDay));
    // create a date object with no time from this date object
    DateTime thisDate(this->year(), this->month(), this->day());
    DateTime lastDate(latestDate.year(), latestDate.month(), latestDate.day());
    return (thisDate.isRangeInBetween(fromDate, toDate, lastDate));
  }
}

DateTime
DateTime::addDays(uint32_t numberOfDays) const
{
  boost::gregorian::day_iterator it(this->date());

  for (uint32_t i = 0; i < numberOfDays; ++i)
  {
    ++it;
  }
  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::nextDay() const
{
  boost::gregorian::day_iterator it(this->date());
  ++it;
  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::subtractDays(uint32_t numberOfDays) const
{
  boost::gregorian::day_iterator it(this->date());

  for (uint32_t i = 0; i < numberOfDays; ++i)
  {
    --it;
  }
  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::addWeeks(int32_t numberOfWeeks) const
{
  boost::gregorian::week_iterator it(this->date());

  for (int32_t i = 0; i < numberOfWeeks; ++i)
  {
    ++it;
  }

  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::addMonths(int32_t numberOfMonths) const
{
  boost::gregorian::month_iterator it(this->date());

  for (int32_t i = 0; i < numberOfMonths; ++i)
  {
    ++it;
  }

  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::subtractMonths(int32_t numberOfMonths) const
{
  boost::gregorian::month_iterator it(this->date());

  for (int32_t i = 0; i < numberOfMonths; ++i)
  {
    --it;
  }

  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

DateTime
DateTime::addYears(int32_t numberOfYears) const
{
  boost::gregorian::year_iterator it(this->date());

  for (int32_t i = 0; i < numberOfYears; ++i)
  {
    ++it;
  }

  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

void
DateTime::addMinutes(int numberOfMinutes)
{
  *this += Hours(numberOfMinutes / MINUTES_PER_HOUR) + Minutes(numberOfMinutes % MINUTES_PER_HOUR);
}

void
DateTime::setHistoricalIncludesTime()
{
  *this += Seconds(1);
}

DateTime
DateTime::getFutureDayOfWeek(const Weekdays aDayOfWeek, int32_t number) const
{
  boost::date_time::first_kday_after<boost::gregorian::date> firstKDayAfter(aDayOfWeek);
  boost::gregorian::date futureDate = firstKDayAfter.get_date(this->date());

  boost::gregorian::week_iterator it(futureDate);

  for (int32_t i = 1; i < number; ++i)
  {
    ++it;
  }

  return ptime(*it, boost::posix_time::time_duration(hours(), minutes(), seconds()));
}

bool
DateTime::isFutureDate()
{
  boost::gregorian::date currentDate(DateTime::localTime().date());
  boost::gregorian::date thisDate(date());

  if (thisDate.year() > currentDate.year())
    return true;
  else if (thisDate.year() < currentDate.year())
    return false;
  else if (thisDate.month() > currentDate.month())
    return true;
  else if (thisDate.month() < currentDate.month())
    return false;
  else if (thisDate.day() > currentDate.day())
    return true;
  else if (thisDate.day() < currentDate.day())
    return false;

  return false;
}

DateTime
DateTime::convertDate(const char* inDate)
{
  if (inDate == nullptr)
  {
    return emptyDate();
  }

  uint16_t year = 0;
  uint16_t month = 0;
  uint16_t day = 0;

  // Expect YYYY-MM-DD
  char dummy;
  int n = std::sscanf(inDate, "%4hd-%2hd-%2hd%c", &year, &month, &day, &dummy);
  if (n != 3)
    return emptyDate();

  try
  {
    return DateTime(year, month, day);
  }
  catch (const std::out_of_range&)
  {}

  return emptyDate();
}

}
