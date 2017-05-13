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

class MapValidation : public MapTraversal
{
public:
  MapValidation(const std::vector<TravelRoute::CityCarrier>& travelRoute, int& city, bool& carrier)
    : MapTraversal(true, travelRoute, city, carrier)
  {
    _visited.resize(travelRoute.size());
  }

protected:
  virtual bool
  checkCarrierExceptions(const CarrierCode& carrier, const CarrierCode& mapCarrier,
                         char prevNodeType) const;
  virtual MapTraversal::MatchReturn
  getResult(bool isOverfly) const;

  virtual MatchReturn matchNode(const SpecifiedRouting* map,
                                const MapNode* node,
                                const MapNode* prevNode,
                                const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                                std::vector<TravelRoute::CityCarrier>::const_iterator& nextCity,
                                bool isOverfly) override;
  virtual bool
  isOverflyPossible(const MapNode* prevNode, const CarrierCode& carrier,
                    const GenericAllianceCode& genericAllianceCode,
                    const CarrierCode& mapCarrier, bool isOverfly = false);
  virtual bool
  checkCarrier(const MapNode* node, const CarrierCode& carrier,
               const GenericAllianceCode& genericAllianceCode, bool isOverfly = false);

private:
  bool
  isRepeat(const std::vector<TravelRoute::CityCarrier>::const_iterator& city, const MapNode* node);

  void updateVisited(const std::vector<TravelRoute::CityCarrier>::const_iterator& city,
                     const MapNode* node);
  std::vector<std::set<const MapNode*> > _visited;
};

} // namespace tse

