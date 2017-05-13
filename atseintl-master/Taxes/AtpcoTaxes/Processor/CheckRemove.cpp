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
#include "Processor/CheckRemove.h"

namespace tax
{

CheckRemove::CheckRemove(PaymentDetail const* paymentDetail) : _paymentDetail(paymentDetail) {}

bool
CheckRemove::
operator()(PaymentWithRules const& paymentWithRules)
{
  return compareWith(*paymentWithRules.paymentDetail);
}

bool
CheckRemove::compareWith(PaymentDetail const& paymentToCompare)
{
  if (_paymentDetail->taxName() == paymentToCompare.taxName() &&
      _paymentDetail->getTaxPointBegin().id() == paymentToCompare.getTaxPointBegin().id())
  {
    return true;
  }

  return false;
}

} // namespace tax
