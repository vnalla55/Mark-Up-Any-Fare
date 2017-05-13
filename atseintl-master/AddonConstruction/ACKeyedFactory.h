#pragma once

#include "AddonConstruction/AddonConstruction.h"
#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructionDefs.h"
#include "AddonConstruction/ConstructionJob.h"
#include "Common/KeyedFactory.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/HashKey.h"

#include <boost/thread/mutex.hpp>

#include <map>
#include <string>
#include <vector>

namespace tse
{

class AddonConstruction;

class ACKeyedFactory : public sfc::KeyedFactory<CacheKey, ConstructedCacheDataWrapper>
{
public:
  ACKeyedFactory() {}

  virtual ~ACKeyedFactory() {}

  ConstructedCacheDataWrapper* create(CacheKey key, ConstructionJob* cj);

  ConstructedCacheDataWrapper* create(CacheKey key) override;

  void destroy(CacheKey key, ConstructedCacheDataWrapper* cacheData) override;

  virtual ConstructedCacheDataWrapper*
  reCreate(const CacheKey& key, ConstructionJob* cj, ConstructedCacheDataWrapper* oldDataWrapper);

  virtual ConstructedCacheDataWrapper*
  reCreate(const CacheKey& key, ConstructedCacheDataWrapper* oldDataWrapper) override;

  virtual bool validate(CacheKey key, ConstructedCacheDataWrapper* object) override;

  virtual void logUnderConstruction(const CacheKey& key, ConstructedCacheDataWrapper* object);

  virtual void logCreation(const CacheKey& key, ConstructedCacheDataWrapper* object);

  virtual void logReCreation(const CacheKey& key, ConstructedCacheDataWrapper* object);

  virtual void logInvalidation(const CacheKey& key, ConstructedCacheDataWrapper* object);

  virtual void getGatewaysString(ConstructedCacheDataWrapper* object, std::ostringstream& oss);

  void buildFlushMapForSpecifiedGws(CacheKey& key,
                                    ConstructionJob* cj,
                                    std::shared_ptr<GatewayPair> gatewayPair);

  void buildSpecifiedGwFlushMapPair(CacheKey& key,
                                    ConstructionJob* cj,
                                    const LocCode& loc1,
                                    const LocCode& loc2);

  void buildFlushMap(CacheKey& key, FlushKey& flushKey);

  void cleanFlushMap(CacheKey& key);

  FlushMap& flushMap() { return _flushMap; }

  boost::mutex& factoryMutex() { return _factoryMutex; }

private:
  FlushMap _flushMap;

  boost::mutex _factoryMutex;
};
}

