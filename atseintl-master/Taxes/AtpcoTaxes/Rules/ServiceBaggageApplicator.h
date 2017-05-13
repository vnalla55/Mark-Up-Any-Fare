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
#include "Rules/BusinessRule.h"
#include "Rules/BusinessRuleApplicator.h"
#include <vector>

namespace tax
{
class BusinessRule;
class GeoPathMapping;
class PaymentDetail;
class ServiceBaggage;
class ServiceBaggageRule;
class YqYr;
class YqYrPath;

class ServiceBaggageApplicator : public BusinessRuleApplicator
{
  typedef RawPayments::value_type RawPayment;

public:
  ServiceBaggageApplicator(ServiceBaggageRule const* serviceBaggageRule,
                           std::vector<YqYr> const* yqYrs,
                           YqYrPath const* yqYrPath,
                           GeoPathMapping const* yqYrMappings,
                           std::shared_ptr<ServiceBaggage const> serviceBaggage,
                           RawPayments& rawPayments);

  ~ServiceBaggageApplicator() {}

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool processYqYrs(PaymentDetail& paymentDetail) const;

  std::vector<YqYr> const* _yqYrs;
  YqYrPath const* _yqYrPath;
  GeoPathMapping const* _yqYrMappings;
  std::shared_ptr<ServiceBaggage const> _serviceBaggage;
  RawPayments& _rawPayments;
  ServiceBaggageRule const* _rule;

protected:
  static bool match(PaymentDetail const& paymentDetail, RawPayment const& rawPayment);
  static bool matchTaxRange(PaymentDetail const& paymentDetail, RawPayment const& rawPayment);
};

} // namespace tax

