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
#include "Rules/TaxPointLoc1StopoverTagRule.h"
#include "Rules/TaxPointLoc1StopoverTagApplicator.h"

namespace tax
{

TaxPointLoc1StopoverTagRule::TaxPointLoc1StopoverTagRule(
    type::StopoverTag const& stopoverTag,
    type::TicketedPointTag const& ticketedPointTag,
    bool fareBreakMustAlsoBeStopover)
  : _stopoverTag(stopoverTag),
    _ticketedPointTag(ticketedPointTag),
    _fareBreakMustAlsoBeStopover(fareBreakMustAlsoBeStopover)
{
}

TaxPointLoc1StopoverTagRule::~TaxPointLoc1StopoverTagRule() {}

TaxPointLoc1StopoverTagRule::ApplicatorType
TaxPointLoc1StopoverTagRule::createApplicator(const type::Index& /*itinIndex*/,
                                              const Request& /*request*/,
                                              Services& /*services*/,
                                              RawPayments& /*itinPayments*/) const
{
  return ApplicatorType(*this, _stopoverTag, _ticketedPointTag, _fareBreakMustAlsoBeStopover);
}

std::string
TaxPointLoc1StopoverTagRule::getDescription(Services&) const
{
  std::string reason = "FOR THE RECORD TO MATCH, LOC1 MUST ";

  if (_stopoverTag == type::StopoverTag::Connection)
  {
    reason += std::string("BE A CONNECTION POINT");
  }
  else if (_stopoverTag == type::StopoverTag::Stopover)
  {
    reason += std::string("BE A STOPOVER");
  }
  else if (_stopoverTag == type::StopoverTag::FareBreak)
  {
    reason += std::string("BE A FARE BREAK POINT");
    if (_fareBreakMustAlsoBeStopover)
      reason += std::string(" AND A STOPOVER");
  }
  else if (_stopoverTag == type::StopoverTag::NotFareBreak)
  {
    reason += std::string("NOT BE A FARE BREAK POINT");
  }

  return reason;
}

} // namespace tax
