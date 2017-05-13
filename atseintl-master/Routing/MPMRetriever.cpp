#include "Routing/MPMRetriever.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/AdditionalMileageRetriever.h"
#include "Routing/MileageRetriever.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageSubstitutionRetriever.h"

namespace tse
{
MPMRetriever::~MPMRetriever() {}

/* Algorithm for MPM retrieval. Try to retrieve mileage data from subsequent data sources
   until successful. */
bool
MPMRetriever::retrieve(MileageRouteItem& mileageRouteItem, DataHandle& dataHandle) const
{
  // try to retrieve data directly from Mileage table. If successful, we are done.
  if (getMileage(mileageRouteItem, dataHandle))
  {
    return true;
  }
  // try to substitute input (mainly the market) for Mileage table from subsequent tables.
  // if managed to retrieve data from Mileage table for substituted input, we are done.
  // to avoid modification of the original mileageRouteItem, work on a deep copy
  MileageRouteItem itemCopy;
  mileageRouteItem.clone(dataHandle, itemCopy);
  bool marketSubstituted;
  if ((marketSubstituted = getMileageSubstitution(itemCopy, dataHandle)) &&
      getMileage(itemCopy, dataHandle))
  {
    // if successful with substituted market, copy the result to the original mileageRouteItem
    mileageRouteItem.mpm() = itemCopy.mpm();
    return true;
  }
  // if itemCopy  changed by mileageSubstitution, copy the market from the original one
  if (marketSubstituted)
  {
    itemCopy.city1()->loc() = mileageRouteItem.city1()->loc();
    itemCopy.city2()->loc() = mileageRouteItem.city2()->loc();
  }
  if (getAdditionalMileage(itemCopy, dataHandle))

  {
    mileageRouteItem.mpm() = itemCopy.mpm();
    return true;
  }
  return false;
}

bool
MPMRetriever::getMileage(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageRetriever(tse::Singleton<MileageRetriever>::instance());
  return mileageRetriever.retrieve(item, dataHandle, MPM);
}

bool
MPMRetriever::getMileageSubstitution(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageSubstitution(
      tse::Singleton<MileageSubstitutionRetriever>::instance());
  return mileageSubstitution.retrieve(item, dataHandle);
}

bool
MPMRetriever::getAdditionalMileage(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& additionalMileage(
      tse::Singleton<AdditionalMileageRetriever>::instance());
  return additionalMileage.retrieve(item, dataHandle);
}
}
