//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
//

#include "Routing/SpecifiedRouting.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MapTraversal.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace tse
{

namespace
{
Logger
logger("atseintl.Routing.SpecifiedRouting");
}

SpecifiedRouting::SpecifiedRouting(const Routing& routing, PricingTrx& trx)
{
  initialize(routing, trx);
}

void
SpecifiedRouting::initialize(const Routing& routing, PricingTrx& trx, const Routing* baseRouting)
{
  _routing = &routing;
  _vendor = routing.vendor();
  _carrier = routing.carrier();
  _tariff = routing.routingTariff();
  _routeNumber = routing.routing();

  bool isActivated = TrxUtil::isFullMapRoutingActivated(trx) &&
                     (baseRouting ? (baseRouting->entryExitPointInd() != GETTERMPTFROMCRXPREF)
                                  : (routing.entryExitPointInd() != GETTERMPTFROMCRXPREF));

  if (isActivated)
  {
    Indicator entryExitPointInd =
        baseRouting ? baseRouting->entryExitPointInd() : routing.entryExitPointInd();

    _terminal = (entryExitPointInd == ENTRYEXITONLY);
  }
  else
  {
    const CarrierPreference* pref =
        trx.dataHandle().getCarrierPreference(_carrier, trx.travelDate());
    _terminal = (pref != nullptr) && (pref->applyrtevaltoterminalpt() == 'Y');
  }

  _passAll = false;

  const std::vector<tse::RoutingMap*>* rmaps = &(routing.rmaps());
  while (rmaps->size() == 1)
  {
    const RoutingMap& localMap = *(rmaps->front());
    if (localMap.loctag() != MapNode::LOCAL)
    {
      break;
    }

    LocCode rtg = localMap.localRouting();
    while (rtg.size() < 4)
      rtg = '0' + rtg;

    const std::vector<Routing*>& routingVect =
        trx.dataHandle().getRouting(_vendor, _carrier, _tariff, rtg, trx.travelDate());

    if (routingVect.empty())
    {
      LOG4CXX_WARN(logger,
                   "Local routing not found " << _vendor << '/' << _carrier << '/' << _tariff << '/'
                                              << rtg);
      return;
    }
    const Routing* localRouting = routingVect.front();
    rmaps = &(localRouting->rmaps());
  }

  if (rmaps->size() == 2 && rmaps->front()->loc1().loc() == MapNode::CATCHALL // lint !e530
      &&
      rmaps->back()->loc1().loc() == MapNode::CATCHALL) // lint !e530
  {
    _passAll = true;
    return;
  }

  std::vector<RoutingMap*>::const_iterator i;
  for (i = rmaps->begin(); i != rmaps->end(); i++)
  {
    if (UNLIKELY((*i)->loc1().locType() == MapNode::CITY &&
        ((*i)->loc1().loc() == EastCoastCode || (*i)->loc1().loc() == WestCoastCode)))
    {
      _node.clear();
      break;
    }
    addNode(trx, **i);
  }
  optimize(trx);
}

std::ostream& operator<<(std::ostream& stream, const SpecifiedRouting& route)
{
  std::map<int16_t, MapNode>::const_iterator i;
  for (i = route._node.begin(); i != route._node.end(); i++)
    stream << i->second << std::endl;
  stream << std::endl;

  return stream;
}

void
SpecifiedRouting::addNode(PricingTrx& trx, const RoutingMap& routeRecord)
{
  MapNode node(trx, routeRecord);

  // ignore local points !!
  if (UNLIKELY(node.tag() == MapNode::LOCAL))
    return;

  // can't have next and be an exit
  if (UNLIKELY(node.tag() == MapNode::EXIT && node.next() > 0))
    node.tag() = MapNode::CONNECT;

  _node[node.id()] = node;
}

bool
SpecifiedRouting::optimize(PricingTrx& trx)
{
  compress();
  reindex();

  if (UNLIKELY(trx.getOptions()->isRtw()))
    reindexNation();

  return true;
}

bool
SpecifiedRouting::compress()
{
  std::set<int16_t> deleted;

  std::map<int16_t, MapNode>::iterator i;
  for (i = _node.begin(); i != _node.end(); i++)
  {
    if (deleted.find(i->first) == deleted.end())
    {
      MapNode* node = &(i->second);
      const MapNode* alt = getAlt(node);
      while (alt != nullptr)
      {
        if (!merge(node, alt))
          break;

        deleted.insert(alt->id());
        alt = getAlt(alt);
      }
    }
  }

  std::set<int16_t>::iterator j = deleted.begin();
  for (; j != deleted.end(); j++)
  {
    _node.erase(*j);
  }

  return !deleted.empty();
}

bool
SpecifiedRouting::merge(MapNode* node, const MapNode* alt)
{
  if (needMergeNode(node, alt))
  {
    node->code().insert(alt->code().begin(), alt->code().end());
    node->nations().insert(alt->nations().begin(), alt->nations().end());
    node->alt() = alt->alt();
    node->zone2nations().insert(alt->zone2nations().begin(), alt->zone2nations().end());

    return true;
  }
  else
  {
    return false;
  }
}

void
SpecifiedRouting::updateNodeMapping(const NodeCode& code, const MapNode* node)
{
  _nation2node[code].insert(node);
  if (_terminal && node->tag() == MapNode::ENTRY)
    _nation2nodeEntry[code].insert(node);
}

bool
SpecifiedRouting::reindex()
{
  _city2node.clear();

  std::map<int16_t, MapNode>::iterator i;
  for (i = _node.begin(); i != _node.end(); i++)
  {
    MapNode* node = &(i->second);

    if (node->type() == MapNode::CITY)
    {
      std::set<NodeCode>::iterator j = node->code().begin();
      for (; j != node->code().end(); ++j)
        _city2node[*j].insert(node);
    }
  }
  return true;
}

void
SpecifiedRouting::reindexNation()
{
  _nation2nodeEntry.clear();
  _nation2node.clear();

  typedef std::map<int16_t, MapNode>::value_type value_type;
  for (value_type& nodePair : _node)
  {
    MapNode* node = &(nodePair.second);

    switch (node->type())
    {
    case MapNode::NATION:
    {
      for (const NodeCode& nCode : node->code())
      {
        updateNodeMapping(nCode, node);
      }
    }
    break;
    case MapNode::ZONE:
    {
      for (const std::set<NationCode>::value_type& nCode : node->zone2nations())
      {
        updateNodeMapping(nCode, node);
      }
    }
    default:
      break;
    }
  }
}

bool
SpecifiedRouting::contains(const TravelRoute::City& city) const
{
  if (_city2node.find(city.loc()) != _city2node.end())
  {
    LOG4CXX_DEBUG(logger, "loc found " << city.loc());
    return true;
  }

  if (!city.airport().empty() && _city2node.find(city.airport()) != _city2node.end())
  {
    LOG4CXX_DEBUG(logger, "airport found " << city.airport());
    return true;
  }

  if (_city2node.find(MapNode::CATCHALL) != _city2node.end())
  {
    LOG4CXX_DEBUG(logger, "ZZZ found " << city.loc());
    return true;
  }

  LOG4CXX_DEBUG(logger, "neither loc nor airport found " << city.loc() << "/" << city.airport());
  return false;
}

bool
SpecifiedRouting::containsNation(const NodeCode& nation) const
{
  return _nation2node.find(nation) != _nation2node.end();
}

/**
 * Compare a city index with a city
 */
bool operator<(const std::map<const NodeCode, std::set<const MapNode*> >::value_type& cityNode,
               const NodeCode& code)
{
  return cityNode.first < code;
}

/**
 * Compare a city with a city index
 */
bool operator<(const NodeCode& code,
               const std::map<const NodeCode, std::set<const MapNode*> >::value_type& cityNode)

{
  return code < cityNode.first;
}

const MapNode*
SpecifiedRouting::getNext(const MapNode* node) const
{
  if (node == nullptr || node->next() == 0)
    return nullptr;

  return getNode(node->next());
}

const MapNode*
SpecifiedRouting::getAlt(const MapNode* node) const
{
  if (node == nullptr || node->alt() == 0)
    return nullptr;

  return getNode(node->alt());
}

const MapNode*
SpecifiedRouting::getNode(int16_t id) const
{
  std::map<int16_t, MapNode>::const_iterator i;
  if (UNLIKELY((i = _node.find(id)) == _node.end()))
  {
    LOG4CXX_WARN(logger,
                 "Error in route map: " << _vendor << '/' << _carrier << '/' << _tariff << '/'
                                        << _routeNumber << " node " << id << " not found: ");
    return nullptr;
  }

  return &(i->second);
}

bool
compareMapNodes(MapNode& mapNode1, MapNode& mapNode2)
{
  if (mapNode1.id() != mapNode2.id())
    return false;
  if (mapNode1.type() != mapNode2.type())
    return false;
  if (mapNode1.tag() != mapNode2.tag())
    return false;
  if (mapNode1.next() != mapNode2.next())
    return false;
  if (mapNode1.alt() != mapNode2.alt())
    return false;
  if (mapNode1.code().size() != mapNode2.code().size())
    return false;

  std::set<NodeCode>::const_iterator code = mapNode1.code().begin();
  std::set<NodeCode>::const_iterator end = mapNode1.code().end();
  for (; code != end; ++code)
  {
    if (mapNode2.code().find(*code) != mapNode2.code().end())
      return false;
  }
  return true;
}

bool
SpecifiedRouting::
operator==(SpecifiedRouting& other)
{
  if (!(_vendor == other._vendor))
    return false;

  if (!(_carrier == other._carrier))
    return false;
  if (!(_tariff == other._tariff))
    return false;
  if (!(_routeNumber == other._routeNumber))
    return false;
  if (_terminal != other._terminal)
    return false;
  if (_passAll != other._passAll)
    return false;

  if (_node.size() != other._node.size())
    return false;
  if (_city2node.size() != other._city2node.size())
    return false;
  if (_nation2node.size() != other._nation2node.size())
    return false;

  std::map<int16_t, MapNode>::const_iterator my = _node.begin();
  std::map<int16_t, MapNode>::const_iterator end = _node.end();

  for (; my != end; ++my)
  {
    int16_t key = my->first;
    MapNode map = my->second;
    if (!compareMapNodes(map, other._node[key]))
      return false;
  }

  return true;
}

bool
SpecifiedRouting::reverseMap(PricingTrx& trx)
{
  std::map<int16_t, MapNode>::iterator i;

  for (i = _node.begin(); i != _node.end(); i++)
  {
    MapNode* node = &(i->second);
    if (node->tag() != MapNode::ENTRY)
      continue;

    if (node->type() != MapNode::AIRLINE && !node->isReverse())
    {
      if (UNLIKELY(!reverse(node, 0)))
        return false;
    }
  }

  reindex();

  if (UNLIKELY(trx.getOptions()->isRtw()))
    reindexNation();

  return true;
}

bool
SpecifiedRouting::reverse(MapNode* node, int16_t next)
{
  int ret = true;
  std::map<int16_t, MapNode>::iterator pos;
  int16_t id = node->id();
  int16_t alt = node->alt();

  if (UNLIKELY(node->isReverse()))
    return true;

  if (node->next() != 0 && ret)
  {
    pos = _node.find(node->next());
    if (UNLIKELY(pos == _node.end()))
      return false;

    if (pos->second.isReverse())
    {
      if (LIKELY(pos->second.next() != 0))
      {
        pos = _node.find(pos->second.next());
        if (UNLIKELY(pos == _node.end()))
          return false;
        if (UNLIKELY(!pos->second.isReverse()))
          return false;

        while (pos->second.alt() != 0)
        {
          pos = _node.find(pos->second.alt());
          if (UNLIKELY(pos == _node.end()))
            return false;
          if (UNLIKELY(!pos->second.isReverse()))
            return false;
        }
        pos->second.alt() = id;
      }
      else
        return false;
    }
    else
    {
      if (UNLIKELY(pos->second.tag() == MapNode::ENTRY))
        return false;
      ret = reverse(&(pos->second), id);
    }
  }

  node->next() = next;
  node->alt() = 0;
  node->isReverse() = true;
  if (node->tag() == '1' || node->tag() == 'X')
    node->tag() = (node->tag() == '1') ? ('X') : ('1');

  if (alt != 0 && ret)
  {
    pos = _node.find(alt);
    if (UNLIKELY(pos == _node.end()))
      return false;

    if (UNLIKELY(pos->second.isReverse()))
    {
      if (pos->second.next() != 0)
      {
        pos = _node.find(pos->second.next());
        if (pos == _node.end())
          return false;
        if (!pos->second.isReverse())
          return false;

        while (pos->second.alt() != 0)
        {
          pos = _node.find(pos->second.alt());
          if (pos == _node.end())
            return false;
          if (!pos->second.isReverse())
            return false;
        }
        pos->second.alt() = next;
      }
      else
        return false;
    }
    else
      ret = reverse(&(pos->second), next);
  }

  return ret;
}

void
SpecifiedRouting::getStartNodesNationZone(const std::vector<TravelRoute::CityCarrier>& travelRoute,
                                          std::set<const MapNode*, MapNodeCmp>& nodes) const
{
  NodeMap::const_iterator nation2NodeItEnd;
  NodeMap::const_iterator nationIt;

  if (getTerminal())
  {
    nation2NodeItEnd = _nation2nodeEntry.end();
    nationIt = _nation2nodeEntry.find(travelRoute.front().boardNation());
  }
  else
  {
    nation2NodeItEnd = _nation2node.end();
    nationIt = _nation2node.find(travelRoute.front().boardNation());
  }

  if (nationIt != nation2NodeItEnd)
    nodes.insert(nationIt->second.begin(), nationIt->second.end());
}

bool
SpecifiedRouting::needMergeNode(MapNode* node, const MapNode* alt) const
{
  return node->type() != NATION && node->type() != ZONE && node->type() == alt->type() &&
         node->tag() == alt->tag() && node->next() == alt->next();
}

bool
SpecifiedRoutingDiag::needMergeNode(MapNode* node, const MapNode* alt) const
{
  return node->type() == alt->type() && node->tag() == alt->tag() && node->next() == alt->next();
}

} // namespace tse
