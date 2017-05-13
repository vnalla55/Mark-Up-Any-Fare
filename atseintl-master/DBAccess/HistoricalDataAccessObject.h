//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/DataAccessObjectBase.h"

#include "DBAccess/DaoPredicates.h"

namespace tse
{
// ------------------------------
//  Historical Data Access Object
// ------------------------------

template <typename Key, typename T, bool lru = true>
class HistoricalDataAccessObject : public DataAccessObjectBase<Key, T, lru>
{
  typedef DataAccessObjectBase<Key, T, lru> Base;

public:
  using Base::invalidate;
  using Base::translateKey;
  using Base::name;

protected:
  DAOUtils::CacheBy _cacheBy;

  explicit HistoricalDataAccessObject(int cacheSize = 0,
                                      const std::string& cacheType = "",
                                      size_t version = 1)
    : Base(cacheSize, cacheType, version), _cacheBy(DAOUtils::HALFMONTHLY)
  {
  }

  virtual ~HistoricalDataAccessObject() {}

  size_t invalidate(const ObjectKey& objectKey) override
  {
    size_t result(0);
    log4cxx::LoggerPtr& logger(getLogger());
    LOG4CXX_DEBUG(logger, "Invalidating " << name() << " keys (" << objectKey.toString() << ")");

    Key key;
    if (_cacheBy == DAOUtils::NODATES)
    {
      if (translateKey(objectKey, key))
      {
        result += invalidate(key);
      }
      else
      {
        LOG4CXX_ERROR(logger,
                      "Invalidate " << name() << " translateKey failed for ("
                                    << objectKey.toString() << ")");
      }
      return result;
    }
    DateTime firstCacheDate(boost::date_time::neg_infin);
    time_t nextCacheDate(0);
    DateTime ticketDate = DAOUtils::firstCacheDate(firstCacheDate, nextCacheDate);
    DateTime last = DateTime::localTime().addDays(1); // allow for time zones
    if (translateKey(objectKey, key, ticketDate))
    {
      while (ticketDate <= last)
      {
        if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
        {
          DateTime dates[2];
          DAOUtils::getDateRange(ticketDate, dates[0], dates[1], _cacheBy);
          LOG4CXX_DEBUG(logger,
                        "Invalidating " << name() << " from " << dates[0].toSimpleString() << " to "
                                        << dates[1].toSimpleString());
        }
        result += invalidate(key);
        ticketDate = DAOUtils::nextCacheDate(ticketDate, _cacheBy);
        setKeyDateRange(key, ticketDate);
      }
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "Invalidate " << name() << " translateKey failed for (" << objectKey.toString()
                                  << ")");
    }
    return result;
  }

  virtual bool translateKey(const ObjectKey& objectKey, Key& key, const DateTime ticketDate) const
  {
    return false;
  }

  void setCacheBy(DAOUtils::CacheBy newValue) { _cacheBy = newValue; }

  log4cxx::LoggerPtr& getLogger() override
  {
    static log4cxx::LoggerPtr logger(
        log4cxx::Logger::getLogger("atseintl.DBAccess.HistoricalDataAccessObject"));
    return logger;
  }

  DAOUtils::CacheBy cacheBy() { return _cacheBy; }

  //      Temporary default to allow incomplete DAOs to link
  //      remove as soon as possible
  virtual void setKeyDateRange(Key& key, const DateTime date) const { return; }

  template <typename I, typename DAO>
  class DummyObjectInserterWithDateRange
  {
  private:
    bool _success;

  public:
    bool success() const { return _success; }

    void putInfoIntoObject(const Key& key, I* info, std::vector<I*>* object)
    {
      object->push_back(info);
    }

    void putInfoIntoObject(const Key& key, I* info, std::vector<const I*>* object)
    {
      object->push_back(info);
    }

    void putInfoIntoObject(const Key& key, I* info, std::map<Key, I*>* object)
    {
      object->insert(std::map<Key, I*>::value_type(key, info));
    }

    template <typename A, typename C, typename D>
    void putInfoIntoObject(const Key& key,
                           I* info,
                           std::tr1::unordered_multimap<const A, I*, C, D>* object)
    {
      object->insert(
          typename std::tr1::unordered_multimap<const A, I*, C, D>::value_type(A(), info));
    }

    template <typename A>
    void putInfoIntoObject(const Key& key, I* info, std::multimap<A, I*>* object)
    {
      object->insert(typename std::multimap<A, I*>::value_type(A(), info));
    }

    void putInfoIntoObject(const Key& key, I* info, I* object)
    {
      // not really a collection, so no insertion necessary
    }

    DummyObjectInserterWithDateRange(std::string& flatKey, ObjectKey& objectKey) : _success(false)
    {
      DateTime startDate;
      DateTime endDate;
      DateTime dummyTicketDate(DateTime::localTime());
      DAOUtils::getDateRange(dummyTicketDate, startDate, endDate, DAO::instance().cacheBy());

      I* info(new I);
      I::dummyData(*info);
      T* object(new T);
      Key key(DAO::instance().createKey(info, startDate, endDate));
      putInfoIntoObject(key, info, object);
      DAO::instance().cache().put(key, object, true);

      objectKey.tableName() = DAO::instance().name();
      objectKey.keyFields().clear();
      DAO::instance().translateKey(key, objectKey);

      KeyStream stream(0);
      stream << key;
      flatKey = stream;

      _success = true;
    }
  };

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
      if (Global::allowHistorical())
      {
        T data;
        Loader<Q, DAO, T> loader(data, loadQuerySupported);
        if (loader.successful())
        {
          if (!loader.gotFromLDC())
          {
            DAO::instance().putByKey(DAO::instance(), data, loader.updateLDC());
          }
        }
      }
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

  virtual CreateResult<T> create(const Key& key, bool) override
  {
    return DataAccessObjectBase<Key, T, lru>::create(key, true);
  }

};

} // namespace tse;

