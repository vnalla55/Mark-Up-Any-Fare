// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/KeyedFactory.h"
#include "Common/TrxCounter.h"
#include "DataModel/BaseTrx.h"
#include "DBAccess/CacheMemoryDiagnostic.h"
#include "Util/FlatSet.h"

#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <algorithm>
#include <list>
#include <set>
#include <vector>

namespace sfc
{
template <typename Key, typename Type, typename FactoryType = KeyedFactory<Key, Type> >
class Cache;
}

namespace tse
{
class CacheDeleterBase : public TrxCounter::Observer
{
public:
  virtual ~CacheDeleterBase();

  static void emptyTrash();

protected:
  CacheDeleterBase();
};

template <typename Key, typename Type>
class TrashDeleter
{
public:
  TrashDeleter(sfc::KeyedFactory<Key, Type>& factory) : _factory(factory) {}
  void operator()(Type* ptr) const
  {
    try
    {
      static Key dummy;
      _factory.destroy(dummy, ptr);
    }
    catch (...) {}
  }

private:
  sfc::KeyedFactory<Key, Type>& _factory;
};

template <typename Key, typename Type>
class CacheDeleter : public CacheDeleterBase
{
  typedef std::vector<Type*> TrashBin;

  class CacheDeleterUnit : boost::noncopyable
  {
  public:
    CacheDeleterUnit(TrashDeleter<Key, Type>& deleter, TrashBin& trash)
      : _activeTrxs(TrxCounter::getActiveTrxsIds()), _deleter(deleter)
    {
      _trashBin.swap(trash);
    }

    ~CacheDeleterUnit() {}

    bool notify(int64_t id)
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
      for (typename TrashBin::const_iterator it(_trashBin.begin()), itend(_trashBin.end());
           it != itend;
           ++it)
      {
        unitDiagnostic._numberObjects += (*it)->size();
      }
    }

    size_t numberActiveTrx() { return _activeTrxs.size(); }

    void emptyTrash()
    {
      std::for_each(_trashBin.begin(), _trashBin.end(), _deleter);
      _trashBin.clear();
    }

  private:
    FlatSet<int64_t> _activeTrxs;
    TrashBin _trashBin;
    TrashDeleter<Key, Type>& _deleter;
  };

public:
  CacheDeleter(sfc::KeyedFactory<Key, Type>& factory) : _deleter(factory), _cache(nullptr) {}
  virtual ~CacheDeleter() {}

  void trxAfterDelete() override
  {
    if (_cache != nullptr)
    {
      _cache->emptyTrash();
    }
  }

  void setCachePtr(sfc::Cache<Key, Type>* cache) { _cache = cache; }

  void trxDeleted(BaseTrx& trx) override
  {
    const int64_t id = trx.getBaseIntId();

    for (typename CacheDeleterUnits::iterator it(_units.begin()); it != _units.end();)
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
    boost::lock_guard<boost::mutex> lock(TrxCounter::activeTrxMutex);
    data._activeTrx = TrxCounter::getActiveTrxsIds();
    for (typename CacheDeleterUnits::const_iterator it(_units.begin()), itend(_units.end());
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
    TrashBin tmp;
    {
      boost::lock_guard<boost::mutex> lock(TrxCounter::activeTrxMutex);
      if (TrxCounter::getActiveTrxsIds().empty())
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

private:
  TrashDeleter<Key, Type> _deleter;
  sfc::Cache<Key, Type>* _cache;
  typedef std::list<CacheDeleterUnit*> CacheDeleterUnits;
  CacheDeleterUnits _units;
};
}
