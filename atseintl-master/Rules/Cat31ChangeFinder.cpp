// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include "Rules/Cat31ChangeFinder.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
FALLBACK_DECL(cat31ChangeFinderOffByOne)

Cat31ChangeFinder::Cat31ChangeFinder(const std::vector<TravelSeg*> travelSegVec,
                                     const std::vector<OriginDestination>& ondVec,
                                     ExchShopCalendar::R3ValidationResult* r3ValidationResults,
                                     const bool isCalendar,
                                     const RexPricingTrx& trx)
  : _travelSegVec(travelSegVec),
    _ondVec(ondVec),
    _r3ValidationResults(r3ValidationResults),
    _isCalendar(isCalendar),
    _trx(trx)
{
}

const int32_t
Cat31ChangeFinder::findOndIndexForSeg(const TravelSeg& travelSeg) const
{
  return _r3ValidationResults->getOndIndexForSeg(travelSeg);
}

DateRange
Cat31ChangeFinder::createDateRangeFromOnd(const TravelSeg& newTvlSeg, const TravelSeg& travelSeg)
    const
{
  const OriginDestination* ondPtr =
      (_ondVec.empty() ? nullptr : &_ondVec.at(findOndIndexForSeg(newTvlSeg)));
  const DateTime ondDate = (ondPtr ? ondPtr->travelDate : newTvlSeg.departureDT());

  return ExchShopCalendar::getDateRangeForOnd(ondPtr, ondDate);
}

std::pair<const AirSeg*, const AirSeg*>
Cat31ChangeFinder::castToAirSeg(const TravelSeg* newTvlSeg, const TravelSeg* tvlSeg)
{
  const AirSeg* newAs = nullptr, *excAs = nullptr;
  if (newTvlSeg->isAir())
    newAs = static_cast<const AirSeg*>(newTvlSeg);
  if (tvlSeg->isAir())
    excAs = static_cast<const AirSeg*>(tvlSeg);

  return {newAs, excAs};
}

bool
Cat31ChangeFinder::matchBothNotAirSeg(const AirSeg* newAs, const AirSeg* excAs) const
{
  return !newAs && !excAs;
}
bool
Cat31ChangeFinder::matchBothAirSeg(const AirSeg* newAs, const AirSeg* excAs) const
{
  return newAs && excAs;
}
bool
Cat31ChangeFinder::matchCarrierAndFlightNumber(const AirSeg* newAs, const AirSeg* excAs) const
{
  return newAs->carrier() == excAs->carrier() && newAs->flightNumber() == excAs->flightNumber();
}

bool
Cat31ChangeFinder::matchedChangedSegOnNewItin(const TravelSeg* tvlSeg)
{
  auto it = _travelSegVec.begin();
  while (it != _travelSegVec.end())
  {
    const TravelSeg* newTvlSeg = *it;
    DateRange dateRange = createDateRangeFromOnd(*newTvlSeg, *tvlSeg).stripHours();

    if (newTvlSeg->boardMultiCity() == tvlSeg->boardMultiCity() &&
        newTvlSeg->offMultiCity() == tvlSeg->offMultiCity() &&
        (tvlSeg->departureDT().getOnlyDate()).isBetween(dateRange.firstDate, dateRange.lastDate))
    {
      const AirSeg* newAirSeg, *airSeg;
      std::tie(newAirSeg, airSeg) = castToAirSeg(newTvlSeg, tvlSeg);

      if (matchBothNotAirSeg(newAirSeg, airSeg) ||
          (matchBothAirSeg(newAirSeg, airSeg) && matchCarrierAndFlightNumber(newAirSeg, airSeg)))
      {
        return true;
      }

      it = _travelSegVec.erase(it);
    }
    else
    {
      ++it;
    }
  }

  return false;
}

void
Cat31ChangeFinder::updateCalendarValidationResults(const TravelSeg& tvlSeg) const
{
  if (_r3ValidationResults)
  {
    const int32_t ondIndex = findOndIndexForSeg(tvlSeg);
    if (ondIndex == ExchShopCalendar::INVALID_OND_INDEX)
      return;

    DateRange dateRange{tvlSeg.departureDT(), tvlSeg.departureDT()};

    // remove _trx param on fallback removal
    if (!fallback::cat31ChangeFinderOffByOne(&_trx))
    {
      _r3ValidationResults->addDateRange(dateRange.stripHours(), ondIndex);
    }
    else
    {
      _r3ValidationResults->addDateRange(dateRange, ondIndex);
    }
  }
}

bool
Cat31ChangeFinder::notChanged(const std::vector<TravelSeg*>& travelSegVec)
{
  if (_isCalendar)
  {
    for (const auto* tvlSeg : travelSegVec)
      updateCalendarValidationResults(*tvlSeg);
  }

  auto isChanged = [&](const TravelSeg* tvlSeg)
  {
    return tvlSeg->changeStatus() == TravelSeg::CHANGED && !matchedChangedSegOnNewItin(tvlSeg);
  };

  return std::none_of(travelSegVec.begin(), travelSegVec.end(), isChanged);
}

} // tse
