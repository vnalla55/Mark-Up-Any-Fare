//----------------------------------------------------------------------------
//  Copyright Sabre 2003
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Fares/DRVController.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"
#include "Fares/CarrierFareController.h"
#include "Fares/FlightTracker.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/SpecifiedRoutingValidator.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"

#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.DRVController");

//---------------------------------------------------------------------------
// DRVController::process()
//
// This method is invoked when a city was not found on the international
// Specified Route Map.  The DRV process will attempt to validate the
// missing city on the local carrier's route map, then continue validating
// the cities on the international route map.
//
// It's possible to perform DRV in the origin and destination country of the
// fare market.
//
// --------------------------------------------------------------------------
bool
DRVController::process(PricingTrx& trx,
                       PaxTypeFare& paxTypeFare,
                       const Routing* routing,
                       const Routing* origAddOnRouting,
                       const Routing* destAddOnRouting,
                       const TravelRoute& tvlRoute,
                       MapInfo* mapInfo)
{

  LOG4CXX_DEBUG(logger, "Entered DRVController::process()");

  bool routingValid = false;

  DRVInfos* drvInfos;
  trx.dataHandle().get(drvInfos); // lint !e530

  DRVInfo* drvInfo;
  trx.dataHandle().get(drvInfo); // lint !e530

  //------------------------------------------------------------------
  //  Perform Domestic Routing Validation to validate missing city
  //------------------------------------------------------------------
  routingValid = performDRV(trx, paxTypeFare, routing, tvlRoute, mapInfo, drvInfo);
  drvInfo->drvInfoStatus() = routingValid;
  drvInfo->routingStatus() = routingValid;
  drvInfos->push_back(drvInfo);

  if (routingValid)
  {
    //-----------------------------------------------------------
    // DRV Passed - Continue validating the International Routing
    //-----------------------------------------------------------
    TravelRoute intlTvlRoute;
    bool rc = buildIntlTravelRoute(trx, tvlRoute, intlTvlRoute, mapInfo->missingCityIndex());
    if (rc)
    {
      updateCityCarrierVec(intlTvlRoute, *drvInfo, drvInfo->intlCityCarrier());
      routingValid = validateIntlRouting(trx,
                                         intlTvlRoute,
                                         routing,
                                         origAddOnRouting,
                                         destAddOnRouting,
                                         mapInfo,
                                         paxTypeFare.fareMarket()->travelDate());

      if (!routingValid)
      {
        //------------------------------------------------------------------
        // DRV passed but the international routing is still not valid.
        // Check whether DRV can be performed for another missing city.
        //------------------------------------------------------------------
        bool alreadyChecked = checkMissingCity(trx, mapInfo->missingCityIndex(), tvlRoute);
        if (!alreadyChecked)
        {
          DRVInfo* drvInfo;
          trx.dataHandle().get(drvInfo); // lint !e530
          if (drvInfo == nullptr)
          {
            return false;
          }
          routingValid = performDRV(trx, paxTypeFare, routing, intlTvlRoute, mapInfo, drvInfo);
          drvInfo->drvInfoStatus() = routingValid;
          drvInfo->routingStatus() = routingValid;
          drvInfos->push_back(drvInfo);
        }
      }
    }
  } // DRV Failed, fail routing

  mapInfo->drvStatus() = routingValid;
  mapInfo->drvInfos() = *drvInfos;

  LOG4CXX_DEBUG(logger, "Leaving DRVController::process()");
  return routingValid;
}

//--------------------------------------------------------------------------
// Perform a series of steps to validate a local routing when a city is
// missing from the international route map.
//
// Step 1:  Get the missingCity and Nation Code from the Travel Route
// Step 2:  Check to see whether this Carrier Allows DRV
// Step 3:  Build the Local Travel Route
// Step 4:  Find a matching Fare Market
// Step 5:  Select the Highest Oneway Fare in the FareMarket vector of
//          paxTypeFares and get its Routing
// Step 6:  Retrieve the Routing from the Database
// Step 7:  Validate the Routing Map or Restrictions
//
//--------------------------------------------------------------------------
bool
DRVController::performDRV(PricingTrx& trx,
                          PaxTypeFare& paxTypeFare,
                          const Routing* routing,
                          const TravelRoute& tvlRoute,
                          MapInfo* mapInfo,
                          DRVInfo* drvInfo)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::performDRV()");

  bool localRoutingValid = false;
  int16_t tvlRouteSize = tvlRoute.travelRoute().size();
  if (LIKELY(mapInfo->missingCityIndex() >= 0 && mapInfo->missingCityIndex() < tvlRouteSize))
  {
    if (carrierAllowsDRV(trx, mapInfo->missingCityIndex(), tvlRoute))
    {
      TravelRoute localTvlRoute;
      if (buildLocalTravelRoute(
              trx, tvlRoute, mapInfo->missingCityIndex(), localTvlRoute, *drvInfo))
      {
        updateCityCarrierVec(localTvlRoute, *drvInfo, drvInfo->localCityCarrier());
        drvInfo->localGovCxr() = localTvlRoute.govCxr();

        //-------------------------------------------------------------------------------
        // DomesticRoutingValidation Indicator = 2, no need to actually validate routing
        //-------------------------------------------------------------------------------
        if (routing->domRtgvalInd() == PASS_ANY_ONLINE_POINT)
        {
          localRoutingValid = true;
        }

        else
        {
          const FareMarket* fareMarket =
              findMatchingFareMarket(trx, localTvlRoute, paxTypeFare.retrievalDate());

          if ((fareMarket == nullptr) && (trx.getTrxType() == PricingTrx::IS_TRX))
          {
            localRoutingValid = true;
          }

          if (fareMarket != nullptr)
          {
            RtgKey rtgKey;

            MoneyAmount highestAmount(0);
            int fareCount = 0;
            PaxTypeFare* selectedPaxTypeFare =
                selectFare(*fareMarket, *drvInfo, highestAmount, fareCount);

            while (selectedPaxTypeFare)
            {
              const Routing* localRtg = RoutingUtil::getRoutingData(
                  trx, *selectedPaxTypeFare, selectedPaxTypeFare->routingNumber());
              if (!localRtg)
              {
                drvInfo->getRoutingFailed() = true;
                localRoutingValid = false;
              }
              else
              {
                setRoutingTariff(localRtg, drvInfo);
                localRoutingValid =
                    validateLocalRouting(trx, paxTypeFare, localTvlRoute, localRtg, drvInfo);
                if (localRoutingValid)
                {
                  LOG4CXX_INFO(logger, "local routing valid");
                  break;
                }
              }
              selectedPaxTypeFare = selectFare(*fareMarket, *drvInfo, highestAmount, fareCount);
            }
          } // Unable to find matching Fare Market
          else
          {
            drvInfo->noFareMarket() = true;
          }
        }
      } // Could not Build Local TravelRoute
    } // Carrier does not want DRV
    else
    {
      drvInfo->notSameCountry() = true;
    }
  } // No cityIndex provided
  else
  {
    drvInfo->missingCityIndexInvalid() = true;
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::performDRV()");
  return localRoutingValid;
}

//---------------------------------------------------------------------------------
// Determine whether the carrier wants to validate the local routing.
//
//     Always validate the local routing when the missing city is within
//     US, CA, PR or the US Virgin Islands.
//
//     If the failed city is not within the US or Canada, check the DRV indicator
//     in the travelRoute that was set based on the Carrier Preference Table.
//
//---------------------------------------------------------------------------------
bool
DRVController::carrierAllowsDRV(PricingTrx& trx,
                                int16_t missingCityIndex,
                                const TravelRoute& tvlRoute)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::carrierAllowsDRV()");

  int16_t tvlRouteSize = tvlRoute.travelRoute().size();

  if (UNLIKELY(tvlRoute.mileageTravelRoute().empty() || missingCityIndex > tvlRouteSize))
  {
    return false;
  }

  const Loc* orig = tvlRoute.mileageTravelRoute().front()->origin();
  const Loc* dest = tvlRoute.mileageTravelRoute().back()->destination();

  const Loc* city = trx.dataHandle().getLoc(
      tvlRoute.travelRoute()[missingCityIndex].offCity().loc(), tvlRoute.travelDate());

  if (UNLIKELY(city == nullptr))
  {
    return false;
  }

  // DRV always applies within US/CA/PR/USVI
  if (LocUtil::isDomesticUSCA(*city))
  {
    if (LocUtil::isDomesticUSCA(*orig) || LocUtil::isDomesticUSCA(*dest))
    {
      return true;
    }
  }

  else
  {
    if ((RoutingUtil::locMatchesOrigin(city, tvlRoute) ||
         RoutingUtil::locMatchesDestination(city, tvlRoute)) &&
        tvlRoute.doNotApplyDRVExceptUS() != true) // lint !e650
    {
      return true;
    }
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::carrierAllowsDRV()");
  return false;
}

void
DRVController::addHiddenStopLocation(TravelRoute& tvlRoute,
                                     bool& flightStopMarket,
                                     std::vector<TravelSeg*>& localTvlSegs,
                                     PricingTrx& trx,
                                     const CarrierCode& carrier,
                                     const TravelSeg* tvlSeg,
                                     bool isFirstOrig,
                                     const Loc& missingCityLoc,
                                     const DateTime& travelDate) const
{
  tvlRoute.flightTrackingCxr() = FlightTracker(trx).getFltTrackingInfo(localTvlSegs, carrier);
  bool addHiddenStops = tvlRoute.flightTrackingCxr();

  if (TrxUtil::isFullMapRoutingActivated(trx) && tvlRoute.unticketedPointInd() != IGNORE_TKTPTSIND)
  {
    addHiddenStops = (tvlRoute.unticketedPointInd() == TKTPTS_ANY);
  }

  if (addHiddenStops && !tvlSeg->hiddenStops().empty())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);
    if (carrier == airSeg->carrier())
    {
      std::vector<const Loc*>::const_iterator it = tvlSeg->hiddenStops().begin();
      std::vector<const Loc*>::const_iterator itEnd = tvlSeg->hiddenStops().end();

      LocCode boardCity = isFirstOrig ? tvlSeg->origAirport() : tvlSeg->destAirport();
      for (; it != itEnd; it++)
      {
        if (checkHiddenStopLocation((*(*it)), missingCityLoc, boardCity))
        {
          createNewTvlSeg(trx, localTvlSegs, boardCity, (*it)->loc(), carrier, travelDate);
          flightStopMarket = true;
          boardCity = (*it)->loc();
        }
      }
    }
  }
}

bool
DRVController::buildLocalTravelRoute(PricingTrx& trx,
                                     const TravelRoute& tvlRoute,
                                     int16_t cityIndex,
                                     TravelRoute& localTvlRoute,
                                     DRVInfo& drvInfo)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::buildLocalTravelRoute()");

  std::vector<TravelSeg*> localTvlSegs;
  CarrierCode carrier;
  LocCode missingCity;

  int16_t tvlRouteSize = tvlRoute.travelRoute().size();
  if (UNLIKELY(cityIndex > tvlRouteSize))
  {
    drvInfo.missingCityIndexInvalid() = true;
    return false;
  }

  missingCity = tvlRoute.travelRoute()[cityIndex].offCity().loc();
  const Loc* missingCityLoc = trx.dataHandle().getLoc(missingCity, tvlRoute.travelDate());

  if (UNLIKELY(tvlRoute.travelRoute()[cityIndex].carrier().empty()))
  {
    drvInfo.notQualifiedForDRV() = true;
    return false;
  }
  else
  {
    carrier = tvlRoute.travelRoute()[cityIndex].carrier();
  }

  //----------------------------------------------------------------------------
  // If Surface travel into the missing City, then get the carrier that follows.
  //----------------------------------------------------------------------------
  if (carrier == SURFACE_CARRIER)
  {
    if (!tvlRoute.travelRoute().empty() && (cityIndex + 1) < tvlRouteSize)
    {
      if (tvlRoute.travelRoute()[cityIndex + 1].carrier().empty())
      {
        drvInfo.notQualifiedForDRV() = true;
        return false;
      }
      else
      {
        carrier = tvlRoute.travelRoute()[cityIndex + 1].carrier();
      }
    }
    else
    {
      drvInfo.notQualifiedForDRV() = true;
      return false;
    }
  }

  std::vector<TravelSeg*>::const_iterator tvlSegIt = tvlRoute.mileageTravelRoute().begin();
  std::vector<TravelRoute::CityCarrier>::const_iterator tvlRouteIt = tvlRoute.travelRoute().begin();

  for (; (tvlRouteIt != (tvlRoute.travelRoute().begin() + cityIndex)) &&
             tvlSegIt != tvlRoute.mileageTravelRoute().end();
       ++tvlRouteIt)
  {
    if (!tvlRouteIt->offCity().isHiddenCity())
      ++tvlSegIt;
  }

  std::vector<TravelSeg*>::const_reverse_iterator rvsTvlSegIt(
      tvlSegIt == tvlRoute.mileageTravelRoute().end() ? tvlSegIt : tvlSegIt + 1);

  int cnt = 2;
  while (cnt > 0)
  {
    if (tvlSegIt == tvlRoute.mileageTravelRoute().end() ||
        !RoutingUtil::locMatchesTvlSeg(missingCityLoc, **tvlSegIt))
    {
      drvInfo.notSameCountry() = true;
      return false;
    }
    else if ((*tvlSegIt)->isAir() && carrier != static_cast<AirSeg*>(*tvlSegIt)->carrier())
    {
      drvInfo.notSameCarrier() = true;
      return false;
    }
    else if (tvlRoute.primarySector() == *tvlSegIt)
    {
      drvInfo.missingCityOrigDest() = true;
      return false;
    }
    localTvlSegs.push_back(*tvlSegIt);
    ++tvlSegIt;
    --cnt;
  }

  for (; tvlSegIt != tvlRoute.mileageTravelRoute().end(); ++tvlSegIt)
  {
    if (tvlRoute.primarySector() == *tvlSegIt)
    {
      addHiddenStopLocation(localTvlRoute,
                            drvInfo.flightStopMarket(),
                            localTvlSegs,
                            trx,
                            carrier,
                            *tvlSegIt,
                            true,
                            *missingCityLoc,
                            tvlRoute.travelDate());
      break;
    }
    else if (RoutingUtil::locMatchesTvlSeg(missingCityLoc, **tvlSegIt))
    {
      if ((*tvlSegIt)->isAir() && carrier != static_cast<AirSeg*>(*tvlSegIt)->carrier())
      {
        drvInfo.notSameCarrier() = true;
        return false;
      }
    }
    else
      break;
    localTvlSegs.push_back(*tvlSegIt);
  }

  ++rvsTvlSegIt;
  for (; rvsTvlSegIt != tvlRoute.mileageTravelRoute().rend(); ++rvsTvlSegIt)
  {
    if (tvlRoute.primarySector() == *rvsTvlSegIt)
    {
      addHiddenStopLocation(localTvlRoute,
                            drvInfo.flightStopMarket(),
                            localTvlSegs,
                            trx,
                            carrier,
                            *rvsTvlSegIt,
                            false,
                            *missingCityLoc,
                            tvlRoute.travelDate());
      break;
    }
    else if (RoutingUtil::locMatchesTvlSeg(missingCityLoc, **rvsTvlSegIt))
    {
      if ((*rvsTvlSegIt)->isAir() && carrier != static_cast<AirSeg*>(*rvsTvlSegIt)->carrier())
      {
        drvInfo.notSameCarrier() = true;
        return false;
      }
    }
    else
      break;
    localTvlSegs.insert(localTvlSegs.begin(), *rvsTvlSegIt);
  }

  //--------------------------------------------------------------------------------
  // All TravelSegs have been accumulated.
  //
  // If flight stops exist on the primarySector and the carrier is a flight
  // tracking carrier, the travel segment for the primarySector must be added to
  // the beginning or the end of the std::vector<TravelSeg*>.
  //--------------------------------------------------------------------------------
  if (localTvlSegs.empty())
  {
    drvInfo.notQualifiedForDRV() = true;
    return false;
  }

  else
  {
    //--------------------------------------------------------------------------------
    // Build the Local TravelRoute from the vector of TravelSegs accumulated
    //--------------------------------------------------------------------------------
    TravelRouteBuilder trb;
    bool rc = trb.buildTravelRoute(trx, localTvlSegs, localTvlRoute, tvlRoute.unticketedPointInd());
    localTvlRoute.travelDate() = tvlRoute.travelDate();

    if (!rc)
    {
      return false;
    }
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::buildLocalTravelRoute() SUCCESS");
  return true;
}

//-----------------------------------------------------------------------------------
//  Build a new travel route to call Specified Routing Validation after domestic
//  routing validation has been performed.  Eliminate local points from the travel
//  route and ensure all other points are located on the international map.
//-----------------------------------------------------------------------------------
bool
DRVController::buildIntlTravelRoute(PricingTrx& trx,
                                    const TravelRoute& tvlRoute,
                                    TravelRoute& intlTvlRoute,
                                    int16_t cityIndex)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::buildIntlTravelRoute()");

  std::vector<TravelSeg*> intlTvlSegs;
  int16_t tvlRouteSize = tvlRoute.travelRoute().size();

  if (cityIndex > tvlRouteSize)
  {
    return false;
  }

  LocCode missingCity = tvlRoute.travelRoute()[cityIndex].offCity().loc();
  const Loc* missingCityLoc = trx.dataHandle().getLoc(missingCity, tvlRoute.travelDate());
  bool localBoardCityFound = false;

  //----------------------------------------------------------------------------
  // Iterate through the vector of TravelSegs in the International Travel Route
  // Copy qualifying travelSegs to a temporaryVector of travelSegs
  //----------------------------------------------------------------------------
  const std::vector<TravelSeg*>& tvlSegs = tvlRoute.mileageTravelRoute();

  std::vector<TravelSeg*>::const_iterator start = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator end = tvlSegs.end();

  for (; start != end; ++start)
  {
    if (!RoutingUtil::locMatchesTvlSeg(missingCityLoc, **start))
    {
      intlTvlSegs.push_back((*start)); // Push back poiner to tvlSeg
      localBoardCityFound = false;
    }

    else if (tvlRoute.primarySector() == *start)
    {
      localBoardCityFound = false;
      intlTvlSegs.push_back((*start)); // Push back pointer to tvlSeg
      intlTvlRoute.primarySector() = tvlRoute.primarySector();
    }

    else
    {
      if (!localBoardCityFound)
      {
        if ((*start)->segmentType() == Arunk)
        {
          ArunkSeg* newTvlSeg;
          trx.dataHandle().get(newTvlSeg); // lint !e530
          createNewTvlSeg(trx, *start, newTvlSeg);
          intlTvlSegs.push_back(newTvlSeg);
        }
        else
        {
          AirSeg* newTvlSeg;
          trx.dataHandle().get(newTvlSeg); // lint !e530
          createNewTvlSeg(trx, *start, newTvlSeg);
          intlTvlSegs.push_back(newTvlSeg);
        }

        localBoardCityFound = true;
      }
      else
      {
        intlTvlSegs.back()->destAirport() = (*start)->destAirport();
      }
    }
  }

  //--------------------------------------------------------------------------------
  // Build the Local TravelRoute from the vector of TravelSegs accumulated
  //--------------------------------------------------------------------------------
  if (intlTvlSegs.empty())
  {
    return false;
  }

  else
  {
    intlTvlRoute.flightTrackingCxr() = true;
    TravelRouteBuilder().buildTravelRoute(
        trx, intlTvlSegs, intlTvlRoute, tvlRoute.unticketedPointInd());
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::buildIntoTravelRoute() SUCCESS");
  return true;
}

//---------------------------------------------------------------------------------
// Find a FareMarket that matches the TravelRoute
//
// Return a matching FareMarket
//---------------------------------------------------------------------------------
const FareMarket*
DRVController::findMatchingFareMarket(PricingTrx& trx,
                                      TravelRoute& localTvlRoute,
                                      const DateTime& date)
{

  LOG4CXX_DEBUG(logger, " Entered DRVController::findMatchingFareMarket()");

  const FareMarket* fareMarket = nullptr;
  fareMarket = TrxUtil::getFareMarket(
      trx, localTvlRoute.govCxr(), localTvlRoute.mileageTravelRoute(), date, nullptr);

  if (((fareMarket == nullptr) && (trx.getTrxType() != PricingTrx::IS_TRX)) ||
       (LIKELY(_trx.isFootNotePrevalidationAllowed())
           && fareMarket != nullptr && trx.getTrxType() != PricingTrx::IS_TRX
           && !fareMarket->footNoteFailedFares().empty()))
  {
    fareMarket = buildFareMarket(trx, localTvlRoute);
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::findMatchingFareMarket()");
  return fareMarket;
}

PaxTypeFare*
DRVController::selectFare(const FareMarket& fareMarket,
                          DRVInfo& drvInfo,
                          MoneyAmount& highestAmount,
                          int& fareCount)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::selectFare()");

  PaxTypeFare* selectedPaxTypeFare = nullptr;
  if (fareMarket.allPaxTypeFare().empty())
  {
    drvInfo.noPaxTypeFares() = true;
    return selectedPaxTypeFare;
  }
  else
  {
    std::vector<PaxTypeFare*>::const_reverse_iterator paxItr = fareMarket.allPaxTypeFare().rbegin();
    std::vector<PaxTypeFare*>::const_reverse_iterator end = fareMarket.allPaxTypeFare().rend();
    int i = 0; // get the n-th fare
    for (; paxItr != end; ++paxItr)
    {
      if (((*paxItr)->isPublished()) &&
          (((*paxItr)->tcrTariffCat() != RuleConst::PRIVATE_TARIFF) &&
           (!((*paxItr)->isNegotiated())) && (!((*paxItr)->isFareByRule()))))
      {
        if (i < fareCount)
        {
          i++;
          continue;
        }
        if ((*paxItr)->nucFareAmount() < (highestAmount - EPSILON))
          return selectedPaxTypeFare; // returns 0

        drvInfo = DRVInfo();

        drvInfo.fareClass() = (*paxItr)->fareClass();
        drvInfo.fareAmount() = (*paxItr)->fareAmount();
        drvInfo.currency() = (*paxItr)->currency();
        drvInfo.vendor() = (*paxItr)->vendor();
        drvInfo.global() = (*paxItr)->globalDirection();
        drvInfo.routingNumber() = (*paxItr)->routingNumber();
        drvInfo.routingTariff1() = (*paxItr)->tcrRoutingTariff1();
        drvInfo.tariffCode1() = (*paxItr)->tcrRoutingTariff1Code();
        drvInfo.routingTariff2() = (*paxItr)->tcrRoutingTariff2();
        drvInfo.tariffCode2() = (*paxItr)->tcrRoutingTariff2Code();

        selectedPaxTypeFare = *paxItr;
        highestAmount = (*paxItr)->nucFareAmount();
        fareCount = (i + 1);
        return selectedPaxTypeFare;
      }
    }
  }

  return selectedPaxTypeFare;
}

//-----------------------------------------------------------------------------------
// Validate the Map Routing or Restrictions for the local routing
//-----------------------------------------------------------------------------------
bool
DRVController::validateLocalRouting(PricingTrx& trx,
                                    PaxTypeFare& paxTypeFare,
                                    TravelRoute& tvlRoute,
                                    const Routing* routing,
                                    DRVInfo* drvInfo)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::validateLocalRouting()");

  bool localRoutingValid = false;

  if (!(routing->rmaps().empty()))
  {
    MapInfo* drvMapInfo;
    trx.dataHandle().get(drvMapInfo); // lint !e530
    SpecifiedRoutingValidator specifiedValidator;
    localRoutingValid = specifiedValidator.validate(
        trx, tvlRoute, routing, drvMapInfo, paxTypeFare.fareMarket()->travelDate());
    drvMapInfo->processed() = true;
    drvMapInfo->valid() = localRoutingValid;
    drvMapInfo->drvStatus() = localRoutingValid;
    drvInfo->mapInfo() = drvMapInfo;
  }
  else
  {
    RestrictionInfos* restrictionInfos;
    trx.dataHandle().get(restrictionInfos); // lint !e530
    drvInfo->restrictions() = restrictionInfos;
    localRoutingValid =
        processRestrictions(paxTypeFare, trx, tvlRoute, drvInfo, routing, routing->rests());
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::validateLocalRouting()");
  return localRoutingValid;
}

//-----------------------------------------------------------------------------------
// Validate the Map Routing or Restrictions for the international routing
//-----------------------------------------------------------------------------------
bool
DRVController::validateIntlRouting(PricingTrx& trx,
                                   TravelRoute& tvlRoute,
                                   const Routing* routing,
                                   const Routing* origAddOnRouting,
                                   const Routing* destAddOnRouting,
                                   MapInfo* mapInfo,
                                   const DateTime& travelDate)
{
  LOG4CXX_DEBUG(logger, " Entered DRVController::validateIntlRouting()");

  bool routingValid = false;

  if (routing != nullptr && !(routing->rmaps().empty()))
  {
    SpecifiedRoutingValidator specifiedValidator;
    int16_t preDRVmissingCityIndex = mapInfo->missingCityIndex();

    routingValid = specifiedValidator.validate(
        trx, tvlRoute, routing, mapInfo, travelDate, origAddOnRouting, destAddOnRouting);

    mapInfo->valid() = routingValid;
    mapInfo->postDRVmissingCityIndex() = mapInfo->missingCityIndex();
    mapInfo->missingCityIndex() = preDRVmissingCityIndex;
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::validateIntlRouting()");
  return routingValid;
}

//---------------------------------------------------------------------------------
// Get a new Fare Market
//---------------------------------------------------------------------------------
const FareMarket*
DRVController::buildFareMarket(PricingTrx& trx, TravelRoute& localTvlRoute)
{

  LOG4CXX_DEBUG(logger, " Entered DRVController::buildFareMarket()");

  FareMarket* fareMarket = nullptr;
  const CarrierCode* cxr = &(localTvlRoute.govCxr());
  const GlobalDirection* gd = &(localTvlRoute.globalDir());

  RepricingTrx* repricingTrx = TrxUtil::reprice(
      trx, localTvlRoute.mileageTravelRoute(), FMDirection::UNKNOWN, false, cxr, gd);

  if (repricingTrx != nullptr)
  {
    fareMarket = repricingTrx->fareMarket().front();
  }

  LOG4CXX_DEBUG(logger, " Leaving DRVController::buildFareMarket()");
  return fareMarket;
}

//----------------------------------------------------
// Create an AirSeg
//----------------------------------------------------
void
DRVController::createNewTvlSeg(PricingTrx& trx, TravelSeg* tvlSeg, TravelSeg* newTvlSeg)
{

  newTvlSeg->origin() = tvlSeg->origin();
  newTvlSeg->destination() = tvlSeg->destination();
  newTvlSeg->origAirport() = tvlSeg->origAirport();
  newTvlSeg->destAirport() = tvlSeg->destAirport();

  if (tvlSeg->segmentType() != Arunk)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
    AirSeg* newAirSeg = dynamic_cast<AirSeg*>(newTvlSeg);

    if (!airSeg->carrier().empty())
    {
      newAirSeg->carrier() = airSeg->carrier();
    }
  }
}

//----------------------------------------------------
// Create a new TravelSeg for hidden points
//----------------------------------------------------
bool
DRVController::createNewTvlSeg(PricingTrx& trx,
                               std::vector<TravelSeg*>& localTvlSegs,
                               const LocCode& boardCity,
                               const LocCode& offCity,
                               const CarrierCode& carrier,
                               const DateTime& travelDate) const
{
  AirSeg* newTvlSeg;
  trx.dataHandle().get(newTvlSeg); // lint !e530

  newTvlSeg->origAirport() = boardCity;
  newTvlSeg->destAirport() = offCity;
  newTvlSeg->carrier() = carrier;
  newTvlSeg->departureDT() = travelDate;

  const Loc* board = trx.dataHandle().getLoc(boardCity, travelDate);
  const Loc* off = trx.dataHandle().getLoc(offCity, travelDate);

  if (board != nullptr && off != nullptr)
  {
    newTvlSeg->origin() = board;
    newTvlSeg->destination() = off;
    localTvlSegs.push_back(newTvlSeg);
    return true;
  }

  return false;
}

//-------------------------------------------------------------
// Compare location of hidden flight stop to missing City
//-------------------------------------------------------------
bool
DRVController::checkHiddenStopLocation(const Loc& hiddenStop,
                                       const Loc& missingCity,
                                       LocCode& boardCity) const
{

  if (hiddenStop.nation() == missingCity.nation() && hiddenStop.loc() != boardCity)
  {
    return true;
  }

  else if (LocUtil::isDomesticUSCA(hiddenStop) && LocUtil::isDomesticUSCA(missingCity))
  {
    if (hiddenStop.loc() != boardCity)
    {
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------
// Check to see DRV was already performed for this missing City
// The assumption is that if the missing city is in the origin country,
// it has already been checked.
//---------------------------------------------------------------------
bool
DRVController::checkMissingCity(PricingTrx& trx,
                                int16_t missingCityIndex,
                                const TravelRoute& tvlRoute)
{

  int16_t tvlRouteSize = tvlRoute.travelRoute().size();

  if (missingCityIndex > tvlRouteSize)
  {
    return false;
  }

  LocCode missingCity = tvlRoute.travelRoute()[missingCityIndex].offCity().loc();
  const Loc* missingCityLoc = trx.dataHandle().getLoc(missingCity, tvlRoute.travelDate());

  if (missingCityLoc != nullptr && RoutingUtil::locMatchesOrigin(missingCityLoc, tvlRoute))
  {
    return true;
  }

  return false;
}

//---------------------------------------------------------------------
// Check to see DRV was already performed for this missing City
// The assumption is that if the missing city is in the origin country,
// it has already been checked.
//---------------------------------------------------------------------
void
DRVController::setRoutingTariff(const Routing* localRtg, DRVInfo* drvInfo)
{

  if (localRtg->routingTariff() == drvInfo->routingTariff1())
  {
    return;
  }
  else if (localRtg->routingTariff() == drvInfo->routingTariff2())
  {
    drvInfo->routingTariff1() = drvInfo->routingTariff2();
    drvInfo->tariffCode1() = drvInfo->tariffCode2();
  }
}
}
