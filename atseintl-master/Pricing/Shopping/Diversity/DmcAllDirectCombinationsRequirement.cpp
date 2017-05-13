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

#include "Pricing/Shopping/Diversity/DmcAllDirectCombinationsRequirement.h"

#include "DataModel/Billing.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

#include <set>

namespace tse
{

DmcAllDirectCombinationsRequirement::DmcAllDirectCombinationsRequirement(
    DmcRequirementsSharedContext& sharedCtx)
  : _sharedCtx(sharedCtx)
{
  _enabled = _sharedCtx._trx.getRequest()->isAllFlightsRepresented();
}

DmcRequirement::Value
DmcAllDirectCombinationsRequirement::getStatus() const
{
  if (!_enabled)
  {
    return 0;
  }
  const ItinStatistic& stats = _sharedCtx._stats;
  Value result = 0;
  if (stats.getMissingDirectOptionsCount() > 0)
  {
    result |= DmcRequirement::NEED_IBF_DIRECTS;
  }

  return result;
}

DmcRequirement::Value
DmcAllDirectCombinationsRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  return getStatus();
}

DmcRequirement::Value
DmcAllDirectCombinationsRequirement::getCombinationCouldSatisfy(const shpq::SopIdxVecArg comb,
                                                               MoneyAmount price) const
{
  if (!_enabled)
  {
    return 0;
  }
  Value result = 0;

  const ShoppingTrx::SchedulingOption* outbound = nullptr, *inbound = nullptr;
  SopCombinationUtil::getSops(_sharedCtx._trx, comb, &outbound, &inbound);

  if ((outbound->itin()->travelSeg().size() == 1) &&
      (inbound && (inbound->itin()->travelSeg().size() == 1)) &&
       getStatus())
  {
    result |= DmcRequirement::NEED_IBF_DIRECTS;
  }

  return result;
}

} // ns tse
