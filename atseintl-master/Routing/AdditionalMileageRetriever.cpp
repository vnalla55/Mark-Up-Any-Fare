#include "Routing/AdditionalMileageRetriever.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRetriever.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
// AdditionalMileageRetriever::retrieve()- Retrieves AdditionalMileage
//                                         for ConstructedFare
//---------------------------------------------------------------------------

bool
AdditionalMileageRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                     DataHandle& dataHandle,
                                     Indicator mileageType) const

{

  const TariffMileageAddon* addOnMkt1 = getData(dataHandle,
                                                mileageRouteItem.city1()->loc(),
                                                mileageRouteItem.segmentCarrier(),
                                                mileageRouteItem.globalDirection(MPM),
                                                mileageRouteItem.travelDate());

  const TariffMileageAddon* addOnMkt2 = getData(dataHandle,
                                                mileageRouteItem.city2()->loc(),
                                                mileageRouteItem.segmentCarrier(),
                                                mileageRouteItem.globalDirection(MPM),
                                                mileageRouteItem.travelDate());
  uint16_t addlMileage1 = 0;
  uint16_t addlMileage2 = 0;

  if (addOnMkt1 != nullptr)
  {
    mileageRouteItem.city1()->loc() = addOnMkt1->publishedLoc();
    addlMileage1 = static_cast<uint16_t>(addOnMkt1->milesAdded());
  }

  if (addOnMkt2 != nullptr)
  {
    mileageRouteItem.city2()->loc() = addOnMkt2->publishedLoc();
    addlMileage2 = static_cast<uint16_t>(addOnMkt2->milesAdded());
  }
  if ((addlMileage1 > 0 || addlMileage2 > 0) && getMileage(dataHandle, mileageRouteItem))
  {
    mileageRouteItem.mpm() = mileageRouteItem.mpm() + addlMileage1 + addlMileage2;
    return true;
  }

  return false;
}

const TariffMileageAddon*
AdditionalMileageRetriever::getData(DataHandle& dataHandle,
                                    const LocCode& unpublishedMkt,
                                    CarrierCode& govCxr,
                                    GlobalDirection& globalDir,
                                    const DateTime& travelDate) const

{
  // First Try with empty Carrier
  std::string emptyCxr;
  const TariffMileageAddon* mileageAddon =
      dataHandle.getTariffMileageAddon(emptyCxr, unpublishedMkt, globalDir, travelDate);
  if (mileageAddon == nullptr)
  {
    const TariffMileageAddon* mileageAddon =
        dataHandle.getTariffMileageAddon(govCxr, unpublishedMkt, globalDir, travelDate);
    return mileageAddon;
  }
  return mileageAddon;
}

bool
AdditionalMileageRetriever::getMileage(DataHandle& dataHandle, MileageRouteItem& item) const
{
  const MileageRetriever& mileageRetriever(tse::Singleton<MileageRetriever>::instance());
  return mileageRetriever.retrieve(item, dataHandle, MPM);
}
}
