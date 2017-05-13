//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Routing/MapValidation.h"


namespace tse
{

class MapValidator : public MapValidation
{
public:
  MapValidator(const std::vector<TravelRoute::CityCarrier>& travelRoute, int& city,
                 bool& carrier);

  virtual ~MapValidator();

protected:
  virtual void
  setupStartingNode(MapNode& currentNode, const MapNode& node, const LocCode& originLoc) override;

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

  virtual bool checkCarrierExceptions(const CarrierCode& carrier,
                                      const CarrierCode& mapCarrier,
                                      char prevNodeType) const override;
  virtual MapTraversal::MatchReturn getResult(bool isOverfly) const override;

  virtual bool checkNode(const MapNode& node,
                         const TravelRoute::City& city,
                         const NationCode& offNation) override;

  virtual bool checkCarrier(const MapNode* node,
                            const CarrierCode& carrier,
                            const GenericAllianceCode& genericAllianceCode,
                            bool isOverfly = false) override;
  virtual bool isOverflyPossible(const MapNode* prevNode,
                                 const CarrierCode& carrier,
                                 const GenericAllianceCode& genericAllianceCode,
                                 const CarrierCode& mapCarrier,
                                 bool isOverfly = false) override;

};

} // namespace tse

