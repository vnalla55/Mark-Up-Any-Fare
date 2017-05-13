#include "DBAccess/RemoteCache/ASIOClient/ClientManager.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Logger.h"
#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientPool.h"

#include <iostream>
#include <map>
#include <mutex>

namespace tse
{

namespace RemoteCache
{

namespace ClientManager
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.ClientManager");

struct PoolKey
{
  PoolKey(const std::string& host,
          const std::string& port)
    : _host(host)
    , _port(port)
  {
  }
  const std::string _host;
  const std::string _port;
};

bool operator < (const PoolKey& first,
                 const PoolKey& second)
{
  if (first._host < second._host)
    return true;
  if (first._host > second._host)
    return false;

  if (first._port < second._port)
    return true;
  if (first._port > second._port)
    return false;

  return false;
}

std::mutex mutex;

bool stopped(false);

typedef std::map<PoolKey, ClientPoolPtr> ClientPoolMap;

ClientPoolMap map;

ClientPoolPtr tryPool(const HostPort& hostPort)
{
  ClientPoolPtr pool;
  if (!(hostPort.empty() || hostPort._sameHost))
  {
    PoolKey key(hostPort._host, hostPort._port);
    auto res(map.emplace(key, ClientPoolPtr()));
    pool = res.first->second;
    if (!pool)
    {
      res.first->second.reset(new ClientPool(hostPort._host, hostPort._port));
      pool = res.first->second;
    }
    if (pool && !pool->isEnabled().first)
    {
      pool.reset();
    }
    if (pool && pool->needHealthcheck())
    {
      bool asyncHealthcheck(ReadConfig::getAsynchronousHealthcheck());
      if (!asyncHealthcheck)
      {
        std::string msg;
        if (!pool->healthcheck(msg))
        {
          pool.reset();
        }
        if (!msg.empty())
        {
          LOG4CXX_WARN(logger, "healthcheck:" << msg);
        }
      }
    }
    if (pool && !pool->healthcheckResult())
    {
      pool.reset();
    }
  }
  return pool;
}

ClientPoolPtr getPool(RCServerAttributes& serverAttributes)
{
  const MallocContextDisabler disableCustomAllocator;
  bool secondary(serverAttributes.isSecondary());
  const HostPort& hpPrimary(serverAttributes.primary());
  const HostPort& hpSecondary(serverAttributes.secondary());
  ClientPoolPtr pool;
  if (secondary)
  {
    if ((pool = tryPool(hpPrimary)))
    {
      serverAttributes.setPrimary();
      ReadConfig::resetClientPool(serverAttributes);
      return pool;
    }
    else if ((pool = tryPool(hpSecondary)))
    {
      return pool;
    }
  }
  else
  {
    if ((pool = tryPool(hpPrimary)))
    {
      return pool;
    }
    else if ((pool = tryPool(hpSecondary)))
    {
      serverAttributes.setSecondary();
      ReadConfig::resetClientPool(serverAttributes);
      return pool;
    }
  }
  return ClientPoolPtr();
}

}// namespace

bool enqueue(RCServerAttributes& serverAtrributes,
             RCRequestPtr request)
{
  const MallocContextDisabler disableCustomAllocator;
  std::unique_lock<std::mutex> lock(mutex);
  if (!stopped)
  {
    ClientPoolPtr pool(getPool(serverAtrributes));
    if (pool && request)
    {
      return pool->enqueue(request);
    }
  }
  return false;
}

void start()
{
  stopped = false;
}

void stop()
{
  const MallocContextDisabler disableCustomAllocator;
  std::unique_lock<std::mutex> lock(mutex);
  stopped = true;
  map.clear();
}

void healthcheck(const std::string& host,
                 const std::string& port,
                 std::string& msg)
{
  HostPort hostPort{host, port};
  if (hostPort.empty())
  {
    msg.assign("master host or port not specified");
  }
  else if (ReadConfig::sameHost(host))
  {
    msg.assign("master same as slave");
  }
  else
  {
    ClientPool pool(host, port);
    pool.healthcheck(msg);
  }
}

}// ClientManager

}// RemoteCache

}// tse
