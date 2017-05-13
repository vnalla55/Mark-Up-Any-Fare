/*----------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"

#include <functional>

namespace tse
{
template <template <class> class Predicate, typename T>
class IsNot : public Predicate<T>
{
  using base = Predicate<T>;

public:
  using Predicate<T>::Predicate;
  bool operator()(const T* rec) const { return !base::operator()(rec); }
};

struct CommonContext
{
  CommonContext(const DateTime& startDateWithTime,
                const DateTime& endDateWithTime,
                const DateTime& ticketDateWithTime)
    : _startDateWithTime(startDateWithTime),
      _endDateWithTime(endDateWithTime),
      _ticketDateWithTime(ticketDateWithTime),
      _startDateWithoutTime(startDateWithTime.date()),
      _endDateWithoutTime((_startDateWithTime == _endDateWithTime) ? _startDateWithoutTime
                                                                   : endDateWithTime.date()),
      _ticketDateWithoutTime(ticketDateWithTime.date())
  {
  }

  CommonContext(const DateTime& dateWithTime, const DateTime& ticketDateWithTime)
    : CommonContext(dateWithTime, dateWithTime, ticketDateWithTime)
  {
  }

  CommonContext() = default;

  const DateTime _startDateWithTime;
  const DateTime _endDateWithTime;
  const DateTime _ticketDateWithTime;

  const boost::gregorian::date _startDateWithoutTime;
  const boost::gregorian::date _endDateWithoutTime;
  const boost::gregorian::date _ticketDateWithoutTime;
};

template <typename T>
class IsCurrentG
{
public:
  IsCurrentG(const DateTime& ticketDate = DateTime::emptyDate()) : _ticketDate(ticketDate)
  {
    if (UNLIKELY(_ticketDate.isEmptyDate()))
    {
      _ticketDate = DateTime::localTime();
    }
  }

  bool operator()(const T* rec) const
  {
    return _ticketDate <= rec->expireDate();
  }

private:
  DateTime _ticketDate;
};

template <typename T>
using IsNotCurrentG = IsNot<IsCurrentG, T>;

template <typename T>
class IsCurrentH
{
public:
  IsCurrentH(const DateTime& ticketDate)
    : _ticketDate(ticketDate), _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }
  bool operator()(const T* rec) const
  {
    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return false;
      if (rec->createDate() > _ticketDate)
        return false;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }
    return true;
  }

private:
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;
  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
using IsNotCurrentH = IsNot<IsCurrentH, T>;

template <typename T>
class IsCurrentCreateDateOnly
{
public:
  IsCurrentCreateDateOnly(const DateTime& ticketDate)
    : _ticketDate(ticketDate.date()), _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }
  bool operator()(const T* rec) const
  {
    if (_ticketDate.historicalIncludesTime())
      return !(rec->createDate() > _ticketDate);

    return !(rec->createDate().date() > _ticketDateNoTime);
  }

private:
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;
  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
class IsNotCurrentCreateDateOnly
{
public:
  IsNotCurrentCreateDateOnly(const DateTime& ticketDate)
    : _ticketDate(ticketDate), _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }
  bool operator()(const T* rec) const
  {
    if (_ticketDate.historicalIncludesTime())
      return rec->createDate() > _ticketDate;

    return rec->createDate().date() > _ticketDateNoTime;
  }

private:
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;
  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
class IsEffectiveH
{
public:
  IsEffectiveH(const DateTime& startDate, const DateTime& endDate, const DateTime& ticketDate)
    : _startDate(startDate.date()),
      _endDate(endDate.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsEffectiveH(const DateTime& date, const DateTime& ticketDate)
    : _startDate(date.date()),
      _endDate(date.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsEffectiveH(const CommonContext& context)
    : _startDate(context._startDateWithoutTime),
      _endDate(context._endDateWithoutTime),
      _ticketDate(context._ticketDateWithTime),
      _ticketDateNoTime(context._ticketDateWithoutTime)
  {
    init(_ticketDate);
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return false;
    if (_startDate > rec->discDate().date())
      return false;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return false;
      if (rec->createDate() > _ticketDate)
        return false;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }

    return true;
  }

private:
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
using IsNotEffectiveH = IsNot<IsEffectiveH, T>;

// ---
// temporary Historical filters
//    These compare the ticket date with the  effective date
// ---

template <typename T>
class IsEffectiveHist
{
public:
  IsEffectiveHist(const DateTime& startDate, const DateTime& endDate, const DateTime& ticketDate)
    : _startDate(startDate.date()),
      _endDate(endDate.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsEffectiveHist(const DateTime& date, const DateTime& ticketDate)
    : _startDate(date.date()),
      _endDate(date.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return false;
    if (_startDate > rec->discDate().date())
      return false;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return false;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return false;
    }

    if (rec->createDate().date() > _ticketDateNoTime && rec->effDate().date() > _ticketDateNoTime)
      return false;

    return true;
  }

private:
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
class IsNotEffectiveHist
{
public:
  IsNotEffectiveHist(const DateTime& startDate, const DateTime& endDate, const DateTime& ticketDate)
    : _startDate(startDate.date()),
      _endDate(endDate.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsNotEffectiveHist(const DateTime& date, const DateTime& ticketDate)
    : _startDate(date.date()),
      _endDate(date.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsNotEffectiveHist(CommonContext context)
    : _startDate(context._startDateWithoutTime),
      _endDate(context._endDateWithoutTime),
      _ticketDate(context._ticketDateWithTime),
      _ticketDateNoTime(context._ticketDateWithoutTime)
  {
    init(_ticketDate);
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return true;

    if ((_startDate > rec->discDate().date()) && !rec->discDate().isEmptyDate())
      return true;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return true;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return true;
    }

    if (rec->createDate().date() > _ticketDateNoTime && rec->effDate().date() > _ticketDateNoTime)
      return true;

    return false;
  }

private:
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

// DVD
template <typename T>
class IsNotEffectiveCatHist
{
public:
  IsNotEffectiveCatHist(const DateTime& startDate,
                        const DateTime& endDate,
                        const DateTime& ticketDate)
    : _startDate(startDate.date()),
      _endDate(endDate.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsNotEffectiveCatHist(const DateTime& date, const DateTime& ticketDate)
    : _startDate(date.date()),
      _endDate(date.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return true;
    if (_startDate > rec->discDate().date())
      return true;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return true;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return true;
    }

    return false;
  }

private:
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

// ------------------------------
// Temporary general data filters
// ------------------------------

template <typename T>
class IsEffectiveG
{
public:
  IsEffectiveG(const DateTime& startDate, const DateTime& endDate, const DateTime& ticketDate)
    : _startDateTime(startDate), _endDateTime(endDate), _ticketDate(ticketDate)
  {
    init();
  }

  IsEffectiveG(const DateTime& date, const DateTime& ticketDate)
    : _startDateTime(date), _endDateTime(date), _ticketDate(ticketDate)
  {
    init();
  }

  IsEffectiveG(const DateTime& date) : _startDateTime(date), _endDateTime(date), _ticketDate(date)
  {
    init();
  }

  IsEffectiveG(const CommonContext context)
    : _startDateTime(context._startDateWithTime),
      _endDateTime(context._endDateWithTime),
      _ticketDate(context._ticketDateWithTime),
      _startDate(context._startDateWithoutTime),
      _endDate(_endDateTime.isEmptyDate() ? _startDate : context._endDateWithoutTime)
  {
    if (_ticketDate.isEmptyDate())
      _ticketDate = DateTime::localTime();
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return false;
    if (_startDate > rec->discDate())
      return false;
    if (_ticketDate > rec->expireDate())
      return false;
    if (_startDateTime > rec->expireDate())
      return false;
    return true;
  }

private:
  const DateTime& _startDateTime;
  const DateTime& _endDateTime;
  DateTime _ticketDate;
  DateTime _startDate;
  DateTime _endDate;

  void init()
  {
    _startDate = _startDateTime.date();
    _endDate = _endDateTime.isEmptyDate() ? _startDate : _endDateTime.date();
    if (_ticketDate.isEmptyDate())
      _ticketDate = DateTime::localTime();
  }
};

template <typename T>
using IsNotEffectiveG = IsNot<IsEffectiveG, T>;

// ------------------------------------
// FareFocus filter for 'effective_now'
// ------------------------------------
template <typename T>
class IsEffectiveNow
{
public:
  IsEffectiveNow(const DateTime& adjustedTicketDate, const DateTime& ticketDate)
    : _adjustedTicketDate(adjustedTicketDate),
      _ticketDate(ticketDate),
      _startDateTime(ticketDate),
      _endDateTime(ticketDate)
  {
    init();
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() == rec->activationDateTime())
    { // effective now
      if (rec->effDate() > _adjustedTicketDate)
        return false;
    }
    else
    {
      if (rec->effDate() > _endDate)
        return false;
    }
    if (_startDate > rec->discDate())
      return false;
    if (rec->expireDate() == rec->lastModDate())
    { // expired now
      if (_adjustedTicketDate > rec->expireDate())
        return false;
    }
    else
    {
      if (_ticketDate > rec->expireDate())
        return false;
      if (_startDateTime > rec->expireDate())
        return false;
    }
    return true;
  }

private:
  const DateTime& _adjustedTicketDate;
  DateTime _ticketDate;
  const DateTime& _startDateTime;
  const DateTime& _endDateTime;
  DateTime _startDate;
  DateTime _endDate;

  void init()
  {
    _startDate = _startDateTime.date();
    _endDate = _endDateTime.isEmptyDate() ? _startDate : _endDateTime.date();
    if (_ticketDate.isEmptyDate())
      _ticketDate = DateTime::localTime();
  }
};

template <template <class> class IsEffectiveT, class T>
class IsEffectiveNotInhibit
{
public:
  IsEffectiveNotInhibit(const DateTime& date, const DateTime& ticketDate)
    : _isEffective(date, ticketDate)
  {
  }

  bool operator()(const T* rec) const { return NotInhibit(rec) && _isEffective(rec); }

private:
  IsEffectiveT<T> _isEffective;
};
}
