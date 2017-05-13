#include "Routing/TPMCollectorWN.h"

#include "Common/LocUtil.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MileageRoute.h"
#include "Routing/Retriever.h"
#include "Routing/SurfaceSectorExemptRetriever.h"
#include "Routing/TPMRetrieverWN.h"

namespace tse
{
TPMCollectorWN::~TPMCollectorWN() {}

bool
TPMCollectorWN::collectMileage(MileageRoute& mileageRoute) const
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
    if (!getTPM(item, dataHandle, mileageRoute.gdPrompt()) && mileageRoute.gdPrompt() != nullptr)
      return false;
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
TPMCollectorWN::getTPM(MileageRouteItem& item, DataHandle& dataHandle, GDPrompt*& gdPrompt) const
{
  const Retriever<TPMRetrieverWN>& tpmRetriever(
      tse::Singleton<Retriever<TPMRetrieverWN> >::instance());
  return tpmRetriever.retrieve(item, dataHandle, gdPrompt);
}

bool
TPMCollectorWN::getSurfaceSector(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const SurfaceSectorExemptRetriever& surfaceSectorExempt(
      tse::Singleton<SurfaceSectorExemptRetriever>::instance());
  return surfaceSectorExempt.retrieve(item, dataHandle);
}
}
