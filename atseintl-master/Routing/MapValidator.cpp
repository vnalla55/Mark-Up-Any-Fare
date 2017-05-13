//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/MapValidator.h"

#include "Common/Logger.h"
#include "DataModel/TrxAborter.h"
#include "Routing/SpecifiedRouting.h"

namespace tse
{

static Logger
logger("atseintl.Routing.MapValidator");

MapValidator::MapValidator(const std::vector<TravelRoute::CityCarrier>& travelRoute, int& city,
                           bool& carrier)
  : MapValidation(travelRoute, city, carrier)
{
}

MapValidator::~MapValidator() {}

void
MapValidator::setupStartingNode(MapNode& currentNode, const MapNode& node,
                                const LocCode& originLoc)
{
  if(node.type() == MapNode::CITY)
  {
    MapTraversal::setupStartingNode(currentNode, node, originLoc);
    return;
  }

  currentNode.code() = node.code();
  currentNode.code().insert(INDUSTRY_CARRIER);
  currentNode.id() = node.id();
  currentNode.next() = node.id();
  currentNode.alt() = 0;
  currentNode.type() = node.type();
  currentNode.tag() = node.tag();

  _path.clear();
}

bool
MapValidator::hasPointOnMap(const TravelRoute::City& city,
                            const NationCode& nation,
                            const SpecifiedRouting* map) const
{
  return hasPointOnMapRTW(city, nation, map);
}

bool
MapValidator::hasPointOnMap(const TravelRoute::City& city,
                            const NationCode& nation,
                            const SpecifiedRouting* map,
                            const SpecifiedRouting* nextMap,
                            const SpecifiedRouting* nextMap2) const
{
  return hasPointOnMapRTW(city, nation, map, nextMap, nextMap2);
}

void
MapValidator::getStartNodes(const SpecifiedRouting* map,
                            std::set<const MapNode*, MapNodeCmp>& nodes)
{
  getStartNodesRTW(map, nodes);
}

bool
MapValidator::checkNode(const MapNode& node,
                        const TravelRoute::City& offCity,
                        const NationCode& offNation)
{
  return checkNodeRTW(node, offCity, offNation);
}

bool
MapValidator::checkCarrierExceptions(const CarrierCode& carrier, const CarrierCode& mapCarrier,
                                     char prevNodeType) const
{
  return MapValidation::checkCarrierExceptions(carrier, mapCarrier, prevNodeType)
      || prevNodeType == MapNode::NATION || prevNodeType == MapNode::ZONE;
}

MapTraversal::MatchReturn
MapValidator::getResult(bool isOverfly) const
{
  return isOverfly?MATCH_OK:MATCH_FAIL_CARRIER;
}

bool
MapValidator::checkCarrier(const MapNode* node, const CarrierCode& carrier,
                           const GenericAllianceCode& genericAllianceCode, bool isOverfly)
{
  return isOverfly || MapValidation::checkCarrier(node, carrier, genericAllianceCode)
      || node->contains(genericAllianceCode);
}

bool
MapValidator::isOverflyPossible(const MapNode* prevNode, const CarrierCode& carrier,
                                const GenericAllianceCode& genericAllianceCode,
                                const CarrierCode& mapCarrier, bool isOverfly)
{
  return _zoneNationPass || isOverfly
      || MapValidation::isOverflyPossible(prevNode, carrier, genericAllianceCode, mapCarrier);
}

} // namespace tse
