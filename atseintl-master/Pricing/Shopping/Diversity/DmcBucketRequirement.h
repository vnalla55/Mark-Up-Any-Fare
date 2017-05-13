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

class DmcBucketRequirement : public DmcRequirement
{
public:
  DmcBucketRequirement(DmcRequirementsSharedContext& sharedCtx);

  Value getStatus() const;
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem);
  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price);

  void setNumberSolutionsRequiredCollected(size_t numberOfSolutionsRequired,
                                           size_t numberOfSolutionsCollected,
                                           MoneyAmount score);

  int getBucketStatus(const Diversity::BucketType bucket) const;
  void setFareCutoffReached() { _isFareCutoffReached = true; }
  void print();

  bool getSopCouldSatisfy(unsigned legId,
                          const SOPInfo& sopInfo,
                          const SopInfosStatistics& statistics) const;

private:
  void adjustBucketDistributionIfNeeded(MoneyAmount score);
  void adjustBucketDistributionUnconditionally();
  size_t getOptionsNeededForBucket(const Diversity::BucketType bucket) const;

  static Value toDmcValue(const Diversity::BucketType bucket);

  bool _isDistributionAdjusted;
  bool _isFareCutoffReached;
  DmcRequirementsSharedContext& _sharedCtx;
};

} // ns tse

