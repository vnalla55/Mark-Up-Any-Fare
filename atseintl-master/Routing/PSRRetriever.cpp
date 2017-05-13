#include "Routing/PSRRetriever.h"

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
//  PSRRetriever::retrieve() Build a vector of PSRs to process
//---------------------------------------------------------------------------
const std::vector<TpdPsr*>&
PSRRetriever::getpsrData(MileageRoute& mileageRoute) const
{

  MileageRouteItem& board(mileageRoute.mileageRouteItems().front());
  MileageRouteItem& off(mileageRoute.mileageRouteItems().back());

  const std::vector<TpdPsr*>& psrList = getPSRData(*mileageRoute.dataHandle(),
                                                   PSR,
                                                   mileageRoute.governingCarrier(),
                                                   board.city1()->area()[0],
                                                   off.city2()->area()[0],
                                                   mileageRoute.ticketingDT(),
                                                   mileageRoute.travelDT());

  return psrList;
}

//---------------------------------------------------------------------------
//  PSRRetriever::retrieve() Retrieve PSRs
//---------------------------------------------------------------------------
const std::vector<TpdPsr*>&
PSRRetriever::getPSRData(DataHandle& dataHandle,
                         Indicator application,
                         const CarrierCode& carrier,
                         Indicator area1,
                         Indicator area2,
                         const DateTime& ticketingDT,
                         const DateTime& travelDT) const

{
  return dataHandle.getTpdPsr(application, carrier, area1, area2, ticketingDT);
}
}
