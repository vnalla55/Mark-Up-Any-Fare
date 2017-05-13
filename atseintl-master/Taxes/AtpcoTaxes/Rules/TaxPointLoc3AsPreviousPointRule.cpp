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

#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/TaxPointLoc3AsPreviousPointApplicator.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"

namespace tax
{

TaxPointLoc3AsPreviousPointRule::TaxPointLoc3AsPreviousPointRule(const LocZone& locZone,
                                                                 const type::Vendor& vendor)
  : _locZone(locZone), _vendor(vendor)
{
}

TaxPointLoc3AsPreviousPointRule::~TaxPointLoc3AsPreviousPointRule() {}

TaxPointLoc3AsPreviousPointRule::ApplicatorType
TaxPointLoc3AsPreviousPointRule::createApplicator(const type::Index& /*itinIndex*/,
                                                  const Request& /*request*/,
                                                  Services& services,
                                                  RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, _locZone, _vendor, services.locService());
}

std::string
TaxPointLoc3AsPreviousPointRule::getDescription(Services&) const
{
  return "POINT BEFORE LOC1 MUST MATCH TO " + _locZone.toString();
}
}
