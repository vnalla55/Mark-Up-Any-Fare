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

#include "Common/OCUtil.h"
#include "DataModel/Services/CarrierFlight.h"
#include "DataModel/Services/CarrierFlightSegment.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/PaymentDetail.h"
#include "Rules/CarrierFlightApplicator.h"
#include "Rules/CarrierFlightRule.h"

namespace tax
{
CarrierFlightApplicator::CarrierFlightApplicator(
    const CarrierFlightRule& rule,
    const GeoPath& geoPath,
    const std::vector<FlightUsage>& flightUsages,
    std::shared_ptr<const CarrierFlight> carrierFlightBefore,
    std::shared_ptr<const CarrierFlight> carrierFlightAfter)
  : BusinessRuleApplicator(&rule),
    _geoPath(geoPath),
    _flightUsages(flightUsages),
    _carrierFlightBefore(carrierFlightBefore),
    _carrierFlightAfter(carrierFlightAfter),
    _carrierFlightRule(rule)
{
}

CarrierFlightApplicator::~CarrierFlightApplicator()
{
}

bool
CarrierFlightApplicator::isFlightNumberInRange(type::FlightNumber flightNumber,
                                               type::FlightNumber flightFrom,
                                               type::FlightNumber flightTo) const
{
  return (flightNumber >= flightFrom) && (flightNumber <= flightTo);
}

bool
CarrierFlightApplicator::isFlightMatchingConditions(
    const Flight* flight, std::shared_ptr<const CarrierFlight> carrierFlight) const
{
  if (carrierFlight != nullptr)
  {
    for (const CarrierFlightSegment& cfs : carrierFlight->segments)
    {
      if (flight->marketingCarrier() == cfs.marketingCarrier &&
          (cfs.operatingCarrier.empty() || (flight->operatingCarrier() == cfs.operatingCarrier)) &&
          isFlightNumberInRange(
              flight->marketingCarrierFlightNumber(), cfs.flightFrom, cfs.flightTo))
        return true;
    }
  }

  return false;
}

bool
CarrierFlightApplicator::matchDirection(type::Index loc1Id, bool beforeTable, bool departureTag)
    const
{
  type::Index tableIndex = beforeTable ? _carrierFlightRule.carrierFlightItemBefore()
                                       : _carrierFlightRule.carrierFlightItemAfter();
  bool usePrevious = departureTag == beforeTable;

  if (tableIndex == 0) // no table; match any flight
    return true;

  if (usePrevious && loc1Id == 0) // Arrival tax applied to origin
    return false;

  if (!usePrevious && loc1Id == _geoPath.geos().size() - 1) // D tax applied to destination
    return false;

  type::Index geoIndex = loc1Id - (usePrevious ? 1 : -1);
  Flight const* flight = _geoPath.geos()[geoIndex].getFlight(_flightUsages);
  std::shared_ptr<CarrierFlight const> carrierFltTable =
      beforeTable ? _carrierFlightBefore : _carrierFlightAfter;
  return isFlightMatchingConditions(flight, carrierFltTable);
}

bool
CarrierFlightApplicator::matches(type::Index loc1Id, bool departureTag) const
{
  return matchDirection(loc1Id, true, departureTag) && matchDirection(loc1Id, false, departureTag);
}

bool
CarrierFlightApplicator::apply(PaymentDetail& paymentDetail) const
{
  const bool departureTag = _carrierFlightRule.taxPointTag() == type::TaxPointTag::Departure;
  const bool ruleMatches = matches(paymentDetail.getTaxPointBegin().id(), departureTag);

  if (ruleMatches)
    return true;

  for (OptionalService& ocItem : paymentDetail.optionalServiceItems())
  {
    if (!ocItem.isFailed() && OCUtil::isOCSegmentRelated(ocItem.type()))
    {
      ocItem.setFailedRule(&_carrierFlightRule);
    }
  }

  paymentDetail.failItinerary(_carrierFlightRule);
  paymentDetail.failYqYrs(_carrierFlightRule);

  return !paymentDetail.isFailed();
}

} // namespace tax
