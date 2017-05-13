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
#include "Rules/TaxPointLoc1Rule.h"

#include "Rules/TaxPointLoc1Applicator.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

TaxPointLoc1Rule::TaxPointLoc1Rule(LocZone const& locZone, type::Vendor const& vendor)
  : _locZone(locZone), _vendor(vendor)
{
}

TaxPointLoc1Rule::~TaxPointLoc1Rule() {}

TaxPointLoc1Rule::ApplicatorType
TaxPointLoc1Rule::createApplicator(type::Index const& /*itinIndex*/,
                                   const Request& /*request*/,
                                   Services& services,
                                   RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services.locService());
}

std::string
TaxPointLoc1Rule::getDescription(Services&) const
{
  return std::string("LOC1 RESTRICTED TO ") + _locZone.toString();
}
}
