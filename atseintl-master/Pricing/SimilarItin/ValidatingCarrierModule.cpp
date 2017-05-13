/*---------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"

#include "Common/FallbackUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/ValidatingCxrUtil.h"
#include "Common/ItinUtil.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diag990Collector.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/SimilarItin/Context.h"

#include <string>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackGSAChildItinFix2)
FALLBACK_DECL(clearGsaClonedFarePathsFamilyLogic)
FALLBACK_DECL(familyLogicRevalidateAllVc)

namespace similaritin
{
void
ValidatingCarrierModule::showDiag(const std::vector<CarrierCode>& farePathCxrVec)
{
  if (LIKELY(!_diag990))
    return;

  if (farePathCxrVec.empty())
    *_diag990 << " CLONED FAREPATH IS INVALID FOR VALIDATING CARRIER INTERLINE " << std::endl;
  else
  {
    *_diag990 << " CLONED FAREPATH VALID FOR GSA. VALIDATING CXR LIST: ";
    for (auto& elem : farePathCxrVec)
      *_diag990 << elem << "  ";
    *_diag990 << "\n";
  }
}

bool
ValidatingCarrierModule::buildCarrierLists(Itin& estItin, FarePath* cloned)
{
  PricingTrx& trx = _context.trx;
  if (!trx.isValidatingCxrGsaApplicable())
    return true;

  // If the vector is empty, we are actually using default validating carriers,
  // No further processing required
  if (cloned->validatingCarriers().empty())
    return true;

  std::vector<CarrierCode> childCxrVec;
  estItin.getValidatingCarriers(trx, childCxrVec);

  // At the end of this step, cloned validating carriers will contain intersection
  // with childCxrVec. And, childCxrVec will remove those carriers from it's list
  PricingUtil::intersectCarrierList(cloned->validatingCarriers(), childCxrVec);

  showDiag(cloned->validatingCarriers());
  // if the result vector is empty, this cloned farepath is not good for further processing
  return !(cloned->validatingCarriers().empty());
}

namespace
{
bool
isChildFarePathValidForValidatingCxr(FarePath& farePath, const std::vector<CarrierCode>& childVCs)
{
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    pu->validatingCarriers() = childVCs;
    for (FareUsage* fu : pu->fareUsage())
    {
      PricingUtil::intersectCarrierList(pu->validatingCarriers(),
                                        fu->paxTypeFare()->validatingCarriers());

      if (pu->validatingCarriers().empty())
        return false;
    }
  }

  return ValidatingCxrUtil::isValidFPathForValidatingCxr(farePath);
}
}

bool
ValidatingCarrierModule::processValidatingCarriers(const Itin& motherItin,
                                                   FarePath& farePath,
                                                   const Itin& childItin,
                                                   bool& revalidationRequired)
{
  PricingTrx& trx = _context.trx;
  if (fallback::fallbackGSAChildItinFix2(&trx))
    return true;

  if (!trx.isValidatingCxrGsaApplicable())
    return true;

  std::vector<CarrierCode> motherVCs, childVCs;
  motherItin.getValidatingCarriers(trx, motherVCs);
  childItin.getValidatingCarriers(trx, childVCs);

  std::sort(motherVCs.begin(), motherVCs.end());
  std::sort(childVCs.begin(), childVCs.end());
  if (fallback::familyLogicRevalidateAllVc(&trx) &&
      std::includes(motherVCs.begin(), motherVCs.end(), childVCs.begin(), childVCs.end()))
    return true;

  // Child has extra VCs. We need to revalidate and update FP and PU VC list
  revalidationRequired = true;

  if (!isChildFarePathValidForValidatingCxr(farePath, childVCs))
    return false;

  if (!farePath.gsaClonedFarePaths().empty())
  {
    farePath.gsaClonedFarePaths().clear();
    if (fallback::clearGsaClonedFarePathsFamilyLogic(&trx))
    {
      const std::vector<FarePath*> clonedFarePaths = farePath.gsaClonedFarePaths();

      for (FarePath* clonedFarePath : clonedFarePaths)
        if (clonedFarePath && isChildFarePathValidForValidatingCxr(*clonedFarePath, childVCs))
          farePath.gsaClonedFarePaths().push_back(clonedFarePath);
    }
  }

  return true;
}
}
}
