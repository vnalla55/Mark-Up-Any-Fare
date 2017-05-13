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

#include "Pricing/Shopping/Diversity/DmcMoreNSRequirement.h"

#include "Pricing/Shopping/PQ/ItinStatistic.h"

namespace tse
{

DmcMoreNSRequirement::DmcMoreNSRequirement(DmcRequirementsSharedContext& sharedCtx)
  : _sharedCtx(sharedCtx)
{
}

DmcMoreNSRequirement::Value
DmcMoreNSRequirement::getStatus() const
{
  const ItinStatistic& stats = _sharedCtx._stats;
  const Diversity& diversity = _sharedCtx._diversity;

  if (!diversity.isAdditionalNonStopsEnabled())
    return 0;

  if (stats.getNonStopsCount() + stats.getAdditionalNonStopsCount() <
          diversity.getMaxOnlineNonStopCount() + diversity.getMaxInterlineNonStopCount() &&
      stats.getAdditionalNonStopsCount() < diversity.getNonStopOptionsCount())
    return NEED_ADDITIONAL_NONSTOPS;

  return 0;
}

DmcMoreNSRequirement::Value
DmcMoreNSRequirement::getCouldSatisfyAdjustment(Value allInOneRequirements) const
{
  if (allInOneRequirements != 0)
    return 0;

  return getStatus();
}

} // ns tse
