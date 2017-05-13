//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Adapter/DiskCacheAdapter.h"

#include "Adapter/MonitorStateNotification.h"
#include "Allocator/TrxMalloc.h"
#include "Common/DiskspaceWatcher.h"
#include "Common/Global.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/TSEException.h"
#include "Common/TseUtil.h"
#include "DataModel/SerializationTrx.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/LDCOperationCounts.h"

#include <fstream>
#include <list>

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <ZThreads/zthread/Exceptions.h>

#define REPORT_COUNT(a, b, c)                                                                      \
  {                                                                                                \
    if (a > 0)                                                                                     \
    {                                                                                              \
      LOG4CXX_DEBUG(_logger,                                                                       \
                    "Processed [" << a << "] " << b << " requests for type [" << c                 \
                                  << "] with data format [" << DISKCACHE.getDataFormatAsString(c)  \
                                  << "].");                                                        \
    }                                                                                              \
  }

#define CHECK_DISK_SPACE_FOR_BDB                                                                   \
  {                                                                                                \
    if (UNLIKELY((DiskCache::isActivated()) && (!DISKCACHE.bdbDisabled()) &&                       \
        (DiskspaceWatcher::isDiskFull())))                                                         \
    {                                                                                              \
      LOG4CXX_ERROR(                                                                               \
          _logger,                                                                                 \
          "Disk full - disabling BerkeleyDB operations!  Restart server to re-activate LDC.");     \
      DISKCACHE.disableBDB();                                                                      \
    }                                                                                              \
  }

namespace tse
{
class LDCActionQueueTask : public TseCallableTask
{
public:
  LDCActionQueueTask(CacheControl& ctl,
                     volatile bool& exiting,
                     LDCOperationCounts& counts,
                     log4cxx::LoggerPtr& logger,
                     uint32_t maxPerCycle)
    : _ctl(ctl), _exiting(exiting), _counts(counts), _logger(logger), _maxPerCycle(maxPerCycle)
  {
  }

  void run() override
  {
    if (UNLIKELY(_maxPerCycle))
    {
      LOG4CXX_DEBUG(_logger,
                    __FUNCTION__ << " Processing [" << _maxPerCycle
                                 << "] max per cache per iteration.");
      uint32_t i = 0;
      for (i = 0; i < _maxPerCycle && (!_exiting) && _ctl.processNextAction(_counts); i++)
        ;

      DISKCACHE.doneWithDB(_ctl.getID());

      if (!_exiting)
      {
        if (i == _maxPerCycle)
        {
          LOG4CXX_DEBUG(_logger,
                        "Action queue task for cache [" << _ctl.getID() << "] processed ["
                                                        << _maxPerCycle << "]");
        }
        else
        {
          LOG4CXX_DEBUG(_logger, "Action queue task for cache [" << _ctl.getID() << "] finished.");
        }
      }
      else
      {
        LOG4CXX_WARN(_logger, "Action queue task for cache [" << _ctl.getID() << "] aborted.");
      }
    }
    else
    {
      LOG4CXX_DEBUG(_logger,
                    __FUNCTION__ << " Processing all available events per cache per iteration.");
      while ((!_exiting) && _ctl.processNextAction(_counts))
        ;

      DISKCACHE.doneWithDB(_ctl.getID());

      if (LIKELY(!_exiting))
      {
        LOG4CXX_DEBUG(_logger, "Action queue task for cache [" << _ctl.getID() << "] finished.");
      }
      else
      {
        LOG4CXX_WARN(_logger, "Action queue task for cache [" << _ctl.getID() << "] aborted.");
      }
    }
  }

private:
  CacheControl& _ctl;
  volatile bool& _exiting;
  LDCOperationCounts& _counts;
  log4cxx::LoggerPtr& _logger;
  uint32_t _maxPerCycle;
};

class LDCFullVerifyTask : public TseCallableTask
{
public:
  LDCFullVerifyTask(CacheControl& ctl, volatile bool& exiting, log4cxx::LoggerPtr& logger)
    : _ctl(ctl), _exiting(exiting), _logger(logger)
  {
  }

  void run() override
  {
    const std::string& cacheName = _ctl.getID();
    LOG4CXX_DEBUG(_logger, "Reading keys for [" << cacheName << "].");
    DISKCACHE.readKeys(cacheName);
    DISKCACHE.doneWithDB(cacheName);
    LOG4CXX_DEBUG(_logger, "Full verification task for cache [" << cacheName << "] finished.");
  }

private:
  CacheControl& _ctl;
  volatile bool& _exiting;
  log4cxx::LoggerPtr& _logger;
};
};

using namespace tse;

static LoadableModuleRegister<Adapter, DiskCacheAdapter>
_("libDiskCacheAdapter.so");

log4cxx::LoggerPtr
DiskCacheAdapter::_logger(log4cxx::Logger::getLogger("atseintl.Adapter.DiskCacheAdapter"));

bool
DiskCacheAdapter::initialize()
{
  return true;
}

bool
DiskCacheAdapter::run(Service& srv)
{
  const MallocContextDisabler context;

  CHECK_DISK_SPACE_FOR_BDB;
  if (DiskCache::isActivated() && DISKCACHE.bdbDisabled() && (!DISKCACHE.getDistCacheEnabled()))
  {
    DiskCache::shutdown();
  }

  if (DiskCache::isActivated() && DISKCACHE.getDistCacheEnabled())
  {
    LOG4CXX_INFO(_logger, "Both LDC and MemCacheD are enabled.");
  }
  else if (DiskCache::isActivated())
  {
    LOG4CXX_INFO(_logger, "LDC is enabled.");
  }
  else if (DISKCACHE.getDistCacheEnabled())
  {
    LOG4CXX_INFO(_logger, "MemCacheD is enabled.");
  }
  else
  {
    _exiting = true;
    LOG4CXX_INFO(_logger, "Both LDC and MemCacheD are disabled.");
    return false;
  }

  DataHandle dummy;
  static size_t previousQueueSize = 0;
  CacheRegistry& registry = CacheRegistry::instance();
  size_t sizeAllQueues(0);
  std::vector<CacheControl*> cntls;
  std::vector<CacheControl*>::iterator crit(cntls.begin());

  // Build list of cache registry objects only for those
  // where LDC is configured (whether enabled or not)

  const DiskCache::CACHE_TYPE_CTRL& ldcControl = DISKCACHE.getCacheControl();
  for (const auto& elem : ldcControl)
  {
    const std::string& nm = elem.first;
    CacheControl* ctl = registry.getCacheControl(nm);
    if (ctl != nullptr)
    {
      cntls.push_back(ctl);
    }
    else
    {
      LOG4CXX_ERROR(_logger, "No CacheRegistry entry found for cache [" << nm << "] !!!");
    }
  }

  LOG4CXX_DEBUG(_logger, "Waiting for LDC requests for [" << cntls.size() << "] cache tables...");

  TseRunnableExecutor fvTaskPool(TseThreadingConst::LDC_FULL_VERIFY_TASK);
  TseRunnableExecutor eventTaskPool(TseThreadingConst::LDC_CACHE_EVENT_TASK);
  TseRunnableExecutor aqTaskPool(TseThreadingConst::LDC_ACTION_QUEUE_TASK);

  while ((!_exiting) && (DiskCache::isActivated() || DISKCACHE.getDistCacheEnabled()))
  {
    try
    {
      if (LIKELY(DiskCache::isActivated() && (!DISKCACHE.bdbDisabled())))
      {
        time_t t;
        time(&t);
        struct tm* local = localtime(&t);
        if ((_lastFullVerificationYear == 0) && (_lastFullVerificationDayOfTheYear == 0))
        {
          _lastFullVerificationYear = local->tm_year;
          _lastFullVerificationDayOfTheYear = local->tm_yday;
        }

        bool runVerify(DISKCACHE.doVerifyNextCycle());
        const int fvh(DISKCACHE.getFullVerifyHour());

        if (UNLIKELY((fvh > 0) && (fvh <= 24)))
        {
          if (fvh == (local->tm_hour + 1))
          {
            if ((_lastFullVerificationYear != local->tm_year) ||
                (_lastFullVerificationDayOfTheYear != local->tm_yday))
            {
              _lastFullVerificationYear = local->tm_year;
              _lastFullVerificationDayOfTheYear = local->tm_yday;
              runVerify = true;
            }
          }
        }

        if (UNLIKELY(runVerify))
        {
          LOG4CXX_INFO(_logger, "Running FULL verification for LDC.");

          //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // Looks like db_verify is too resource-intensive
          //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // bool saveBool( DISKCACHE.dbVerifyOnOpen() ) ;
          // DISKCACHE.setDbVerifyOnOpen( true ) ;
          // DISKCACHE.doneWithEnvironment( true ) ;
          //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

          std::list<LDCFullVerifyTask> fvTasks;
          for (crit = cntls.begin(); ((crit != cntls.end()) && (!_exiting)); ++crit)
          {
            CacheControl* ctl = *crit;
            fvTasks.push_back(LDCFullVerifyTask(*ctl, _exiting, _logger));
            fvTaskPool.execute(fvTasks.back());
          }
          fvTaskPool.wait();
          LOG4CXX_INFO(_logger, "Full verification tasks completed for [" << fvTaskPool.size() << "] caches.");
          //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
          // DISKCACHE.setDbVerifyOnOpen( saveBool ) ;
          //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

          DISKCACHE.doneWithEnvironment();
        }
      }

      sizeAllQueues = DISKCACHE.sizeAllQueues();

      uint32_t maxPerCycle(DISKCACHE.maxPerCycle());

      if (UNLIKELY(maxPerCycle))
      {
        LOG4CXX_DEBUG(_logger, "Processing [" << maxPerCycle << "] max per cache per iteration.");
        LDCOPCOUNTS opCounts;

        while ((!_exiting) && (sizeAllQueues > 0))
        {
          if (sizeAllQueues != previousQueueSize)
          {
            LOG4CXX_INFO(_logger, "LDC action queue size (all queues) = [" << sizeAllQueues << "]");
            previousQueueSize = sizeAllQueues;
          }

          CHECK_DISK_SPACE_FOR_BDB;

          SerializationTrx trx;
          std::list<LDCActionQueueTask> tasks;
          for (crit = cntls.begin(); ((crit != cntls.end()) && (!_exiting)); ++crit)
          {
            CacheControl* ctl = (*crit);
            if ((ctl->actionQueueSize() > 0) && (!_exiting))
            {
              const std::string& cacheName = ctl->getID();
              LDCOperationCounts& counters = opCounts[cacheName];
              tasks.push_back(LDCActionQueueTask(*ctl, _exiting, counters, _logger, maxPerCycle));
              aqTaskPool.execute(tasks.back());
              LOG4CXX_DEBUG(_logger, "Submitted action queue task for cache [" << cacheName << "].");
            }
          }
          LOG4CXX_DEBUG(_logger, "Waiting for action queue tasks to complete.");
          aqTaskPool.wait();
          LOG4CXX_DEBUG(_logger, "Action queue tasks completed for [" << aqTaskPool.size() << "] caches.");
          if (!_exiting)
          {
            if (DiskCache::isActivated() && (!DISKCACHE.bdbDisabled()))
            {
              LOG4CXX_DEBUG(_logger, "Updating last update time for LDC.");
              DISKCACHE.updateLastUpdateTime();
            }
          }

          if ((!_exiting))
          {
            DISKCACHE.doneWithEnvironment();
          }

          sizeAllQueues = DISKCACHE.sizeAllQueues();
        }

        if ((!_exiting) && (IS_DEBUG_ENABLED(_logger)))
        {
          for (LDCOPCOUNTS::const_iterator it = opCounts.begin(); it != opCounts.end(); ++it)
          {
            const LDCOperationCounts& counters = (*it).second;
            if (!counters.empty())
            {
              const std::string& cacheName = (*it).first;
              REPORT_COUNT(counters.goodWrites, "successful WRITE", cacheName);
              REPORT_COUNT(counters.badWrites, "UNSUCCESSFUL WRITE", cacheName);
              REPORT_COUNT(counters.goodRemoves, "successful REMOVE", cacheName);
              REPORT_COUNT(counters.badRemoves, "UNSUCCESSFUL REMOVE", cacheName);
              REPORT_COUNT(counters.goodClears, "successful CLEAR", cacheName);
              REPORT_COUNT(counters.badClears, "UNSUCCESSFUL CLEAR", cacheName);
              SUPPRESS_UNUSED_WARNING(cacheName);
            }
          }
        }
      }
      else
      {
        if (sizeAllQueues != previousQueueSize)
        {
          LOG4CXX_INFO(_logger, "LDC action queue size (all queues) = [" << sizeAllQueues << "]");
          previousQueueSize = sizeAllQueues;
        }

        CHECK_DISK_SPACE_FOR_BDB;

        if ((!_exiting) && (sizeAllQueues > 0))
        {
          LOG4CXX_DEBUG(_logger, "Processing all available events per cache per iteration.");
          SerializationTrx trx;
          LDCOPCOUNTS opCounts;
          std::list<LDCActionQueueTask> tasks;
          for (crit = cntls.begin(); ((crit != cntls.end()) && (!_exiting)); ++crit)
          {
            CacheControl* ctl = *crit;
            if ((ctl->actionQueueSize() > 0) && (!_exiting))
            {
              const std::string& cacheName = ctl->getID();
              LDCOperationCounts& counters = opCounts[cacheName];
              tasks.push_back(LDCActionQueueTask(*ctl, _exiting, counters, _logger, 0));
              eventTaskPool.execute(tasks.back());
              LOG4CXX_DEBUG(_logger, "Submitted action queue task for cache [" << cacheName << "].");
            }
          }
          LOG4CXX_DEBUG(_logger, "Waiting for action queue tasks to complete.");
          eventTaskPool.wait();
          LOG4CXX_DEBUG(
              _logger, "Action queue tasks completed for [" << eventTaskPool.size() << "] caches.");
          if (UNLIKELY((!_exiting) && (IS_DEBUG_ENABLED(_logger))))
          {
            for (LDCOPCOUNTS::const_iterator it = opCounts.begin(); it != opCounts.end(); ++it)
            {
              const LDCOperationCounts& counters = (*it).second;
              if (!counters.empty())
              {
                const std::string& cacheName = (*it).first;
                REPORT_COUNT(counters.goodWrites, "successful WRITE", cacheName);
                REPORT_COUNT(counters.badWrites, "UNSUCCESSFUL WRITE", cacheName);
                REPORT_COUNT(counters.goodRemoves, "successful REMOVE", cacheName);
                REPORT_COUNT(counters.badRemoves, "UNSUCCESSFUL REMOVE", cacheName);
                REPORT_COUNT(counters.goodClears, "successful CLEAR", cacheName);
                REPORT_COUNT(counters.badClears, "UNSUCCESSFUL CLEAR", cacheName);
                SUPPRESS_UNUSED_WARNING(cacheName);
              }
            }
          }
        }

        if (LIKELY(!_exiting))
        {
          if (LIKELY(DiskCache::isActivated() && (!DISKCACHE.bdbDisabled())))
          {
            LOG4CXX_DEBUG(_logger, "Updating last update time for LDC.");
            DISKCACHE.updateLastUpdateTime();
          }
        }

        if ((!_exiting) && (sizeAllQueues > 0))
        {
          DISKCACHE.doneWithEnvironment();
        }
      }

      if (LIKELY(!_exiting))
      {
        MonitorStateNotification::mark();
      }

      if (LIKELY(!_exiting))
      {
        boost::unique_lock<boost::mutex> lock(_waitBoostMutex);
        if (UNLIKELY(_waitBoostCondition.timed_wait(lock, boost::posix_time::milliseconds(DISKCACHE.getThreadSleepSecs() * 1000))))
        {
          LOG4CXX_DEBUG(_logger, "Signaled to wake up before reaching  [" << DISKCACHE.getThreadSleepSecs() << "] seconds.");
        }
        else
        {
          LOG4CXX_DEBUG(_logger, "Woke up after sleeping for [" << DISKCACHE.getThreadSleepSecs() << "] seconds.");
        }
      }
    }
    catch (TSEException& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      LOG4CXX_ERROR(_logger, ex.where());
      std::string err(
          "DiskCacheAdapter - TSEException caught and discarded; disk cache thread shouldn't die.");
      LOG4CXX_ERROR(_logger, err);
      TseUtil::alert(err.c_str());
    }
    catch (const ZThread::Synchronization_Exception& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      std::string err("DiskCacheAdapter - ZThread::Synchronization_Exception caught and discarded; "
                      "disk cache thread shouldn't die.");
      LOG4CXX_ERROR(_logger, err);
      TseUtil::alert(err.c_str());
    }
    catch (boost::thread_interrupted&)
    {
      LOG4CXX_ERROR(_logger, "Interrupted");
      std::string err("DiskCacheAdapter - boost::thread_interrupted caught and discarded; "
                      "disk cache thread shouldn't die.");
      LOG4CXX_ERROR(_logger, err);
      TseUtil::alert(err.c_str());
    }
    catch (std::exception& ex)
    {
      LOG4CXX_ERROR(_logger, ex.what());
      std::string err("DiskCacheAdapter - std::Exception caught and discarded; disk cache thread "
                      "shouldn't die.");
      LOG4CXX_ERROR(_logger, err);
      TseUtil::alert(err.c_str());
    }
    catch (...)
    {
      std::string err("DiskCacheAdapter - Unknown exception (...) caught and discarded; disk cache "
                      "thread shouldn't die.");
      LOG4CXX_ERROR(_logger, err);
      TseUtil::alert(err.c_str());
    }
  }

  return true;
}

void
DiskCacheAdapter::shutdown()
{
  static bool hasBeenShutDown(false);

  bool doShutdown(false);

  {
    boost::lock_guard<boost::mutex> lock(_shutdownMutex);
    if (!hasBeenShutDown)
    {
      doShutdown = true;
      hasBeenShutDown = true;
    }
  }

  if (doShutdown)
  {
    LOG4CXX_DEBUG(_logger, "Starting DiskCacheAdapter shutdown.");

    DiskCache::shutdown();

    _exiting = true;

    _waitBoostCondition.notify_one();

    LOG4CXX_DEBUG(_logger, "DiskCacheAdapter shutdown complete.");
  }
}

void
DiskCacheAdapter::preShutdown()
{
  _exiting = true;
}
