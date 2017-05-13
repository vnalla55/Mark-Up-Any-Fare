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

#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <boost/noncopyable.hpp>
#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include "KeyedFactory.h"
#include "CacheMemoryDiagnostic.h"

#include <mutex>
#include <thread>

namespace sfc
{
  template <typename Key, typename Type, typename FactoryType = KeyedFactory<Key,Type>> class Cache;
}
namespace tse
{
class BaseTrx;
class CacheDeleterBase;
class TrxCounter
{
public:
  static void registerDeleter(CacheDeleterBase* deleter);
  static void unregisterDeleter(CacheDeleterBase* deleter);
  static void appendTrx(BaseTrx* trx);
  static void removeTrx(const BaseTrx* trx);
  static const std::set<boost::int64_t>& getActiveTrxs();

  static std::set<CacheDeleterBase*> _observers;
  static std::set<boost::int64_t> _activeTrxs;
  static std::mutex _observersMutex;
  static std::mutex _activeTrxMutex;

  static bool _fallbackCacheDeleter;
private:
  TrxCounter();
};
class CacheDeleterBase : boost::noncopyable
{
public:
  virtual ~CacheDeleterBase();
  virtual void notify(boost::int64_t id) = 0;
  virtual void clearAccumulator() = 0;

  static void emptyTrash();
protected:
  CacheDeleterBase();
};
template<typename Key, typename Type> class TrashDeleter
{
public:
  TrashDeleter(sfc::KeyedFactory<Key, Type>& factory)
    : _factory(factory)
  {
  }
  void operator () (Type* ptr) const
  {
    try
    {
      static Key dummy;
      _factory.destroy(dummy, ptr);
    }
    catch (...)
    {}
  }
private:
  sfc::KeyedFactory<Key, Type>& _factory;
};
template<typename Key, typename Type> class CacheDeleter : public CacheDeleterBase
{
  typedef std::vector<Type*> TrashBin;
  class CacheDeleterUnit : boost::noncopyable
  {
  public:
    CacheDeleterUnit(TrashDeleter<Key, Type>& deleter,
                     TrashBin& trash)
      : _activeTrxs(TrxCounter::getActiveTrxs())
      , _deleter(deleter)
    {
      _trashBin.swap(trash);
    }
    ~CacheDeleterUnit()
    {
    }
    bool notify(boost::int64_t id)
    {
      _activeTrxs.erase(id);
      if (_activeTrxs.empty())
      {
        emptyTrash();
        return true;
      }
      return false;
    }
    void objectsIn(CacheMemoryDiagnostic::CacheDeleterUnitDiagnostic& unitDiagnostic)
    {
      unitDiagnostic._activeTrx = _activeTrxs;
      for (typename TrashBin::const_iterator it(_trashBin.begin()),
                                              itend(_trashBin.end());
            it != itend;
            ++it)
      {
        unitDiagnostic._numberObjects += (*it)->size();
      }
    }
    size_t numberActiveTrx()
    {
      return _activeTrxs.size();
    }
    void emptyTrash()
    {
#if 0
      static Type tp;
      time_t t;
      time(&t);
      struct tm *local = localtime(&t);
      char buffer[80];
      strftime(buffer, 80, "%D %T", local);
      std::cout << buffer
                << ','
                << typeid(tp).name()
                << ":emptyTrash:_trashBin.size()="
                << _trashBin.size()
                << std::endl;
#endif// 0
      std::for_each(_trashBin.begin(), _trashBin.end(), _deleter);
      _trashBin.clear();
    }
  private:
    std::set<boost::int64_t> _activeTrxs;
    TrashBin _trashBin;
    TrashDeleter<Key, Type>& _deleter;
  };
public:
  CacheDeleter(sfc::KeyedFactory<Key, Type>& factory)
    : _deleter(factory)
    , _cache(0)
  {
  }
  virtual ~CacheDeleter()
  {
  }
  virtual void clearAccumulator()
  {
    if (_cache != 0)
    {
      _cache->emptyTrash();
    }
  }
  void setCachePtr(sfc::Cache<Key, Type>* cache)
  {
    _cache = cache;
  }
  virtual void notify(boost::int64_t id)
  {
    for (typename CacheDeleterUnits::iterator it(_units.begin());
         it != _units.end(); )
    {
      if ((*it)->notify(id))
      {
        delete *it;
        it = _units.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
  void objectsIn(CacheMemoryDiagnostic& data)
  {
    std::unique_lock<std::mutex> lock(TrxCounter::_activeTrxMutex);
    data._activeTrx = TrxCounter::getActiveTrxs();
    for (typename CacheDeleterUnits::const_iterator it(_units.begin()),
                                                    itend(_units.end());
          it != itend;
          ++it)
    {
      CacheMemoryDiagnostic::CacheDeleterUnitDiagnostic unitData;
      (*it)->objectsIn(unitData);
      data._units.push_back(unitData);
    }
  }
  void moveToTrashBin(TrashBin& trash)
  {
    if (TrxCounter::_fallbackCacheDeleter)
    {
      std::unique_lock<std::mutex> lock(TrxCounter::_activeTrxMutex);
      _units.push_back(new CacheDeleterUnit(_deleter, trash));
    }
    else// new code
    {
      TrashBin tmp;
      {
        std::unique_lock<std::mutex> lock(TrxCounter::_activeTrxMutex);
        if (TrxCounter::getActiveTrxs().empty())
        {
          tmp.swap(trash);
        }
        else
        {
          _units.push_back(new CacheDeleterUnit(_deleter, trash));
        }
      }
      if (!tmp.empty())
      {
        std::for_each(tmp.begin(), tmp.end(), _deleter);
      }
    }
  }
private:
  TrashDeleter<Key, Type> _deleter;
  sfc::Cache<Key, Type>* _cache;
  typedef std::list<CacheDeleterUnit*> CacheDeleterUnits;
  CacheDeleterUnits _units;
};
}
