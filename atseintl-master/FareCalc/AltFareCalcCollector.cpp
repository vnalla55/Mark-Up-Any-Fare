#include "FareCalc/AltFareCalcCollector.h"

#include "DataModel/PricingTrx.h"
#include "FareCalc/AltFareCalculation.h"
#include "FareCalc/FareCalculation.h"

namespace tse
{
FareCalculation*
AltFareCalcCollector::createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig)
{
  AltFareCalculation* fareCalculation = nullptr;
  fareCalculation = trx->dataHandle().create<AltFareCalculation>();

  if (fareCalculation != nullptr)
    fareCalculation->initialize(trx, fcConfig, this);

  return fareCalculation;
}
}
