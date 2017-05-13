//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/SpecifiedRouting.h"
#include "Routing/TravelRoute.h"

#include <tuple>
#include <vector>

namespace tse
{

class PricingTrx;
class Routing;
class MapInfo;
class RouteStringExtraction;

class RoutingGetter
{
public:
  virtual const SpecifiedRouting* getMap() = 0;
};

class SpecifiedRoutingValidator
{
public:
  bool validateOld(PricingTrx& trx,
                   const TravelRoute& tvlRoute,
                   const Routing*,
                   MapInfo* mapInfo,
                   const DateTime& travelDate,
                   const Routing* origAddOnRouting = nullptr,
                   const Routing* destAddOnRouting = nullptr);

  bool validate(PricingTrx& trx,
                const TravelRoute& tvlRoute,
                const Routing*,
                MapInfo* mapInfo,
                const DateTime& travelDate,
                const Routing* origAddOnRouting = nullptr,
                const Routing* destAddOnRouting = nullptr);

  void extractReversedRouteStrings(PricingTrx& trx,
                                   RouteStringExtraction& search,
                                   std::vector<std::string>* routeStrings,
                                   SpecifiedRouting& map,
                                   const SpecifiedRouting* nextMap,
                                   const SpecifiedRouting* nextMap2) const;
    void
    extractRouteStrings(PricingTrx& trx, SpecifiedRouting& map, const CarrierCode& carrier,
                        const TravelRoute& tvlRoute, std::vector<std::string>* routeStrings,
                        const SpecifiedRouting* nextMap, const SpecifiedRouting* nextMap2,
                        uint16_t rtgStrMaxResSize, const Indicator& directionalInd,
                        bool isFareReversed) const;

protected:
  void displayRoutingDiag(PricingTrx& trx,
                          const DateTime& travelDate,
                          const TravelRoute& tvlRoute,
                          const Routing* routing,
                          const Routing* origAddOnRouting,
                          const Routing* destAddOnRouting,
                          MapInfo* mapInfo);
    void
    extractRouteStrings(PricingTrx& trx, RouteStringExtraction& search, SpecifiedRouting& map,
                        const CarrierCode& carrier,
                        std::vector<std::string>* routeStrings, const SpecifiedRouting* nextMap,
                        const SpecifiedRouting* nextMap2, uint16_t rtgStrMaxResSize,
                        const Indicator& directionalInd, bool isFareReversed) const;

  private:
    bool validateMapOld(PricingTrx& trx,
                        int& missingCity,
                        bool& missingCarrier,
                        const std::vector<TravelRoute::CityCarrier>& route,
                        const SpecifiedRouting& map,
                        const SpecifiedRouting* nextMap = nullptr,
                        const SpecifiedRouting* nextMap2 = nullptr) const;

    typedef std::tuple<RoutingGetter*, RoutingGetter*, RoutingGetter*> MapsGetter;

    bool validateMap(PricingTrx& trx,
                     int& missingCity,
                     bool& missingCarrier,
                     const std::vector<TravelRoute::CityCarrier>& route,
                     MapsGetter) const;

  void reverseRoute(const std::vector<TravelRoute::CityCarrier>& route,
                    std::vector<TravelRoute::CityCarrier>& reverse) const;
};

} // namespace tse

