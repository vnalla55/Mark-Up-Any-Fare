#pragma once

#include "FareCalc/FareCalcCollector.h"

namespace tse
{
class CalcTotals;
class FareCalcConfig;
class FareCalculation;
class FarePath;
class Itin;
class PricingTrx;

class NoPNRFareCalcCollector : public FareCalcCollector
{
public:
  bool initialize(PricingTrx& pricingTrx, Itin* itin, const FareCalcConfig* fcConfig);

private:
  CalcTotals* createCalcTotals(PricingTrx& pricingTrx,
                               const FareCalcConfig* fcConfig,
                               const FarePath* fp) override;

  virtual FareCalculation*
  createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig) override;
};

} // namespace tse

