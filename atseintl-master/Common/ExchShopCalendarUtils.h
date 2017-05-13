//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once
#include "Common/DateTime.h"
#include "DataModel/PricingTrx.h"

#include <utility>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace tse
{
struct Cat31Info;
class DataHandle;
class Itin;
class RexPricingTrx;
class RexBaseTrx;

namespace ExchShopCalendar
{
using DatePeriod = boost::posix_time::time_period;

static constexpr int32_t FIRST_OND = 0;
static constexpr int32_t INVALID_OND_INDEX = -1;

struct DateRange
{
  DateTime firstDate = DateTime::emptyDate();
  DateTime lastDate = DateTime::emptyDate();

  bool isValid() const
  {
    return !firstDate.isEmptyDate() && !lastDate.isEmptyDate() && firstDate <= lastDate;
  }

  bool isInDateRange(const DateTime& dateBetweenDates) const
  {
    return firstDate >= dateBetweenDates && lastDate <= dateBetweenDates;
  }
  void reset()
  {
    firstDate = DateTime::emptyDate();
    lastDate = DateTime::emptyDate();
  }
  void convertFromOnd(const PricingTrx::OriginDestination& ond)
  {
    firstDate = ond.travelDate.subtractDays(ond.calDaysBefore);
    lastDate = ond.travelDate.addDays(ond.calDaysAfter);
  }

  DateRange stripHours() const
  {
    if (!isValid())
      return *this;
    return {firstDate.getOnlyDate(), lastDate.getOnlyDate()};
  }
  DateRange getInclusiveRange() const
  {
    if (!isValid())
      return *this;

    DateRange result = this->stripHours();
    return {result.firstDate, result.lastDate.nextDay()};
  }
};

std::ostream& operator<<(std::ostream& out, const DateRange& dateRange);
bool operator<(const DateRange& lhs, const DateRange& rhs);

enum DateApplication : unsigned
{
  WHOLE_PERIOD = 0,
  SAME_DEPARTURE_DATE,
  LATER_DEPARTURE_DATE,
  DATE_APPLICATION_MAX_SIZE
};

class R3ValidationResult
{
  friend class ExchShopCalendarUtilsTest;

public:
  R3ValidationResult(const std::vector<PricingTrx::OriginDestination>& onds,
                     const Itin& excItin,
                     const Itin& curNewItin,
                     DataHandle& dataHandle);
  R3ValidationResult(const R3ValidationResult&) = delete;
  R3ValidationResult& operator=(const R3ValidationResult&) = delete;

  virtual ~R3ValidationResult() {}
  virtual int32_t getOndIndexForSeg(const TravelSeg& tvlSeg) const;
  bool addDateRange(DateRange dateRange, uint32_t ondIndex);
  DateRange getDateRange() const;
  DateRange getDateRangeForOnd(uint32_t ondIndex) const;
  bool isValid() const { return _status; }
  DateRange intersection(const DateRange dateRange, uint32_t ondIndex) const;

private:
  std::vector<DatePeriod> _dateRanges;
  std::map<int16_t, int32_t> _mapLegToOndIndex;
  bool _status = true;

  DatePeriod intersection(const DatePeriod datePeriod, uint32_t ondIndex) const;
  int32_t legExistsOnItin(
      const std::vector<TravelSeg*>& segVec,
      const Itin& newItin,
      DataHandle& dataHandle,
      const std::function<bool(const TravelSeg*, const TravelSeg*, DataHandle&, bool)>& matcher);
  virtual bool isSameCity(const TravelSeg* lhs,
                          const TravelSeg* rhs,
                          DataHandle& dataHandle,
                          bool isBoarding) const;
  virtual bool isSameCountry(const TravelSeg* lhs,
                             const TravelSeg* rhs,
                             DataHandle& dataHandle,
                             bool isBoarding) const;
  virtual bool isSameSubArea(const TravelSeg* lhs,
                             const TravelSeg* rhs,
                             DataHandle& dataHandle,
                             bool isBoarding) const;
  void forceMatch(const std::vector<PricingTrx::OriginDestination>& ondVec, const int32_t legId);

  void matchToFirstNotMatched(const std::vector<PricingTrx::OriginDestination>& ondVec,
                              const int32_t legId);

  void matchToFirstHigherOrLast(const std::vector<PricingTrx::OriginDestination>& ondVec,
                                const int32_t legId);
};

DateRange
getFirstDateRange(const RexPricingTrx& trx);

DateRange
getLastDateRange(const RexPricingTrx& trx);

DateRange
getDateRangeForOnd(const PricingTrx::OriginDestination* ond, const DateTime& depDate);

bool
isEXSCalendar(const RexBaseTrx& trx);

bool
validateInputParams(const PricingTrx::OriginDestination& ond, bool isTestRequest);

bool
validateCalendarDates(const DateTime& matchingOndDate,
                      const Cat31Info& cat31Info,
                      const bool isTestRequest);

DateApplication
convertToDateApplication(const Indicator ind);

std::string
getDateApplicationInString(const DateApplication& dateAppl);

} // ExchShopCalendar
} // tse
