// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Rules/BusinessRuleApplicator.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class ThirdPartyTagRule;
class PaymentDetail;

class ThirdPartyTagApplicator : public BusinessRuleApplicator
{
public:
  ThirdPartyTagApplicator(const ThirdPartyTagRule& rule,
                          const type::PaidBy3rdPartyTag& paidBy3rdPartyTag,
                          const type::FormOfPayment& formOfPayment);

  ~ThirdPartyTagApplicator();

  bool apply(PaymentDetail& /*paymentDetail*/) const;

private:
  const type::PaidBy3rdPartyTag& _paidBy3rdPartyTag;
  const type::FormOfPayment& _formOfPayment;
};

} // namespace tax
