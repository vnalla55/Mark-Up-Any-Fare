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

#include "Rules/ApplicationTag01Applicator.h"

#include "Common/LocationUtil.h"
#include "Common/Timestamp.h"
#include "DomainDataObjects/Flight.h"
#include "Rules/ApplicationTag01Rule.h"
#include "Rules/GeoUtils.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/LocService.h"

namespace tax
{

namespace {
  const type::Nation US("US");
  const LocZone AREA1{type::LocType::Area, "1"};
  const LocZone AREA2{type::LocType::Area, "2"};
  const LocZone AREA3{type::LocType::Area, "3"};

  // using identical logic for now, to enable easy change if ATPCO says otherwise
  bool isInUS(const Geo& geo) { return geo.getNation() == US; }
  bool isIn50USStates(const Geo& geo) { return geo.getNation() == US; }
}

ApplicationTag01Applicator::ApplicationTag01Applicator(const ApplicationTag01Rule& parent,
                                                   const Itin& itin,
                                                   const LocService& locService,
                                                   const MileageService& mileageService,
                                                   const type::Timestamp& ticketingDate)
  : BusinessRuleApplicator(&parent),
    _itin(itin),
    _locService(locService),
    _mileageService(mileageService),
    _applicationTagRule(parent),
    _ticketingDate(ticketingDate)
{
}

bool
ApplicationTag01Applicator::apply(PaymentDetail& paymentDetail) const
{
  if (matchesPreconditions(paymentDetail))
  {
    if (considerOW(_itin, paymentDetail))
    {
      paymentDetail.roundTripOrOpenJaw() = false;
      paymentDetail.specUS_RTOJLogic() = true;
      return true;
    }

    if (considerOJ(_itin, paymentDetail))
    {
      paymentDetail.roundTripOrOpenJaw() = true;
      paymentDetail.specUS_RTOJLogic() = true;
      return true;
    }
  }

  // Neither OW nor OJ - don't change anything
  return true;
}

bool
ApplicationTag01Applicator::considerOW(const Itin& itin, PaymentDetail& paymentDetail) const
{
  return considerOWDistanceValidation(itin, paymentDetail) || considerOWIataSegments(itin);
}

bool
ApplicationTag01Applicator::considerOWDistanceValidation(Itin const& itin,
                                                       PaymentDetail& paymentDetail) const
{
  auto getDistance = [&](type::Index first, type::Index last)
  {
    return _mileageService.getDistance(*itin.geoPath(),
                                       itin.flightUsages(),
                                       first,
                                       last,
                                       _ticketingDate);
  };

  // A Journey is considered a one way journey IF:
  // Journey Origin country code != Journey destination country code
  if (LocationUtil::isDomestic(itin.geoPath()->getOriginNation(), itin.geoPath()->getDestinationNation()))
    return false;

  const type::Index lastGeoId = itin.geoPath()->geos().size() - 1;

  //AND:  The distance in TPMs between the journey origin and journey destination is
  const type::Miles distanceOriginDestination = getDistance(0, lastGeoId);

  // Greater than the distance between the journey origin and the first stopover
  // point (12hr) in the US 50 states OR
  for (type::Index i = 1; i < lastGeoId; ++i)
  {
    const Geo& geo = itin.geoPath()->geos()[i];
    if (isIn50USStates(geo) && paymentDetail.isUSStopover(i))
    {
      type::Miles distanceOriginFirstStop = getDistance(0, i);

      if (distanceOriginDestination > distanceOriginFirstStop)
        return true;

      break;
    }
  }

  // Greater than the distance between the last stopover point in the US 50 states
  // and the journey destination
  for (type::Index i = lastGeoId - 1; i != 0; --i)
  {
    const Geo& geo = itin.geoPath()->geos()[i];
    if (isIn50USStates(geo) && paymentDetail.isUSStopover(i))
    {
      type::Miles distanceLastStopDestination = getDistance(i, lastGeoId);

      if (distanceOriginDestination > distanceLastStopDestination)
        return true;

      break;
    }
  }

  return false;
}

bool
ApplicationTag01Applicator::considerOWIataSegments(Itin const& itin)
    const
{
  const GeoPath& geoPath = *itin.geoPath();
  bool hasIata1To2 = false;
  bool hasIata1To3 = false;

  type::GlobalDirection iata1To2Dir = type::GlobalDirection::NO_DIR;
  type::GlobalDirection iata1To3Dir = type::GlobalDirection::NO_DIR;

  for (type::Index geoId = 0; geoId < geoPath.geos().size() - 1; geoId += 2)
  {
    const Geo& geo = geoPath.geos()[geoId];
    const Geo& geoPlus = geoPath.geos()[geoId + 1];

    if (sectorBetweenAreas(geo, geoPlus, AREA1, AREA2) &&
        geo.getFlight(itin.flightUsages())->isGroundTransport() == false)
    {
      if (hasIata1To2) // found second sector from area 1 to 2
        return false;

      hasIata1To2 = true;
      const std::vector<MileageService::GeoIdGlobDir> gloDir =
          _mileageService.getGlobDir(*itin.geoPath(), itin.flightUsages(),
                                     geoId, geoId + 1, _ticketingDate);
      if (gloDir.size() > 0)
        iata1To2Dir = gloDir[gloDir.size() - 1].second;
    };

    if (sectorBetweenAreas(geo, geoPlus, AREA1, AREA3) &&
        geo.getFlight(itin.flightUsages())->isGroundTransport() == false)
    {
      if (hasIata1To3) // found second sector from area 1 to 3
        return false;

      hasIata1To3 = true;
      const std::vector<MileageService::GeoIdGlobDir> gloDir =
          _mileageService.getGlobDir(*itin.geoPath(), itin.flightUsages(),
                                     geoId, geoId + 1, _ticketingDate);
      if (gloDir.size() > 0)
        iata1To3Dir = gloDir[gloDir.size() - 1].second;
    };
  }

  if (iata1To2Dir == type::GlobalDirection::NO_DIR ||
      iata1To3Dir == type::GlobalDirection::NO_DIR)
    return false;

  if (hasIata1To2 && hasIata1To3 && iata1To2Dir != iata1To3Dir)
    return true;
  else
    return false;
}

bool
ApplicationTag01Applicator::considerOJ(const Itin& itin, PaymentDetail& paymentDetail) const
{
  // All other journeys in which origin country code != journey destination country code
  if (LocationUtil::isInternational(itin.geoPath()->getOriginNation(), itin.geoPath()->getDestinationNation()))
    return true;

  // AND/OR that contain an interior surface sector, one or both of whose end points are in the US,
  // are considered open jaw journeys
  const GeoPath& geoPath = *itin.geoPath();
  for (type::Index geoId = 0; geoId < geoPath.geos().size() - 1; ++geoId)
  {
    const Geo& geo = geoPath.geos()[geoId];
    const Geo& geo2 = geoPath.geos()[geoId + 1];
    bool isSurfaceSegment = (geoId % 2 == 1) &&
                            paymentDetail.getMutableTaxPointsProperties()[geoId].isSurface() &&
                            paymentDetail.getMutableTaxPointsProperties()[geoId + 1].isSurface();
    if ((isSurfaceSegment || geo.getFlight(itin.flightUsages())->isGroundTransport()) &&
        (isInUS(geo) || isInUS(geo2)))
      return true;
  }

  return false;
}

bool
ApplicationTag01Applicator::matchesPreconditions(PaymentDetail& paymentDetail) const
{
  // Apply only when: Point of sale is not in US
  Geo pointOfSale;
  pointOfSale.loc().code() = _itin.pointOfSale()->loc();
  pointOfSale.loc().nation() = _locService.getNation(_itin.pointOfSale()->loc());
  if (isInUS(pointOfSale))
    return false;

  // Apply only when: Journey origin is not in the 50 US states and journey destination is not
  // in the 50 US states
  if (isIn50USStates(_itin.geoPath()->geos().front()) ||
      isIn50USStates(_itin.geoPath()->geos().back()))
    return false;

  // Apply only when: Journey origin and journey destination are not both in buffer zone (that is,
  // either journey origin or journey destination may be in the buffer zone, but not both)
  if (originAndDestinationBothInBufferZone())
    return false;

  if (!hasStopoverIn50USStates(paymentDetail))
    return false;

  return true;
}

bool
ApplicationTag01Applicator::originAndDestinationBothInBufferZone() const
{
  return _itin.geoPath()->geos().front().loc().inBufferZone() &&
         _itin.geoPath()->geos().back().loc().inBufferZone();
}

bool
ApplicationTag01Applicator::hasStopoverIn50USStates(PaymentDetail& paymentDetail) const
{
  // Tax Type = 005 or 006: Journey includes a ticketed arrival point in the 50 US states followed
  // directly by a scheduled departure 12 or more hours after scheduled arrival (arrival city and
  // departure city need not be the same) and/or
  // Journey includes a ticketed departure point in the 50 US states preceded directly by
  // a scheduled arrival 12 or more hours before scheduled departure (arrival city and departure
  // city need not be the same) isUSTimeStopover
  const TaxPointsProperties& properties{paymentDetail.taxPointsProperties()};

  for (const Geo& geo : _itin.geoPath()->geos())
  {
    if (isIn50USStates(geo) &&
        !geo.isUnticketed() &&
        properties[geo.id()].isUSTimeStopover == true)
    {
      return true;
    }
  }

  return false;
}

bool ApplicationTag01Applicator::sectorBetweenAreas(const Geo& geo1,
                                                    const Geo& geo2,
                                                    const LocZone& area1,
                                                    const LocZone& area2) const
{
  const type::Vendor& vendor = _applicationTagRule.getVendor();
  return ((_locService.isInLoc(geo1.locCode(), area1, vendor) &&
           _locService.isInLoc(geo2.locCode(), area2, vendor)) ||
          (_locService.isInLoc(geo2.locCode(), area1, vendor) &&
           _locService.isInLoc(geo1.locCode(), area2, vendor)));
}

} // namespace tax
