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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Request;
class TaxPointLoc2CompareApplicator;

class TaxPointLoc2CompareRule : public BusinessRule
{
public:
  typedef TaxPointLoc2CompareApplicator ApplicatorType;
  TaxPointLoc2CompareRule(const type::TaxPointLoc2Compare& taxPointLoc2Compare,
                          const type::TicketedPointTag& ticketedPointTag);
  virtual ~TaxPointLoc2CompareRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  const type::TaxPointLoc2Compare& getTaxPointLoc2Compare() const { return _taxPointLoc2Compare; }
  const type::TicketedPointTag& getTicketedPointTag() const { return _ticketedPointTag; }

private:
  type::TaxPointLoc2Compare _taxPointLoc2Compare;
  type::TicketedPointTag _ticketedPointTag;
};
}
