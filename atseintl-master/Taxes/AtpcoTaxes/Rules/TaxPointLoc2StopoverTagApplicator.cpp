// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Rules/TaxPointLoc2StopoverTagApplicator.h"

#include "Common/LocationUtil.h"
#include "Common/OCUtil.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/PaymentDetail.h"
#include "Rules/TaxPointFinder.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"
#include "Rules/TimeStopoverChecker.h"
#include "Rules/TurnaroundCalculator.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/MileageService.h"
#include "ServiceInterfaces/Services.h"

#include <cassert>

namespace tax
{

namespace
{
static const type::TaxProcessingApplTag taxProcessingApplTag07_findFareForTaxC9{"07"};

static const type::TaxMatchingApplTag taxMatchingApplTag03_furthestInDirOfTravel{"03"};
static const type::TaxMatchingApplTag taxMatchingApplTag05_furthestIntBeforeDomStop{"05"};
static const type::TaxMatchingApplTag taxMatchingApplTag06_fareBreakMustBeStop{"06"};
static const type::TaxMatchingApplTag taxMatchingApplTag07_lastDomesticPoint{"07"};

bool isPointBeforeTurnaround(type::Index pointId,
                             type::Index turnaroundId,
                             int direction)
{
  return direction == 1 ? pointId < turnaroundId : pointId > turnaroundId;
}

type::Index getFirstDomStopoverOrDestination(const PaymentDetail& paymentDetail,
                                             const std::vector<Geo>& geos,
                                             type::Index loc1Id)
{
  const type::Nation& loc1Nation = geos[loc1Id].getNation();

  for (type::Index i = loc1Id + 1; i < geos.size(); ++i)
  {
    if (LocationUtil::isDomestic(loc1Nation, geos[i].getNation()) &&
        paymentDetail.isStopover(i))
    {
      return i - 1;
    }
  }

  return geos.back().id();
}

}

TaxPointLoc2StopoverTagApplicator::TaxPointLoc2StopoverTagApplicator(
    const TaxPointLoc2StopoverTagRule& rule,
    const Itin& itin,
    const Services& services)
  : BusinessRuleApplicator(&rule),
    _rule(rule),
    _geoPath(*itin.geoPath()),
    _itin(itin),
    _services(services),
    _taxPointTagToMatch(type::TaxPointTag::Arrival),
    failApplication(false)
{
  if (_rule.getTaxPointTag() == type::TaxPointTag::Arrival)
  {
    _direction = -1;
    _taxPointTagToMatch = type::TaxPointTag::Departure;
  }
  else if (_rule.getTaxPointTag() == type::TaxPointTag::Departure)
  {
    _direction = 1;
    _taxPointTagToMatch = type::TaxPointTag::Arrival;
  }
  else
  {
    failApplication = true;
  }
}

TaxPointLoc2StopoverTagApplicator::~TaxPointLoc2StopoverTagApplicator() {}

bool
TaxPointLoc2StopoverTagApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (BOOST_UNLIKELY(failApplication))
  {
    paymentDetail.applicatorFailMessage() = "TaxPointTag should only be Departure or Arrival here";
    return false;
  }

  // Loc2 should be first point after/before Loc1, this is default when constructing
  if (_rule.getLoc2StopoverTag() == type::Loc2StopoverTag::Blank)
    return true;

  bool result = false;
  bool matchTicketedOnly =
      (paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly);

  result = applyForYqYr(paymentDetail,
                        paymentDetail.taxPointsProperties(),
                        matchTicketedOnly,
                        paymentDetail.getMutableYqYrDetails()) || result;
  result = applyForItinerary(paymentDetail, matchTicketedOnly) || result;

  return result;
}

bool TaxPointLoc2StopoverTagApplicator::applyForItinerary(PaymentDetail& paymentDetail,
                                                          bool matchTicketedOnly) const
{
  const type::Index& loc1Id = paymentDetail.getTaxPointLoc1().id();
  type::Index furthestIndex = findFurthestIndex(paymentDetail, matchTicketedOnly);
  bool fareBreakMustAlsoBeStopover =
      _rule.getTaxMatchingApplTag() == taxMatchingApplTag06_fareBreakMustBeStop;

  // LOOP FOR OC
  for (OptionalService& optionalService : paymentDetail.optionalServiceItems())
  {
    if (OCUtil::isOCSegmentRelated(optionalService.type()) && !optionalService.isFailed())
    {
      const Geo& geo = _geoPath.geos()[optionalService.getTaxPointEnd().id()];

      if (matchTicketedOnly && geo.isUnticketed())
      {
        optionalService.setFailedRule(&_rule);
      }

      // TODO: Verify logic to be used - is end point of OC always stopover and farebreak?
      // Logic for furthestPoint in OC does not work, but no tax uses such restriction
      if (TaxPointUtil(paymentDetail).checkIfMatchesStopoverTagCondition(
              _rule.getLoc2StopoverTag(),
              optionalService.getTaxPointEnd().id(),
              furthestIndex))
      {
        paymentDetail.loc2StopoverTag() = _rule.getLoc2StopoverTag();
        return true;
      }
    }
  }

  TaxPointUtil util(paymentDetail);
  for (type::Index i = loc1Id + _direction; i < _geoPath.geos().size(); i += 2 * _direction)
  {
    const Geo& geo = _geoPath.geos()[i];

    if (matchTicketedOnly && geo.isUnticketed())
      continue;

    if (util.checkIfMatchesStopoverTagCondition(_rule.getLoc2StopoverTag(),
                                                i,
                                                furthestIndex))
    {
      if (_rule.getLoc2StopoverTag() == type::Loc2StopoverTag::FareBreak &&
          fareBreakMustAlsoBeStopover &&
          !paymentDetail.isStopover(i))
        break;

      paymentDetail.setTaxPointLoc2(&geo);
      paymentDetail.setTaxPointEnd(geo);
      paymentDetail.loc2StopoverTag() = _rule.getLoc2StopoverTag();
      return true;
    }
  }

  paymentDetail.setFailedRule(&_rule);
  return !paymentDetail.areAllOptionalServicesFailed();
}

bool
TaxPointLoc2StopoverTagApplicator::applyForYqYr(PaymentDetail& paymentDetail,
                                                const TaxPointsProperties& properties,
                                                bool matchTicketedOnly,
                                                TaxableYqYrs& taxableYqYrs) const
{
  const type::Index loc1Id = paymentDetail.getTaxPointLoc1().id();
  bool fareBreakMustAlsoBeStopover =
      _rule.getTaxMatchingApplTag() == taxMatchingApplTag06_fareBreakMustBeStop;
  // Should find furthestIndex in each YqYr component, but there are no such sequences coded
  // so fixing is not high priority
  type::Index furthestIndex = findFurthestIndex(paymentDetail, matchTicketedOnly);

  std::vector<const Geo*> loc2 = TaxPointFinder(
      loc1Id, furthestIndex, properties, matchTicketedOnly, _direction, _geoPath.geos())
          .find(_rule.getLoc2StopoverTag(),
                fareBreakMustAlsoBeStopover,
                taxableYqYrs._ranges);
  assert(loc2.size() == taxableYqYrs._subject.size());

  for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
  {
    if (taxableYqYrs.isFailedRule(i))
     continue;

    if (!loc2[i])
    {
      taxableYqYrs.setFailedRule(i, _rule);
      continue;
    }

    taxableYqYrs._data[i]._taxPointLoc2 = loc2[i];
    taxableYqYrs._data[i]._taxPointEnd = loc2[i];
  }
  return !taxableYqYrs.areAllFailed();
}

type::Index
TaxPointLoc2StopoverTagApplicator::findFurthestIndex(const PaymentDetail& paymentDetail,
                                                     bool matchTicketedOnly) const
{
  if (_rule.getLoc2StopoverTag() == type::Loc2StopoverTag::Furthest)
  {
    if (_rule.getTaxMatchingApplTag() == taxMatchingApplTag03_furthestInDirOfTravel)
    {
      return findFurthestPointInDirection(paymentDetail);
    }
    else if (_rule.getTaxMatchingApplTag() == taxMatchingApplTag05_furthestIntBeforeDomStop &&
             paymentDetail.taxName().taxCode() == "OY" &&
             paymentDetail.taxName().taxPointTag() == type::TaxPointTag::Departure)
    {
      return findFurthestIntPointBeforeDomStop(paymentDetail, matchTicketedOnly);
    }
    else if (_rule.getTaxMatchingApplTag() == taxMatchingApplTag07_lastDomesticPoint &&
             _rule.getTaxProcessingApplTag() == taxProcessingApplTag07_findFareForTaxC9 &&
             paymentDetail.taxName().taxCode() == "C9" &&
             paymentDetail.taxName().taxPointTag() == type::TaxPointTag::Departure &&
             paymentDetail.taxableUnits().hasTag(type::TaxableUnit::Itinerary) &&
             LocationUtil::doTaxLoc1AndLoc2NationMatch(
                 _rule.getTaxPointLoc1Zone(), _rule.getTaxPointLoc2Zone()))
    {
      return findLastDomPointBeforeInt(paymentDetail, matchTicketedOnly);
    }
    else
    {
      return findFurthestPointInFareComponent(paymentDetail, matchTicketedOnly);
    }
  }

  return 0;
}

type::Index
TaxPointLoc2StopoverTagApplicator::findLastDomPointBeforeInt(
    const PaymentDetail& paymentDetail,
    bool matchTicketedOnly) const
{
  SpecificTimeStopoverChecker check48HoursStopovers(48*60);
  std::vector<bool> stopovers48hours =
      LocationUtil::findStopovers(_geoPath.geos(), _itin.flightUsages(), check48HoursStopovers);

  const type::Index& loc1Id = paymentDetail.getTaxPointLoc1().id();
  const type::Nation& loc1Nation = paymentDetail.getTaxPointLoc1().getNation();

  bool foundDom48Stop = stopovers48hours[loc1Id];
  type::Index loc2Id = 0;
  for (type::Index i = loc1Id + 1; i < _geoPath.geos().size(); ++i)
  {
    const Geo& geo = _geoPath.geos()[i];

    if (matchTicketedOnly && geo.isUnticketed())
      continue;

    if (LocationUtil::isInternational(loc1Nation, geo.getNation()))
      break;

    foundDom48Stop = stopovers48hours[i] || foundDom48Stop;

    if (geo.isArrival())
      loc2Id = i;
  }

  return (foundDom48Stop && loc2Id > 0) ? loc2Id : 0;
}

type::Index
TaxPointLoc2StopoverTagApplicator::findFurthestIntPointBeforeDomStop(
    const PaymentDetail& paymentDetail,
    bool matchTicketedOnly) const
{
  const type::Index& loc1Id = paymentDetail.getTaxPointLoc1().id();
  const type::Nation& loc1Nation = paymentDetail.getTaxPointLoc1().getNation();
  const type::Index lastId = getFirstDomStopoverOrDestination(paymentDetail, _geoPath.geos(), loc1Id);

  const auto miles = _services.mileageService().getMiles(_geoPath,
                                                         _itin.flightUsages(),
                                                         loc1Id,
                                                         lastId,
                                                         type::Timestamp(_itin.travelOriginDate()));

  for (const auto& geoIdMile : miles)
  {
    const Geo& geo = _geoPath.geos()[geoIdMile.first];

    if (geo.loc().tag() == _taxPointTagToMatch &&
        LocationUtil::isInternational(loc1Nation, geo.getNation()) &&
        (!matchTicketedOnly || (!geo.isUnticketed())))
      return geoIdMile.first;
  }

  return loc1Id;
}

type::Index
TaxPointLoc2StopoverTagApplicator::findFurthestPointInFareComponent(
    const PaymentDetail& paymentDetail,
    bool matchTicketedOnly) const
{
  const type::Index& loc1Id = paymentDetail.getTaxPointLoc1().id();
  const type::Index& fareBreakId =
      TaxPointUtil(paymentDetail).findNearestFareBreak(_direction, _geoPath.geos().size());

  const auto miles = _services.mileageService().getMiles(_geoPath,
                                                         _itin.flightUsages(),
                                                         loc1Id,
                                                         fareBreakId,
                                                         type::Timestamp(_itin.travelOriginDate()));

  for(const MileageService::GeoIdMile& geoIdMile : miles)
  {
    const type::Index& id = geoIdMile.first;
    const Geo& geo = _geoPath.geos()[id];

    if ((geo.loc().tag() == _taxPointTagToMatch) &&
        (!matchTicketedOnly || (!geo.isUnticketed())))
    {
      return id;
    }
  }

  return loc1Id;
}

type::Index
TaxPointLoc2StopoverTagApplicator::findFurthestPointInDirection(
    const PaymentDetail& paymentDetail) const
{
  type::Index loc1Id = paymentDetail.getTaxPointLoc1().id();
  type::Index lastLocId;
  if (paymentDetail.roundTripOrOpenJaw())
  {
    TurnaroundCalculator turnaroundCalculator(_geoPath,
                                              _services.mileageService(),
                                              paymentDetail.taxPointsProperties(),
                                              paymentDetail.specUS_RTOJLogic(),
                                              _rule.hasAlternateTurnaroundDeterminationLogic());

    const Geo* turnaround = _itin.getTurnaround(turnaroundCalculator);
    if (turnaround && isPointBeforeTurnaround(loc1Id, turnaround->id(), _direction))
    {
      // outbound direction
      lastLocId = turnaround->id();
    }
    else
    {
      // inbound direction
      lastLocId = _direction == 1 ? _geoPath.geos().size() - 1 : 0;
    }
  }
  else // one way journey
  {
    lastLocId = _direction == 1 ? _geoPath.geos().size() - 1 : 0;
  }

  const std::vector<MileageService::GeoIdMile> miles =
      _services.mileageService().getMiles(_geoPath,
                                          _itin.flightUsages(),
                                          loc1Id,
                                          lastLocId,
                                          type::Timestamp(_itin.travelOriginDate()));

  auto found = std::find_if(miles.begin(),
                            miles.end(),
                            [this](const MileageService::GeoIdMile& mile)
                            {
                              return _geoPath.geos()[mile.first].loc().tag() == _taxPointTagToMatch;
                            });

  return found != miles.end() ? found->first : loc1Id;
}

} // namespace tax
