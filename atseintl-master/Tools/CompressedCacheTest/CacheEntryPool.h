//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <boost/unordered_set.hpp>

#include <mutex>

namespace tse
{

class CacheEntryPool
{
public:
  typedef boost::unordered_set<void*> PoolSet;

  char* createEntry(size_t size)
  {
    char* entry(new char[size]);
    std::unique_lock<std::mutex> lock(_mutex);
    _poolSet.insert(entry);
    return entry;
  }

  void removeEntry(void* ptr)
  {
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _poolSet.erase(ptr);
    }
    delete [] static_cast<char*>(ptr);
  }

  void* entry(void* ptr)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_poolSet.find(ptr) != _poolSet.end())
    {
      return ptr;
    }
    return 0;
  }

  static CacheEntryPool& instance()
  {
    static CacheEntryPool instance;
    return instance;
  }
private:
  PoolSet _poolSet;
  std::mutex _mutex;
};

}// tse
