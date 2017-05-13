#include "Routing/TPMRetriever.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRetriever.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageSubstitutionRetriever.h"
#include "Routing/TPMConstructor.h"

namespace tse
{
TPMRetriever::~TPMRetriever() {}

/* Algorithm for TPM retrieval. Try to retrieve mileage data from subsequent data sources
   until successful. */
bool
TPMRetriever::retrieve(MileageRouteItem& mileageRouteItem, DataHandle& dataHandle) const
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
  //    bool marketSubstituted;

  if (getMileageSubstitution(itemCopy, dataHandle) && getMileage(itemCopy, dataHandle))
  {
    // if successful for substituted market, copy the result to the original mileageRouteItem
    mileageRouteItem.tpm() = itemCopy.tpm();
    return true;
  }
  // if unsuccessful at retrieving TPM from Mileage, try to construct it
  return getConstructed(mileageRouteItem, dataHandle);
}

bool
TPMRetriever::getMileage(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageRetriever(tse::Singleton<MileageRetriever>::instance());
  return mileageRetriever.retrieve(item, dataHandle, TPM);
}

bool
TPMRetriever::getMileageSubstitution(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageSubstitution(
      tse::Singleton<MileageSubstitutionRetriever>::instance());
  return mileageSubstitution.retrieve(item, dataHandle);
}

bool
TPMRetriever::getConstructed(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& tpmConstructor(tse::Singleton<TPMConstructor>::instance());
  return tpmConstructor.retrieve(item, dataHandle);
}
}
