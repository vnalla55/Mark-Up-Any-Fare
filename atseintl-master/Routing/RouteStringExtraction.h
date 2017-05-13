//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Logger.h"
#include "Routing/MapTraversal.h"

namespace tse
{
class RouteStringExtraction : public MapTraversal
{
public:
  RouteStringExtraction(const std::vector<TravelRoute::CityCarrier>& aTravelRoute, int& city,
                        bool& carrier, const uint16_t rtgStrMaxResSize)
  : MapTraversal(false, aTravelRoute, city, carrier, rtgStrMaxResSize)
  {}

protected:
  static log4cxx::LoggerPtr _logger;
};

class RouteStringExtractionRTW: public RouteStringExtraction
{
public:
  RouteStringExtractionRTW(const std::vector<TravelRoute::CityCarrier>& aTravelRoute, int& city,
                           bool& carrier, const uint16_t rtgStrMaxResSize)
  : RouteStringExtraction(aTravelRoute, city, carrier, rtgStrMaxResSize)
  {}

protected:
  virtual bool hasPointOnMap(const TravelRoute::City& city,
                             const NationCode& nation,
                             const SpecifiedRouting* map) const override;

  virtual bool hasPointOnMap(const TravelRoute::City& city,
                             const NationCode& nation,
                             const SpecifiedRouting* map,
                             const SpecifiedRouting* nextMap,
                             const SpecifiedRouting* nextMap2) const override;

  virtual void
  getStartNodes(const SpecifiedRouting* map, std::set<const MapNode*, MapNodeCmp>& nodes) override;

  virtual bool checkNode(const MapNode& node,
                         const TravelRoute::City& offCity,
                         const NationCode& offNation) override;

  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

