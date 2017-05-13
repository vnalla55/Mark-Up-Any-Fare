//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Routing/MapNode.h"
#include "Routing/SpecifiedRouting.h"
#include "Routing/TravelRoute.h"

#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <stdint.h>

namespace tse
{

class DataHandle;
class SpecifiedRouting;

class MapTraversal
{
public:
  typedef std::tuple<PricingTrx&, const SpecifiedRouting*, const SpecifiedRouting*,
      const SpecifiedRouting*> SpecRoutingDef;
  static constexpr int TRX = 0;
  static constexpr int MAP = 1;
  static constexpr int NEXTMAP = 2;
  static constexpr int NEXTMAP2 = 3;

  bool execute(PricingTrx& trx,
               std::vector<std::string>* result,
               const SpecifiedRouting* map,
               const SpecifiedRouting* nextMap = nullptr,
               const SpecifiedRouting* nextMap2 = nullptr);

protected:
  enum MatchReturn
  {
    MATCH_OK,
    MATCH_FAIL_CITY,
    MATCH_FAIL_CARRIER,
    MATCH_FAIL_END
  };

  bool _exitOnFound;
  const std::vector<TravelRoute::CityCarrier>* _travelRoute;
  int& _missingCity;
  bool& _missingCarrier;
  std::vector<const MapNode*> _path;
  std::set<std::string> _result;
  const uint16_t _rtgStrMaxResSize;
  bool _zoneNationPass;

  MapTraversal(bool exitOnFound,
               const std::vector<TravelRoute::CityCarrier>& travelRoute,
               int& city,
               bool& carrier,
               const uint16_t rtgStrMaxResSize = 0);

  virtual ~MapTraversal();

  virtual void
  setupStartingNode(MapNode& currentNode, const MapNode& node, const LocCode& originLoc);

  virtual bool
  hasPointOnMap(const TravelRoute::City&, const NationCode&, const SpecifiedRouting*) const;

  // should be removed with fallbackRoutingValidationStartOnSecondMap
  virtual bool hasPointOnMap(const TravelRoute::City& city,
                             const NationCode& nation,
                             const SpecifiedRouting* map,
                             const SpecifiedRouting* nextMap,
                             const SpecifiedRouting* nextMap2) const;
  inline bool hasPointOnMapRTW(const TravelRoute::City& city,
                               const NationCode& nation,
                               const SpecifiedRouting* map) const
  {
    return MapTraversal::hasPointOnMap(city, nation, map) || map->containsNation(nation);
  }
  inline bool hasPointOnMapRTW(const TravelRoute::City& city,
                               const NationCode& nation,
                               const SpecifiedRouting* map,
                               const SpecifiedRouting* nextMap,
                               const SpecifiedRouting* nextMap2) const
  {
    return MapTraversal::hasPointOnMap(city, nation, map, nextMap, nextMap2)
        || map->containsNation(nation);
  }

  virtual void
  getStartNodes(const SpecifiedRouting* map, std::set<const MapNode*, MapNodeCmp>& nodes);

  inline void
  getStartNodesRTW(const SpecifiedRouting* map, std::set<const MapNode*,MapNodeCmp>& nodes)
  {
    MapTraversal::getStartNodes(map, nodes);
    map->getStartNodesNationZone(*_travelRoute, nodes);
  }

  bool
  enumeratePaths(const MapNode* currentNode, const MapNode* prevNode,
                 const MapNode* lastValidatedNode,
                 const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                 SpecRoutingDef& specMaps, bool isOverfly);

  virtual MatchReturn matchNode(const SpecifiedRouting* map,
                                const MapNode* node,
                                const MapNode* prevNode,
                                const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                                std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
                                bool isOverfly);

  virtual bool
  checkNode(const MapNode& node, const TravelRoute::City& city, const NationCode& nation);

  inline bool
  checkNodeRTW(const MapNode& node, const TravelRoute::City& offCity, const NationCode& offNation)
  {
    return (node.type() == MapNode::CITY && MapTraversal::checkNode(node, offCity, offNation))
        || (node.type() == MapNode::NATION && node.contains(offNation))
        || (node.type() == MapNode::ZONE && node.hasNationInZone(offNation));
  }

  inline bool isSameNation(const NationCode& n1, const NationCode& n2) const
  {
    if (n1 == n2)
      return true;
    return (n1 == UNITED_STATES || n1 == CANADA) && (n2 == UNITED_STATES || n2 == CANADA);
  }

  virtual void onMatchFail(const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                           MatchReturn matchReturn);
  bool jumpToMap(const MapNode* node,
                 const MapNode* lastValidatedNode,
                 const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                 SpecRoutingDef& specMaps);

  bool continueMap(const MapNode* node,
                   const MapNode* lastValidatedNode,
                   const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                   SpecRoutingDef& specMaps);

  void getNodeAndResZonesIntersection(MapNode& boardZones, MapNode& offZones, DataHandle& dh,
                                      const NationCode& nation) const;

  virtual void outputPath(const PricingTrx& trx);

  void processString(std::string str);
  void removeAsterisks(std::string& str) const;
  void removeRepetitions(std::string& str) const;
  bool isReversed(const std::string& str) const;
  void reverseString(std::string& str) const;
  bool startsWithOrig(const std::string& str) const;
  void trimToOrig(std::string& str, std::string& trimStr);
  bool printNode(std::ostringstream& stream, const MapNode& node, bool isRTWorCT,
                 const std::string& before);

  friend class MapTraversalTest;

private:
  static bool isRestrictedNode(const MapNode& node);
};

} // namespace tse

