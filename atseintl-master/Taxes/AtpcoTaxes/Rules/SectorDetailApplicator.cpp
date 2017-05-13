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
#include "DataModel/Services/SectorDetail.h"
#include "DomainDataObjects/Fare.h"
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Rules/PaymentDetail.h"
#include "Rules/SectorDetailApplicator.h"
#include "Rules/SectorDetailRule.h"

namespace tax
{
SectorDetailApplicator::SectorDetailApplicator(const BusinessRule* businessRule,
                                               std::shared_ptr<const SectorDetail> sectorDetail,
                                               const type::SectorDetailApplTag& sectorDetailApplTag,
                                               const std::vector<FlightUsage>& flightUsages,
                                               const std::vector<Flight>& flights,
                                               const std::vector<Fare>& fares,
                                               const std::vector<FareUsage>& fareUsages,
                                               const GeoPathMapping& geoPathMapping,
                                               const GeoPath& geoPath)
  : BusinessRuleApplicator(businessRule),
    _sectorDetail(sectorDetail),
    _sectorDetailApplTag(sectorDetailApplTag),
    _flightUsages(flightUsages),
    _geoPath(geoPath),
    _matcher(flightUsages, flights, fares, fareUsages, geoPathMapping)
{
}

SectorDetailApplicator::~SectorDetailApplicator()
{
}

bool
SectorDetailApplicator::applyFirstInternationalSector() const
{
  bool match = false;

  type::Index geoId = 0;
  while (geoId < _geoPath.geos().size() - 1)
  {
    if (LocationUtil::isInternational(_geoPath.geos()[geoId].getNation(),
                                      _geoPath.geos()[geoId + 1].getNation()))
    {
      match = true;
      break;
    }
    geoId = geoId + 2;
  }
  if (!match)
    return false;

  type::Index flightId = geoId / 2;

  for (const SectorDetailEntry& entry : _sectorDetail->entries)
  {
    if (_matcher.matchSectorDetails(entry, flightId))
    {
      if (entry.applTag == type::SectorDetailAppl::Positive)
        return true;
      if (entry.applTag == type::SectorDetailAppl::Negative)
        return false;
    }
  }
  return false;
}

bool
SectorDetailApplicator::applyAnySectorLoc1Loc2(PaymentDetail& paymentDetail) const
{
  type::Index flightBeginId = paymentDetail.getTaxPointBegin().id() / 2;
  type::Index flightEndId = paymentDetail.getTaxPointEnd().id() / 2;

  if (flightBeginId > flightEndId)
    std::swap(flightBeginId, flightEndId);

  return applyAnySector(flightBeginId, flightEndId);
}

bool
SectorDetailApplicator::applyAnySectorOrginDestination() const
{
  return applyAnySector(0, _flightUsages.size() - 1);
}

bool
SectorDetailApplicator::applyAnySector(type::Index flightBeginId, type::Index flightEndId) const
{
  for (type::Index flightId = flightBeginId; flightId <= flightEndId; flightId++)
  {
    for (const SectorDetailEntry& entry : _sectorDetail->entries)
    {
      if (_matcher.matchSectorDetails(entry, flightId))
      {
        if (entry.applTag == type::SectorDetailAppl::Positive)
          return true;
        if (entry.applTag == type::SectorDetailAppl::Negative)
          continue;
      }
    }
  }
  // didn't return yet - no positive match
  return false;
}

bool
SectorDetailApplicator::applyAllSectorLoc1Loc2(PaymentDetail& paymentDetail) const
{
  type::Index flightBeginId = paymentDetail.getTaxPointBegin().id() / 2;
  type::Index flightEndId = paymentDetail.getTaxPointEnd().id() / 2;

  if (flightBeginId > flightEndId)
    std::swap(flightBeginId, flightEndId);

  return applyEverySector(flightBeginId, flightEndId);
}

bool
SectorDetailApplicator::applyEverySectorInItin() const
{
  return applyEverySector(0, _flightUsages.size() - 1);
}

bool
SectorDetailApplicator::applyEverySector(type::Index flightBeginId, type::Index flightEndId) const
{
  bool match;

  for (type::Index flightId = flightBeginId; flightId <= flightEndId; flightId++)
  {
    match = false;
    for (const SectorDetailEntry& entry : _sectorDetail->entries)
    {
      if (_matcher.matchSectorDetails(entry, flightId))
      {
        if (entry.applTag == type::SectorDetailAppl::Positive)
        {
          match = true;
          break;
        }
        if (entry.applTag == type::SectorDetailAppl::Negative)
          return false;
      }
    }
    if (!match)
      return false;
  }

  return true;
}

bool
SectorDetailApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_sectorDetail == nullptr)
    return false;

  if (_sectorDetailApplTag == type::SectorDetailApplTag::AnySectorLoc1Loc2)
    return applyAnySectorLoc1Loc2(paymentDetail);

  else if (_sectorDetailApplTag == type::SectorDetailApplTag::AnySectorOrginDestination)
    return applyAnySectorOrginDestination();

  else if (_sectorDetailApplTag == type::SectorDetailApplTag::FirstInternationalSector)
    return applyFirstInternationalSector();

  else if (_sectorDetailApplTag == type::SectorDetailApplTag::EverySectorInItin)
    return applyEverySectorInItin();

  else if (_sectorDetailApplTag == type::SectorDetailApplTag::AllSectorLoc1Loc2)
    return applyAllSectorLoc1Loc2(paymentDetail);

  else
    return false;
}

} // namespace tax
