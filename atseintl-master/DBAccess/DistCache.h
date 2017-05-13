//----------------------------------------------------------------------------
//
//  File:    DistCache.h
//
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

#include <string>

#include <libmemcached/memcached.h>

// just under 1Meg
const size_t MEMCACHE_MAX_BLOB_BYTES = (1024 * 1024 - 1000 - 1);

namespace tse
{
class DistCache
{
private:
  memcached_st* _memc;
  memcached_return rc;

public:
  DistCache(bool nonblocking = false,
            bool nodelay = false,
            uint32_t distribution = MEMCACHED_DISTRIBUTION_CONSISTENT);
  ~DistCache();
  bool addServers(const char* serverList);
  unsigned int connections() { return serverCount(); }
  unsigned int serverCount();
  bool
  set(const char* key, size_t keyLength, const char* value, size_t valueLength, time_t timeToLive);
  bool setWithKey(char* mKey,
                  size_t mKeyLength,
                  char* key,
                  size_t keyLength,
                  char* value,
                  size_t valueLength,
                  time_t timeToLive);
  bool get(const char* key, size_t keyLength, char** value, size_t* valueLength);
  bool getMultiple(const char** keys, size_t* keyLength, unsigned int numKeys);
  bool next(char* key, size_t& keyLength, std::string& text);
  bool next(char* key, size_t& keyLength, char*& value, size_t& length);
  bool remove(const char* key, size_t keyLength, time_t expiration);
  bool remove(const char* key, time_t expiration);
  bool replace(const char* key, size_t keyLength, char* value, size_t length, time_t timeToLive);
};
} // tse

