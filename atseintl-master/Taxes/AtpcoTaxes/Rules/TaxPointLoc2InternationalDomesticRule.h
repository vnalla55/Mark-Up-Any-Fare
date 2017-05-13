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
class TaxPointLoc2InternationalDomesticApplicator;

class TaxPointLoc2InternationalDomesticRule : public BusinessRule
{
public:
  typedef TaxPointLoc2InternationalDomesticApplicator ApplicatorType;
  TaxPointLoc2InternationalDomesticRule(const type::IntlDomInd& intlDomInd,
                                        const type::TicketedPointTag& ticketedPointTag);
  virtual ~TaxPointLoc2InternationalDomesticRule();

  virtual std::string getDescription(Services& services) const override;
  ApplicatorType createApplicator(const type::Index& /*itinIndex*/,
                                  const Request& /*request*/,
                                  Services& /*services*/,
                                  RawPayments& /*itinPayments*/) const;

  const type::IntlDomInd& intlDomInd() const { return _intlDomInd; }
  const type::TicketedPointTag& ticketedPointTag() const { return _ticketedPointTag; }

private:
  type::IntlDomInd _intlDomInd;
  type::TicketedPointTag _ticketedPointTag;
};
}
