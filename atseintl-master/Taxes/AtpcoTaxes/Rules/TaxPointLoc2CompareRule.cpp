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
#include "Rules/TaxPointLoc2CompareRule.h"
#include "Rules/TaxPointLoc2CompareApplicator.h"
#include "ServiceInterfaces/Services.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"

namespace tax
{

TaxPointLoc2CompareRule::TaxPointLoc2CompareRule(
    const type::TaxPointLoc2Compare& taxPointLoc2Compare,
    const type::TicketedPointTag& ticketedPointTag)
  : _taxPointLoc2Compare(taxPointLoc2Compare), _ticketedPointTag(ticketedPointTag)
{
}

TaxPointLoc2CompareRule::~TaxPointLoc2CompareRule() {}

TaxPointLoc2CompareRule::ApplicatorType
TaxPointLoc2CompareRule::createApplicator(const type::Index& /*itinIndex*/,
                                          const Request& /*request*/,
                                          Services& /*services*/,
                                          RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this);
}

std::string
TaxPointLoc2CompareRule::getDescription(Services&) const
{
  std::string rule;
  if (_taxPointLoc2Compare == type::TaxPointLoc2Compare::Stopover)
    rule = "STOPOVER";
  else if (_taxPointLoc2Compare == type::TaxPointLoc2Compare::Point)
    rule = "POINT";

  return "LOC2 COMPARISON, LOC2 MUST BE DIFFERENT\n FROM THE SUBSEQUENT/PRECEDING " + rule +
         " TO LOC1";
}
}
