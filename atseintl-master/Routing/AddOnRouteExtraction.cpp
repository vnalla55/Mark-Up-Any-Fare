//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/AddOnRouteExtraction.h"

#include "Common/Logger.h"
#include "DBAccess/Loc.h"
#include "Routing/SpecifiedRouting.h"

namespace tse
{

static Logger
logger("atseintl.Routing.AddOnRouteExtraction");

AddOnRouteExtraction::AddOnRouteExtraction(const Loc* loc)
  : MapTraversal(false, _dummyRoute, _missCity, _missCarrier), // lint !e1403
    _loc(loc)
{
  _dummyRoute.resize(1);
  TravelRoute::CityCarrier& city = _dummyRoute[0];
  city.boardCity().loc() = loc->loc();
  city.boardNation() = loc->nation();
  city.offCity().loc() = loc->loc();
  city.offNation() = loc->nation();
}

/**
 * Compare the current node against the point in the itinerary and evaluate whether
 * to continue traversing the next node. The previous node is also examined if it's
 * an AIRLINE node.
 */
MapTraversal::MatchReturn
AddOnRouteExtraction::matchNode(const SpecifiedRouting* map,
                                const MapNode* node,
                                const MapNode* prevNode,
                                const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                                std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
                                bool isOverfly)
{
  bool has_nation = false;

  nextCity = city;
  if (node->type() == MapNode::CITY)
  {
    if (_visited.find(node) != _visited.end())
      _visited.insert(node);

    if (!(has_nation = hasNation(*node, _loc->nation())) || map->getNext(node) == nullptr)
    {
      if (!has_nation)
        _path.pop_back();

      std::vector<const MapNode*>::iterator i = _path.begin();
      for (; i != _path.end(); ++i)
      {
        if ((*i)->contains(_loc->loc())) // lint !e1561
        {
          outputPath();
          break;
        }
      }
      if (!has_nation)
        _path.push_back(node);

      return MATCH_FAIL_CITY;
    }
  }
  return MATCH_OK;
}

void
AddOnRouteExtraction::getStartNodes(const SpecifiedRouting* map,
                                    std::set<const MapNode*, MapNodeCmp>& nodes)
{
  // start nodes are all nodes that contain the nation

  NationCode const& nation = _loc->nation();

  std::map<int16_t, MapNode>::const_iterator i = map->nodes().begin();
  for (; i != map->nodes().end(); ++i)
  {
    MapNode const* node = &i->second;
    if (node->type() != MapNode::CITY)
      continue;
    if (node->nations().find(nation) != node->nations().end())
      nodes.insert(node);
  }
}

void
AddOnRouteExtraction::outputPath()
{
  if (_path.empty())
    return;

  std::ostringstream stream;

  std::vector<const MapNode*>::const_iterator i;
  for (i = _path.begin(); i != _path.end(); i++)
  {
    if ((*i) == _path.front())
    {
      if ((*i)->tag() == MapNode::ENTRY)
        stream << '*';
      if ((*i)->contains(_loc->loc())) // lint !e1561
        stream << _loc->loc();
      else
        stream << **i;
    }
    else if ((*i) == _path.back())
    {
      stream << "-";
      if ((*i)->contains(_loc->loc())) // lint !e1561
        stream << _loc->loc();
      else
        stream << **i;
      if ((*i)->tag() == MapNode::EXIT)
        stream << '*';
    }
    else
    {
      stream << "-" << **i;
    }
  }

  processString(stream.str());
}

bool
AddOnRouteExtraction::hasNation(const MapNode& node, const NationCode& nation)
{
  if (nation == NATION_US || nation == NATION_CA)
    return node.hasNation(NATION_US) || node.hasNation(NATION_CA);
  return node.hasNation(nation);
}

} // namespace tse
