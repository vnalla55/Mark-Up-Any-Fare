#pragma once

#include "FareCalc/FareCalcCollector.h"

namespace tse
{

class AltFareCalcCollector : public FareCalcCollector
{
private:
  virtual FareCalculation*
  createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig) override;
};

} // namespace tse

