//-----------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-----------------------------------------------------------------------------
//

#include "Routing/SpecifiedRoutingCache.h"

#include "DataModel/PricingTrx.h"
#include "Routing/SpecifiedRouting.h"

namespace tse
{
SpecifiedRouting&
SpecifiedRoutingCache::get(const SpecifiedRoutingKey& key, PricingTrx& trx)
{
  SpecifiedRouting*& specifiedRouting = _mapCache[key].routing;
  if (specifiedRouting == nullptr)
  {
    specifiedRouting = create(key, trx);
  }

  return *specifiedRouting;
}

SpecifiedRouting*
SpecifiedRoutingCache::getReverse(const SpecifiedRoutingKey& key, PricingTrx& trx)
{
  CacheItem& item = _mapCache[key];

  if (UNLIKELY(item.reverseRoutingValid == false))
  {
    return nullptr;
  }

  if (item.reverseRouting == nullptr)
  {
    item.reverseRouting = createReverse(key, trx);
    if (UNLIKELY(item.reverseRouting == nullptr))
    {
      item.reverseRoutingValid = false;
    }
  }

  return item.reverseRouting;
}

namespace
{

SpecifiedRoutingCache&
getCache(PricingTrx& trx)
{
  SpecifiedRoutingCache*& specifiedRoutingCache = trx.specifiedRoutingCache();
  if (specifiedRoutingCache == nullptr)
  {
    specifiedRoutingCache = trx.dataHandle().create<SpecifiedRoutingCache>();
  }

  return *specifiedRoutingCache;
}
}

SpecifiedRouting&
SpecifiedRoutingCache::getSpecifiedRouting(PricingTrx& trx, const SpecifiedRoutingKey& key)
{
  boost::lock_guard<boost::mutex> g(trx.mutexRoutingCache());
  return getCache(trx).get(key, trx);
}

SpecifiedRouting*
SpecifiedRoutingCache::getSpecifiedRoutingReverse(PricingTrx& trx, const SpecifiedRoutingKey& key)
{
  boost::lock_guard<boost::mutex> g(trx.mutexRoutingCache());
  return getCache(trx).getReverse(key, trx);
}

SpecifiedRouting*
SpecifiedRoutingCache::create(const SpecifiedRoutingKey& key, PricingTrx& trx)
{
  SpecifiedRouting* sr;
  if(key.mergeNationZone())
    sr = trx.dataHandle().create<SpecifiedRoutingDiag>();
  else
    sr = trx.dataHandle().create<SpecifiedRouting>();
  sr->initialize(key.routing(), trx);
  return sr;
}

SpecifiedRouting*
SpecifiedRoutingCache::createReverse(const SpecifiedRoutingKey& key, PricingTrx& trx)
{
  SpecifiedRouting* rev = trx.dataHandle().create<SpecifiedRouting>();
  SpecifiedRouting& sr = get(key, trx);
  *rev = sr;
  bool res = rev->reverseMap(trx);
  if (LIKELY(res))
  {
    return rev;
  }
  else
  {
    return nullptr;
  }
}

} // namespace tse

