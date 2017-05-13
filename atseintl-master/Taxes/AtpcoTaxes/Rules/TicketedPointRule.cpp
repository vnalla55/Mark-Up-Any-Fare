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

#include "Rules/TicketedPointRule.h"

#include "Rules/TicketedPointApplicator.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TicketedPointRule::TicketedPointRule() {}

TicketedPointRule::~TicketedPointRule() {}

TicketedPointRule::ApplicatorType
TicketedPointRule::createApplicator(type::Index const& itinIndex,
                                    const Request& request,
                                    Services& /* services */,
                                    RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(this, *itin.geoPath());
}

std::string
TicketedPointRule::getDescription(Services&) const
{
  return "APPLIES TO TICKETED POINTS ONLY";
}
}
