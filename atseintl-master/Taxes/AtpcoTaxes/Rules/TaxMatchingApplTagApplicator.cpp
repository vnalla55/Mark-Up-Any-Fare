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
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/TaxMatchingApplTagApplicator.h"
#include "Rules/TaxMatchingApplTagRule.h"
#include "Rules/TurnaroundCalculator.h"
#include "ServiceInterfaces/MileageService.h"

namespace tax
{

class CurrencyService;

TaxMatchingApplTagApplicator::TaxMatchingApplTagApplicator(const TaxMatchingApplTagRule* rule,
                                                           const Itin& itin,
                                                           const MileageService& mileageService)
  : BusinessRuleApplicator(rule),
    _rule(rule),
    _itin(itin),
    _geoPath(*itin.geoPath()),
    _geoPathMapping(*itin.geoPathMapping()),
    _mileageService(mileageService)
{
}

TaxMatchingApplTagApplicator::~TaxMatchingApplTagApplicator() {}

bool
TaxMatchingApplTagApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (_rule->taxMatchingApplTag() == "01")
  {
    type::Index geoBeginId = paymentDetail.getTaxPointBegin().id();
    type::Index geoEndId = paymentDetail.getTaxPointEnd().id();

    bool bothSameNation = LocationUtil::isDomestic(_geoPath.geos()[geoBeginId].getNation(),
                                                   _geoPath.geos()[geoEndId].getNation());
    bool bothFareBreak =
        (paymentDetail.isFareBreak(geoBeginId) && paymentDetail.isFareBreak(geoEndId));

    return bothSameNation && bothFareBreak && isDomesticFare(geoBeginId, geoEndId);
  }
  else if (_rule->taxMatchingApplTag() == "02")
  {
    if (paymentDetail.taxName().taxPointTag() != type::TaxPointTag::Departure)
      return true;

    bool matchTicketedOnly =
        (paymentDetail.ticketedPointTag() == type::TicketedPointTag::MatchTicketedPointsOnly);

    const Geo& loc1 = paymentDetail.getTaxPointBegin();
    type::Index endId = _geoPath.geos().back().id(); // default value for one-way
    if (paymentDetail.roundTripOrOpenJaw()) // but if rt/oj, override it
    {
      TurnaroundCalculator turnaroundCalculator(_geoPath,
                                                _mileageService,
                                                paymentDetail.taxPointsProperties(),
                                                paymentDetail.specUS_RTOJLogic(),
                                                _rule->alternateTurnaroundDeterminationLogic());
      const Geo* turnaround = _itin.getTurnaround(turnaroundCalculator);

      if (turnaround == nullptr || turnaround->id() <= loc1.id())
        endId = _geoPath.geos().back().id();
      else
        endId = turnaround->id();
    }

    for (type::Index geoId = loc1.id() + 1; geoId <= endId; ++geoId)
    {
      const Geo& geo = _geoPath.geos()[geoId];
      if (matchTicketedOnly && geo.isUnticketed())
        continue;

      if (LocationUtil::isInternational(loc1.getNation(), geo.getNation()))
        return true;
    }

    return false;
  }

  // accept everything when Tag not implemented
  return true;
}

bool
TaxMatchingApplTagApplicator::isDomesticFare(type::Index start, type::Index end) const
{
  if (start > end)
  {
    std::swap(start, end);
  }

  for(Mapping const & mapping : _geoPathMapping.mappings())
  {
    if (mapping.maps().front().index() == start && mapping.maps().back().index() == end)
    {
      for(Map const & map : mapping.maps())
      {
        if (LocationUtil::isInternational(_geoPath.geos()[start].getNation(),
                                          _geoPath.geos()[map.index()].getNation()))
        {
          return false;
        }
      }
      return true;
    }
  }

  return false;
}
}
