// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
class CustomerRestrictionRule;
class CustomerService;

class CustomerRestrictionApplicator: public BusinessRuleApplicator
{
public:
  CustomerRestrictionApplicator(const CustomerRestrictionRule& parent,
      const CustomerService& customer, const type::PseudoCityCode& pcc);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const CustomerRestrictionRule& _customerRestrictionRule;
  const CustomerService& _customerService;
  const type::PseudoCityCode& _pcc;
};

} /* namespace tax */

