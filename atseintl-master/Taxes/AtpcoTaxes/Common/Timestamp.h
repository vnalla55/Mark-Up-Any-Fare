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

#pragma once

#include <iostream>
#include <stdint.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Util/BranchPrediction.h"

namespace tax
{

namespace type
{

class Date
{
  // The date is coded in an unsigned int (32 bits) as follows:
  // Bits 0-7 - day of month, 1-31
  // Bits 8-15 - month, 1-12
  // Bits 16-31 - year, 0 means seasonal date.
  // Special cases:
  // Value 0x00000000 means negative infinity,
  // Value 0xFFFFFFFF means positive infinity,
  // Value 0xFFFFFFFE means invalid date.
  // Value 0xFFFFFFFD means blank (i.e. unspecified) date.
public:
  Date()
  {
    _daynum = 0xFFFFFFFD;
  };
  Date(int16_t year, int16_t month, int16_t day);
  explicit Date(const boost::gregorian::date& boostDate);
  Date(int16_t month, int16_t day);
  Date(const Date& other) : _daynum(other._daynum), _boostDate(other._boostDate) {};
  void init(int16_t year, int16_t month, int16_t day);
  void init(const boost::gregorian::date& boostDate);

  // parameter 'format' is a boost::date_facet format
  std::string format(const std::string& format) const;

  const Date& operator=(const Date& other)
  {
    _daynum = other._daynum;
    _boostDate = other._boostDate;
    return *this;
  };
  Date advance(int16_t days) const;
  int32_t operator-(const Date& other) const;

  bool operator<(const Date& other) const;
  bool operator<=(const Date& other) const;
  bool operator>(const Date& other) const
  {
    return other < *this;
  };
  bool operator>=(const Date& other) const
  {
    return other <= *this;
  };
  bool operator==(const Date& other) const;
  bool operator!=(const Date& other) const;
  bool is_seasonal() const
  {
    return (_daynum & 0xFFFF0000u) == 0 && _daynum > 0;
  };
  bool is_between(const Date&, const Date&) const;
  bool is_special() const { return _daynum - 1u >= 0xFFFF0000u; }

  uint16_t year() const { return uint16_t((_daynum >> 16) & 0xFFFF); }
  uint16_t month() const { return uint16_t(((_daynum >> 8) & 0xFF)); }
  uint16_t day() const { return uint16_t(_daynum & 0xFF); }

  bool is_invalid() const { return _daynum == _invalidDate._daynum; }
  bool is_neg_infinity() const { return _daynum == _negInfinity._daynum; }
  bool is_pos_infinity() const { return _daynum == _posInfinity._daynum; }
  bool is_blank_date() const { return _daynum == _blankDate._daynum; }
  bool is_correct() const
  {
    return !is_invalid() && !is_neg_infinity() && !is_pos_infinity() && !is_blank_date();
  }
  static const Date& invalid_date() { return _invalidDate; }
  static const Date& neg_infinity() { return _negInfinity; }
  static const Date& pos_infinity() { return _posInfinity; }
  static const Date& blank_date() { return _blankDate; }
  static Date make_date(int16_t year, int16_t month, int16_t day);

private:
  uint32_t _daynum;
  boost::gregorian::date _boostDate;
  static const int monthLengths[];
  static const std::string monthNames[];
  static const Date _invalidDate;
  static const Date _negInfinity;
  static const Date _posInfinity;
  static const Date _blankDate;
  explicit Date(uint32_t daynum) { _daynum = daynum; }
};

std::ostream& operator<<(std::ostream& stream, Date date);
std::istream& operator>>(std::istream& stream, Date& date);

class Time
{
public:
  Time() : _mins(0) {};
  Time(int16_t hour, int16_t minute);
  Time(const boost::posix_time::time_duration& boostTime);
  Time(const Time& other)
  {
    _mins = other._mins;
  };

  void init(int16_t hour, int16_t minute);

  int16_t operator-(const Time& other) const;
  Time operator+(int16_t minutes) const;
  Time operator-(int16_t minutes) const;
  std::string str() const;
  int16_t hour() const { return int16_t(_mins / 60); }
  int16_t min() const { return int16_t(_mins % 60); }
  const Time& operator=(const Time& other)
  {
    _mins = other._mins;
    return *this;
  }
  bool operator==(const Time& other) const { return _mins == other._mins; }
  bool operator!=(const Time& other) const { return _mins != other._mins; }
  bool operator<(const Time& other) const { return _mins < other._mins; }
  bool operator>(const Time& other) const { return _mins > other._mins; }
  bool operator<=(const Time& other) const { return _mins <= other._mins; }
  bool operator>=(const Time& other) const { return _mins >= other._mins; }

  bool is_blank() { return _mins == _blankTime; }
  static Time blank_time() { return Time(_blankTime); }

private:
  int16_t _mins;
  static const int16_t _blankTime;
  explicit Time(int16_t mins) { _mins = mins; }
};

std::ostream& operator<<(std::ostream& stream, Time time);
std::istream& operator>>(std::istream& stream, Time& time);

class Timestamp
{
public:
  Timestamp() : _date(), _time() {};
  Timestamp(const Date& date, const Time& time = Time());
  Timestamp(const Timestamp& other) : _date(other._date), _time(other._time) {};
  Time& time() { return _time; }
  Date& date() { return _date; }
  Time time() const { return _time; }
  Date date() const { return _date; }
  uint16_t year() const { return _date.year(); }
  uint16_t month() const { return _date.month(); }
  uint16_t day() const { return _date.day(); }
  int16_t hour() const { return _time.hour(); }
  int16_t min() const { return _time.min(); }
  int32_t operator-(const Timestamp& other)
  {
    return 24 * 60 * (_date - other._date) + (_time - other._time);
  }
  const Timestamp& operator=(const Timestamp& other)
  {
    _date = other._date;
    _time = other._time;
    return *this;
  }
  bool operator==(const Timestamp& other) const
  {
    return _date == other._date && _time == other._time;
  }
  bool operator!=(const Timestamp& other) const
  {
    return _date != other._date || _time != other._time;
  }
  bool operator<(const Timestamp& other) const
  {
    if (_date == other._date)
      return _time < other._time;
    return _date < other._date;
  }
  bool operator>(const Timestamp& other) const
  {
    if (_date == other._date)
      return _time > other._time;
    return _date > other._date;
  }
  bool operator<=(const Timestamp& other) const
  {
    if (_date == other._date)
      return _time <= other._time;
    return _date <= other._date;
  }
  bool operator>=(const Timestamp& other) const
  {
    if (_date == other._date)
      return _time >= other._time;
    return _date >= other._date;
  }

  static const Timestamp& emptyTimestamp() { return _emptyTimestamp; }

private:
  Date _date;
  Time _time;
  static const Timestamp _emptyTimestamp;
};

std::ostream& operator<<(std::ostream& stream, Timestamp ts);
std::istream& operator>>(std::istream& stream, Timestamp& ts);
}
}
