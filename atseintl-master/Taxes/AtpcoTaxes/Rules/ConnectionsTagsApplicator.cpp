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

#include "Rules/ConnectionsTagsApplicator.h"

#include "Common/LocationUtil.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/ConnectionsTagsRule.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TurnaroundCalculator.h"
#include "ServiceInterfaces/MileageService.h"

namespace tax
{
namespace
{
typedef std::pair<type::Index, type::Miles> GeoIdMile;

typedef bool
InternationalGatewayChecker(bool isFromDomestic, bool isToDomestic);

bool
isDomesticToInternational(bool isFromDomestic, bool isToDomestic)
{
  return isFromDomestic && !isToDomestic;
}

bool
isInternationalToDomestic(bool isFromDomestic, bool isToDomestic)
{
  return !isFromDomestic && isToDomestic;
}

bool
isInternationalToInternational(bool isFromDomestic, bool isToDomestic)
{
  return !isFromDomestic && !isToDomestic;
}

bool
isMultiairport(const Geo& arrival, const Geo& departure, bool)
{
  return arrival.locCode() != departure.locCode();
}

bool
isDifferentMarketingCarrier(const Flight* arrival, const Flight* departure)
{
  return arrival->marketingCarrier() != departure->marketingCarrier();
}

bool
isGroundTransport(const Flight* arrival, const Flight* departure)
{
  return arrival->isGroundTransport() && !departure->isGroundTransport();
}

template <InternationalGatewayChecker checker>
bool
isGateway(const Geo& arrival, const Geo& departure, bool matchTicketedOnly)
{
  const Geo* prevDeparture = arrival.prev();
  const Geo* nextArrival = departure.next();
  if (matchTicketedOnly)
  {
    while (prevDeparture->isUnticketed())
      prevDeparture = prevDeparture->prev()->prev();
    while (nextArrival->isUnticketed())
      nextArrival = nextArrival->next()->next();
  }
  assert (prevDeparture != nullptr);
  assert (nextArrival != nullptr);

  bool isFromDomestic = LocationUtil::isDomestic(prevDeparture->getNation(), arrival.getNation());
  bool isToDomestic = LocationUtil::isDomestic(departure.getNation(), nextArrival->getNation());
  return checker(isFromDomestic, isToDomestic);
}

bool
isWhollyConnected(const TaxPointsProperties& taxPointsProperties)
{
  for (const TaxPointProperties& taxPointProperties : taxPointsProperties)
  {
    if ((taxPointProperties.isTimeStopover && *taxPointProperties.isTimeStopover) ||
        taxPointProperties.isSurfaceStopover() ||
        taxPointProperties.isExtendedStopover)
      return false;
  }
  return true;
}

bool
isFareBreakOrStop(const TaxPointProperties& properties)
{
  return (properties.isFareBreak ||
          (properties.isTimeStopover && *properties.isTimeStopover) ||
          properties.isSurfaceStopover() ||
          properties.isOpen);
}

bool
isStopoverOrFareBreakBetween(const Geo* geo,
                             const Geo* geo2,
                             const TaxPointsProperties& properties,
                             type::TaxPointTag taxPointTag)
{
  if (geo->id() > geo2->id())
    std::swap(geo, geo2);

  type::Index firstId(geo->id()), lastId(geo2->id()), increment(2);
  if (taxPointTag == type::TaxPointTag::Departure) // we want to check only arrivals
  {
    firstId += geo->isDeparture() ? 1 : 2;

    if (geo2->isDeparture())
    {
      if (geo2->prev() && geo2->prev()->cityCode() == geo2->cityCode())
        lastId -= 3;
      else
        lastId -= 1;
    }
    else
      lastId -= 2;
  }
  else if (taxPointTag == type::TaxPointTag::Arrival) // we want to check only departures
  {
    if (geo->isDeparture())
      firstId += 2;
    else
    {
      if (geo->next() && geo->next()->cityCode() == geo->cityCode())
        firstId += 3;
      else
        firstId += 1;
    }

    lastId -= geo2->isDeparture() ? 2 : 1;
  }
  else
  {
    increment = 1;
    firstId += geo->isDeparture() ? 1 : 2;
    lastId -= geo2->isDeparture() ? 2 : 1;
  }

  if (lastId >= properties.size() || firstId >= properties.size())
    return false;

  for (type::Index geoId = firstId; geoId <= lastId; geoId += increment)
  {
    if (isFareBreakOrStop(properties[geoId]))
      return true;
  }
  return false;
}

typedef bool
StopoverChecker(const Geo& arrival, const Geo& departure, bool matchTicketedOnly);
typedef type::ConnectionsTag Tag;

void
applyStopoverTag(const GeoPath& geoPath,
                 TaxPointsProperties& properties,
                 StopoverChecker& stopoverChecker,
                 type::ConnectionsTag tag,
                 type::TicketedPointTag ticketedPointTag)
{
  bool matchTicketedOnly = (ticketedPointTag == type::TicketedPointTag::MatchTicketedPointsOnly);
  type::Index firstArrival = 1;
  type::Index lastArrival = type::Index(geoPath.geos().size() - 1);
  for (type::Index i = firstArrival; i < lastArrival; i += 2)
  {
    const Geo& arrivalGeo = geoPath.geos()[i];
    const Geo& departureGeo = geoPath.geos()[i + 1];
    if (matchTicketedOnly && arrivalGeo.isUnticketed() && departureGeo.isUnticketed())
      continue;

    if (arrivalGeo.cityCode() == departureGeo.cityCode())
    {
      if (stopoverChecker(arrivalGeo, departureGeo, matchTicketedOnly))
      {
        properties[i].isExtendedStopover.setTag(tag);
        properties[i + 1].isExtendedStopover.setTag(tag);
      }
    }
  }
}

} // anonymous namespace

ConnectionsTagsApplicator::ConnectionsTagsApplicator(
    const ConnectionsTagsRule& rule,
    const Itin& itin,
    const type::Timestamp travelDate,
    const MileageService& mileageService)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _connectionsTagsSet(rule.getConnectionsTagsSet()),
    _itin(itin),
    _geoPath(*itin.geoPath()),
    _flightUsages(itin.flightUsages()),
    _travelDate(travelDate),
    _mileageService(mileageService)
{
}

ConnectionsTagsApplicator::~ConnectionsTagsApplicator()
{
}

bool
ConnectionsTagsApplicator::apply(PaymentDetail& paymentDetail) const
{
  using type::ConnectionsTag;
  TaxPointsProperties& properties = paymentDetail.getMutableTaxPointsProperties();

  if (_rule.caSurfaceException())
  {
    for (auto& tpp : properties)
    {
      if (tpp.isSurface())
        tpp.setIsCASurfaceException(true);
    }
  }

  for (const ConnectionsTag& connectionsTag : _connectionsTagsSet)
  {
    if (connectionsTag == ConnectionsTag::GroundTransport)
    {
      applyGroundTransportAsStopover(properties);
    }
    else if (connectionsTag == ConnectionsTag::Multiairport)
    {
      applyStopoverTag(_geoPath,
                       properties,
                       isMultiairport,
                       ConnectionsTag::Multiairport,
                       paymentDetail.ticketedPointTag());
    }
    else if (connectionsTag == ConnectionsTag::DifferentMarketingCarrier)
    {
      applyDifferentMarketingCarrierAsStopover(properties);
    }
    else if (connectionsTag == ConnectionsTag::DomesticToInternational)
    {
      applyStopoverTag(_geoPath,
                       properties,
                       isGateway<isDomesticToInternational>,
                       ConnectionsTag::DomesticToInternational,
                       paymentDetail.ticketedPointTag());
    }
    else if (connectionsTag == ConnectionsTag::InternationalToDomestic)
    {
      applyStopoverTag(_geoPath,
                       properties,
                       isGateway<isInternationalToDomestic>,
                       ConnectionsTag::InternationalToDomestic,
                       paymentDetail.ticketedPointTag());
    }
    else if (connectionsTag == ConnectionsTag::InternationalToInternational)
    {
      applyStopoverTag(_geoPath,
                       properties,
                       isGateway<isInternationalToInternational>,
                       ConnectionsTag::InternationalToInternational,
                       paymentDetail.ticketedPointTag());
    }
    else if (connectionsTag == ConnectionsTag::TurnaroundPoint)
    {
      applyTurnaroundPointAsStopover(paymentDetail);
    }
    else if (connectionsTag == ConnectionsTag::FareBreak)
    {
      applyFareBreakAsStopover(properties);
    }
    else if (connectionsTag == ConnectionsTag::FurthestFareBreak)
    {
      applyFarthestFareBreakAsStopover(properties);
    }
    else if (connectionsTag == ConnectionsTag::TurnaroundPointForConnection)
    {
      applyTurnaroundPointForConnectionAsStopover(paymentDetail);
    }
  }

  return true;
}

template <ConnectionsTagsApplicator::FlightStopoverChecker checker>
bool
ConnectionsTagsApplicator::isFlightStopover(const Geo& arrivalGeo, const Geo& departureGeo) const
{
  const Flight* arrivalFlight = arrivalGeo.getFlight(_flightUsages);
  const Flight* departureFlight = departureGeo.getFlight(_flightUsages);
  return checker(arrivalFlight, departureFlight);
}

void
ConnectionsTagsApplicator::applyGroundTransportAsStopover(
    TaxPointsProperties& properties) const
{
  type::Index firstArrival = 1;
  type::Index lastArrival = type::Index(_geoPath.geos().size() - 1);
  for (type::Index i = firstArrival; i < lastArrival; i += 2)
  {
    const Geo& arrivalGeo = _geoPath.geos()[i];
    const Geo& departureGeo = _geoPath.geos()[i + 1];

    if (arrivalGeo.cityCode() == departureGeo.cityCode())
    {
      if (isFlightStopover<isGroundTransport>(arrivalGeo, departureGeo))
      {
        properties[i].isExtendedStopover.setTag(type::ConnectionsTag::GroundTransport);
        properties[i + 1].isExtendedStopover.setTag(type::ConnectionsTag::GroundTransport);
      }
    }
  }
}

void
ConnectionsTagsApplicator::applyDifferentMarketingCarrierAsStopover(
    TaxPointsProperties& properties) const
{
  type::Index firstArrival = 1;
  type::Index lastArrival = type::Index(_geoPath.geos().size() - 1);
  for (type::Index i = firstArrival; i < lastArrival; i += 2)
  {
    const Geo& arrivalGeo = _geoPath.geos()[i];
    const Geo& departureGeo = _geoPath.geos()[i + 1];

    if (arrivalGeo.cityCode() == departureGeo.cityCode())
    {
      if (isFlightStopover<isDifferentMarketingCarrier>(arrivalGeo, departureGeo))
      {
        properties[i].isExtendedStopover.setTag(type::ConnectionsTag::DifferentMarketingCarrier);
        properties[i + 1].isExtendedStopover.setTag(
            type::ConnectionsTag::DifferentMarketingCarrier);
      }
    }
  }
}

void
ConnectionsTagsApplicator::applyTurnaroundPointForConnectionAsStopover(PaymentDetail& paymentDetail)
    const
{
  TaxPointsProperties& properties = paymentDetail.getMutableTaxPointsProperties();

  if (!isWhollyConnected(properties))
    return;

  TurnaroundCalculator turnaroundCalculator(_geoPath,
                                            _mileageService,
                                            properties,
                                            paymentDetail.specUS_RTOJLogic(),
                                            _rule.alternateTurnaroundDeterminationLogic());
  const Geo* geo = _itin.getTurnaround(turnaroundCalculator);
  if (!geo) // no matching fare break found
    return;

  properties[geo->id()].isExtendedStopover.setTag(Tag::TurnaroundPointForConnection);

  if (geo->isDeparture())
  {
    if (!geo->isFirst())
    {
      type::Index arrivalGeoId = geo->id() - 1;
      properties[arrivalGeoId].isExtendedStopover.setTag(Tag::TurnaroundPointForConnection);
    }
  }
  else
  {
    if (!geo->isLast())
    {
      type::Index departureGeoId = geo->id() + 1;
      properties[departureGeoId].isExtendedStopover.setTag(Tag::TurnaroundPointForConnection);
    }
  }
}

void
ConnectionsTagsApplicator::applyTurnaroundPointAsStopover(PaymentDetail& paymentDetail) const
{
  if (!paymentDetail.roundTripOrOpenJaw())
    return;

  TaxPointsProperties& properties = paymentDetail.getMutableTaxPointsProperties();

  TurnaroundCalculator turnaroundCalculator(_geoPath,
                                            _mileageService,
                                            properties,
                                            paymentDetail.specUS_RTOJLogic(),
                                            _rule.alternateTurnaroundDeterminationLogic());
  const Geo* turnaroundGeo = _itin.getTurnaround(turnaroundCalculator);
  if (!turnaroundGeo) // no matching fare break found
    return;

  if (doubleOccurrence(turnaroundGeo->id(), properties))
    return;

  const Geo* farthestExceptTurnaroundFareBreakOrStopGeo =
      getFarthestFareBreakOrStopoverExceptTurnaround(paymentDetail, turnaroundGeo->cityCode());
  if (farthestExceptTurnaroundFareBreakOrStopGeo)
  {
    if (isStopoverOrFareBreakBetween(turnaroundGeo,
                                     farthestExceptTurnaroundFareBreakOrStopGeo,
                                     properties,
                                     paymentDetail.taxName().taxPointTag()))
      return;
  }

  properties[turnaroundGeo->id()].isExtendedStopover.setTag(Tag::TurnaroundPoint);

  if (turnaroundGeo->isDeparture())
  {
    if (turnaroundGeo->isFirst())
      return;

    type::Index arrivalGeoId = turnaroundGeo->id() - 1;
    const Geo& arrivalGeo = _geoPath.geos()[arrivalGeoId];

    if (turnaroundGeo->cityCode() == arrivalGeo.cityCode())
      properties[arrivalGeoId].isExtendedStopover.setTag(Tag::TurnaroundPoint);
  }
  else
  {
    if (turnaroundGeo->isLast())
      return;

    type::Index departureGeoId = turnaroundGeo->id() + 1;
    const Geo& departureGeo = _geoPath.geos()[departureGeoId];

    if (turnaroundGeo->cityCode() == departureGeo.cityCode())
      properties[departureGeoId].isExtendedStopover.setTag(Tag::TurnaroundPoint);
  }
}

bool
isSameOrPairedPoint(const Geo& geoToCheck, const Geo& geo)
{
  if (geoToCheck.id() == geo.id())
    return true;

  if (geoToCheck.isDeparture() && geo.isArrival() && (geoToCheck.id() == geo.id() + 1))
    return true;

  if (geoToCheck.isArrival() && geo.isDeparture() && (geoToCheck.id() == geo.id() - 1))
    return true;

  return false;
}

bool
ConnectionsTagsApplicator::doubleOccurrence(type::Index geoId,
                                            TaxPointsProperties& properties) const
{
  const Geo& geoToCheck = _geoPath.geos()[geoId];

  for (type::Index id = 1; id < _geoPath.geos().size() - 1; id += 2)
  {
    const Geo& geo = _geoPath.geos()[id];
    if (isSameOrPairedPoint(geoToCheck, geo))
      continue;

    if (geoToCheck.cityCode() == geo.cityCode() && isFareBreakOrStop(properties[id]))
      return true;
  }

  return false;
}

void
ConnectionsTagsApplicator::applyFareBreakAsStopover(
    TaxPointsProperties& taxPointsProperties) const
{
  for (TaxPointProperties& taxPointProperties : taxPointsProperties)
  {
    if (taxPointProperties.isFareBreak)
      taxPointProperties.isExtendedStopover.setTag(Tag::FareBreak);
  }
}

void
ConnectionsTagsApplicator::applyFarthestFareBreakAsStopover(
    TaxPointsProperties& properties) const
{
  const Geo* farthestFareBreak = getFarthestFareBreak(properties);
  if (farthestFareBreak != nullptr)
    properties[farthestFareBreak->id()].isExtendedStopover.setTag(Tag::FurthestFareBreak);
  else
    return;

  const Geo* neighbour = farthestFareBreak->samePointDifferentTag();
  if (neighbour != nullptr)
    properties[neighbour->id()].isExtendedStopover.setTag(Tag::FurthestFareBreak);
  else
    return;
}

const Geo*
ConnectionsTagsApplicator::getFarthestFareBreak(const TaxPointsProperties& properties)
    const
{
  const std::vector<GeoIdMile>& geoIdMiles =
      _mileageService.getAllMilesFromStart(_geoPath, _flightUsages, _travelDate);

  for (const GeoIdMile& geoIdMile : geoIdMiles)
  {
    type::Index geoId = geoIdMile.first;
    if (properties[geoId].isFareBreak)
      return &_geoPath.geos()[geoId];
  }

  return nullptr;
}

const Geo*
ConnectionsTagsApplicator::getFarthestFareBreakOrStopoverExceptTurnaround(PaymentDetail& paymentDetail,
                                                                const type::CityCode& turnaroundCityCode) const
{
  const TaxPointsProperties& properties = paymentDetail.taxPointsProperties();
  const std::vector<GeoIdMile>& geoIdMiles =
      _mileageService.getAllMilesFromStart(_geoPath, _flightUsages, _travelDate);

  for (const GeoIdMile& geoIdMile : geoIdMiles)
  {
    type::Index geoId = geoIdMile.first;
    if (isFareBreakOrStop(properties[geoId]) &&
        _geoPath.geos()[geoId].cityCode() != turnaroundCityCode)
      return &_geoPath.geos()[geoId];
  }

  return nullptr;
}

} // namespace tax
