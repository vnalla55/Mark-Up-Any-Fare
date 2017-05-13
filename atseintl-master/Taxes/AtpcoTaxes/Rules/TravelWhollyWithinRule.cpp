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

#include "Rules/TravelWhollyWithinRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/TravelWhollyWithinApplicator.h"

namespace tax
{

TravelWhollyWithinRule::TravelWhollyWithinRule(type::TicketedPointTag const& ticketedPoint,
                                               LocZone const& zone,
                                               type::Vendor const& vendor)
  : _ticketedPoint(ticketedPoint), _locZone(zone), _vendor(vendor)
{
}

TravelWhollyWithinRule::~TravelWhollyWithinRule() {}

TravelWhollyWithinRule::ApplicatorType
TravelWhollyWithinRule::createApplicator(type::Index const& itinIndex,
                                         const Request& request,
                                         Services& services,
                                         RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(*this, request.geoPaths()[geoPathRefId], services.locService());
}

std::string
TravelWhollyWithinRule::getDescription(Services&) const
{
  return std::string("THE JOURNEY GEO SPEC TRAVEL WHOLLY WITHIN ") + _locZone.toString();
}

} // namespace tax
