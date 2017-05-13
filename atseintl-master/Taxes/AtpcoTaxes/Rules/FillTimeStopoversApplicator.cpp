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
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeOps.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/FillTimeStopoversApplicator.h"
#include "Rules/FillTimeStopoversRule.h"
#include "Rules/PaymentDetail.h"

#include <boost/variant/apply_visitor.hpp>

namespace tax
{
namespace
{

FillTimeStopoversApplicator::VariantStopoverChecker
getTimeChecker(type::StopoverTimeTag num, type::StopoverTimeUnit unit, const Itin& itin)
{
  if (num == "D")
    return DomesticTimeStopoverChecker(itin);
  if (num == "A")
    return SpecialDomesticTimeStopoverChecker(itin);

  int number = toInt(num);
  if (number == 0 && unit == type::StopoverTimeUnit::Blank)
    return SpecificTimeStopoverChecker(24 * 60);
  if (number == 0 && unit == type::StopoverTimeUnit::Days)
    return DateStopoverChecker(number);
  if (number <= 0)
    throw std::range_error(std::string("Unsupported stopover length: ") + num.asString());
  if (unit == type::StopoverTimeUnit::Minutes)
    return SpecificTimeStopoverChecker(number);
  if (unit == type::StopoverTimeUnit::Hours)
    return SpecificTimeStopoverChecker(number * 60);
  if (unit == type::StopoverTimeUnit::Days)
    return SpecificTimeStopoverChecker(number * 24 * 60);
  if (unit == type::StopoverTimeUnit::Months)
    return MonthsStopoverChecker(number);
  if (unit == type::StopoverTimeUnit::HoursSameDay)
    return SameDayStopoverChecker(number);

  throw std::range_error(std::string("Unsupported time number/units combination: number is '") +
                         num.asString() + "', unit is '" + safeEnumToString(unit) + "'");
}

class IsStopover : public boost::static_visitor<bool>
{
  const FlightUsage& _prev;
  const FlightUsage& _next;

public:
  IsStopover(const FlightUsage& prev, const FlightUsage& next) : _prev(prev), _next(next) {}

  template <typename T>
  bool operator()(const T& checker) const { return checker.isStopover(_prev, _next); }
};

} // namespace

FillTimeStopoversApplicator::FillTimeStopoversApplicator(const FillTimeStopoversRule* businessRule,
                                                         const Itin& itin)
  : BusinessRuleApplicator(businessRule),
    _fillTimeStopoversRule(businessRule),
    _itin(itin),
    _timeStopoverChecker(getTimeChecker(businessRule->number(), businessRule->unit(), itin))
{
}

bool
FillTimeStopoversApplicator::isStopover(const FlightUsage& prev, const FlightUsage& next) const
{
  return boost::apply_visitor(IsStopover{prev, next}, _timeStopoverChecker);
}

FillTimeStopoversApplicator::~FillTimeStopoversApplicator()
{
}

bool
FillTimeStopoversApplicator::apply(PaymentDetail& paymentDetail) const
{
  const std::vector<Geo>& geos(_itin.geoPath()->geos());
  TaxPointsProperties& taxPointsProperties = paymentDetail.getMutableTaxPointsProperties();
  for (const Geo& geo : geos)
  {
    const type::Index geoId = geo.id();
    TaxPointProperties& properties = taxPointsProperties[geoId];
    if (properties.isOpen)
      continue;

    if ((0 < geoId) && (geoId < geos.size() - 1))
    {
      const type::Index flightId = (geoId / 2);

      const FlightUsage* flightUsageBefore = nullptr;
      const FlightUsage* flightUsageAfter = nullptr;

      if ((geoId % 2) == 0) // departure
      {
        flightUsageBefore = &_itin.flightUsages()[flightId - 1];
        flightUsageAfter = &_itin.flightUsages()[flightId];
      }
      else // arrival
      {
        flightUsageBefore = &_itin.flightUsages()[flightId];
        flightUsageAfter = &_itin.flightUsages()[flightId + 1];
      }

      if (flightUsageBefore->forcedConnection() == type::ForcedConnection::Blank)
      {
        if (flightUsageBefore != nullptr && flightUsageAfter != nullptr)
        {
          properties.isTimeStopover = isStopover(*flightUsageBefore, *flightUsageAfter);
        }
      }
    }
  }

  return true;
}

} // namespace tax
