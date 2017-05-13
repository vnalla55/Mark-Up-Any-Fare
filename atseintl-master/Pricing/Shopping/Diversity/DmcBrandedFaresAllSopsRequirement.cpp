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

#include "Pricing/Shopping/Diversity/DmcBrandedFaresAllSopsRequirement.h"

#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <set>

namespace tse
{


DmcBrandedFaresAllSopsRequirement::DmcBrandedFaresAllSopsRequirement(
    DmcRequirementsSharedContext& sharedCtx)
  : _sharedCtx(sharedCtx), _isOneWay(sharedCtx._trx.legs().size() == 1)
{
  _enabled = _sharedCtx._trx.getRequest()->isBrandedFaresRequest();
}

DmcRequirement::Value
DmcBrandedFaresAllSopsRequirement::getStatus() const
{
  if (!_enabled)
  {
    return 0;
  }
  const ItinStatistic& stats = _sharedCtx._stats;
  Value result = 0;
  if (stats.getUnusedSopIds(0).size() > 0)
    result |= DmcRequirement::NEED_OUTBOUNDS;
  if (!_isOneWay && stats.getUnusedSopIds(1).size() > 0)
    result |= DmcRequirement::NEED_INBOUNDS;
  return result;
}

DmcRequirement::Value
DmcBrandedFaresAllSopsRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  return getStatus();
}

DmcRequirement::Value
DmcBrandedFaresAllSopsRequirement::getCombinationCouldSatisfy(shpq::SopIdxVecArg comb,
                                                              MoneyAmount price) const
{
  if (!_enabled)
  {
    return 0;
  }
  Value result = 0;
  if (isSopIdUnused(0, comb[0]))
  {
    result |= DmcRequirement::NEED_OUTBOUNDS;
  }
  if (!_isOneWay && comb.size() > 1)
  {
    if (isSopIdUnused(1, comb[1]))
    {
      result |= DmcRequirement::NEED_INBOUNDS;
    }
  }
  return result;
}

bool
DmcBrandedFaresAllSopsRequirement::isSopIdUnused(size_t legIdx, size_t sopId) const
{
  const ItinStatistic::UnusedSopIds& unusedSopIds = _sharedCtx._stats.getUnusedSopIds(legIdx);
  return (unusedSopIds.find(sopId) != unusedSopIds.end());
}

} // ns tse
