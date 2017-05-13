//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/DataAccessObjectBase.h"

namespace tse
{
template <typename T>
class IsEffective
{
public:
  IsEffective(const DateTime& startDate,
              const DateTime& endDate = DateTime::emptyDate(),
              const DateTime& ticketDate = DateTime::emptyDate(),
              bool isHistorical = false)
    : _startDateTime(startDate),
      _endDateTime(endDate),
      _ticketDate(ticketDate),
      _isHistorical(isHistorical)
  {
    init();
  }

  IsEffective(const DateTime& date, const DateTime& ticketDate, bool isHistorical)
    : _startDateTime(date), _endDateTime(date), _ticketDate(ticketDate), _isHistorical(isHistorical)
  {
    init();
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return false;
    if (_startDate > rec->discDate())
      return false;
    if (_ticketDate > rec->expireDate())
      return false;
    if (_isHistorical)
    {
      if (rec->createDate().date() > _ticketDateOnly && rec->effDate().date() > _ticketDateOnly)
        return false;
    }
    else
    {
      if (_startDateTime > rec->expireDate())
        return false;
    }

    return true;
  }

private:
  DateTime _startDateTime;
  DateTime _endDateTime;
  DateTime _startDate;
  DateTime _endDate;
  DateTime _ticketDate;
  boost::gregorian::date _ticketDateOnly;
  bool _isHistorical;

  void init()
  {
    if (_endDateTime.isEmptyDate())
      _endDateTime = _startDateTime;
    if (_ticketDate.isEmptyDate())
    {
      _ticketDate = DateTime::localTime();
      _isHistorical = false;
    }
    _startDate = _startDateTime.date();
    _endDate = _endDateTime.date();
    if (_isHistorical)
    {
      _ticketDateOnly = _ticketDate.date();
    }
  }
};

template <typename T>
class IsNotEffective
{
public:
  IsNotEffective(const DateTime& startDate,
                 const DateTime& endDate = DateTime::emptyDate(),
                 const DateTime& ticketDate = DateTime::emptyDate(),
                 bool isHistorical = false)
    : _startDateTime(startDate),
      _endDateTime(endDate),
      _ticketDate(ticketDate),
      _isHistorical(isHistorical)
  {
    init();
  }

  IsNotEffective(const DateTime& date, const DateTime& ticketDate, bool isHistorical)
    : _startDateTime(date), _endDateTime(date), _ticketDate(ticketDate), _isHistorical(isHistorical)
  {
    init();
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return true;
    if (_startDate > rec->discDate())
      return true;
    if (_ticketDate > rec->expireDate())
      return true;
    if (_isHistorical)
    {
      if (rec->createDate().date() > _ticketDateOnly && rec->effDate().date() > _ticketDateOnly)
        return true;
    }
    else if (_startDateTime > rec->expireDate())
    {
      return true;
    }
    return false;
  }

private:
  DateTime _startDateTime;
  DateTime _endDateTime;
  DateTime _startDate;
  DateTime _endDate;
  DateTime _ticketDate;
  boost::gregorian::date _ticketDateOnly;
  bool _isHistorical;

  void init()
  {
    if (_endDateTime.isEmptyDate())
      _endDateTime = _startDateTime;
    if (_ticketDate.isEmptyDate())
    {
      if (Global::allowHistorical())
        throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);

      _ticketDate = DateTime::localTime();
      _isHistorical = false;
    }
    _startDate = _startDateTime.date();
    _endDate = _endDateTime.date();
    if (_isHistorical)
    {
      _ticketDateOnly = _ticketDate.date();
    }
  }
};

template <typename T>
bool
Inhibit(const T* t)
{
  return (t->inhibit() != 'N');
}

template <typename T>
bool
NotInhibit(const T* t)
{
  return (t->inhibit() == 'N');
}

template <typename T>
bool
InhibitForFD(const T* t)
{
  return (t->inhibit() != 'N' && t->inhibit() != 'D');
}

template <typename T>
bool
NotInhibitForFD(const T* t)
{
  return (t->inhibit() == 'N' || t->inhibit() == 'D');
}

template <typename T>
void
get(T*& t, DeleteList& deleteList)
{
  t = static_cast<T*>(::malloc(sizeof(T)));
  new (t) T;

  deleteList.adoptPooled(t);
}

template <typename Key, typename T, bool lru = true>
class DataAccessObject : public DataAccessObjectBase<Key, T, lru>
{
  typedef DataAccessObjectBase<Key, T, lru> Base;

protected:
  explicit DataAccessObject(int cacheSize = 0,
                            const std::string& cacheType = "",
                            size_t version = 1)
    : Base(cacheSize, cacheType, version)
  {
  }

  virtual ~DataAccessObject() {}

  log4cxx::LoggerPtr& getLogger() override
  {
    static log4cxx::LoggerPtr logger(
        log4cxx::Logger::getLogger("atseintl.DBAccess.DataAccessObject"));
    return logger;
  }

  template <typename QUERY, typename DAO, typename DATA>
  class Loader
  {
  private:
    bool _successful;
    bool _updateLDC;

  public:
    bool successful() const { return _successful; }

    bool updateLDC() const { return _updateLDC; }

    bool gotFromLDC() const { return (DAO::instance().loadSource() == DiskCache::LOADSOURCE_LDC); }

    Loader(DATA& data, bool loadQuerySupported = true) : _successful(true), _updateLDC(true)
    {
      DAO& dao(DAO::instance());
      std::string name(dao.name());
      log4cxx::LoggerPtr& logger(dao.getLogger());

      // Track calls for code coverage
      dao.incrementLoadCallCount();

      dao.setLoadSource(DiskCache::LOADSOURCE_OTHER);
      if (dao.ldcHelper().loadFromLDC())
      {
        dao.setLoadSource(DiskCache::LOADSOURCE_LDC);
        _updateLDC = false;
      }
      else if (loadQuerySupported && CacheManager::instance().cacheLoadOnStart(name))
      {
        dao.setLoadSource(DiskCache::LOADSOURCE_DB);
        DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
        DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(dao.cacheClass());
        try { QUERY(dbAdapter->getAdapter()).execute(data); }
        catch (...)
        {
          LOG4CXX_WARN(logger, "DB exception in object load for cache [" << name << "].");
          dao.ldcHelper().freeObjectMemory(data);
          throw;
        }
      }
      else
      {
        _successful = false;
      }
    }
  };

  template <typename Q, typename I, typename DAO>
  struct StartupLoader
  {
    StartupLoader(bool loadQuerySupported = true)
    {
      T data;
      Loader<Q, DAO, T> loader(data, loadQuerySupported);
      if (loader.successful() && !loader.gotFromLDC())
        DAO::instance().putByKey(DAO::instance(), data, loader.updateLDC());
    }
  };

  template <typename I, typename DAO>
  struct StartupLoaderNoDB
  {
    struct NoQuery
    {
      NoQuery(tse::DBAdapter* dummy) {}

      void execute(T& data) {}
    };

    StartupLoaderNoDB() { StartupLoader<NoQuery, I, DAO>(false); }
  };

  template <typename U>
  const T*
  applyFilter(DeleteList& del, typename Base::DAOCache::pointer_type& recs, const U& filter)
  {
    static T emptyT;
    if (recs.get() == nullptr || recs->empty())
    {
      return &emptyT;
    }
    typename T::iterator iter = std::find_if(recs->begin(), recs->end(), filter);
    if (iter == recs->end())
    {
      del.copy(recs);
      return recs.get();
    }
    T* ret = new T();
    ret->reserve(recs->size() - 1);
    if (iter != recs->begin())
    {
      std::copy(recs->begin(), iter, std::back_inserter(*ret));
    }
    std::remove_copy_if(++iter, recs->end(), std::back_inserter(*ret), filter);
    if (ret->empty())
    {
      delete ret;
      return &emptyT;
    }
    del.copy(recs);
    del.adopt(ret);
    return ret;
  }

  template <typename U, typename V, typename W>
  const T* searchAndFilter(DeleteList& del,
                           typename Base::DAOCache::pointer_type& recs,
                           const U& comparator,
                           const V& value,
                           const W& filter)
  {
    static T emptyT;
    if (UNLIKELY(recs.get() == nullptr || recs->empty()))
    {
      return &emptyT;
    }
    std::pair<typename T::iterator, typename T::iterator> range =
        std::equal_range(recs->begin(), recs->end(), value, comparator);
    if (UNLIKELY(range.first == range.second))
    {
      return &emptyT;
    }
    typename T::iterator iter = std::find_if(range.first, range.second, filter);
    if (UNLIKELY(range.first == recs->begin() && iter == recs->end()))
    {
      del.copy(recs);
      return recs.get();
    }

    T* ret = new T();
    ret->reserve(std::distance(range.first, range.second) - 1);
    if (iter != range.first)
    {
      std::copy(range.first, iter, std::back_inserter(*ret));
    }
    if (iter != range.second)
    {
      std::remove_copy_if(++iter, range.second, std::back_inserter(*ret), filter);
    }

    if (UNLIKELY(ret->empty()))
    {
      delete ret;
      return &emptyT;
    }
    del.copy(recs);
    del.adopt(ret);
    return ret;
  }
};

} // namespace tse;

