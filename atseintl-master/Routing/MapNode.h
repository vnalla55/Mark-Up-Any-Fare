//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Code.h"
#include "Common/TseCodeTypes.h"

#include <iostream>
#include <set>
#include <string>

#include <stdint.h>

//----------------------------------------------------

namespace tse
{

class RoutingMap;
class PricingTrx;
typedef Code<3> NodeCode;

class MapNode
{
private:
  int16_t _id;
  char _type;
  char _tag;
  int16_t _next;
  int16_t _alt;
  bool _reverse;
  std::set<NodeCode> _code;
  std::set<NationCode> _nations;
  std::set<NationCode> _zone2nations;

public:
  // Constants
  //
  static constexpr char CITY = 'C';
  static constexpr char AIRLINE = 'A';
  static constexpr char ENTRY = '1';
  static constexpr char CONNECT = ' ';
  static constexpr char EXIT = 'X';
  static constexpr char LOCAL = 'L';
  static constexpr char COMMONPOINT = 'J';
  static constexpr char NATION = 'N';
  static constexpr char ZONE = 'Z';
  static NodeCode CATCHALL;

  MapNode();
  MapNode(PricingTrx& trx, const RoutingMap& routeRecord);
  MapNode(const MapNode& MapNode);
  virtual ~MapNode();

  // Accessors a.k.a crime against humanity
  //
  int16_t& id() { return _id; }
  const int16_t& id() const { return _id; }

  char& type() { return _type; }
  const char& type() const { return _type; }

  char& tag() { return _tag; }
  const char& tag() const { return _tag; }

  int16_t& next() { return _next; }
  const int16_t& next() const { return _next; }

  int16_t& alt() { return _alt; }
  const int16_t& alt() const { return _alt; }

  bool& isReverse() { return _reverse; }
  const bool& isReverse() const { return _reverse; }

  std::set<NodeCode>& code() { return _code; }
  const std::set<NodeCode>& code() const { return _code; }

  std::set<NationCode>& nations() { return _nations; }
  const std::set<NationCode>& nations() const { return _nations; }

  std::set<NationCode>& zone2nations() { return _zone2nations; }
  const std::set<NationCode>& zone2nations() const { return _zone2nations; }

  void printNodeElements(std::ostream& stream,
                         const std::string& prefix,
                         const std::string& suffix) const;
  friend std::ostream& operator<<(std::ostream& stream, const MapNode& node);

  // Convenience functions
  //
  bool contains(const NodeCode& code) const;
  bool hasNation(const NationCode& nation) const;
  bool hasNationInZone(const NationCode& nation) const;
};

class MapNodeCmp
{
public:
  bool operator()(const MapNode* n1, const MapNode* n2) const { return (n1->id() < n2->id()); }
};

} // namespace tse

