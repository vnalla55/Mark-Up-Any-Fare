#include "Routing/TPMCollectorWNfromItin.h"

#include "Common/LocUtil.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MileageRoute.h"
#include "Routing/Retriever.h"
#include "Routing/SurfaceSectorExemptRetriever.h"
#include "Routing/TPMRetriever.h"

namespace tse
{
TPMCollectorWNfromItin::~TPMCollectorWNfromItin() {}

bool
TPMCollectorWNfromItin::collectMileage(MileageRoute& mileageRoute) const
{
  DataHandle& dataHandle(*mileageRoute.dataHandle());
  MileageRouteItems::iterator beg(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::iterator itr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::iterator end(mileageRoute.mileageRouteItems().end());
  // for each route segment retrieve TPM
  for (; itr != end; ++itr)
  {
    MileageRouteItem& item(*itr);
    // do not retrieve TPM for airports within the same multi-airport city
    if (item.origCityOrAirport()->loc() == item.destCityOrAirport()->loc())
      continue;
    getTPM(item, dataHandle);
    // for segments different from first and last, check TPM Surface Segment Exemption
    if (itr != beg && itr != end - 1)
    {
      getSurfaceSector(item, dataHandle);
    }
    if (!itr->tpmSurfaceSectorExempt())
    {
      mileageRoute.mileageRouteTPM() += item.tpm();
    }
  }
  return true;
}

bool
TPMCollectorWNfromItin::getTPM(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<TPMRetriever>& tpmRetriever(tse::Singleton<Retriever<TPMRetriever> >::instance());
  return tpmRetriever.retrieve(item, dataHandle);
}

bool
TPMCollectorWNfromItin::getSurfaceSector(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const SurfaceSectorExemptRetriever& surfaceSectorExempt(
      tse::Singleton<SurfaceSectorExemptRetriever>::instance());
  return surfaceSectorExempt.retrieve(item, dataHandle);
}
}
