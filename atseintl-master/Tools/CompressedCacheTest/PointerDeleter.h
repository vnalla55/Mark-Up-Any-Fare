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

#include <map>
#include <boost/noncopyable.hpp>

#include <mutex>

namespace sfc
{

template <typename T> class PointerDeleter : boost::noncopyable
{
  struct Entry
  {
    Entry()
      : _refCount(0)
      , _delete(false)
    {
    }
    unsigned _refCount;
    bool _delete;
  };
  std::mutex _mutex;
  typedef std::map<T*, Entry> Map;
  Map _inUse;
 public:
  PointerDeleter()
  {
  }

  void addInUse(T* ptr)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    std::pair<typename Map::iterator, bool> result(_inUse.insert(std::make_pair(ptr, Entry())));
    ++result.first->second._refCount;
  }

  void removeInUse(T* ptr)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    typename Map::iterator mit(_inUse.find(ptr));
    if (mit != _inUse.end())
    {
      --mit->second._refCount;
      if (0 == mit->second._refCount)
      {
        if (mit->second._delete)
        {
          delete ptr;
        }
        _inUse.erase(mit);
      }
    }
  }

  void del(T* ptr)
  {
    if (ptr != 0)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      typename Map::iterator mit(_inUse.find(ptr));
      if (mit != _inUse.end())
      {
        mit->second._delete = true;
      }
      else
      {
        delete ptr;
      }
    }
  }
};

template<typename T> struct DeleterFunc
{
  DeleterFunc(T* t,
              PointerDeleter<T>& deleter)
    : _deleter(deleter)
  {
    if (t != 0)
    {
      _deleter.addInUse(t);
    }
  }

  void operator()(T* t)
  {
    if (t != 0)
    {
      _deleter.removeInUse(t);
    }
  }
  PointerDeleter<T>& _deleter;
};

}// sfc
