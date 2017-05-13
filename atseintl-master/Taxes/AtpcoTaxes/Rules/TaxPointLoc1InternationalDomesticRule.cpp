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
#include "Rules/TaxPointLoc1InternationalDomesticRule.h"
#include "Rules/TaxPointLoc1InternationalDomesticApplicator.h"
#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TaxPointLoc1InternationalDomesticRule::TaxPointLoc1InternationalDomesticRule(
    type::AdjacentIntlDomInd const& adjacentIntlDomInd,
    type::TicketedPointTag const& ticketedPointTag)
  : _adjacentIntlDomInd(adjacentIntlDomInd), _ticketedPointTag(ticketedPointTag)
{
}

TaxPointLoc1InternationalDomesticRule::~TaxPointLoc1InternationalDomesticRule() {}

TaxPointLoc1InternationalDomesticRule::ApplicatorType
TaxPointLoc1InternationalDomesticRule::createApplicator(type::Index const& itinIndex,
                                                        const Request& request,
                                                        Services& services,
                                                        RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(*this, request.geoPaths()[geoPathRefId], services.locService());
}

std::string
TaxPointLoc1InternationalDomesticRule::getDescription(Services&) const
{
  if (_adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverInternational)
    return "LOC1 ADJACENT STOPOVER MUST BE INTERNATIONAL";
  else if (_adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentStopoverDomestic)
    return "LOC1 ADJACENT STOPOVER MUST BE DOMESTIC";
  else if (_adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentInternational)
    return "LOC1 ADJACENT POINT MUST BE INTERNATIONAL";
  else if (_adjacentIntlDomInd == type::AdjacentIntlDomInd::AdjacentDomestic)
    return "LOC1 ADJACENT POINT MUST BE DOMESTIC";
  else
    return "";
}

} // namespace tax
