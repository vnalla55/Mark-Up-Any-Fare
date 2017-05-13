//-------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Adapter/CacheNotifyAdapter.h"

#include "Adapter/CacheNotifyControl.h"
#include "Adapter/MonitorStateNotification.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TSEException.h"
#include "DBAccess/BoundFareDAO.h"
#include "DBAccess/CacheDeleter.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheNotifyInfo.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/MarketRoutingDAO.h"
#include "DBAccess/ObjectKey.h"
#include "DBAccess/Queries/QueryGetCacheNotify.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <thread>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/tokenizer.hpp>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#define FULL_MEMORY_BARRIER __sync_synchronize()

namespace tse
{

FIXEDFALLBACK_DECL(fallbackFilterCacheNotify);
FIXEDFALLBACK_DECL(fallbackRmHistCaches);

}// tse

using namespace tse;

static LoadableModuleRegister<Adapter, CacheNotifyAdapter>
_("libCacheNotifyAdapter.so");

log4cxx::LoggerPtr
CacheNotifyAdapter::_logger(log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyAdapter"));
log4cxx::LoggerPtr
CacheNotifyAdapter::_ordernologger(
    log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyAdapterOrderNo"));
log4cxx::LoggerPtr
CacheNotifyAdapter::_ordernoSkippedLogger(
    log4cxx::Logger::getLogger("atseintl.Adapter.CacheNotifyAdapterOrderNoSkipped"));

namespace
{
ConfigurableValue<bool>
useOrderNo("CACHE_ADP", "USE_ORDERNO", false);
ConfigurableValue<bool>
usePartitionedQuery("CACHE_ADP", "USE_PARTITIONED_ORDERNO", false);
ConfigurableValue<bool>
redoMissingOrderNos("CACHE_ADP", "REDO_MISSING_ORDERNO", false);
ConfigurableValue<std::string>
redoMissingOrderNoFile("CACHE_ADP",
                       "REDO_MISSING_ORDERNO_FILE",
                       "cachenotification-unprocessed.log");
ConfigurableValue<bool>
resyncFromLast("CACHE_ADP", "RESYNC_FROM_LAST_ORDERNO", false);
ConfigurableValue<std::string>
resyncFromLastFile("CACHE_ADP", "RESYNC_FROM_LAST_ORDERNO_FILE", "cachenotification-resync.log");
ConfigurableValue<std::string>
directory("DISK_CACHE_OPTIONS", "DIRECTORY", "../ldc");
ConfigurableValue<bool>
merchCacheActivated("CACHE_ADP", "MERCH_CACHE_ACTIVATED", false);
ConfigurableValue<uint32_t>
pollInterval("CACHE_ADP", "POLL_INTERVAL", 60);
ConfigurableValue<int>
pollSize("CACHE_ADP", "POLL_SIZE", 1000);
ConfigurableValue<std::string>
configFile("CACHE_ADP", "CONFIG_FILE", "cacheNotify.xml");
ConfigurableValue<std::string>
threadAliveFile("CACHE_ADP", "THREAD_ALIVE_FILE", "cache.thread.alive");

DateTime
localTime()
{
  time_t t;
  time(&t);
  tm ltime;
  localtime_r(&t, &ltime);
  uint16_t year = static_cast<uint16_t>(ltime.tm_year + 1900);
  uint16_t month = static_cast<uint16_t>(ltime.tm_mon + 1);
  uint16_t day = static_cast<uint16_t>(ltime.tm_mday);
  return DateTime(year, month, day, ltime.tm_hour, ltime.tm_min, ltime.tm_sec);
}

struct ScopedFileRenameReader
{
  typedef std::tr1::unordered_set<long> OrderNos;
  typedef OrderNos::iterator OrderNosIterator;
  typedef OrderNos::const_iterator OrderNosConstIterator;
  typedef std::tr1::unordered_map<std::string, OrderNos> ByCache;
  typedef ByCache::iterator ByCacheIterator;
  typedef ByCache::const_iterator ByCacheConstIterator;

  typedef std::vector<std::pair<std::string, long>> OrderNoByCache;
  bool _enabled;
  log4cxx::LoggerPtr _logger;
  std::string _filename;
  std::string _newfilename;
  size_t _max;

  ScopedFileRenameReader(bool enabled,
                         log4cxx::LoggerPtr logger,
                         const std::string& filename,
                         const std::string& newfilename,
                         size_t max = 4096)
    : _enabled(enabled), _logger(logger), _filename(filename), _newfilename(newfilename), _max(max)
  {
  }

  ByCache& load(ByCache& byCache)
  {
    if (_enabled)
    {
      errno = 0;
      if (rename(_filename.c_str(), _newfilename.c_str()) == 0)
      {
        byCache.clear();
        LOG4CXX_DEBUG(_logger,
                      "Unprocessed orderno log [" << _filename << "]  found and renamed to [ "
                                                  << _newfilename << "]");
        std::ifstream file(_newfilename.c_str());
        std::string line;
        while (file && getline(file, line))
        {
          std::vector<std::string> list;
          boost::split(list, line, boost::is_any_of(","));
          if (list.size() >= 3)
          {
            std::string& cache(list[0]);
            long orderno(atol(list[2].c_str()));
            OrderNos& orderNos(byCache[cache]);
            if (orderNos.find(orderno) == orderNos.end())
            {
              orderNos.insert(orderno);
            }
          }
        }
        unlink(_newfilename.c_str());
      }
      else
      {
        LOG4CXX_DEBUG(_logger,
                      "Unprocessed orderno log [" << _filename << "] error in rename to [ "
                                                  << _newfilename << "] error[ " << errno << "] "
                                                  << strerror(errno));
      }
    }
    return byCache;
  }
};

struct ScopedFileRenameWriter
{
  typedef std::tr1::unordered_map<std::string, long> ByEntity;
  typedef ByEntity::iterator ByEntityIterator;
  typedef ByEntity::const_iterator ByEntityConstIterator;

  typedef std::tr1::unordered_map<std::string, ByEntity> ByCacheEntity;
  typedef ByCacheEntity::iterator ByCacheEntityIterator;
  typedef ByCacheEntity::const_iterator ByCacheEntityConstIterator;

  bool _enabled;
  log4cxx::LoggerPtr _logger;
  std::string _filename;
  std::string _newfilename;
  std::ofstream _os;
  bool _open;
  bool _synchronized;
  bool _exiting;
  ScopedFileRenameWriter(bool enabled,
                         log4cxx::LoggerPtr logger,
                         const std::string& filename,
                         const std::string& newfilename)
    : _enabled(enabled),
      _logger(logger),
      _filename(filename),
      _newfilename(newfilename),
      _open(true),
      _synchronized(false),
      _exiting(false)
  {
    if (_enabled)
    {
      try
      {
        _os.open(_filename.c_str());
      }
      catch (...)
      {
        _open = false;
        LOG4CXX_DEBUG(_logger,
                      "Unable to open resync cache notification orderno log [" << _filename
                                                                               << "] for output.");
      }
    }
  }

  ~ScopedFileRenameWriter()
  {
    if (_enabled)
    {
      if (_open && !_exiting)
      {
        bool okay(true);
        try
        {
          _os.close();
        }
        catch (...)
        {
          okay = false;
        }
        FULL_MEMORY_BARRIER;
        if (okay && rename(_filename.c_str(), _newfilename.c_str()) == 0)
        {
          LOG4CXX_DEBUG(_logger,
                        "Moved last processed cache notification log entry from "
                            << _filename << " " << _newfilename);
        }
        else
        {
          LOG4CXX_DEBUG(_logger,
                        "Unsuccessful close or unable to move last processed cache notification "
                        "log entry from "
                            << _filename << " " << _newfilename);
          LOG4CXX_DEBUG(_logger, "Removing cache notification log [" << _newfilename << "]");
          if (unlink(_newfilename.c_str()))
          {
            LOG4CXX_DEBUG(_logger,
                          "Error removing cache notification log [" << _newfilename << "]"
                                                                    << strerror(errno));
          }
        }
      }
    }
  }

  void write(const std::string& table, const ByEntity& byEntity)
  {
    if (LIKELY(_enabled))
    {
      errno = 0;
      LOG4CXX_DEBUG(_logger,
                    "Writing Last processed cache notification orderno restart log for table ["
                        << table << "] to [" << _filename << "]");

      ByEntityConstIterator i(byEntity.begin());
      ByEntityConstIterator e(byEntity.end());
      if (LIKELY(_open))
      {
        for (; i != e; i++)
        {
          try
          {
            _os << table << ',' << i->first << ',' << i->second << std::endl;
            ;
          }
          catch (...)
          {
            if (!_os)
            {
              _open = false;
              try
              {
                _os.close();
              }
              catch (...)
              {
              }
              LOG4CXX_DEBUG(_logger, "Last processed log entry write failed.");
            }
          }
        }
      }
    }
  }

  bool synchronized(bool empty)
  {
    _synchronized = empty;
    LOG4CXX_INFO(_logger, "synchronized [" << (_synchronized ? "true" : "false") << "]");
    return _synchronized;
  }
  bool exiting(bool e)
  {
    _exiting = e;
    LOG4CXX_INFO(_logger, "exiting [" << (_exiting ? "true" : "false") << "]");
    return _exiting;
  }
};

struct ScopedFileRenameLoad
{
  typedef std::tr1::unordered_map<std::string, long> ByEntity;
  typedef ByEntity::iterator ByEntityIterator;
  typedef ByEntity::const_iterator ByEntityConstIterator;

  typedef std::tr1::unordered_map<std::string, ByEntity> ByCacheEntity;
  typedef ByCacheEntity::iterator ByCacheEntityIterator;
  typedef ByCacheEntity::const_iterator ByCacheEntityConstIterator;

  bool _enabled;
  log4cxx::LoggerPtr _logger;
  std::string _filename;
  std::string _newfilename;
  uint64_t _lifetime;
  ScopedFileRenameLoad(bool enabled,
                       log4cxx::LoggerPtr logger,
                       const std::string& filename,
                       const std::string& newfilename,
                       uint64_t lifetime = 3600 // seconds
                       )
    : _enabled(enabled),
      _logger(logger),
      _filename(filename),
      _newfilename(newfilename),
      _lifetime(lifetime)
  {
  }

  ~ScopedFileRenameLoad() {}

  void load(ByCacheEntity& byCacheEntity)
  {
    // Refuse to keep versions older than 6 * _processingDelay seconds.
    struct stat st;
    DateTime age;
    DateTime oldest(localTime().subtractSeconds(_lifetime));
    int rc = stat(_filename.c_str(), &st);

    if (rc == 0)
    {
      struct tm* tminfo = localtime(&(st.st_mtime));
      age = DateTime(static_cast<uint16_t>(tminfo->tm_year + 1900),
                     static_cast<uint16_t>(tminfo->tm_mon + 1),
                     static_cast<uint16_t>(tminfo->tm_mday),
                     tminfo->tm_hour,
                     tminfo->tm_min,
                     tminfo->tm_sec);

      LOG4CXX_DEBUG(_logger,
                    "Reload file[" << _filename << "] max age " << _lifetime << " seconds old.");

      if (age < oldest)
      {
        LOG4CXX_DEBUG(_logger,
                      "Reload file[" << _filename << "] with timestamp " << age << " is too old.");
        unlink(_filename.c_str());
        _enabled = false;
      }
    }

    if (_enabled)
    {
      errno = 0;
      if (rename(_filename.c_str(), _newfilename.c_str()) == 0)
      {
        LOG4CXX_DEBUG(_logger,
                      "Rename resync cache notification log [" << _filename << "] and load");
        std::ifstream file(_newfilename.c_str());
        if (file)
        {
          std::string line;
          while (file && getline(file, line))
          {
            std::vector<std::string> list;
            boost::split(list, line, boost::is_any_of(","));
            if (list.size() >= 3)
            {
              std::string& table(list[0]);
              std::string& entity(list[1]);
              long orderno(atol(list[2].c_str()));
              byCacheEntity[table][entity] = orderno;
              LOG4CXX_INFO(_logger,
                           "Cache Entity Order No Resync [" << table << ',' << entity << ','
                                                            << orderno << ']');
            }
          }
        }
      }
      else
      {
        LOG4CXX_DEBUG(_logger,
                      "Unprocessed orderno log [" << _filename << "] error in rename to [ "
                                                  << _newfilename << "] error[ " << errno << "] "
                                                  << strerror(errno));
      }
      unlink(_newfilename.c_str());
    }
  }
};

template <typename Query>
struct CacheNotifyOrderNoHelper
{
  DBAdapterPool::pointer_type& adapter;
  std::string table;
  volatile bool& exiting;
  OrderNosByEntityType& lastOrderNos;
  ScopedFileRenameWriter& resyncLog;
  CacheNotifyAdapter& cn;
  DateTime& priorCutoff;
  DateTime& lastCutoff;
  int pollsize;
  bool resync;
  CacheNotifyOrderNoHelper(DBAdapterPool::pointer_type& adapter,
                           const std::string& table,
                           volatile bool& exiting,
                           OrderNosByEntityType& lastOrderNos,
                           ScopedFileRenameWriter& resyncLog,
                           CacheNotifyAdapter& cn,
                           DateTime& priorCutoff,
                           DateTime& lastCutoff,
                           int pollsize,
                           bool resync)
    : adapter(adapter),
      table(table),
      exiting(exiting),
      lastOrderNos(lastOrderNos),
      resyncLog(resyncLog),
      cn(cn),
      priorCutoff(priorCutoff),
      lastCutoff(lastCutoff),
      pollsize(pollsize),
      resync(resync)
  {
  }

  template <typename Executor>
  void operator()(Service& service, Executor execute, bool historical = false)
  {
    if (LIKELY(!exiting))
    {
      std::vector<tse::CacheNotifyInfo> infos;
      const DateTime& cutoff(cn.nextCutoff(lastCutoff, resync));
      Query query(adapter->getAdapter());

      (query.*
       execute)(infos, CacheNotifyAdapter::map2Vector(lastOrderNos), priorCutoff, cutoff, pollsize);
      if (!exiting && infos.size())
      {
        bool recordLast(true);
        cn.processByOrderNo(service, table, infos, lastOrderNos, historical, recordLast);
        resyncLog.write(table, lastOrderNos);
      }
    }
  }
};

template <typename Query>
struct MissingOrderNoHelper
{
  DBAdapterPool::pointer_type adapter;
  std::string table;
  log4cxx::LoggerPtr logger;

  MissingOrderNoHelper(DBAdapterPool::pointer_type adapter,
                       const std::string& table,
                       log4cxx::LoggerPtr logger)
    : adapter(adapter), table(table), logger(logger)
  {
  }

  template <typename Executor>
  void operator()(Service& service,
                  Executor execute,
                  ProcessedOrderNos& missedNotifications,
                  CacheNotifyAdapter& cn,
                  bool historical = false)
  {
    // max 100 items per cache per iteration in blocks of 10 ordernos
    std::vector<tse::CacheNotifyInfo> infos;
    size_t count(0);
    const std::vector<int64_t>& allMissed(CacheNotifyAdapter::set2Vector(missedNotifications));
    {
      size_t blocks(10);
      size_t blocksize(10);
      // i = 0 : test for i < 10 [ blocksize ]
      // i = 1 : test for all < 20 [ 1 * blocksize + blocksize ]
      bool done(!allMissed.size());
      if (!done)
      {
        for (size_t i = 0; !done && i < blocks; i++)
        {
          std::vector<int64_t> missed;
          // i =  0 j =  0 -- i  = 0 j  =  9
          // i =  1 j = 10 -- i  = 1 j  = 19
          // i =  2 j = 20 -- i  = 2 j  = 29
          for (size_t j = i * 10; j < i * blocksize + blocksize && j < allMissed.size(); j++)
          {
            missed.push_back(allMissed[j]);
            missedNotifications.erase(allMissed[j]);
          }

          done = !missed.size();
          if (!done)
          {
            Query query(adapter->getAdapter());
            (query.*execute)(infos, missed);
            count += missed.size();
          }
        }
      }
    }
    if (infos.size())
    {
      OrderNosByEntityType empty;
      bool recordLast(false);
      cn.processByOrderNo(service, table, infos, empty, historical, recordLast);
      LOG4CXX_INFO(logger,
                   "Redo [" << count << "] entries for table [" << table
                            << "] with remaining total [" << missedNotifications.size() << "]");
    }
  }
};
}

class CacheNotifyAdapter::GetFlushIntervals
{
public:
  GetFlushIntervals(CacheNotifyAdapter& cacheNotifyAdapter)
    : _cacheNotifyAdapter(cacheNotifyAdapter)
  {
    _now = DateTime::localTime();
  }

  ~GetFlushIntervals() {}

  void operator()(std::pair<const std::string, CacheControl*>& p)
  {
    const std::string& cacheName = p.first;
    int secs = CacheManager::instance().cacheFlushInterval(cacheName);
    if (secs > 0)
    {
      _cacheNotifyAdapter._nextFlushTime[cacheName] = _now.addSeconds(secs);
    }
  }

private:
  CacheNotifyAdapter& _cacheNotifyAdapter;
  DateTime _now;
};

bool
CacheNotifyAdapter::initialize()
{
  // get config parameters
  _pollInterval = pollInterval.getValue();
  _pollSize = pollSize.getValue();

  _useOrderNo = useOrderNo.getValue();
  LOG4CXX_INFO(_logger, "[CACHE_ADP] USE_ORDERNO[" << _useOrderNo << "]");
  if (_useOrderNo)
  {
    _usePartitionedQuery = usePartitionedQuery.getValue();
    LOG4CXX_INFO(_logger, "[CACHE_ADP] USE_PARTITIONED_ORDERNO[" << _usePartitionedQuery << "]");

    _redoMissingOrderNos = redoMissingOrderNos.getValue();
    LOG4CXX_INFO(_logger, "[CACHE_ADP] REDO_MISSING_ORDERNO[" << _redoMissingOrderNos << "]");

    _redoMissingOrderNoFile = directory.getValue() + "/" + redoMissingOrderNoFile.getValue();
    LOG4CXX_INFO(_logger,
                 "[CACHE_ADP] REDO_MISSING_ORDERNO_FILE[" << _redoMissingOrderNoFile << "]");

    _resyncFromLast = resyncFromLast.getValue();
    LOG4CXX_INFO(_logger, "[CACHE_ADP] RESYNC_FROM_LAST_ORDERNO[" << _resyncFromLast << "]");

    _resyncFromLastFile = directory.getValue() + "/" + resyncFromLastFile.getValue();

    LOG4CXX_INFO(_logger,
                 "[CACHE_ADP] RESYNC_FROM_LAST_ORDERNO_FILE[" << _resyncFromLastFile << "]");
  } // _useOrderNo
  ////////////////////////////////////////////////////////////////////////
  // _useRefactorOrderNo
  // run with refactored objects
  ////////////////////////////////////////////////////////////////////////

  if (_resyncFromLast || _redoMissingOrderNos)
  {
    _useRefactorOrderNo = true;
  }

  CacheNotifyControl cacheControl(
      configFile.getValue(), _keyFields, _cacheIds, _historicalKeys, _historicalIds);
  cacheControl.parse();

  _processingDelay = QueryCacheNotificationGlobals::processingDelay();

  LOG4CXX_DEBUG(_logger, "PROCESSING_DELAY = " << _processingDelay);

  _threadAliveFile = threadAliveFile.getValue();

  if (_useOrderNo)
  {
    LOG4CXX_INFO(_logger, "Will resync Cache Notify IDs using last order number re-sync.");
    if (_resyncFromLast)
    {
      uint64_t lifetime = 6 * _processingDelay;

      ScopedFileRenameLoad loader(
          _resyncFromLast, _logger, _resyncFromLastFile, _resyncFromLastFile + ".work", lifetime);
      ScopedFileRenameLoad::ByCacheEntity byCacheEntity;
      loader.load(byCacheEntity);
      _lastFareOrderNosByEntity = byCacheEntity["Fare"];
      _lastRoutingOrderNosByEntity = byCacheEntity["Routing"];
      _lastRuleOrderNosByEntity = byCacheEntity["Rule"];
      _lastSupportOrderNosByEntity = byCacheEntity["Support"];
      _lastIntlOrderNosByEntity = byCacheEntity["Intl"];
      _lastMerchOrderNosByEntity = byCacheEntity["Merch"];
      _lastHistOrderNosByEntity = byCacheEntity["HistHist"];
      _lastFareHistOrderNosByEntity = byCacheEntity["FareHist"];
      _lastRoutingHistOrderNosByEntity = byCacheEntity["RoutingHist"];
      _lastRuleHistOrderNosByEntity = byCacheEntity["RuleHist"];
      _lastSupportHistOrderNosByEntity = byCacheEntity["SupportHist"];
      _lastIntlHistOrderNosByEntity = byCacheEntity["IntlHist"];
      _lastMerchHistOrderNosByEntity = byCacheEntity["MerchHist"];
    }
  }

  DateTime startFrom = Global::startTime();
  if (DISKCACHE.isActivated())
  {
    startFrom = DISKCACHE.getLastUpdateTime();
    if ((startFrom == DateTime::emptyDate()) || (DISKCACHE.isTooOld(startFrom)))
    {
      startFrom = Global::startTime();
    }
  }

  _cutoff = startFrom.subtractSeconds(_processingDelay);
  std::string messagePrefix("Using LDC's last update time [");

  if (!DISKCACHE.isActivated())
    messagePrefix = "Start [";

  LOG4CXX_INFO(_logger,
               messagePrefix << startFrom << "] minus processing delay [" << _processingDelay
                             << " seconds] = cutoff [" << _cutoff << "]");

  if (!_useOrderNo)
    updateLastNotifyIDs(_cutoff);

  _dbSyncToken = DataManager::getSyncToken();

  // get cache flush intervals
  CacheRegistry& cacheRegistry = CacheRegistry::instance();
  GetFlushIntervals func(*this);
  cacheRegistry.forEach(func);

  return true;
}

bool
CacheNotifyAdapter::run(Service& srv)
{
  bool rc = false;
  if (_useOrderNo)
  {
    if (_useRefactorOrderNo)
    {
      rc = runOrderNoRefactor(srv);
    }
    else
    {
      rc = runOrderNo(srv);
    }
  }
  else
    rc = runIdNo(srv);
  return rc;
}

namespace
{
std::mutex exitMutex;
std::condition_variable exitCondition;
}

bool
CacheNotifyAdapter::runIdNo(Service& srv)
{
  const MallocContextDisabler context;

  touchThreadAliveFile();

  std::string request;

  std::unique_lock<std::mutex> lock(exitMutex, std::defer_lock);
  lock.lock();

  while (!_exiting)
  {
    CacheTrx::processDelayedUpdates();
    try
    {
      try
      {
        exitCondition.wait_for(lock, std::chrono::seconds(_pollInterval));
      }
      catch (...)
      {
      }

      if (_exiting)
        return false;

      // Resync IDs if needed
      //
      resyncLastNotifyIDs();

      LOG4CXX_DEBUG(_logger, "Polling cache notify tables...");

      DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
      DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Default");

      std::vector<CacheNotifyInfo> infos;

      if (_exiting)
        return false;

      QueryGetAllFareCacheNotify allFare(dbAdapter->getAdapter());
      allFare.findAllFareCacheNotify(infos, static_cast<int32_t>(_lastFareNotify), _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        _lastFareNotify = process(srv, "Fare", infos, _lastFareOrderNosByEntity);

      if (_exiting)
        return false;

      QueryGetAllRoutingCacheNotify allRtg(dbAdapter->getAdapter());
      allRtg.findAllRoutingCacheNotify(infos, static_cast<int32_t>(_lastRoutingNotify), _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        _lastRoutingNotify = process(srv, "Routing", infos, _lastRoutingOrderNosByEntity);

      if (_exiting)
        return false;

      QueryGetAllRuleCacheNotify allRule(dbAdapter->getAdapter());
      allRule.findAllRuleCacheNotify(infos, static_cast<int32_t>(_lastRuleNotify), _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        _lastRuleNotify = process(srv, "Rule", infos, _lastRuleOrderNosByEntity);

      if (_exiting)
        return false;

      QueryGetAllSupportCacheNotify allSupp(dbAdapter->getAdapter());
      allSupp.findAllSupportCacheNotify(infos, static_cast<int32_t>(_lastSupportNotify), _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        _lastSupportNotify = process(srv, "Support", infos, _lastSupportOrderNosByEntity);

      if (_exiting)
        return false;

      QueryGetAllIntlCacheNotify allIntl(dbAdapter->getAdapter());
      allIntl.findAllIntlCacheNotify(infos, static_cast<int32_t>(_lastIntlNotify), _pollSize);
      if (!infos.empty())
        _lastIntlNotify = process(srv, "Intl", infos, _lastIntlOrderNosByEntity);

      if (_exiting)
        return false;

      if (merchCacheActivated.getValue())
      {
        QueryGetAllMerchandisingCacheNotify allMerch(dbAdapter->getAdapter());
        allMerch.findAllMerchandisingCacheNotify(
            infos, static_cast<int32_t>(_lastMerchNotify), _pollSize);
        if (!infos.empty())
          _lastMerchNotify = process(srv, "Merch", infos, _lastMerchOrderNosByEntity);

        if (_exiting)
          return false;
      }

      if (Global::allowHistorical())
      {
        DBAdapterPool::pointer_type dbHistAdapter = dbAdapterPool.get("Historical", true);
        QueryGetAllCacheNotifyHistorical allHist(dbHistAdapter->getAdapter());
        allHist.findAllCacheNotifyHistorical(
            infos, static_cast<int32_t>(_lastHistNotify), _pollSize);
        if (!infos.empty())
        {
          _lastHistNotify = process(srv, "Hist", infos, _lastHistOrderNosByEntity, true);
        }
        if (_exiting)
          return false;
        QueryGetAllFareCacheNotify allFareHist(dbHistAdapter->getAdapter());
        allFareHist.findAllFareCacheNotify(
            infos, static_cast<int32_t>(_lastFareHistNotify), _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          _lastFareHistNotify =
              process(srv, "FareHist", infos, _lastFareHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllRoutingCacheNotify allRoutingHist(dbHistAdapter->getAdapter());
        allRoutingHist.findAllRoutingCacheNotify(
            infos, static_cast<int32_t>(_lastRoutingHistNotify), _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          _lastRoutingHistNotify =
              process(srv, "RoutingHist", infos, _lastRoutingHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllRuleCacheNotify allRuleHist(dbHistAdapter->getAdapter());
        allRuleHist.findAllRuleCacheNotify(
            infos, static_cast<int32_t>(_lastRuleHistNotify), _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          _lastRuleHistNotify =
              process(srv, "RuleHist", infos, _lastRuleHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllSupportCacheNotify allSupportHist(dbHistAdapter->getAdapter());
        allSupportHist.findAllSupportCacheNotify(
            infos, static_cast<int32_t>(_lastSupportHistNotify), _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          _lastSupportHistNotify =
              process(srv, "SupportHist", infos, _lastSupportHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllIntlCacheNotify allIntlHist(dbHistAdapter->getAdapter());
        allIntlHist.findAllIntlCacheNotify(
            infos, static_cast<int32_t>(_lastIntlHistNotify), _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          _lastIntlHistNotify =
              process(srv, "IntlHist", infos, _lastIntlHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        if (merchCacheActivated.getValue())
        {
          QueryGetAllMerchandisingCacheNotify allMerchHist(dbHistAdapter->getAdapter());
          allMerchHist.findAllMerchandisingCacheNotify(
              infos, static_cast<int32_t>(_lastMerchHistNotify), _pollSize);
          if (_exiting)
            return false;
          if (!infos.empty())
            _lastMerchHistNotify =
                process(srv, "MerchHist", infos, _lastMerchHistOrderNosByEntity, true);
          if (_exiting)
            return false;
        }
      }

      LOG4CXX_DEBUG(_logger, "Polling direct injection table...");
      if (_tseServer != nullptr)
      {
        _tseServer->siphonInjectedCacheNotifications(infos);
        if (_exiting)
          return false;
        if (!infos.empty())
        {
          LOG4CXX_DEBUG(_logger, "   --> FOUND " << infos.size());
          process(srv, "DirectInjection", infos, _directInjectionOrderNos);
        }
        else
        {
          LOG4CXX_DEBUG(_logger, "   --> NONE FOUND");
        }
        if (_exiting)
          return false;
      }
      else
      {
        LOG4CXX_DEBUG(_logger, "   --> NO TSESERVER AVAILABLE");
      }

      LOG4CXX_DEBUG(_logger, "Checking cache flush intervals...");
      clearStaleCaches(srv);

      try
      {
        BoundFareDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in BoundFareDAO::checkForUpdates");
      }
      try
      {
        MarketRoutingDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "std::exception" << e.what() << " in MarketRoutingDAO::checkForUpdates");
      }
      try
      {
        CacheDeleterBase::emptyTrash();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "CacheDeleterBase::emptyTrash():" << e.what());
      }
      try
      {
        DataManager& dm = DataManager::instance();
        dm.dbConnChanged();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "loadConfigValues():" << e.what());
      }
      LOG4CXX_DEBUG(_logger, "Sleep tight...");
    }
    catch (TSEException& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      LOG4CXX_ERROR(_logger, ex.where());
      LOG4CXX_ERROR(_logger, "TSEException caught and discarded; notify thread shouldn't die.");
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(_logger, "Exception: " << ex.code() << " " << ex.message());
      LOG4CXX_ERROR(_logger,
                    "ErrorResponseException caught and discarded; notify thread shouldn't die.");
    }
    catch (std::exception& e)
    {
      LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in CacheNotify::run");
    }
    catch (...)
    {
      LOG4CXX_ERROR(_logger, "unexpected exception in CacheNotifyAdapter::run");
    }

    touchThreadAliveFile();
  }

  return true;
}

const DateTime
CacheNotifyAdapter::nextCutoff(DateTime& last, const bool resync)
{
  DateTime restartFrom = last;
  if (UNLIKELY(!_usePartitionedQuery && resync))
  {
    restartFrom = last.subtractSeconds(_processingDelay);
    LOG4CXX_INFO(_logger,
                 "Reconnect causing resync. Resyncing Cache Notify using ["
                     << last << "] minus processing delay [" << _processingDelay
                     << " seconds] = restarting from [" << restartFrom << "]");
  }

  last = DateTime::localTime();
  return restartFrom;
}

void
CacheNotifyAdapter::nextEventSet(bool resync)
{
  if (resync)
  {
    _currentInterval = 0;
    for (auto& event : _allEvents)
      event.clear();
  }
  else
  {
    if (_intervals % INTERVAL_INSTANCES == 0)
    {
      AllCacheProcessed& prior = _allEvents[_currentInterval];
      prior.clear();
    }
  }
}

bool
CacheNotifyAdapter::processedPriorEvent(const std::string& cache, size_t orderno)
{
  bool rc = false;
  const int32_t ordn = static_cast<const int32_t>(orderno);

  for (auto& event : _allEvents)
  {
    if (event[cache].find(ordn) != event[cache].end())
    {
      LOG4CXX_DEBUG(_ordernoSkippedLogger, "allEvents," << cache << "," << ordn);
      rc = true;
      break;
    }
  }
  return rc;
}

bool
CacheNotifyAdapter::runOrderNo(Service& srv)
{
  const MallocContextDisabler context;

  touchThreadAliveFile();

  std::string request;

  TseSynchronizingValue syncToken = DataManager::getSyncToken();

  // Account for
  DateTime lastFareRunTime = _cutoff;
  DateTime lastRuleRunTime = _cutoff;
  DateTime lastRoutingRunTime = _cutoff;
  DateTime lastSupportRunTime = _cutoff;
  DateTime lastIntlRunTime = _cutoff;
  DateTime lastMerchRunTime = _cutoff;

  DateTime lastHistRunTime = _cutoff;
  DateTime lastFareHistRunTime = _cutoff;
  DateTime lastRuleHistRunTime = _cutoff;
  DateTime lastRoutingHistRunTime = _cutoff;
  DateTime lastSupportHistRunTime = _cutoff;
  DateTime lastIntlHistRunTime = _cutoff;
  DateTime lastMerchHistRunTime = _cutoff;

  DateTime priorFareRunTime = _cutoff;
  DateTime priorRuleRunTime = _cutoff;
  DateTime priorRoutingRunTime = _cutoff;
  DateTime priorSupportRunTime = _cutoff;
  DateTime priorIntlRunTime = _cutoff;
  DateTime priorMerchRunTime = _cutoff;

  DateTime priorHistRunTime = _cutoff;
  DateTime priorFareHistRunTime = _cutoff;
  DateTime priorRuleHistRunTime = _cutoff;
  DateTime priorRoutingHistRunTime = _cutoff;
  DateTime priorSupportHistRunTime = _cutoff;
  DateTime priorIntlHistRunTime = _cutoff;
  DateTime priorMerchHistRunTime = _cutoff;

  bool resync = false;

  std::unique_lock<std::mutex> lock(exitMutex, std::defer_lock);
  lock.lock();

  while (!_exiting)
  {
    try
    {
      if (_exiting)
        return false;

      LOG4CXX_DEBUG(_logger, "runOrderNo: Polling cache notify tables...");

      DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
      DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Default");

      std::vector<CacheNotifyInfo> infos;

      if (_exiting)
        return false;

      // The initial cutoff may be set in initialize to ldc last
      // processed time.  Update last run before every loop and
      // increment the iteration count.
      _intervals++;
      QueryGetAllFareCacheNotifyOrderNo allFare(dbAdapter->getAdapter());
      allFare.findAllFareCacheNotifyOrderNo(infos,
                                            map2Vector(_lastFareOrderNosByEntity),
                                            priorFareRunTime,
                                            nextCutoff(lastFareRunTime, resync),
                                            _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        processByOrderNo(srv, "Fare", infos, _lastFareOrderNosByEntity);
      if (_exiting)
        return false;

      QueryGetAllRoutingCacheNotifyOrderNo allRtg(dbAdapter->getAdapter());
      allRtg.findAllRoutingCacheNotifyOrderNo(infos,
                                              map2Vector(_lastRoutingOrderNosByEntity),
                                              priorRoutingRunTime,
                                              nextCutoff(lastRoutingRunTime, resync),
                                              _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        processByOrderNo(srv, "Routing", infos, _lastRoutingOrderNosByEntity);
      if (_exiting)
        return false;

      QueryGetAllRuleCacheNotifyOrderNo allRule(dbAdapter->getAdapter());
      allRule.findAllRuleCacheNotifyOrderNo(infos,
                                            map2Vector(_lastRuleOrderNosByEntity),
                                            priorRuleRunTime,
                                            nextCutoff(lastRuleRunTime, resync),
                                            _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        processByOrderNo(srv, "Rule", infos, _lastRuleOrderNosByEntity);
      if (_exiting)
        return false;

      QueryGetAllSupportCacheNotifyOrderNo allSupp(dbAdapter->getAdapter());
      allSupp.findAllSupportCacheNotifyOrderNo(infos,
                                               map2Vector(_lastSupportOrderNosByEntity),
                                               priorSupportRunTime,
                                               nextCutoff(lastSupportRunTime, resync),
                                               _pollSize);
      if (_exiting)
        return false;

      if (!infos.empty())
        processByOrderNo(srv, "Support", infos, _lastSupportOrderNosByEntity);
      if (_exiting)
        return false;

      QueryGetAllIntlCacheNotifyOrderNo allIntl(dbAdapter->getAdapter());
      allIntl.findAllIntlCacheNotifyOrderNo(infos,
                                            map2Vector(_lastIntlOrderNosByEntity),
                                            priorIntlRunTime,
                                            nextCutoff(lastIntlRunTime, resync),
                                            _pollSize);
      if (!infos.empty())
        processByOrderNo(srv, "Intl", infos, _lastIntlOrderNosByEntity);
      if (_exiting)
        return false;

      if (merchCacheActivated.getValue())
      {
        QueryGetAllMerchandisingCacheNotifyOrderNo allMerch(dbAdapter->getAdapter());
        allMerch.findAllMerchandisingCacheNotifyOrderNo(infos,
                                                        map2Vector(_lastMerchOrderNosByEntity),
                                                        priorMerchRunTime,
                                                        nextCutoff(lastMerchRunTime, resync),
                                                        _pollSize);
        if (!infos.empty())
          processByOrderNo(srv, "Merch", infos, _lastMerchOrderNosByEntity);
        if (_exiting)
          return false;
      }

      if (Global::allowHistorical())
      {
        DBAdapterPool::pointer_type dbHistAdapter = dbAdapterPool.get("Historical", true);
        QueryGetAllCacheNotifyOrderNoHistorical allHist(dbHistAdapter->getAdapter());
        allHist.findAllCacheNotifyOrderNoHistorical(infos,
                                                    map2Vector(_lastHistOrderNosByEntity),
                                                    priorHistRunTime,
                                                    nextCutoff(lastHistRunTime, resync),
                                                    _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "HistHist", infos, _lastHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllFareCacheNotifyOrderNo allFareHist(dbHistAdapter->getAdapter());
        allFareHist.findAllFareCacheNotifyOrderNo(infos,
                                                  map2Vector(_lastFareHistOrderNosByEntity),
                                                  priorFareHistRunTime,
                                                  nextCutoff(lastFareHistRunTime, resync),
                                                  _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "FareHist", infos, _lastFareHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllRoutingCacheNotifyOrderNo allRoutingHist(dbHistAdapter->getAdapter());
        allRoutingHist.findAllRoutingCacheNotifyOrderNo(
            infos,
            map2Vector(_lastRoutingHistOrderNosByEntity),
            priorRoutingHistRunTime,
            nextCutoff(lastRoutingHistRunTime, resync),
            _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "RoutingHist", infos, _lastRoutingHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllRuleCacheNotifyOrderNo allRuleHist(dbHistAdapter->getAdapter());
        allRuleHist.findAllRuleCacheNotifyOrderNo(infos,
                                                  map2Vector(_lastRuleHistOrderNosByEntity),
                                                  priorRuleHistRunTime,
                                                  nextCutoff(lastRuleHistRunTime, resync),
                                                  _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "RuleHist", infos, _lastRuleHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllSupportCacheNotifyOrderNo allSupportHist(dbHistAdapter->getAdapter());
        allSupportHist.findAllSupportCacheNotifyOrderNo(
            infos,
            map2Vector(_lastSupportHistOrderNosByEntity),
            priorSupportHistRunTime,
            nextCutoff(lastSupportHistRunTime, resync),
            _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "SupportHist", infos, _lastSupportHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        QueryGetAllIntlCacheNotifyOrderNo allIntlHist(dbHistAdapter->getAdapter());
        allIntlHist.findAllIntlCacheNotifyOrderNo(infos,
                                                  map2Vector(_lastIntlHistOrderNosByEntity),
                                                  priorIntlHistRunTime,
                                                  nextCutoff(lastIntlHistRunTime, resync),
                                                  _pollSize);
        if (_exiting)
          return false;
        if (!infos.empty())
          processByOrderNo(srv, "IntlHist", infos, _lastIntlHistOrderNosByEntity, true);
        if (_exiting)
          return false;

        if (merchCacheActivated.getValue())
        {
          QueryGetAllMerchandisingCacheNotifyOrderNo allMerchHist(dbHistAdapter->getAdapter());
          allMerchHist.findAllMerchandisingCacheNotifyOrderNo(
              infos,
              map2Vector(_lastMerchHistOrderNosByEntity),
              priorMerchHistRunTime,
              nextCutoff(lastMerchHistRunTime, resync),
              _pollSize);
          if (_exiting)
            return false;
          if (!infos.empty())
            processByOrderNo(srv, "MerchHist", infos, _lastMerchHistOrderNosByEntity, true);
          if (_exiting)
            return false;
        }
      }

      LOG4CXX_DEBUG(_logger, "Polling direct injection table...");
      if (_tseServer != nullptr)
      {
        _tseServer->siphonInjectedCacheNotifications(infos);
        if (_exiting)
          return false;
        if (!infos.empty())
        {
          LOG4CXX_DEBUG(_logger, "   --> FOUND " << infos.size());
          processByOrderNo(srv, "DirectInjection", infos, _directInjectionOrderNos);
        }
        else
        {
          LOG4CXX_DEBUG(_logger, "   --> NONE FOUND");
        }
        if (_exiting)
          return false;
      }
      else
      {
        LOG4CXX_DEBUG(_logger, "   --> NO TSESERVER AVAILABLE");
      }

      LOG4CXX_DEBUG(_logger, "Checking cache flush intervals...");
      clearStaleCaches(srv);

      try
      {
        BoundFareDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in BoundFareDAO::checkForUpdates");
      }
      try
      {
        MarketRoutingDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "std::exception" << e.what() << " in MarketRoutingDAO::checkForUpdates");
      }
      try
      {
        CacheDeleterBase::emptyTrash();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "CacheDeleterBase::emptyTrash():" << e.what());
      }
      try
      {
        DataManager& dm = DataManager::instance();
        dm.dbConnChanged();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "loadConfigValues():" << e.what());
      }
      LOG4CXX_DEBUG(_logger, "Sleep tight...");
    }
    catch (TSEException& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      LOG4CXX_ERROR(_logger, ex.where());
      LOG4CXX_ERROR(_logger, "TSEException caught and discarded; notify thread shouldn't die.");
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(_logger, "Exception: " << ex.code() << " " << ex.message());
      LOG4CXX_ERROR(_logger,
                    "ErrorResponseException caught and discarded; notify thread shouldn't die.");
    }
    catch (std::exception& e)
    {
      LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in CacheNotify::run");
    }
    catch (...)
    {
      LOG4CXX_ERROR(_logger, "unexpected exception in CacheNotifyAdapter::run");
    }

    touchThreadAliveFile();

    CacheTrx::processDelayedUpdates();

    try
    {
      exitCondition.wait_for(lock, std::chrono::seconds(_pollInterval));
    }
    catch (...)
    {
    }
    try
    {
      resync = _dbSyncToken != syncToken;
      nextEventSet(resync);
    }
    catch (...)
    {
    }
  }
  return true;
}

bool
CacheNotifyAdapter::runOrderNoRefactor(Service& srv)
{
  const MallocContextDisabler context;

  touchThreadAliveFile();

  std::string request;

  TseSynchronizingValue syncToken = DataManager::getSyncToken();

  DateTime lastFareRunTime = _cutoff;
  DateTime lastRuleRunTime = _cutoff;
  DateTime lastRoutingRunTime = _cutoff;
  DateTime lastSupportRunTime = _cutoff;
  DateTime lastIntlRunTime = _cutoff;
  DateTime lastMerchRunTime = _cutoff;

  DateTime lastHistRunTime = _cutoff;
  DateTime lastFareHistRunTime = _cutoff;
  DateTime lastRuleHistRunTime = _cutoff;
  DateTime lastRoutingHistRunTime = _cutoff;
  DateTime lastSupportHistRunTime = _cutoff;
  DateTime lastIntlHistRunTime = _cutoff;
  DateTime lastMerchHistRunTime = _cutoff;

  if (_usePartitionedQuery)
  {
    DateTime now = DateTime::localTime();

    lastFareRunTime = now;
    lastRuleRunTime = now;
    lastRoutingRunTime = now;
    lastSupportRunTime = now;
    lastIntlRunTime = now;
    lastMerchRunTime = now;

    lastHistRunTime = now;
    lastFareHistRunTime = now;
    lastRuleHistRunTime = now;
    lastRoutingHistRunTime = now;
    lastSupportHistRunTime = now;
    lastIntlHistRunTime = now;
    lastMerchHistRunTime = now;
  }

  DateTime priorFareRunTime = _cutoff;
  DateTime priorRuleRunTime = _cutoff;
  DateTime priorRoutingRunTime = _cutoff;
  DateTime priorSupportRunTime = _cutoff;
  DateTime priorIntlRunTime = _cutoff;
  DateTime priorMerchRunTime = _cutoff;

  DateTime priorHistRunTime = _cutoff;
  DateTime priorFareHistRunTime = _cutoff;
  DateTime priorRuleHistRunTime = _cutoff;
  DateTime priorRoutingHistRunTime = _cutoff;
  DateTime priorSupportHistRunTime = _cutoff;
  DateTime priorIntlHistRunTime = _cutoff;
  DateTime priorMerchHistRunTime = _cutoff;

  bool resync = false;
  ScopedFileRenameReader::ByCache missedByCache;

  std::unique_lock<std::mutex> lock(exitMutex, std::defer_lock);
  lock.lock();

  while (!_exiting)
  {
    try
    {
      if (_exiting)
        return false;

      LOG4CXX_DEBUG(_logger, "runOrderNoRefactor: Polling cache notify tables...");

      DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
      DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Default");

      std::vector<CacheNotifyInfo> infos;

      if (_exiting)
        return false;

      // The initial cutoff may be set in initialize to ldc last
      // processed time.  Update last run before every loop and
      // increment the iteration count. Used to monitor duplicates.
      _intervals++;

      // Collect the replay [ missed ] ordernos from the monitor process.
      ScopedFileRenameReader reprocess(_redoMissingOrderNos,
                                       _logger,
                                       _redoMissingOrderNoFile,
                                       _redoMissingOrderNoFile + ".work");
      reprocess.load(missedByCache); // reload or reuse prior items not found, processing up to 100
      // per cache per minute.

      ScopedFileRenameWriter resyncLog(
          _resyncFromLast, _logger, _resyncFromLastFile + ".work", _resyncFromLastFile);
      if (_redoMissingOrderNos)
      {
        MissingOrderNoHelper<QueryGetMissingFareCacheNotifyOrderNo> fareHelper(
            dbAdapter, "Fare", _logger);
        fareHelper(srv,
                   &QueryGetMissingFareCacheNotifyOrderNo::findMissingFareCacheNotifyOrderNo,
                   missedByCache["Fare"],
                   *this);
        if (_exiting)
          return false;
        MissingOrderNoHelper<QueryGetMissingRoutingCacheNotifyOrderNo> routingHelper(
            dbAdapter, "Routing", _logger);
        routingHelper(
            srv,
            &QueryGetMissingRoutingCacheNotifyOrderNo::findMissingRoutingCacheNotifyOrderNo,
            missedByCache["Routing"],
            *this);
        if (_exiting)
          return false;
        MissingOrderNoHelper<QueryGetMissingRuleCacheNotifyOrderNo> ruleHelper(
            dbAdapter, "Rule", _logger);
        ruleHelper(srv,
                   &tse::QueryGetMissingRuleCacheNotifyOrderNo::findMissingRuleCacheNotifyOrderNo,
                   missedByCache["Rule"],
                   *this);
        if (_exiting)
          return false;
        MissingOrderNoHelper<QueryGetMissingSupportCacheNotifyOrderNo> supportHelper(
            dbAdapter, "Support", _logger);
        supportHelper(
            srv,
            &tse::QueryGetMissingSupportCacheNotifyOrderNo::findMissingSupportCacheNotifyOrderNo,
            missedByCache["Support"],
            *this);
        if (_exiting)
          return false;
        MissingOrderNoHelper<QueryGetMissingIntlCacheNotifyOrderNo> intlHelper(
            dbAdapter, "Intl", _logger);
        intlHelper(srv,
                   &tse::QueryGetMissingIntlCacheNotifyOrderNo::findMissingIntlCacheNotifyOrderNo,
                   missedByCache["Intl"],
                   *this);
        if (_exiting)
          return false;
        if (merchCacheActivated.getValue())
        {
          MissingOrderNoHelper<QueryGetMissingMerchandisingCacheNotifyOrderNo> merchHelper(
              dbAdapter, "Merch", _logger);
          merchHelper(srv,
                      &tse::QueryGetMissingMerchandisingCacheNotifyOrderNo::
                          findMissingMerchandisingCacheNotifyOrderNo,
                      missedByCache["Merch"],
                      *this);
          if (_exiting)
            return false;
        }
      }

      CacheNotifyOrderNoHelper<QueryGetAllFareCacheNotifyOrderNo> fare(dbAdapter,
                                                                       "Fare",
                                                                       _exiting,
                                                                       _lastFareOrderNosByEntity,
                                                                       resyncLog,
                                                                       *this,
                                                                       priorFareRunTime,
                                                                       lastFareRunTime,
                                                                       _pollSize,
                                                                       resync);
      fare(srv, &QueryGetAllFareCacheNotifyOrderNo::findAllFareCacheNotifyOrderNo);
      if (_exiting)
        return false;

      CacheNotifyOrderNoHelper<QueryGetAllRoutingCacheNotifyOrderNo> routing(
          dbAdapter,
          "Routing",
          _exiting,
          _lastRoutingOrderNosByEntity,
          resyncLog,
          *this,
          priorRoutingRunTime,
          lastRoutingRunTime,
          _pollSize,
          resync);
      routing(srv, &QueryGetAllRoutingCacheNotifyOrderNo::findAllRoutingCacheNotifyOrderNo);
      if (_exiting)
        return false;

      CacheNotifyOrderNoHelper<QueryGetAllRuleCacheNotifyOrderNo> rule(dbAdapter,
                                                                       "Rule",
                                                                       _exiting,
                                                                       _lastRuleOrderNosByEntity,
                                                                       resyncLog,
                                                                       *this,
                                                                       priorRuleRunTime,
                                                                       lastRuleRunTime,
                                                                       _pollSize,
                                                                       resync);
      rule(srv, &QueryGetAllRuleCacheNotifyOrderNo::findAllRuleCacheNotifyOrderNo);
      if (_exiting)
        return false;

      CacheNotifyOrderNoHelper<QueryGetAllSupportCacheNotifyOrderNo> support(
          dbAdapter,
          "Support",
          _exiting,
          _lastSupportOrderNosByEntity,
          resyncLog,
          *this,
          priorSupportRunTime,
          lastSupportRunTime,
          _pollSize,
          resync);
      support(srv, &QueryGetAllSupportCacheNotifyOrderNo::findAllSupportCacheNotifyOrderNo);
      if (_exiting)
        return false;

      CacheNotifyOrderNoHelper<QueryGetAllIntlCacheNotifyOrderNo> intl(dbAdapter,
                                                                       "Intl",
                                                                       _exiting,
                                                                       _lastIntlOrderNosByEntity,
                                                                       resyncLog,
                                                                       *this,
                                                                       priorIntlRunTime,
                                                                       lastIntlRunTime,
                                                                       _pollSize,
                                                                       resync);
      intl(srv, &QueryGetAllIntlCacheNotifyOrderNo::findAllIntlCacheNotifyOrderNo);
      if (_exiting)
        return false;

      if (merchCacheActivated.getValue())
      {
        CacheNotifyOrderNoHelper<QueryGetAllMerchandisingCacheNotifyOrderNo> merch(
            dbAdapter,
            "Merch",
            _exiting,
            _lastMerchOrderNosByEntity,
            resyncLog,
            *this,
            priorMerchRunTime,
            lastMerchRunTime,
            _pollSize,
            resync);
        merch(srv,
              &QueryGetAllMerchandisingCacheNotifyOrderNo::findAllMerchandisingCacheNotifyOrderNo);
        if (_exiting)
          return false;
      }

      if (Global::allowHistorical())
      {
        DBAdapterPool::pointer_type dbHistAdapter = dbAdapterPool.get("Historical", true);

        bool historical(true);
        if (_redoMissingOrderNos)
        {
          MissingOrderNoHelper<QueryGetMissingCacheNotifyOrderNoHistorical> histHelper(
              dbHistAdapter, "HistHist", _logger);
          histHelper(
              srv,
              &QueryGetMissingCacheNotifyOrderNoHistorical::findMissingCacheNotifyOrderNoHistorical,
              missedByCache["HistHist"],
              *this,
              historical);
          if (_exiting)
            return false;
          MissingOrderNoHelper<QueryGetMissingFareCacheNotifyOrderNo> fareHistHelper(
              dbHistAdapter, "FareHist", _logger);
          fareHistHelper(srv,
                         &QueryGetMissingFareCacheNotifyOrderNo::findMissingFareCacheNotifyOrderNo,
                         missedByCache["FareHist"],
                         *this,
                         historical);
          if (_exiting)
            return false;

          MissingOrderNoHelper<QueryGetMissingRoutingCacheNotifyOrderNo> routingHistHelper(
              dbHistAdapter, "RoutingHist", _logger);
          routingHistHelper(
              srv,
              &QueryGetMissingRoutingCacheNotifyOrderNo::findMissingRoutingCacheNotifyOrderNo,
              missedByCache["RoutingHist"],
              *this,
              historical);
          if (_exiting)
            return false;

          MissingOrderNoHelper<QueryGetMissingRuleCacheNotifyOrderNo> ruleHistHelper(
              dbHistAdapter, "RuleHist", _logger);
          ruleHistHelper(srv,
                         &QueryGetMissingRuleCacheNotifyOrderNo::findMissingRuleCacheNotifyOrderNo,
                         missedByCache["RuleHist"],
                         *this,
                         historical);
          if (_exiting)
            return false;

          MissingOrderNoHelper<QueryGetMissingSupportCacheNotifyOrderNo> supportHistHelper(
              dbHistAdapter, "SupportHist", _logger);
          supportHistHelper(
              srv,
              &QueryGetMissingSupportCacheNotifyOrderNo::findMissingSupportCacheNotifyOrderNo,
              missedByCache["SupportHist"],
              *this,
              historical);
          if (_exiting)
            return false;

          MissingOrderNoHelper<QueryGetMissingIntlCacheNotifyOrderNo> intlHistHelper(
              dbHistAdapter, "IntlHist", _logger);
          intlHistHelper(srv,
                         &QueryGetMissingIntlCacheNotifyOrderNo::findMissingIntlCacheNotifyOrderNo,
                         missedByCache["IntlHist"],
                         *this,
                         historical);
          if (_exiting)
            return false;

          if (merchCacheActivated.getValue())
          {
            MissingOrderNoHelper<QueryGetMissingMerchandisingCacheNotifyOrderNo> merchHistHelper(
                dbHistAdapter, "MerchHist", _logger);
            merchHistHelper(srv,
                            &QueryGetMissingMerchandisingCacheNotifyOrderNo::
                                findMissingMerchandisingCacheNotifyOrderNo,
                            missedByCache["MerchHist"],
                            *this,
                            historical);
            if (_exiting)
              return false;
          }
        }

        CacheNotifyOrderNoHelper<QueryGetAllCacheNotifyOrderNoHistorical> hist(
            dbHistAdapter,
            "HistHist",
            _exiting,
            _lastHistOrderNosByEntity,
            resyncLog,
            *this,
            priorHistRunTime,
            lastHistRunTime,
            _pollSize,
            resync);
        hist(srv,
             &QueryGetAllCacheNotifyOrderNoHistorical::findAllCacheNotifyOrderNoHistorical,
             historical);
        if (_exiting)
          return false;

        CacheNotifyOrderNoHelper<QueryGetAllFareCacheNotifyOrderNo> fare(
            dbHistAdapter,
            "FareHist",
            _exiting,
            _lastFareHistOrderNosByEntity,
            resyncLog,
            *this,
            priorFareHistRunTime,
            lastFareHistRunTime,
            _pollSize,
            resync);
        fare(srv, &QueryGetAllFareCacheNotifyOrderNo::findAllFareCacheNotifyOrderNo, historical);
        if (_exiting)
          return false;

        CacheNotifyOrderNoHelper<QueryGetAllRoutingCacheNotifyOrderNo> routing(
            dbHistAdapter,
            "RoutingHist",
            _exiting,
            _lastRoutingHistOrderNosByEntity,
            resyncLog,
            *this,
            priorRoutingHistRunTime,
            lastRoutingHistRunTime,
            _pollSize,
            resync);
        routing(srv,
                &QueryGetAllRoutingCacheNotifyOrderNo::findAllRoutingCacheNotifyOrderNo,
                historical);
        if (_exiting)
          return false;

        CacheNotifyOrderNoHelper<QueryGetAllRuleCacheNotifyOrderNo> rule(
            dbHistAdapter,
            "RuleHist",
            _exiting,
            _lastRuleHistOrderNosByEntity,
            resyncLog,
            *this,
            priorRuleHistRunTime,
            lastRuleHistRunTime,
            _pollSize,
            resync);
        rule(srv, &QueryGetAllRuleCacheNotifyOrderNo::findAllRuleCacheNotifyOrderNo, historical);
        if (_exiting)
          return false;

        CacheNotifyOrderNoHelper<QueryGetAllSupportCacheNotifyOrderNo> support(
            dbHistAdapter,
            "SupportHist",
            _exiting,
            _lastSupportHistOrderNosByEntity,
            resyncLog,
            *this,
            priorSupportHistRunTime,
            lastSupportHistRunTime,
            _pollSize,
            resync);
        support(srv,
                &QueryGetAllSupportCacheNotifyOrderNo::findAllSupportCacheNotifyOrderNo,
                historical);
        if (_exiting)
          return false;

        CacheNotifyOrderNoHelper<QueryGetAllIntlCacheNotifyOrderNo> intl(
            dbHistAdapter,
            "IntlHist",
            _exiting,
            _lastIntlHistOrderNosByEntity,
            resyncLog,
            *this,
            priorIntlHistRunTime,
            lastIntlHistRunTime,
            _pollSize,
            resync);
        intl(srv, &QueryGetAllIntlCacheNotifyOrderNo::findAllIntlCacheNotifyOrderNo, historical);
        if (_exiting)
          return false;

        if (merchCacheActivated.getValue())
        {
          CacheNotifyOrderNoHelper<QueryGetAllMerchandisingCacheNotifyOrderNo> merch(
              dbHistAdapter,
              "MerchHist",
              _exiting,
              _lastMerchHistOrderNosByEntity,
              resyncLog,
              *this,
              priorMerchHistRunTime,
              lastMerchHistRunTime,
              _pollSize,
              resync);
          merch(srv,
                &QueryGetAllMerchandisingCacheNotifyOrderNo::findAllMerchandisingCacheNotifyOrderNo,
                historical);
          if (_exiting)
            return false;
        }
      }

      LOG4CXX_DEBUG(_logger, "Polling direct injection table...");
      if (_tseServer != nullptr)
      {
        _tseServer->siphonInjectedCacheNotifications(infos);
        if (_exiting)
          return false;
        if (!infos.empty())
        {
          LOG4CXX_DEBUG(_logger, "   --> FOUND " << infos.size());
          processByOrderNo(srv, "DirectInjection", infos, _directInjectionOrderNos);
        }
        else
        {
          LOG4CXX_DEBUG(_logger, "   --> NONE FOUND");
        }
        if (_exiting)
          return false;
      }
      else
      {
        LOG4CXX_DEBUG(_logger, "   --> NO TSESERVER AVAILABLE");
      }

      LOG4CXX_DEBUG(_logger, "Checking cache flush intervals...");
      clearStaleCaches(srv);

      try
      {
        BoundFareDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in BoundFareDAO::checkForUpdates");
      }
      try
      {
        MarketRoutingDAO::checkForUpdates();
      }
      catch (std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "std::exception" << e.what() << " in MarketRoutingDAO::checkForUpdates");
      }
      try
      {
        CacheDeleterBase::emptyTrash();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "CacheDeleterBase::emptyTrash():" << e.what());
      }
      try
      {
        DataManager& dm = DataManager::instance();
        dm.dbConnChanged();
      }
      catch (const std::exception& e)
      {
        LOG4CXX_ERROR(_logger,
                      "CacheNotifyAdapter::run calling "
                          << "loadConfigValues():" << e.what());
      }
      LOG4CXX_DEBUG(_logger, "Sleep tight...");

      touchThreadAliveFile();
      {
        MonitorStateNotification emptyQueueNotification;
        CacheTrx::processDelayedUpdates();
        try
        {
          exitCondition.wait_for(lock, std::chrono::seconds(_pollInterval));
        }
        catch (...)
        {
        }
      }

      resyncLog.synchronized(MonitorStateNotification::empty());
      resyncLog.exiting(_exiting);
    }
    catch (TSEException& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      LOG4CXX_ERROR(_logger, ex.where());
      LOG4CXX_ERROR(_logger, "TSEException caught and discarded; notify thread shouldn't die.");
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(_logger, "Exception: " << ex.code() << " " << ex.message());
      LOG4CXX_ERROR(_logger,
                    "ErrorResponseException caught and discarded; notify thread shouldn't die.");
    }
    catch (std::exception& e)
    {
      LOG4CXX_ERROR(_logger, "std::exception" << e.what() << " in CacheNotify::run");
    }
    catch (...)
    {
      LOG4CXX_ERROR(_logger, "unexpected exception in CacheNotifyAdapter::run");
    }

    try
    {
      resync = _dbSyncToken != syncToken;
      nextEventSet(resync);
    }
    catch (...)
    {
    }
  }
  return true;
}

void
CacheNotifyAdapter::processByOrderNo(Service& srv,
                                     const std::string& table,
                                     std::vector<CacheNotifyInfo>& infos,
                                     OrderNosByEntityType& lastOrderNosByEntityType,
                                     bool isHistorical
                                     // Save the last orderno processed for resyncs
                                     ,
                                     bool recordLast)
{
  CacheUpdateEventPtrSet uniqueEvents;
  std::vector<CacheNotifyInfo>::iterator i;
  for (i = infos.begin(); (!_exiting) && i != infos.end(); ++i)
  {
    LOG4CXX_DEBUG(_logger,
                  "Processing id[" << i->id() << "] orderno[" << i->orderno() << ']'
                                   << i->entityType());
    CacheTrx cacheTrx;
    int64_t current = -1;
    OrderNosByEntityType::const_iterator iOrderno(lastOrderNosByEntityType.find(i->entityType()));
    OrderNosByEntityType::const_iterator eOrderno(lastOrderNosByEntityType.end());
    if (iOrderno != eOrderno)
    {
      current = iOrderno->second;
      if (static_cast<uint64_t>(current) == i->orderno())
      {
        LOG4CXX_DEBUG(_ordernoSkippedLogger, "ByEntity," << table << "," << i->orderno());
      }
    }

    if (static_cast<uint64_t>(current) != i->orderno() && !processedPriorEvent(table, i->orderno()))
    {
      if (populateTrx(cacheTrx, uniqueEvents, i->entityType(), i->keyString(), isHistorical))
      {
        srv.process(cacheTrx);
      }
      const int32_t ordn = static_cast<const int32_t>(i->orderno());
      _allEvents[_currentInterval][table].insert(ordn);

      if (LIKELY(recordLast))
        lastOrderNosByEntityType[i->entityType()] = ordn;

      LOG4CXX_DEBUG(_logger,
                    table << ',' << i->entityType() << ',' << i->orderno() << ','
                          << i->createDate());
      LOG4CXX_INFO(_ordernologger, table << "," << i->entityType() << "," << ordn);
    }
    else
    {
      LOG4CXX_DEBUG(_logger,
                    "Skipping duplicate" << table << ',' << i->entityType() << ',' << i->orderno()
                                         << ',' << i->createDate());
    }
  }
  infos.clear();
}

uint64_t
CacheNotifyAdapter::process(Service& srv,
                            const std::string& table,
                            std::vector<CacheNotifyInfo>& infos,
                            OrderNosByEntityType& lastOrderNosByEntityType,
                            bool isHistorical)
{
  CacheUpdateEventPtrSet uniqueEvents;
  uint64_t lastProcessed(0);
  std::vector<CacheNotifyInfo>::iterator i;

  DateTime _cutoff = DateTime::localTime().subtractSeconds(_processingDelay);

  LOG4CXX_DEBUG(_logger,
                "Processing cache notifications up to " << _cutoff.dateToIsoExtendedString() << ' '
                                                        << _cutoff.timeToSimpleString());

  for (i = infos.begin(); (!_exiting) && i != infos.end(); ++i)
  {
    lastProcessed = i->id();

    if (i->createDate() > _cutoff)
    {
      --lastProcessed; // make sure this record is retrieved again
      break;
    }

    LOG4CXX_DEBUG(_logger, "Processing " << i->id() << ' ' << i->entityType());
    CacheTrx cacheTrx;
    if (populateTrx(cacheTrx, uniqueEvents, i->entityType(), i->keyString(), isHistorical))
    {
      srv.process(cacheTrx);
    }
    lastOrderNosByEntityType[i->entityType()] = static_cast<int32_t>(i->orderno());
    LOG4CXX_DEBUG(_logger,
                  table << ',' << i->entityType() << ',' << i->orderno() << ',' << i->createDate());
    LOG4CXX_INFO(_ordernologger, table << "," << i->entityType() << "," << i->orderno());
  }
  infos.clear();

  return lastProcessed;
}

bool
CacheNotifyAdapter::populateTrx(CacheTrx& cacheTrx,
                                CacheUpdateEventPtrSet& uniqueEvents,
                                std::string& entityType,
                                std::string& keyString,
                                bool isHistorical)
{
  LOG4CXX_DEBUG(_logger, "Decoding\n" << keyString);
  const std::vector<std::string>* keyFields = getKeyFields(entityType, isHistorical);
  const auto cacheIds(fallback::fixed::fallbackFilterCacheNotify() ? getCacheIds(entityType, isHistorical)
                      : CacheNotifyControlInitializer::instance().getCacheIds(entityType, isHistorical));
  if (cacheIds == nullptr)
    return false; // we're not prepared for this entity

  ObjectKey objectKey;
  objectKey.entityType() = entityType;
  CacheUpdateAction requestType(CACHE_UPDATE_NONE);

  bool start = true;
  size_t k = 0;

  boost::char_separator<char> sep("|", "\n", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char>> tokenizer(keyString, sep);
  boost::tokenizer<boost::char_separator<char>>::iterator i = tokenizer.begin();
  while (!_exiting)
  {
    if (i == tokenizer.end() || *i == "\n")
    {
      std::vector<std::string>::const_iterator j = cacheIds->begin();
      for (; (!_exiting) && (j != cacheIds->end()); ++j)
      {
        objectKey.tableName() = *j;
        if (LIKELY(cacheTrx.push_back(requestType, objectKey, uniqueEvents)))
        {
          LOG4CXX_DEBUG(_logger, "CacheName=" << objectKey.tableName());
        }
        else
        {
          static log4cxx::LoggerPtr cacheLogger(
              log4cxx::Logger::getLogger("atseintl.CacheUpdate.CacheUpdateService"));
          if (cacheLogger->isInfoEnabled())
          {
            std::string msg;
            for (ObjectKey::KeyFields::const_iterator it(objectKey.keyFields().begin()),
                 itend(objectKey.keyFields().end());
                 it != itend;
                 ++it)
            {
              msg += ' ' + it->first + '=' + it->second;
            }
            msg += " ignored";
            LOG4CXX_INFO(cacheLogger, objectKey.tableName() << ": " << msg);
          }
        }
      }
      if (i == tokenizer.end())
        break;
      requestType = CACHE_UPDATE_NONE;
      objectKey.keyFields().clear();
      start = true;
      k = 0;
    }
    else
    {
      if (start)
      {
        if (*i == "^")
          break;
        switch ((*i)[0])
        {
        case 'A':
        case 'C':
          LOG4CXX_DEBUG(_logger, *i << " - UPDATE");
          requestType = CACHE_UPDATE_INVALIDATE;
          break;
        case 'D':
          LOG4CXX_DEBUG(_logger, *i << " - DELETE");
          requestType = CACHE_UPDATE_INVALIDATE;
          break;
        case 'F':
          LOG4CXX_DEBUG(_logger, *i << " - CLEAR");
          requestType = CACHE_UPDATE_CLEAR;
          break;
        }
        start = false;
      }
      else
      {
        if (keyFields != nullptr && k < keyFields->size())
        {
          const std::string& keyField = (*keyFields)[k];
          objectKey.keyFields()[keyField] = boost::trim_copy(*i);
          LOG4CXX_DEBUG(_logger, ' ' << keyField << ' ' << *i);
          k++;
        }
      }
    }
    ++i;
  }

  return true;
}

bool
CacheNotifyAdapter::clearStaleCaches(Service& srv)
{
  if (_nextFlushTime.empty())
    return true; // nothing to process

  DateTime _now = DateTime::localTime();

  CacheTrx cacheTrx;
  ObjectKey objectKey;
  std::map<std::string, DateTime>::iterator i = _nextFlushTime.begin();
  for (; i != _nextFlushTime.end(); ++i)
  {
    if (i->second <= _now)
    {
      const std::string& cacheName = i->first;
      objectKey.tableName() = cacheName;
      cacheTrx.push_back(CACHE_UPDATE_CLEAR, objectKey);
      int secs = CacheManager::instance().cacheFlushInterval(cacheName);
      if (secs > 0)
      {
        i->second = _now.addSeconds(secs);
      }
      else
      {
        _nextFlushTime.erase(i);
      }
      LOG4CXX_DEBUG(_logger,
                    "Cache for " << i->first << " flushed. Next time will be on " << i->second);
    }
  }

  return srv.process(cacheTrx);
}

void
CacheNotifyAdapter::updateLastNotifyIDs(const DateTime& _cutoff)
{
  LOG4CXX_INFO(_logger, "Starting updateLastNotifyIDs");
  // get max(id) of notify tables
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Default");

  QueryGetLastFareCacheNotify lastFare(dbAdapter->getAdapter());
  _lastFareNotify = lastFare.findLastFareCacheNotify(_cutoff);
  LOG4CXX_INFO(_logger, "Starting Fare Cache Notify from " << _lastFareNotify);

  QueryGetLastRoutingCacheNotify lastRtg(dbAdapter->getAdapter());
  _lastRoutingNotify = lastRtg.findLastRoutingCacheNotify(_cutoff);
  LOG4CXX_INFO(_logger, "Starting Routing Cache Notify from " << _lastRoutingNotify);

  QueryGetLastRuleCacheNotify lastRule(dbAdapter->getAdapter());
  _lastRuleNotify = lastRule.findLastRuleCacheNotify(_cutoff);
  LOG4CXX_INFO(_logger, "Starting Rule Cache Notify from " << _lastRuleNotify);

  QueryGetLastSupportCacheNotify lastSupp(dbAdapter->getAdapter());
  _lastSupportNotify = lastSupp.findLastSupportCacheNotify(_cutoff);
  LOG4CXX_INFO(_logger, "Starting Support Cache Notify from " << _lastSupportNotify);

  QueryGetLastIntlCacheNotify lastIntl(dbAdapter->getAdapter());
  _lastIntlNotify = lastIntl.findLastIntlCacheNotify(_cutoff);
  LOG4CXX_INFO(_logger, "Starting Intl Cache Notify from " << _lastIntlNotify);

  if (merchCacheActivated.getValue())
  {
    QueryGetLastMerchandisingCacheNotify lastMerch(dbAdapter->getAdapter());
    _lastMerchNotify = lastMerch.findLastMerchandisingCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger, "Starting Merchandising Cache Notify from " << _lastMerchNotify);
  }

  LOG4CXX_DEBUG(_logger, _cacheIds.size() << " caches");
  if (Global::allowHistorical())
  {
    DBAdapterPool::pointer_type dbHistAdapter = dbAdapterPool.get("Historical", true);
    QueryGetLastCacheNotifyHistorical lastHist(dbHistAdapter->getAdapter());
    _lastHistNotify = lastHist.findLastCacheNotifyHistorical(_cutoff);
    LOG4CXX_INFO(_logger, "Starting Historical Cache Notify from " << _lastHistNotify);

    QueryGetLastFareCacheNotify lastFareHist(dbHistAdapter->getAdapter());
    _lastFareHistNotify = lastFareHist.findLastFareCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger, "Starting Historical Fare Cache Notify from " << _lastFareHistNotify);

    QueryGetLastRoutingCacheNotify lastRoutingHist(dbHistAdapter->getAdapter());
    _lastRoutingHistNotify = lastRoutingHist.findLastRoutingCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger,
                 "Starting Historical Routing Cache Notify from " << _lastRoutingHistNotify);

    QueryGetLastRuleCacheNotify lastRuleHist(dbHistAdapter->getAdapter());
    _lastRuleHistNotify = lastRuleHist.findLastRuleCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger, "Starting Historical Rule Cache Notify from " << _lastRuleHistNotify);

    QueryGetLastSupportCacheNotify lastSupportHist(dbHistAdapter->getAdapter());
    _lastSupportHistNotify = lastSupportHist.findLastSupportCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger,
                 "Starting Historical Support Cache Notify from " << _lastSupportHistNotify);

    QueryGetLastIntlCacheNotify lastIntlHist(dbHistAdapter->getAdapter());
    _lastIntlHistNotify = lastIntlHist.findLastIntlCacheNotify(_cutoff);
    LOG4CXX_INFO(_logger, "Starting Historical Intl Cache Notify from " << _lastIntlHistNotify);
    LOG4CXX_DEBUG(_logger, _historicalIds.size() << " historical caches");

    if (merchCacheActivated.getValue())
    {
      QueryGetLastMerchandisingCacheNotify lastMerchHist(dbHistAdapter->getAdapter());
      _lastMerchHistNotify = lastMerchHist.findLastMerchandisingCacheNotify(_cutoff);
      LOG4CXX_INFO(_logger,
                   "Starting Historical Merchandising Cache Notify from " << _lastMerchHistNotify);
      LOG4CXX_DEBUG(_logger, _historicalIds.size() << " historical caches");
    }
  }
  LOG4CXX_INFO(_logger, "Finishing updateLastNotifyIDs");
}

void
CacheNotifyAdapter::resyncLastNotifyIDs()
{
  TseSynchronizingValue syncToken = DataManager::getSyncToken();
  if (_dbSyncToken != syncToken)
  {
    if (!_useOrderNo)
    {
      DateTime now(DateTime::localTime());
      DateTime _cutoff = now.subtractSeconds(_processingDelay);

      LOG4CXX_INFO(_logger,
                   "Resyncing Cache Notify IDs using ["
                       << now << "] minus processing delay [" << _processingDelay
                       << " seconds] = cutoff [" << _cutoff << "]");

      updateLastNotifyIDs(_cutoff);
    }
    _dbSyncToken = syncToken;
  }
}

const std::vector<std::string>*
CacheNotifyAdapter::getKeyFields(std::string& entityType, bool isHistorical)
{
  std::map<std::string, std::vector<std::string>>::iterator i;
  if (UNLIKELY(isHistorical))
  {
    i = _historicalKeys.find(entityType);
    if (i == _historicalKeys.end())
      return nullptr;
  }
  else
  {
    i = _keyFields.find(entityType);
    if (i == _keyFields.end())
      return nullptr;
  }
  return &(i->second);
}

const std::vector<std::string>*
CacheNotifyAdapter::getCacheIds(std::string& entityType, bool isHistorical)
{
  std::map<std::string, std::vector<std::string>>::iterator i;
  if (UNLIKELY(isHistorical))
  {
    i = _historicalIds.find(entityType);
    if (i == _historicalIds.end())
      return nullptr;
  }
  else
  {
    i = _cacheIds.find(entityType);
    if (i == _cacheIds.end())
      return nullptr;
  }
  return &(i->second);
}

void
CacheNotifyAdapter::shutdown()
{
  std::unique_lock<std::mutex> lock(exitMutex, std::defer_lock);
  lock.lock();

  _exiting = true;

  exitCondition.notify_one();
}

void
CacheNotifyAdapter::preShutdown()
{
  std::unique_lock<std::mutex> lock(exitMutex, std::defer_lock);
  lock.lock();

  _exiting = true;

  exitCondition.notify_one();
}

int
CacheNotifyAdapter::touchThreadAliveFile()
{
  int retval(0);

  bool fileExists = (access(_threadAliveFile.c_str(), F_OK) == 0);

  if (fileExists)
  {
    time_t theTime = time(nullptr);
    struct utimbuf utb;
    utb.actime = theTime;
    utb.modtime = theTime;
    retval = utime(_threadAliveFile.c_str(), &utb);
    if (retval != 0)
    {
      LOG4CXX_ERROR(_logger,
                    "Problem updating mod time for [" << _threadAliveFile << "] - "
                                                      << strerror(retval));
    }
  }
  else
  {
    FILE* fp(fopen(_threadAliveFile.c_str(), "a"));
    if (fp != nullptr)
    {
      fclose(fp);
    }
    else
    {
      retval = errno;
      LOG4CXX_ERROR(_logger, "Problem creating [" << _threadAliveFile << "]: " << strerror(retval));
    }
  }

  return retval;
}

std::vector<int64_t>
CacheNotifyAdapter::map2Vector(const OrderNosByEntityType& rhs)
{
  // Always use the vector, but if empty use magic [ non existent ] orderno[-1].
  static const int64_t RESET_ORDER_NO = -1;

  std::vector<int64_t> ordernos;

  OrderNosByEntityType::const_iterator i(rhs.begin());
  OrderNosByEntityType::const_iterator e(rhs.end());
  if (!rhs.size())
  {
    ordernos.push_back(RESET_ORDER_NO);
  }
  else
  {
    for (; i != e; i++)
      ordernos.push_back(i->second);
  }
  return ordernos;
}

std::vector<int64_t>
CacheNotifyAdapter::set2Vector(const ProcessedOrderNos& rhs)
{
  // An empty vector will not be processed by the query object.
  std::vector<int64_t> ordernos;

  ProcessedOrderNosConstIterator i(rhs.begin());
  ProcessedOrderNosConstIterator e(rhs.end());
  for (; i != e; i++)
    ordernos.push_back(*i);

  return ordernos;
}

namespace
{

Logger filterLogger("atseintl.Adapter.CacheNotifyAdapter.Filter");

void filterCacheIds(CacheNotifyControlMap& cacheIds)
{
  static CacheRegistry& registry(CacheRegistry::instance());
  for (auto it(cacheIds.begin()); it != cacheIds.end(); )
  {
    std::vector<std::string> filtered;
    for (const auto& cache : it->second)
    {
      CacheControl* ctrl(registry.getCacheControl(cache));
      if (!ctrl)
      {
        LOG4CXX_INFO(filterLogger, "  removed cache id:" << cache);
      }
      else
      {
        filtered.push_back(cache);
      }
    }
    it->second.swap(filtered);
    if (!fallback::fixed::fallbackRmHistCaches() && !Global::allowHistorical())
    {
      std::vector<std::string> filtered;
      for (const auto& cache : it->second)
      {
        if (!boost::ends_with(cache, "HISTORICAL"))
        {
          filtered.push_back(cache);
        }
        else
        {
          LOG4CXX_INFO(filterLogger, "  removed historical cache id:" << cache);
        }
      }
      it->second.swap(filtered);
    }
    if (it->second.empty())
    {
      LOG4CXX_INFO(filterLogger, "removed entity:" << it->first);
      it = cacheIds.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

}// namespace

const CacheNotifyControlInitializer& CacheNotifyControlInitializer::instance()
{
  static const CacheNotifyControlInitializer initializer;
  return initializer;
}

CacheNotifyControlInitializer::CacheNotifyControlInitializer()
{
  try
  {
    CacheNotifyControl cacheControl(
      configFile.getValue(), _keyFields, _cacheIds, _historicalKeys, _historicalIds);
    cacheControl.parse();
    filterCacheIds(_cacheIds);
    filterCacheIds(_historicalIds);
    _valid = true;
  }
  catch (...)
  {// ignore
  }
}

bool CacheNotifyControlInitializer::inUse(const std::string& entityType) const
{
  if (_cacheIds.find(entityType) != _cacheIds.end()
      || _historicalIds.find(entityType) != _historicalIds.end())
  {
    return true;
  }
  return false;
}

const std::vector<std::string>* CacheNotifyControlInitializer::getCacheIds(const std::string& entityType,
                                                                           bool isHistorical) const
{
  if (isHistorical)
  {
    auto it(_historicalIds.find(entityType));
    return it == _historicalIds.end() ? nullptr : &(it->second);
  }
  else
  {
    auto it(_cacheIds.find(entityType));
    return it == _cacheIds.end() ? nullptr : &(it->second);
  }
}
