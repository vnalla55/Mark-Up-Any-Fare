#include "Routing/SurfaceSectorExemptRetriever.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/SurfaceSectorExempt.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
// SurfaceSectorExepmtRetriever::retrieve()- Retrieves substituion mkt for
//---------------------------------------------------------------------------

bool
SurfaceSectorExemptRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                       DataHandle& dataHandle,
                                       Indicator mileageType) const
{
  // try board and off point
  const SurfaceSectorExempt* isExempt = getData(dataHandle,
                                                mileageRouteItem.city1()->loc(),
                                                mileageRouteItem.city2()->loc(),
                                                mileageRouteItem.travelDate());

  if (isExempt == nullptr)
  {
    // try off and board point
    isExempt = getData(dataHandle,
                       mileageRouteItem.city2()->loc(),
                       mileageRouteItem.city1()->loc(),
                       mileageRouteItem.travelDate());

    if (isExempt == nullptr)
    {
      // try to use MultiTransport cities
      const Loc* multiTransportOrigin = mileageRouteItem.multiTransportOrigin();
      const Loc* multiTransportDestination = mileageRouteItem.multiTransportDestination();
      if ((multiTransportOrigin != nullptr) && (multiTransportDestination != nullptr) &&
          ((multiTransportOrigin->loc() != mileageRouteItem.city1()->loc()) ||
           (multiTransportDestination->loc() != mileageRouteItem.city2()->loc())))
      {
        isExempt = getData(dataHandle,
                           multiTransportOrigin->loc(),
                           multiTransportDestination->loc(),
                           mileageRouteItem.travelDate());

        if (isExempt == nullptr)
        {
          isExempt = getData(dataHandle,
                             multiTransportDestination->loc(),
                             multiTransportOrigin->loc(),
                             mileageRouteItem.travelDate());
        }
      }
    }
  }

  if (isExempt)
  {
    mileageRouteItem.tpmSurfaceSectorExempt() = true;
    return true;
  }

  return false;
}

const SurfaceSectorExempt*
SurfaceSectorExemptRetriever::getData(DataHandle& dataHandle,
                                      const LocCode& loc1,
                                      const LocCode& loc2,
                                      const DateTime& travelDate) const
{
  return dataHandle.getSurfaceSectorExempt(loc1, loc2, travelDate);
}
}
