//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/RouteStringExtraction.h"

#include "Routing/SpecifiedRouting.h"

namespace tse
{

log4cxx::LoggerPtr RouteStringExtraction::_logger(log4cxx::Logger::getLogger("atseintl.Routing.RouteStringExtraction"));
log4cxx::LoggerPtr RouteStringExtractionRTW::_logger(log4cxx::Logger::getLogger("atseintl.Routing.RouteStringExtractionRTW"));

bool
RouteStringExtractionRTW::hasPointOnMap(const TravelRoute::City& city,
                                        const NationCode& nation,
                                        const SpecifiedRouting* map) const
{
  return hasPointOnMapRTW(city, nation, map);
}

bool
RouteStringExtractionRTW::hasPointOnMap(const TravelRoute::City& city,
                                        const NationCode& nation,
                                        const SpecifiedRouting* map,
                                        const SpecifiedRouting* nextMap,
                                        const SpecifiedRouting* nextMap2) const
{
  return hasPointOnMapRTW(city, nation, map, nextMap, nextMap2);
}

void
RouteStringExtractionRTW::getStartNodes(const SpecifiedRouting* map,
                                        std::set<const MapNode*,MapNodeCmp>& nodes)
{
  getStartNodesRTW(map, nodes);
}

bool
RouteStringExtractionRTW::checkNode(const MapNode& node, const TravelRoute::City& offCity,
                                    const NationCode& offNation)
{
  return checkNodeRTW(node, offCity, offNation);
}

} // namespace tse
