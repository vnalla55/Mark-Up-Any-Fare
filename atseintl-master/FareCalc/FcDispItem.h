#pragma once

#include "Common/FcConfig.h"
#include "Common/TsePrimitiveTypes.h"

#include <iosfwd>

namespace tse
{

class PricingTrx;
class FareCalcConfig;
class FareCalcCollector;

class FcDispItem
{
public:
  FcDispItem(PricingTrx& trx, // non-const for dataHandle usage
             const FareCalcConfig& fcConfig,
             FareCalcCollector& fcCollector)
    : _trx(trx), _fcConfig(&trx, &fcConfig), _fcCollector(fcCollector)
  {
  }

  virtual ~FcDispItem() {}

  virtual std::string toString() const = 0;

  friend std::ostream& operator<<(std::ostream& os, const FcDispItem& i);

  static std::string
  convertAmount(const MoneyAmount amount, uint16_t inNoDec, uint16_t outNoDigit = 2);

protected:
  PricingTrx& _trx;
  FcConfig _fcConfig;
  FareCalcCollector& _fcCollector;
};

std::ostream& operator<<(std::ostream& os, const FcDispItem& i);

} // namespace tse

