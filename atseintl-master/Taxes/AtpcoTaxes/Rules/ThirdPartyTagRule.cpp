// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/SafeEnumToString.h"
#include "Rules/ThirdPartyTagRule.h"
#include "Rules/ThirdPartyTagApplicator.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

ThirdPartyTagRule::ThirdPartyTagRule(const type::PaidBy3rdPartyTag& paidBy3rdPartyTag)
  : _paidBy3rdPartyTag(paidBy3rdPartyTag)
{
}

ThirdPartyTagRule::~ThirdPartyTagRule() {}

ThirdPartyTagRule::ApplicatorType
ThirdPartyTagRule::createApplicator(const type::Index& /*itinIndex*/,
                                    const Request& request,
                                    Services& /*services*/,
                                    RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, _paidBy3rdPartyTag, request.ticketingOptions().formOfPayment());
}

std::string
ThirdPartyTagRule::getDescription(Services&) const
{
  return "FORM OF PAYMENT MUST MATCH TO PADIBY3RDPARTYTAG: " +
         boost::lexical_cast<std::string>(_paidBy3rdPartyTag);
}
}
