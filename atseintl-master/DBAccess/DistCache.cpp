#include "DBAccess/DistCache.h"

#include "Common/Logger.h"

#include <cstdio>
#include <cstring>
#include <iostream>


namespace
{
log4cxx::LoggerPtr
getLogger()
{
  static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DistCache"));
  return logger;
}
}

const int
FAILURE_RETRIES(0);
using namespace tse;

#define DEBUG_MESSAGE(context, success, rest)                                                      \
  LOG4CXX_DEBUG(getLogger(), context << (success ? "succeeded" : "failed") << rest);

DistCache::DistCache(bool nonblocking, bool nodelay, uint32_t distribution)
  : _memc(nullptr), rc(MEMCACHED_SUCCESS)
{
  _memc = memcached_create(nullptr);

  memcached_behavior_set(_memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, distribution);
  memcached_behavior_set(_memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, FAILURE_RETRIES);
  memcached_behavior_set(_memc, MEMCACHED_BEHAVIOR_TCP_NODELAY, nodelay ? 1 : 0);
  memcached_behavior_set(_memc, MEMCACHED_BEHAVIOR_NO_BLOCK, nonblocking ? 1 : 0);
}

DistCache::~DistCache() { memcached_free(_memc); }

bool
DistCache::addServers(const char* serverList)
{
  char* mutable_server_list(strdup(serverList));
  if (!mutable_server_list)
  {
    bool success = false;
    LOG4CXX_ERROR(getLogger(), "failed allocation for server list");
    return success;
  }
  memcached_server_st* servers = memcached_servers_parse(mutable_server_list);
  if (servers)
  {
    rc = memcached_server_push(_memc, servers);
  }
  bool success(rc == MEMCACHED_SUCCESS);
  DEBUG_MESSAGE("DistCache::addServers", success, " connect to serverlist[" << serverList << "]");
  memcached_server_list_free(servers);
  free(mutable_server_list);
  return success;
}

unsigned int
DistCache::serverCount()
{
  return (memcached_server_count(_memc));
}

bool
DistCache::set(
    const char* key, size_t keyLength, const char* value, size_t valueLength, time_t timeToLive)
{
  rc = MEMCACHED_FAILURE;
  bool success(false);
  if (valueLength < MEMCACHE_MAX_BLOB_BYTES)
  {
    rc = memcached_set(_memc, key, keyLength, value, valueLength, timeToLive, (uint32_t)0);

    success = (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
    if (!success)
    {
      LOG4CXX_ERROR(getLogger(), "DistCache::set:" << memcached_strerror(_memc, rc));
    }
    if (IS_DEBUG_ENABLED(getLogger()))
    {
      char buffer[MEMCACHED_MAX_KEY];
      memset(buffer, 0, MEMCACHED_MAX_KEY);
      strncpy(buffer, key, keyLength);
      DEBUG_MESSAGE("DistCache::set ", success, " with key[" << buffer << "]");
    }
  }
  else
  {
    LOG4CXX_INFO(getLogger(), "DistCache::set:too big valueLength=" << valueLength);
  }
  return success;
}

bool
DistCache::setWithKey(char* mKey,
                      size_t mKeyLength,
                      char* key,
                      size_t keyLength,
                      char* value,
                      size_t valueLength,
                      time_t timeToLive)
{
  int attempt = 0;
  while ((rc != MEMCACHED_SUCCESS && rc != MEMCACHED_BUFFERED) && attempt <= FAILURE_RETRIES)
  {
    rc = memcached_set_by_key(
        _memc, mKey, mKeyLength, key, keyLength, value, valueLength, timeToLive, 0);
    attempt++;
  }
  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (IS_DEBUG_ENABLED(getLogger()))
  {
    char buffer[MEMCACHED_MAX_KEY];
    memset(buffer, 0, MEMCACHED_MAX_KEY);
    strncpy(buffer, key, keyLength);
    DEBUG_MESSAGE("DistCache::setWithKey ", success, " with key[" << buffer << "]");
  }
  return success;
}

bool
DistCache::get(const char* key, size_t keyLength, char** value, size_t* valueLength)
{
  uint32_t flags;
  *value = memcached_get(_memc, key, keyLength, valueLength, &flags, &rc);

  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (!success)
  {
    LOG4CXX_DEBUG(getLogger(), "DistCache::set:" << memcached_strerror(_memc, rc));
  }
  if (IS_DEBUG_ENABLED(getLogger()))
  {
    char buffer[MEMCACHED_MAX_KEY];
    memset(buffer, 0, MEMCACHED_MAX_KEY);
    strncpy(buffer, key, keyLength);
    DEBUG_MESSAGE("DistCache::get ", success, " with key[" << buffer << "]");
  }
  return success;
}

bool
DistCache::getMultiple(const char** keys, size_t* keyLength, unsigned int numKeys)
{
  rc = memcached_mget(_memc, const_cast<char**>(keys), keyLength, numKeys);
  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (IS_DEBUG_ENABLED(getLogger()))
    DEBUG_MESSAGE("DistCache::getMultiple ", success, " ");
  return success;
}

bool
DistCache::next(char* key, size_t& keyLength, std::string& text)
{
  bool rc(false);
  char* value(nullptr);
  size_t length(0);
  if ((rc = next(key, keyLength, value, length)))
    text = std::string(value, length);
  if (value)
    delete[] value;
  return rc;
}

bool
DistCache::next(char* key, size_t& keyLength, char*& value, size_t& length)
{
  uint32_t flags;
  value = memcached_fetch(_memc, key, &keyLength, &length, &flags, &rc);
  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (IS_DEBUG_ENABLED(getLogger()))
  {
    char buffer[MEMCACHED_MAX_KEY];
    memset(buffer, 0, MEMCACHED_MAX_KEY);
    strncpy(buffer, key, keyLength);
    DEBUG_MESSAGE("DistCache::next ", success, " with key[" << buffer << "]");
  }
  return success;
}

bool
DistCache::replace(const char* key, size_t keyLength, char* value, size_t length, time_t timeToLive)
{
  rc = memcached_replace(_memc, key, keyLength, value, length, timeToLive, (uint32_t)0);
  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (IS_DEBUG_ENABLED(getLogger()))
  {
    char buffer[MEMCACHED_MAX_KEY];
    memset(buffer, 0, MEMCACHED_MAX_KEY);
    strncpy(buffer, key, keyLength);
    DEBUG_MESSAGE("DistCache::replace ", success, " with key[" << buffer << "]");
  }
  return success;
}

bool
DistCache::remove(const char* key, time_t expiration)
{
  return remove(key, strlen(key), expiration);
}

bool
DistCache::remove(const char* key, size_t keyLength, time_t expiration)
{
  if (!keyLength)
  {
    LOG4CXX_ERROR(getLogger(), "key is empty");
    return false;
  }
  rc = memcached_delete(_memc, key, keyLength, expiration);
  bool success(rc == MEMCACHED_SUCCESS || rc == MEMCACHED_BUFFERED);
  if (!success)
  {
    LOG4CXX_DEBUG(getLogger(), "DistCache::set:" << memcached_strerror(_memc, rc));
  }
  if (IS_DEBUG_ENABLED(getLogger()))
  {
    char buffer[MEMCACHED_MAX_KEY];
    memset(buffer, 0, MEMCACHED_MAX_KEY);
    strncpy(buffer, key, keyLength);
    DEBUG_MESSAGE("DistCache::replace ", success, " with key[" << buffer << "]");
  }
  return success;
}
