//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#include "Routing/MapNode.h"

#include "Common/LocUtil.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/ZoneInfo.h"

#include <algorithm>

using namespace std;

namespace tse
{

NodeCode MapNode::CATCHALL = "ZZZ";

MapNode::MapNode() : _id(0), _type(0), _tag(0), _next(0), _alt(0), _reverse(false) {}

MapNode::MapNode(const MapNode& node)
  : _id(node._id),
    _type(node._type),
    _tag(node._tag),
    _next(node._next),
    _alt(node._alt),
    _reverse(node._reverse),
    _code(node._code),
    _zone2nations(node._zone2nations)
// FIXME: is it ok here that we don't copy other members?
{
}

MapNode::MapNode(PricingTrx& trx, const RoutingMap& routeRecord)
  : _id(routeRecord.loc1No()),
    _type(routeRecord.loc1().locType()),
    _tag(routeRecord.loctag()),
    _next(routeRecord.nextLocNo()),
    _alt(routeRecord.altLocNo()),
    _reverse(false)
{
  _code.insert(routeRecord.loc1().loc());
  if (!routeRecord.nation().empty())
    _nations.insert(routeRecord.nation());

  if (UNLIKELY(trx.getOptions()->isRtw() && routeRecord.loc1().locType() == ZONE))
  {
    Zone zone = routeRecord.loc1().loc();
    LocUtil::padZoneNo(zone);
    const ZoneInfo* zoneInfo = trx.dataHandle().getZone(routeRecord.vendor(), zone, RESERVED, 
        trx.ticketingDate());
    if(zoneInfo == nullptr)
      return;

    for (const std::vector<ZoneInfo::ZoneSeg>& zoneSegs : zoneInfo->sets())
    {
      for (const ZoneInfo::ZoneSeg& zoneSeg : zoneSegs)
      {
        if (zoneSeg.locType() == LOCTYPE_NATION)
          _zone2nations.insert(zoneSeg.loc());
      }
    }
  }
}

MapNode::~MapNode() {}

bool
MapNode::contains(const NodeCode& code) const
{ // lint -e{530}
  return (_code.find(code) != _code.end() || _code.find(CATCHALL) != _code.end());
}

bool
MapNode::hasNation(const NationCode& nation) const
{
  return (_nations.find(nation) != _nations.end());
}

bool
MapNode::hasNationInZone(const NationCode& nation) const
{
  return _zone2nations.find(nation) != _zone2nations.end();
}

void
MapNode::printNodeElements(
    ostream& stream, const std::string& before, const std::string& after) const
{
  stream << before;

  set<tse::NodeCode>::const_iterator i = _code.begin();
  const std::string prefix = (_type == NATION) ? "*" : "";

  if (LIKELY(i != _code.end()))
  {
    stream << prefix << *i;
    ++i;
  }

  while (i != _code.end())
    stream << "/" << prefix << *i++;

  stream << after;
}

ostream& operator<<(ostream& stream, const MapNode& node)
{
  const set<tse::NodeCode>& code = node.code();
  set<tse::NodeCode>::const_iterator i = code.begin();
  while (i != code.end())
  {
    stream << *i;
    if (++i != code.end())
      stream << "/";
  }

  return stream;
}

} // namespace tse
