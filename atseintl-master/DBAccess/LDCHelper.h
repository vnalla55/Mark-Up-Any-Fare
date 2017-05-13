//-----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#ifndef LDC_HELPER_H
#define LDC_HELPER_H

#include "Allocator/TrxMalloc.h"
#include "Common/ObjectComparison.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseWrappers.h"
#include "DBAccess/DataBlobHelper.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/DistCache.h"
#include "DBAccess/DistCachePool.h"
#include "DBAccess/FareCalcConfigText.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/LDCOperationCounts.h"
#include "Util/Time.h"

#include <sstream>
#include <string>
#include <vector>

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DBAdapter;
class MarketRouting;
class TSIInfo;
class RuleCategoryDescInfo;
class DST;

typedef HashKey<NationCode> NationKey;
typedef HashKey<CarrierCode> CarrierKey;
typedef HashKey<int> IntKey;
typedef HashKey<CurrencyCode> CurrencyKey;
typedef HashKey<DBEClass> DBEClassKey;
typedef HashKey<UserApplCode> UserApplKey;
typedef HashKey<LocCode> LocCodeKey;
typedef HashKey<std::string> StringKey;
typedef HashKey<PaxTypeCode> PaxCodeKey;
typedef HashKey<AlphaCode> AlphaKey;
typedef HashKey<CatNumber> CatKey;
typedef HashKey<Indicator> CharKey;
typedef HashKey<TaxCode> TaxCodeKey;

// inline bool rowsReturned( tse::BoolWrapper * pObj )
// {
//   return ( ( pObj != NULL ) && ( *pObj == true ) ) ;
// }
//
// inline bool rowsReturned( tse::FareCalcConfigText * pObj )
// {
//   return ( ( pObj != NULL ) && ( pObj->fccTextMap().size() > 0 ) ) ;
// }
//
// inline bool rowsReturned( tse::MarketRouting * pObj )
// {
//   return ( true ) ;
// }
//
// inline bool rowsReturned( tse::TSIInfo * pObj )
// {
//   return ( true ) ;
// }
//
// inline bool rowsReturned( tse::RuleCategoryDescInfo * pObj )
// {
//   return ( pObj != NULL ) ;
// }
//
// inline bool rowsReturned( tse::DST * pObj )
// {
//   return ( pObj != NULL ) ;
// }
//
// template<typename COLLECTION>
// inline bool rowsReturned( COLLECTION * pObj )
// {
//   return ( ( pObj != NULL ) && ( pObj->size() > 0 ) ) ;
// }

template <class DAOIMPL, class Key, class T, class CachePtr>
class LDCHelper
{
private:
  DAOIMPL& _dao;
  DiskCache::LoadSource _loadSource;

  LDCHelper();
  LDCHelper(const LDCHelper& other);
  LDCHelper& operator=(const LDCHelper& other);

  DiskCache::CacheTypeOptions* _cto;
  bool _loadmaster;
  bool _eventmaster;
  const CachePtr _uninitializedCacheEntry;

protected:
  template <typename A,
            typename B,
            typename C,
            typename D,
            typename E,
            typename F,
            typename G,
            typename H,
            typename I,
            typename J>
  static void keyFromFlatKey(const std::string& flatKey, HashKey<A, B, C, D, E, F, G, H, I, J>& key)
  {
    key.fromString(flatKey);
  }

  static void keyFromFlatKey(const std::string& flatKey, int& key) { key = atoi(flatKey.c_str()); }

  static void keyFromFlatKey(const std::string& flatKey, bool& key)
  {
    if (flatKey == "0")
    {
      key = false;
    }
    else
    {
      key = true;
    }
  }

  static void keyFromFlatKey(const std::string& flatKey, char& key) { key = flatKey.c_str()[0]; }

  template <typename ANY>
  static void keyFromFlatKey(const std::string& flatKey, ANY& key)
  {
    key = flatKey;
  }

public:
  LDCHelper(DAOIMPL& dao)
    : _dao(dao),
      _loadSource(DiskCache::LOADSOURCE_OTHER),
      _cto(nullptr),
      _loadmaster(DISKCACHE.getDistCacheLoadMaster()),
      _eventmaster(DISKCACHE.getDistCacheEventMaster())
  {
  }

  bool init(const std::string& name)
  {
    _cto = DISKCACHE.getCacheTypeOptions(name);

    if (_cto)
    {
      _dao.cache().getCacheImpl()->setCacheTypeOptions(_cto);
    }

    return (_cto != nullptr);
  }

  log4cxx::LoggerPtr& getLogger() { return _dao.getLogger(); }

  const std::string& name() { return _dao.name(); }

  DAOIMPL& dao() { return _dao; }

  DiskCache::LoadSource loadSource() const { return _loadSource; }

  void setLoadSource(DiskCache::LoadSource val) { _loadSource = val; }

  std::string loadSourceAsString() const { return DiskCache::loadSourceToString(_loadSource); }

  bool ldcEnabled() const { return dao().cache().ldcEnabled(); }

  DiskCache::DataBlob* createDataBlob(const std::string& flatKey, const T& data)
  {
    // Caller is responsible to clear the blob when finished!
    return DataBlobHelper<Key, T>::flatten(flatKey, data, name(), *_cto);
  }

  // Compare objects on disk to those in memory
  bool compareMemCacheToLDC(std::vector<std::string>& results, uint32_t& numCompared)
  {
    bool retval(true);

    std::shared_ptr<std::vector<Key>> keys = _dao.cache().keys();
    for (typename std::vector<Key>::const_iterator kit = keys->begin(); kit != keys->end(); ++kit)
    {
      const Key& key = (*kit);
      CachePtr ptr = _dao.cache().getIfResident(key);
      ++numCompared;
      if (ptr != _uninitializedCacheEntry)
      {
        KeyStream stream(0);
        stream << key;
        std::string flatKey(stream);
        if (!flatKey.empty())
        {
          std::stringstream errmsg;
          DiskCache::DataBlob* blob = createDataBlob(flatKey, *ptr);
          if (blob)
          {
            T* ldcObj = nullptr;
            DiskCache::DataBlob* blob2 = new DiskCache::DataBlob();

            DiskCache::DBRead dbrc;

            dbrc = DISKCACHE.readFromDB(name(), flatKey, blob2);
            if (dbrc == DiskCache::DBREAD_SUCCESS)
            {
              ldcObj = DataBlobHelper<Key, T>::unflatten(blob2, flatKey.c_str(), name(), *_cto);
              if (ldcObj != nullptr)
              {
                if (tse::objectsIdentical(*ldcObj, *(ptr.get())))
                {
                  retval = (retval && true);
                }
                else
                {
                  errmsg << "NOT_IDENTICAL for key [" << flatKey << "]";
                  results.push_back(errmsg.str());

                  // std::ostringstream os1 ;
                  // std::string memAsString ;
                  // objToTextString( *(ptr.get()), memAsString ) ;
                  // os1 << "  ==> MEM: " << memAsString ;
                  // results.push_back( os1.str() ) ;

                  // std::ostringstream os2 ;
                  // std::string ldcAsString ;
                  // objToTextString( *ldcObj, ldcAsString ) ;
                  // os2 << "  ==> LDC: " << ldcAsString ;
                  // results.push_back( os2.str() ) ;

                  std::ostringstream os1;
                  objToTextStream(flatKey, *(ptr.get()), os1);
                  results.push_back("<== MEMORY ==>");
                  results.push_back(os1.str());
                  results.push_back("<============>");

                  std::ostringstream os2;
                  objToTextStream(flatKey, *ldcObj, os2);
                  results.push_back("<== LDC ==>");
                  results.push_back(os2.str());
                  results.push_back("<=========>");

                  retval = false;
                }
                delete ldcObj;
              }
              else
              {
                errmsg << "DESERIALIZATION_FAILURE for key [" << flatKey << "]";
                results.push_back(errmsg.str());
                retval = false;
              }
            }
            else
            {
              errmsg << "UNABLE TO READ OBJECT FROM DB for key [" << flatKey << "]";
              results.push_back(errmsg.str());
              retval = false;
            }
            delete blob2;
          }
          else
          {
            errmsg << "SERIALIZATION_FAILURE for key [" << flatKey << "]";
            results.push_back(errmsg.str());
            retval = false;
          }

          delete blob;
        }
      }
    }

    return retval;
  }

  inline T* getFromDB(const Key& key) { return dao().create(key); }

  inline T* getFromDBUsingFlatKey(const std::string& flatKey)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    return getFromDB(key);
  }

  // Compare objects in memory to those in the DB
  bool compareMemCacheToDB(std::vector<std::string>& results, uint32_t& numCompared)
  {
    bool retval(true);

    std::shared_ptr<std::vector<Key>> keys = _dao.cache().keys();
    for (typename std::vector<Key>::const_iterator kit = keys->begin(); kit != keys->end(); ++kit)
    {
      const Key& key = (*kit);
      CachePtr ptr(_dao.cache().getIfResident(key));
      bool isIdentical(false);
      ++numCompared;

      T* memObj = ptr.get();
      T* dbObj = getFromDB(key);

      if ((memObj == nullptr) && (dbObj == nullptr))
      {
        isIdentical = true;
      }
      else if ((memObj != nullptr) && (dbObj != nullptr))
      {
        isIdentical = objectsIdentical(*memObj, *dbObj);
      }

      if (!isIdentical)
      {
        retval = false;

        KeyStream stream(0);
        stream << key;
        std::string flatKey(stream);

        std::ostringstream errmsg;
        errmsg << "NOT_IDENTICAL for key [" << flatKey << "]";
        results.push_back(errmsg.str());

        // std::ostringstream os1 ;
        // std::string memAsString ;
        // objToTextString( *memObj, memAsString ) ;
        // os1 << "  ==> MEM: " << memAsString ;
        // results.push_back( os1.str() ) ;
        //
        // std::ostringstream os2 ;
        // std::string dbAsString ;
        // objToTextString( *dbObj, dbAsString ) ;
        // os2 << "  ==>  DB: " << dbAsString ;
        // results.push_back( os2.str() ) ;

        std::ostringstream os1;
        objToTextStream(flatKey, *memObj, os1);
        results.push_back("<== MEMORY ==>");
        results.push_back(os1.str());
        results.push_back("<============>");

        std::ostringstream os2;
        objToTextStream(flatKey, *dbObj, os2);
        results.push_back("<== DB ==>");
        results.push_back(os2.str());
        results.push_back("<========>");
      }

      if (dbObj != nullptr)
      {
        delete dbObj;
      }
    }

    return retval;
  }

  void setDistCache(const std::string& key, DiskCache::DataBlob* blob) const
  {
    if (_cto && _cto->useDistCache && blob)
    {
      uint32_t length(0);
      char* data(blob->getRawData(&length));
      if (length && length < MEMCACHE_MAX_BLOB_BYTES)
      {
        KeyStream stream(_dao.name(), _dao.tableVersion());
        stream << key;

        const size_t ttl(_cto->ttl);
        DistCacheScopedLockQueueMemberPtr dist;
        dist->set(stream, stream.size(), data, length, ttl);
      }
    }
  }

  void removeFromDistCache(const std::string& key)
  {
    if (_cto && _cto->useDistCache)
    {
      const time_t no_wait(0);
      KeyStream stream(_dao.name(), _dao.tableVersion());
      stream << key;
      DistCacheScopedLockQueueMemberPtr dist;
      dist->remove(stream, static_cast<uint32_t>(stream.size()), no_wait);
    }
  }

  class DeserializeTask : public TseCallableTask
  {
  public:
    DeserializeTask(LDCHelper& helper,
                    DiskCache::DataBlob* blob,
                    const std::string& flatKey,
                    size_t& numCreated)
      : _helper(helper),
        _blob(blob),
        _flatKey(flatKey),
        _numCreated(numCreated),
        _idx(numCreated),
        _done(false)
    {
    }

    ~DeserializeTask() {}

    void run() override
    {
      const MallocContextDisabler context;
      if (LIKELY(_blob))
      {
        if (UNLIKELY(_helper._loadmaster))
        {
          _helper.setDistCache(_flatKey, _blob);
        }

        T* obj = DataBlobHelper<Key, T>::unflatten(
            _blob, _flatKey.c_str(), _helper.name(), *_helper._cto);
        delete _blob;
        _blob = nullptr;

        if (LIKELY(obj))
        {
          Key key;
          try { key.fromString(_flatKey); }
          catch (const std::exception& e)
          {
            LOG4CXX_ERROR(_helper.getLogger(),
                          "Exception: key.fromString in cache [" << _helper.name() << "] for key["
                                                                 << _flatKey.c_str() << "] "
                                                                 << e.what());
          }
          catch (...)
          {
            LOG4CXX_ERROR(_helper.getLogger(),
                          "Exception: key.fromString in cache [" << _helper.name() << "] for key["
                                                                 << _flatKey.c_str() << "] ");
          }

          _helper.dao().cache().put(key, obj, false);
          ++_numCreated;
        }
      }
      _done = true;
    }

  private:
    LDCHelper& _helper;
    DiskCache::DataBlob* _blob;
    const std::string _flatKey;
    size_t& _numCreated;
    size_t _idx;
    bool _done;
  };

  bool loadFromLDC()
  {
    size_t numCreated(0);
    size_t numRead(0);
    size_t maxAllowed(DISKCACHE.getMaxActiveDeserializeTasks());
    const auto readSleepTime = std::chrono::duration_cast<Time::Duration>(
        std::chrono::milliseconds(DISKCACHE.getDeserializeSleepMillis()));

    if (dao().ldcEnabled() && (!DISKCACHE.bdbDisabled()))
    {
      if (DISKCACHE.isTooOld())
      {
        LOG4CXX_DEBUG(getLogger(), "LDC is too old for cache [" << name() << "].");
      }
      else
      {
        TseScopedExecutor executor(TseThreadingConst::LDC_TASK, DISKCACHE.getSerializeQueueThreadMax());
        DBC* dbCur(nullptr);
        DiskCache::DBRead dbrc(DiskCache::DBREAD_SUCCESS);
        DiskCache::Timer readTimer;

        std::vector<DeserializeTask*> tasks;

        readTimer.reset();

        while ((dbrc == DiskCache::DBREAD_SUCCESS) && (!DISKCACHE.shuttingDown()) &&
               (!DISKCACHE.bdbDisabled()))
        {
          if (UNLIKELY(maxAllowed && ((numRead - numCreated) > maxAllowed)))
          {
            LOG4CXX_DEBUG(getLogger(),
                          "More Active Items than Max Allowed ["
                              << maxAllowed
                              << "] present in the Task Queue, so waiting to read for cache ["
                              << name() << "]");
            Time::sleepFor(readSleepTime);
          }
          DiskCache::DataBlob* blob = new DiskCache::DataBlob();
          std::string flatKey;

          dbrc = DISKCACHE.readNextFromDB(&dbCur, name(), flatKey, blob);
          numRead++;

          if (dbrc == DiskCache::DBREAD_SUCCESS)
          {
            DeserializeTask* dt = new DeserializeTask(*this, blob, flatKey, numCreated);
            executor.execute(dt);
            tasks.push_back(dt);
          }
          else
          {
            delete blob;
            blob = nullptr;
          }
        }

        if (dbrc == DiskCache::DBREAD_FAILURE)
        {
          LOG4CXX_ERROR(getLogger(),
                        "LDC load failure for [" << name() << "] !!! - PROBLEM READING FROM DB");

          LOG4CXX_DEBUG(getLogger(), "Cancelling Deserializer tasks for [" << name() << "] !!!");
          executor.cancel();
        }

        LOG4CXX_DEBUG(getLogger(),
                      "Waiting for Deserializer tasks to finish for [" << name() << "] !!!");

        executor.wait();

        for (auto& task : tasks)
        {
          delete task;
        }

        readTimer.checkpoint();

        if (!executor.isCanceled())
        {
          LOG4CXX_DEBUG(getLogger(),
                        "Completed load from LDC for ["
                            << name() << "]"
                            << " in " << readTimer.elapsed() << "ms (elapsed)"
                            << " and " << readTimer.cpu() << "ms (cpu).");
        }
        else
        {

          LOG4CXX_INFO(getLogger(),
                       "Clearing out any items loaded in cache for [" << name() << "].");
          dao().clear();

          LOG4CXX_INFO(getLogger(), "Removing BerkeleyDB file for [" << name() << "] !!!");
          DISKCACHE.removeDB(name());
          numCreated = 0;
        }
      }
    }

    LOG4CXX_DEBUG(getLogger(),
                  "Loaded [" << numCreated << "] object" << (numCreated == 1 ? "" : "s")
                             << " from LDC for cache [" << name() << "].");
    return (numCreated > 0);
  }

  void invalidate(const std::string& flatKey)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    ObjectKey objectKey(name());
    dao().translateKey(key, objectKey);
    dao().invalidate(objectKey);
  }

  bool objectExistsInMem(const std::string& flatKey)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    CachePtr ptr = dao().cache().getIfResident(key);
    return (ptr != _uninitializedCacheEntry);
  }

  bool objectExistsInDB(const std::string& flatKey)
  {
    bool retval(false);
    T* dbObj = getFromDBUsingFlatKey(flatKey);
    if (dbObj != nullptr)
    {
      retval = true;
      delete dbObj;
      dbObj = nullptr;
    }
    return retval;
  }

  bool objectExistsInLDC(const std::string& flatKey, time_t& timestamp)
  {
    bool retval(false);
    DiskCache::DataBlob blob;

    DiskCache::DBRead dbrc = DISKCACHE.readFromDB(name(), const_cast<std::string&>(flatKey), &blob);
    retval = (dbrc == DiskCache::DBREAD_SUCCESS);

    if (retval)
    {
      timestamp = blob.getHeader()->timestamp;
    }

    return retval;
  }

  void objToXMLString(const std::string& flatKey, T& obj, std::string& result)
  {
    std::ostringstream out;
    objToTextStream(flatKey, obj, out, true);
    result = "Unhandled format: xml. Using fallback format: text.\n" +
             out.str();
  }

  void objToTextString(const std::string& flatKey, T& obj, std::string& result)
  {
    std::ostringstream out;
    objToTextStream(flatKey, obj, out, true);
    result = out.str();
  }

  bool objToTextStream(const std::string& flatKey, const T& obj, std::ostringstream& strm,
                       bool pprint = false)
  {
    bool retval(false);
    try
    {
      Flattenizable::Archive archive;
      archive.setOStream(&strm);
      archive.setStringifyDates();
      if (pprint)
      {
        archive.setPrettyPrint();
      }
      std::string ignore;
      FLATTENIZE_SAVE(archive, obj, 0, ignore, ignore);
      retval = true;
    }
    catch (const std::exception& e)
    {
      LOG4CXX_ERROR(getLogger(),
                    "LDC text stream failure for cache [" << name() << "] key [" << flatKey
                                                          << "] !!! - " << e.what());
    }
    catch (...)
    {
      LOG4CXX_ERROR(getLogger(),
                    "LDC text stream failure for cache [" << name() << "] key [" << flatKey
                                                          << "] !!! - UNKNOWN EXCEPTION");
    }

    return retval;
  }

  void getMemObjectAsXML(const std::string& flatKey, std::string& result)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    CachePtr ptr = dao().cache().getIfResident(key);
    if (ptr != _uninitializedCacheEntry)
    {
      objToXMLString(flatKey, *ptr, result);
    }
    else
    {
      result = "NOT_FOUND";
    }
  }

  void getLDCObjectAsXML(const std::string& flatKey, std::string& result)
  {
    DiskCache::DataBlob* blob(new DiskCache::DataBlob());
    DiskCache::DBRead dbrc(DISKCACHE.readFromDB(name(), const_cast<std::string&>(flatKey), blob));
    if (dbrc == DiskCache::DBREAD_SUCCESS)
    {
      T* ldcObj(DataBlobHelper<Key, T>::unflatten(blob, flatKey.c_str(), name(), *_cto));
      if (ldcObj != nullptr)
      {
        objToXMLString(flatKey, *ldcObj, result);
        delete ldcObj;
      }
      else
      {
        result = "DESERIALIZATION_ERROR";
      }
    }
    else
    {
      result = "NOT_FOUND";
    }
    delete blob;
  }

  void getDBObjectAsXML(const std::string& flatKey, std::string& result)
  {
    T* dbObj = getFromDBUsingFlatKey(flatKey);
    if (dbObj != nullptr)
    {
      objToXMLString(flatKey, *dbObj, result);
      delete dbObj;
    }
    else
    {
      result = "NOT_FOUND";
    }
  }

  void getMemObjectAsText(const std::string& flatKey, std::string& result)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    CachePtr ptr = dao().cache().getIfResident(key);
    if (ptr.get() != nullptr)
    {
      objToTextString(flatKey, *ptr, result);
    }
  }

  void getLDCObjectAsText(const std::string& flatKey, std::string& result)
  {
    DiskCache::DataBlob* blob(new DiskCache::DataBlob());
    DiskCache::DBRead dbrc(DISKCACHE.readFromDB(name(), const_cast<std::string&>(flatKey), blob));
    if (dbrc == DiskCache::DBREAD_SUCCESS)
    {
      T* ldcObj(DataBlobHelper<Key, T>::unflatten(blob, flatKey.c_str(), name(), *_cto));
      if (ldcObj != nullptr)
      {
        objToTextString(flatKey, *ldcObj, result);
        delete ldcObj;
      }
      else
      {
        result = "DESERIALIZATION_ERROR";
      }
    }
    else
    {
      result = "NOT_FOUND";
    }
    delete blob;
  }

  void getDBObjectAsText(const std::string& flatKey, std::string& result)
  {
    T* dbObj = getFromDBUsingFlatKey(flatKey);
    if (dbObj != nullptr)
    {
      objToTextString(flatKey, *dbObj, result);
      delete dbObj;
    }
    else
    {
      result = "NOT_FOUND";
    }
  }

  void getMemObjectAsFlat(const std::string& flatKey, std::string& result)
  {
    Key key;
    keyFromFlatKey(flatKey, key);
    CachePtr ptr = dao().cache().getIfResident(key);
    if (ptr.get() != nullptr)
    {
      std::ostringstream strm;
      objToTextStream(flatKey, *(ptr.get()), strm);
      result = strm.str();
    }
  }

  void getLDCObjectAsFlat(const std::string& flatKey, std::string& result)
  {
    DiskCache::DataBlob* blob(new DiskCache::DataBlob());
    DiskCache::DBRead dbrc(DISKCACHE.readFromDB(name(), const_cast<std::string&>(flatKey), blob));
    if (dbrc == DiskCache::DBREAD_SUCCESS)
    {
      std::ostringstream strm;
      T* obj(nullptr);
      char* bufferToDelete(nullptr);
      char* buffer(nullptr);
      size_t dataSize(0);

      try
      {
        blob->getData(buffer, dataSize, bufferToDelete);
        if (dataSize > 0)
        {
          obj = new T;
          Flattenizable::Archive archive;
          archive.setOStream(&strm);
          FLATTENIZE_RESTORE(archive, *obj, buffer, dataSize);
        }
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(getLogger(),
                      "LDC text stream failure for cache [" << name() << "] key [" << flatKey
                                                            << "] !!! - " << e.what());
      }
      catch (...)
      {
        LOG4CXX_ERROR(getLogger(),
                      "LDC text stream failure for cache [" << name() << "] key [" << flatKey
                                                            << "] !!! - UNKNOWN EXCEPTION");
      }

      delete obj;
      obj = nullptr;
      delete[] bufferToDelete;
      bufferToDelete = nullptr;

      result = strm.str();
    }
    else
    {
      result = "NOT_FOUND";
    }
    delete blob;
  }

  void getDBObjectAsFlat(const std::string& flatKey, std::string& result)
  {
    T* dbObj = getFromDBUsingFlatKey(flatKey);
    if (dbObj != nullptr)
    {
      std::ostringstream strm;
      objToTextStream(flatKey, *dbObj, strm);
      result = strm.str();
      delete dbObj;
    }
    else
    {
      result = "NOT_FOUND";
    }
  }

  void getAllFlatKeys(std::set<std::string>& list, bool inclValues = false)
  {
    std::shared_ptr<std::vector<Key>> keys = dao().cache().keys();
    for (typename std::vector<Key>::const_iterator kit = keys->begin(); kit != keys->end(); ++kit)
    {
      const Key& key = (*kit);
      CachePtr ptr = dao().cache().getIfResident(key);
      if (ptr.get() != nullptr)
      {
        KeyStream stream(0);
        stream << key;
        std::string returnString(stream);

        if (inclValues)
        {
          std::ostringstream f;
          objToTextStream(returnString, *ptr, f);
          returnString.append(" <=> ");
          returnString.append(f.str());
        }

        list.insert(returnString);
      }
    }
  }

  bool writeToLDC(const Key& key, time_t reqTime, bool whileLoading, bool distCacheOp, bool ldcOp)
  {
    bool retval(false);

    DiskCache::DataBlob* blob(nullptr);

    CachePtr ptr = dao().cache().getIfResident(key);
    if (ptr != _uninitializedCacheEntry)
    {
      KeyStream stream(0);
      stream << key;
      std::string flatKey(stream);
      DiskCache::Timer serializeTimer;
      DiskCache::Timer ioTimer;
      bool debugLogging(IS_DEBUG_ENABLED(getLogger()));

      if (UNLIKELY(IS_DEBUG_ENABLED(getLogger())))
      {
        serializeTimer.reset();
      }

      blob = createDataBlob(flatKey, *ptr);

      if (UNLIKELY(debugLogging))
      {
        serializeTimer.checkpoint();

        LOG4CXX_DEBUG(getLogger(),
                      "Cache [" << name() << "] key [" << flatKey << "] serialized"
                                << " in " << serializeTimer.elapsed() << "ms (elapsed)"
                                << " and " << serializeTimer.cpu() << "ms (cpu).");
      }

      if (LIKELY(blob != nullptr))
      {
        if (UNLIKELY((!whileLoading || _loadmaster) && _cto->useDistCache && distCacheOp))
        {
          setDistCache(flatKey, blob);
        }

        if (ldcOp && _cto && _cto->enabled)
        {
          if (UNLIKELY(debugLogging))
          {
            ioTimer.reset();
          }
          retval = DISKCACHE.writeToDB(name(), flatKey, *blob);
          delete blob;
          blob = nullptr;

          if (!retval)
          {
            // if write fails, disable any further LDC activity for
            // this cache
            LOG4CXX_ERROR(getLogger(),
                          "Disabling LDC for [" << name() << "] due to write failure for key ["
                                                << flatKey << "] !!!");
            _cto->disableCloseAndRemove();
          }

          if (UNLIKELY(debugLogging && retval))
          {
            ioTimer.checkpoint();
            LOG4CXX_DEBUG(getLogger(),
                          "Cache [" << name() << "] key [" << flatKey << "] disk write performed"
                                    << " in " << ioTimer.elapsed() << "ms (elapsed)"
                                    << " and " << ioTimer.cpu() << "ms (cpu).");
          }
        }
      }
    }
    else
    {
      retval = false;
#if 0
          LOG4CXX_DEBUG( getLogger()
                       , "writeLDC failed - cache key no longer resident for cache [" << name()
                                                                                      << "] key ["
                                                                                      << flatKey
                                                                                      << "]."
                       ) ;
#endif
    }

    return retval;
  }

  bool removeFromLDC(const Key& key, time_t reqTime, bool whileLoading, bool distCacheOp)
  {
    bool retval(true);

    KeyStream stream(0);
    stream << key;
    std::string flatKey(stream);

    if (UNLIKELY(_eventmaster && !whileLoading && distCacheOp))
    {
      removeFromDistCache(flatKey);
    }

    if (LIKELY(_cto && _cto->enabled))
    {
      retval = DISKCACHE.remove(reqTime, whileLoading, name(), flatKey);
      if (UNLIKELY(!retval))
      {
        // if remove fails, disable any further LDC activity for
        // this cache
        LOG4CXX_ERROR(getLogger(),
                      "Disabling LDC for [" << name() << "] due to delete failure for key ["
                                            << flatKey << "] !!!");
        _cto->disableCloseAndRemove();
      }
    }

    return retval;
  }

  bool clearLDC(time_t reqTime, bool whileLoading)
  {
    bool retval(true);

    if (_cto && _cto->enabled)
    {
      retval = DISKCACHE.remove(reqTime, whileLoading, name(), DISKCACHE_ALL);
      if (!retval)
      {
        // if remove fails, disable any further LDC activity for
        // this cache
        LOG4CXX_ERROR(getLogger(),
                      "Disabling LDC for [" << name() << "] due to truncate failure !!!");
        _cto->disableCloseAndRemove();
      }
    }

    return retval;
  }

  bool processNextAction(LDCOperationCounts& counts)
  {
    LDCOperation<Key> dco;
    bool retval = dao().cache().actionQueueNext(dco);

    if (retval)
    {
      switch (dco.getType())
      {
      case LDCOperation<Key>::WRITE:

        if (LIKELY(ldc::keyInitialized(dco.getKey()) &&
            (writeToLDC(
                dco.getKey(), dco.getTime(), dco.whileLoading(), dco.distCacheOp(), dco.ldcOp()))))
        {
          ++counts.goodWrites;
        }
        else
        {
          ++counts.badWrites;
        }
        break;

      case LDCOperation<Key>::REMOVE:

        if (LIKELY(ldc::keyInitialized(dco.getKey()) &&
            (removeFromLDC(dco.getKey(), dco.getTime(), dco.whileLoading(), dco.distCacheOp()))))
        {
          ++counts.goodRemoves;
        }
        else
        {
          ++counts.badRemoves;
        }
        break;

      case LDCOperation<Key>::CLEAR:
        if (clearLDC(dco.getTime(), dco.whileLoading()))
        {
          ++counts.goodClears;
        }
        else
        {
          ++counts.badClears;
        }
        break;

      default:
        LOG4CXX_WARN(getLogger(), "Unsupported LDC action queue request type!");
        break;
      }
    }

    return retval;
  }

  template <typename I>
  void freeObjectMemory(std::vector<I*>& vec)
  {
    deleteVectorOfPointers(vec);
  }

  template <typename I>
  void freeObjectMemory(std::vector<I>& vec)
  {
    // nothing to do here; this is compilation aid, nothing more
  }

  template <typename I>
  void freeObjectMemory(I& junk)
  {
    // nothing to do here; this is compilation aid, nothing more
  }

  template <typename K, typename P>
  void freeObjectMemory(typename std::map<K, P*>& mapOfPtrs)
  {
    for (auto& mapOfPtr : mapOfPtrs)
    {
      delete mapOfPtr.second;
    }
  }
};

} // namespace tse;

#endif // LDC_HELPER_H
