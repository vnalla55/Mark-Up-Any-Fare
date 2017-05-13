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

namespace tax
{
class FallbackService;
class OptionalService;
class PaymentDetail;
class ServiceBaggage;
class ServiceBaggageEntry;
class TaxOnOptionalServiceRule;

class TaxOnOptionalServiceApplicator : public BusinessRuleApplicator
{
  friend class TaxOnOptionalServiceTest;

public:
  TaxOnOptionalServiceApplicator(const TaxOnOptionalServiceRule& rule,
                                 std::shared_ptr<ServiceBaggage const> serviceBaggage,
                                 const FallbackService& fallbackService,
                                 bool restrictServiceBaggage);

  ~TaxOnOptionalServiceApplicator() {}

  bool apply(PaymentDetail& paymentDetail) const;

private:
  void
  failAllOptionalServices(PaymentDetail& paymentDetail) const;

  bool
  checkOptionalService(const ServiceBaggageEntry& entry,
                       const OptionalService& optionalService) const;

  TaxOnOptionalServiceRule const& _rule;
  std::shared_ptr<ServiceBaggage const> _serviceBaggage;
  const FallbackService& _fallbackService;
  bool _restrictServiceBaggage;
};

} // namespace tax
