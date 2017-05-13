//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "Routing/MileageRouteBuilder.h"

#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiAirportCity.h"
#include "Diagnostic/DiagCollector.h"
#include "Routing/GlobalDirectionRetriever.h"
#include "Routing/MileageRoute.h"
#include "Routing/MultiTransportRetriever.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"

#include <numeric>

namespace tse
{
MileageRouteBuilder::MileageRouteBuilder(bool isWNfromItin) : _isWNfromItin(isWNfromItin) {}
//----------------------------------------------------------------------------
// MileageRouteBuilder::buildMileageRoute(). For WN Entry.
//----------------------------------------------------------------------------

bool
MileageRouteBuilder::buildMileageRoute(MileageTrx& mileageTrx,
                                       MileageRoute& mileageRoute,
                                       DiagCollector* diag) const
{
  const std::vector<MileageTrx::MileageItem*>& mileageItems(mileageTrx.items());
  MileageRouteItems& routeItems(mileageRoute.mileageRouteItems());
  std::vector<std::pair<MileageRouteItem*, const MileageTrx::MileageItem*> > tmprouteItems;

  mileageRoute.ticketingDT() = mileageTrx.inputDT();

  mileageRoute.diagnosticHandle() = diag;
  setCRSMultiHost(mileageRoute, mileageTrx.getRequest());

  const DateTime travelDate = mileageTrx.inputDT();
  mileageRoute.travelDT() = travelDate;

  DataHandle& dataHandle(mileageTrx.dataHandle());
  const MultiTransportRetriever& multiTransport(
      tse::Singleton<MultiTransportRetriever>::instance());
  routeItems.reserve(mileageItems.size());
  std::vector<MileageTrx::MileageItem*>::const_iterator itr(mileageItems.begin() + 1);
  std::vector<MileageTrx::MileageItem*>::const_iterator end(mileageItems.end());
  for (; itr != end; ++itr)
  {
    MileageRouteItem* routeItem = nullptr;

    // lint --e{413}
    const MileageTrx::MileageItem& prev(**(itr - 1));
    mileageTrx.dataHandle().get(routeItem);
    mileageTrx.dataHandle().get(routeItem->city1());
    mileageTrx.dataHandle().get(routeItem->city2());
    if (!prev.cityLoc)
    {
      return false;
    }

    *routeItem->city1() = *prev.cityLoc;
    routeItem->travelDate() = travelDate;
    if ((routeItem->isSurface() = prev.isSurface) || prev.carrierCode.empty())
    {
      routeItem->segmentCarrier() = INDUSTRY_CARRIER;
    }
    else
    {
      routeItem->segmentCarrier() = prev.carrierCode;
    }
    while (itr != end - 1 && (*itr)->isHidden)
    {
      routeItem->hiddenLocs().push_back((*itr)->cityLoc);
      ++itr;
    }
    const MileageTrx::MileageItem& item(**itr);
    *routeItem->city2() = *item.cityLoc;
    routeItem->isStopover() = item.stopType == StopType::StopOver;

    if (_isWNfromItin)
    {
      const GlobalDirectionRetriever& globalDirectionRetriever(
          tse::Singleton<GlobalDirectionRetriever>::instance());
      globalDirectionRetriever.retrieve(*routeItem, dataHandle);
    }
    else
    {
      routeItem->tpmGlobalDirection() = static_cast<GlobalDirection>(GlobalDirection::NO_DIR);
      strToGlobalDirection(routeItem->tpmGlobalDirection(), item.tpmGlobalDirection);
    }
    multiTransport.retrieve(*routeItem, dataHandle);
    routeItems.push_back(*routeItem);
  }

  mileageRoute.dataHandle() = &mileageTrx.dataHandle();
  getGoverningCarrier(mileageRoute);
  return true;
}

//----------------------------------------------------------------------------
// MileageRouteBuilder::buildMileageRoute(). For WPQ Entry.
//----------------------------------------------------------------------------

bool
MileageRouteBuilder::buildMileageRoute(PricingTrx& trx,
                                       const TravelRoute& travelRoute,
                                       MileageRoute& mileageRoute,
                                       DataHandle& dataHandle,
                                       DateTime& ticketingDate) const
{
  setCRSMultiHost(mileageRoute, trx.getRequest());
  const std::vector<TravelSeg*>& mileageItems(travelRoute.mileageTravelRoute());
  MileageRouteItems& routeItems(mileageRoute.mileageRouteItems());

  mileageRoute.ticketingDT() = ticketingDate;

  const DateTime travelDate = travelRoute.travelDate();
  mileageRoute.travelDT() = travelDate;

  bool isDirectFromRouteBegin = true;

  const GlobalDirection& globalDirection(travelRoute.globalDir());
  GlobalDirectionRetriever& globalDirectionRetriever(
      tse::Singleton<GlobalDirectionRetriever>::instance());
  routeItems.reserve(mileageItems.size());
  std::vector<TravelSeg*>::const_iterator itr(mileageItems.begin());
  std::vector<TravelSeg*>::const_iterator end(mileageItems.end());
  for (; itr != end; ++itr)
  {
    MileageRouteItem* routeItem = nullptr;

    // lint --e{413}
    const TravelSeg* seg(*itr);
    dataHandle.get(routeItem);
    dataHandle.get(routeItem->city1());
    dataHandle.get(routeItem->city2());
    *routeItem->city1() = *seg->origin();
    if (const Loc* loc1 =
            dataHandle.getLoc(FareMarketUtil::getMultiCity(travelRoute.govCxr(),
                                                           seg->origin()->loc(),
                                                           travelRoute.geoTravelType(),
                                                           travelDate),
                              travelDate))
    {
      dataHandle.get(routeItem->multiTransportOrigin());
      *routeItem->multiTransportOrigin() = *loc1;
    }
    *routeItem->city2() = *seg->destination();
    if (const Loc* loc2 =
            dataHandle.getLoc(FareMarketUtil::getMultiCity(travelRoute.govCxr(),
                                                           seg->destination()->loc(),
                                                           travelRoute.geoTravelType(),
                                                           travelDate),
                              travelDate))
    {
      dataHandle.get(routeItem->multiTransportDestination());
      *routeItem->multiTransportDestination() = *loc2;
    }
    routeItem->travelDate() = travelDate;
    routeItem->isSurface() = (dynamic_cast<const AirSeg*>(seg) == nullptr);
    routeItem->mpmGlobalDirection() = globalDirection;
    if (!seg->hiddenStops().empty())
    {
      copy(seg->hiddenStops().begin(),
           seg->hiddenStops().end(),
           back_inserter(routeItem->hiddenLocs()));
    }
    routeItem->segmentCarrier() =
        routeItem->isSurface() ? INDUSTRY_CARRIER : dynamic_cast<const AirSeg*>(seg)->carrier();
    // retrieve TPM Global Direction
    globalDirectionRetriever.retrieve(*routeItem, dataHandle, &trx);
    // Determin stopover
    if ((itr + 1) != end)
    {
      const TravelSeg* nextSeg(*(itr + 1));
      routeItem->isStopover() = nextSeg->isStopOver(seg, GeoTravelType::International);
    }
    else
      routeItem->isStopover() = seg->stopOver();

    routeItem->forcedConx() = seg->forcedConx();
    routeItem->forcedStopOver() = seg->forcedStopOver();
    routeItem->pnrSegment() = seg->pnrSegment();

    setDirectService(routeItem, isDirectFromRouteBegin);
    isDirectFromRouteBegin = false; // for further items
    routeItem->isDirectToRouteEnd() = ((end - itr) == 2);
    routeItems.push_back(*routeItem);
  }

  setOccurrences(mileageRoute);

  mileageRoute.dataHandle() = &dataHandle;
  mileageRoute.globalDirection() = globalDirection;
  mileageRoute.governingCarrier() = travelRoute.govCxr();

  return true;
}

bool
MileageRouteBuilder::getGoverningCarrier(MileageRoute& mileageRoute) const
{
  const MileageRouteItems& mileageRouteItems(mileageRoute.mileageRouteItems());
  DataHandle& dataHandle(*mileageRoute.dataHandle());
  std::vector<TravelSeg*> asv;
  asv.reserve(mileageRouteItems.size());
  MileageRouteItems::const_iterator itr(mileageRouteItems.begin());
  MileageRouteItems::const_iterator end(mileageRouteItems.end());
  for (; itr != end; ++itr)
  {
    const MileageRouteItem& item(*itr);
    AirSeg* as = nullptr;

    // lint --e{413}
    dataHandle.get(as);
    as->origin() = item.city1();
    as->destination() = item.city2();
    as->carrier() = item.segmentCarrier();
    as->departureDT() = mileageRoute.ticketingDT();
    asv.push_back(as);
  }
  std::set<CarrierCode> governingCarriers;
  GoverningCarrier governingCarrier;
  if (!governingCarrier.getGoverningCarrier(asv, governingCarriers))
  {
    return false;
  }
  mileageRoute.governingCarrier() = *governingCarriers.begin();
  return true;
}

void
MileageRouteBuilder::setDirectService(MileageRouteItem* routeItem, bool isDirectFromRouteBegin)
    const
{
  if (isDirectFromRouteBegin)
    routeItem->isDirectFromRouteBegin() = true;

  else
    routeItem->isDirectFromRouteBegin() = false;
}

void
MileageRouteBuilder::setOccurrences(MileageRoute& mileageRoute) const
{
  MileageRouteItems& routeItems(mileageRoute.mileageRouteItems());

  if (routeItems.size() > 3)
  {
    MileageRouteItems::reverse_iterator itr(routeItems.rbegin() + 1);
    MileageRouteItems::reverse_iterator itrLast(routeItems.rend());

    for (; itr != itrLast; ++itr)
    {
      MileageRouteItems::reverse_iterator itr2 = itr + 1;
      for (; itr2 != itrLast; ++itr2)
      {
        if ((*itr).city2()->loc() == (*itr2).city2()->loc())
          (*itr2).isLastOccurrenceToRouteEnd() = false;
      }
    }
  }
  if (routeItems.size() > 3)
  {
    MileageRouteItems::iterator itr(routeItems.begin());
    MileageRouteItems::iterator itrLast(routeItems.end());

    for (; itr != itrLast; ++itr)
    {
      MileageRouteItems::iterator itr2 = itr + 1;

      for (; itr2 != itrLast; ++itr2)
      {
        if ((*itr).city2()->loc() == (*itr2).city2()->loc())
          (*itr2).isFirstOccurrenceFromRouteBegin() = false;
      }
    }
  }
}
void
MileageRouteBuilder::buildWNMileageRoute(MileageRoute& mileageRoute) const
{
  bool isDirectFromRouteBegin = true;

  MileageRouteItems& routeItems(mileageRoute.mileageRouteItems());

  MileageRouteItems::iterator itr(routeItems.begin());
  MileageRouteItems::iterator itrLast(routeItems.end());
  for (; itr != itrLast; ++itr)
  {
    MileageRouteItem& item(*itr);
    setDirectService(&item, isDirectFromRouteBegin);
    isDirectFromRouteBegin = false; // for further items
    item.isDirectToRouteEnd() = itr == (itrLast - 2);
  }
  setOccurrences(mileageRoute);
}
void
MileageRouteBuilder::setCRSMultiHost(MileageRoute& mRoute, PricingRequest* request) const
{
  std::string crs = EMPTY_STRING();
  if (LIKELY(request != nullptr && request->ticketingAgent() != nullptr))
  {
    if (!(request->ticketingAgent()->vendorCrsCode().empty()))
      crs = request->ticketingAgent()->vendorCrsCode();
    else if (LIKELY(request->ticketingAgent()->tvlAgencyPCC().size() == 4))
      crs = request->ticketingAgent()->cxrCode();
    else
      mRoute.multiHost() = request->ticketingAgent()->hostCarrier();

    // Determine user application
    if (LIKELY(!crs.empty()))
    {
      if (UNLIKELY(crs == AXESS_MULTIHOST_ID))
        mRoute.crs() = AXESS_USER;
      else if (crs == ABACUS_MULTIHOST_ID)
        mRoute.crs() = ABACUS_USER;
      else if (crs == INFINI_MULTIHOST_ID)
        mRoute.crs() = INFINI_USER;
      else if (LIKELY(crs == SABRE_MULTIHOST_ID))
        mRoute.crs() = SABRE_USER;
    }
  }
}
}
