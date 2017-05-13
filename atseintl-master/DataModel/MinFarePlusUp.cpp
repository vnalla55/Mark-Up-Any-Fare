#include "DataModel/MinFarePlusUp.h"

#include "DataModel/FarePath.h"

namespace tse
{
void
MinFarePlusUpItem::applyDynamicPriceDeviation(FarePath& fp, Percent deviation)
{
  rollbackDynamicPriceDeviation();
  MoneyAmount amt = deviation / 100 * plusUpAmount;
  accumulateDynamicPriceDeviation(amt);
  fp.accumulatePriceDeviationAmount(amt);
}
}
