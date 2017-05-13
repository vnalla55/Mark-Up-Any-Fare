//-----------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-----------------------------------------------------------------------------
//

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/HashKey.h"
#include "Routing/SpecifiedRouting.h"

namespace tse
{
class SpecifiedRoutingKey
{
public:
  SpecifiedRoutingKey() : _routing(nullptr), _mergeNationZone(false) {};
  SpecifiedRoutingKey(const Routing& routing, const DateTime& travelDate, bool mergeNationZone = false)
    : _routing(&routing),
      _travelDate(travelDate),
      _mergeNationZone(mergeNationZone),
      _key(routing.vendor(),
           routing.carrier(),
           routing.routingTariff(),
           routing.routing(),
           travelDate.toSimpleString(),
           mergeNationZone) {};

  friend bool operator<(const SpecifiedRoutingKey& key, const SpecifiedRoutingKey& key2)
  {
    return key._key < key2._key;
  }

  friend bool operator==(const SpecifiedRoutingKey& key, const SpecifiedRoutingKey& key2)
  {
    return key._key == key2._key;
  }

  const Routing& routing() const
  {
    return *_routing;
  };

  const DateTime& travelDate() const
  {
    return _travelDate;
  };

  const bool& mergeNationZone() const
  {
    return _mergeNationZone;
  };

private:
  const Routing* _routing;
  DateTime _travelDate;
  bool _mergeNationZone;

  typedef HashKey<VendorCode, CarrierCode, TariffNumber, RoutingNumber, std::string, bool>
  SpecifiedRoutingKeyInternal;

  SpecifiedRoutingKeyInternal _key;
};


class SpecifiedRoutingCache
{
public:
  // lock the mutex and check the pointer to specifiedroutingcache if its null, if null initialize
  // it and
  // get the pointer to specifiedrouting.
  static SpecifiedRouting& getSpecifiedRouting(PricingTrx& trx, const SpecifiedRoutingKey& key);
  static SpecifiedRouting*
  getSpecifiedRoutingReverse(PricingTrx& trx, const SpecifiedRoutingKey& key);

private:
  SpecifiedRouting& get(const SpecifiedRoutingKey& key, PricingTrx& trx);
  SpecifiedRouting* getReverse(const SpecifiedRoutingKey& key, PricingTrx& trx);
  SpecifiedRouting* create(const SpecifiedRoutingKey& key, PricingTrx& trx);
  SpecifiedRouting* createReverse(const SpecifiedRoutingKey& key, PricingTrx& trx);

  struct CacheItem
  {
    CacheItem() : routing(nullptr), reverseRouting(nullptr), reverseRoutingValid(true) {}
    SpecifiedRouting* routing;
    SpecifiedRouting* reverseRouting;
    bool reverseRoutingValid;
  };

  std::map<SpecifiedRoutingKey, CacheItem> _mapCache;
};

} // namespace tse

