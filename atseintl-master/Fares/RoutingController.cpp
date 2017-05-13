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

#include "Fares/RoutingController.h"

#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RoutingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/MileageTypeData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag450Collector.h"
#include "Diagnostic/Diag452Collector.h"
#include "Diagnostic/Diag455Collector.h"
#include "Diagnostic/Diag460Collector.h"
#include "Fares/DRVController.h"
#include "Fares/SpecialRouting.h"
#include "Routing/MileageSurchargeException.h"
#include "Routing/MileageValidator.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/RestrictionValidatorFactory.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RtgKey.h"
#include "Routing/SpecifiedRoutingValidator.h"
#include "Routing/StopTypeRestrictionValidator.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackMileageSurchargeExceptionValidation);

static Logger
logger("atseintl.Fares.RoutingController");

bool
RoutingController::RestrictionKey::
operator<(const RestrictionKey& key) const
{
  if (this->_restriction < key._restriction)
    return true;
  else if (key._restriction < this->_restriction)
    return false;
  else if (UNLIKELY(_checkFlip && this->_market1 == key._market2 && this->_market2 == key._market1))
    return false;
  else if (this->_market1 < key._market1)
    return true;
  else if (key._market1 < this->_market1)
    return false;
  else if (this->_market2 < key._market2)
    return true;
  return false;
}

RoutingController::RoutingController(PricingTrx& trx)
  : _trx(trx),
    _tvlRouteMap(nullptr),
    _tvlRouteMapMutex(nullptr),
    _routingKeyMap(nullptr),
    _routingKeyMapMutex(nullptr),
    _fareMarketMap(nullptr),
    _fareMarketMapMutex(nullptr)
{
  _trx.dataHandle().get(_tvlRouteMap);
  _trx.dataHandle().get(_tvlRouteMapMutex);
  _trx.dataHandle().get(_routingKeyMap);
  _trx.dataHandle().get(_routingKeyMapMutex);
  _trx.dataHandle().get(_fareMarketMap);
  _trx.dataHandle().get(_fareMarketMapMutex);
}

bool
RoutingController::buildTvlRouteMapAndProcess(FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::buildTvlRouteMapAndProcess()");

  TravelRouteBuilder builder;
  TravelRoute* travelRoute;
  _trx.dataHandle().get(travelRoute); // lint !e530

  if (!builder.buildTravelRoute(_trx, fareMarket, *travelRoute))
  {
    LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
    return false;
  }

  // build hash key for travel route

  std::string temp;
  for (auto& elem : travelRoute->travelRoute())
  {
    temp = elem.stopover();
    temp = temp + elem.boardCity().loc() + elem.offCity().loc() + elem.carrier();
  }

  std::ostringstream ossHash;
  ossHash << travelRoute->origin() << "." << travelRoute->destination() << "."
          << travelRoute->travelRoute().size() << temp;

  std::string tvlRouteHash = ossHash.str();

  TvlRouteMap::iterator tvlRouteMapI;

  {
    boost::lock_guard<boost::mutex> g(*_tvlRouteMapMutex);
    tvlRouteMapI = _tvlRouteMap->find(tvlRouteHash);
  }

  if (tvlRouteMapI == _tvlRouteMap->end())
  {
    {
      boost::lock_guard<boost::mutex> g(*_tvlRouteMapMutex);
      _tvlRouteMap->insert(TvlRouteMap::value_type(tvlRouteHash, _routingKeyMap));
    }
    RoutingInfos* routingInfos;
    _trx.dataHandle().get(routingInfos); // lint !e530

    process(fareMarket, *travelRoute, *routingInfos);
    // inside above process() update map for rtgKey
    processRoutingDiagnostic(*travelRoute, *routingInfos, fareMarket);
  }
  else
  {
    LOG4CXX_INFO(logger,
                 "RoutingController::buildTvlRouteMapAndProcess - FareMarket Skip Routing Process");

    // This fare market map will be used as a list of fare markets which
    // need to have paxtypefare updated later
    {
      boost::lock_guard<boost::mutex> g(*_fareMarketMapMutex);
      _fareMarketMap->insert(FareMarketMap::value_type(&fareMarket, travelRoute));
    }
  }

  LOG4CXX_INFO(logger, " Leaving RoutingController::buildTvlRouteMapAndProcess()");

  return true;
}

bool
RoutingController::validatePaxTypeFare(FareMarket& fareMarket)
{
  FareMarketMap::iterator fareMarketMapI;

  {
    boost::lock_guard<boost::mutex> g(*_fareMarketMapMutex);
    fareMarketMapI = _fareMarketMap->find(&fareMarket);
  }

  if (fareMarketMapI == _fareMarketMap->end())
    return true;

  LOG4CXX_INFO(logger,
               "RoutingController::validatePaxTypeFare - FareMarket Use Routing Previous Result");

  TravelRoute& travelRoute = (*(*fareMarketMapI).second);

  // build hash key for travel route
  std::string temp;
  for (auto& elem : travelRoute.travelRoute())
  {
    temp = elem.stopover();
    temp = temp + elem.boardCity().loc() + elem.offCity().loc() + elem.carrier();
  }

  std::ostringstream ossHash;
  ossHash << travelRoute.origin() << "." << travelRoute.destination() << "."
          << travelRoute.travelRoute().size() << temp;

  std::string tvlRouteHash = ossHash.str();

  TvlRouteMap::iterator tvlRouteMapI;

  {
    boost::lock_guard<boost::mutex> g(*_tvlRouteMapMutex);
    tvlRouteMapI = _tvlRouteMap->find(tvlRouteHash);
  }

  bool specialRoutingFound;
  specialRoutingFound = fareMarket.specialRtgFound();

  if (tvlRouteMapI != _tvlRouteMap->end())
  {
    RoutingInfos& routingInfos = *(*tvlRouteMapI).second;
    // start loop to update each paxTypeFare
    std::vector<PaxTypeFare*>::iterator paxTypeFareI = fareMarket.allPaxTypeFare().begin();
    fbrPaxTypeFareVector fbrFares;

    for (; paxTypeFareI != fareMarket.allPaxTypeFare().end(); paxTypeFareI++)
    {
      PaxTypeFare& paxTypeFare = **paxTypeFareI;

      LOG4CXX_INFO(logger,
                   "RoutingController::validatePaxTypeFare for "
                       << paxTypeFare.fareMarket()->boardMultiCity() << "-"
                       << paxTypeFare.fareMarket()->offMultiCity() << " DIR "
                       << paxTypeFare.directionality() << " RoutingProcessed "
                       << (paxTypeFare.isRoutingProcessed() ? "T" : "F"));

      if (isSpecialRouting(paxTypeFare))
      {
        if (needSpecialRouting(paxTypeFare))
          fbrFares.push_back(*paxTypeFareI);

        continue;
      }

      if ((!paxTypeFare.isRoutingProcessed()) &&
          (paxTypeFare.isValidForDiscount() || paxTypeFare.cat25BasePaxFare() ||
           (paxTypeFare.retrievalInfo() != nullptr && paxTypeFare.retrievalInfo()->keep()) ||
           (specialRoutingFound && paxTypeFare.isValidForRouting())))
      {
        const Routing* routing;
        const Routing* origAddOnRouting = nullptr;
        const Routing* destAddOnRouting = nullptr;

        getRoutings(_trx, paxTypeFare, routing, origAddOnRouting, destAddOnRouting);

        RtgKey rKey;
        buildRtgKey(paxTypeFare, fareMarket, routing, origAddOnRouting, destAddOnRouting, rKey);

        RoutingKeyMap::iterator pos = routingInfos.find(rKey);

        if (pos != routingInfos.end())
        {
          const RoutingInfo& rtgInfo = *(*pos).second;
          // use routing validation result from processed fare market to update paxTypeFare
          if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->psrHipExempt())
            paxTypeFare.setPsrHipExempt(true);

          paxTypeFare.setRoutingValid(rtgInfo.routingStatus());
          updatePaxTypeFare(rKey, paxTypeFare, routingInfos);
          paxTypeFare.setRoutingProcessed(true);
        }
      } // end if isRoutingProcessed()

    } // end for loop each paxTypeFare

    // Apply SpecialRouting
    processSpecialRouting(fbrFares, routingInfos, travelRoute);

    // Apply Mileage SurchargeException
    processSurchargeException(_trx, fareMarket.allPaxTypeFare(), routingInfos, travelRoute);

  } // end if found travelroute
  else
  {
    // send error msg.
    LOG4CXX_INFO(logger, " Fare Market did not get validated");
    return false;
  }

  return true;
}

bool
RoutingController::process(FareMarket& fareMarket,
                           TravelRoute& tvlRoute,
                           RoutingInfos& routingInfos)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::process()");

  RtgKeyMap rtMap;

  if (UNLIKELY(!TrxUtil::isFullMapRoutingActivated(_trx)))
  {
    if (tvlRoute.travelRoute().empty() &&
        !TravelRouteBuilder().buildTravelRoute(_trx, fareMarket, tvlRoute))
    {
      LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
      return false;
    }
  }
  else
  {
    if (UNLIKELY(!TravelRouteBuilder().buildTravelRoutes(_trx, fareMarket, tvlRoute)))
    {
      return false;
    }
  }

  fbrPaxTypeFareVector fbrFares;

  if(_trx.getOptions()->isRtw())
  {
    bool isGenericRouting = true;
    processFares(fareMarket, tvlRoute, routingInfos, fbrFares, rtMap, isGenericRouting);
  }

  processFares(fareMarket, tvlRoute, routingInfos, fbrFares, rtMap);

  // Apply SpecialRouting
  processSpecialRouting(fbrFares, routingInfos, tvlRoute);

  // Apply Mileage SurchargeException
  processSurchargeException(_trx, fareMarket.allPaxTypeFare(), routingInfos, tvlRoute);

  LOG4CXX_INFO(logger, " Leaving RoutingController::process()");

  return true;
}

bool
RoutingController::collectSpecialRouting(FareMarket& fareMarket, PaxTypeFare& paxTypeFare,
                                         fbrPaxTypeFareVector& fbrFares)
{
  if (UNLIKELY(paxTypeFare.isDummyFare() && _trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    dummyFareMileage(_trx, paxTypeFare, fareMarket);
    return false;
  }

  if (isSpecialRouting(paxTypeFare))
  {
    if (needSpecialRouting(paxTypeFare))
      fbrFares.push_back(&paxTypeFare);

    return false;
  }
  return true;
}


void
RoutingController::processFares(FareMarket& fareMarket, TravelRoute& tvlRoute,
                                RoutingInfos& routingInfos, fbrPaxTypeFareVector& fbrFares,
                                RtgKeyMap& rtMap, bool genericRouting)
{
  bool specialRoutingFound = fareMarket.specialRtgFound();

  for (PaxTypeFare* ptf : fareMarket.allPaxTypeFare())
  {
    PaxTypeFare& paxTypeFare = *ptf;

    LOG4CXX_INFO(logger,
                 "RoutingController::process() for "
                     << paxTypeFare.fareMarket()->boardMultiCity() << "-"
                     << paxTypeFare.fareMarket()->offMultiCity() << " DIR "
                     << paxTypeFare.directionality() << " RoutingProcessed "
                     << (paxTypeFare.isRoutingProcessed() ? "T" : "F"));

    if(!genericRouting && !collectSpecialRouting(fareMarket, paxTypeFare, fbrFares))
      continue;

    if(!(!paxTypeFare.isRoutingProcessed()
        && (paxTypeFare.isValidForDiscount() || paxTypeFare.cat25BasePaxFare()
            || (paxTypeFare.retrievalInfo() != nullptr && paxTypeFare.retrievalInfo()->keep())
            || (specialRoutingFound && paxTypeFare.isValidForRouting()))))
      continue;

    processFare(paxTypeFare, fareMarket, tvlRoute, routingInfos, rtMap, genericRouting);

    if(_trx.getOptions()->isRtw() && genericRouting)
      prepareFareForSpecRtgValid(paxTypeFare);
  } // end for

  if (LIKELY(_trx.isFootNotePrevalidationAllowed()))
  {
    for (auto fare : fareMarket.footNoteFailedFares())
    {
      PaxTypeFare paxTypeFare;
      paxTypeFare.initialize(fare.first, nullptr, &fareMarket, _trx);

      if (!(!paxTypeFare.isRoutingProcessed() && (paxTypeFare.isValidForDiscount()
          || (specialRoutingFound && paxTypeFare.isValidForRouting()))))
        continue;

      processFare(paxTypeFare, fareMarket, tvlRoute, routingInfos, rtMap, genericRouting);

      if (_trx.getOptions()->isRtw() && genericRouting)
        prepareFareForSpecRtgValid(paxTypeFare);
    }
  }
}

bool
RoutingController::process(PaxTypeFare& paxTypeFare,
                           TravelRoute& tvlRoute,
                           RoutingInfos& routingInfos)
{
  if (paxTypeFare.isRoutingProcessed())
    return true;

  LOG4CXX_INFO(logger, " Entered RoutingController::process(paxTypeFare)");

  // Assumption: The paxTypeFare is not Cat25 or Cat35 fare.
  //             No Special routing process is needed.
  FareMarket& fareMarket = *(paxTypeFare.fareMarket());

  if (!TrxUtil::isFullMapRoutingActivated(_trx) && tvlRoute.travelRoute().empty())
  {
    if (!TravelRouteBuilder().buildTravelRoute(_trx, fareMarket, tvlRoute))
    {
      LOG4CXX_INFO(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
      return false;
    }
  }
  else
  {
    if (!TravelRouteBuilder().buildTravelRoutes(_trx, fareMarket, tvlRoute))
    {
      return false;
    }
  }

  LOG4CXX_INFO(logger,
               "RoutingController::process(paxTypeFare) for "
                   << fareMarket.boardMultiCity() << "-" << fareMarket.offMultiCity() << " DIR "
                   << paxTypeFare.directionality() << " RoutingProcessed "
                   << (paxTypeFare.isRoutingProcessed() ? "T" : "F"));

  RtgKeyMap rtMap;

  if(_trx.getOptions()->isRtw())
  {
    bool isGenericRouting = true;
    processFare(paxTypeFare, fareMarket, tvlRoute, routingInfos, rtMap, isGenericRouting);

    prepareFareForSpecRtgValid(paxTypeFare);
  }

  processFare(paxTypeFare, fareMarket, tvlRoute, routingInfos, rtMap);

  // Apply Mileage SurchargeException
  std::vector<PaxTypeFare*> tmpVec;
  tmpVec.push_back(&paxTypeFare);
  processSurchargeException(_trx, tmpVec, routingInfos, tvlRoute);

  LOG4CXX_INFO(logger, " Leaving RoutingController::process(paxTypeFare)");

  return true;
}

inline void
RoutingController::processFare(PaxTypeFare& paxTypeFare, FareMarket& fareMarket,
                               TravelRoute& tvlRoute, RoutingInfos& routingInfos,
                               RtgKeyMap& rtMap, bool genericRouting)
{
  const Routing* routing;
  const Routing* origAddOnRouting = nullptr;
  const Routing* destAddOnRouting = nullptr;

  if(_trx.getOptions()->isRtw())
  {
    if(genericRouting)
    {
      routing = RoutingUtil::getRoutingData(_trx, paxTypeFare, GENERIC_ROUTING);
      if(!routing)
        getRoutings(_trx, paxTypeFare, routing, origAddOnRouting, destAddOnRouting);
    } else
      getRoutings(_trx, paxTypeFare, routing, origAddOnRouting, destAddOnRouting);
  } else
    getRoutings(_trx, paxTypeFare, routing, origAddOnRouting, destAddOnRouting);

  if (LIKELY(TrxUtil::isFullMapRoutingActivated(_trx)))
  {
    processPaxTypeFare(paxTypeFare,
                       tvlRoute,
                       fareMarket,
                       routingInfos,
                       rtMap,
                       routing,
                       origAddOnRouting,
                       destAddOnRouting);
  }
  else
  {
    RtgKey rKey;
    buildRtgKey(paxTypeFare, fareMarket, routing, origAddOnRouting, destAddOnRouting, rKey);

    std::map<RtgKey, bool>::iterator pos = rtMap.find(rKey);
    if (pos == rtMap.end())
    {
      bool status = processRoutingValidation(rKey,
                                             _trx,
                                             tvlRoute,
                                             &paxTypeFare,
                                             routing,
                                             origAddOnRouting,
                                             destAddOnRouting,
                                             routingInfos,
                                             fareMarket.travelDate());
      paxTypeFare.setRoutingValid(status);

      std::map<const RtgKey, RoutingInfo*>::iterator rInfosItr = routingInfos.find(rKey);
      if (rInfosItr != routingInfos.end())
      {
        const RoutingInfo& rtgInfo = *(*rInfosItr).second;
        if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->psrHipExempt())
        {
          paxTypeFare.setPsrHipExempt(true);
        }
      }
      rtMap.insert(std::map<RtgKey, bool>::value_type(rKey, status));
    }
    else
    {
      paxTypeFare.setRoutingValid((pos)->second);
      updatePaxTypeFare(rKey, paxTypeFare, routingInfos);
    }

    paxTypeFare.setRoutingProcessed(true);
  }
}

void
RoutingController::prepareFareForSpecRtgValid(PaxTypeFare& ptf)
{
  if(ptf.isRoutingProcessed() && ptf.isRoutingValid())
  {
    const Routing* routing;
    const Routing* origAddOnRouting = nullptr;
    const Routing* destAddOnRouting = nullptr;

    getRoutings(_trx, ptf, routing, origAddOnRouting, destAddOnRouting);

    if(routing)
    {
      ptf.setRoutingProcessed(false);
      ptf.setRoutingValid(false);
    }
  }
}

void
RoutingController::updatePaxTypeFare(RtgKey& rKey, PaxTypeFare& fare, RoutingInfos& infos)
{
  std::map<const RtgKey, RoutingInfo*>::iterator i = infos.find(rKey);

  if (LIKELY(i != infos.end()))
  {
    const RoutingInfo& rInfo = *((*i).second);
    bool isRouting = rInfo.mileageInfo() == nullptr;
    fare.setIsRouting(isRouting);
    fare.setRoutingMapValid(rInfo.routingMapStatus());
    if (rInfo.mileageInfo() != nullptr)
    {
      updateFareSurcharge(fare, *(rInfo.mileageInfo()));
      if (rInfo.mileageInfo() != nullptr && rInfo.mileageInfo()->psrHipExempt())
      {
        fare.setPsrHipExempt(true);
      }
    }
  }
}

bool
RoutingController::processShoppingRouting(FareMarket& fareMarket,
                                          const uint32_t& bitIndex,
                                          PaxTypeFare& curFare,
                                          ShoppingRtgMap& rtMap)
{
  bool requiredConditions = (PricingTrx::ESV_TRX == _trx.getTrxType()) ||
                            curFare.isKeepForRoutingValidation() ||
                            (!curFare.isFlightBitmapInvalid() && curFare.isFlightValid(bitIndex));

  if (UNLIKELY(!requiredConditions))
    return true;

  const Routing* routing = nullptr;
  const Routing* origAddOnRouting = nullptr;
  const Routing* destAddOnRouting = nullptr;
  TravelRoute tvlRoute;
  RtgKey rKey;

  if (LIKELY(TrxUtil::isFullMapRoutingActivated(_trx)))
  {
    if (UNLIKELY(!collectRoutingData(
            fareMarket, curFare, routing, origAddOnRouting, destAddOnRouting, tvlRoute, rKey)))
      return false;
  }
  else
  {
    if (!collectRoutingDataNotFullMapRouting(
            fareMarket, curFare, routing, origAddOnRouting, destAddOnRouting, tvlRoute, rKey))
      return false;
  }


  if (isSpecialRouting(curFare))
  {
    processShoppingSpecialRouting(curFare, bitIndex, tvlRoute, rtMap);
    return true;
  }

  ShoppingRtgMap::iterator pos = rtMap.find(rKey);
  if (pos == rtMap.end())
  {
    RoutingValue rtgValue;
    processShoppingNonSpecialRouting(fareMarket,
                                     curFare,
                                     tvlRoute,
                                     routing,
                                     origAddOnRouting,
                                     destAddOnRouting,
                                     rKey,
                                     rtMap,
                                     rtgValue);
    rtMap.insert(ShoppingRtgMap::value_type(rKey, rtgValue));
    updateFlightBitStatus(curFare, bitIndex, rtgValue);
  }
  else
  {
    updateFlightBitStatus(curFare, bitIndex, pos->second);
  }

  curFare.setRoutingProcessed(true);
  // set it to valid in case one of the itins is valid
  curFare.setRoutingValid(true);
  return true;
}

bool
RoutingController::process(ShoppingTrx& trx,
                           FareMarket& fareMarket,
                           const uint32_t& bitIndex,
                           PaxTypeFare& curFare,
                           ShoppingRtgMap& rtMap)
{
  LOG4CXX_DEBUG(logger, "Entered ShoppingRoutingController::process()");

  return processShoppingRouting(fareMarket, bitIndex, curFare, rtMap);
}

void
RoutingController::updateFlightBitStatus(PaxTypeFare& curFare,
                                         const uint32_t& bitIndex,
                                         const RoutingValue& rtgValue)
{
  if (UNLIKELY(PricingTrx::ESV_TRX == _trx.getTrxType()))
  {
    if (!(rtgValue.status))
      curFare.setFlightInvalidESV(bitIndex, RuleConst::ROUTING_FAIL);
    return;
  }

  if (!(rtgValue.status))
  {
    curFare.setFlightInvalid(bitIndex, RuleConst::ROUTING_FAIL);
  }

  if (!curFare.isRouting())
  {
    curFare.setFlightMPMPercentage(bitIndex, rtgValue.mpmPctg);
    curFare.mileageSurchargePctg() = 0;
    curFare.mileageSurchargeAmt() = 0.0;
  }
}

bool
RoutingController::collectRoutingData(FareMarket& fareMarket,
                                      PaxTypeFare& curFare,
                                      const Routing*& routing,
                                      const Routing*& origAddOnRouting,
                                      const Routing*& destAddOnRouting,
                                      TravelRoute& tvlRoute,
                                      RtgKey& rtKey)
{
  getRoutings(_trx, curFare, routing, origAddOnRouting, destAddOnRouting);

  const Indicator unticketedPointInd = (routing ? routing->unticketedPointInd() : IGNORE_TKTPTSIND);
  if (UNLIKELY(!TravelRouteBuilder().buildTravelRoute(_trx, fareMarket, tvlRoute, unticketedPointInd)))
  {
    return false;
  }

  buildRtgKey(curFare, fareMarket, routing, origAddOnRouting, destAddOnRouting, rtKey);
  return true;
}

void
RoutingController::processShoppingSpecialRouting(PaxTypeFare& curFare,
                                                 const uint32_t& bitIndex,
                                                 TravelRoute& tvlRoute,
                                                 ShoppingRtgMap& rtMap)
{
  RoutingInfos routingInfos;

  ShoppingRtgMap::iterator it = rtMap.begin();
  ShoppingRtgMap::iterator posE = rtMap.end();
  for (; it != posE; ++it)
  {
    RoutingInfos rtgInfos = it->second.rtginfos;
    if (UNLIKELY(rtgInfos.empty()))
      continue;

    RoutingInfo* rtgInfo = rtgInfos.begin()->second;
    if (UNLIKELY(rtgInfo == nullptr))
      continue;

    RtgKey key = it->first;
    routingInfos.insert(RoutingInfos::value_type(key, rtgInfo));
  }

  fbrPaxTypeFareVector fbrFares;

  curFare.setRoutingProcessed(false);
  fbrFares.push_back(&curFare);

  processSpecialRouting(fbrFares, routingInfos, tvlRoute);

  if (!(curFare.isRoutingValid()))
  {
    if (PricingTrx::ESV_TRX == _trx.getTrxType())
    {
      curFare.setFlightInvalidESV(bitIndex, RuleConst::ROUTING_FAIL);
    }
    else
    {
      curFare.setFlightInvalid(bitIndex, RuleConst::ROUTING_FAIL);
    }
  }

  curFare.setRoutingValid(true);
}

void
RoutingController::processShoppingNonSpecialRouting(FareMarket& fareMarket,
                                                    PaxTypeFare& curFare,
                                                    TravelRoute& tvlRoute,
                                                    const Routing* routing,
                                                    const Routing* origAddOnRouting,
                                                    const Routing* destAddOnRouting,
                                                    RtgKey& rKey,
                                                    ShoppingRtgMap& rtMap,
                                                    RoutingValue& rtgValue)
{
  const DateTime& travelDate = _trx.adjustedTravelDate(fareMarket.travelDate());

  RoutingInfos routingInfos;
  bool status = processRoutingValidation(rKey,
                                         _trx,
                                         tvlRoute,
                                         &curFare,
                                         routing,
                                         origAddOnRouting,
                                         destAddOnRouting,
                                         routingInfos,
                                         travelDate);

  processRoutingDiagnostic(tvlRoute, routingInfos, fareMarket);

  std::map<const RtgKey, RoutingInfo*>::iterator rInfosItr = routingInfos.find(rKey);
  if (LIKELY(rInfosItr != routingInfos.end()))
  {
    const RoutingInfo& rtgInfo = *(*rInfosItr).second;
    if (rtgInfo.mileageInfo() != nullptr)
    {
      if (rtgInfo.mileageInfo()->psrHipExempt())
      {
        curFare.setPsrHipExempt(true);
      }
      if (rtgInfo.mileageInfo()->valid())
      {
        status = true;
      }
      else
      {
        status = validateMileageSurchargeException(_trx, curFare, tvlRoute, rtgInfo);
      }
    }
  }

  rtgValue.mpmPctg = curFare.mileageSurchargePctg();
  rtgValue.status = status;
  rtgValue.rtginfos = routingInfos;
}

bool
RoutingController::processShoppingNonSpecialRouting(FareMarket& fareMarket,
                                                    PaxTypeFare& curFare,
                                                    ShoppingRtgMap& rtMap)
{
  LOG4CXX_DEBUG(logger, "Entered ShoppingRoutingController::processShoppingNonSpecialRouting()");

  const Routing* routing = nullptr;
  const Routing* origAddOnRouting = nullptr;
  const Routing* destAddOnRouting = nullptr;
  TravelRoute tvlRoute;
  RtgKey rKey;

  if (LIKELY(TrxUtil::isFullMapRoutingActivated(_trx)))
  {
    if (UNLIKELY(!collectRoutingData(
            fareMarket, curFare, routing, origAddOnRouting, destAddOnRouting, tvlRoute, rKey)))
      return false;
  }
  else
  {
    if (!collectRoutingDataNotFullMapRouting(
            fareMarket, curFare, routing, origAddOnRouting, destAddOnRouting, tvlRoute, rKey))
      return false;
  }


  ShoppingRtgMap::iterator pos = rtMap.find(rKey);

  if (pos == rtMap.end())
  {
    RoutingValue rtgValue;
    processShoppingNonSpecialRouting(fareMarket,
                                     curFare,
                                     tvlRoute,
                                     routing,
                                     origAddOnRouting,
                                     destAddOnRouting,
                                     rKey,
                                     rtMap,
                                     rtgValue);
    rtMap.insert(ShoppingRtgMap::value_type(rKey, rtgValue));
  }

  curFare.setRoutingValid(true);
  return true;
}

bool
RoutingController::validateMileageSurchargeException(PricingTrx& trx,
                                                     PaxTypeFare& curFare,
                                                     TravelRoute& tvlRoute,
                                                     const RoutingInfo& rtgInfo)
{
  if (fallback::fixed::fallbackMileageSurchargeExceptionValidation())
    return false;

  MileageSurchargeException mse;
  const MileageInfo* milageInfo = rtgInfo.mileageInfo();

  return mse.validateSingleFare(tvlRoute, curFare, trx, *milageInfo);
}

bool
RoutingController::collectRoutingDataNotFullMapRouting(FareMarket& fareMarket,
                                                       PaxTypeFare& curFare,
                                                       const Routing*& routing,
                                                       const Routing*& origAddOnRouting,
                                                       const Routing*& destAddOnRouting,
                                                       TravelRoute& tvlRoute,
                                                       RtgKey& rtKey)
{
  TravelRouteBuilder builder;

  if (!builder.buildTravelRoute(_trx, fareMarket, tvlRoute))
  {
    LOG4CXX_ERROR(logger, "TravelRouteBuilder::buildTravelRoute - Failed");
    return false;
  }

  getRoutings(_trx, curFare, routing, origAddOnRouting, destAddOnRouting);

  buildRtgKey(curFare, fareMarket, routing, origAddOnRouting, destAddOnRouting, rtKey);
  return true;
}

void
RoutingController::processPaxTypeFare(PaxTypeFare& paxTypeFare,
                                      TravelRoute& tvlRoute,
                                      FareMarket& fareMarket,
                                      RoutingInfos& routingInfos,
                                      RtgKeyMap& rtMap,
                                      const Routing* routing,
                                      const Routing* origAddOnRouting,
                                      const Routing* destAddOnRouting)
{
  RtgKey rKey;
  buildRtgKey(paxTypeFare, fareMarket, routing, origAddOnRouting, destAddOnRouting, rKey);

  std::map<RtgKey, bool>::iterator pos = rtMap.find(rKey);
  if (pos == rtMap.end())
  {
    bool tktPointsOnly = false;
    if (tvlRoute.travelRouteTktOnly())
    {
      tktPointsOnly = RoutingUtil::isTicketedPointOnly(routing, fareMarket.fltTrkIndicator());
    }

    TravelRoute& currentTvlRoute = tktPointsOnly ? *tvlRoute.travelRouteTktOnly() : tvlRoute;

    bool status = processRoutingValidation(rKey, _trx, currentTvlRoute, &paxTypeFare, routing,
        origAddOnRouting, destAddOnRouting, routingInfos, fareMarket.travelDate());

    paxTypeFare.setRoutingValid(status);

    std::map<const RtgKey, RoutingInfo*>::iterator rInfosItr = routingInfos.find(rKey);
    if (LIKELY(rInfosItr != routingInfos.end()))
    {
      const RoutingInfo& rtgInfo = *(*rInfosItr).second;
      if (rtgInfo.mileageInfo() != nullptr && rtgInfo.mileageInfo()->psrHipExempt())
      {
        paxTypeFare.setPsrHipExempt(true);
      }
    }

    rtMap.insert(std::map<RtgKey, bool>::value_type(rKey, status));
  }
  else
  {
    paxTypeFare.setRoutingValid((pos)->second);
    updatePaxTypeFare(rKey, paxTypeFare, routingInfos);
  }
  paxTypeFare.setRoutingProcessed(true);
}

class Except3_12and17 : public std::unary_function<const RoutingRestriction*, bool>
{
public:
  Except3_12and17(bool constructed) : _constructed(constructed) {}
  bool operator()(const RoutingRestriction* r) const
  {
    if (_constructed)
      return r->restriction() != ROUTING_RESTRICTION_3
          && r->restriction() != RTW_ROUTING_RESTRICTION_12
          && r->restriction() != ROUTING_RESTRICTION_17;
    else
      return r->restriction() != RTW_ROUTING_RESTRICTION_12
          && r->restriction() != ROUTING_RESTRICTION_17;
  }

private:
  bool _constructed;
};

void
RoutingController::processRoutingDiagnostic(const TravelRoute& tvlRoute,
                                            const RoutingInfos& routingInfos,
                                            FareMarket& fareMarket)

{
  //-----------------------------------------------------------------------------------
  //  Display Diagnostic 450  Routing Validation Results
  //-----------------------------------------------------------------------------------
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic450))
  {
    DCFactory* factory = DCFactory::instance();
    Diag450Collector* diagPtr = dynamic_cast<Diag450Collector*>(factory->create(_trx));
    Diag450Collector& diag = *diagPtr;
    diag.enable(Diagnostic450);

    diag.displayRoutingValidationResults(_trx, tvlRoute, &routingInfos);

    diag.flushMsg();
  }

  //-----------------------------------------------------------------------------------
  //  Display Diagnostic 455  Route Map and Restrictions Display
  //-----------------------------------------------------------------------------------
  else if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic455))
  {
    DCFactory* factory = DCFactory::instance();
    Diag455Collector* diagPtr = dynamic_cast<Diag455Collector*>(factory->create(_trx));
    Diag455Collector& diag = *diagPtr;
    diag.enable(Diagnostic455);

    diag.displayRouteMapAndRestrictions(_trx, tvlRoute, &routingInfos);

    diag.flushMsg();
  }

  //-----------------------------------------------------------------------------------
  //  Display Diagnostic 460 paxTypeFareVector with results of Routing Validation
  //-----------------------------------------------------------------------------------
  else if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic460))
  {
    DCFactory* factory = DCFactory::instance();
    Diag460Collector* diagPtr = dynamic_cast<Diag460Collector*>(factory->create(_trx));
    Diag460Collector& diag = *diagPtr;
    diag.enable(Diagnostic460);

    diag.displayPaxTypeFares(_trx, fareMarket, tvlRoute);

    diag.flushMsg();
  }
  else
  {
    LOG4CXX_INFO(logger, " Unknown Diagnostic Type in RoutingController::process()");
  }

  // TODO : add fail  code  */
}

bool
RoutingController::processRoutingCheckMissingData(RtgKey& rKey,
                                                  RoutingInfo* routingInfo,
                                                  const Routing* routing,
                                                  const Routing* origAddOnRouting,
                                                  const Routing* destAddOnRouting)
{
  // missing data - don't process
  if (UNLIKELY((!routing && rKey.routingNumber() != MILEAGE_ROUTING) ||
               (!origAddOnRouting && !rKey.addOnRouting1().empty() &&
                rKey.addOnRouting1() != MILEAGE_ROUTING) ||
               (!destAddOnRouting && !rKey.addOnRouting2().empty() &&
                rKey.addOnRouting2() != MILEAGE_ROUTING)))
  {
    LOG4CXX_WARN(logger,
                 "Missing routing data: vendor=" << rKey.vendor() << " carrier=" << rKey.carrier()
                                                 << " routing tariff=" << rKey.routingTariff()
                                                 << " routing number=" << rKey.routingNumber()
                                                 << " add on routing 1=" << rKey.addOnRouting1()
                                                 << " add on routing 2=" << rKey.addOnRouting2());
    routingInfo->routingStatus() = false;
    return false;
  }

  else
  {
    return true;
  }
}

void
RoutingController::processRoutingBreakTravel(RtgKey& rKey,
                                             PricingTrx& trx,
                                             TravelRoute& tvlRoute,
                                             RoutingInfo* routingInfo,
                                             TravelRoute& baseTvlRoute,
                                             TravelRoute& origAddOnTvlRoute,
                                             TravelRoute& destAddOnTvlRoute,
                                             const Routing* origAddOnRouting,
                                             const Routing* destAddOnRouting)
{
  buildBaseTravelRoute(origAddOnRouting, destAddOnRouting, baseTvlRoute, tvlRoute, rKey);
  updateCityCarrierVec(baseTvlRoute, *routingInfo, routingInfo->rtgBaseCityCarrier());

  if (origAddOnRouting != nullptr)
  {
    if (!buildAddOnTravelRoute(trx, origAddOnTvlRoute, tvlRoute, true))
    {
      LOG4CXX_DEBUG(logger, "origin travel route construction failed");
    }
    else
    {
      updateCityCarrierVec(origAddOnTvlRoute, *routingInfo, routingInfo->rtgAddon1CityCarrier());
    }
  }

  if (destAddOnRouting != nullptr)
  {
    if (!buildAddOnTravelRoute(trx, destAddOnTvlRoute, tvlRoute, false))
    {
      LOG4CXX_DEBUG(logger, "destination travel route construction failed");
    }
    else
    {
      updateCityCarrierVec(destAddOnTvlRoute, *routingInfo, routingInfo->rtgAddon2CityCarrier());
    }
  }
}

void
RoutingController::processRoutingProcessRestrictions(RtgKey& rKey,
                                                     PricingTrx& trx,
                                                     TravelRoute& tvlRoute,
                                                     PaxTypeFare* paxTypeFare,
                                                     RoutingInfo* routingInfo,
                                                     TravelRoute& origAddOnTvlRoute,
                                                     TravelRoute& destAddOnTvlRoute,
                                                     const Routing* routing,
                                                     const Routing* origAddOnRouting,
                                                     const Routing* destAddOnRouting,
                                                     bool& isRoutingValid)
{
  if ((routing != nullptr && !(routing->rests().empty())) ||
      (origAddOnRouting != nullptr && !origAddOnRouting->rests().empty()) ||
      (destAddOnRouting != nullptr && !destAddOnRouting->rests().empty()))
  {
    paxTypeFare->setIsRouting(true);

    RestrictionInfos* restrictionInfos;
    trx.dataHandle().get(restrictionInfos); // lint !e530
    routingInfo->restrictions() = restrictionInfos;

    Except3_12and17 except(origAddOnRouting != nullptr || destAddOnRouting != nullptr);
    std::vector<RoutingRestriction*> baseRouteRestrictions;
    std::vector<RoutingRestriction*> origRouteRestrictions;
    std::vector<RoutingRestriction*> destRouteRestrictions;
    if (LIKELY(routing))
      std::remove_copy_if(routing->rests().begin(),
                          routing->rests().end(),
                          std::back_inserter(baseRouteRestrictions),
                          std::not1(except));
    if (origAddOnRouting)
      std::remove_copy_if(origAddOnRouting->rests().begin(),
                          origAddOnRouting->rests().end(),
                          std::back_inserter(origRouteRestrictions),
                          std::not1(except));
    if (destAddOnRouting)
      std::remove_copy_if(destAddOnRouting->rests().begin(),
                          destAddOnRouting->rests().end(),
                          std::back_inserter(destRouteRestrictions),
                          std::not1(except));

    isRoutingValid =
        (baseRouteRestrictions.empty() ||
         processRestrictions(
             *paxTypeFare, trx, tvlRoute, routingInfo, routing, baseRouteRestrictions)) &&
        (origRouteRestrictions.empty() ||
         processRestrictions(
             *paxTypeFare, trx, tvlRoute, routingInfo, routing, origRouteRestrictions)) &&
        (destRouteRestrictions.empty() ||
         processRestrictions(
             *paxTypeFare, trx, tvlRoute, routingInfo, routing, destRouteRestrictions));

    //--------------------------------------------------------
    // Process Restrictions 3 and 17 for constructed routings.
    //--------------------------------------------------------
    if (origAddOnRouting != nullptr || destAddOnRouting != nullptr)
    {
      TravelRoute baseTravelRoute;
      buildBaseTravelRouteForRestrictions(baseTravelRoute, tvlRoute, rKey);
      updateCityCarrierVec(baseTravelRoute, *routingInfo, routingInfo->restBaseCityCarrier());

      // prepare combined travel routes for restrictions 3 and 17
      // e.g. if origin add-on routing and specified routing have the same routing number,
      // restriction 3 and 17 must be validated against joined travel route (add-on + base),
      // not only against the travel route portion it is attached to.
      TravelRoute origAddOnPlusBaseTR, basePlusDestAddOnTR;
      const TravelRoute* origAddOnTR, *baseTR, *destAddOnTR;
      if (origAddOnRouting != nullptr && routing != nullptr &&
          origAddOnRouting->routing() == routing->routing())
      {
        if (destAddOnRouting != nullptr && routing->routing() == destAddOnRouting->routing())
        {
          origAddOnTR = baseTR = destAddOnTR = &tvlRoute;
        }
        else
        {
          origAddOnPlusBaseTR =
              add(origAddOnTvlRoute, baseTravelRoute, tvlRoute.unticketedPointInd());
          origAddOnTR = baseTR = &origAddOnPlusBaseTR;
          destAddOnTR = &destAddOnTvlRoute;
        }
      }

      else if (routing != nullptr && destAddOnRouting != nullptr &&
               routing->routing() == destAddOnRouting->routing())
      {
        basePlusDestAddOnTR =
            add(baseTravelRoute, destAddOnTvlRoute, tvlRoute.unticketedPointInd());
        origAddOnTR = &origAddOnTvlRoute;
        baseTR = destAddOnTR = &basePlusDestAddOnTR;
      }

      else
      {
        origAddOnTR = &origAddOnTvlRoute;
        baseTR = &baseTravelRoute;
        destAddOnTR = &destAddOnTvlRoute;
      }

      // validate restrictions 3 and 17 against prepared travel route portions
      isRoutingValid &=
          processAddOnRestrictions(routing, *paxTypeFare, trx, *baseTR, routingInfo, Base);
      if (origAddOnRouting != nullptr)
      {
        isRoutingValid &= processAddOnRestrictions(
            origAddOnRouting, *paxTypeFare, trx, *origAddOnTR, routingInfo, OrigAddOn);
      }

      if (destAddOnRouting != nullptr)
      {
        isRoutingValid &= processAddOnRestrictions(
            destAddOnRouting, *paxTypeFare, trx, *destAddOnTR, routingInfo, DestAddOn);
      }
    }

    //-------------------------------------------------
    // Process Restriction 17's for Specified Routings.
    //-------------------------------------------------
    else
    {
      isRoutingValid &=
          processAddOnRestrictions(routing, *paxTypeFare, trx, tvlRoute, routingInfo, Whole);
    }
  }
}

void
RoutingController::processRoutingMileageRouting(RtgKey& rKey,
                                                PricingTrx& trx,
                                                TravelRoute& tvlRoute,
                                                PaxTypeFare* paxTypeFare,
                                                RoutingInfo* routingInfo,
                                                bool& isRoutingValid,
                                                bool& isMileageValid)
{
  if (rKey.routingNumber() == MILEAGE_ROUTING || rKey.addOnRouting1() == MILEAGE_ROUTING ||
      rKey.addOnRouting2() == MILEAGE_ROUTING)
  {
    paxTypeFare->setIsRouting(false);

    if(trx.getOptions() && trx.getOptions()->isRtw())
    {
      isMileageValid = false;
      isRoutingValid = false;
      return;
    }

    if (!mileageValidated(*paxTypeFare))
    {
      MileageInfo* mileageInfo;
      trx.dataHandle().get(mileageInfo); // lint !e530
      routingInfo->mileageInfo() = mileageInfo;
      if (isRoutingValid && !validateMileage(trx, *paxTypeFare, tvlRoute, *mileageInfo))
      {
        // do not ivalidate fare due to mileage before considering surcharge exception
        isMileageValid = false;
      }
    }

    else
    {
      routingInfo->mileageInfo() = paxTypeFare->mileageInfo();
      // do not ivalidate fare due to mileage before considering surcharge exception
      if (isRoutingValid && routingInfo->mileageInfo()->valid() == false)
      {
        isMileageValid = false;
      }
    }
    updateFareSurcharge(*paxTypeFare, *(routingInfo->mileageInfo()));
    if (!isMileageValid && routingInfo->mileageInfo()->surchargeAmt() == 0)
    {
      isRoutingValid = false;
    }

    if (routingInfo->mileageInfo()->psrApplies())
    {
      paxTypeFare->setIsRouting(true);
    }
  }

  else
  {
    paxTypeFare->setIsRouting(true);
  }
}

void
RoutingController::processRoutingRoutMapValidation(RtgKey& rKey,
                                                   PricingTrx& trx,
                                                   TravelRoute& tvlRoute,
                                                   PaxTypeFare* paxTypeFare,
                                                   RoutingInfo* routingInfo,
                                                   TravelRoute& baseTvlRoute,
                                                   TravelRoute& origAddOnTvlRoute,
                                                   TravelRoute& destAddOnTvlRoute,
                                                   MapInfo*& mapInfo,
                                                   MapInfo*& rtgAddonMapInfo,
                                                   const Routing* routing,
                                                   const Routing* origAddOnRouting,
                                                   const Routing* destAddOnRouting,
                                                   const DateTime& travelDate,
                                                   bool& isRoutingValid)
{

  if ((routing != nullptr && !routing->rmaps().empty()) ||
      (routingInfo->origAddOnRouting() && (!routingInfo->origAddOnRouting()->rmaps().empty())) ||
      (routingInfo->destAddOnRouting() && (!routingInfo->destAddOnRouting()->rmaps().empty())))
  {

    //--------------------------------------
    // Validate maps for Specified Routings
    //--------------------------------------
    if (origAddOnRouting == nullptr && destAddOnRouting == nullptr)
    {
      if (LIKELY(isRoutingValid && !routing->rmaps().empty()))
      {
        isRoutingValid = validateRoutingMaps(trx,
                                             *paxTypeFare,
                                             tvlRoute,
                                             mapInfo,
                                             routing,
                                             travelDate,
                                             routingInfo->origAddOnRouting(),
                                             routingInfo->destAddOnRouting());
        routingInfo->mapInfo() = mapInfo;
      }
    }

    //-------------------------------------------------
    // Process Map Validation for Constructed Routings
    //-------------------------------------------------
    else
    {
      //------------------------------------------------------------------------------------
      // Mileage Base with Routing Addon(s) OR  Routing Base with No Maps (Restriction Only)
      //------------------------------------------------------------------------------------
      if (rKey.routingNumber() == MILEAGE_ROUTING || routing->rmaps().empty())
      {
        if (rKey.addOnRouting1() != MILEAGE_ROUTING && routingInfo->origAddOnRouting() != nullptr &&
            !routingInfo->origAddOnRouting()->rmaps().empty() && !origAddOnTvlRoute.empty())
        {
          isRoutingValid = validateRoutingMaps(trx,
                                               *paxTypeFare,
                                               origAddOnTvlRoute,
                                               mapInfo,
                                               routingInfo->origAddOnRouting(),
                                               travelDate,
                                               nullptr,
                                               nullptr);
        }

        if (mapInfo != nullptr && mapInfo->routeStrings() != nullptr)
        {
          routingInfo->mapInfo() = mapInfo;
          mapInfo->processed() = true;
        }

        if (rKey.addOnRouting2() != MILEAGE_ROUTING && routingInfo->destAddOnRouting() != nullptr &&
            !routingInfo->destAddOnRouting()->rmaps().empty() && !destAddOnTvlRoute.empty())
        {

          isRoutingValid = isRoutingValid && validateRoutingMaps(trx,
                                                                 *paxTypeFare,
                                                                 destAddOnTvlRoute,
                                                                 rtgAddonMapInfo,
                                                                 routingInfo->destAddOnRouting(),
                                                                 travelDate,
                                                                 nullptr,
                                                                 nullptr);
        }

        if (rtgAddonMapInfo != nullptr && rtgAddonMapInfo->routeStrings() != nullptr)
        {
          routingInfo->rtgAddonMapInfo() = rtgAddonMapInfo;
          rtgAddonMapInfo->processed() = true;
        }
      }

      //--------------------------------------------------------------------
      // Routing Base with Mileage Addon(s) or Routing Addon(s) with no map
      //--------------------------------------------------------------------
      else if (RoutingUtil::processBaseTravelRoute(*routingInfo))
      {
        if (routing != nullptr && !routing->rmaps().empty() && !baseTvlRoute.empty())
        {
          // prune the travel route if dest is gateway
          if (routingInfo->destAddOnRouting() != nullptr &&
              routingInfo->destAddOnRouting()->routing() == MILEAGE_ROUTING &&
              destAddOnTvlRoute.empty())
          {
            LOG4CXX_DEBUG(logger, "    dest removed from travel route");
            if (baseTvlRoute.travelRoute().empty() == false)
            {
              baseTvlRoute.travelRoute().pop_back();
            }
          }
          // prune the travel route if orig is gateway
          if (routingInfo->origAddOnRouting() != nullptr &&
              routingInfo->origAddOnRouting()->routing() == MILEAGE_ROUTING &&
              origAddOnTvlRoute.empty())
          {
            LOG4CXX_DEBUG(logger, "    dest removed from travel route");
            if (baseTvlRoute.travelRoute().empty() == false)
            {
              baseTvlRoute.travelRoute().erase(baseTvlRoute.travelRoute().begin());
            }
          }
          if (!baseTvlRoute.empty())
          {
            isRoutingValid = validateRoutingMaps(trx,
                                                 *paxTypeFare,
                                                 baseTvlRoute,
                                                 mapInfo,
                                                 routing,
                                                 travelDate,
                                                 routingInfo->origAddOnRouting(),
                                                 routingInfo->destAddOnRouting());
          }
          else
          {
            mapInfo->processed() = true;
          }
        }
        else
        {
          mapInfo->processed() = true;
        }
      }

      // ---------------------------------
      // Routing Base with Routing Addons
      //----------------------------------
      else
      {
        isRoutingValid = validateRoutingMaps(trx,
                                             *paxTypeFare,
                                             tvlRoute,
                                             mapInfo,
                                             routing,
                                             travelDate,
                                             routingInfo->origAddOnRouting(),
                                             routingInfo->destAddOnRouting());
      }
    }
    routingInfo->mapInfo() = mapInfo;
  }
}

bool
RoutingController::processRoutingValidation(RtgKey& rKey,
                                            PricingTrx& trx,
                                            TravelRoute& tvlRoute,
                                            PaxTypeFare* paxTypeFare,
                                            const Routing* routing,
                                            const Routing* origAddOnRouting,
                                            const Routing* destAddOnRouting,
                                            RoutingInfos& routingInfos,
                                            const DateTime& travelDate)
{
  LOG4CXX_DEBUG(logger,
                "Entered RoutingController::processRoutingValidation() for "
                    << paxTypeFare->fareMarket()->boardMultiCity() << "-"
                    << paxTypeFare->fareMarket()->offMultiCity() << " DIR "
                    << paxTypeFare->directionality())

  RoutingInfo* routingInfo;
  trx.dataHandle().get(routingInfo); // lint !e530
  routingInfos.insert(RoutingInfos::value_type(rKey, routingInfo));

  {
    boost::lock_guard<boost::mutex> g(*_routingKeyMapMutex);
    _routingKeyMap->insert(RoutingKeyMap::value_type(rKey, routingInfo));
  }

  routingInfo->rtgKey() = rKey;
  routingInfo->routing() = routing;
  routingInfo->origAddOnRouting() = origAddOnRouting;
  routingInfo->destAddOnRouting() = destAddOnRouting;
  RoutingUtil::updateRoutingInfo(*paxTypeFare, routing, *routingInfo, false, true);

  // missing data - don't process
  if (UNLIKELY(!processRoutingCheckMissingData(
                   rKey, routingInfo, routing, origAddOnRouting, destAddOnRouting)))
  {
    return false;
  }

  bool isRoutingValid(true);
  bool isMileageValid(true);

  //--------------------------------------------------------------------------
  // Break travel route into separate parts if constructed for Map Validation
  //--------------------------------------------------------------------------
  TravelRoute baseTvlRoute;
  TravelRoute origAddOnTvlRoute;
  TravelRoute destAddOnTvlRoute;

  //----------------------
  // Process Break Travel
  //----------------------
  processRoutingBreakTravel(rKey,
                            trx,
                            tvlRoute,
                            routingInfo,
                            baseTvlRoute,
                            origAddOnTvlRoute,
                            destAddOnTvlRoute,
                            origAddOnRouting,
                            destAddOnRouting);

  //----------------------
  // Process Restrictions
  //----------------------
  processRoutingProcessRestrictions(rKey,
                                    trx,
                                    tvlRoute,
                                    paxTypeFare,
                                    routingInfo,
                                    origAddOnTvlRoute,
                                    destAddOnTvlRoute,
                                    routing,
                                    origAddOnRouting,
                                    destAddOnRouting,
                                    isRoutingValid);

  MapInfo* mapInfo;
  trx.dataHandle().get(mapInfo); // lint !e530
  MapInfo* rtgAddonMapInfo;
  trx.dataHandle().get(rtgAddonMapInfo); // lint !e530

  bool areRestrictionsValid = isRoutingValid;

  //--------------------------
  // Process Mileage Routings
  //--------------------------
  processRoutingMileageRouting(
      rKey, trx, tvlRoute, paxTypeFare, routingInfo, areRestrictionsValid, isMileageValid);

  // if restrictions failed don't validate routing
  if (isRoutingValid)
  {
    //----------------------
    // Route Map validation
    //----------------------
    processRoutingRoutMapValidation(rKey,
                                    trx,
                                    tvlRoute,
                                    paxTypeFare,
                                    routingInfo,
                                    baseTvlRoute,
                                    origAddOnTvlRoute,
                                    destAddOnTvlRoute,
                                    mapInfo,
                                    rtgAddonMapInfo,
                                    routing,
                                    origAddOnRouting,
                                    destAddOnRouting,
                                    travelDate,
                                    isRoutingValid);
  }

  // setting flag
  routingInfo->routingMapStatus() = isRoutingValid;

  if (!areRestrictionsValid)
  // this part was taken from processRoutingRoutMapValidation
  // and is processed when processRoutingMileageRouting set flag - as before
  {
    if (rKey.addOnRouting2() != MILEAGE_ROUTING && routingInfo->destAddOnRouting() &&
        !routingInfo->destAddOnRouting()->rmaps().empty() && !destAddOnTvlRoute.empty())
    {
      routingInfo->rtgAddonMapInfo() = rtgAddonMapInfo;
      rtgAddonMapInfo->processed() = false;
    }
  }

  // isMileageValid - do not ivalidate fare due to mileage before considering surcharge exception
  routingInfo->routingStatus() = areRestrictionsValid && isMileageValid && isRoutingValid;
  return areRestrictionsValid && isRoutingValid;
}

bool
RoutingController::validateRoutingMaps(PricingTrx& trx,
                                       PaxTypeFare& paxTypeFare,
                                       TravelRoute& tvlRoute,
                                       MapInfo* mapInfo,
                                       const Routing* routing,
                                       const DateTime& travelDate,
                                       const Routing* origAddOnRouting,
                                       const Routing* destAddOnRouting)
{
  TSELatencyData metics(trx, "FVO ROUTING MAP");
  LOG4CXX_DEBUG(logger, " Entered RoutingController::validateRoutingMaps()");
  bool isRoutingValid(true);
  SpecifiedRoutingValidator specifiedValidator;
  {
    TSELatencyData metics(trx, "FVO MAP SPECIFIED");
    isRoutingValid = specifiedValidator.validate(
        trx, tvlRoute, routing, mapInfo, travelDate, origAddOnRouting, destAddOnRouting);

    mapInfo->processed() = true;
    mapInfo->valid() = isRoutingValid;
  }

  if (!isRoutingValid)
  {
    if (domesticRoutingValidationAllowed(trx, tvlRoute, routing, mapInfo))
    {
      TSELatencyData metics(trx, "FVO MAP DRV");
      DRVController drvController(trx);
      isRoutingValid = drvController.process(
          trx, paxTypeFare, routing, origAddOnRouting, destAddOnRouting, tvlRoute, mapInfo);
    }
  }

  return isRoutingValid;
}

bool
RoutingController::validateMileage(PricingTrx& trx,
                                   PaxTypeFare& paxTypeFare,
                                   const TravelRoute& tvlRoute,
                                   MileageInfo& mileageInfo)
{
  LOG4CXX_DEBUG(logger, "Entered RoutingController::validateMileage()");

  if (LIKELY(!mileageInfo.processed()))
  {
    TSELatencyData metics(trx, "FVO ROUTING MILEAGE");
    MileageValidator mileageValidator;
    Diag452Collector* diagPtr = nullptr;
    DCFactory* factory = nullptr;
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic452 &&
                 trx.diagnostic().shouldDisplay(*paxTypeFare.fareMarket())))
    {
      factory = DCFactory::instance();
      diagPtr = dynamic_cast<Diag452Collector*>(factory->create(trx));
      diagPtr->enable(Diagnostic452);
    }

    mileageValidator.validate(trx, mileageInfo, tvlRoute, &paxTypeFare, diagPtr);

    if (UNLIKELY(diagPtr))
    {
      diagPtr->flushMsg();
    }
    mileageInfo.processed() = true;
    updateFareSurcharge(paxTypeFare, mileageInfo);
    paxTypeFare.mileageInfo() = &mileageInfo;
  }
  else
  {
    updateFareSurcharge(paxTypeFare, mileageInfo);
  }

  LOG4CXX_DEBUG(logger, "Leaving RoutingController::validateMileage() : Successful");
  return mileageInfo.valid();
}

void
RoutingController::updateFareSurcharge(PaxTypeFare& paxTypeFare, const MileageInfo& mInfo)
{
  LOG4CXX_DEBUG(logger, "  RoutingController::updateFareSurcharge()");
  // update surcharge even if mileage is exceeded - needed for surcharge exception
  // if (mInfo.valid())
  //{
  // The fare market shall only be evaluated for exceptional fares for South Atlantic if
  // applicable TPM for South Atlantic Exception is greater than 0 otherwise it is not a
  // market where exception is applicable
  if (LIKELY(mInfo.totalApplicableTPMSAException() == NO_TPM))
  {
    paxTypeFare.mileageSurchargePctg() = mInfo.surchargeAmt();
    paxTypeFare.mileageSurchargeAmt() =
        (paxTypeFare.nucFareAmount() * mInfo.surchargeAmt()) / HUNDRED;
  }
  // The exception is only applicable for YY fares if Routing is valid
  else if (paxTypeFare.isRouting() && paxTypeFare.carrier() == INDUSTRY_CARRIER)
  {
    paxTypeFare.mileageSurchargePctg() = mInfo.surchargeAmtSAException();
    paxTypeFare.mileageSurchargeAmt() =
        (paxTypeFare.nucFareAmount() * mInfo.surchargeAmtSAException()) / HUNDRED;
  }
  else
  {
    paxTypeFare.mileageSurchargePctg() = mInfo.surchargeAmt();
    paxTypeFare.mileageSurchargeAmt() =
        (paxTypeFare.nucFareAmount() * mInfo.surchargeAmt()) / HUNDRED;
  }
  CurrencyUtil::truncateNUCAmount(paxTypeFare.mileageSurchargeAmt());

  paxTypeFare.rexSecondMileageSurchargeAmt() =
      (paxTypeFare.rexSecondNucFareAmount() * paxTypeFare.mileageSurchargePctg()) / HUNDRED;
  CurrencyUtil::truncateNUCAmount(paxTypeFare.rexSecondMileageSurchargeAmt());
}

bool
RoutingController::isPositiveRestriction(const RoutingRestriction* restr)
{
  // add rest 4
  return (restr->restriction() == ROUTING_RESTRICTION_1 ||
          restr->restriction() == ROUTING_RESTRICTION_2 ||
          restr->restriction() == ROUTING_RESTRICTION_4 ||
          restr->restriction() == ROUTING_RESTRICTION_5 ||
          restr->restriction() == ROUTING_RESTRICTION_7 ||
          restr->restriction() == RTW_ROUTING_RESTRICTION_8 ||
          restr->restriction() == RTW_ROUTING_RESTRICTION_9 ||
          restr->restriction() == RTW_ROUTING_RESTRICTION_10 ||
          restr->restriction() == ROUTING_RESTRICTION_18 ||
          restr->restriction() == ROUTING_RESTRICTION_19 ||
          restr->restriction() == ROUTING_RESTRICTION_21) &&
         (restr->negViaAppl() == PERMITTED || restr->negViaAppl() == REQUIRED);
}

bool
RoutingController::isRestSeqUseAndLogic(
    const std::vector<RoutingRestriction*>& allRouteRestrictions)
{
  std::vector<RoutingRestriction*>::const_iterator itr = allRouteRestrictions.begin();
  std::vector<RoutingRestriction*>::const_iterator end = allRouteRestrictions.end();
  for (; itr != end; itr++)
  {
    // logica applicable only for restriction2 set
    if (UNLIKELY((*itr)->restriction() == ROUTING_RESTRICTION_2 && (*itr)->negViaAppl() != PERMITTED &&
        (*itr)->negViaAppl() != REQUIRED))
      return true;
  }
  return false;
}

//------------------------------------------------------------------------------
//  Process Routing Restrictions and compare multiple restrictions of the
//  same type.
//------------------------------------------------------------------------------
bool
RoutingController::processRestrictions(PaxTypeFare& paxTypeFare,
                                       PricingTrx& trx,
                                       const TravelRoute& tvlRoute,
                                       RoutingInfo* routingInfo,
                                       const Routing* routing,
                                       const std::vector<RoutingRestriction*>& allRouteRestrictions)
{
  LOG4CXX_DEBUG(logger, " Entered RoutingController::processRestrictions()");

  bool isRoutingValid(true);
  RestrictionValidity validRestrictions;

  bool useAndLogic = isRestSeqUseAndLogic(allRouteRestrictions);

  std::vector<RoutingRestriction*>::const_iterator itr = allRouteRestrictions.begin();
  std::vector<RoutingRestriction*>::const_iterator end = allRouteRestrictions.end();

  for (; itr != end; itr++)
  {
    const RoutingRestriction* rest = *itr;

    RestrictionKey key(rest->restriction(), rest->market1(), rest->market2(), useAndLogic);
    RestrictionInfo info;
    const RestrictionNumber& restNo = rest->restriction();

    LOG4CXX_DEBUG(logger,
                  "Processing " << routing->vendor() << " / " << routing->carrier() << " / "
                                << routing->routingTariff() << " / " << routing->routing()
                                << " restriction %s" << restNo);

    if ((!useAndLogic || rest->restriction() != ROUTING_RESTRICTION_2) &&
        isPositiveRestriction(*itr))
    {
      RestrictionValidity::iterator it = validRestrictions.find(key);
      if (it == validRestrictions.end())
      {
        info.processed() = true;
        info.valid() = validateRestriction(trx, paxTypeFare, *rest, tvlRoute, routingInfo);
        validRestrictions.insert(RestrictionValidity::value_type(key, info.valid()));
      }
      else if (!(*it).second)
      {
        info.processed() = true;
        info.valid() = validateRestriction(trx, paxTypeFare, *rest, tvlRoute, routingInfo);
        if (info.valid())
        {
          (*it).second = true;
        }
      }
      routingInfo->restrictions()->insert(RestrictionInfos::value_type(rest, info));
    }
    else
    {
      info.processed() = true;
      info.valid() = validateRestriction(trx, paxTypeFare, *rest, tvlRoute, routingInfo);
      routingInfo->restrictions()->insert(RestrictionInfos::value_type(rest, info));

      if (restNo == MILEAGE_RESTRICTION_16)
      {
        // do not invalidate fare due to mileage before considering surcharge exceptions
        validRestrictions.insert(RestrictionValidity::value_type(key, true));
      }
      else
      {
        validRestrictions.insert(RestrictionValidity::value_type(key, info.valid()));

        if (!info.valid())
          isRoutingValid = false;
      }
    }
  }

  //----------------------------------------------
  // Check Multiple Restrictions of the same type

  //----------------------------------------------
  if (isRoutingValid)
  {
    isRoutingValid =
        (std::find_if(validRestrictions.begin(), validRestrictions.end(), RestrictionInvalid()) ==
         validRestrictions.end());
  }

  return isRoutingValid;
}

//------------------------------------------------------------------------------
//  Process Routing Restrictions and compare multiple restrictions of the
//  same type.
//------------------------------------------------------------------------------
bool
RoutingController::processAddOnRestrictions(const Routing* routing,
                                            PaxTypeFare& paxTypeFare,
                                            PricingTrx& trx,
                                            const TravelRoute& tvlRoute,
                                            RoutingInfo* routingInfo,
                                            RoutingComponent rtgComponent)
{
  if (routing == nullptr || routing->rests().empty() || tvlRoute.empty())
    return true;

  LOG4CXX_DEBUG(logger, " Entered RoutingController::processAddOnRestrictions()");

  bool isRoutingValid = true;

  RestrictionInfos* restrictionInfos = routingInfo->restrictions();

  std::set<CarrierCode> cxrSet;
  bool process17 = rtgComponent != Whole ||
                   (routing->rmaps().empty() && paxTypeFare.carrier() != INDUSTRY_CARRIER);
  bool isRTW = trx.getOptions() && trx.getOptions()->isRtw();
  std::vector<const RoutingRestriction*> list17;
  std::map<std::string, std::vector<const RoutingRestriction*> > nationPairMap;

  for (RoutingRestriction* rest : routing->rests())
  {
    RestrictionInfo info;
    info.base() = rtgComponent == Base || rtgComponent == Whole;
    info.origAddOn() = rtgComponent == OrigAddOn;
    info.destAddOn() = rtgComponent == DestAddOn;

    const RestrictionNumber& restNo = rest->restriction();

    if (rtgComponent != Whole && restNo == ROUTING_RESTRICTION_3 &&
        (rest->negViaAppl() == NOT_PERMITTED || rest->negViaAppl() == REQUIRED))
    {
      LOG4CXX_DEBUG(logger,
                    "Processing " << routing->vendor() << " / " << routing->carrier() << " / "
                                  << routing->routingTariff() << " / " << routing->routing()
                                  << " restriction " << restNo);
      info.processed() = true;
      info.valid() = validateRestriction(trx, paxTypeFare, *rest, tvlRoute, routingInfo);
      restrictionInfos->insert(RestrictionInfos::value_type(rest, info));

      isRoutingValid &= info.valid();
    }
    else if (process17 && restNo == ROUTING_RESTRICTION_17 && rest->negViaAppl() == ' ')
    {
      cxrSet.insert(rest->viaCarrier());
      list17.insert(list17.end(), rest);
    }
    else if (isRTW && restNo == RTW_ROUTING_RESTRICTION_12)
    {
      groupRestriction12(trx, rest, nationPairMap);
    }
  }

  if(isRoutingValid && !nationPairMap.empty())
      isRoutingValid &= validateRestriction12(trx.dataHandle(), tvlRoute, nationPairMap, routingInfo);

  //---------------------------------------------------
  // Process Restriction 17's for Constructed Routings
  //---------------------------------------------------
  if (isRoutingValid && process17 && !cxrSet.empty())
    isRoutingValid &= validateRestriction17(tvlRoute, trx, routingInfo, cxrSet, list17);

  return isRoutingValid;
}

void
RoutingController::groupRestriction12(
    PricingTrx& trx,
    const RoutingRestriction* rest,
    std::map<std::string, std::vector<const RoutingRestriction*>>& nationPairMap)
{
  std::string key = rest->market1type() + rest->market1() + rest->market2type() + rest->market2();
  if (rest->market1type() == MapNode::CITY && rest->market2type() == MapNode::CITY)
  {
    const Loc* market1Loc = trx.dataHandle().getLoc(rest->market1(), trx.travelDate());
    const Loc* market2Loc = trx.dataHandle().getLoc(rest->market2(), trx.travelDate());
    if (market1Loc && market2Loc)
    {
      const NationCode& nation1 = getDomesticNationGroup(market1Loc->nation());
      const NationCode& nation2 = getDomesticNationGroup(market2Loc->nation());

      if (nation1 < nation2)
        key = nation1 + nation2;
      else
        key = nation2 + nation1;
    }
  }
  nationPairMap[key].push_back(rest);
}

const NationCode&
RoutingController::getDomesticNationGroup(const NationCode& nation)
{
  if (nation == EAST_URAL_RUSSIA)
    return RUSSIA;

  if (nation == CANADA)
    return UNITED_STATES;

  return nation;
}

void
RoutingController::createCityGroups(const std::vector<const RoutingRestriction*> restrictions12,
                                    FlatSet<std::pair<Indicator, LocCode>>& cityGroup1,
                                    FlatSet<std::pair<Indicator, LocCode>>& cityGroup2)
{
  for (const RoutingRestriction* restriction : restrictions12)
  {
    cityGroup1.unsafe_insert(
        std::make_pair(restriction->market1type(), restriction->market1()));
    cityGroup2.unsafe_insert(
        std::make_pair(restriction->market2type(), restriction->market2()));
  }

  cityGroup1.order();
  cityGroup2.order();
}

bool
RoutingController::mileageValidated(PaxTypeFare& paxTypeFare)
{
  return paxTypeFare.mileageInfo() != nullptr;
}

bool
RoutingController::validateRestriction(PricingTrx& trx,
                                       PaxTypeFare& paxTypeFare,
                                       const RoutingRestriction& restriction,
                                       const TravelRoute& tvlRoute,
                                       RoutingInfo* routingInfo)
{
  const RestrictionNumber& restNo(restriction.restriction());
  if (restNo == MILEAGE_RESTRICTION_16)
  {
    if(trx.getOptions() && trx.getOptions()->isRtw())
      return true;

    if (!mileageValidated(paxTypeFare))
    {
      MileageInfo* mileageInfo;
      trx.dataHandle().get(mileageInfo); // lint !e530
      routingInfo->mileageInfo() = mileageInfo;
      return validateMileage(trx, paxTypeFare, tvlRoute, *mileageInfo);
    }
    routingInfo->mileageInfo() = paxTypeFare.mileageInfo();
    updateFareSurcharge(paxTypeFare, *(routingInfo->mileageInfo()));
    return routingInfo->mileageInfo()->valid();
  }
  else
  {
    TSELatencyData metics(trx, "FVO ROUTING RESTRICTION");
    RestrictionValidatorFactory& rVFactory(tse::Singleton<RestrictionValidatorFactory>::instance());
    RestrictionValidator* validator =
        rVFactory.getRestrictionValidator(restriction.restriction(), trx);
    if (LIKELY(validator))
    {
      return validator->validate(tvlRoute, restriction, trx);
    }
  }
  LOG4CXX_INFO(logger, " RestrictionValidatorFactory Failed to Create Validator");
  return false;
}

class PopulateRestrictionInfo : public std::unary_function<const RoutingRestriction*, void>
{
public:
  PopulateRestrictionInfo(RestrictionInfos* infos, bool result) : _infos(infos), _result(result) {}
  void operator()(const RoutingRestriction* r)
  {
    RestrictionInfo info;
    info.processed() = true;
    info.valid() = _result;
    _infos->insert(RestrictionInfos::value_type(r, info));
  }

private:
  RestrictionInfos* _infos;
  bool _result;
};

bool
RoutingController::validateRestriction12(DataHandle& dataHandle, const TravelRoute& tvlRoute,
                                         std::map<std::string, std::vector<const RoutingRestriction*> >& nationPairMap,
                                         RoutingInfo* routingInfo)
{
  bool res = true;
  StopTypeRestrictionValidator validator;

  for (const auto& item : nationPairMap)
  {
    uint32_t nonStops = 0;

    if (item.second.size() == 1)
    {
      for (const RoutingRestriction* restriction : item.second)
      {
        nonStops += validator.countNonStops(dataHandle, tvlRoute.travelRoute(),
            std::make_pair(restriction->market1type(), restriction->market1()),
            std::make_pair(restriction->market2type(), restriction->market2()));
      }
    }
    else
    {
      FlatSet<std::pair<Indicator, LocCode>> cityGroup1, cityGroup2;

      createCityGroups(item.second, cityGroup1, cityGroup2);

      for (const auto &city1 : cityGroup1)
        for (const auto &city2 : cityGroup2)
          nonStops += validator.countNonStops(dataHandle, tvlRoute.travelRoute(), city1, city2);
    }

    bool result = nonStops < 2;

    std::for_each(item.second.begin(), item.second.end(),
                  PopulateRestrictionInfo(routingInfo->restrictions(), result));

    if (!result)
      res = false;
  }

  return res;
}

bool
RoutingController::validateRestriction17(const TravelRoute& tvlRoute,
                                         PricingTrx& trx,
                                         RoutingInfo* routingInfo,
                                         std::set<CarrierCode>& cxrSet,
                                         std::vector<const RoutingRestriction*>& list17)
{
  LOG4CXX_DEBUG(logger, " Entered RoutingController::validateRestriction17()");

  //-----------------------------------------------------------------------------
  // Validate the carriers in the travelRoute against the unique set of carriers.
  //-----------------------------------------------------------------------------
  bool result = true;
  cxrSet.insert(tvlRoute.govCxr());

  for (const TravelRoute::CityCarrier& cityCarrier : tvlRoute.travelRoute())
  {
    if (cityCarrier.carrier() == SURFACE_CARRIER)
    {
      continue;
    }

    if (cxrSet.find(cityCarrier.carrier()) == cxrSet.end())
    {
      if (trx.getOptions()->isRtw())
      {
        const std::vector<AirlineAllianceCarrierInfo*>& allianceOptCxrInfoVec =
          trx.dataHandle().getAirlineAllianceCarrier(cityCarrier.carrier());
        if (allianceOptCxrInfoVec.empty() ||
            cxrSet.find(allianceOptCxrInfoVec.front()->genericAllianceCode()) == cxrSet.end())
        {
          result = false;
        }
      }
      else
        result = false; // At least one instance of carrier does not match
    }
  }

  //-------------------------------
  // Populate the restrictionInfos
  //-------------------------------
  std::for_each(
      list17.begin(), list17.end(), PopulateRestrictionInfo(routingInfo->restrictions(), result));

  LOG4CXX_INFO(logger, " Leaving RoutingController::validateRestriction17()");
  return result;
}

//---------------------------------------------------------------------------------
// Build a Routing Key for Routing Retrieval
//---------------------------------------------------------------------------------
void
RoutingController::buildRtgKey(PaxTypeFare& paxTypeFare,
                               const FareMarket& fareMarket,
                               const Routing* routing,
                               const Routing*& origAddOnRouting,
                               const Routing*& destAddOnRouting,
                               RtgKey& rKey)
{

  //------------------------------------------------
  // Mileage Routings for SITA have no routing data
  //------------------------------------------------
  if (routing == nullptr)
  {
    rKey.vendor() = paxTypeFare.vendor();
    rKey.carrier() = fareMarket.governingCarrier();
    rKey.routingTariff() = paxTypeFare.tcrRoutingTariff1();
    rKey.routingNumber() = paxTypeFare.routingNumber();
  }
  else
  {
    rKey.vendor() = routing->vendor();
    rKey.carrier() = routing->carrier();
    rKey.routingTariff() = routing->routingTariff();
    rKey.routingNumber() = routing->routing();
  }

  if (paxTypeFare.hasConstructedRouting())
  {
    if (origAddOnRouting != nullptr)
    {
      rKey.addOnRouting1() = origAddOnRouting->routing();
    }
    else
    {
      if (UNLIKELY((paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
                    paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED) &&
                   paxTypeFare.origAddonRouting() != MILEAGE_ROUTING))
      {
        rKey.addOnRouting1() = paxTypeFare.origAddonRouting();
      }
    }

    if (destAddOnRouting != nullptr)
    {
      rKey.addOnRouting2() = destAddOnRouting->routing();
    }
    else
    {
      if (UNLIKELY((paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
           paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED) &&
          paxTypeFare.destAddonRouting() != MILEAGE_ROUTING))
      {
        rKey.addOnRouting2() = paxTypeFare.destAddonRouting();
      }
    }

    if (paxTypeFare.fare()->isReversed())
    {
      const Routing* savedOrigAddOnRouting = origAddOnRouting;
      origAddOnRouting = destAddOnRouting;
      destAddOnRouting = savedOrigAddOnRouting;

      RoutingNumber savedRtgNumber = rKey.addOnRouting1();
      rKey.addOnRouting1() = rKey.addOnRouting2();
      rKey.addOnRouting2() = savedRtgNumber;
    }
  }

  if (rKey.routingNumber() == MILEAGE_ROUTING || rKey.addOnRouting1() == MILEAGE_ROUTING ||
      rKey.addOnRouting2() == MILEAGE_ROUTING)
    rKey.dirOutbound() =
        (paxTypeFare.directionality() == FROM || paxTypeFare.isOutboundFareForCarnivalInbound());
  else
    rKey.dirOutbound() = true;
}

//---------------------------------------------------------------------------------
// Determine whether Domestic Routing Validation is allowed for this
// TravelRoute and Routing.
//---------------------------------------------------------------------------------
bool
RoutingController::domesticRoutingValidationAllowed(PricingTrx& trx,
                                                    TravelRoute& tvlRoute,
                                                    const Routing* routing,
                                                    MapInfo* mapInfo)
{
  if (tvlRoute.doNotApplyDRV() ||
      routing->domRtgvalInd() == NO_DOMESTIC_ROUTE_VALIDATION)
  {
    return false;
  }

  if (UNLIKELY(trx.getOptions() &&
      trx.getOptions()->isRtw()))
  {
    for (const RoutingMap* rMap : routing->rmaps())
    {
      if(rMap->loc1().locType() == MapNode::NATION || rMap->loc1().locType() == MapNode::ZONE)
        return false;
    }
    return true;
  }

  const Loc* orig = tvlRoute.mileageTravelRoute().front()->origin();
  const Loc* dest = tvlRoute.mileageTravelRoute().back()->destination();

  return LocUtil::isInternational(*orig, *dest);
}

/**
* Build travel route for the add on portion of a constructed route
*
* @param addOnTvlRoute - new add on route
* @param tvlRoute - original constructed route
* @param orig - true = origin, false = destination
* @returns bool, false if no add on
*/
bool
RoutingController::buildAddOnTravelRoute(PricingTrx& trx,
                                         TravelRoute& addOnTvlRoute,
                                         TravelRoute& tvlRoute,
                                         bool orig)
{
  std::vector<TravelSeg*> tvlSegs;

  const LocCode& locCode = orig ? tvlRoute.travelRoute().front().boardCity().loc()
                                : tvlRoute.travelRoute().back().offCity().loc();

  const Loc* loc = trx.dataHandle().getLoc(locCode, tvlRoute.travelDate());

  if (!buildTravelSegs(tvlRoute, loc, tvlSegs))
    return false;

  if (UNLIKELY(tvlSegs.empty()))
    return false;

  addOnTvlRoute.geoTravelType() = TravelSegUtil::getGeoTravelType(tvlSegs, _trx.dataHandle());
  TravelRouteBuilder trb;

  return trb.buildTravelRoute(_trx, tvlSegs, addOnTvlRoute, tvlRoute.unticketedPointInd());
}

/**
* Build travel route for the base portion of a constructed route for Map Validation
* @param addOnTvlRoute - new travel route
* @param tvlRoute - original constructed route
* @param rKey - routing key
* @returns bool, false if error
*/
bool
RoutingController::buildBaseTravelRouteForRestrictions(TravelRoute& newTvlRoute,
                                                       TravelRoute& tvlRoute,
                                                       RtgKey& rKey)
{
  std::vector<TravelSeg*> newTvlSegs;

  if (UNLIKELY(!buildTvlSegs(tvlRoute, newTvlRoute, newTvlSegs, rKey)))
    return false;

  if (UNLIKELY(newTvlSegs.empty()))
    return false;

  newTvlRoute.geoTravelType() = TravelSegUtil::getGeoTravelType(newTvlSegs, _trx.dataHandle());
  TravelRouteBuilder trb;

  return trb.buildTravelRoute(_trx, newTvlSegs, newTvlRoute, tvlRoute.unticketedPointInd());
}

/**
* Build a vector of travel segs for Routing Base to validate
* routing restrictions 3 and 17.
*/
bool
RoutingController::buildTvlSegs(TravelRoute& tvlRoute,
                                TravelRoute& newTvlRoute,
                                std::vector<TravelSeg*>& newTvlSegs,
                                RtgKey& rKey)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::buildTravelSegs()");

  bool origGatewayFound = false;
  const Loc* origLoc = tvlRoute.mileageTravelRoute().front()->origin();
  const Loc* destLoc = tvlRoute.mileageTravelRoute().back()->destination();

  //----------------------------------------------------------------------------
  // Iterate through the vector of TravelSegs.
  // Copy qualifying travelSegs to a temporaryVector of travelSegs
  //----------------------------------------------------------------------------
  std::vector<TravelSeg*>& tvlSegs = tvlRoute.mileageTravelRoute();
  std::vector<TravelSeg*>::iterator start = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator end = tvlSegs.end();

  for (; start != end; ++start)
  {
    if (!rKey.addOnRouting1().empty())
    {
      if (!origGatewayFound)
      {
        //-------------------------------------------------------------------------
        // Move this tvlSeg only if the Orig AND Dest of the TvlSeg are NOT in the
        // Origin Country.  Start from the gateway in the origin country.
        //-------------------------------------------------------------------------
        if (!RoutingUtil::locMatchesTvlSeg(origLoc, **start))
        {
          newTvlSegs.push_back((*start));
          origGatewayFound = true;
        }
      }
      else
      {
        if (!rKey.addOnRouting2().empty())
        {
          //---------------------------------------------------------------------
          // Move this tvlSeg only if the Orig AND Dest of the tvlSeg are NOT in
          // the Destination Country.  Build only to the Gateway.
          //---------------------------------------------------------------------
          if (!RoutingUtil::locMatchesTvlSeg(destLoc, **start))
          {
            newTvlSegs.push_back((*start));
          }
        }
        else
        {
          newTvlSegs.push_back((*start));
        }
      }
    }
    else
    {
      if (LIKELY(!rKey.addOnRouting2().empty()))
      {
        //---------------------------------------------------------------------
        // Move this tvlSeg only if the Orig AND Dest of the tvlSeg are NOT in
        // the Destination Country.  Build only to the Gateway.
        //---------------------------------------------------------------------
        if (!RoutingUtil::locMatchesTvlSeg(destLoc, **start))
        {
          newTvlSegs.push_back((*start));
        }
      }
    }
  }

  if (LIKELY(!newTvlSegs.empty()))
  {
    newTvlRoute.doNotApplyDRV() = true;
    return true;
  }

  return false;
}

/**
* Build travel route for the base portion of a constructed route for
* Map Validation
*
* @param addOnTvlRoute - new travel route
* @param tvlRoute - original constructed route
* @param rKey - routing key
* @returns bool, false if error
*/
bool
RoutingController::buildBaseTravelRoute(const Routing* origAddOnRouting,
                                        const Routing* destAddOnRouting,
                                        TravelRoute& newTvlRoute,
                                        TravelRoute& tvlRoute,
                                        RtgKey& rKey)
{
  std::vector<TravelSeg*> newTvlSegs;

  if (!buildTravelSegs(origAddOnRouting, destAddOnRouting, tvlRoute, newTvlRoute, newTvlSegs, rKey))
    return false;

  if (UNLIKELY(newTvlSegs.empty()))
    return false;

  newTvlRoute.geoTravelType() = TravelSegUtil::getGeoTravelType(newTvlSegs, _trx.dataHandle());
  TravelRouteBuilder trb;

  return trb.buildTravelRoute(_trx, newTvlSegs, newTvlRoute, tvlRoute.unticketedPointInd());
}

/**
* Build a vector of travel segs for Routing Addons
*/
bool
RoutingController::buildTravelSegs(TravelRoute& tvlRoute,
                                   const Loc* loc,
                                   std::vector<TravelSeg*>& newTvlSegs)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::buildTravelSegs()");

  if (UNLIKELY(!loc))
    return false;

  bool primarySectorFound = false;

  //----------------------------------------------------------------------------
  // Iterate through the vector of TravelSegs.
  // Copy qualifying travelSegs to a temporaryVector of travelSegs
  //----------------------------------------------------------------------------
  std::vector<TravelSeg*>& tvlSegs = tvlRoute.mileageTravelRoute();
  std::vector<TravelSeg*>::iterator start = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator end = tvlSegs.end();

  for (; start != end; ++start)
  {
    if (tvlRoute.primarySector() == *start && RoutingUtil::locMatchesOrigin(loc, tvlRoute))
    {
      break; // All TravelSegs accumulated
    }

    else if (tvlRoute.primarySector() == *start &&
             RoutingUtil::locMatchesDestination(loc, tvlRoute))
    {
      primarySectorFound = true;
    }

    //------------------------------------------------------------------
    // All Travel Must be in the Same Country
    //------------------------------------------------------------------
    else if (RoutingUtil::locMatchesOrigin(loc, tvlRoute) || primarySectorFound)
    {
      if (RoutingUtil::locMatchesTvlSeg(loc, **start))
      {
        newTvlSegs.push_back((*start));
      }
    }
  }

  if (!newTvlSegs.empty())
  {
    return true;
  }

  return false;
}

//--------------------------------------------------------------------
// Build a vector of travel segs for Routing Base with Mileage Addons
//
// Base Routing + MPM Addon
//     Build Origin to Gateway in Destination Country
//
// MPM Addon + Base Routing + MPM Addon
//     Build from Gateway in Origin Country to Gateway in
//     Destination Country.
//
// MPM Addon + Base Routing
//     Build from Gateway in Origin Country to Destination
//
//--------------------------------------------------------------------
bool
RoutingController::buildTravelSegs(const Routing* origAddOnRouting,
                                   const Routing* destAddOnRouting,
                                   TravelRoute& tvlRoute,
                                   TravelRoute& newTvlRoute,
                                   std::vector<TravelSeg*>& newTvlSegs,
                                   RtgKey& rKey)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::buildTravelSegs()");

  bool origGatewayFound = false;
  const Loc* origLoc = tvlRoute.mileageTravelRoute().front()->origin();
  const Loc* destLoc = tvlRoute.mileageTravelRoute().back()->destination();

  bool origAddonHasNoMap = (rKey.addOnRouting1() == MILEAGE_ROUTING ||
                            (origAddOnRouting && origAddOnRouting->rmaps().empty()));

  bool destAddonHasNoMap = (rKey.addOnRouting2() == MILEAGE_ROUTING ||
                            (destAddOnRouting && destAddOnRouting->rmaps().empty()));

  //----------------------------------------------------------------------------
  // Iterate through the vector of TravelSegs.
  // Copy qualifying travelSegs to a temporaryVector of travelSegs
  //----------------------------------------------------------------------------
  std::vector<TravelSeg*>& tvlSegs = tvlRoute.mileageTravelRoute();
  std::vector<TravelSeg*>::iterator start = tvlSegs.begin();
  std::vector<TravelSeg*>::iterator end = tvlSegs.end();

  for (; start != end; ++start)
  {
    if (origAddonHasNoMap)
    {
      if (!origGatewayFound)
      {
        // Move this tvlSeg only if the Orig AND Dest of the TvlSeg are NOT in the
        // Origin Country.  Start from the gateway in the origin country.
        if (!RoutingUtil::locMatchesTvlSeg(origLoc, **start))
        {
          newTvlSegs.push_back((*start));
          origGatewayFound = true;
        }
      }
      else
      {
        if (destAddonHasNoMap)
        {
          // Move this tvlSeg only if the Orig AND Dest of the tvlSeg are NOT in
          // the Destination Country.  Build only to the Gateway.
          if (!RoutingUtil::locMatchesTvlSeg(destLoc, **start))
          {
            newTvlSegs.push_back((*start));
          }
        }
        else
        {
          newTvlSegs.push_back((*start));
        }
      }
    }
    else
    {
      if (destAddonHasNoMap)
      {
        // Move this tvlSeg only if the Orig AND Dest of the tvlSeg are NOT in
        // the Destination Country.  Build only to the Gateway.
        if (!RoutingUtil::locMatchesTvlSeg(destLoc, **start))
        {
          newTvlSegs.push_back((*start));
        }
      }
    }
  }

  if (!newTvlSegs.empty())
  {
    newTvlRoute.doNotApplyDRV() = true;
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------------
// Update the RoutingInfo with the vector of cityCarriers for the local or intl tvl route
//-----------------------------------------------------------------------------------
void
RoutingController::updateCityCarrierVec(TravelRoute& tvlRoute,
                                        RoutingInfo& drvInfo,
                                        DRVCityCarrier& cityCarrierVec)
{
  LOG4CXX_DEBUG(logger, " Entering RoutingController::updateCityCarrierVec()");

  cityCarrierVec.insert(
      cityCarrierVec.end(), tvlRoute.travelRoute().begin(), tvlRoute.travelRoute().end());

  LOG4CXX_DEBUG(logger, " Leaving RoutingController::updateCityCarrierVec()");
}

const Routing*
RoutingController::getRoutingData(PricingTrx& trx, PaxTypeFare& paxTypeFare) const
{
  return RoutingUtil::getRoutingData(trx, paxTypeFare, paxTypeFare.routingNumber());
}

//-----------------------------------------------------------------------------------
// Update the RoutingInfo with the vector of cityCarriers for the local or intl tvl route
//-----------------------------------------------------------------------------------
void
RoutingController::getRoutings(PricingTrx& trx,
                               PaxTypeFare& paxTypeFare,
                               const Routing*& routing,
                               const Routing*& origAddOnRouting,
                               const Routing*& destAddOnRouting)
{
  LOG4CXX_INFO(logger, " Entering RoutingController::getRoutings()");

  const FareMarket* fm = paxTypeFare.fareMarket();

  routing = getRoutingData(trx, paxTypeFare);

  if (paxTypeFare.hasConstructedRouting())
  {
    if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
        paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    {
      origAddOnRouting =
          RoutingUtil::getRoutingData(trx, paxTypeFare, paxTypeFare.origAddonRouting());
      if (UNLIKELY(!origAddOnRouting && paxTypeFare.origAddonRouting() == MILEAGE_ROUTING &&
          paxTypeFare.vendor() == Vendor::SITA))
      {
        origAddOnRouting = RoutingUtil::getStaticRoutingMileageHeaderData(
            trx, fm->governingCarrier(), paxTypeFare);
      }
    }

    if (paxTypeFare.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
        paxTypeFare.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    {
      destAddOnRouting =
          RoutingUtil::getRoutingData(trx, paxTypeFare, paxTypeFare.destAddonRouting());
      if (UNLIKELY(!destAddOnRouting && paxTypeFare.destAddonRouting() == MILEAGE_ROUTING &&
          paxTypeFare.vendor() == Vendor::SITA))
      {
        destAddOnRouting = RoutingUtil::getStaticRoutingMileageHeaderData(
            trx, fm->governingCarrier(), paxTypeFare);
      }
    }
  }
}

bool
RoutingController::isSpecialRouting(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.routingNumber() == CAT25_DOMESTIC ||
         paxTypeFare.routingNumber() == CAT25_INTERNATIONAL ||
         paxTypeFare.routingNumber() == CAT25_EMPTY_ROUTING;
}

bool
RoutingController::needSpecialRouting(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.isValid() ||
         (paxTypeFare.retrievalInfo() != nullptr && paxTypeFare.retrievalInfo()->keep());
}

void
RoutingController::processSpecialRouting(fbrPaxTypeFareVector& fbrFares,
                                         RoutingInfos& rInfos,
                                         const TravelRoute& tvlRoute)
{
  if (fbrFares.empty())
    return;

  SpecialRouting validator;

  if (validator.validate(_trx, fbrFares, rInfos, tvlRoute))
  {
    LOG4CXX_DEBUG(logger, " Fare By Rule Roting Validation SuccessFull");
    return;
  }
  LOG4CXX_INFO(logger, " Fare By Rule Roting Validation Failed");
}

struct SurchargeApplied : public std::unary_function<RoutingInfos::value_type, bool>
{
  bool operator()(const RoutingInfos::value_type& d) const
  {
    return (d.second->mileageInfo() != nullptr && d.second->mileageInfo()->surchargeAmt() != 0);
  }
}; // lint !e1509

void
RoutingController::processSurchargeException(PricingTrx& trx,
                                             std::vector<PaxTypeFare*>& pFares,
                                             RoutingInfos& infos,
                                             const TravelRoute& tvlRoute)
{
  LOG4CXX_INFO(logger, " Entered RoutingController::ProcessSurcharegeException()");

  if (UNLIKELY(trx.getOptions()->isRtw()))
  {
    LOG4CXX_INFO(logger, " Leaving RoutingController::ProcessSurcharegeException() - RTW request");
    return;
  }

  RoutingInfos::iterator i(infos.end());
  i = find_if(infos.begin(), infos.end(), SurchargeApplied());
  bool surchargeApplied = (i != infos.end());

  if (surchargeApplied)
  {
    MileageInfo& mInfo = *(i->second->mileageInfo());
    MileageSurchargeException mse;
    {
      LOG4CXX_DEBUG(logger,
                    " Calling MileageSurcharageException from "
                    "RoutingController::ProcessSurcharegeException()");
      TSELatencyData metics(trx, "FVO ROUTING MILEAGE SURCHARGE");
      mse.validate(tvlRoute, pFares, trx, mInfo);
    }
  }
  LOG4CXX_INFO(logger, " Leaving RoutingController::ProcessSurcharegeException()");
}

TravelRoute
RoutingController::add(const TravelRoute& l,
                       const TravelRoute& r,
                       const Indicator& unticketedPointInd)
{
  std::vector<TravelSeg*> segs(l.mileageTravelRoute());
  segs.insert(segs.end(), r.mileageTravelRoute().begin(), r.mileageTravelRoute().end());
  TravelRouteBuilder trb;
  TravelRoute res;

  trb.buildTravelRoute(_trx, segs, res, unticketedPointInd);
  return res;
}

void
RoutingController::dummyFareMileage(PricingTrx& trx, PaxTypeFare& paxTypeFare, FareMarket& fm)
{
  const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(trx);
  dummyFareMilePercentage(excTrx, paxTypeFare, fm);
  dummyFareMileCity(excTrx, paxTypeFare, fm);
  return;
}

void
RoutingController::dummyFareMilePercentage(const ExchangePricingTrx& excTrx,
                                           PaxTypeFare& paxTypeFare,
                                           FareMarket& fm)
{

  // const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(trx);
  if (excTrx.exchangeOverrides().dummyFareMiles().empty())
    return;
  int16_t mileagePctg;
  std::map<const TravelSeg*, int16_t>::const_iterator fcMileageIter;
  std::vector<TravelSeg*>::const_iterator tvlSegIter = fm.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegLast = fm.travelSeg().end() - 1;
  for (; tvlSegIter < tvlSegLast; tvlSegIter++)
  {
    fcMileageIter = excTrx.exchangeOverrides().dummyFareMiles().find(*tvlSegIter);

    if (fcMileageIter == excTrx.exchangeOverrides().dummyFareMiles().end())
      continue;
    if (fcMileageIter->second >= 0)
      return; // error - mile pctg in the middle of FC
  }

  fcMileageIter = excTrx.exchangeOverrides().dummyFareMiles().find(*tvlSegLast);
  if (fcMileageIter == excTrx.exchangeOverrides().dummyFareMiles().end())
    return;
  if (fcMileageIter->second < 0)
    return;
  mileagePctg = fcMileageIter->second;
  MileageInfo* mileageInfo = nullptr;

  mileageInfo = paxTypeFare.mileageInfo();
  if (!mileageInfo)
  {
    excTrx.dataHandle().get(mileageInfo);
    paxTypeFare.mileageInfo() = mileageInfo;
  }
  mileageInfo->tpd() = false;
  mileageInfo->surchargeAmt() = mileagePctg;
  mileageInfo->valid() = true;
  mileageInfo->processed() = true;

  paxTypeFare.mileageSurchargePctg() = mileageInfo->surchargeAmt();
  paxTypeFare.setIsRouting(false); // indicate Mileage Fare Component
}

void
RoutingController::dummyFareMileCity(const ExchangePricingTrx& excTrx,
                                     PaxTypeFare& paxTypeFare,
                                     FareMarket& fm)
{

  bool mileageInfoAllocated = false;
  // const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(trx);
  // TPD applied

  if (excTrx.exchangeOverrides().dummyFareMileCity().empty() &&
      excTrx.exchangeOverrides().dummyFareMileTktCity().empty() &&
      excTrx.exchangeOverrides().mileageTypeData().empty())
    return;

  MileageInfo* mileageInfo = nullptr;
  mileageInfo = paxTypeFare.mileageInfo();
  if (!mileageInfo)
  {
    excTrx.dataHandle().get(mileageInfo);
    if (!mileageInfo)
      return;
    paxTypeFare.mileageInfo() = mileageInfo;
    mileageInfoAllocated = true; // no Mileage percentage information.
  }

  bool tpdSpecificCity = false;
  std::vector<TravelSeg*>& travelSegVec = fm.travelSeg();
  std::vector<TravelSeg*>::const_iterator tvlSegIter = travelSegVec.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEnd = travelSegVec.end();
  TravelSeg* tvlSegLast = travelSegVec.back();

  for (; tvlSegIter != tvlSegEnd; tvlSegIter++)
  {
    if ((excTrx.exchangeOverrides().dummyFareMileCity().find(*tvlSegIter) !=
         excTrx.exchangeOverrides().dummyFareMileCity().end()) &&
        ((*tvlSegIter) != tvlSegLast))
    {
      tpdSpecificCity = true;
      mileageInfo->tpd() = true;
      mileageInfo->tpdViaGeoLocs().insert(
          std::make_pair((*tvlSegIter)->offMultiCity(), CITY_MATCH)); // tpd applied
    }
    if (excTrx.exchangeOverrides().dummyFareMileTktCity().find(*tvlSegIter) !=
        excTrx.exchangeOverrides().dummyFareMileTktCity().end())
    {
      mileageInfo->southAtlanticTPMExclusion() = true;
      mileageInfo->southAtlanticTPDCities().push_back(
          std::make_pair((*tvlSegIter)->boardMultiCity(), (*tvlSegIter)->offMultiCity()));
    }

    std::vector<MileageTypeData*>::const_iterator mileTypeDataI =
        excTrx.exchangeOverrides().mileageTypeData().begin();
    for (; mileTypeDataI != excTrx.exchangeOverrides().mileageTypeData().end(); mileTypeDataI++)

      if ((*mileTypeDataI)->travelSeg() == *tvlSegIter)
      {

        if ((*mileTypeDataI)->type() == MileageTypeData::EQUALIZATION)
        {
          mileageInfo->mileageEqualizationApplies() = true;
          paxTypeFare.setIsRouting(false); // needed in FcDispFareUSage
          tpdSpecificCity = true; // avoid below logic to reset back to Mile Fare Component
        }
      }
  }
  if (mileageInfo->southAtlanticTPMExclusion())
  {
    // As here it was set by dummy fare override, we need push city pair
    // of last segment into the TPDCities to have correct FareCalc display
    mileageInfo->southAtlanticTPDCities().push_back(
        std::make_pair(tvlSegLast->boardMultiCity(), tvlSegLast->offMultiCity()));
  }
  if (!tpdSpecificCity)
  {
    if (excTrx.exchangeOverrides().dummyFareMileCity().find(tvlSegLast) ==
        excTrx.exchangeOverrides().dummyFareMileCity().end())
    {
      mileageInfo->tpd() = false; // wrong input Mileage City.
      if (mileageInfoAllocated)
        paxTypeFare.setIsRouting(true); // not Mileage Fare Component
      return;
    }
    mileageInfo->tpd() = true;
  }
}
}
