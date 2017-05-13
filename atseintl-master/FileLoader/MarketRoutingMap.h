//----------------------------------------------------------------------------
//
//  File:           MarketRoutingMap.h
//  Created:        09 Jul 2008
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/MarketRoutingInfo.h"

#include <tr1/unordered_map>

namespace tse
{
struct market_routing_map_hash_func
{
  size_t operator()(const MarketRoutingKey& key) const
  {
    tse::Hasher hasher(tse::Global::hasherMethod());
    hasher << key;
    return hasher.hash();
  }
};

typedef std::tr1::unordered_map<MarketRoutingKey,
                                std::vector<MarketRoutingSinglesVec*>,
                                market_routing_map_hash_func> MarketRoutingSinglesMap;

typedef std::tr1::unordered_map<MarketRoutingKey,
                                std::vector<MarketRoutingDoublesVec*>,
                                market_routing_map_hash_func> MarketRoutingDoublesMap;

} // end namespace

