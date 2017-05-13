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
#pragma once

#include <string>
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxPointLoc1StopoverTagApplicator;

class TaxPointLoc1StopoverTagRule : public BusinessRule
{
public:
  typedef TaxPointLoc1StopoverTagApplicator ApplicatorType;
  TaxPointLoc1StopoverTagRule(type::StopoverTag const& stopoverTag,
                              type::TicketedPointTag const& ticketedPointTag,
                              bool fareBreakMustAlsoBeStopover);
  virtual ~TaxPointLoc1StopoverTagRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  const type::StopoverTag& getStopoverTag() const { return _stopoverTag; }
private:
  type::StopoverTag _stopoverTag;
  type::TicketedPointTag _ticketedPointTag;
  bool _fareBreakMustAlsoBeStopover;
};

} // namespace tax
