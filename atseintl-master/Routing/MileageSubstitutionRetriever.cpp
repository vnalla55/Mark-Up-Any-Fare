#include "Routing/MileageSubstitutionRetriever.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MileageSubstitution.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
// MileageSubstitutionRetriever::retrieve()- Retrieves substituion mkt for
//                                           mileage retrieval
//---------------------------------------------------------------------------

bool
MileageSubstitutionRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                       DataHandle& dataHandle,
                                       Indicator mileageType) const
{

  const MileageSubstitution* mileageSubst1 =
      getData(dataHandle, mileageRouteItem.city1()->loc(), mileageRouteItem.travelDate());
  const MileageSubstitution* mileageSubst2 =
      getData(dataHandle, mileageRouteItem.city2()->loc(), mileageRouteItem.travelDate());

  if (mileageSubst1 != nullptr)
  {
    mileageRouteItem.city1()->loc() = mileageSubst1->publishedLoc();
  }

  if (mileageSubst2 != nullptr)
  {
    mileageRouteItem.city2()->loc() = mileageSubst2->publishedLoc();
  }
  return (mileageSubst1 != nullptr || mileageSubst2 != nullptr);
}

const MileageSubstitution*
MileageSubstitutionRetriever::getData(DataHandle& dataHandle,
                                      const LocCode& loc,
                                      const DateTime& travelDate) const
{
  return dataHandle.getMileageSubstitution(loc, travelDate);
}
}
