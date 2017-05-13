//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Routing.h"
#include "Routing/MapNode.h"
#include "Routing/TravelRoute.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{
class SpecifiedRouting
{
public:
  using NodeMap = std::map<const NodeCode, std::set<const MapNode*>>;

  // Lifecycle
  //
  SpecifiedRouting() = default;

  SpecifiedRouting(const Routing& routing, PricingTrx& trx);
  virtual ~SpecifiedRouting() = default;
  void initialize(const Routing& routing, PricingTrx& trx, const Routing* baseRouting = nullptr);

  // Constants
  //
  static constexpr char ENTRY_EXIT = '1'; // Common point ind; means join map at entry/exit only

  // Processing
  //

  const MapNode* getNext(const MapNode* node) const;
  const MapNode* getAlt(const MapNode* node) const;

  friend std::ostream& operator<<(std::ostream& stream, const SpecifiedRouting& route);

  // TB: added for testing purposes;
  //     remove if really unnecessary (and face consequences :-)
  bool operator==(SpecifiedRouting& other);

  CarrierCode const& carrier() const { return _carrier; }

  std::map<int16_t, MapNode> const& nodes() const { return _node; }

  bool reverseMap(PricingTrx& trx);
  void getStartNodesNationZone(const std::vector<TravelRoute::CityCarrier>& travelRoute,
                               std::set<const MapNode*, MapNodeCmp>& nodes) const;
  bool contains(const TravelRoute::City& city) const;
  bool containsNation(const NodeCode& nation) const;

  bool getTerminal() const { return _terminal; }
  void setTerminal(bool term) { _terminal = term; }

protected:
  // map data
  //
  const Routing* _routing = nullptr;
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _tariff = 0;
  RoutingNumber _routeNumber;
  bool _terminal = false;
  bool _passAll = false; // all locs are ZZZ

  std::map<int16_t, MapNode> _node;

  // map indexes
  //
  NodeMap _city2node;
  NodeMap _nation2node;
  NodeMap _nation2nodeEntry;

  const MapNode* getNode(int16_t id) const;

  // Node construction
  //
  void addNode(PricingTrx& trx, const RoutingMap& routeRecord);
  bool optimize(PricingTrx& trx);
  bool compress();
  bool merge(MapNode* node, const MapNode* alt);
  void updateNodeMapping(const NodeCode& code, const MapNode* node);
  bool reindex();
  void reindexNation();
  bool reverse(MapNode* node, int16_t next);

  friend class MapTraversal;
  friend class MapValidator;
  friend class MapValidatorTest;

  friend bool
  operator<(const std::map<const NodeCode, std::set<const MapNode*> >::value_type& cityNode,
            const NodeCode& code);

  friend bool
  operator<(const NodeCode& code,
            const std::map<const NodeCode, std::set<const MapNode*> >::value_type& cityNode);

  virtual bool needMergeNode(MapNode* node, const MapNode* alt) const;
};

class SpecifiedRoutingDiag : public SpecifiedRouting
{
public:
  SpecifiedRoutingDiag() = default;
  SpecifiedRoutingDiag(const Routing& routing, PricingTrx& trx) : SpecifiedRouting(routing, trx) {}

protected:
  virtual bool needMergeNode(MapNode* node, const MapNode* alt) const override;
};

} // namespace tse

