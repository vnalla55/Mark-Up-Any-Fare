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

#include "Common/Timestamp.h"

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include <iomanip>
#include <locale>
#include <stdexcept>
#include <sstream>

using namespace std;

namespace tax
{

namespace type
{

const int16_t Time::_blankTime = 0x7FFF;

Time::Time(int16_t hour, int16_t minute) { init(hour, minute); }

Time::Time(const boost::posix_time::time_duration& boostTime)
{
  if (UNLIKELY(boostTime.is_special()))
  {
    _mins = blank_time()._mins;
  }
  else
  {
    init(static_cast<int16_t>(boostTime.hours()), static_cast<int16_t>(boostTime.minutes()));
  }
}

void
Time::init(int16_t hour, int16_t minute)
{
  if (hour < 0 || hour >= 24)
  {
    throw out_of_range("Hour out of 0..23 range!");
  }
  if (minute < 0 || minute >= 60)
  {
    throw out_of_range("Minute out of 0..59 range!");
  }
  _mins = int16_t(60 * hour + minute);
}

int16_t
Time::
operator-(const Time& other) const
{
  return int16_t(_mins - other._mins);
}

Time
Time::
operator+(int16_t minutes) const
{
  const int16_t minsPerDay = int16_t(24 * 60);
  int16_t newMins = int16_t(_mins + minutes);
  if (newMins < 0)
  {
    newMins = int16_t(minsPerDay - (minsPerDay - newMins) % minsPerDay);
  }
  else
  {
    newMins = int16_t(newMins % minsPerDay);
  }
  return Time(newMins / 60, newMins % 60);
}

Time
Time::
operator-(int16_t minutes) const
{
  return *this + int16_t(-minutes);
}

std::string
Time::str() const
{
  ostringstream stream;

  stream << setfill('0') << setw(2) << hour() << ':' << setw(2) << min();
  return stream.str();
}

std::ostream& operator<<(std::ostream& stream, Time time)
{
  char fill = stream.fill();
  stream << setfill('0') << setw(2) << time.hour() << ':' << setw(2) << time.min() << setfill(fill);
  return stream;
}

std::istream& operator>>(std::istream& stream, Time& time)
{
  int16_t hour, min;
  char c(' ');
  stream >> hour;
  stream >> c;
  if (c != ':')
  {
    throw out_of_range("Time separator is not a colon!");
  }
  stream >> min;
  time = Time(hour, min);
  return stream;
}

const int Date::monthLengths[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const string Date::monthNames[] = { "",    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const Date Date::_invalidDate(0xFFFFFFFEu);
const Date Date::_negInfinity(0);
const Date Date::_posInfinity(0xFFFFFFFFu);
const Date Date::_blankDate(0xFFFFFFFDu);

bool
Date::is_between(const Date& start, const Date& end) const
{
  if (start.is_seasonal() && end.is_seasonal())
  {
    unsigned short day = _daynum & 0xFFFF;
    if (start._daynum <= end._daynum)
    {
      return start._daynum <= day && day <= end._daynum;
    }
    else
    {
      return day <= end._daynum || start._daynum <= day;
    }
  }
  else if (start.is_seasonal() || end.is_seasonal())
  {
    throw logic_error("Only one date is seasonal in in_between!");
  }

  return (start.is_blank_date() || (start <= *this)) &&
         (end.is_blank_date() || (*this <= end));
}

Date
Date::make_date(int16_t year, int16_t month, int16_t day)
{
  try
  {
    return Date(year, month, day);
  }
  catch (const out_of_range&)
  {
    return Date::invalid_date();
  }
}

Date::Date(int16_t month, int16_t day) : _daynum((month << 8) + day)
{
  if (month < 1 || month > 12)
  {
    throw out_of_range("Seasonal date: month out of range");
  }
  if (day < 1 || day > monthLengths[month])
  {
    throw out_of_range("Seasonal date: day out of range");
  }
}

Date::Date(int16_t year, int16_t month, int16_t day)
{
  init(year, month, day);
  if (month < 1 || month > 12)
  {
    throw out_of_range("Month out of range");
  }
  if (day < 1 || day > monthLengths[month])
  {
    throw out_of_range("Day out of range");
  }
  if (day == 29 && month == 2 && year % 4 != 0)
  {
    throw out_of_range("February 29th on a non-leap year");
  }
  if (year <= 1799)
  {
    _daynum = 0; // negative infinity
  }
  else if (year >= 9999)
  {
    _daynum = 0xFFFFFFFF; // positive infinity
  }
}

Date::Date(const boost::gregorian::date& boostDate)
{
  if (UNLIKELY(boostDate.is_not_a_date()))
  {
    _daynum = 0xFFFFFFFDu; // blank date
  }
  else if (UNLIKELY(boostDate.is_pos_infinity()))
  {
    _daynum = 0xFFFFFFFF; // positive infinity
  }
  else if (UNLIKELY(boostDate.is_neg_infinity()))
  {
    _daynum = 0; // negative infinity
  }
  else
  {
    init(boostDate);
  }
}

void
Date::init(int16_t year, int16_t month, int16_t day)
{
  _daynum = (year << 16) + (month << 8) + day;
  _boostDate = boost::gregorian::date(year, month, day);
}

void
Date::init(const boost::gregorian::date& boostDate)
{
  _daynum = (boostDate.year() << 16) + (boostDate.month() << 8) + boostDate.day();
  _boostDate = boostDate;
}


std::string
Date::format(const std::string& format) const
{
  boost::gregorian::date_facet* facet =
      new boost::gregorian::date_facet(format.c_str());

  std::ostringstream ss;
  ss.imbue(std::locale(std::locale(), facet));
  ss << _boostDate;

  return ss.str();
}

bool
Date::
operator<=(const Date& other) const
{
  if (UNLIKELY(_daynum == 0xFFFFFFFE || other._daynum == 0xFFFFFFFE))
  {
    throw std::domain_error("Comparing an invalid date!");
  }
  if (UNLIKELY(_daynum == 0xFFFFFFFD || other._daynum == 0xFFFFFFFD))
  {
    throw std::domain_error("Comparing a blank date!");
  }
  if (UNLIKELY(is_seasonal() || other.is_seasonal()))
  {
    return (_daynum & 0xFFFF) <= (other._daynum & 0xFFFF);
  }
  else
  {
    return _daynum <= other._daynum;
  }
}

bool
Date::
operator<(const Date& other) const
{
  if (_daynum == 0xFFFFFFFE || other._daynum == 0xFFFFFFFE)
  {
    throw std::domain_error("Comparing an invalid date!");
  }
  if (_daynum == 0xFFFFFFFD || other._daynum == 0xFFFFFFFD)
  {
    throw std::domain_error("Comparing a blank date!");
  }
  if (is_seasonal() || other.is_seasonal())
  {
    return (_daynum & 0xFFFF) < (other._daynum & 0xFFFF);
  }
  else
  {
    return _daynum < other._daynum;
  }
}

bool
Date::
operator==(const Date& other) const
{
  if (UNLIKELY(is_seasonal() || other.is_seasonal()))
  {
    return (_daynum & 0xFFFF) == (other._daynum & 0xFFFF);
  }
  else
  {
    return _daynum == other._daynum;
  }
}

bool
Date::
operator!=(const Date& other) const
{
  return !(*this == other);
}

int32_t
Date::
operator-(const Date& other) const
{
  return static_cast<int32_t>((_boostDate - other._boostDate).days());
}

std::ostream& operator<<(std::ostream& stream, Date date)
{
  if (date.is_neg_infinity())
  {
    stream << "1799-12-31";
    return stream;
  }
  else if (date.is_pos_infinity())
  {
    stream << "9999-12-31";
    return stream;
  }
  else if (date.is_invalid())
  {
    stream << "???\?-?\?-??";
    return stream;
  }
  else if (date.is_blank_date())
  {
    stream << "    -  -  ";
    return stream;
  }
  char f = stream.fill();
  stream << setfill('0') << setw(4) << right;
  if (date.year() == 0)
  {
    stream << "****";
  }
  else
  {
    stream << date.year();
  }
  stream << '-' << setw(2) << date.month() << '-' << setw(2) << date.day();
  stream << setfill(f);
  return stream;
}

std::istream& operator>>(std::istream& stream, Date& date)
{
  int16_t year, month, day;
  char separator;
  stream >> year;
  stream >> separator;
  stream >> month;
  stream >> separator;
  stream >> day;
  if (year == 0)
  {
    date = Date(month, day);
  }
  else if (year <= 1799)
  {
    date = Date::neg_infinity();
  }
  else if (year >= 9999)
  {
    date = Date::pos_infinity();
  }
  else
  {
    date = Date(year, month, day);
  }
  return stream;
}

Date
Date::advance(int16_t offset) const
{
  if (is_special())
  {
    return *this;
  }

  if (UNLIKELY(is_seasonal()))
  {
    throw logic_error("Seasonal dates not supported in advance()");
  }

  boost::gregorian::date date = _boostDate + boost::gregorian::date_duration(offset);
  return Date(date);
}

Timestamp::Timestamp(const Date& date, const Time& time) : _date(date), _time(time)
{
  if (_date.is_seasonal())
  {
    throw out_of_range("Seasonal dates not supported in a timestamp!");
  }
  if (_date.is_blank_date() && _time.is_blank())
  {
    return;
  }
  if (_date.is_neg_infinity())
  {
    _time = Time(0, 0);
    return;
  }
  if (_date.is_pos_infinity())
  {
    _time = Time(23, 59);
    return;
  }
  if (_date.is_invalid())
  {
    throw out_of_range("Invalid date used in a timestamp!");
  }
  if (_date.is_special())
  {
    throw out_of_range("Special dates not supported in a timestamp!");
  }
  if (_date.is_blank_date() || _time.is_blank())
  {
    throw out_of_range("Either both date and time should be blank, or none!");
  }
}

std::istream& operator>>(std::istream& str, Timestamp& ts)
{
  char separator;
  str >> ts.date();
  str >> separator;
  if (separator != ' ')
  {
    throw out_of_range("Invalid timestamp separator, space expected");
  }
  str >> ts.time();
  return str;
}

std::ostream& operator<<(std::ostream& str, Timestamp ts)
{
  str << ts.date();
  str << ' ';
  str << ts.time();
  return str;
}

const Timestamp Timestamp::_emptyTimestamp = Timestamp(Date(1980, 1, 1), Time(0, 0));

} // namespace type

} // namespace tax
