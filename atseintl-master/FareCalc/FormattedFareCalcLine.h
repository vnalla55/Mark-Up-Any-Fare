#pragma once

#include "FareCalc/Optional.h"

#include <string>

namespace tse
{

class PricingTrx;
class FarePath;
class FareCalcConfig;
class FareCalcCollector;
class CalcTotals;

class FormattedFareCalcLine : public FareCalc::Optional<std::string>
{
public:
  FormattedFareCalcLine() : _trx(nullptr), _farePath(nullptr), _fcConfig(nullptr), _fcCollector(nullptr), _calcTotals(nullptr) {}

  void initialize(PricingTrx* trx,
                  const FarePath* farePath,
                  const FareCalcConfig* fcConfig,
                  FareCalcCollector* fcCollector,
                  CalcTotals* calcTotals)
  {
    _trx = trx;
    _farePath = farePath;
    _fcConfig = fcConfig;
    _fcCollector = fcCollector;
    _calcTotals = calcTotals;
  }

private:
  void compute(std::string& val) const override;

  PricingTrx* _trx;
  const FarePath* _farePath;
  const FareCalcConfig* _fcConfig;
  FareCalcCollector* _fcCollector;
  CalcTotals* _calcTotals;
};

} // namespace tse

