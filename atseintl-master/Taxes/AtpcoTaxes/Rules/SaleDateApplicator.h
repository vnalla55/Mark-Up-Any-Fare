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

#include "Rules/BusinessRuleApplicator.h"
#include "Common/Timestamp.h"

namespace tax
{

class BusinessRule;
class PaymentDetail;

class SaleDateApplicator : public BusinessRuleApplicator
{
public:
  SaleDateApplicator(BusinessRule const* parent,
                     type::Date effDate,
                     type::Timestamp discDate,
                     type::Timestamp saleDate);
  ~SaleDateApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool _value;
};
}
