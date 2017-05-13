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
#include "Rules/BlankLimitApplicator.h"
#include "Rules/BlankLimitRule.h"

namespace tax
{

BlankLimitRule::BlankLimitRule() {}

BlankLimitRule::~BlankLimitRule() {}

BlankLimitRule::ApplicatorType
BlankLimitRule::createApplicator(const type::Index& /*itinIndex*/,
                                 const Request& /*request*/,
                                 Services& /*cache*/,
                                 RawPayments& rawPayments) const
{
  return ApplicatorType(this, rawPayments);
}

std::string
BlankLimitRule::getDescription(Services&) const
{
  return "UNLIMITED TAX APPLICATION";
}
}
