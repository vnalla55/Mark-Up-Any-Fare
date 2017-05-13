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
#include "ServiceInterfaces/Services.h"

namespace tax
{
class Fare;
class PercentageTaxRule;
class PaymentDetail;
class Services;

class PercentageTaxApplicator : public BusinessRuleApplicator
{
public:
  PercentageTaxApplicator(PercentageTaxRule const& parent,
                          const Services& services,
                          const type::CurrencyCode& paymentCurrency);

  ~PercentageTaxApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  PercentageTaxRule const& _percentageTaxRule;
  const Services& _services;
  const type::CurrencyCode& _paymentCurrency;
};
}
