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
#include "Rules/TaxPointLoc2InternationalDomesticRule.h"
#include "Rules/TaxPointLoc2InternationalDomesticApplicator.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TaxPointLoc2InternationalDomesticRule::TaxPointLoc2InternationalDomesticRule(
    const type::IntlDomInd& intlDomInd, const type::TicketedPointTag& ticketedPointTag)
  : _intlDomInd(intlDomInd), _ticketedPointTag(ticketedPointTag)
{
}

TaxPointLoc2InternationalDomesticRule::~TaxPointLoc2InternationalDomesticRule() {}

TaxPointLoc2InternationalDomesticRule::ApplicatorType
TaxPointLoc2InternationalDomesticRule::createApplicator(const type::Index& /*itinIndex*/,
                                                        const Request& /*request*/,
                                                        Services& /*services*/,
                                                        RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this);
}

std::string
TaxPointLoc2InternationalDomesticRule::getDescription(Services&) const
{
  if (_intlDomInd == type::IntlDomInd::International)
    return "LOC2 MUST HAVE DIFFERENT NATION FROM LOC1";
  else if (_intlDomInd == type::IntlDomInd::Domestic)
    return "LOC2 MUST HAVE THE SAME NATION AS LOC1";
  else
    return "";
}
}
