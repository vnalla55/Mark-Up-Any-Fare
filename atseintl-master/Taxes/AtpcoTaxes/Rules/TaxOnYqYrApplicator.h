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

#include <vector>

#include "Common/ProrateCalculator.h"
#include "Common/RangeUtils.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class PaymentDetail;
class ServiceBaggage;
class TaxOnYqYrRule;

class TaxOnYqYrApplicator : public BusinessRuleApplicator
{
public:
  TaxOnYqYrApplicator(const TaxOnYqYrRule& rule,
                      std::shared_ptr<ServiceBaggage const> serviceBaggage,
                      bool restrictServiceBaggage);

  ~TaxOnYqYrApplicator() {}

  bool apply(PaymentDetail& paymentDetail) const;

private:
  TaxOnYqYrRule const& _rule;

  std::shared_ptr<ServiceBaggage const> _serviceBaggage;
  bool _restrictServiceBaggage;
};

} // namespace tax
