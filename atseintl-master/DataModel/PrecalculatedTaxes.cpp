// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DataModel/PrecalculatedTaxes.h"

namespace tse
{
// Every valid amount will be higher than INVALID_AMOUNT.
const MoneyAmount INVALID_AMOUNT = -std::numeric_limits<MoneyAmount>::infinity();

MoneyAmount
PrecalculatedTaxes::getAmount(Type type, const PaxTypeFare& fare) const
{
  const auto fareIt = _fareToAmountMap.find(&fare);
  if (fareIt != _fareToAmountMap.end())
    return fareIt->second.amount(type);

  return INVALID_AMOUNT;
}

void
PrecalculatedTaxes::copyAmountsIfPresent(const PrecalculatedTaxes& other,
                                         const PaxTypeFare& otherFare,
                                         const PaxTypeFare& localFare)
{
  const auto fareIt = other._fareToAmountMap.find(&otherFare);
  if (fareIt == other._fareToAmountMap.end())
    return;

  PrecalculatedTaxesAmount& localAmount = _fareToAmountMap[&localFare];
  for (int i = 0; i < Type::Count; ++i)
  {
    const MoneyAmount taxAmount = fareIt->second.amount(Type(i));
    if (taxAmount != INVALID_AMOUNT)
      localAmount.setAmount(Type(i), taxAmount);
  }
}

void
PrecalculatedTaxes::clearAll()
{
  _fareToAmountMap.clear();
  std::fill(std::begin(_defaultAmounts), std::end(_defaultAmounts), 0.0);
  std::fill(std::begin(_processed), std::end(_processed), false);
}

MoneyAmount
PrecalculatedTaxes::getTotalTaxAmount(const PaxTypeFare& fare) const
{
  const auto fareIt = _fareToAmountMap.find(&fare);
  if (fareIt == _fareToAmountMap.end())
    return getDefaultTotalTaxAmount();

  return getTotalTaxAmount(fareIt->second);
}

MoneyAmount
PrecalculatedTaxes::getTotalTaxAmount(const PrecalculatedTaxesAmount& amounts) const
{
  MoneyAmount total = 0.0;
  for (int i = 0; i < Type::Count; ++i)
  {
    const MoneyAmount taxAmount = amounts.amount(Type(i));
    total += (taxAmount != INVALID_AMOUNT ? taxAmount : _defaultAmounts[i]);
  }

  return total;
}

bool
PrecalculatedTaxes::operator==(const PrecalculatedTaxes& other) const
{
  return _fareToAmountMap == other._fareToAmountMap &&
         std::equal(std::begin(_defaultAmounts),
                    std::end(_defaultAmounts),
                    std::begin(other._defaultAmounts)) &&
         std::equal(std::begin(_processed), std::end(_processed), std::begin(other._processed));
}
}
