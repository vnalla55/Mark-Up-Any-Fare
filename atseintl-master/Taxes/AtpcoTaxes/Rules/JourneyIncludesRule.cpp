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

#include "Rules/JourneyIncludesRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/JourneyIncludesApplicator.h"

namespace tax
{

JourneyIncludesRule::JourneyIncludesRule(type::TicketedPointTag const& ticketedPoint,
                                         LocZone const& zone,
                                         type::Vendor const& vendor,
                                         bool mustBeStopAndNotOriginDestination)
  : _ticketedPoint(ticketedPoint),
    _locZone(zone),
    _vendor(vendor),
    _mustBeStopAndNotOriginDestination(mustBeStopAndNotOriginDestination)
{
}

JourneyIncludesRule::~JourneyIncludesRule() {}

JourneyIncludesRule::ApplicatorType
JourneyIncludesRule::createApplicator(type::Index const& itinIndex,
                                      const Request& request,
                                      Services& services,
                                      RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(*this, request.geoPaths()[geoPathRefId], services.locService());
}

std::string
JourneyIncludesRule::getDescription(Services&) const
{
  return std::string("THE JOURNEY GEO SPEC TRAVEL INCLUDES ")
      + _locZone.toString()
      + ((_mustBeStopAndNotOriginDestination)
          ? " WHICH MUST BE STOPOVER AND NOT AN ORIGIN/DESTINATION"
          : "");
}
}
