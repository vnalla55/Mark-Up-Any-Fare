//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "Routing/SpecifiedRoutingValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/RoutingMap.h"
#include "Routing/MapValidation.h"
#include "Routing/MapValidator.h"
#include "Routing/RouteStringExtraction.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/SpecifiedRouting.h"
#include "Routing/SpecifiedRoutingCache.h"
#include "Routing/TravelRoute.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace tse
{
FALLBACK_DECL(fallbackFMRbyte58);
FALLBACK_DECL(fallbackRoutingValidationStartOnSecondMap);

namespace
{
class EmptyRoutingGetter : public RoutingGetter
{
public:
  virtual const SpecifiedRouting* getMap() override { return nullptr; };
};

class SpecifiedRoutingGetter : public RoutingGetter
{
public:
  SpecifiedRoutingGetter(PricingTrx& trx, const SpecifiedRoutingKey& key)
    : _trx(&trx), _key(&key), _map(&SpecifiedRoutingCache::getSpecifiedRouting(trx, key))
  {
  }

  virtual const SpecifiedRouting* getMap() override { return _map; }

  PricingTrx* getTrx() const { return _trx; }
  const SpecifiedRoutingKey* getKey() const { return _key; }
  bool isAddon() const { return false; }

private:
  PricingTrx* _trx;
  const SpecifiedRoutingKey* _key;
  SpecifiedRouting* _map;
};

class AddonSpecifiedRoutingGetter : public RoutingGetter
{
public:
  AddonSpecifiedRoutingGetter(PricingTrx& trx,
                              const SpecifiedRoutingKey& key,
                              const Routing* routing,
                              const Routing* addOnRouting)
    : _trx(&trx), _key(&key), _routing(routing), _addOnRouting(addOnRouting)
  {
  }

  virtual const SpecifiedRouting* getMap() override
  {
    if (!_initialized)
    {
      _initialized = true;
      if (_addOnRouting != nullptr && _routing != _addOnRouting)
      {
        _trx->dataHandle().get(_map);
        _map->initialize(*_addOnRouting, *_trx, _routing);
      }
    }

    return _map;
  }

  PricingTrx* getTrx() const { return _trx; }
  const SpecifiedRoutingKey* getKey() const { return _key; }
  bool isAddon() const { return true; }

private:
  PricingTrx* _trx = nullptr;
  const SpecifiedRoutingKey* _key = nullptr;
  SpecifiedRouting* _map = nullptr;
  const Routing* _routing = nullptr;
  const Routing* _addOnRouting = nullptr;
  bool _initialized = false;
};

template <class BaseGetter>
class ReversedSpecifiedRoutingGetter : public RoutingGetter
{
public:
  ReversedSpecifiedRoutingGetter(BaseGetter& baseMap) : _baseMap(baseMap) {}

  virtual const SpecifiedRouting* getMap() override
  {
    if (!_initialized)
    {
      _initialized = true;
      if (_baseMap.isAddon())
      {
        const SpecifiedRouting* map = _baseMap.getMap();
        if (map)
        {
          _addOnReversedMap = *map;
          if (LIKELY(_addOnReversedMap.reverseMap(*_baseMap.getTrx())))
            _reversedMap = &_addOnReversedMap;
        }
      }
      else
        _reversedMap = SpecifiedRoutingCache::getSpecifiedRoutingReverse(*_baseMap.getTrx(),
                                                                         *_baseMap.getKey());
    }

    return _reversedMap;
  }

private:
  BaseGetter& _baseMap;
  const SpecifiedRouting* _reversedMap = nullptr;
  SpecifiedRouting _addOnReversedMap;
  bool _initialized = false;
};
}

static Logger
logger("atseintl.Routing.SpecifiedRoutingValidator");

/**
 * check if the travel route is a valid traversal of the map.
 * @param trx Transaction
 * @param tvlRoute Travel route
 * @param routing Routing (includes header, map and restrictions)
 * @param mapInfo container for routing strings returned
 */
bool
SpecifiedRoutingValidator::validateOld(PricingTrx& trx,
                                       const TravelRoute& tvlRoute,
                                       const Routing* routing,
                                       MapInfo* mapInfo,
                                       const DateTime& travelDate,
                                       const Routing* origAddOnRouting,
                                       const Routing* destAddOnRouting)
{
  LOG4CXX_INFO(logger, " Entered SpecifiedRoutingValidator()::validate()");

  bool ret = false;

  SpecifiedRoutingKey key(*routing, travelDate);
  SpecifiedRouting& map = SpecifiedRoutingCache::getSpecifiedRouting(trx, key);

  SpecifiedRouting* origAddOnMap = nullptr;
  if (origAddOnRouting != nullptr && routing != origAddOnRouting)
  {
    trx.dataHandle().get(origAddOnMap);
    origAddOnMap->initialize(*origAddOnRouting, trx, routing);
  }

  SpecifiedRouting* destAddOnMap = nullptr;
  if (destAddOnRouting != nullptr && routing != destAddOnRouting)
  {
    trx.dataHandle().get(destAddOnMap);
    destAddOnMap->initialize(*destAddOnRouting, trx, routing);
  }

  int missingCityF = -1;
  bool missingCarrierF = false;

  if (TrxUtil::isFullMapRoutingActivated(trx) &&
      (routing->directionalInd() == MAPDIR_L2R || !fallback::fallbackFMRbyte58(&trx)))
  {
    boost::function<bool(const SpecifiedRouting&, const SpecifiedRouting*, const SpecifiedRouting*)>
    validator;
    validator = (boost::bind(&SpecifiedRoutingValidator::validateMapOld,
                             *this,
                             boost::ref(trx),
                             boost::ref(missingCityF),
                             boost::ref(missingCarrierF),
                             boost::ref(tvlRoute.travelRoute()),
                             _1,
                             _2,
                             _3));

    if (origAddOnMap != nullptr)
      ret = validator(*origAddOnMap, &map, destAddOnMap);
    else
      ret = validator(map, destAddOnMap, nullptr);

    if (!ret && (routing->directionalInd() != MAPDIR_L2R))
    {
      if (origAddOnMap)
      {
        SpecifiedRouting revOrigMap;
        revOrigMap = *origAddOnMap;

        SpecifiedRouting* revMap = SpecifiedRoutingCache::getSpecifiedRoutingReverse(trx, key);
        if (revMap != nullptr && revOrigMap.reverseMap(trx))
        {
          if (destAddOnMap)
          {
            SpecifiedRouting revDestMap = *destAddOnMap;
            if (revDestMap.reverseMap(trx))
              ret = validator(revOrigMap, revMap, &revDestMap);
          }
          else
          {
            ret = validator(revOrigMap, revMap, nullptr);
          }
        }
      }
      else
      {
        SpecifiedRouting* revMap = SpecifiedRoutingCache::getSpecifiedRoutingReverse(trx, key);
        if (revMap != nullptr)
        {
          if (destAddOnMap)
          {
            SpecifiedRouting revDestMap = *destAddOnMap;
            if (revDestMap.reverseMap(trx))
              ret = validator(*revMap, &revDestMap, nullptr);
          }
          else
            ret = validator(*revMap, nullptr, nullptr);
        }
      }
    }
  }
  else
  {
    SpecifiedRouting revOrigMap;
    SpecifiedRouting* revMap = nullptr;
    SpecifiedRouting revDestMap;

    if (origAddOnMap != nullptr)
    {
      bool reversed = true;
      SpecifiedRouting* revOrigAddOnMap = nullptr;

      ret = validateMapOld(trx,
                           missingCityF,
                           missingCarrierF,
                           tvlRoute.travelRoute(),
                           *origAddOnMap,
                           &map,
                           destAddOnMap);

      if (!ret && reversed)
      {
        revOrigMap = *origAddOnMap;
        revMap = SpecifiedRoutingCache::getSpecifiedRoutingReverse(trx, key);
        if (revMap == nullptr)
        {
          reversed = false;
        }
        else
        {
          ret = validateMapOld(trx,
                               missingCityF,
                               missingCarrierF,
                               tvlRoute.travelRoute(),
                               *origAddOnMap,
                               revMap,
                               destAddOnMap);
        }
      }
      if (!ret && reversed)
      {
        revOrigAddOnMap = &revOrigMap;
        reversed &= revOrigAddOnMap->reverseMap(trx);

        if (reversed)
          ret = validateMapOld(trx,
                               missingCityF,
                               missingCarrierF,
                               tvlRoute.travelRoute(),
                               *revOrigAddOnMap,
                               &map,
                               destAddOnMap);
      }
      if (!ret && reversed)
      {
        ret = validateMapOld(trx,
                             missingCityF,
                             missingCarrierF,
                             tvlRoute.travelRoute(),
                             *revOrigAddOnMap,
                             revMap,
                             destAddOnMap);
      }
      if (destAddOnMap != nullptr)
      {
        SpecifiedRouting* revDestAddOnMap = nullptr;

        if (!ret && reversed)
        {
          revDestMap = *destAddOnMap;
          revDestAddOnMap = &revDestMap;
          reversed &= revDestAddOnMap->reverseMap(trx);

          if (reversed)
            ret = validateMapOld(trx,
                                 missingCityF,
                                 missingCarrierF,
                                 tvlRoute.travelRoute(),
                                 *origAddOnMap,
                                 &map,
                                 revDestAddOnMap);
        }
        if (!ret && reversed)
        {
          ret = validateMapOld(trx,
                               missingCityF,
                               missingCarrierF,
                               tvlRoute.travelRoute(),
                               *revOrigAddOnMap,
                               &map,
                               revDestAddOnMap);
        }
        if (!ret && reversed)
        {
          ret = validateMapOld(trx,
                               missingCityF,
                               missingCarrierF,
                               tvlRoute.travelRoute(),
                               *origAddOnMap,
                               revMap,
                               revDestAddOnMap);
        }
        if (!ret && reversed)
        {
          ret = validateMapOld(trx,
                               missingCityF,
                               missingCarrierF,
                               tvlRoute.travelRoute(),
                               *revOrigAddOnMap,
                               revMap,
                               revDestAddOnMap);
        }
      }
    }
    else
    {
      bool reversed = true;

      ret = validateMapOld(
          trx, missingCityF, missingCarrierF, tvlRoute.travelRoute(), map, destAddOnMap);
      if (!ret)
      {
        revMap = SpecifiedRoutingCache::getSpecifiedRoutingReverse(trx, key);
        if (revMap == nullptr)
        {
          reversed = false;
        }
        else
        {
          ret = validateMapOld(
              trx, missingCityF, missingCarrierF, tvlRoute.travelRoute(), *revMap, destAddOnMap);
        }
      }
      if (destAddOnMap != nullptr)
      {
        SpecifiedRouting* revDestAddOnMap = nullptr;

        if (!ret && reversed)
        {
          revDestMap = *destAddOnMap;
          revDestAddOnMap = &revDestMap;
          reversed &= revDestAddOnMap->reverseMap(trx);

          if (reversed)
            ret = validateMapOld(
                trx, missingCityF, missingCarrierF, tvlRoute.travelRoute(), map, revDestAddOnMap);
        }
        if (!ret && reversed)
        {
          ret = validateMapOld(
              trx, missingCityF, missingCarrierF, tvlRoute.travelRoute(), *revMap, revDestAddOnMap);
        }
      }
    }
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic450
      || trx.diagnostic().diagnosticType() == Diagnostic455)
  {
    displayRoutingDiag(
        trx, travelDate, tvlRoute, routing, origAddOnRouting, destAddOnRouting, mapInfo);
  }

  if (ret)
  {
    mapInfo->missingCityIndex() = -1;
    mapInfo->missingCarrier() = false;
  }
  else
  {
    mapInfo->missingCityIndex() = missingCityF;
    mapInfo->missingCarrier() = missingCarrierF;

    if (mapInfo->missingCityIndex() < 0) // origin
      mapInfo->missingCityIndex() = 0;
    else if (mapInfo->missingCityIndex() >=
             static_cast<int>(tvlRoute.travelRoute().size())) // destination
      mapInfo->missingCityIndex() = static_cast<int>(tvlRoute.travelRoute().size() - 1);
  }

  LOG4CXX_INFO(logger, " Exited SpecifiedRoutingValidator()::validate()");
  return ret;
}

bool
SpecifiedRoutingValidator::validate(PricingTrx& trx,
                                    const TravelRoute& tvlRoute,
                                    const Routing* routing,
                                    MapInfo* mapInfo,
                                    const DateTime& travelDate,
                                    const Routing* origAddOnRouting,
                                    const Routing* destAddOnRouting)
{
  if (fallback::fallbackRoutingValidationStartOnSecondMap(&trx))
    return validateOld(
        trx, tvlRoute, routing, mapInfo, travelDate, origAddOnRouting, destAddOnRouting);

  LOG4CXX_INFO(logger, " Entered SpecifiedRoutingValidator()::validate()");

  SpecifiedRoutingKey key(*routing, travelDate);

  SpecifiedRoutingGetter map(trx, key);
  AddonSpecifiedRoutingGetter origMap(trx, key, routing, origAddOnRouting);
  AddonSpecifiedRoutingGetter destMap(trx, key, routing, destAddOnRouting);
  ReversedSpecifiedRoutingGetter<SpecifiedRoutingGetter> reversedMap(map);
  ReversedSpecifiedRoutingGetter<AddonSpecifiedRoutingGetter> reversedOrigMap(origMap);
  ReversedSpecifiedRoutingGetter<AddonSpecifiedRoutingGetter> reversedDestMap(destMap);
  EmptyRoutingGetter emptyMap;

  bool checkMixedDir = false;
  bool checkAllReversed = false;
  if (TrxUtil::isFullMapRoutingActivated(trx) &&
      (routing->directionalInd() == MAPDIR_L2R || !fallback::fallbackFMRbyte58(&trx)))
  {
    if (routing->directionalInd() != MAPDIR_L2R)
      checkAllReversed = true;
  }
  else
    checkMixedDir = true;

  std::vector<MapsGetter> mapVector;

  mapVector.push_back(std::make_tuple(&map, &destMap, &emptyMap));
  if (checkMixedDir || checkAllReversed)
    mapVector.push_back(std::make_tuple(&reversedMap, &reversedDestMap, &emptyMap));

  mapVector.push_back(std::make_tuple(&origMap, &map, &destMap));
  mapVector.push_back(std::make_tuple(&destMap, &emptyMap, &emptyMap));

  if (checkMixedDir || checkAllReversed)
  {
    mapVector.push_back(std::make_tuple(&reversedOrigMap, &reversedMap, &reversedDestMap));
    mapVector.push_back(std::make_tuple(&reversedDestMap, &emptyMap, &emptyMap));
  }
  if (checkMixedDir)
  {
    mapVector.push_back(std::make_tuple(&map, &reversedDestMap, &emptyMap));
    mapVector.push_back(std::make_tuple(&reversedMap, &destMap, &emptyMap));

    mapVector.push_back(std::make_tuple(&origMap, &reversedMap, &reversedDestMap));
    mapVector.push_back(std::make_tuple(&reversedOrigMap, &map, &reversedDestMap));
    mapVector.push_back(std::make_tuple(&reversedOrigMap, &reversedMap, &destMap));

    mapVector.push_back(std::make_tuple(&reversedOrigMap, &map, &destMap));
    mapVector.push_back(std::make_tuple(&origMap, &reversedMap, &destMap));
    mapVector.push_back(std::make_tuple(&origMap, &map, &reversedDestMap));
  }

  int missingCityF = -1;
  bool missingCarrierF = false;
  bool ret = false;
  for (const MapsGetter& mapGetter : mapVector)
  {
    ret = validateMap(trx, missingCityF, missingCarrierF, tvlRoute.travelRoute(), mapGetter);
    if (ret)
      break;
  }

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic450 ||
      trx.diagnostic().diagnosticType() == Diagnostic455))
  {
    displayRoutingDiag(
        trx, travelDate, tvlRoute, routing, origAddOnRouting, destAddOnRouting, mapInfo);
  }

  if (ret)
  {
    mapInfo->missingCityIndex() = -1;
    mapInfo->missingCarrier() = false;
  }
  else
  {
    mapInfo->missingCityIndex() = missingCityF;
    mapInfo->missingCarrier() = missingCarrierF;

    if (mapInfo->missingCityIndex() < 0) // origin
      mapInfo->missingCityIndex() = 0;
    else if (UNLIKELY(mapInfo->missingCityIndex() >=
             static_cast<int>(tvlRoute.travelRoute().size()))) // destination
      mapInfo->missingCityIndex() = static_cast<int>(tvlRoute.travelRoute().size() - 1);
  }

  LOG4CXX_INFO(logger, " Exited SpecifiedRoutingValidator()::validate()");
  return ret;
}

void
SpecifiedRoutingValidator::displayRoutingDiag(PricingTrx& trx,
                                              const DateTime& travelDate,
                                              const TravelRoute& tvlRoute,
                                              const Routing* routing,
                                              const Routing* origAddOnRouting,
                                              const Routing* destAddOnRouting,
                                              MapInfo* mapInfo)
{
  SpecifiedRoutingKey key(*routing, travelDate, true);
  SpecifiedRouting* map = &SpecifiedRoutingCache::getSpecifiedRouting(trx, key);

  SpecifiedRouting* origAddOnMap = nullptr;
  SpecifiedRouting* destAddOnMap = nullptr;

  if (origAddOnRouting != nullptr && routing != origAddOnRouting)
  {
    SpecifiedRoutingDiag* origAddOnMapDiag = nullptr;
    trx.dataHandle().get(origAddOnMapDiag);
    origAddOnMap = origAddOnMapDiag;
    origAddOnMap->initialize(*origAddOnRouting, trx, routing);
  }

  if (destAddOnRouting != nullptr && routing != destAddOnRouting)
  {
    SpecifiedRoutingDiag* destAddOnMapDiag = nullptr;
    trx.dataHandle().get(destAddOnMapDiag);
    destAddOnMap = destAddOnMapDiag;
    destAddOnMap->initialize(*destAddOnRouting, trx, routing);
  }

  std::vector<std::string>* routeStrings = nullptr;

  // lint --e{413}
  trx.dataHandle().get(routeStrings);
  mapInfo->routeStrings() = routeStrings;

  Indicator directionalInd = MAPDIR_IGNOREIND;
  if (TrxUtil::isFullMapRoutingActivated(trx))
  {
    if (!trx.diagnostic().isDiag455ParamSet())
      directionalInd = routing->directionalInd();
    else
    {
      map->setTerminal(false);
      if (origAddOnMap)
        origAddOnMap->setTerminal(false);
      if (destAddOnMap)
        destAddOnMap->setTerminal(false);
    }
  }

  bool isFareReversed = false;

  if (fallback::fallbackRoutingValidationStartOnSecondMap(&trx))
  {
    if (origAddOnMap != nullptr)
      extractRouteStrings(trx,
                          *origAddOnMap,
                          routing->carrier(),
                          tvlRoute,
                          routeStrings,
                          map,
                          destAddOnMap,
                          USHRT_MAX,
                          directionalInd,
                          isFareReversed);
    else
      extractRouteStrings(trx,
                          *map,
                          routing->carrier(),
                          tvlRoute,
                          routeStrings,
                          destAddOnMap,
                          nullptr,
                          USHRT_MAX,
                          directionalInd,
                          isFareReversed);
  }
  else
  {
    if (origAddOnMap != nullptr)
      extractRouteStrings(trx,
                          *origAddOnMap,
                          routing->carrier(),
                          tvlRoute,
                          routeStrings,
                          map,
                          destAddOnMap,
                          USHRT_MAX,
                          directionalInd,
                          isFareReversed);

    extractRouteStrings(trx,
                        *map,
                        routing->carrier(),
                        tvlRoute,
                        routeStrings,
                        destAddOnMap,
                        nullptr,
                        USHRT_MAX,
                        directionalInd,
                        isFareReversed);

    if (destAddOnMap != nullptr)
      extractRouteStrings(trx,
                          *destAddOnMap,
                          routing->carrier(),
                          tvlRoute,
                          routeStrings,
                          nullptr,
                          nullptr,
                          USHRT_MAX,
                          directionalInd,
                          isFareReversed);
  }
}

/**
 * perform validation of route against routing maps
 * @returns index of last match
 */
bool
SpecifiedRoutingValidator::validateMapOld(PricingTrx& trx,
                                          int& missingCity,
                                          bool& missingCarrier,
                                          const std::vector<TravelRoute::CityCarrier>& travelRoute,
                                          const SpecifiedRouting& map,
                                          const SpecifiedRouting* nextMap,
                                          const SpecifiedRouting* nextMap2) const
{
  if (trx.getOptions()->isRtw())
  {
    MapValidator search(travelRoute, missingCity, missingCarrier);

    return search.execute(trx, nullptr, &map, nextMap, nextMap2);
  }
  else
  {
    MapValidation search(travelRoute, missingCity, missingCarrier);

    return search.execute(trx, nullptr, &map, nextMap, nextMap2);
  }
}

bool
SpecifiedRoutingValidator::validateMap(PricingTrx& trx,
                                       int& missingCity,
                                       bool& missingCarrier,
                                       const std::vector<TravelRoute::CityCarrier>& travelRoute,
                                       MapsGetter mapGetter) const
{
  if (!std::get<0>(mapGetter)->getMap())
    return false;

  static constexpr size_t MAP_0 = 0;
  static constexpr size_t MAP_1 = 1;
  static constexpr size_t MAP_2 = 2;

  if (UNLIKELY(trx.getOptions()->isRtw()))
  {
    MapValidator search(travelRoute, missingCity, missingCarrier);

    return search.execute(trx,
                          nullptr,
                          std::get<MAP_0>(mapGetter)->getMap(),
                          std::get<MAP_1>(mapGetter)->getMap(),
                          std::get<MAP_2>(mapGetter)->getMap());
  }
  else
  {
    MapValidation search(travelRoute, missingCity, missingCarrier);

    return search.execute(trx,
                          nullptr,
                          std::get<MAP_0>(mapGetter)->getMap(),
                          std::get<MAP_1>(mapGetter)->getMap(),
                          std::get<MAP_2>(mapGetter)->getMap());
  }
}

void
SpecifiedRoutingValidator::extractReversedRouteStrings(PricingTrx& trx,
                                                       RouteStringExtraction& search,
                                                       std::vector<std::string>* routeStrings,
                                                       SpecifiedRouting& map,
                                                       const SpecifiedRouting* nextMap,
                                                       const SpecifiedRouting* nextMap2) const
{
  SpecifiedRouting revMap, revNextMap, revNextMap2;

  revMap = map;
  revMap.reverseMap(trx);

  if (revMap.reverseMap(trx))
  {
    if (nextMap)
    {
      revNextMap = *nextMap;
      if (revNextMap.reverseMap(trx))
      {
        if (nextMap2)
        {
          revNextMap2 = *nextMap2;
          if (revNextMap2.reverseMap(trx))
            search.execute(trx, routeStrings, &revMap, &revNextMap, &revNextMap2);
        }
        else
          search.execute(trx, routeStrings, &revMap, &revNextMap, nullptr);
      }
    }
    else
      search.execute(trx, routeStrings, &revMap, nullptr, nullptr);
  }
}

void
SpecifiedRoutingValidator::extractRouteStrings(PricingTrx& trx,
                                               SpecifiedRouting& map,
                                               const CarrierCode& carrier,
                                               const TravelRoute& tvlRoute,
                                               std::vector<std::string>* routeStrings,
                                               const SpecifiedRouting * nextMap,
                                               const SpecifiedRouting* nextMap2,
                                               const uint16_t rtgStrMaxResSize,
                                               const Indicator& directionalInd,
                                               bool isFareReversed) const
{
  int missingCity;
  bool missingCarrier;
  std::vector<TravelRoute::CityCarrier> travelRoute(0);

  TravelRoute::CityCarrier cc;
  cc.carrier() = carrier;
  cc.boardNation() =  tvlRoute.travelRoute().front().boardNation();
  cc.offNation() =  tvlRoute.travelRoute().back().offNation();
  cc.boardCity() = tvlRoute.travelRoute().front().boardCity();
  cc.offCity() = tvlRoute.travelRoute().back().offCity();
  travelRoute.push_back(cc);

  if(travelRoute.front().boardCity().loc() == travelRoute.back().offCity().loc())
  {
    RouteStringExtractionRTW search(travelRoute, missingCity, missingCarrier, rtgStrMaxResSize);

    extractRouteStrings(trx, search, map, carrier, routeStrings, nextMap, nextMap2,
        rtgStrMaxResSize, directionalInd, isFareReversed);
  } else
  {
    RouteStringExtraction search(travelRoute, missingCity, missingCarrier, rtgStrMaxResSize);

    extractRouteStrings(trx, search, map, carrier, routeStrings, nextMap, nextMap2,
        rtgStrMaxResSize, directionalInd, isFareReversed);
  }
}

void
SpecifiedRoutingValidator::extractRouteStrings(PricingTrx& trx, RouteStringExtraction& search,
                                               SpecifiedRouting& map, const CarrierCode& carrier,
                                               std::vector<std::string>* routeStrings,
                                               const SpecifiedRouting * nextMap,
                                               const SpecifiedRouting* nextMap2,
                                               const uint16_t rtgStrMaxResSize,
                                               const Indicator& directionalInd,
                                               bool isFareReversed) const
{
  bool reversed = true;

  if(!TrxUtil::isFullMapRoutingActivated(static_cast<PricingTrx&>(trx))
      || (directionalInd != MAPDIR_L2R && fallback::fallbackFMRbyte58(&trx)))
  {
    search.execute(trx, routeStrings, &map, nextMap, nextMap2);

    SpecifiedRouting revMap, revNextMap, revNextMap2;

    revMap = map;
    reversed = revMap.reverseMap(trx);

    if (reversed)
      search.execute(trx, routeStrings, &revMap, nextMap, nextMap2);

    if (nextMap)
    {
      revNextMap = *nextMap;
      reversed = revNextMap.reverseMap(trx);
      if (reversed)
      {
        search.execute(trx, routeStrings, &map, &revNextMap, nextMap2);
        search.execute(trx, routeStrings, &revMap, &revNextMap, nextMap2);
      }
    }

    if (nextMap2)
    {
      revNextMap2 = *nextMap2;
      reversed = revNextMap2.reverseMap(trx);
      if (reversed)
      {
        search.execute(trx, routeStrings, &map, nextMap, &revNextMap2);
        search.execute(trx, routeStrings, &revMap, nextMap, &revNextMap2);
        search.execute(trx, routeStrings, &map, &revNextMap, &revNextMap2);
        search.execute(trx, routeStrings, &revMap, &revNextMap, &revNextMap2);
      }
    }
  }
  else
  {
    if(directionalInd == MAPDIR_L2R)
    {
      if (isFareReversed)
        extractReversedRouteStrings(trx, search, routeStrings, map, nextMap, nextMap2);
      else
        search.execute(trx, routeStrings, &map, nextMap, nextMap2);
    }
    else
    {
      search.execute(trx, routeStrings, &map, nextMap, nextMap2);
      extractReversedRouteStrings(trx, search, routeStrings, map, nextMap, nextMap2);
    }
  }
}

void
SpecifiedRoutingValidator::reverseRoute(const std::vector<TravelRoute::CityCarrier>& route,
                                        std::vector<TravelRoute::CityCarrier>& reverse) const
{
  reverse.clear();
  reverse.reserve(route.size());
  std::vector<TravelRoute::CityCarrier>::const_reverse_iterator i;
  for (i = route.rbegin(); i != route.rend(); i++)
  {
    TravelRoute::CityCarrier cc;
    cc.boardCity() = i->offCity();
    cc.offCity() = i->boardCity();
    cc.carrier() = i->carrier();
    cc.stopover() = i->stopover();
    cc.genericAllianceCode() = i->genericAllianceCode();
    reverse.push_back(cc);
  }
}
}
