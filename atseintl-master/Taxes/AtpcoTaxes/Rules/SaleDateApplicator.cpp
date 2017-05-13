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
#include "Rules/SaleDateApplicator.h"

namespace tax
{

SaleDateApplicator::SaleDateApplicator(BusinessRule const* parent,
                                       type::Date effDate,
                                       type::Timestamp discDate,
                                       type::Timestamp saleDate)
  : BusinessRuleApplicator(parent), _value(effDate <= saleDate.date() && saleDate <= discDate)
{
}

SaleDateApplicator::~SaleDateApplicator() {}

bool
SaleDateApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  return _value;
}
}
