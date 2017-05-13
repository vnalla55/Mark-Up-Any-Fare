#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include <map>

namespace sfc
{

template <typename T>
class PointerDeleter : boost::noncopyable
{
  struct Entry
  {
    Entry() : _refCount(0), _delete(false) {}
    unsigned _refCount;
    bool _delete;
  };
  boost::mutex _mutex;
  typedef std::map<T*, Entry> Map;
  Map _inUse;

public:
  PointerDeleter() {}

  void addInUse(T* ptr)
  {
    boost::mutex::scoped_lock lock(_mutex);
    std::pair<typename Map::iterator, bool> result(_inUse.insert(std::make_pair(ptr, Entry())));
    ++result.first->second._refCount;
  }

  void removeInUse(T* ptr)
  {
    boost::mutex::scoped_lock lock(_mutex);
    typename Map::iterator mit(_inUse.find(ptr));
    if (LIKELY(mit != _inUse.end()))
    {
      --mit->second._refCount;
      if (LIKELY(0 == mit->second._refCount))
      {
        if (UNLIKELY(mit->second._delete))
        {
          delete ptr;
        }
        _inUse.erase(mit);
      }
    }
  }

  void del(T* ptr)
  {
    if (ptr != nullptr)
    {
      boost::mutex::scoped_lock lock(_mutex);
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

template <typename T>
struct DeleterFunc
{
  DeleterFunc(T* t, PointerDeleter<T>& deleter) : _deleter(deleter)
  {
    if (LIKELY(t != nullptr))
    {
      _deleter.addInUse(t);
    }
  }

  void operator()(T* t)
  {
    if (LIKELY(t != nullptr))
    {
      _deleter.removeInUse(t);
    }
  }
  PointerDeleter<T>& _deleter;
};

} // sfc

