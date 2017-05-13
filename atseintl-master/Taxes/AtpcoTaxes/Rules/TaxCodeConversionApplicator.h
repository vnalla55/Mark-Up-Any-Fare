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

namespace tax
{

class PaymentDetail;
class TaxCodeConversionRule;
class Services;

class TaxCodeConversionApplicator : public BusinessRuleApplicator
{
public:
  TaxCodeConversionApplicator(const TaxCodeConversionRule& rule, const Services& services);
  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Services& _services;
};

} // namespace tax

