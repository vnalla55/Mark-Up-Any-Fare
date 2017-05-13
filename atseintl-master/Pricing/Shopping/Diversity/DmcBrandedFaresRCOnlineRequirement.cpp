// ----------------------------------------------------------------
//
//   Author: Michal Mlynek
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

#include "Pricing/Shopping/Diversity/DmcBrandedFaresRCOnlineRequirement.h"

#include "DataModel/Billing.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <set>

namespace tse
{


DmcBrandedFaresRCOnlineRequirement::DmcBrandedFaresRCOnlineRequirement(
    DmcRequirementsSharedContext& sharedCtx)
  : _sharedCtx(sharedCtx), _requestingCarrier("0")
{

  _enabled = _sharedCtx._trx.getRequest()->isBrandedFaresRequest();
  if (_enabled)
  {
    const tse::Billing* billing = sharedCtx._trx.billing();
    if (billing)
      _requestingCarrier = billing->partitionID();
  }
}

DmcRequirement::Value
DmcBrandedFaresRCOnlineRequirement::getStatus() const
{
  if (!_enabled)
  {
    return 0;
  }
  const ItinStatistic& stats = _sharedCtx._stats;
  Value result = 0;
  if (stats.getMissingRCOnlineOptionsCount() > 0)
  {
    result |= DmcRequirement::NEED_RC_ONLINES;
  }

  return result;
}

DmcRequirement::Value
DmcBrandedFaresRCOnlineRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  return getStatus();
}

DmcRequirement::Value
DmcBrandedFaresRCOnlineRequirement::getCombinationCouldSatisfy(const shpq::SopIdxVecArg comb,
                                                               MoneyAmount price) const
{
  if (!_enabled)
  {
    return 0;
  }
  Value result = 0;
  if (ShoppingUtil::isOnlineOptionForCarrier(_sharedCtx._trx, comb, _requestingCarrier) &&
      getStatus())
  {
    result |= DmcRequirement::NEED_RC_ONLINES;
  }

  return result;
}

} // ns tse
