#include "Routing/MPMCollectorMV.h"

#include "Routing/MileageRoute.h"
#include "Routing/MPMRetriever.h"
#include "Routing/Retriever.h"

namespace tse
{

MPMCollectorMV::~MPMCollectorMV() {}

bool
MPMCollectorMV::collectMileage(MileageRoute& mileageRoute) const
{
  // create city pair of global origin and destination
  MileageRouteItem mileageRouteItem(mileageRoute.mileageRouteItems().front());
  mileageRouteItem.city2() = mileageRoute.mileageRouteItems().back().city2();
  mileageRouteItem.multiTransportDestination() =
      mileageRoute.mileageRouteItems().back().multiTransportDestination();
  mileageRouteItem.segmentCarrier() = mileageRoute.governingCarrier();
  // retrieve MPM for so prepared market and copy the result into original mileageRoute
  if (getMPM(mileageRouteItem, *mileageRoute.dataHandle()))
  {
    mileageRoute.mileageRouteMPM() = mileageRouteItem.mpm();
    return true;
  }
  return false;
}

bool
MPMCollectorMV::getMPM(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<MPMRetriever>& mpmRetriever(tse::Singleton<Retriever<MPMRetriever> >::instance());
  return mpmRetriever.retrieve(item, dataHandle);
}
}
