#include "AddonConstruction/ACKeyedFactory.h"

#include "Common/Logger.h"
#include "Util/BranchPrediction.h"

#include <boost/thread/lock_guard.hpp>

namespace tse
{
namespace
{
Logger
logger("atseintl.AddonConstruction.ACKeyedFactory");
}

ConstructedCacheDataWrapper*
ACKeyedFactory::create(CacheKey key, ConstructionJob* cj)
{
  ConstructedCacheDataWrapper* cacheWrapper = nullptr;

  if (cj)
  {
    cacheWrapper = new ConstructedCacheDataWrapper;

    AddonConstruction::runConstructionProcess(*cj, *cacheWrapper);

    boost::lock_guard<boost::mutex> g(_factoryMutex);

    CacheGatewayPairVec::iterator gatewaysCacheIt = cacheWrapper->gateways().begin();

    for (; gatewaysCacheIt != cacheWrapper->gateways().end(); ++gatewaysCacheIt)
    {
      std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;

      buildFlushMapForSpecifiedGws(key, cj, gatewayPair);

      if (gatewayPair->isGw1ConstructPoint())
      {
        FlushKey vnCxrIntMktGwMkt(cj->vendorCode(),
                                  cj->carrier(),
                                  cj->origin(),
                                  gatewayPair->gateway1(),
                                  ADDON_FLUSH_IND);

        buildFlushMap(key, vnCxrIntMktGwMkt);

        if (cj->origin() != cj->boardMultiCity())
        {
          FlushKey vnCxrIntMktGwMkt(cj->vendorCode(),
                                    cj->carrier(),
                                    cj->boardMultiCity(),
                                    gatewayPair->gateway1(),
                                    ADDON_FLUSH_IND);

          buildFlushMap(key, vnCxrIntMktGwMkt);
        }
      }

      if (gatewayPair->isGw2ConstructPoint())
      {
        FlushKey vnCxrIntMktGwMkt(cj->vendorCode(),
                                  cj->carrier(),
                                  cj->destination(),
                                  gatewayPair->gateway2(),
                                  ADDON_FLUSH_IND);

        buildFlushMap(key, vnCxrIntMktGwMkt);

        if (cj->destination() != cj->offMultiCity())
        {
          FlushKey vnCxrIntMktGwMkt(cj->vendorCode(),
                                    cj->carrier(),
                                    cj->offMultiCity(),
                                    gatewayPair->gateway1(),
                                    ADDON_FLUSH_IND);

          buildFlushMap(key, vnCxrIntMktGwMkt);
        }
      }
    }
  }

  logCreation(key, cacheWrapper);

  return cacheWrapper;
}

ConstructedCacheDataWrapper*
ACKeyedFactory::create(CacheKey key)
{
  ConstructedCacheDataWrapper* cacheWrapper = nullptr;

  logCreation(key, cacheWrapper);

  return cacheWrapper;
}

void
ACKeyedFactory::destroy(CacheKey key, ConstructedCacheDataWrapper* cacheData)
{
  cleanFlushMap(key);

  logInvalidation(key, cacheData);

  delete cacheData;
}

ConstructedCacheDataWrapper*
ACKeyedFactory::reCreate(const CacheKey& key,
                         ConstructionJob* cj,
                         ConstructedCacheDataWrapper* oldDataWrapper)
{
  ConstructedCacheDataWrapper* cacheWrapper = new ConstructedCacheDataWrapper;

  CacheGatewayPairVec::iterator gatewaysCacheIt = oldDataWrapper->gateways().begin();

  for (; gatewaysCacheIt != oldDataWrapper->gateways().end(); ++gatewaysCacheIt)
  {
    std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;

    if (!gatewayPair->needsReconstruction())
    {
      for (auto cf : oldDataWrapper->ccFares())
      {
        if (cf->gateway1() == gatewayPair->gateway1() && cf->gateway2() == gatewayPair->gateway2())
        {
          (cacheWrapper->ccFares()).push_back(std::move(cf));
        }
        else if (cf->gateway1() == gatewayPair->gateway2() &&
                 cf->gateway2() == gatewayPair->gateway1())
        {
          (cacheWrapper->ccFares()).push_back(std::move(cf));
        }
      }
    }

    (cacheWrapper->gateways()).push_back(gatewayPair);
  }

  AddonConstruction::runReconstructionProcess(*cj, *cacheWrapper);

  logReCreation(key, cacheWrapper);

  return cacheWrapper;
}

ConstructedCacheDataWrapper*
ACKeyedFactory::reCreate(const CacheKey& key, ConstructedCacheDataWrapper* oldDataWrapper)
{
  ConstructedCacheDataWrapper* cacheWrapper = nullptr;

  logReCreation(key, cacheWrapper);

  return cacheWrapper;
}

bool
ACKeyedFactory::validate(CacheKey key, ConstructedCacheDataWrapper* object)
{
  CacheGatewayPairVec::iterator gatewaysCacheIt = object->gateways().begin();

  for (; gatewaysCacheIt != object->gateways().end(); ++gatewaysCacheIt)
  {
    std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;

    if (gatewayPair->needsReconstruction())
      return false;
  }

  return true;
}

void
ACKeyedFactory::logUnderConstruction(const CacheKey& key, ConstructedCacheDataWrapper* object)
{
  if (UNLIKELY(logger->isDebugEnabled()))
  {
    std::ostringstream oss;
    oss << "Key to be recreated: " << key.toString() << " ";
    getGatewaysString(object, oss);

    LOG4CXX_DEBUG(logger, oss.str());
  }
}

void
ACKeyedFactory::logCreation(const CacheKey& key, ConstructedCacheDataWrapper* object)
{
  if (UNLIKELY(logger->isDebugEnabled()))
  {
    std::ostringstream oss;
    oss << "Key created: " << key.toString() << " ";
    getGatewaysString(object, oss);

    LOG4CXX_DEBUG(logger, oss.str());
  }
}

void
ACKeyedFactory::logReCreation(const CacheKey& key, ConstructedCacheDataWrapper* object)
{
  if (UNLIKELY(logger->isDebugEnabled()))
  {
    std::ostringstream oss;
    oss << "Key recreated: " << key.toString() << " ";
    getGatewaysString(object, oss);

    LOG4CXX_DEBUG(logger, oss.str());
  }
}

void
ACKeyedFactory::logInvalidation(const CacheKey& key, ConstructedCacheDataWrapper* object)
{
  if (UNLIKELY(logger->isDebugEnabled()))
  {
    std::ostringstream oss;
    oss << "Key invalidated: " << key.toString() << " ";
    getGatewaysString(object, oss);

    LOG4CXX_DEBUG(logger, oss.str());
  }
}

void
ACKeyedFactory::getGatewaysString(ConstructedCacheDataWrapper* object, std::ostringstream& oss)
{
  if (!object)
    return;

  CacheGatewayPairVec::iterator gatewaysCacheIt = object->gateways().begin();

  for (; gatewaysCacheIt != object->gateways().end(); ++gatewaysCacheIt)
  {
    std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;
    oss << "Gateway: " << gatewayPair->gateway1() << "-" << gatewayPair->gateway2() << ", "
        << gatewayPair->multiCity1() << "-" << gatewayPair->multiCity2();
  }
}

void
ACKeyedFactory::buildFlushMapForSpecifiedGws(CacheKey& key,
                                             ConstructionJob* cj,
                                             std::shared_ptr<GatewayPair> gatewayPair)
{

  buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->gateway1(), gatewayPair->gateway2());

  if (gatewayPair->gateway1() != gatewayPair->multiCity1() &&
      gatewayPair->gateway2() != gatewayPair->multiCity2())
  {
    buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->gateway1(), gatewayPair->multiCity2());

    buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->multiCity1(), gatewayPair->multiCity2());

    buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->multiCity1(), gatewayPair->gateway2());
  }
  else if (gatewayPair->gateway1() != gatewayPair->multiCity1() &&
           gatewayPair->gateway2() == gatewayPair->multiCity2())
  {
    buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->multiCity1(), gatewayPair->gateway2());
  }
  else if (gatewayPair->gateway1() == gatewayPair->multiCity1() &&
           gatewayPair->gateway2() != gatewayPair->multiCity2())
  {
    buildSpecifiedGwFlushMapPair(key, cj, gatewayPair->gateway1(), gatewayPair->multiCity2());
  }
}

void
ACKeyedFactory::buildSpecifiedGwFlushMapPair(CacheKey& key,
                                             ConstructionJob* cj,
                                             const LocCode& loc1,
                                             const LocCode& loc2)
{
  if (loc1 < loc2)
  {
    FlushKey vnCxrMktKey(cj->vendorCode(), cj->carrier(), loc1, loc2, SPECIFIED_FLUSH_IND);

    buildFlushMap(key, vnCxrMktKey);
  }
  else
  {
    FlushKey vnCxrMktKey(cj->vendorCode(), cj->carrier(), loc2, loc1, SPECIFIED_FLUSH_IND);

    buildFlushMap(key, vnCxrMktKey);
  }
}

void
ACKeyedFactory::buildFlushMap(CacheKey& key, FlushKey& flushKey)
{
  FlushMapIter iter = _flushMap.find(flushKey);

  if (iter == _flushMap.end())
  {
    FlushVec cacheKeyVec = new std::vector<CacheKey>;
    cacheKeyVec->push_back(key);

    _flushMap.insert(flushValueType(flushKey, cacheKeyVec));
  }
  else
  {
    FlushVec cacheKeyVec = iter->second;
    FlushVecIter flushVecIter = std::find(cacheKeyVec->begin(), cacheKeyVec->end(), key);

    if (flushVecIter == cacheKeyVec->end())
      cacheKeyVec->push_back(key);
  }
}

void
ACKeyedFactory::cleanFlushMap(CacheKey& key)
{
  boost::lock_guard<boost::mutex> g(_factoryMutex);

  FlushMapIter iter = _flushMap.begin();

  CarrierCode& carrier = key._c;
  VendorCode& vendor = key._d;

  for (; iter != _flushMap.end();)
  {
    if (iter->first._a == vendor && iter->first._b == carrier)
    {
      FlushVec flushVec = iter->second;

      if (flushVec)
      {
        FlushVecIter flushVecIter = std::find(flushVec->begin(), flushVec->end(), key);

        if (flushVecIter != flushVec->end())
          flushVec->erase(flushVecIter);

        if (flushVec->empty())
        {
          delete flushVec;
          _flushMap.erase(iter++);
          continue;
        }
      }
    }

    ++iter;
  }
}

} // tse end
