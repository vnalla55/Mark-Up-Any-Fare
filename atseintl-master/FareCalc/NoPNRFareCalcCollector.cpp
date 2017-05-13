#include "FareCalc/NoPNRFareCalcCollector.h"

#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareCalc/FareCalculation.h"
#include "FareCalc/NoPNRFareCalculation.h"
#include "FareCalc/NoPNRFcCollector.h"

namespace tse
{
FareCalculation*
NoPNRFareCalcCollector::createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig)
{
  NoPNRFareCalculation* fareCalculation = nullptr;
  fareCalculation = trx->dataHandle().create<NoPNRFareCalculation>();

  if (fareCalculation != nullptr)
    fareCalculation->initialize(trx, fcConfig, this);

  return fareCalculation;
}

CalcTotals*
NoPNRFareCalcCollector::createCalcTotals(PricingTrx& pricingTrx,
                                         const FareCalcConfig* fcConfig,
                                         const FarePath* fp)
{
  CalcTotals* totals = getCalcTotals(&pricingTrx, fp, fcConfig);
  if (totals != nullptr && fp->processed() == true)
  {
    FareCalc::NoPNRFcCollector collector(&pricingTrx, fp, fcConfig, this, totals);
    collector.collect();
  }

  return totals;
}

bool
NoPNRFareCalcCollector::initialize(PricingTrx& pricingTrx,
                                   Itin* itin,
                                   const FareCalcConfig* fcConfig)
{
  bool baseRetVal = FareCalcCollector::initialize(pricingTrx, itin, fcConfig);
  return baseRetVal;
}
} // tse namespace
