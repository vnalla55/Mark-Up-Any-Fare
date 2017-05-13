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

#include "Common/LocationUtil.h"
#include "Rules/TimeStopoverChecker.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "DataModel/Common/CodeOps.h"

#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace tax
{

namespace
{

bool isDomestic(const Itin& itin, type::Nation nation = type::Nation{UninitializedCode})
{
  return itin.geoPath()->isJourneyDomestic() && matches(nation, itin.geoPath()->getOriginNation());
}

int domesticThresholdFor(type::Nation nation)
{
  return (nation == "US" || nation == "CA") ? 240 : 360;
}

bool timeDifferenceExceedsThreshold(const FlightUsage& prev, const FlightUsage& next,
                                    int thresholdInMinutes)
{
  int timeDiff = next.departureTime() - prev.arrivalTime();
  timeDiff += 24 * 60 * (next.departureDate() - prev.arrivalDate());
  return timeDiff > thresholdInMinutes;
}

} // anonymous namespace

SpecificTimeStopoverChecker::SpecificTimeStopoverChecker(int minutes) : _minutes(minutes)
{
  _minutes = minutes;
}

bool
SpecificTimeStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  return timeDifferenceExceedsThreshold(prev, next, _minutes);
}

SameDayStopoverChecker::SameDayStopoverChecker(int hours) { _minutes = hours * 60; }

bool
MonthsStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  type::Date arrDate = prev.arrivalDate();
  uint16_t year = arrDate.year();
  uint16_t month = arrDate.month();
  uint16_t day = arrDate.day();
  boost::gregorian::date dateG(year, month, day);
  boost::gregorian::date lastDateG(dateG + boost::gregorian::months(_months));
  type::Date lastDate(lastDateG.year(), lastDateG.month(), lastDateG.day());
  type::Date depDate = next.departureDate();
  return lastDate < depDate || (lastDate == depDate && prev.arrivalTime() < next.departureTime());
}

bool
SameDayStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  if (next.departureDate() != prev.arrivalDate())
    return true;

  int timeDiff = next.departureTime() - prev.arrivalTime();
  return timeDiff > _minutes;
}

DateStopoverChecker::DateStopoverChecker(int days) : _days(days) {}

bool
DateStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  return (next.departureDate() - prev.arrivalDate()) > _days;
}

SpecialDomesticTimeStopoverChecker::SpecialDomesticTimeStopoverChecker(const Itin& itin)
  : _geos(itin.geoPath()->geos())
{
}

bool
SpecialDomesticTimeStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next)
    const
{
  if (timeDifferenceExceedsThreshold(prev, next, 24 * 60))
    return true;

  const Geo& nextOrigin = _geos[next.getId() * 2];
  const Geo& nextDestination = _geos[next.getId() * 2 + 1];

  if (LocationUtil::isInternational(nextOrigin.getNation(), nextDestination.getNation()))
    return false;

  type::Timestamp prevArrival(prev.arrivalDate(), prev.arrivalTime());
  type::Timestamp nextDeparture(next.departureDate(), next.departureTime());

  if (prev.arrivalTime().hour() < 17)
    return nextDeparture - prevArrival > 6 * 60;

  return prev.arrivalDate() < next.departureDate() && next.departureTime() > type::Time(10, 0);
}

DomesticTimeStopoverChecker::DomesticTimeStopoverChecker(const Itin& itin)
  : _byMinutes(domesticThresholdFor(itin.geoPath()->getOriginNation())),
    _isDomestic(isDomestic(itin))
{
}

DomesticTimeStopoverChecker::DomesticTimeStopoverChecker(const Itin& itin, int minutes,
                                                         type::Nation nation)
  : _byMinutes(minutes),
    _isDomestic(isDomestic(itin, nation))
{
}

bool
DomesticTimeStopoverChecker::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  return _isDomestic && _byMinutes.isStopover(prev, next);
}
}
