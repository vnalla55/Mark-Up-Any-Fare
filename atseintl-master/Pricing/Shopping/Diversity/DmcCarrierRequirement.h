// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{
namespace shpq
{
class SoloPQItem;
}

class DmcCarrierRequirement : public DmcRequirement
{
public:
  DmcCarrierRequirement(DmcRequirementsSharedContext& sharedCtx);

  Value getStatus() const;
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;
  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price) const;

  void adjustCarrierRequirementsIfNeeded(size_t numberOfSolutionsRequired,
                                         size_t numberOfSolutionsCollected);

  void setFareCutoffReached() { _isFareCutoffReached = true; }
  void print() const;
  bool isOCOAndOCOMapAvailable() const { return _sharedCtx._diversity.isOCO(); }

  void printOCOIfSet() const;

  bool getSopCouldSatisfy(unsigned legId,
                          const SOPInfo& sopInfo,
                          const SopInfosStatistics& statistics) const;

private:
  bool isAllCarriersSatisfied() const;
  bool isCarrierOptionsNeeded(const CarrierCode& carrier) const;

  bool isCarrierRequirementsAdjustmentNeeded() const;
  void adjustCarrierRequirements();

  bool _isFareCutoffReached;
  bool _carriersWereAdjusted;
  DmcRequirementsSharedContext& _sharedCtx;
};

} // ns tse

