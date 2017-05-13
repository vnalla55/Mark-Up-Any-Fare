#include "Routing/TPMConstructor.h"

#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MPMRetriever.h"
#include "Routing/Retriever.h"

namespace tse
{
//---------------------------------------------------------------------------
// TPMConstructor::retrieve() --- constructs TPM either from MPM or by using GCM.
//---------------------------------------------------------------------------

bool
TPMConstructor::retrieve(MileageRouteItem& mileageRouteItem,
                         DataHandle& dataHandle,
                         Indicator mileageType) const
{
  mileageRouteItem.isConstructed() = true;
  MileageRouteItem itemCopy(mileageRouteItem);

  itemCopy.globalDirection(MPM) = itemCopy.globalDirection(TPM);
  itemCopy.mpm() = 0;

  if (getData(itemCopy, dataHandle) || itemCopy.mpm() > 0)
  {
    mileageRouteItem.tpm() = static_cast<uint16_t>(TseUtil::getTPMFromMPM(itemCopy.mpm()));
  }
  else
  {
    if ((mileageRouteItem.city1()->loc() == NEWARK && mileageRouteItem.city2()->loc() == NEWYORK) ||
        (mileageRouteItem.city2()->loc() == NEWARK && mileageRouteItem.city1()->loc() == NEWYORK))
    {
      mileageRouteItem.tpm() = 0;
    }
    else
    {
      mileageRouteItem.tpm() = static_cast<uint16_t>(TseUtil::greatCircleMiles(
          *mileageRouteItem.origCityOrAirport(), *mileageRouteItem.destCityOrAirport()));
    }
  }
  return true;
}

bool
TPMConstructor::getData(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<MPMRetriever>& mpmRetriever(tse::Singleton<Retriever<MPMRetriever> >::instance());
  return mpmRetriever.retrieve(item, dataHandle);
}
}
