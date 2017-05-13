//----------------------------------------------------------------------------
//
//  File:    DistCachePool.h
//
//  Description:

//     DistCachePool holds the set of connections and acts as a
//     resource acquisition lock.  The user acquires a lock and a
//     distributed cache connection by instantiating a DistCachePool
//
//  Copyright (c) Sabre 2008
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

#include "Common/ConcurrentQueue.h"

#include <stdexcept>

#include <libmemcached/memcached.h>

namespace tse
{
class DistCache;
typedef concurrent_queue<DistCache*> ConcurrentDistCacheQueue;

class DistCachePool
{
public:
  static bool initialize(const std::string& serverList,
                         uint32_t size = 5,
                         bool nodelay = false,
                         bool nonblocking = false,
                         int distribution = MEMCACHED_DISTRIBUTION_CONSISTENT);
  DistCachePool();
  ~DistCachePool();

  static bool isActive();

  operator bool() const { return m_memcached != nullptr; }

  DistCache* operator->() const
  {
    if (!m_memcached)
      throw std::runtime_error("Null Pointer: m_memcached");
    return m_memcached;
  }

  DistCache& operator*() const
  {
    if (!m_memcached)
      throw std::runtime_error("Null Pointer: m_memcached");
    return *m_memcached;
  }

private:
  mutable DistCache* m_memcached;

  static bool m_constructed;

  static ConcurrentDistCacheQueue _concurrentQueue;
  // not implemented
  DistCachePool& operator=(const DistCachePool&);
  DistCachePool(const DistCachePool&);
};

class MemcachedReadPool
{
public:
  MemcachedReadPool();
  ~MemcachedReadPool();

  operator bool() const { return _memcached != nullptr; }
  DistCache* operator->() { return _memcached; }
  void release();
  static bool initialize(const std::string& serverList,
                         unsigned size,
                         bool nodelay,
                         bool nonblocking,
                         int distribution);

private:
  DistCache* _memcached;
  static ConcurrentDistCacheQueue _concurrentQueue;
  // not implemented
  MemcachedReadPool(const MemcachedReadPool&);
  MemcachedReadPool& operator=(const MemcachedReadPool&);
};
}
typedef tse::DistCachePool DistCacheScopedLockQueueMemberPtr;

