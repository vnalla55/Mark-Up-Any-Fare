//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Routing/MapTraversal.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/ATPResNationZones.h"
#include "Routing/SpecifiedRouting.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(fallbackRoutingValidationStartOnSecondMap);
FALLBACK_DECL(reworkTrxAborter);

static Logger
logger("atseintl.Routing.MapTraversal");

static const std::string delims = "-/";
static const char repDelim = '-';
static const char altDelim = '/';

MapTraversal::MapTraversal(bool exitOnFound,
                           const std::vector<TravelRoute::CityCarrier>& travelRoute,
                           int& city,
                           bool& carrier,
                           const uint16_t rtgStrMaxResSize /*default = 0*/)
  : _exitOnFound(exitOnFound),
    _travelRoute(&travelRoute),
    _missingCity(city),
    _missingCarrier(carrier),
    _rtgStrMaxResSize(rtgStrMaxResSize),
    _zoneNationPass(false)
{
}

MapTraversal::~MapTraversal() {}

bool
MapTraversal::hasPointOnMap(const TravelRoute::City& city,
                            const NationCode&,
                            const SpecifiedRouting* map) const
{
  return map->contains(city);
}

// should be removed with fallbackRoutingValidationStartOnSecondMap
bool
MapTraversal::hasPointOnMap(const TravelRoute::City& city,
                            const NationCode& nation,
                            const SpecifiedRouting* map,
                            const SpecifiedRouting* nextMap,
                            const SpecifiedRouting* nextMap2) const
{
  return map->contains(city) || (nextMap != nullptr && nextMap->contains(city))
      || (nextMap2 != nullptr && nextMap2->contains(city));
}

void
MapTraversal::setupStartingNode(MapNode& currentNode, const MapNode& node,
                                const LocCode& originLoc)
{
  std::set<tse::NodeCode> cityCodes;

  if(node.contains(originLoc))
  {
    cityCodes.insert(originLoc);
    currentNode.code() = cityCodes;
  } else
    currentNode.code() = node.code();

  currentNode.id() = node.id();
  currentNode.next() = node.next();
  currentNode.alt() = node.alt();
  currentNode.type() = node.type();
  currentNode.tag() = node.tag();

  _path.clear();
  _path.push_back(&currentNode);
}

bool
MapTraversal::execute(PricingTrx& trx,
                      std::vector<std::string>* result,
                      const SpecifiedRouting* map,
                      const SpecifiedRouting* nextMap,
                      const SpecifiedRouting* nextMap2)
{
  if (!fallback::fallbackRoutingValidationStartOnSecondMap(&trx))
    TSE_ASSERT(map);

  const TravelRoute::City& origin = _travelRoute->front().boardCity();
  const TravelRoute::City& dest = _travelRoute->back().offCity();

  if(_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize)
    return true;

  if(map->_passAll)
  {
    if(nextMap != nullptr)
      return execute(trx, result, nextMap, nextMap2, nullptr);
    else
    {
      if(result != nullptr)
        result->push_back(origin.loc() + "-" + dest.loc());
      return true;
    }
  }

  if (fallback::fallbackRoutingValidationStartOnSecondMap(&trx))
  {
    if (!hasPointOnMap(origin, _travelRoute->front().boardNation(), map, nextMap, nextMap2))
    {
      _missingCity = 0;
      return false;
    }

    if (!hasPointOnMap(dest, _travelRoute->back().offNation(), map, nextMap, nextMap2))
    {
      _missingCity = _travelRoute->size() - 1;
      return false;
    }

    if (!hasPointOnMap(origin, _travelRoute->front().boardNation(), map, nullptr, nullptr))
    {
      if (nextMap != nullptr)
        return execute(trx, result, nextMap, nextMap2, nullptr);
      else
      {
        _missingCity = 0;
        return false;
      }
    }
  }
  else
  {
    if (!hasPointOnMap(origin, _travelRoute->front().boardNation(), map))
    {
      _missingCity = 0;
      return false;
    }

    if (nextMap2 && !hasPointOnMap(dest, _travelRoute->back().offNation(), nextMap2))
      nextMap2 = nullptr;
    if (!nextMap2 && nextMap && !hasPointOnMap(dest, _travelRoute->back().offNation(), nextMap))
      nextMap = nullptr;

    if (!nextMap && !nextMap2 && !hasPointOnMap(dest, _travelRoute->back().offNation(), map))
    {
      _missingCity = _travelRoute->size() - 1;
      return false;
    }
  }

  std::set<const MapNode*, MapNodeCmp> nodes;
  getStartNodes(map, nodes);

  SpecRoutingDef specMaps(trx, map, nextMap, nextMap2);

  std::set<const MapNode*, MapNodeCmp>::iterator i;
  for(i = nodes.begin(); i != nodes.end(); ++i)
  {
    MapNode currentNode;
    setupStartingNode(currentNode, **i, origin.loc());

    if(enumeratePaths(map->getNext(&currentNode), &currentNode, &currentNode,
        _travelRoute->begin(), specMaps, OVERFLY_NOT_APPLY))
    {
      _missingCity = -1;
      if(_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize)
        break;
      if(_exitOnFound)
        return true;
    }
  }

  if(result != nullptr)
  {
    result->clear();
    result->insert(result->end(), _result.begin(), _result.end());
  }

  return !_result.empty();
}

void
MapTraversal::getStartNodes(const SpecifiedRouting* map,
                            std::set<const MapNode*, MapNodeCmp>& nodes)
{
  const TravelRoute::City& origin = _travelRoute->front().boardCity();

  std::map<const tse::NodeCode, std::set<const MapNode*>>::const_iterator i;
  std::set<const MapNode*>::const_iterator j;

  i = map->_city2node.find(origin.loc());
  if (i != map->_city2node.end())
  {
    for (j = i->second.begin(); j != i->second.end(); j++)
    {
      if (map->_terminal && (*j)->tag() != MapNode::ENTRY)
        continue;
      nodes.insert(*j);
    }
  }
  i = map->_city2node.find(MapNode::CATCHALL);
  if (UNLIKELY(i != map->_city2node.end()))
  {
    for (j = i->second.begin(); j != i->second.end(); j++)
    {
      if (map->_terminal && (*j)->tag() != MapNode::ENTRY)
        continue;
      nodes.insert(*j);
    }
  }

  if (!origin.airport().empty())
  {
    i = map->_city2node.find(origin.airport());
    if (i != map->_city2node.end())
    {
      for (j = i->second.begin(); j != i->second.end(); j++)
      {
        if (map->_terminal && (*j)->tag() != MapNode::ENTRY)
          continue;
        nodes.insert(*j);
      }
    }
  }
}

/**
 * Enumerate all paths matching an itinerary starting from a node.
 * Recurses through 'next' nodes and iterates through 'alt' nodes.
 */
bool
MapTraversal::enumeratePaths(const MapNode* currentNode, const MapNode* prevNode,
                             const MapNode* lastValidatedNode,
                             const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                             SpecRoutingDef& specMaps, bool isOverfly)
{
  PricingTrx& trx = std::get<TRX>(specMaps);
  const SpecifiedRouting* map = std::get<MAP>(specMaps);

  if (fallback::reworkTrxAborter(&trx))
    checkTrxAborted(trx);
  else
    trx.checkTrxAborted();

  bool found = false;

  if (!currentNode)
    onMatchFail(city, MATCH_FAIL_END);
  else do
  {
    std::vector<TravelRoute::CityCarrier>::const_iterator nextCity;

    _path.push_back(currentNode);
    MatchReturn ret = matchNode(map, currentNode, prevNode, city, nextCity, isOverfly);
    if (ret == MATCH_OK)
    {
      const MapNode* nextNode = map->getNext(currentNode);

      if (nextCity == _travelRoute->end())
      {
        if (!map->_terminal || currentNode->tag() == MapNode::EXIT)
        {
          found = true;
          outputPath(trx);
        }
        if (!(found &&
              (_exitOnFound || (_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize))))
        {
          if(city != nextCity)
            found |= enumeratePaths(nextNode, currentNode, currentNode, city, specMaps, isOverfly);
          else
            found |= enumeratePaths(nextNode, currentNode, lastValidatedNode, city, specMaps,
                isOverfly);
        }
      }
      else
      {
        if (city != nextCity)
        {
          _zoneNationPass = false;
          if (UNLIKELY(trx.getOptions()->isRtw() &&
              (currentNode->type() == MapNode::NATION || currentNode->type() == MapNode::ZONE)))
          {
            _zoneNationPass = true;
            found |= enumeratePaths(currentNode, prevNode, prevNode, nextCity, specMaps,
                OVERFLY_NOT_APPLY);
            _zoneNationPass = false;
          }

          if(!found)
            found |= enumeratePaths(nextNode, currentNode, currentNode, nextCity, specMaps,
                OVERFLY_NOT_APPLY);
          if(!found)
            found |= enumeratePaths(nextNode, currentNode, lastValidatedNode, city, specMaps,
                isOverfly);
        }
        else
        {
          bool zoneNationPass = _zoneNationPass;
          const bool applyOverfly = !_zoneNationPass
              && (currentNode->type() != MapNode::AIRLINE || isOverfly);
          _zoneNationPass = false;
          found |= enumeratePaths(nextNode, currentNode, lastValidatedNode, nextCity, specMaps,
              applyOverfly);
          if(zoneNationPass)
            break;
        }
      }
    }
    else
    {
      onMatchFail(city, ret);
    }
    if (found && (_exitOnFound || (_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize)))
      break;

    _path.pop_back();

    currentNode = map->getAlt(currentNode);
  } while (currentNode);

  // try enumerating paths from the next map
  if (!(found && (_exitOnFound || (_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize))))
    found |= jumpToMap(prevNode, lastValidatedNode, city, specMaps);

  return found;
}

/**
 * Compare the current node against the point in the itinerary and evaluate whether
 * to continue traversing the next node. The previous node is also examined if it's
 * an AIRLINE node.
 */
MapTraversal::MatchReturn
MapTraversal::matchNode(const SpecifiedRouting* map,
                        const MapNode* node,
                        const MapNode* prevNode,
                        const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                        std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
                        bool isOverfly)
{
  nextCity = city;
  if (checkNode(*node, city->offCity(), city->offNation()))
  {
    nextCity++;
  }
  return MATCH_OK;
}

/**
 *  Check if a node contains a city.
 */
bool
MapTraversal::checkNode(const MapNode& node,
                        const TravelRoute::City& city,
                        const NationCode& nation)
{
  // lint --e{1561}
  if (node.contains(city.loc()))
    return true;

  if (!city.airport().empty() && node.contains(city.airport()))
    return true;

  return false;
}

/**
 *  Check if we've gone farther than ever along the travel route.
 *  If so, adjust the missing city.
 */
void
MapTraversal::onMatchFail(const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                          MatchReturn matchReturn)
{

  int const depth = city - _travelRoute->begin();

  bool const checkDepth =
      (depth > _missingCity) && (static_cast<std::size_t>(depth) < (_travelRoute->size() - 1));

  if (!checkDepth)
  {
    return;
  }

  bool const checkBoardNation =
      isSameNation(city->boardNation(), _travelRoute->front().boardNation()) &&
      isSameNation(city->offNation(), _travelRoute->front().boardNation()) &&
      isSameNation((city + 1)->offNation(), _travelRoute->front().boardNation());

  bool const checkOffNation =
      isSameNation(city->boardNation(), _travelRoute->back().offNation()) &&
      isSameNation(city->offNation(), _travelRoute->back().offNation()) &&
      isSameNation((city + 1)->offNation(), _travelRoute->back().offNation());

  if (checkBoardNation || checkOffNation)
  {
    _missingCity = depth;
    _missingCarrier = (matchReturn == MATCH_FAIL_CARRIER);
  }
}

/**
 * If add-on construction, join up with another routing map.
 */
bool
MapTraversal::jumpToMap(const MapNode* node, const MapNode* lastValidatedNode,
                        const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                        SpecRoutingDef& specMaps)
{
  if (!std::get<NEXTMAP>(specMaps))
    return false;

  if (node->type() == MapNode::AIRLINE) // doesn't apply, only cities are joined
    return false;

  if (std::get<MAP>(specMaps)->_routing->commonPointInd() == SpecifiedRouting::ENTRY_EXIT &&
      node->tag() != MapNode::EXIT)
    return false;

  SpecRoutingDef updSpecMaps(std::get<TRX>(specMaps), std::get<NEXTMAP>(specMaps), std::get<NEXTMAP2>(specMaps), nullptr);

  return continueMap(node, lastValidatedNode, city, updSpecMaps);
}

/**
 * Continue traversing a map starting from where a previous map left off.
 * Note: called by another instance of SpecifiedRouting
 */
bool
MapTraversal::continueMap(const MapNode* node, const MapNode* lastValidatedNode,
                          const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                          SpecRoutingDef& specMaps)
{
  const SpecifiedRouting* map = std::get<MAP>(specMaps);

  bool found = false;
  // find last validated city = preview city from const_iterator "city"
  const TravelRoute::City prevCity =
      (city == _travelRoute->begin()) ? city->boardCity() : (city - 1)->offCity();

  // match all cities from the previous map's node against the cities in this map
  SpecifiedRouting::NodeMap cityNodes;

  for (const auto& elem : map->_city2node)
  {
    if (node->code().find(elem.first) != node->code().end())
      cityNodes.insert(elem);
  }

  // collect all unique nodes
  std::set<const MapNode*, MapNodeCmp> nodes;
  std::map<const tse::NodeCode, std::set<const MapNode*>>::iterator i;

  for (i = cityNodes.begin(); i != cityNodes.end(); ++i)
  {
    nodes.insert(i->second.begin(), i->second.end());
  }

  // loop through map nodes
  std::set<const MapNode*, MapNodeCmp>::iterator j;
  for (j = nodes.begin(); j != nodes.end(); ++j)
  {
    if (map->_routing->commonPointInd() == SpecifiedRouting::ENTRY_EXIT &&
        (*j)->tag() != MapNode::ENTRY)
      continue;

    std::set<tse::NodeCode> cityCodes;
    set_intersection((*j)->code().begin(),
                     (*j)->code().end(),
                     node->code().begin(),
                     node->code().end(),
                     inserter(cityCodes, cityCodes.begin()));

    // when we are trying to connect by the same node which at the same time is last validated city
    // on itinerary
    // we have to make sure that this city is in second map
    if (lastValidatedNode == node && cityCodes.find(prevCity.loc()) == cityCodes.end() &&
        _exitOnFound)
    {
      LOG4CXX_DEBUG(logger,
                    std::string("We can't allow to connecting maps between node1(id ")
                        << node->id() << "):" << *node << " and node2(id " << (*j)->id()
                        << "):" << **j << " because intersected part or node2 doesn't contain city "
                        << prevCity.loc() << " which is validated city on node1");
      continue;
    }

    MapNode currentNode;
    currentNode.type() = node->type();
    currentNode.tag() = MapNode::COMMONPOINT;
    currentNode.code() = cityCodes;

    _path.push_back(&currentNode);

    found |= enumeratePaths(map->getNext(*j), *j, lastValidatedNode, city, specMaps,
        OVERFLY_NOT_APPLY);
    _path.pop_back();

    if (found && (_exitOnFound || (_rtgStrMaxResSize > 0 && _result.size() >= _rtgStrMaxResSize)))
      break;
  }

  return found;
}

bool
MapTraversal::printNode(std::ostringstream& stream, const MapNode& node, bool isRTWorCT,
                        const std::string& before)
{
  if(!isRTWorCT && MapTraversal::isRestrictedNode(node))
  {
    stream.clear();
    return false;
  }

  if(_rtgStrMaxResSize == USHRT_MAX && node.tag() == MapNode::COMMONPOINT)
    node.printNodeElements(stream, before + "/", "/");
  else
    node.printNodeElements(stream, before, "");
  return true;
}

void
MapTraversal::getNodeAndResZonesIntersection(MapNode& boardZones, MapNode& offZones,
                                             DataHandle& dh, const NationCode& nation) const
{
  static constexpr int nodeCodeLen = 3;

  const std::vector<ATPResNationZones*>& resZones = dh.getATPResNationZones(nation);

  if(!resZones.empty() && !resZones.front()->zones().empty())
  {
    std::set<NodeCode> zoneSet;
    for (const std::string& resZone : resZones.front()->zones())
    {
      NodeCode zoneCode;
      if(resZone.length() > nodeCodeLen)
      {
        size_t pos = resZone.length() - nodeCodeLen;
        zoneCode = resZone.substr(pos, nodeCodeLen);
      }
      else if(resZone.length() == nodeCodeLen)
        zoneCode = resZone;

      zoneSet.insert(zoneCode);
    }

    std::set_intersection(zoneSet.begin(), zoneSet.end(), _path.front()->code().begin(),
        _path.front()->code().end(), std::inserter(boardZones.code(), boardZones.code().end()));

    std::set_intersection(zoneSet.begin(), zoneSet.end(), _path.back()->code().begin(),
        _path.back()->code().end(), std::inserter(offZones.code(), offZones.code().end()));
  }
}

void
MapTraversal::outputPath(const PricingTrx& trx)
{
  if (UNLIKELY(_path.empty()))
    return;

  const tse::NodeCode& orig = _travelRoute->front().boardCity().loc();
  const tse::NodeCode& dest = _travelRoute->back().offCity().loc();

  std::ostringstream stream;

  MapNode boardZones;
  MapNode offZones;

  if(orig == dest)
  {
    getNodeAndResZonesIntersection(boardZones, offZones, trx.dataHandle(),
          _travelRoute->front().boardNation());
  }

  std::vector<const MapNode*>::const_iterator i;
  for (i = _path.begin(); i != _path.end(); ++i)
  {
    if ((*i) == _path.front())
    {
      if ((*i)->tag() == MapNode::ENTRY)
        stream << '*';

      if ((*i)->type() != MapNode::CITY)
      {
        if ((**i).type() == MapNode::ZONE)
        {
          const MapNode& node = boardZones.code().empty() ? **i : boardZones;
          if (!printNode(stream, node, orig == dest, ""))
            break;
        }
        else if ((**i).type() == MapNode::NATION)
          stream << _travelRoute->front().boardNation();
      }
      else
        stream << orig;
    }
    else if ((*i) == _path.back())
    {
      if ((*i)->type() != MapNode::CITY)
      {
        if ((**i).type() == MapNode::ZONE)
        {
          const MapNode& node = offZones.code().empty() ? **i : offZones;
          if (!printNode(stream, node, orig == dest, "-"))
            break;
        }
        else if ((**i).type() == MapNode::NATION)
          stream << "-" << _travelRoute->back().offNation();
      }
      else
        stream << "-" << dest;

      if ((*i)->tag() == MapNode::EXIT)
        stream << '*';
    }
    else
    {
      ++i;
      if (i == _path.end() || (i != _path.end() && (*i)->tag() != MapNode::COMMONPOINT))
        --i;

      if (!printNode(stream, **i, orig == dest, "-"))
        break;
    }
  }

  if (LIKELY(_rtgStrMaxResSize != USHRT_MAX))
    processString(stream.str());
  else
  {
    LOG4CXX_DEBUG(logger, orig << "-" << dest << ": " << stream.str());

    _result.insert(stream.str());
  }
}

bool
MapTraversal::isRestrictedNode(const MapNode& node)
{
  switch (node.type())
  {
  case MapNode::AIRLINE:
    return (node.contains(NodeCode("*A")) || node.contains(NodeCode("*O")) ||
            node.contains(NodeCode("*S")));
    break;

  case MapNode::NATION:
  case MapNode::ZONE:
    return true;

  default:
    break;
  }

  return false;
}

void
MapTraversal::processString(std::string str)
{
#ifndef DEBUG_LOG_DISABLED
  const tse::NodeCode& orig = _travelRoute->front().boardCity().loc();
  const tse::NodeCode& dest = _travelRoute->back().offCity().loc();
#endif
  std::set<std::string>::const_iterator pos;

  if (UNLIKELY(str.empty()))
    return;
  removeAsterisks(str);
  removeRepetitions(str);
  if (UNLIKELY(isReversed(str)))
  reverseString(str);
  if (UNLIKELY(!startsWithOrig(str)))
  {
    std::string trimStr = "";
    trimToOrig(str, trimStr);
    pos = _result.find(trimStr);
    if (pos == _result.end())
    {
      LOG4CXX_DEBUG(logger, orig << "-" << dest << ": " << trimStr);

      _result.insert(trimStr);
    }
  }

  pos = _result.find(str);
  if (UNLIKELY(pos != _result.end()))
    return;

  LOG4CXX_DEBUG(logger, orig << "-" << dest << ": " << str);

  _result.insert(str);
}

void
MapTraversal::removeAsterisks(std::string& str) const
{
  if (UNLIKELY(str.empty()))
    return;
  if (str[0] == '*')
    str.erase(str.begin());
  if (str[str.length() - 1] == '*')
    str.erase(str.end() - 1);
}

void
MapTraversal::removeRepetitions(std::string& str) const
{
  std::string::size_type pos1(str.find(repDelim));
  if (UNLIKELY(pos1 == std::string::npos))
    return;
  std::string::size_type pos2(pos1);
  std::string prev(str.substr(0, pos1++));
  while (pos1 < str.size() - 1 && (pos2 = str.find(repDelim, pos1)) != std::string::npos)
  {
    if (str.substr(pos1, pos2 - pos1) == prev && prev.find(altDelim) == std::string::npos)
      str.erase(pos1, pos2 - pos1 + 1);
    else
    {
      prev = str.substr(pos1, pos2 - pos1);
      pos1 = pos2 + 1;
    }
  }
  if (pos1 < str.size() - 1 && str.substr(pos1) == prev)
    str.erase(pos1 - 1);
}
bool
MapTraversal::isReversed(const std::string& str) const
{
  return str.substr(0, 3) == _travelRoute->back().offCity().loc() ||
         str.substr(str.size() - 3) == _travelRoute->front().boardCity().loc();
}
void
MapTraversal::reverseString(std::string& str) const
{
  std::string rev;
  std::string::size_type pos;
  while ((pos = str.find_last_of(delims)) != std::string::npos)
  {
    rev.append(str.substr(pos + 1));
    rev.append(1, str[pos]);
    str.erase(pos);
  }
  rev.append(str);
  rev.swap(str);
}
bool
MapTraversal::startsWithOrig(const std::string& str) const
{
  return str.substr(0, 3) == _travelRoute->front().boardCity().loc();
}
void
MapTraversal::trimToOrig(std::string& str, std::string& trimStr)
{
  std::string::size_type pos(str.find(_travelRoute->front().boardCity().loc()));
  if (pos == std::string::npos)
    return;
  std::string::size_type before(str.rfind(repDelim, pos));
  pos = str.find(repDelim, pos);
  if (pos == std::string::npos)
    return;
  if (before != std::string::npos)
  {
    trimStr = str.substr(0, before + 1) + _travelRoute->front().boardCity().loc();
    reverseString(trimStr);
  }
  str.assign(_travelRoute->front().boardCity().loc() + str.substr(pos));
}

class CompareNode
{
public:
  CompareNode() {}

  bool operator()(const MapNode* n1, const MapNode* n2)
  {
    return (std::find_first_of(
                n1->code().begin(), n1->code().end(), n2->code().begin(), n2->code().end()) !=
            n1->code().end());
  }
};

} // namespace tse
