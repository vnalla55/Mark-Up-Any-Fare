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
#include "Rules/AlternateRefAkHiFactorsRule.h"
#include "Rules/AlternateRefAkHiFactorsApplicator.h"
#include "ServiceInterfaces/Services.h"
#include "DataModel/Common/Types.h"

namespace tax
{

AlternateRefAkHiFactorsRule::AlternateRefAkHiFactorsRule() {}

AlternateRefAkHiFactorsRule::~AlternateRefAkHiFactorsRule() {}

AlternateRefAkHiFactorsRule::ApplicatorType
AlternateRefAkHiFactorsRule::createApplicator(type::Index const& /*itinIndex*/,
                                              const Request& /*request*/,
                                              Services& services,
                                              RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services.aKHIFactorService(), services.locService());
}

std::string
AlternateRefAkHiFactorsRule::getDescription(Services&) const
{
  return std::string("ALTERNATE REF AK HI FACTORS RULE");
}

} // namespace tax
