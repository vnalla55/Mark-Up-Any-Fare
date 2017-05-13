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
#include "Rules/TaxPointLoc3AsNextStopoverRule.h"
#include "Rules/TaxPointLoc3AsNextStopoverApplicator.h"
#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TaxPointLoc3AsNextStopoverRule::TaxPointLoc3AsNextStopoverRule(const LocZone& locZone3,
                                                               type::Vendor vendor)
  : _locZone3(locZone3), _vendor(vendor)
{
}

TaxPointLoc3AsNextStopoverRule::~TaxPointLoc3AsNextStopoverRule() {}

TaxPointLoc3AsNextStopoverRule::ApplicatorType
TaxPointLoc3AsNextStopoverRule::createApplicator(const type::Index& itinIndex,
                                                 const Request& request,
                                                 Services& services,
                                                 RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(*this,
                        request.geoPaths()[itin.geoPathRefId()],
                        services.locService());
}

std::string
TaxPointLoc3AsNextStopoverRule::getDescription(Services&) const
{
  return "NEXT STOPOVER AFTER LOC1 MUST MATCH TO " + _locZone3.toString();
}
}
