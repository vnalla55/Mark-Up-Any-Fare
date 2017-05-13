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
#include "Processor/BusinessRulesProcessor.h"
#include "Rules/ItinPayments.h"

namespace tax
{

class PaymentDetail;

struct CheckRemove
{
  CheckRemove(PaymentDetail const* paymentDetail);

  bool operator()(PaymentWithRules const& paymentWithRules);

private:
  bool compareWith(PaymentDetail const& paymentToCompare);

  PaymentDetail const* _paymentDetail;
};

} // namespace tax
