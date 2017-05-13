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
class TaxPointLoc1InternationalDomesticApplicator;

class TaxPointLoc1InternationalDomesticRule : public BusinessRule
{
public:
  typedef TaxPointLoc1InternationalDomesticApplicator ApplicatorType;
  TaxPointLoc1InternationalDomesticRule(type::AdjacentIntlDomInd const& adjacentIntlDomInd,
                                        type::TicketedPointTag const& ticketedPointTag);
  virtual ~TaxPointLoc1InternationalDomesticRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(type::Index const& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const;

  type::AdjacentIntlDomInd const& getAdjacentIntlDomInd() const { return _adjacentIntlDomInd; }
  type::TicketedPointTag const& getTicketedPointTag() const { return _ticketedPointTag; }

private:
  type::AdjacentIntlDomInd _adjacentIntlDomInd;
  type::TicketedPointTag _ticketedPointTag;
};
}
