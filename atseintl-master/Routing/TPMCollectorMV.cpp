#include "Routing/TPMCollectorMV.h"

#include "Common/LocUtil.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MileageRoute.h"
#include "Routing/Retriever.h"
#include "Routing/SouthAtlanticTPMExclusion.h"
#include "Routing/SurfaceSectorExemptRetriever.h"
#include "Routing/TPMRetriever.h"

namespace tse
{
TPMCollectorMV::~TPMCollectorMV() {}

bool
TPMCollectorMV::collectMileage(MileageRoute& mileageRoute) const
{
  DataHandle& dataHandle(*mileageRoute.dataHandle());
  MileageRouteItems::iterator begin(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::iterator end(mileageRoute.mileageRouteItems().end());
  MileageRouteItems::iterator itr(begin);

  // for each route segment retrieve TPM
  for (; itr != end; ++itr)
  {
    MileageRouteItem& item(*itr);
    // do not retrieve TPM for airports within the same multi-airport city
    if (item.origCityOrAirport()->loc() == item.destCityOrAirport()->loc())
      continue;
    getTPM(item, dataHandle);
    // for surface segments check TPM Surface Segment Exemption
    if (item.isSurface() && (itr != begin && itr != (end - 1))) // Surface sector not at fare break
    {
      getSurfaceSector(item, dataHandle);
    }
  }
  getSouthAtlantic(mileageRoute);
  return true;
}

bool
TPMCollectorMV::getTPM(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<TPMRetriever>& tpmRetriever(tse::Singleton<Retriever<TPMRetriever> >::instance());
  return tpmRetriever.retrieve(item, dataHandle);
}

bool
TPMCollectorMV::getSurfaceSector(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const SurfaceSectorExemptRetriever& surfaceSectorExempt(
      tse::Singleton<SurfaceSectorExemptRetriever>::instance());
  return surfaceSectorExempt.retrieve(item, dataHandle);
}

bool
TPMCollectorMV::getSouthAtlantic(MileageRoute& route) const
{
  const SouthAtlanticTPMExclusion& southAtlanticTPMExclusion(
      tse::Singleton<SouthAtlanticTPMExclusion>::instance());
  return southAtlanticTPMExclusion.apply(route);
}
}
