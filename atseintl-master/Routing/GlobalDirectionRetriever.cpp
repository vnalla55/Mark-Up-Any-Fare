#include "Routing/GlobalDirectionRetriever.h"

#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
template <typename T>
struct ConstRemover : public std::unary_function<const T, T>
{
  T* operator()(const T* t) { return const_cast<T*>(t); }
}; // lint !e1509

static Logger
logger("atseintl.Routing.GlobalDirectionRetriever");

bool
GlobalDirectionRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                   DataHandle& dataHandle,
                                   Indicator mileageType) const
{
  return retrieve(mileageRouteItem, dataHandle, nullptr, mileageType);
}
//---------------------------------------------------------------------------
// GlobalDirectionRetriever::retrieve() --- Retrieves  TPM GlobalDirection for each segment.
//---------------------------------------------------------------------------
bool
GlobalDirectionRetriever::retrieve(MileageRouteItem& mileageRouteItem,
                                   DataHandle& dataHandle,
                                   PricingTrx* pricingTrx,
                                   Indicator mileageType) const
{
  std::vector<TravelSeg*> asv;
  const Loc* orig = mileageRouteItem.city1();
  const Loc* dest = mileageRouteItem.city2();

  GlobalDirection gd = GlobalDirection::XX;
  AirSeg* as = nullptr;

  // lint --e{413}
  dataHandle.get(as);
  as->origin() = orig;
  as->destination() = dest;
  // TODO TravelSeg has a noon-const vector of Loc* as hiddenStop. But in mileageTrx all locs
  // are const. We may need to cast the const away, in this particular case. Looking for a
  // Better solution.
  //
  if (!mileageRouteItem.hiddenLocs().empty())
  {
    std::transform(mileageRouteItem.hiddenLocs().begin(),
                   mileageRouteItem.hiddenLocs().end(),
                   std::back_inserter(as->hiddenStops()),
                   ConstRemover<Loc>());
  }
  asv.push_back(as);
  getData(mileageRouteItem.travelDate(), asv, gd, pricingTrx);

  if (UNLIKELY(gd == GlobalDirection::XX))
  {
    LOG4CXX_INFO(logger, "GlobalDirectionRetriver::retrieve(TPM-GlobalDirection) Failed");
    return false;
  }
  mileageRouteItem.globalDirection(mileageType) = gd;
  return true;
}

bool
GlobalDirectionRetriever::getData(DateTime& travelDate,
                                  const std::vector<TravelSeg*>& asv,
                                  GlobalDirection& gd,
                                  PricingTrx* pricingTrx) const
{
  return GlobalDirectionFinderV2Adapter::getGlobalDirection(pricingTrx, travelDate, asv, gd);
}

//--------------------------------------------------------------------------
// GlobalDirectionRetriever::retrieve() --- Retrieves  MPM GlobalDirection from first city
// and to all consecutive cities.
//----------------------------------------------------------------------------
bool
GlobalDirectionRetriever::retrieve(MileageRouteItems& mileageRouteItems, DataHandle& dataHandle)
    const
{
  std::vector<TravelSeg*> asv;
  asv.reserve(mileageRouteItems.size());

  MileageRouteItems::iterator itr(mileageRouteItems.begin());
  MileageRouteItems::const_iterator end(mileageRouteItems.end());

  for (; itr != end; ++itr)
  {
    // create the vector of TravelSeg*
    MileageRouteItem& item(*itr);
    AirSeg* as = nullptr;

    // lint --e{413}
    dataHandle.get(as);
    as->origin() = item.city1();
    as->destination() = item.city2();
    if (!item.hiddenLocs().empty())
    {
      std::transform(item.hiddenLocs().begin(),
                     item.hiddenLocs().end(),
                     std::back_inserter(as->hiddenStops()),
                     ConstRemover<Loc>());
    }
    asv.push_back(as);
  }
  GlobalDirection gd = GlobalDirection::XX;
  getData(mileageRouteItems.front().travelDate(), asv, gd, nullptr);

  if (gd == GlobalDirection::XX)
  {
    LOG4CXX_INFO(logger, "GlobalDirectionRetriver::retrieve(MPM-GlobalDirection) Failed");
    return false;
  }
  mileageRouteItems.back().mpmGlobalDirection() = gd;

  return true;
}
}
