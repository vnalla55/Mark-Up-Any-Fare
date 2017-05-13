// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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
#include "ItinAnalyzer/SoloJourney.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ShoppingUtil.h"

namespace tse
{
namespace
{
ConfigurableValue<float>
avsLocalToFlowCosSizeRatio("SHOPPING_DIVERSITY", "AVS_LOCAL_TO_FLOW_COS_SIZE_RATIO", 1.25f);
}

SoloJourney::SoloJourney(const PricingTrx& trx, const Itin* itin, size_t startIdx)
  : _trx(&trx),
    _itin(itin),
    _range(startIdx, startIdx),
    _avsLocalToFlowCosSizeRatio(avsLocalToFlowCosSizeRatio.getValue())
{
}

void
SoloJourney::endJourney(size_t endIdx)
{
  _range = SegmentRange(_range.getStartIdx(), endIdx);
}

SoloJourney::COSList*
SoloJourney::getAvl(const SegmentRange& avlRange, const size_t segIdx, const SOPUsage& sopUsage)
    const
{
  std::vector<COSList>* flowAvlVector = nullptr;
  COSList* result(nullptr), *localResult(nullptr);

  AvlCI it = _trx->availabilityMap().find(buildAvlKey(avlRange));

  if (LIKELY(it != _trx->availabilityMap().end()))
  {
    const int localIndex = segIdx - avlRange.getStartIdx();
    flowAvlVector = it->second;
    result = &(flowAvlVector->at(localIndex));
  }

  {
    SegmentRange localRange(segIdx, segIdx + 1);
    AvlCI localIt = _trx->availabilityMap().find(buildAvlKey(localRange));

    if (LIKELY(localIt != _trx->availabilityMap().end()))
      localResult = &(localIt->second->at(0));

    if (isAA13HoursConnection(segIdx, sopUsage))
    {
      result = localResult;
    }
    else if (result && localResult && localResult->size() > result->size())
    {
      if (isFlowAvailFromAvs(avlRange, *flowAvlVector))
        result = localResult;
    }
  }

  if (UNLIKELY(!result))
    // we may not get all availability type from ASv2 (ASv2 req.
    // ASO/@P50 = "C" (custom) or "R" (interline logic)).
    // In such case return merge avail.
    result = &(_itin->travelSeg()[segIdx]->classOfService());

  return result;
}

uint64_t
SoloJourney::buildAvlKey(const SegmentRange& avlRange) const
{
  uint64_t key = 0;
  size_t shift = 0; // segIdx can not be used (it may not start from 0)
  for (size_t segIdx = avlRange.getStartIdx(); segIdx < avlRange.getEndIdx(); ++segIdx, ++shift)
  {
    key += (((uint64_t)(_itin->travelSeg()[segIdx]->originalId())) << (shift * 16));
  }
  return key;
}

bool
SoloJourney::isAA13HoursConnection(const size_t segIdx, const SOPUsage& sopUsage) const
{
  AirSeg* currAirSeg;
  AirSeg* nextAirSeg;

  if ((segIdx >= sopUsage.itin_->travelSeg().size()) || // prevent array out of boundary
      (sopUsage.itin_->travelSeg().size() <= 1)) // it's already local availability
  {
    return false;
  }

  if (segIdx == (sopUsage.itin_->travelSeg().size() - 1)) // special case for last segment
  {
    currAirSeg = dynamic_cast<AirSeg*>(sopUsage.itin_->travelSeg()[segIdx - 1]);
    nextAirSeg = dynamic_cast<AirSeg*>(sopUsage.itin_->travelSeg()[segIdx]);
  }
  else
  {
    currAirSeg = dynamic_cast<AirSeg*>(sopUsage.itin_->travelSeg()[segIdx]);
    nextAirSeg = dynamic_cast<AirSeg*>(sopUsage.itin_->travelSeg()[segIdx + 1]);
  }

  if ((currAirSeg->carrier() == SPECIAL_CARRIER_AA) &&
      (nextAirSeg->carrier() == SPECIAL_CARRIER_AA))
  {
    // check if connection time is more than 13 hours. 46800=(13x3600)mins
    if (nextAirSeg->isStopOverWithOutForceCnx(currAirSeg, 46800))
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool
SoloJourney::isFlowAvailFromAvs(const SegmentRange& range, const std::vector<COSList>& flowAvl)
    const
{
  // If flow availability comes from AVS whereas local from DCA or cache data, we prefer local
  // classes even if the carrier is flow. Unfortunately, we can't be sure about the source of
  // availability sent to IS. Therefore, we rely on assumption that AVS availability consists of
  // significantly fewer classes than DCA or cache data.

  if (UNLIKELY(range.getSize() <= 1))
    return false;

  const float avgNumFlowClasses = calcAvgNumFlowClasses(flowAvl);
  if (UNLIKELY(avgNumFlowClasses < EPSILON))
    return true;

  const size_t minNumLocalClasses = calcMinNumLocalClasses(range);
  const float localToFlowRatio = static_cast<float>(minNumLocalClasses) / avgNumFlowClasses;

  return localToFlowRatio > _avsLocalToFlowCosSizeRatio;
}

size_t
SoloJourney::calcMinNumLocalClasses(const SegmentRange& range) const
{
  size_t minNumLocalClasses = std::numeric_limits<size_t>::max();

  for (size_t curIdx = range.getStartIdx(); curIdx < range.getEndIdx(); ++curIdx)
  {
    const uint64_t localAvlKey = buildAvlKey(SegmentRange(curIdx, curIdx + 1));
    const AvlCI localAvlCI = _trx->availabilityMap().find(localAvlKey);

    if (UNLIKELY(localAvlCI == _trx->availabilityMap().end()))
      return 0;

    const COSList& cosList = localAvlCI->second->at(0);
    minNumLocalClasses = std::min(minNumLocalClasses, cosList.size());
  }

  return minNumLocalClasses;
}

float
SoloJourney::calcAvgNumFlowClasses(const std::vector<COSList>& flowAvl) const
{
  if (UNLIKELY(flowAvl.empty()))
    return 0.0f;

  size_t sum = 0;

  for (const COSList& cosList : flowAvl)
  {
    sum += cosList.size();
  }

  return static_cast<float>(sum) / static_cast<float>(flowAvl.size());
}

} // namespace tse
