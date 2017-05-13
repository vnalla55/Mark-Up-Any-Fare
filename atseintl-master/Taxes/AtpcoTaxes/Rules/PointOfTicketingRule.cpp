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

#include "Rules/PointOfTicketingRule.h"
#include "Rules/PointOfTicketingApplicator.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

PointOfTicketingRule::PointOfTicketingRule(const LocZone& locZone, const type::Vendor& vendor)
  : _locZone(locZone), _vendor(vendor)
{
}

PointOfTicketingRule::~PointOfTicketingRule() {}

PointOfTicketingRule::ApplicatorType
PointOfTicketingRule::createApplicator(const type::Index& /*itinIndex*/,
                                       const Request& request,
                                       Services& services,
                                       RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, request.ticketingOptions().ticketingPoint(), services.locService());
}

std::string
PointOfTicketingRule::getDescription(Services&) const
{
  return std::string("POINT OF TICKETING RESTRICTED TO ") + _locZone.toString();
}
}
