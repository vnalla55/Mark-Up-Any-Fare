#include "Routing/MileageRetriever.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
// MileageRetriever::retrieve() --- Retrieves Mileage( MPM/TPM) from Mileage Table
//---------------------------------------------------------------------------

bool
MileageRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                           DataHandle& dataHandle,
                           Indicator mileageType) const
{
  const Mileage* mileage = getData(dataHandle,
                                   mileageRouteItem.city1()->loc(),
                                   mileageRouteItem.city2()->loc(),
                                   mileageType,
                                   mileageRouteItem.globalDirection(mileageType),
                                   mileageRouteItem.travelDate());

  Loc* multiTransportOrigin = mileageRouteItem.multiTransportOrigin();
  Loc* multiTransportDestination = mileageRouteItem.multiTransportDestination();
  if (mileage == nullptr && multiTransportDestination != nullptr)
    mileage = getData(dataHandle,
                      mileageRouteItem.city1()->loc(),
                      multiTransportDestination->loc(),
                      mileageType,
                      mileageRouteItem.globalDirection(mileageType),
                      mileageRouteItem.travelDate());
  if (mileage == nullptr && multiTransportOrigin != nullptr)
    mileage = getData(dataHandle,
                      multiTransportOrigin->loc(),
                      mileageRouteItem.city2()->loc(),
                      mileageType,
                      mileageRouteItem.globalDirection(mileageType),
                      mileageRouteItem.travelDate());
  if (mileage == nullptr && multiTransportOrigin != nullptr && multiTransportDestination != nullptr)
    mileage = getData(dataHandle,
                      multiTransportOrigin->loc(),
                      multiTransportDestination->loc(),
                      mileageType,
                      mileageRouteItem.globalDirection(mileageType),
                      mileageRouteItem.travelDate());
  if (mileage != nullptr)
  {
    mileageRouteItem.mileage(mileageType) = static_cast<uint16_t>(mileage->mileage());
    return true;
  }
  return false;
}

const Mileage*
MileageRetriever::getData(DataHandle& dataHandle,
                          const LocCode& loc1,
                          const LocCode& loc2,
                          Indicator mileageType,
                          GlobalDirection globalDirection,
                          const DateTime& travelDate) const
{
  return dataHandle.getMileage(loc1, loc2, mileageType, globalDirection, travelDate);
}
}
