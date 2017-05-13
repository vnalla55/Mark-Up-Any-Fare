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
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Util/FlatMap.h"

#include <algorithm>
#include <numeric>

namespace tse
{
class PaxTypeFare;

extern const MoneyAmount INVALID_AMOUNT;

struct PrecalculatedTaxesAmount
{
  enum Type
  {
    YQYR,
    CAT12,
    XF,

    Count
  };

  PrecalculatedTaxesAmount()
  {
    std::fill(std::begin(_amounts), std::end(_amounts), INVALID_AMOUNT);
  }

  bool has(Type type) const { return _amounts[type] != INVALID_AMOUNT; }

  MoneyAmount amount(Type type) const { return _amounts[type]; }
  void setAmount(Type type, MoneyAmount m) { _amounts[type] = m; }

  friend bool operator==(const PrecalculatedTaxesAmount& l, const PrecalculatedTaxesAmount& r)
  {
    return std::equal(std::begin(l._amounts), std::end(l._amounts), std::begin(r._amounts));
  }

private:
  MoneyAmount _amounts[Count];
};

struct PrecalculatedTaxes
{
  typedef PrecalculatedTaxesAmount::Type Type;

  typedef FlatMap<const PaxTypeFare*, PrecalculatedTaxesAmount> FareToAmountMap;
  typedef FareToAmountMap::const_iterator const_iterator;

  MoneyAmount getAmount(Type type, const PaxTypeFare& fare) const;

  void setAmount(Type type, const PaxTypeFare& fare, MoneyAmount amount)
  {
    _fareToAmountMap[&fare].setAmount(type, amount);
  }

  void setAmounts(const PaxTypeFare& fare, const PrecalculatedTaxesAmount& amounts)
  {
    _fareToAmountMap[&fare] = amounts;
  }

  void copyAmountsIfPresent(const PrecalculatedTaxes& other,
                            const PaxTypeFare& otherFare,
                            const PaxTypeFare& localFare);

  void clearAll();

  FareToAmountMap::size_type size() const { return _fareToAmountMap.size(); }
  bool empty() const { return _fareToAmountMap.empty(); }
  const_iterator begin() const { return _fareToAmountMap.begin(); }
  const_iterator end() const { return _fareToAmountMap.end(); }

  MoneyAmount getDefaultAmount(Type type) const { return _defaultAmounts[type]; }
  void setDefaultAmount(Type type, MoneyAmount amount) { _defaultAmounts[type] = amount; }

  bool isProcessed(Type type) const { return _processed[type]; }
  void setProcessed(Type type, bool processed = true) { _processed[type] = processed; }

  MoneyAmount getTotalTaxAmount(const PaxTypeFare& fare) const;
  MoneyAmount getTotalTaxAmount(const PrecalculatedTaxesAmount& amounts) const;

  MoneyAmount getDefaultTotalTaxAmount() const
  {
    return std::accumulate(std::begin(_defaultAmounts), std::end(_defaultAmounts), 0.0);
  }

  bool operator==(const PrecalculatedTaxes& other) const;
  bool operator!=(const PrecalculatedTaxes& other) const { return !(*this == other); }

private:
  FareToAmountMap _fareToAmountMap;
  MoneyAmount _defaultAmounts[Type::Count] = {};
  bool _processed[Type::Count] = {};
};
}
