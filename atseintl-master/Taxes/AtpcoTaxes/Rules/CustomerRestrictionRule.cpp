// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Rules/CustomerRestrictionApplicator.h"
#include "Rules/CustomerRestrictionRule.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{

CustomerRestrictionRule::CustomerRestrictionRule(const type::CarrierCode& carrier)
  : _carrierCode(carrier)
{
}

std::string
CustomerRestrictionRule::getDescription(Services&) const
{
  return "CUSTOMER RESTICTION RULE";
}

CustomerRestrictionRule::ApplicatorType
CustomerRestrictionRule::createApplicator(type::Index const& itinIndex,
                                const Request& request,
                                Services& services,
                                RawPayments& /*itinPayments*/) const
{
  type::Index posIndex = request.getItinByIndex(itinIndex).pointOfSaleRefId();

  return ApplicatorType(*this, services.customerService(),
    request.pointsOfSale()[posIndex].agentPcc());
}

} /* namespace tax */
