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

#include "Pricing/Shopping/Diversity/DmcBucketRequirement.h"

#include "DataModel/Diversity.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <cassert>
#include <iomanip>

namespace tse
{

DmcBucketRequirement::DmcBucketRequirement(DmcRequirementsSharedContext& sharedCtx)
  : _isDistributionAdjusted(false), _isFareCutoffReached(false), _sharedCtx(sharedCtx)
{
}

DmcBucketRequirement::Value
DmcBucketRequirement::getStatus() const
{
  Value result = 0;

  if (!_isFareCutoffReached)
  {
    if (!_isDistributionAdjusted)
    {
      if (getOptionsNeededForBucket(Diversity::GOLD) != 0)
        result |= NEED_GOLD;
      if (getOptionsNeededForBucket(Diversity::UGLY) != 0)
        result |= NEED_UGLY;
    }
    if (getOptionsNeededForBucket(Diversity::LUXURY) != 0)
      result |= NEED_LUXURY;
    if (UNLIKELY(getOptionsNeededForBucket(Diversity::JUNK) != 0))
      result |= NEED_JUNK;
  }

  return result;
}

DmcBucketRequirement::Value
DmcBucketRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem)
{
  adjustBucketDistributionIfNeeded(pqItem->getScore());

  Value result = getStatus();

  // We always "could" satisfy L and J, so we need to check only for G and U
  if (result & (NEED_GOLD | NEED_UGLY))
  {
    MoneyAmount priceSep = _sharedCtx._diversity.getFareAmountSeparator();
    if (pqItem->getScore() > priceSep)
    {
      // No luck, clear G and U bit
      result &= ~(NEED_GOLD | NEED_UGLY);
    }
  }

  return result;
}

DmcBucketRequirement::Value
DmcBucketRequirement::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price)
{
  adjustBucketDistributionIfNeeded(price);

  const Value status = getStatus();
  if (!status)
    return 0;

  const ItinStatistic& stats = _sharedCtx._stats;

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_sharedCtx._trx, comb, &outbound, &inbound);
  Diversity::BucketType bucket = stats.detectBucket(outbound, inbound, price);

  const Value combCouldSatisfy = toDmcValue(bucket);
  return (status & combCouldSatisfy);
}

void
DmcBucketRequirement::setNumberSolutionsRequiredCollected(size_t numberOfSolutionsRequired,
                                                          size_t numberOfSolutionsCollected,
                                                          MoneyAmount score)
{
  if (numberOfSolutionsRequired == numberOfSolutionsCollected)
  {
    adjustBucketDistributionIfNeeded(score);
  }
}

int
DmcBucketRequirement::getBucketStatus(const Diversity::BucketType bucket) const
{
  const Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;

  return stats.getBucketSize(bucket) - (size_t)(diversity.getBucketDistribution(bucket) *
                                                diversity.getNumberOfOptionsToGenerate());
}

void
DmcBucketRequirement::print()
{
  DiagCollector& dc = *_sharedCtx._dc;
  const Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;
  const size_t numberOfSolutionsRequired = _sharedCtx._diversity.getNumberOfOptionsToGenerate();

  dc << "\tG: " << std::setw(3) << stats.getBucketSize(Diversity::GOLD) << "/"
     << (size_t)(diversity.getBucketDistribution(Diversity::GOLD) * numberOfSolutionsRequired);
  dc << "     U: " << std::setw(3) << stats.getBucketSize(Diversity::UGLY) << "/"
     << (size_t)(diversity.getBucketDistribution(Diversity::UGLY) * numberOfSolutionsRequired);
  dc << "     L: " << std::setw(3) << stats.getBucketSize(Diversity::LUXURY) << "/"
     << (size_t)(diversity.getBucketDistribution(Diversity::LUXURY) * numberOfSolutionsRequired);
  dc << "     J: " << std::setw(3) << stats.getBucketSize(Diversity::JUNK) << "/"
     << (size_t)(diversity.getBucketDistribution(Diversity::JUNK) * numberOfSolutionsRequired);
}

bool
DmcBucketRequirement::getSopCouldSatisfy(unsigned legId,
                                         const SOPInfo& sopInfo,
                                         const SopInfosStatistics& statistics) const
{
  const Value status = getStatus();
  if (UNLIKELY(status == 0))
  {
    return true;
  }

  int32_t duration = 0;
  const std::vector<int32_t>& flightsTime = statistics.minimumFlightTimeMinutes;
  for (unsigned currentLegId = 0; currentLegId < flightsTime.size(); ++currentLegId)
  {
    if (currentLegId == legId)
    {
      const ShoppingTrx::SchedulingOption& sop =
          SopCombinationUtil::getSop(_sharedCtx._trx, legId, sopInfo._sopIndex);
      duration += SopCombinationUtil::getDuration(sop);
    }
    else
    {
      duration += flightsTime[currentLegId];
    }
  }

  const Diversity::BucketType sopBucket =
      _sharedCtx._stats.detectBucket(duration, statistics.score);
  const Value sopStatus = toDmcValue(sopBucket);
  return status & sopStatus;
}

void
DmcBucketRequirement::adjustBucketDistributionIfNeeded(MoneyAmount score)
{
  if (!_sharedCtx._trx.getRequest()->isBrandedFaresRequest() && !_isDistributionAdjusted &&
      score > _sharedCtx._diversity.getFareAmountSeparator())
    adjustBucketDistributionUnconditionally();
}

void
DmcBucketRequirement::adjustBucketDistributionUnconditionally()
{
  // We have reached price separator. That means we can't generate any golds and uglies more.
  // We will adjust bucket diversity requirements to real state

  Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;
  DiagCollector* dc = _sharedCtx._dc;

  if (dc)
  {
    *dc << "Buckets adjustment: Fare amount separator reached\n";
    *dc << "\tFrom:";
    _sharedCtx.printRequirements(true);
  }

  {
    bool isReallyAdjusted = false;

    // Decrease requirement for GOLD to real state if needed
    if (getOptionsNeededForBucket(Diversity::GOLD) != 0)
    {
      diversity.setBucketDistribution(Diversity::GOLD,
                                      ((float)stats.getBucketSize(Diversity::GOLD)) /
                                          ((float)diversity.getNumberOfOptionsToGenerate()));
      isReallyAdjusted = true;
    }

    // Decrease requirement for UGLY to real state if needed
    if (getOptionsNeededForBucket(Diversity::UGLY) != 0)
    {
      diversity.setBucketDistribution(Diversity::UGLY,
                                      ((float)stats.getBucketSize(Diversity::UGLY)) /
                                          ((float)diversity.getNumberOfOptionsToGenerate()));
      isReallyAdjusted = true;
    }

    // Adjust requirement for LUXURY accordingly
    if (isReallyAdjusted)
    {
      diversity.setBucketDistribution(Diversity::LUXURY,
                                      1.0 - diversity.getBucketDistribution(Diversity::GOLD) -
                                          diversity.getBucketDistribution(Diversity::UGLY) -
                                          diversity.getBucketDistribution(Diversity::JUNK));
    }
  }

  _isDistributionAdjusted = true;

  if (dc)
  {
    *dc << "\tTo:";
    _sharedCtx.printRequirements(true);
  }
}

size_t
DmcBucketRequirement::getOptionsNeededForBucket(const Diversity::BucketType bucket) const
{
  const Diversity& diversity = _sharedCtx._diversity;
  const ItinStatistic& stats = _sharedCtx._stats;

  size_t minBucketSize = (size_t)(_sharedCtx._diversity.getBucketDistribution(bucket) *
                                  diversity.getNumberOfOptionsToGenerate());

  if (minBucketSize < stats.getBucketSize(bucket))
    return 0;

  return (minBucketSize - stats.getBucketSize(bucket));
}

DmcBucketRequirement::Value
DmcBucketRequirement::toDmcValue(const Diversity::BucketType bucket)
{
  switch (bucket)
  {
  case Diversity::GOLD:
    return NEED_GOLD;

  case Diversity::UGLY:
    return NEED_UGLY;

  case Diversity::LUXURY:
    return NEED_LUXURY;

  case Diversity::JUNK:
    return NEED_JUNK;

  default:
    break;
  }

  assert(false);
  return 0;
}

} // ns tse
