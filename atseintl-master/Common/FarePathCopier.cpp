//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#include "Common/FarePathCopier.h"

#include "Common/Logger.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingUnit.h"

namespace tse
{
namespace
{
Logger logger("atseintl.Common.FarePathCopier");
}

FarePath*
FarePathCopier::getDuplicate(const FarePath& source)
{
  FarePath* result = duplicateBase(source);

  if (UNLIKELY(source.netRemitFarePath()))
  {
    result->netRemitFarePath() =
        dynamic_cast<NetRemitFarePath*>(getDuplicate(*source.netRemitFarePath()));

    TSE_ASSERT(result->netRemitFarePath());
  }

  if (UNLIKELY(!source.gsaClonedFarePaths().empty()))
  {
    result->gsaClonedFarePaths().clear();
    for (const FarePath* gsaFarePath : source.gsaClonedFarePaths())
      result->gsaClonedFarePaths().push_back(duplicateBase(*gsaFarePath));
  }

  return result;
}

FarePath*
FarePathCopier::duplicateBase(const FarePath& source)
{
  FarePath* result = source.clone(_dataHandle);

  result->pricingUnit().clear();

  std::map<PricingUnit*, PricingUnit*> stPUreplacement;

  for (PricingUnit* sourcePu : source.pricingUnit())
  {
    PricingUnit* pu = sourcePu->clone(_dataHandle);

    if (sourcePu->isSideTripPU())
      stPUreplacement[sourcePu] = pu;

    pu->fareUsage().clear();

    for (FareUsage* sourceFu : sourcePu->fareUsage())
      pu->fareUsage().push_back(sourceFu->clone(_dataHandle));

    result->pricingUnit().push_back(pu);
  }

  fixMainTripSideTripLink(stPUreplacement, source, *result);

  return result;
}

void
FarePathCopier::fixMainTripSideTripLink(const std::map<PricingUnit*, PricingUnit*>& stPUreplacement,
                                        const FarePath& sourceFarePath,
                                        FarePath& resultFarePath) const
{
  const size_t puUpperBound =
      std::min(sourceFarePath.pricingUnit().size(), resultFarePath.pricingUnit().size());
  for (size_t puId = 0; puId != puUpperBound; ++puId)
  {
    PricingUnit* sourcePu = sourceFarePath.pricingUnit()[puId];
    PricingUnit* resultPu = resultFarePath.pricingUnit()[puId];
    if (sourcePu->hasSideTrip() == false)
      continue;

    resultPu->sideTripPUs() = fixSideTripPUVector(stPUreplacement, sourcePu->sideTripPUs());

    const size_t fuUpperBound =
        std::min(sourcePu->fareUsage().size(), resultPu->fareUsage().size());
    for (size_t fuId = 0; fuId != fuUpperBound; ++fuId)
    {
      FareUsage* sourceFu = sourcePu->fareUsage()[fuId];
      if (sourceFu->hasSideTrip() == false)
        continue;

      resultPu->fareUsage()[fuId]->sideTripPUs() =
          fixSideTripPUVector(stPUreplacement, sourceFu->sideTripPUs());
    }
  }
}

std::vector<PricingUnit*>
FarePathCopier::fixSideTripPUVector(const std::map<PricingUnit*, PricingUnit*>& stPUreplacement,
                                    const std::vector<PricingUnit*>& sourceVect) const
{
  std::vector<PricingUnit*> result;
  for (PricingUnit* sourcePricignUnit : sourceVect)
  {
    auto replacement = stPUreplacement.find(sourcePricignUnit);
    if (replacement != stPUreplacement.end())
      result.push_back(replacement->second);
    else
      LOG4CXX_ERROR(logger, __FILE__ << ":" << __LINE__ << " SideTrip Link Broken??");
  }
  return result;
}
}
