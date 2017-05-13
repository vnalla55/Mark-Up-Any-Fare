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

#include "Common/ExchShopCalendarUtils.h"
#include "Common/LocUtil.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "Rules/ReissueTable.h"

#include <boost/range/adaptors.hpp>

#include <type_traits>

namespace tse
{
namespace ExchShopCalendar
{
std::ostream& operator<<(std::ostream& out, const DateRange& dateRange)
{
  out << (dateRange.firstDate.dateToString(YYYYMMDD, "-") + "/" +
          dateRange.lastDate.dateToString(YYYYMMDD, "-"));
  return out;
}

bool operator<(const DateRange& lhs, const DateRange& rhs)
{
  return lhs.firstDate < rhs.firstDate ||
         (lhs.firstDate == rhs.firstDate && lhs.lastDate < rhs.lastDate);
}

R3ValidationResult::R3ValidationResult(const std::vector<PricingTrx::OriginDestination>& onds,
                                       const Itin& excItin,
                                       const Itin& curNewItin,
                                       DataHandle& dataHandle)
{
  for (auto& ond : onds)
  {
    // time_period creates a period as [begin, end), we must add one more day to last date
    _dateRanges.emplace_back(ond.travelDate.subtractDays(ond.calDaysBefore),
                             ond.travelDate.addDays(ond.calDaysAfter + 1));
  }

  int16_t skipped = 0;
  std::map<int32_t, std::vector<TravelSeg*>> legMap;

  for (auto* seg : excItin.travelSeg())
  {
    legMap[seg->legId()].push_back(seg);
  }

  std::vector<std::function<bool(const TravelSeg*, const TravelSeg*, DataHandle&, bool)>>
  matcherFuncs{[&](const TravelSeg* lhs, const TravelSeg* rhs, DataHandle& dh, bool is)
               { return isSameCity(lhs, rhs, dh, is); },
               [&](const TravelSeg* lhs, const TravelSeg* rhs, DataHandle& dh, bool is)
               { return isSameCountry(lhs, rhs, dh, is); },
               [&](const TravelSeg* lhs, const TravelSeg* rhs, DataHandle& dh, bool is)
               { return isSameSubArea(lhs, rhs, dh, is); }};

  for (const auto& matcher : matcherFuncs)
  {
    if (legMap.empty())
      break;

    auto legIt = legMap.begin();
    while (legIt != legMap.end())
    {
      auto& pair = *legIt;
      const uint32_t legId = pair.first;
      const std::vector<TravelSeg*>& segVec = pair.second;
      int16_t matched = legExistsOnItin(segVec, curNewItin, dataHandle, matcher);

      if (matched != INVALID_OND_INDEX)
      {
        _mapLegToOndIndex[legId] = matched - skipped;
        legIt = legMap.erase(legIt);
        skipped += onds[matched].skippedOND;
      }
      else
      {
        ++legIt;
      }
    }
  }

  if (!legMap.empty())
  {
    for (auto legId : legMap | boost::adaptors::map_keys)
    {
      forceMatch(onds, legId);
    }
  }
}

int32_t
R3ValidationResult::legExistsOnItin(
    const std::vector<TravelSeg*>& segVec,
    const Itin& newItin,
    DataHandle& dataHandle,
    const std::function<bool(const TravelSeg*, const TravelSeg*, DataHandle&, bool)>& matcher)
{
  for (auto* fareMarket : newItin.fareMarket())
  {
    const std::vector<TravelSeg*>& newSegVec = fareMarket->travelSeg();
    TSE_ASSERT(!newSegVec.empty());
    const TravelSeg* boardSeg = nullptr, *offSeg = nullptr;
    for (const TravelSeg* seg : newSegVec)
    {
      if (!boardSeg && matcher(seg, segVec.front(), dataHandle, true))
      {
        boardSeg = seg;
      }

      if (!offSeg && matcher(seg, segVec.back(), dataHandle, false))
      {
        offSeg = seg;
      }

      if (boardSeg && offSeg)
        return boardSeg->legId() - 1;
    }
  }
  return INVALID_OND_INDEX;
}

bool
R3ValidationResult::isSameCity(const TravelSeg* lhs,
                               const TravelSeg* rhs,
                               DataHandle& dataHandle,
                               bool isBoarding) const
{
  auto func = [&](const TravelSeg& seg)
  { return (isBoarding ? seg.boardMultiCity() : seg.offMultiCity()); };

  return LocUtil::isSameCity(func(*lhs), func(*rhs), dataHandle);
}

bool
R3ValidationResult::isSameCountry(const TravelSeg* lhs,
                                  const TravelSeg* rhs,
                                  DataHandle& dataHandle,
                                  bool isBoarding) const
{
  auto func = [&](const TravelSeg& seg)
  { return (isBoarding ? *seg.origin() : *seg.destination()); };

  return LocUtil::isInSameCountry(func(*lhs), func(*rhs));
}

bool
R3ValidationResult::isSameSubArea(const TravelSeg* lhs,
                                  const TravelSeg* rhs,
                                  DataHandle& dataHandle,
                                  bool isBoarding) const
{
  auto func = [&](const TravelSeg& seg)
  { return (isBoarding ? *seg.origin() : *seg.destination()); };

  return func(*lhs).subarea() == func(*rhs).subarea();
}

void
R3ValidationResult::forceMatch(const std::vector<PricingTrx::OriginDestination>& ondVec,
                               const int32_t legId)
{
  if (_mapLegToOndIndex.size() < _dateRanges.size())
    matchToFirstNotMatched(ondVec, legId);
  else
    matchToFirstHigherOrLast(ondVec, legId);
}

void
R3ValidationResult::matchToFirstNotMatched(const std::vector<PricingTrx::OriginDestination>& ondVec,
                                           const int32_t legId)
{
  for (std::decay_t<decltype(ondVec)>::size_type index = 0; index < ondVec.size(); ++index)
  {
    if (_mapLegToOndIndex.find(index) == _mapLegToOndIndex.end())
    {
      _mapLegToOndIndex[legId] = _mapLegToOndIndex[index];
      return;
    }
  }
}

void
R3ValidationResult::matchToFirstHigherOrLast(
    const std::vector<PricingTrx::OriginDestination>& ondVec, const int32_t legId)
{
  auto valueToMap = ondVec.size() - 1;
  const auto upperBoundIt = _mapLegToOndIndex.upper_bound(legId);
  if (upperBoundIt != _mapLegToOndIndex.end())
  {
    valueToMap = upperBoundIt->second;
  }

  _mapLegToOndIndex[legId] = valueToMap;
}

int32_t
R3ValidationResult::getOndIndexForSeg(const TravelSeg& tvlSeg) const
{
  if (_mapLegToOndIndex.find(tvlSeg.legId()) == _mapLegToOndIndex.end())
    return INVALID_OND_INDEX;

  return _mapLegToOndIndex.at(tvlSeg.legId());
}

bool
R3ValidationResult::addDateRange(DateRange dateRange, uint32_t ondIndex)
{
  if (_status)
  {
    TSE_ASSERT(_dateRanges.size() > ondIndex);
    DatePeriod& resultPeriod = _dateRanges[ondIndex];
    DatePeriod newPeriod(dateRange.firstDate, dateRange.lastDate.addDays(1));
    resultPeriod = intersection(newPeriod, ondIndex);
    _status = !resultPeriod.is_null();
  }
  return _status;
}

DateRange
R3ValidationResult::getDateRange() const
{
  TSE_ASSERT(!_dateRanges.empty());
  return {_dateRanges.front().begin(), _dateRanges.back().last()};
}

DateRange R3ValidationResult::getDateRangeForOnd(uint32_t ondIndex) const
{
  TSE_ASSERT(_dateRanges.size() > ondIndex);
  const DatePeriod& resultPeriod = _dateRanges[ondIndex];
  return {resultPeriod.begin(), resultPeriod.last()};
}

DateRange R3ValidationResult::intersection(const DateRange dateRange, uint32_t ondIndex) const
{
  DatePeriod resultPeriod =
      intersection(DatePeriod(dateRange.firstDate, dateRange.lastDate), ondIndex);
  if (resultPeriod.is_null())
    return {DateTime::emptyDate(), DateTime::emptyDate()};
  return {resultPeriod.begin(), resultPeriod.last()};
}

DatePeriod
R3ValidationResult::intersection(const DatePeriod datePeriod, uint32_t ondIndex) const
{
  TSE_ASSERT(_dateRanges.size() > ondIndex);
  return _dateRanges[ondIndex].intersection(datePeriod);
}

DateRange
getFirstDateRange(const RexPricingTrx& trx)
{
  const PricingTrx::OriginDestination* ond =
      (!trx.orgDest.empty() ? &trx.orgDest.front() : nullptr);
  const DateTime& depDate = trx.curNewItin()->travelSeg().front()->departureDT();

  return getDateRangeForOnd(ond, depDate);
}

DateRange
getLastDateRange(const RexPricingTrx& trx)
{
  const PricingTrx::OriginDestination* ond = (!trx.orgDest.empty() ? &trx.orgDest.back() : nullptr);
  const DateTime& depDate = trx.curNewItin()->travelSeg().back()->departureDT();

  return getDateRangeForOnd(ond, depDate);
}

DateRange
getDateRangeForOnd(const PricingTrx::OriginDestination* ond, const DateTime& depDate)
{
  if (!ond)
    return {depDate, depDate};

  return {depDate.subtractDays(ond->calDaysBefore), depDate.addDays(ond->calDaysAfter)};
}

bool
isEXSCalendar(const RexBaseTrx& trx)
{
  for (const auto& ond : trx.originDest())
  {
    if (ond.calDaysBefore | ond.calDaysAfter)
      return true;
  }
  return false;
}

bool
validateInputParams(const PricingTrx::OriginDestination& ond, bool isTestRequest)
{
  if (isTestRequest)
    return true;

  std::array<uint32_t, 2> validOptions = {0, 3};

  auto isValid = [&](uint32_t ondValue)
                     {
                        for(uint32_t option : validOptions)
                        {
                          if (option == ondValue)
                            return true;
                        }
                        return false;
                     };

  return isValid(ond.calDaysBefore) && isValid(ond.calDaysAfter);
}

bool
validateCalendarDates(const DateTime& travelDate,
                      const Cat31Info& cat31Info,
                      const bool isTestRequest)
{
  if (isTestRequest)
  {
    return true;
  }

  std::array<uint32_t, 2> validOptions{0, 3};

  return std::any_of(validOptions.cbegin(),
                     validOptions.cend(),
                     [&](const auto& option)
                     {
    return travelDate.subtractDays(option) >= cat31Info.rec3DateRange.firstDate &&
           travelDate.addDays(option) <= cat31Info.rec3DateRange.lastDate;
  });
}

DateApplication
convertToDateApplication(const Indicator ind)
{
  switch (ind)
  {
  case ReissueTable::MATCH_SAME_DEPARTURE_DATE:
    return ExchShopCalendar::SAME_DEPARTURE_DATE;
  case ReissueTable::MATCH_LATER_DEPARTURE_DATE:
    return ExchShopCalendar::LATER_DEPARTURE_DATE;
  default:
    return ExchShopCalendar::WHOLE_PERIOD;
  }
}
std::string
getDateApplicationInString(const DateApplication& dateAppl)
{
  switch (dateAppl)
  {
  case SAME_DEPARTURE_DATE:
    return "SAME DEPARTURE";
  case LATER_DEPARTURE_DATE:
    return "LATER DEPARTURE";
  case WHOLE_PERIOD:
    return "WHOLE PERIOD";
  case DATE_APPLICATION_MAX_SIZE:
  default:
    break;
  }
  TSE_ASSERT(!"INCORRECT DATE APPLICATION TO DEDUCE SUPPLIED");
}
} // ExchShopCalendar
} // tse
