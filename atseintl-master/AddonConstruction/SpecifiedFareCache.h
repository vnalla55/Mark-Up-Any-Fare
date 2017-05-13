#pragma once

#include <shared_mutex>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


namespace tse
{

class AddonCombFareClassInfo;
class FareInfo;

typedef std::vector<AddonCombFareClassInfo*> AddonFareClasses;


struct SpecifiedFare
{
  const FareInfo* _specFare = nullptr;
  const AddonFareClasses* _origAddonFareClasses = nullptr;
  const AddonFareClasses* _destAddonFareClasses = nullptr;

  SpecifiedFare() = default;
  SpecifiedFare(const FareInfo* fare);
};


typedef std::vector<SpecifiedFare> SpecifiedFareList;


class SpecifiedFareCache
{
public:
  void add(const LocCode& gateway1,
           const LocCode& gateway2,
           const CarrierCode& carrier,
           const VendorCode& vendor,
           SpecifiedFareList* fares);

  SpecifiedFareList* get(const LocCode& gateway1,
                         const LocCode& gateway2,
                         const CarrierCode& carrier,
                         const VendorCode& vendor) const;

private:
  typedef std::tuple<LocCode, LocCode, CarrierCode, VendorCode> CacheKey;
  typedef std::unordered_map<CacheKey, SpecifiedFareList*, boost::hash<CacheKey> > Cache;
  typedef std::shared_timed_mutex CacheMutex;

  // SpecifiedFareList pointers are assumed to be managed by transaction-scoped 
  // DataHandle - hence they are not deleted on cache destruction.
  Cache _cache;
  mutable CacheMutex _mutex;
};

}

