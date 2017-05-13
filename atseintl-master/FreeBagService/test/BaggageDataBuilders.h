#pragma once

#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/BaggageCharge.h"
#include "test/include/TestDataBuilders.h"

namespace tse
{
class ChargeBuilder : public GenericBuilder<BaggageCharge>
{
  typedef GenericBuilder<BaggageCharge> Base;

public:
  using Base::Base;

  ChargeBuilder& withS7(OptionalServicesInfo* s7)
  {
    _ptr->optFee() = s7;
    return *this;
  }

  ChargeBuilder& withS5(SubCodeInfo* s5)
  {
    _ptr->subCodeInfo() = s5;
    return *this;
  }

  ChargeBuilder& withAmt(MoneyAmount amt, CurrencyCode cur)
  {
    _ptr->feeAmount() = amt;
    _ptr->feeCurrency() = cur;
    return *this;
  }

  template <class... T>
  ChargeBuilder& forBags(T... matchedBags)
  {
    for (const size_t bagNo : {matchedBags...})
      _ptr->setMatchedBag(bagNo);
    return *this;
  }
};
}
