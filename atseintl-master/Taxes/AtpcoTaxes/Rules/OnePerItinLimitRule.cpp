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

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/DummyApplicator.h"
#include "Rules/OnePerItinLimitRule.h"

namespace tax
{

OnePerItinLimitRule::OnePerItinLimitRule() {}

OnePerItinLimitRule::~OnePerItinLimitRule() {}

OnePerItinLimitRule::ApplicatorType
OnePerItinLimitRule::createApplicator(const type::Index& /*itinIndex*/,
                                      const Request& /*request*/,
                                      Services& /*services*/,
                                      RawPayments& /*rawPayments*/) const
{
  return DummyApplicator(*this, true);
}

std::string
OnePerItinLimitRule::getDescription(Services&) const
{
  return "TAX LIMITED TO ONE PER SINGLE ITINERARY";
}
}
