//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/MapValidation.h"

#include "Common/Logger.h"
#include "Routing/SpecifiedRouting.h"

namespace tse
{

static Logger
logger("atseintl.Routing.MapValidation");

/**
 * Compare the current node against the point in the itinerary and evaluate whether
 * to continue traversing the next node. The previous node is also examined if it's
 * an AIRLINE node.
 */

bool
MapValidation::checkCarrierExceptions(const CarrierCode& carrier, const CarrierCode& mapCarrier,
                                      char prevNodeType) const
{
  return carrier == mapCarrier || carrier == SURFACE_CARRIER;
}

MapTraversal::MatchReturn
MapValidation::getResult(bool /*isOverfly*/) const
{
  return MATCH_FAIL_CARRIER;
}

MapTraversal::MatchReturn
MapValidation::matchNode(const SpecifiedRouting* map,
                         const MapNode* node,
                         const MapNode* prevNode,
                         const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                         std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
                         bool isOverfly)
{
  if (isRepeat(city, node))
  {
    return MapTraversal::MATCH_FAIL_CITY;
  }

  nextCity = city;

  if (node->type() == MapNode::AIRLINE)
  {
    if(checkCarrier(node, city->carrier(), city->genericAllianceCode(), isOverfly))
    {
      updateVisited( city, node );
      return MATCH_OK;
    }
    return MATCH_FAIL_CARRIER;
  }
  if (checkNode(*node, city->offCity(), city->offNation()))
  {
    if (prevNode != nullptr && prevNode->type() == MapNode::AIRLINE)
    {
      if(checkCarrier(prevNode, city->carrier(), city->genericAllianceCode(), !isOverfly))
      {
        nextCity++;
        updateVisited(city, node);
        return MATCH_OK;
      }
      return getResult(isOverfly);
    }
    if(checkCarrierExceptions(city->carrier(), map->carrier(), prevNode->type()))
    {
      nextCity++;
      updateVisited( city, node );
      return MATCH_OK;
    }
    return getResult(isOverfly);
  }
  if(isOverflyPossible(prevNode, city->carrier(), city->genericAllianceCode(), map->carrier(),
      isOverfly))
  {
    updateVisited(city, node);
    return MapTraversal::MATCH_OK;
  }

  return MapTraversal::MATCH_FAIL_CARRIER;
}

bool
MapValidation::checkCarrier(const MapNode* node, const CarrierCode& carrier,
                            const GenericAllianceCode& /*genericAllianceCode*/, bool /*isOverfly*/)
{
  return node->contains(carrier) || node->contains(INDUSTRY_CARRIER)
      || carrier == SURFACE_CARRIER;
}

bool
MapValidation::isOverflyPossible(const MapNode* prevNode, const CarrierCode& carrier,
                                 const GenericAllianceCode& genericAllianceCode,
                                 const CarrierCode& mapCarrier, bool /*isOverfly*/)
{
  return (prevNode == nullptr || (prevNode->type() != MapNode::AIRLINE && carrier == mapCarrier)
      || checkCarrier(prevNode, carrier, genericAllianceCode));
}

/**
 * Check if we've already been here
 */

bool
MapValidation::isRepeat(const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                        const MapNode* node)
{
  size_t depth = city - _travelRoute->begin();
  std::set<const MapNode*>& visited = _visited[depth];
  if (visited.find(node) == visited.end())
  {
    //      visited.insert(node);

    return false;
  }

  LOG4CXX_DEBUG(logger,
                city->boardCity().loc() << "-" << city->offCity().loc() << ": skipping " << *node);
  return true;
}

void
MapValidation::updateVisited(const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                             const MapNode* node)
{
  size_t depth = city - _travelRoute->begin();
  std::set<const MapNode*>& visited = _visited[depth];
  visited.insert(node);
}

} // namespace tse
