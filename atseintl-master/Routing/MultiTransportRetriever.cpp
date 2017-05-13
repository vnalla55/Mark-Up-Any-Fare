#include "Routing/MultiTransportRetriever.h"

#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
//---------------------------------------------------------------------------
// MultiTransportRetriever::retrieve() --- Retrieves multiTransport City code.
//---------------------------------------------------------------------------

bool
MultiTransportRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                  DataHandle& dataHandle,
                                  Indicator mileageType) const
{
  GeoTravelType geoTvlType = GeoTravelType::International;
  CarrierCode cxr;

  const LocCode* city1 = getData(
      dataHandle, mileageRouteItem.city1()->loc(), cxr, geoTvlType, mileageRouteItem.travelDate());
  const LocCode* city2 = getData(
      dataHandle, mileageRouteItem.city2()->loc(), cxr, geoTvlType, mileageRouteItem.travelDate());

  if (city1 != nullptr)
  {
    if (const Loc* loc1 = getLoc(dataHandle, *city1, mileageRouteItem.travelDate()))
    {
      dataHandle.get(mileageRouteItem.multiTransportOrigin());
      *mileageRouteItem.multiTransportOrigin() = *loc1;
    }
  }
  if (city2 != nullptr)
  {
    if (const Loc* loc2 = getLoc(dataHandle, *city2, mileageRouteItem.travelDate()))
    {
      dataHandle.get(mileageRouteItem.multiTransportDestination());
      *mileageRouteItem.multiTransportDestination() = *loc2;
    }
  }
  return (city1 != nullptr || city2 != nullptr);
}
LocCode
MultiTransportRetriever::retrieve(DataHandle& dh, const LocCode& loc, const DateTime& tDate) const
{
  LocCode ret = loc;
  CarrierCode cxr;
  const LocCode* l = getData(dh, loc, cxr, GeoTravelType::International, tDate);
  if (l)
    ret = *l;
  return ret;
}

class NotEqualsLoc
{
public:
  NotEqualsLoc(const LocCode& loc) : _loc(loc) {}
  bool operator()(const MultiTransport* mt) const { return mt->multitranscity() != _loc; }

private:
  const LocCode& _loc;
};

const LocCode*
MultiTransportRetriever::getData(DataHandle& dataHandle,
                                 const LocCode& loc,
                                 const CarrierCode& carrier,
                                 GeoTravelType geoTvlType,
                                 const DateTime& tvlDate) const
{
  const std::vector<MultiTransport*>& multiTransports =
      dataHandle.getMultiTransportCity(loc, carrier, geoTvlType, tvlDate);
  std::vector<MultiTransport*>::const_iterator city =
      std::find_if(multiTransports.begin(), multiTransports.end(), NotEqualsLoc(loc));
  if (city != multiTransports.end())
  {
    return &(*city)->multitranscity();
  }
  return nullptr;
}

const Loc*
MultiTransportRetriever::getLoc(DataHandle& dataHandle, const LocCode& loc, const DateTime& date)
    const
{
  return dataHandle.getLoc(loc, date);
}
}
