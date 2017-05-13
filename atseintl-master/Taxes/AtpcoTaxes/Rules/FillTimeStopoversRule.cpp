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
#include "Rules/FillTimeStopoversRule.h"
#include "Rules/FillTimeStopoversApplicatorFacade.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

FillTimeStopoversRule::FillTimeStopoversRule(tax::type::StopoverTimeTag number,
                                             const tax::type::StopoverTimeUnit& unit)
  : _number(number), _unit(unit)
{
}

FillTimeStopoversRule::~FillTimeStopoversRule() {}

FillTimeStopoversRule::ApplicatorType
FillTimeStopoversRule::createApplicator(const type::Index& itinIndex,
                                        const Request& request,
                                        Services& /*services*/,
                                        RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(this, request.getItinByIndex(itinIndex));
}

std::string
FillTimeStopoversRule::getDescription(Services&) const
{
  return "FILLER IS NOT A REGULAR RULE";
}

} // namespace tax
