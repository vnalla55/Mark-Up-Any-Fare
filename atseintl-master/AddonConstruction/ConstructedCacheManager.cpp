/**
 * @file     ConstructedCacheManager.cpp
 * @date     02/16/2005
 * @author   Konstantin Sidorin, Vadim Nikushin
 *
 * @brief     Implementations for Constructed Fares Cache Manager.
 *
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#include "AddonConstruction/ConstructedCacheManager.h"

#include "AddonConstruction/ConstructionDefs.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "DataModel/CacheTrx.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/SITAConstructedFareInfo.h"

#include <iostream>

namespace tse
{

ACKeyedFactory ConstructedCacheManager::_acKeyedFactory;
ACLRUCache
ConstructedCacheManager::_aclruCache(ConstructedCacheManager::_acKeyedFactory);

ConstructedCacheManager* ConstructedCacheManager::_instance = nullptr;

log4cxx::LoggerPtr
ConstructedCacheManager::_logger(
    log4cxx::Logger::getLogger("atseintl.AddonConstruction.CacheManager"));

namespace
{
ConfigurableValue<bool>
constructedFareCache("ADDON_CONSTRUCTION", "CONSTRUCTED_FARE_CACHE");
ConfigurableValue<size_t>
constructedFareCacheSize("ADDON_CONSTRUCTION", "CONSTRUCTED_FARE_CACHE_SIZE", ADDON_CACHE_SIZE);
}

ConstructedCacheManager::ConstructedCacheManager() : _useCache(constructedFareCache.getValue())
{
  _aclruCache.reserve(constructedFareCacheSize.getValue());
}

ConstructedCacheManager&
ConstructedCacheManager::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    const MallocContextDisabler disableCustomAllocator;
    _instance = new ConstructedCacheManager;
  }

  return *_instance;
}

void
ConstructedCacheManager::flushAll()
{
  LOG4CXX_DEBUG(_logger, "ConstructedCacheManager::flushAll called");
  _aclruCache.flush();
}

void
ConstructedCacheManager::flushVendorCxrFares(const VendorCode& vendor, const CarrierCode& carrier)
{
  LOG4CXX_INFO(_logger, "ConstructedCacheManager::flushVendorCxrFares called");

  std::shared_ptr<std::vector<CacheKey>> cacheKeys = _aclruCache.keys();

  std::vector<CacheKey> keys = *cacheKeys;

  for (const auto cacheKey : keys)
  {
    if (cacheKey._c == vendor && cacheKey._d == carrier)
    {
      _aclruCache.invalidate(cacheKey);

      LOG4CXX_INFO(_logger, "CacheKey: " << cacheKey.toString());
    }
  }
}

void
ConstructedCacheManager::flushSpecifiedFares(const LocCode& market1,
                                             const LocCode& market2,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier)
{
  LOG4CXX_INFO(_logger, "ConstructedCacheManager::flushSpecifiedFares called");

  std::vector<CacheKey>* flushVec = nullptr;

  boost::lock_guard<boost::mutex> g(_acKeyedFactory.factoryMutex());

  FlushKey flushKey(vendor, carrier, market1, market2, SPECIFIED_FLUSH_IND);
  FlushMapIter iter = _acKeyedFactory.flushMap().find(flushKey);

  if (iter != _acKeyedFactory.flushMap().end())
    flushVec = iter->second;

  if (flushVec)
  {
    LOG4CXX_INFO(_logger,
                 "flushSpecifiedFares: Key to flush exists: CARRIER="
                     << carrier << ", VENDOR=" << vendor << ", MARKET1=" << market1
                     << ", MARKET2=" << market2);

    std::ostringstream logGatewaysStr;

    for (const auto cacheKey : *flushVec)
    {
      std::shared_ptr<ConstructedCacheDataWrapper> cacheWrapper =
          _aclruCache.getIfResident(cacheKey);

      if (cacheWrapper)
      {
        CacheGatewayPairVec::iterator gatewaysCacheIt = cacheWrapper->gateways().begin();

        for (; gatewaysCacheIt != cacheWrapper->gateways().end(); ++gatewaysCacheIt)
        {
          std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;

          if ((gatewayPair->gateway1() == market1 && gatewayPair->gateway2() == market2) ||
              (gatewayPair->gateway1() == market2 && gatewayPair->gateway2() == market1) ||
              (gatewayPair->gateway1() == market1 && gatewayPair->multiCity2() == market2) ||
              (gatewayPair->multiCity1() == market1 && gatewayPair->gateway2() == market2) ||
              (gatewayPair->multiCity1() == market1 && gatewayPair->multiCity2() == market2) ||
              (gatewayPair->gateway1() == market2 && gatewayPair->multiCity2() == market1) ||
              (gatewayPair->multiCity1() == market2 && gatewayPair->gateway2() == market1) ||
              (gatewayPair->multiCity1() == market2 && gatewayPair->multiCity2() == market1))
          {
            gatewayPair->needsReconstruction() = true;

            if (_logger->isInfoEnabled())
            {
              logGatewaysStr << gatewayPair->gateway1() << "-" << gatewayPair->gateway2() << " ";
            }
          }
        }
      }
      LOG4CXX_INFO(_logger,
                   "CacheKey: " << cacheKey.toString()
                                << " GateWayPairs: " << logGatewaysStr.str());
    }
  }
}

void
ConstructedCacheManager::flushAddonFares(const LocCode& interiorMarket,
                                         const LocCode& gatewayMarket,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier)
{
  LOG4CXX_INFO(_logger, "ConstructedCacheManager::flushAddonFares called");

  std::vector<CacheKey>* flushVec = nullptr;

  boost::lock_guard<boost::mutex> g(_acKeyedFactory.factoryMutex());

  FlushKey flushKey(vendor, carrier, interiorMarket, gatewayMarket, ADDON_FLUSH_IND);

  FlushMapIter iter = _acKeyedFactory.flushMap().find(flushKey);

  if (iter != _acKeyedFactory.flushMap().end())
    flushVec = iter->second;

  if (flushVec)
  {
    LOG4CXX_INFO(_logger,
                 "flushAddonFares: Key to flush exists: CARRIER="
                     << carrier << ", VENDOR=" << vendor << ", CARRIER=" << carrier
                     << ", INTERIORMARKET=" << interiorMarket
                     << ", GATEWAYMARKET=" << gatewayMarket);
    for (const auto cacheKey : *flushVec)
    {
      // LRU Cache Key is:
      //_a is Origin
      //_b is Destination
      //_c is Carrier
      //_d is Vendor
      //

      std::shared_ptr<ConstructedCacheDataWrapper> cacheWrapper =
          _aclruCache.getIfResident(cacheKey);

      if (cacheWrapper)
      {
        std::ostringstream logGatewaysStr;

        CacheGatewayPairVec::iterator gatewaysCacheIt = cacheWrapper->gateways().begin();

        for (; gatewaysCacheIt != cacheWrapper->gateways().end(); ++gatewaysCacheIt)
        {
          std::shared_ptr<GatewayPair> gatewayPair = *gatewaysCacheIt;

          if (cacheKey._a == interiorMarket)
          {
            if (gatewayPair->isGw1ConstructPoint())
            {
              if (gatewayPair->gateway1() == gatewayMarket ||
                  gatewayPair->multiCity1() == gatewayMarket)
              {
                gatewayPair->needsReconstruction() = true;

                if (_logger->isInfoEnabled())
                {
                  logGatewaysStr << gatewayPair->gateway1() << "-" << gatewayPair->gateway2()
                                 << " ";
                }
              }
            }
          }
          else if (cacheKey._b == interiorMarket)
          {
            if (gatewayPair->isGw2ConstructPoint())
            {
              if (gatewayPair->gateway2() == gatewayMarket ||
                  gatewayPair->multiCity2() == gatewayMarket)
              {
                gatewayPair->needsReconstruction() = true;

                if (_logger->isInfoEnabled())
                {
                  logGatewaysStr << gatewayPair->gateway1() << "-" << gatewayPair->gateway2()
                                 << " ";
                }
              }
            }
          }
        }
        LOG4CXX_INFO(_logger,
                     "CacheKey: " << cacheKey.toString()
                                  << " GateWayPairs: " << logGatewaysStr.str());
      }
    }
  }
}

size_t
ConstructedCacheManager::size()
{
  return _aclruCache.size();
}
}
