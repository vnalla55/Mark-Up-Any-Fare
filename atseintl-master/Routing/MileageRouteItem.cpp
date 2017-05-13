#include "Routing/MileageRouteItem.h"

#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"

namespace tse
{
static bool
isInLoc(const Loc& loc, const LocKey& locKey)
{
  return LocUtil::isInLoc(loc, locKey.locType(), locKey.loc(), Vendor::SABRE, MANUAL);
}

static bool
isInLoc(const Loc* loc, const Loc* multiTransportLoc, const LocKey& locKey)
{
  if (isInLoc(*loc, locKey))
    return true;
  return (multiTransportLoc && isInLoc(*multiTransportLoc, locKey));
}

bool
MileageRouteItem::isOriginInLoc(const LocKey& locKey) const
{
  return isInLoc(city1(), multiTransportOrigin(), locKey);
}

bool
MileageRouteItem::isDestinationInLoc(const LocKey& locKey) const
{
  return isInLoc(city2(), multiTransportDestination(), locKey);
}

void
MileageRouteItem::clone(DataHandle& dataHandle, MileageRouteItem& itemCopy)
{
  dataHandle.get(itemCopy._city1);
  *itemCopy._city1 = *_city1;
  dataHandle.get(itemCopy._city2);
  *itemCopy._city2 = *_city2;
  itemCopy._travelDate = _travelDate;
  itemCopy._isSurface = _isSurface;
  itemCopy._isConstructed = _isConstructed;
  itemCopy._tpmGlobalDirection = _tpmGlobalDirection;
  itemCopy._mpmGlobalDirection = _mpmGlobalDirection;
  itemCopy._tpm = _tpm;
  itemCopy._mpm = _mpm;
  itemCopy._tpd = _tpd;
  itemCopy._southAtlanticExclusion = _southAtlanticExclusion;
  itemCopy._tpmSurfaceSectorExempt = _tpmSurfaceSectorExempt;
  itemCopy._segmentCarrier = _segmentCarrier;
  itemCopy._isStopover = _isStopover;
  itemCopy._isDirectFromRouteBegin = _isDirectFromRouteBegin;
  itemCopy._isDirectToRouteEnd = _isDirectToRouteEnd;
  itemCopy._isFirstOccurrenceFromRouteBegin = _isFirstOccurrenceFromRouteBegin;
  itemCopy._isLastOccurrenceToRouteEnd = _isLastOccurrenceToRouteEnd;
  itemCopy._failedDirService = _failedDirService;
  itemCopy._pnrSegment = _pnrSegment;
  itemCopy._forcedStopOver = _forcedStopOver;
  if (!_hiddenLocs.empty())
  {
    itemCopy._hiddenLocs.clear();
    itemCopy._hiddenLocs.reserve(_hiddenLocs.size());
    std::vector<const Loc*>::const_iterator itr(_hiddenLocs.begin());
    std::vector<const Loc*>::const_iterator end(_hiddenLocs.end());
    for (; itr != end; ++itr)
    {
      Loc* loc = nullptr;
      dataHandle.get(loc); // lint --e{413}
      *loc = **itr;
      itemCopy._hiddenLocs.push_back(loc);
    }
  }
  itemCopy._psrApplies = _psrApplies;
  itemCopy._psrMayApply = _psrMayApply;
  itemCopy._psrStopNotAllowed = _psrStopNotAllowed;
  // deep copy of tpdViaGeoLocs not needed as of now
  // should such a need appear, the followitn code must be changed
  itemCopy._condTpdViaGeoLocs = _condTpdViaGeoLocs;
  if (_multiTransportOrigin != nullptr)
  {
    dataHandle.get(itemCopy._multiTransportOrigin);
    *itemCopy._multiTransportOrigin = *_multiTransportOrigin;
  }
  if (_multiTransportDestination != nullptr)
  {
    dataHandle.get(itemCopy._multiTransportDestination);
    *itemCopy._multiTransportDestination = *_multiTransportDestination;
  }
}
}
