//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "DBAccess/Cache.h"
#include "DBAccess/CompressedCache.h"
#include "DBAccess/DualMapCache.h"
#include "DBAccess/FIFOCache.h"
#include "DBAccess/GenericCache.h"
#include "DBAccess/LRUCache.h"
#include "DBAccess/MirrorCache.h"
#include "DBAccess/PerfectHash.h"
#include "DBAccess/SimpleCache.h"

#include <string>

namespace tse
{
static const char* CACHE_TYPE_LRU = "LRUCache";
static const char* CACHE_TYPE_SIMPLE = "SimpleCache";
static const char* CACHE_TYPE_GENERIC = "GenericCache";
static const char* CACHE_TYPE_FIFO = "FIFOCache";
static const char* CACHE_TYPE_DUAL = "DualMapCache";
static const char* CACHE_TYPE_MIRROR = "MirrorCache";
static const char* CACHE_TYPE_PERFECT = "PerfectCache";
static const char* CACHE_TYPE_COMPRESSED = "CompressedCache";

static const char* CACHE_NAME_UNDEFINED = "UNDEFINED";

template <typename Key, typename T>
class CacheFactory
{
public:
  static sfc::Cache<Key, T>* create(const std::string& cacheParams,
                                    sfc::KeyedFactory<Key, T>& factory,
                                    size_t capacity,
                                    size_t version)
  {
    sfc::Cache<Key, T>* retval;

    // Unfortunately, cache name cannot yet be determined unless we change ALL of the DAO
    // object constructors.  Therefore, it will be set post-construction by the caller.
    std::string cacheName(CACHE_NAME_UNDEFINED);

    //------------------------------------------------------
    // Cache parameters can consist of the cache type name
    // followed by a colon and additional options
    //------------------------------------------------------

    std::string::const_iterator colon = std::find(cacheParams.begin(), cacheParams.end(), ':');
    std::string cacheType(cacheParams.begin(), colon);
    std::string cacheArgs;
    if (colon != cacheParams.end())
    {
      cacheArgs.assign(colon + 1, cacheParams.end());
    }

    //------------------------------------------------------
    // Now create the appropriate cache object
    //------------------------------------------------------

    if (cacheType == CACHE_TYPE_LRU)
    {
      retval = new sfc::LRUCache<Key, T>(factory, cacheName, version, capacity);
    }
    else if (cacheType == CACHE_TYPE_SIMPLE)
    {
      retval = new sfc::SimpleCache<Key, T>(factory, cacheName, capacity, version);
    }
    else if (cacheType == CACHE_TYPE_GENERIC)
    {
      retval = new sfc::GenericCache<Key, T>(factory, cacheName, capacity, version);
    }
    else if (cacheType == CACHE_TYPE_FIFO)
    {
      retval = new sfc::FIFOCache<Key, T>(factory, cacheName, capacity, version);
    }
    else if (cacheType == CACHE_TYPE_DUAL)
    {
      retval = new sfc::DualMapCache<Key, T>(factory, cacheName, capacity, version);
    }
    else if (cacheType == CACHE_TYPE_MIRROR)
    {
      if (cacheArgs.empty())
      {
        retval = new MirrorCache<Key, T>(factory, cacheName, version);
      }
      else
      {
        retval = new MirrorCache<Key, T>(factory, cacheName, version, atoi(cacheArgs.c_str()));
      }
    }
    else if (cacheType == CACHE_TYPE_PERFECT)
    {
      retval = new typename PerfectHashGenerator<Key, T>::CacheType(
          factory, cacheName, capacity, version);
    }
    else if (cacheType == CACHE_TYPE_COMPRESSED)
    {
      retval = new sfc::CompressedCache<Key, T>(factory, cacheName, capacity, version);
    }
    else
    {
      // If cache type missing or invalid, default to one of the following:
      if (capacity == Global::getUnlimitedCacheSize())
      {
        if (CacheManager::useGenericCache())
        {
          retval = new sfc::GenericCache<Key, T>(factory, cacheName, capacity, version);
        }
        else
        {
          retval = new sfc::SimpleCache<Key, T>(factory, cacheName, capacity, version);
        }
      }
      else
      {
        retval = new sfc::LRUCache<Key, T>(factory, cacheName, version, capacity);
        // retval = new sfc::FIFOCache<Key,T>( factory, cacheName, capacity, version ) ;
      }
    }

    tse::ConfigMan& config = Global::config();
    int accumulatorSize(0);
    if (config.getValue("ACCUMULATOR_SIZE", accumulatorSize, "TSE_SERVER"))
    {
      retval->setAccumulatorSize(accumulatorSize);
    }

    return retval;
  }

private:
  CacheFactory(const CacheFactory& rhs);
  CacheFactory& operator=(const CacheFactory& rhs);

}; // class CacheFactory

} // namespace tse

