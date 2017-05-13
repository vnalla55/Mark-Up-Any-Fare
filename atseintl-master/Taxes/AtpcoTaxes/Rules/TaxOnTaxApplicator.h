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
#include "Rules/BusinessRule.h" // for RawPayment and RawPayments

#include <memory>

namespace tax
{
class PaymentDetail;
class ServiceBaggage;
class Services;
class TaxOnTaxRule;

class TaxOnTaxApplicator : public BusinessRuleApplicator
{
  typedef RawPayments::value_type RawPayment;

public:
  TaxOnTaxApplicator(const TaxOnTaxRule& businessRule,
                     const Services& services,
                     std::shared_ptr<ServiceBaggage const> serviceBaggage,
                     bool restrictServiceBaggage,
                     RawPayments& rawPayments);

  ~TaxOnTaxApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

  bool isInRange(PaymentDetail& paymentDetail, RawPayment const& rawPayment) const;

private:
  bool isTaxNameOnList(TaxName const& taxName) const;

  TaxOnTaxRule const& _rule;
  const Services& _services;
  std::shared_ptr<ServiceBaggage const> _serviceBaggage;
  bool _restrictServiceBaggage;
  RawPayments& _rawPayments;
};

} // namespace tax

