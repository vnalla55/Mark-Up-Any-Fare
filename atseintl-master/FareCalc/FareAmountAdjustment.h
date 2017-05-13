
#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <set>
#include <vector>

namespace tse
{

class CalcTotals;
class FareUsage;

namespace FareCalc
{

class FareAmountAdjustment
{
public:
  enum ADJ_TYPE
  {
    NO_ADJ,
    ADJ_FARE_COMP,
    ADJ_TOTAL
  };

  FareAmountAdjustment(const CalcTotals& calcTotals, const std::vector<FareUsage*>& fuv)
    : _status(false), _adjType(NO_ADJ), _adjAmount(0), _calcTotals(calcTotals), _fuv(fuv)
  {
    _status = process();
  }

  ~FareAmountAdjustment() {}

  bool status() const { return _status; }
  MoneyAmount adjAmount() const { return _adjAmount; }

  bool isAdjusted(const FareUsage* fu) const
  {
    return (_status && _adjType == ADJ_FARE_COMP && _adjSubjects.count(fu) > 0);
  }

  bool isAdjusted() const { return (_status && _adjType == ADJ_TOTAL); }

private:
  bool process();

private:
  bool _status;
  ADJ_TYPE _adjType;
  MoneyAmount _adjAmount;

  const CalcTotals& _calcTotals;
  const std::vector<FareUsage*>& _fuv;
  std::set<const FareUsage*> _adjSubjects;
};
}
} // namespace tse::FareCalc

