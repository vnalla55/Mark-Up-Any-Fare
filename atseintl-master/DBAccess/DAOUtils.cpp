//----------------------------------------------------------------------------
//
//              File:           DAOUtils.cpp
//              Description:    DAOUtils
//              Created:        08/10/2007
//              Authors:        Tony Lam
//
//              Updates:
//
//         2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//
//        ----------------------------------------------------------------------------

#include "DBAccess/DAOUtils.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Global.h"
#include "Common/Logger.h"

#include <boost/thread/tss.hpp>

namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DAOUtils"));

int
initializeDateRangeCacheSize()
{
  int size(0), configSize(0);

  if (!tse::Global::config().getValue("DATE_RANGE_CACHE_SIZE", configSize, "TSE_SERVER"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, "DATE_RANGE_CACHE_SIZE", "TSE_SERVER");
  }

  if (configSize > 0)
  {
    size = configSize;
  }

  return size;
}

struct DateRangeCacheEntry
{
  DateRangeCacheEntry() : _ticketDateInt(0), _cacheBy(tse::DAOUtils::ERROR), _init(false) {}
  boost::uint64_t _ticketDateInt;
  tse::DAOUtils::CacheBy _cacheBy;
  tse::DateTime _startDate;
  tse::DateTime _endDate;
  bool _init;
};
struct DateRangeCache
{
  explicit DateRangeCache(size_t dateRangeCacheSize) : _array(dateRangeCacheSize) {}
  ~DateRangeCache()
  {
    // std::cerr << __FUNCTION__ << std::endl;
  }
  std::vector<DateRangeCacheEntry> _array;
};
boost::thread_specific_ptr<DateRangeCache> _dateRangeCache;
}
namespace tse
{

void
DAOUtils::getDateRange(DateTime ticketDate,
                       DateTime& startDate,
                       DateTime& endDate,
                       CacheBy cacheBy)
{
  static size_t dateRangeCacheSize(initializeDateRangeCacheSize());
  DateRangeCacheEntry* entry(nullptr);
  boost::uint64_t ticketDateInt(0);
  if (UNLIKELY(dateRangeCacheSize > 0))
  {
    if (!_dateRangeCache.get())
    {
      // std::cerr << "new cache" << std::endl;
      _dateRangeCache.reset(new DateRangeCache(dateRangeCacheSize));
    }
    ticketDateInt = ticketDate.get64BitRep();
    size_t index((ticketDateInt - cacheBy) % dateRangeCacheSize);
    entry = &(_dateRangeCache->_array[index]);
    if (entry->_init && cacheBy == entry->_cacheBy && ticketDateInt == entry->_ticketDateInt)
    {
      // std::cerr << "found:" << entry->_startDate << ' ' << entry->_endDate << std::endl;
      startDate = entry->_startDate;
      endDate = entry->_endDate;
      return;
    }
  }
  // std::cerr << "not found" << std::endl;
  Year year = ticketDate.year();
  Month month = ticketDate.month();
  Day day = ticketDate.day();

  switch (cacheBy)
  {
  case DAILY: // create date range by daily
    startDate = DateTime(year, month, day);
    endDate = startDate;
    break;
  case WEEKLY: // create date range by weekly
  {
    uint32_t dow = ticketDate.date().day_of_week().as_number();
    startDate = DateTime(year, month, day).subtractDays(dow);
    endDate = startDate.addDays(6);
    break;
  }
  case HALFMONTHLY: // create date range by Monthly
    day = day < 16 ? 1 : 16;
    startDate = DateTime(year, month, day);
    if (day == 1)
    {
      day = 15;
    }
    else
    {
      day = ticketDate.endOfMonthDay(year, month);
      if (UNLIKELY(month == 2 && !ticketDate.isLeapYear(year) && day > 28))
        day = 28;
    }
    endDate = DateTime(year, month, day);
    break;
  case MONTHLY: // create date range by Monthly
    day = 1;
    startDate = DateTime(year, month, day);
    day = ticketDate.endOfMonthDay(year, month);
    if (month == 2 && !ticketDate.isLeapYear(year) && day > 28)
      day = 28;
    endDate = DateTime(year, month, day);
    break;
  case TWOMONTHS: // create a two month date range
    day = 1;
    if ((month & 0x01) == 0)
      month = month - 1;
    startDate = DateTime(year, month, day);
    endDate = startDate.addMonths(2).subtractDays(1);
    break;
  case THREEMONTHS: // create a three month date range
    day = 1;
    month = (((int)(month + 2) / 3) * 3) - 2;
    startDate = DateTime(year, month, day);
    endDate = startDate.addMonths(3).subtractDays(1);
    break;
  case FOURMONTHS: // create a four month date range
    day = 1;
    month = (((int)(month + 3) / 4) * 4) - 3;
    startDate = DateTime(year, month, day);
    endDate = startDate.addMonths(4).subtractDays(1);
    break;
  case SIXMONTHS: // create a six month date range
    day = 1;
    month = (((int)(month + 5) / 6) * 6) - 5;
    startDate = DateTime(year, month, day);
    endDate = startDate.addMonths(6).subtractDays(1);
    break;
  case YEARLY: // create date range by yearly
    month = 1;
    day = 1;
    startDate = DateTime(year, month, day);
    month = 12;
    day = 31;
    endDate = DateTime(year, month, day);
    break;
  case NODATES:
    startDate = Global::startTime().subtractMonths(24);
    endDate = Global::startTime().addYears(1);
    break;
  default:
    LOG4CXX_ERROR(_logger, "Invalid date range in DAOUtils");
    startDate = Global::startTime();
    endDate = startDate.addYears(5);
  }
  endDate = endDate + tse::Hours(23) + tse::Minutes(59) + tse::Seconds(59);
  if (UNLIKELY(entry))
  {
    entry->_init = true;
    entry->_ticketDateInt = ticketDateInt;
    entry->_cacheBy = cacheBy;
    entry->_endDate = endDate;
    entry->_startDate = startDate;
  }
}

DateTime
DAOUtils::nextCacheDate(DateTime startDate, CacheBy cacheBy)
{
  switch (cacheBy)
  {
  case NODATES:
  {
    return startDate.addYears(5);
  }
  case DAILY: // add a day
  {
    return startDate.addDays(1);
  }
  case WEEKLY: // add a week
  {
    return startDate.addWeeks(1);
  }
  case HALFMONTHLY: // half of a month
  {
    Year year = startDate.year();
    Month month = startDate.month();
    if (startDate.day() < 16)
    {
      return DateTime(year, month, 16);
    }
    else
    {
      DateTime newDate = DateTime(year, month, 1);
      return newDate.addMonths(1);
    }
  }
  case MONTHLY: // add a month
  {
    return startDate.addMonths(1);
  }
  case TWOMONTHS: // add two months
  {
    return startDate.addMonths(2);
  }
  case THREEMONTHS: // add three months
  {
    return startDate.addMonths(3);
  }
  case FOURMONTHS: // add four months
  {
    return startDate.addMonths(4);
  }
  case SIXMONTHS: // add six months
  {
    return startDate.addMonths(6);
  }
  case YEARLY: // add a year
  {
    return startDate.addYears(1);
  }
  default:
    LOG4CXX_ERROR(_logger, "Invalid date range in nextCacheDate");
    return startDate.addYears(5);
  }
}

DateTime
DAOUtils::firstCacheDate(DateTime& firstCacheDate, time_t& nextCacheDate)
{
  time_t now;
  time(&now);
  if (LIKELY(now >= nextCacheDate))
  {
    tm ltime;
    localtime_r(&now, &ltime);

    Year year = 1900 + ltime.tm_year;
    Month month = 1 + ltime.tm_mon;
    Day day = ltime.tm_mday;

    ltime.tm_mday++;
    ltime.tm_hour = 0;
    ltime.tm_min = 0;
    ltime.tm_sec = 0;
    ltime.tm_isdst = -1;
    nextCacheDate = mktime(&ltime);
    LOG4CXX_DEBUG(_logger,
                  "Next cache date is " << 1 + ltime.tm_mon << '/' << ltime.tm_mday << '/'
                                        << 1900 + ltime.tm_year);

    year = year - 2;
    if (month == 2 && day > 28)
      day = 28;

    // Minus one additional day to cover all possible time zones
    firstCacheDate = DateTime(year, month, day).subtractDays(1);
    LOG4CXX_DEBUG(_logger, "Earliest historical cache date is " << firstCacheDate);
  }
  return firstCacheDate;
}

} // End of Name space tse
