#include "AddonConstruction/SpecifiedFareCache.h"

#include <mutex>


namespace tse
{

SpecifiedFare::SpecifiedFare(const FareInfo* fare)
: _specFare(fare)
{ }


void SpecifiedFareCache::add(const LocCode& gateway1,
                             const LocCode& gateway2,
                             const CarrierCode& carrier,
                             const VendorCode& vendor,
                             SpecifiedFareList* fares)
{
  std::unique_lock<CacheMutex> lock(_mutex);

  _cache.emplace(CacheKey(gateway1, gateway2, carrier, vendor), fares);
}


SpecifiedFareList* SpecifiedFareCache::get(const LocCode& gateway1,
                                           const LocCode& gateway2,
                                           const CarrierCode& carrier,
                                           const VendorCode& vendor) const
{
  std::shared_lock<CacheMutex> lock(_mutex);

  Cache::const_iterator i(_cache.find(CacheKey(gateway1, gateway2, carrier, vendor)));

  if (i != _cache.end())
    return i->second;

  return nullptr;
}

}

