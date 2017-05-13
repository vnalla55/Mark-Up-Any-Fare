//----------------------------------------------------------------------------
//
//  File:    DistCachePool.cpp
//
//  Description:

//     DistCachePool holds the set of connections and acts as a
//     resource acquisition lock.  The user acquires a lock and a
//     distributed cache connection by instantiatine a DistCachePool
//  As a meaningful name the object is typedefed as
//
// typedef tse::DistCachePool DistCacheScopedLockQueueMemberPtr;
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

// FIXME: <cstdio> should be in logger.h
#include "DBAccess/DistCachePool.h"

#include "Common/Logger.h"
#include "DBAccess/DistCache.h"

#include <cstdio>

namespace
{
log4cxx::LoggerPtr
getLogger()
{
  static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DistCache"));
  return logger;
}
}

namespace tse
{

// statics
ConcurrentDistCacheQueue DistCachePool::_concurrentQueue;
bool
DistCachePool::m_constructed(false);

DistCachePool::DistCachePool() : m_memcached(nullptr) { _concurrentQueue.wait_and_pop(m_memcached); }

DistCachePool::~DistCachePool()
{
  if (m_memcached != nullptr)
  {
    _concurrentQueue.push(m_memcached);
  }
}

bool
DistCachePool::isActive()
{
  return m_constructed;
}

const int
NUMAUXCONNECTIONS(5);

bool
DistCachePool::initialize(
    const std::string& serverList, uint32_t size, bool nodelay, bool nonblocking, int distribution)
{
  if (!m_constructed)
  {
    LOG4CXX_DEBUG(getLogger(), __FUNCTION__);
    for (int i = 0; i < NUMAUXCONNECTIONS; ++i)
    {
      DistCache* memcachecxx = new DistCache(nonblocking, nodelay, distribution);
      if (!memcachecxx->addServers(serverList.c_str()))
      {
        break;
      }
      _concurrentQueue.push(memcachecxx);
    }
    m_constructed =
        static_cast<int>(_concurrentQueue.size()) == NUMAUXCONNECTIONS &&
        MemcachedReadPool::initialize(serverList, size, nodelay, nonblocking, distribution);
  }

  return m_constructed;
}

// MemcachedReadPool
ConcurrentDistCacheQueue MemcachedReadPool::_concurrentQueue;

MemcachedReadPool::MemcachedReadPool() : _memcached(nullptr) { _concurrentQueue.try_pop(_memcached); }

MemcachedReadPool::~MemcachedReadPool() { release(); }

void
MemcachedReadPool::release()
{
  if (_memcached != nullptr)
  {
    _concurrentQueue.push(_memcached);
    _memcached = nullptr;
  }
}

bool
MemcachedReadPool::initialize(
    const std::string& serverList, unsigned size, bool nodelay, bool nonblocking, int distribution)
{
  for (unsigned i = 0; i < size; ++i)
  {
    DistCache* memcached = new DistCache(nonblocking, nodelay, distribution);
    if (!memcached->addServers(serverList.c_str()))
    {
      break;
    }
    _concurrentQueue.push(memcached);
  }
  size_t qsz(_concurrentQueue.size());
  if (qsz != size)
  {
    LOG4CXX_ERROR(getLogger(),
                  __FUNCTION__ << ":created " << qsz << " instances," << size << " requested");
  }
  return qsz == size;
}
} // tse
