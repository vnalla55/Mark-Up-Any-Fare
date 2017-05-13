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
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class PaymentDetail;
class TaxPointLoc1StopoverTagRule;

class TaxPointLoc1StopoverTagApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc1StopoverTagApplicator(TaxPointLoc1StopoverTagRule const& rule,
                                    type::StopoverTag const& stopoverTag,
                                    type::TicketedPointTag const& ticketedPointTag,
                                    bool fareBreakMustAlsoBeStopover);
  ~TaxPointLoc1StopoverTagApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  type::StopoverTag const& _stopoverTag;
  type::TicketedPointTag const& _ticketedPointTag;
  bool _fareBreakMustAlsoBeStopover;
};

} // namespace tax

