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

#include "Pricing/Shopping/Diversity/DmcBrandedFaresTakeAllRequirement.h"

#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <set>

namespace tse
{

DmcBrandedFaresTakeAllRequirement::DmcBrandedFaresTakeAllRequirement(
    DmcRequirementsSharedContext& sharedCtx)
  : _sharedCtx(sharedCtx),
    _optionsRequested(sharedCtx._trx.getOptions()->getRequestedNumberOfSolutions())
{
  _enabled = _sharedCtx._trx.getRequest()->isBrandedFaresRequest();
}

DmcRequirement::Value
DmcBrandedFaresTakeAllRequirement::getStatus() const
{
  const ItinStatistic& stats = _sharedCtx._stats;
  if (!_enabled || !stats.ibfNeedsAny())
  {
    return 0;
  }
  Value result = 0;
  if (stats.getTotalOptionsCount() < _optionsRequested)
  {
    result |= DmcRequirement::NEED_JUNK;
  }

  return result;
}

DmcRequirement::Value
DmcBrandedFaresTakeAllRequirement::getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const
{
  return getStatus();
}

DmcRequirement::Value
DmcBrandedFaresTakeAllRequirement::getCombinationCouldSatisfy(const shpq::SopIdxVecArg comb,
                                                              MoneyAmount price) const
{
  return getStatus();
}

} // ns tse
