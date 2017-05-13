//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/TravelRouteBuilder.h"

#include "Common/FareMarketUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Fares/FlightTracker.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"

#include <algorithm>
#include <set>
#include <vector>

namespace tse
{

static Logger
logger("atseintl.Routing.TravelRouteBuilder");

TravelRouteBuilder::TravelRouteBuilder() {}

TravelRouteBuilder::~TravelRouteBuilder() {}

bool
TravelRouteBuilder::buildTravelRoute(const PricingTrx& trx,
                                     FareMarket& fareMarket,
                                     TravelRoute& tvlRoute)
{
  return buildTravelRoute(trx, fareMarket, tvlRoute, IGNORE_TKTPTSIND);
}

bool
TravelRouteBuilder::buildTravelRoute(const PricingTrx& trx,
                                     FareMarket& fareMarket,
                                     TravelRoute& tvlRoute,
                                     const Indicator& unticketedPoint)
{
  LOG4CXX_INFO(logger, "Entered TravelRouteBuilder::process()");

  // copy travelSeg

  tvlRoute.mileageTravelRoute().insert(tvlRoute.mileageTravelRoute().end(),
                                       fareMarket.travelSeg().begin(),
                                       fareMarket.travelSeg().end());

  tvlRoute.flightTrackingCxr() = fareMarket.fltTrkIndicator();
  tvlRoute.govCxr() = fareMarket.governingCarrier();
  tvlRoute.globalDir() = fareMarket.getGlobalDirection();
  tvlRoute.travelDate() = trx.adjustedTravelDate(TseUtil::getTravelDate(fareMarket.travelSeg()));
  tvlRoute.totalTPM() = 0;
  tvlRoute.maxPermittedMileage() = 0;
  tvlRoute.geoTravelType() = fareMarket.geoTravelType();
  tvlRoute.unticketedPointInd() = unticketedPoint;
  setCarrierPreferences(fareMarket, tvlRoute);

  bool success = fillCityCarrier(fareMarket.travelSeg(), tvlRoute);
  if(success)
  {
    updateCityCarrier(trx, tvlRoute);
    LOG4CXX_INFO(logger, "TravelRouteBuilder ::process()");
    return true ;
  }

  LOG4CXX_INFO(logger, "Leaving TraveRouteBuilder::process() Failed");
  return false;
}

// Build a temporary travel route with flight stops for restriction validation.
// Called by Aircraft Restriction Validator
bool
TravelRouteBuilder::buildTempTravelRoute(const TravelRoute& oldTvlRoute,
                                         TravelRoute& tvlRoute,
                                         const Indicator& unticketedPoint)
{
  const std::vector<TravelSeg*>& tvlSegs = oldTvlRoute.mileageTravelRoute();
  LOG4CXX_INFO(logger, "TravelRouteBuilder ::Temporary Travel Route process()");

  tvlRoute.govCxr() = oldTvlRoute.govCxr();
  tvlRoute.geoTravelType() = oldTvlRoute.geoTravelType();
  tvlRoute.travelDate() = oldTvlRoute.travelDate();
  tvlRoute.unticketedPointInd() = unticketedPoint;

  bool success = fillCityCarrier(tvlSegs, tvlRoute);
  if (LIKELY(success))
  {
    LOG4CXX_INFO(logger, "TravelRouteBuilder ::Temporary Travel Route process()");
    return true;
  }

  LOG4CXX_INFO(logger, "Leaving TraveRouteBuilder::Temporary Travel Route process() Failed");
  return false;
}

bool
TravelRouteBuilder::fillCityCarrier(const std::vector<TravelSeg*>& tvlSegs, TravelRoute& tvlRoute)
{
  LOG4CXX_DEBUG(logger, " Filling in City Carrier in Travel Route Builder");

  TravelRoute::City boardCity, offCity;
  NationCode boardNation, offNation;
  TravelRoute::CityCarrier cityCarrier;
  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator last = tvlSegs.end();

  if (LIKELY(!tvlSegs.empty()))
  {
    TravelSeg& origin = *tvlSegs.front();
    TravelSeg& dest = *tvlSegs.back();
    tvlRoute.origin() = FareMarketUtil::getMultiCity(
        tvlRoute.govCxr(), origin.origAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
    tvlRoute.originNation() = origin.origin()->nation();
    tvlRoute.destination() = FareMarketUtil::getMultiCity(
        tvlRoute.govCxr(), dest.destAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
    tvlRoute.destinationNation() = dest.destination()->nation();
  }

  for (; itr != last; ++itr)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*itr);
    if (airSeg)
    {
      cityCarrier.carrier() = airSeg->carrier();
    }
    else
    {
      cityCarrier.carrier() = "XX";
    }
    TravelSeg* seg(*itr);
    cityCarrier.stopover() = seg->stopOver();

    bool addHiddenStops = airSeg && !seg->hiddenStops().empty();
    if (tvlRoute.unticketedPointInd() == IGNORE_TKTPTSIND)
    {
      addHiddenStops &= tvlRoute.flightTrackingCxr();
    }
    else
    {
      addHiddenStops &= (tvlRoute.unticketedPointInd() == TKTPTS_ANY);
    }

    if (addHiddenStops)
    {
      LOG4CXX_DEBUG(logger, " TravelRouteBuilder::Processing Hidden City ");
      boardCity.loc() = FareMarketUtil::getMultiCity(
          tvlRoute.govCxr(), seg->origAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
      if (boardCity.loc() != seg->origAirport())
        boardCity.airport() = seg->origAirport();
      else
        boardCity.airport().clear();
      boardNation = seg->origin()->nation();
      boardCity.isHiddenCity() = false;

      std::vector<const Loc*>::iterator it = seg->hiddenStops().begin();
      std::vector<const Loc*>::iterator hidEnd(seg->hiddenStops().end());
      for (; it != hidEnd; ++it)
      {
        const Loc* hid(*it);
        offCity.loc() = FareMarketUtil::getMultiCity(
            tvlRoute.govCxr(), hid->loc(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
        if (offCity.loc() != hid->loc())
          offCity.airport() = hid->loc();
        else
          offCity.airport().clear();
        offNation = hid->nation();
        offCity.isHiddenCity() = true;
        cityCarrier.boardCity() = boardCity;
        cityCarrier.boardNation() = boardNation;
        cityCarrier.offCity() = offCity;
        cityCarrier.offNation() = offNation;

        tvlRoute.travelRoute().push_back(cityCarrier);
        boardCity = offCity;
      }

      offCity.loc() = FareMarketUtil::getMultiCity(
          tvlRoute.govCxr(), seg->destAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
      if (offCity.loc() != seg->destAirport())
        offCity.airport() = seg->destAirport();
      else
        offCity.airport().clear();
      offNation = seg->destination()->nation();
      offCity.isHiddenCity() = false;
      cityCarrier.boardCity() = boardCity;
      cityCarrier.boardNation() = boardNation;
      cityCarrier.offCity() = offCity;
      cityCarrier.offNation() = offNation;

      tvlRoute.travelRoute().push_back(cityCarrier);

    } // end of if
    else
    {
      LOG4CXX_DEBUG(logger, " Travel Route Builder :: processing without Hidden Cities");
      boardCity.loc() = FareMarketUtil::getMultiCity(
          tvlRoute.govCxr(), seg->origAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
      if (boardCity.loc() != seg->origAirport())
        boardCity.airport() = seg->origAirport();
      else
        boardCity.airport().clear();
      boardNation = seg->origin()->nation();
      boardCity.isHiddenCity() = false;
      offCity.loc() = FareMarketUtil::getMultiCity(
          tvlRoute.govCxr(), seg->destAirport(), tvlRoute.geoTravelType(), tvlRoute.travelDate());
      if (offCity.loc() != seg->destAirport())
        offCity.airport() = seg->destAirport();
      else
        offCity.airport().clear();
      offNation = seg->destination()->nation();
      offCity.isHiddenCity() = false;
      if (airSeg || boardCity.loc() != offCity.loc())
      {
        cityCarrier.boardCity() = boardCity;
        cityCarrier.boardNation() = boardNation;
        cityCarrier.offCity() = offCity;
        cityCarrier.offNation() = offNation;

        tvlRoute.travelRoute().push_back(cityCarrier);
      }
      else
      {
        LOG4CXX_DEBUG(logger,
                      " Surface within " << boardCity.loc() << " (" << seg->origAirport() << "-"
                                         << seg->destAirport() << ") skipped");
      }
    }

  } // end of tvlSeg for
  ///@todo Need to add appropriate check for true or false.

  LOG4CXX_DEBUG(logger, " TravelRouteBuilder::fillCityCarrier() complete");
  if (UNLIKELY(tvlRoute.travelRoute().empty()))
    return false;
  else
    return true;
}

void
TravelRouteBuilder::updateCityCarrier(const PricingTrx& trx, TravelRoute& tvlRoute) const
{
  if(trx.getOptions()->isRtw())
  {
    for (TravelRoute::CityCarrier& cityCarrier : tvlRoute.travelRoute())
    {
      const std::vector<AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfo =
          trx.dataHandle().getAirlineAllianceCarrier(cityCarrier.carrier());

      if(!airlineAllianceCarrierInfo.empty())
        cityCarrier.genericAllianceCode() = airlineAllianceCarrierInfo.front()->genericAllianceCode();
    }
  }
}

//------------------------------------------------------------------------
// Build a temporary travel route for Domestic Route Validation and
// for map validation for constructed routings.
//------------------------------------------------------------------------
bool
TravelRouteBuilder::buildTravelRoute(const PricingTrx& trx,
                                     const std::vector<TravelSeg*>& tvlSegs,
                                     TravelRoute& tvlRoute,
                                     const Indicator& unticketedPoint)
{
  LOG4CXX_INFO(logger, " Entering TravelRouteBuilder ::process()");

  // Get Governing Carrier
  std::set<CarrierCode> govCxrSets;
  GoverningCarrier gc(const_cast<PricingTrx*>(&trx));
  gc.getGoverningCarrier(tvlSegs, govCxrSets);
  std::set<CarrierCode>::iterator i = govCxrSets.begin();
  const CarrierCode& govCxr = govCxrSets.empty() ? ANY_CARRIER : *i;
  tvlRoute.govCxr() = govCxr;

  tvlRoute.mileageTravelRoute().insert(
      tvlRoute.mileageTravelRoute().end(), tvlSegs.begin(), tvlSegs.end());

  FlightTracker ft(trx);
  if (LIKELY(!tvlSegs.empty()))
  {
    tvlRoute.flightTrackingCxr() = ft.getFltTrackingInfo(tvlSegs, govCxr);
  }

  tvlRoute.travelDate() = trx.adjustedTravelDate(TseUtil::getTravelDate(tvlSegs));
  tvlRoute.unticketedPointInd() = unticketedPoint;

  bool success = fillCityCarrier(tvlSegs, tvlRoute);
  if(success)
  {
    updateCityCarrier(trx, tvlRoute);
    LOG4CXX_INFO(logger, " Leaving TravelRouteBuilder ::process() Successful");
    return true ;
  }

  LOG4CXX_INFO(logger, "Leaving TraveRouteBuilder::process() Failed");
  return false;
}

// Set carrier's preferences from the Carrier Preference Table
void
TravelRouteBuilder::setCarrierPreferences(FareMarket& fareMarket, TravelRoute& tvlRoute)
{

  tvlRoute.primarySector() = fareMarket.primarySector();

  const CarrierPreference* carrierPref = fareMarket.governingCarrierPref();

  tvlRoute.terminalPoints() = (carrierPref != nullptr) && (carrierPref->applyrtevaltoterminalpt() == YES);
  tvlRoute.doNotApplyDRVExceptUS() =
      (carrierPref != nullptr) && (carrierPref->noApplydrvexceptus() == YES);
}

bool
TravelRouteBuilder::buildTravelRoutes(const PricingTrx& trx,
                                      FareMarket& fareMarket,
                                      TravelRoute& tvlRoute)
{
  if (LIKELY(tvlRoute.travelRoute().empty()))
  {
    if (UNLIKELY(!buildTravelRoute(trx, fareMarket, tvlRoute, TKTPTS_ANY)))
    {
      LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
      return false;
    }

    if (UNLIKELY(!buildTravelRoute(trx, fareMarket, *tvlRoute.travelRouteTktOnly(), TKTPTS_TKTONLY)))
    {
      LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute travelRouteTktOnly - Failed");
      return false;
    }
  }

  if (tvlRoute.travelRouteTktOnly() &&
      tvlRoute.travelRouteTktOnly()->travelRoute().size() == tvlRoute.travelRoute().size())
  {
    tvlRoute.travelRouteTktOnly() = nullptr;
    tvlRoute.unticketedPointInd() = IGNORE_TKTPTSIND;
  }

  return true;
}

}
