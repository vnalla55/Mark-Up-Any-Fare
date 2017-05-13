//----------------------------------------------------------------------------
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Rules/PricingUnitLogic.h"

#include "Common/TSELatencyData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "MinFares/HIPMinimumFare.h"

namespace tse
{
// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool PricingUnitLogic::shouldConvertRTtoCT
//
// Description:
//
// @param
//
// @return true if it should
//
// </PRE>
// ----------------------------------------------------------------------------

bool
PricingUnitLogic::shouldConvertRTtoCT(PricingTrx& trx, PricingUnit& pu, const FarePath& fp)
{
  bool mustReclassify = false;

  // Get the HIP plus up values

  TSELatencyData hipMetrics(trx, "PO RT TO CT HIP INTERACTION");

  std::vector<FareUsage*>::iterator fareUsage = pu.fareUsage().begin();

  HIPMinimumFare hipMinimumFare(trx);

  hipMinimumFare.process(**fareUsage, pu, fp);

  const MinFarePlusUp minFarePlusUp1 = (**fareUsage).minFarePlusUp();
  const MinFarePlusUp::const_iterator hipIter1 = minFarePlusUp1.find(HIP);

  fareUsage++;

  hipMinimumFare.process(**fareUsage, pu, fp);

  const MinFarePlusUp minFarePlusUp2 = (**fareUsage).minFarePlusUp();
  const MinFarePlusUp::const_iterator hipIter2 = minFarePlusUp2.find(HIP);

  if (UNLIKELY(hipIter1 != minFarePlusUp1.end() && hipIter2 != minFarePlusUp2.end()))
  {
    MinFarePlusUpItem& hipPlusUpItem1 = *(hipIter1->second);
    MinFarePlusUpItem& hipPlusUpItem2 = *(hipIter2->second);

    mustReclassify = (hipPlusUpItem1.boardPoint != hipPlusUpItem2.boardPoint ||
                      hipPlusUpItem2.offPoint != hipPlusUpItem1.offPoint);
  }
  else if ((hipIter1 == minFarePlusUp1.end() && hipIter2 != minFarePlusUp2.end()) ||
           (hipIter1 != minFarePlusUp1.end() && hipIter2 == minFarePlusUp2.end()))

  {
    mustReclassify = true;
  }

  return mustReclassify;
}
}
