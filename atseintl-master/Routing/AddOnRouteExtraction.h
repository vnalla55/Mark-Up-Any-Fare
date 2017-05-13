//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Routing/MapTraversal.h"

namespace tse
{
class Loc;

class AddOnRouteExtraction : public MapTraversal
{
public:
  AddOnRouteExtraction(const Loc* city);

protected:
  MapTraversal::MatchReturn
  matchNode(const SpecifiedRouting* map,
            const MapNode* node,
            const MapNode* prevNode,
            const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
            std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
            bool isOverfly) override;

  void
  getStartNodes(const SpecifiedRouting* map, std::set<const MapNode*, MapNodeCmp>& nodes) override;

  void outputPath();

private:
  const Loc* _loc;
  std::vector<TravelRoute::CityCarrier> _dummyRoute;
  std::set<const MapNode*> _visited;

  // not used, but required by MapTraversal constructor
  int _missCity;
  bool _missCarrier;

  static bool hasNation(const MapNode&, const NationCode&);
};

} // namespace tse

