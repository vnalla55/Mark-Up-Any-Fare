#include "Routing/IsInLoc.h"

#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DBAccess/TpdPsr.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteItem.h"

#include <vector>

namespace tse
{
/**
 * Unary predicate returning true if given item's destination is in the stored location.
 * It also checks stopoverNotAllowed accordance.
 * If item fails only because stopover does not match, cond is set to true.
 */
IsInLoc::IsInLoc(const TpdPsrViaGeoLoc& loc, bool& cond, bool origLoc1)
  : _loc(loc), _cond(cond), _origLoc1(origLoc1)
{
}

bool
IsInLoc::
operator()(MileageRouteItem& item)
{
  if (!item.isDestinationInLoc(_loc.loc()))
    return false;

  bool passLoc = true;
  int i = 0; // indicates if "stopoverNotAllowed" must be checked

  if (_loc.noStopBtwViaAndLoc1() == YES || _loc.noStopBtwViaAndLoc2() == YES)
    i++;
  bool isFirstOccurrence =
      (_origLoc1) ? item.isFirstOccurrenceFromRouteBegin() : item.isLastOccurrenceToRouteEnd();
  bool isLastOccurrence =
      (_origLoc1) ? item.isLastOccurrenceToRouteEnd() : item.isFirstOccurrenceFromRouteBegin();
  bool isStopover = (item.isStopover() || item.forcedStopOver() == 'T');

  if (_loc.reqDirectSvcBtwViaAndLoc1() == YES)
  {
    passLoc = (_origLoc1) ? item.isDirectFromRouteBegin() : item.isDirectToRouteEnd();
    if (!passLoc)
    {
      if (isFirstOccurrence)
        item.failedDirService() = true;
    }
  }
  if (passLoc && _loc.reqDirectSvcBtwViaAndLoc2() == YES)
  {
    passLoc = (_origLoc1) ? item.isDirectToRouteEnd() : item.isDirectFromRouteBegin();
    if (!passLoc)
    {
      if (isLastOccurrence)
        item.failedDirService() = true;
    }
  }
  if (passLoc && _loc.noStopBtwViaAndLoc1() == YES)
  {
    passLoc = (isFirstOccurrence && (!isStopover || item.forcedConx() == 'T'));
  }
  if (passLoc && _loc.noStopBtwViaAndLoc2() == YES)
  {
    passLoc = (isLastOccurrence && (!isStopover || item.forcedConx() == 'T'));
  }
  // Check "stopoverNotAllowed" only if the above fields were not coded
  if (i == 0 && passLoc && _loc.stopoverNotAllowed() == STPOVRNOTALWD_YES)
  {
    passLoc = (!isStopover || item.forcedConx() == 'T');
  }
  if (LIKELY(passLoc))
    return true;

  if (!_cond)
    _cond = true;
  return false;
}
bool
IsInLoc::conditional()
{
  return _cond;
}
}
