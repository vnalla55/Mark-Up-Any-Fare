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

#include "Rules/ExemptTagRule.h"

#include "Rules/ExemptTagApplicator.h"

namespace tax
{

ExemptTagRule::ExemptTagRule() {}

ExemptTagRule::~ExemptTagRule() {}

ExemptTagRule::ApplicatorType
ExemptTagRule::createApplicator(type::Index const& /* itinIndex */,
                                const Request& /* request */,
                                Services& services,
                                RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, services);
}

std::string
ExemptTagRule::getDescription(Services&) const
{
  return "THIS SEQUENCE IS EXEMPTED";
}
}
