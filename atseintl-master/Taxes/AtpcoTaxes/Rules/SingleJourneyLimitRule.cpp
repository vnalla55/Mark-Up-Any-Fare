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
#include "SingleJourneyLimitApplicator.h"
#include "SingleJourneyLimitRule.h"

namespace tax
{

SingleJourneyLimitRule::SingleJourneyLimitRule() {}

SingleJourneyLimitRule::~SingleJourneyLimitRule() {}

SingleJourneyLimitRule::ApplicatorType
SingleJourneyLimitRule::createApplicator(const type::Index& itinIndex,
                                         const Request& request,
                                         Services& cache,
                                         RawPayments& rawPayments) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  const type::Index& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(this, request.geoPaths()[geoPathRefId], cache.locService(), rawPayments);
}

std::string
SingleJourneyLimitRule::getDescription(Services&) const
{
  return "TAX LIMITED PER SINGLE JOURNEY";
}
}
