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
#include "Rules/BusinessRule.h" // for RawPayment
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class BlankLimitRule;
class PaymentDetail;

class BlankLimitApplicator : public BusinessRuleApplicator
{
public:
  BlankLimitApplicator(const BlankLimitRule* parent, RawPayments& rawPayments);

  ~BlankLimitApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  RawPayments& _rawPayments;
};

} // namespace tax

