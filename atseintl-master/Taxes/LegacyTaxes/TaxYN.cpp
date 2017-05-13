// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxYN.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"

#include "Common/FallbackUtil.h"

using namespace tse;

MoneyAmount
TaxYN::fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse)
{
  MoneyAmount moneyAmount = 0.0;

  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;

  for (pricingUnitI = taxResponse.farePath()->pricingUnit().begin();
       pricingUnitI != taxResponse.farePath()->pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      TravelSeg* travelSeg = (*(*fareUsageI)).travelSeg().front();
      if (travelSeg->origAirport() != "PMV")
        continue;

      moneyAmount += (*fareUsageI)->totalFareAmount();
    }
  }

  return Tax::fareAmountInNUC(trx, taxResponse) - moneyAmount;
}

MoneyAmount
TaxYN::discFactor(const PricingTrx& trx, int16_t segmentOrder)
{
  MoneyAmount discFactor = 1.0;
  Percent discPercent = trx.getRequest()->discountPercentage(segmentOrder);

  if (discPercent >= 0 && discPercent <= 100)
    discFactor = (1.0 - discPercent / 100.0);

  return discFactor;
}
