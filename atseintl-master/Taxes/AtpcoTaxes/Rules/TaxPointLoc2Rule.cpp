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
#include <stdexcept>

#include "Rules/TaxPointLoc2Rule.h"
#include "Rules/TaxPointLoc2Applicator.h"

#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TaxPointLoc2Rule::TaxPointLoc2Rule(const LocZone& locZone,
                                   const type::Vendor& vendor)
  : _locZone(locZone),
    _vendor(vendor)
{
}

TaxPointLoc2Rule::~TaxPointLoc2Rule() {}

TaxPointLoc2Rule::ApplicatorType
TaxPointLoc2Rule::createApplicator(const type::Index& /*itinIndex*/,
                                   const Request& /*request*/,
                                   Services& services,
                                   RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this,
                        services.locService());
}

std::string
TaxPointLoc2Rule::getDescription(Services&) const
{
  return std::string("LOC2 RESTRICTED TO ") + _locZone.toString();
}
}
