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
#include "Rules/ThirdPartyTagApplicator.h"
#include "Rules/ThirdPartyTagRule.h"

namespace tax
{

ThirdPartyTagApplicator::ThirdPartyTagApplicator(const ThirdPartyTagRule& rule,
                                                 const type::PaidBy3rdPartyTag& paidBy3rdPartyTag,
                                                 const type::FormOfPayment& formOfPayment)
  : BusinessRuleApplicator(&rule),
    _paidBy3rdPartyTag(paidBy3rdPartyTag),
    _formOfPayment(formOfPayment)
{
}

ThirdPartyTagApplicator::~ThirdPartyTagApplicator() {}

bool
ThirdPartyTagApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  if (_paidBy3rdPartyTag == type::PaidBy3rdPartyTag::Government)
  {
    return (_formOfPayment == type::FormOfPayment::Government);
  }
  else if (_paidBy3rdPartyTag == type::PaidBy3rdPartyTag::Miles)
  {
    return (_formOfPayment == type::FormOfPayment::Miles);
  }
  else if (_paidBy3rdPartyTag == type::PaidBy3rdPartyTag::Blank)
  {
    return true;
  }

  return false;
}

} // namespace tax
