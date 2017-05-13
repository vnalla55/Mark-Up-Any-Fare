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

#include "Pricing/Shopping/Diversity/DmcAtLeastOneNSRequirement.h"

#include "Pricing/Shopping/PQ/ItinStatistic.h"

namespace tse
{

DmcAtLeastOneNSRequirement::DmcAtLeastOneNSRequirement(DmcRequirementsSharedContext& sharedCtx)
  : _isFareCutoffReached(false), _sharedCtx(sharedCtx)
{
}

DmcAtLeastOneNSRequirement::Value
DmcAtLeastOneNSRequirement::getStatus() const
{
  const ItinStatistic& stats = _sharedCtx._stats;

  if (!_isFareCutoffReached)
  {
    if (stats.getOnlineNonStopsCount() == 0)
      return NEED_NONSTOPS;
  }
  else
  {
    if (stats.getOnlineNonStopsCount() == 0)
      return NEED_NONSTOPS;
  }

  return 0;
}

} // ns tse
