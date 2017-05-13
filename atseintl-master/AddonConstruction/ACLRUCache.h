#pragma once
#include "AddonConstruction/ACKeyedFactory.h"
#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/SPLRUCache.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"

namespace tse
{
class ConstructionJob;

class ACLRUCache : public sfc::SPLRUCache<CacheKey, ConstructedCacheDataWrapper, ACKeyedFactory>
{
public:
  ACLRUCache(ACKeyedFactory& factory)
    : sfc::SPLRUCache<CacheKey, ConstructedCacheDataWrapper, ACKeyedFactory>(
          factory, "AddOnConstruction", ConstructedCacheDataWrapper::objectVersion())
  {
  }

  std::shared_ptr<ConstructedCacheDataWrapper> get(const CacheKey& key, ConstructionJob* cj)
  {
    return sfc::SPLRUCache<CacheKey, ConstructedCacheDataWrapper, ACKeyedFactory>::get(key, cj);
  }

  size_t invalidate(const CacheKey& key) override
  {
    return sfc::SPLRUCache<CacheKey, ConstructedCacheDataWrapper, ACKeyedFactory>::invalidate(key);
  }

  ACLRUCache* getCacheImpl() { return this; }
};
}

