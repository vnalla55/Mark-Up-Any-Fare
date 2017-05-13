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

#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/LocService.h"
#include "Rules/JourneyIncludesRule.h"
#include "Rules/JourneyIncludesApplicator.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

JourneyIncludesApplicator::JourneyIncludesApplicator(JourneyIncludesRule const& rule,
                                                     GeoPath const& geoPath,
                                                     LocService const& locService)
  : BusinessRuleApplicator(&rule),
    _journeyIncludesRule(rule),
    _geoPath(geoPath),
    _locService(locService)
{
}

JourneyIncludesApplicator::~JourneyIncludesApplicator() {}

bool
JourneyIncludesApplicator::apply(PaymentDetail& paymentDetail) const
{
  if (!_journeyIncludesRule.getMustBeStopAndNotOriginDestination())
  {
    for(const Geo & geo : _geoPath.geos())
    {
      if (doesJourneyTicketedPointTagMatch(geo) && doesJourneyInclude(geo))
        return true;
    }
  }
  else // TaxMatchingApplTag08 - to match, must be a stopover, excluding origin/destination
  {
    for (type::Index i = 1; i < _geoPath.geos().size() - 1; ++i)
    {
      const Geo& geo = _geoPath.geos()[i];
      if (geo.isUnticketed()) // stopovers are ticketed by definition
        continue;

      if (paymentDetail.isStopover(i) && doesJourneyInclude(geo))
        return true;
    }
  }

  return false;
}

bool
JourneyIncludesApplicator::doesJourneyTicketedPointTagMatch(const Geo& geo) const
{
  return !(_journeyIncludesRule.getTicketedPointTag() ==
               type::TicketedPointTag::MatchTicketedPointsOnly &&
           geo.unticketedTransfer() == type::UnticketedTransfer::Yes);
}

bool
JourneyIncludesApplicator::doesJourneyInclude(const Geo& geo) const
{
  return _locService.isInLoc(
      geo.loc().code(), _journeyIncludesRule.getLocZone(), _journeyIncludesRule.getVendor());
}

} // namespace tax
