#include "DataModel/DelayedUpdates.h"

#include "Allocator/TrxMalloc.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/Thread/TseThreadingConst.h"
#include "Common/TseSrvStats.h"
#include "DataModel/CacheUpdateEvent.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/ObjectKey.h"
#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"

#include <cassert>
#include <iomanip>

#include <time.h>

namespace tse
{

FIXEDFALLBACK_DECL(fallbackRCLoadOnUpdate);

namespace
{

Logger logger("atseintl.DataModel.DelayedUpdates");
Logger cacheLogger("atseintl.DataModel.CacheUpdateLog");

boost::atomic<long> numberTasks(0);

struct CompareTimes
{
  bool operator ()(CacheUpdateEventPtr event,
                   std::time_t time) const
  {
    return event->_time < time;
  }
};

std::string writeResult(CacheUpdateEventPtr event,
                        size_t deleteCount = 0,
                        bool duplicate = false)
{
  std::ostringstream os;
  const ObjectKey& key(event->_key);
  os << key.tableName() << ": ";
  if (CACHE_UPDATE_CLEAR == event->_action)
  {
    os << " flush";
  }
  else if (UNLIKELY(CACHE_UPDATE_WARMUP == event->_action))
  {
    os << " stored";
  }
  for (const auto& pair : key.keyFields())
  {
    os << ' ' << pair.first << '=' << pair.second;
  }
  if (deleteCount > 0)
  {
    os << " deleted " << deleteCount;
  }
  if (duplicate)
  {
    os << " duplicate";
  }
  std::tm tmb{};
  if (::localtime_r(&event->_time, &tmb))
  {
    os << " received " << std::put_time(&tmb,"%X");
  }
  return os.str();
}

CacheUpdatePtrVector immediateUpdates;
CacheUpdatePtrList delayedUpdates;

boost::mutex mutex;

CacheUpdateEventPtrSet immediateUpdateSet;
CacheUpdateEventPtrSet delayedUpdateSet;

void processEvent(CacheUpdateEventPtr event)
{
  const MallocContextDisabler context;
  size_t result(0);
  try
  {
    switch (event->_action)
    {
    case CACHE_UPDATE_WARMUP:
      event->_ctrl->store(event->_key);
      break;
    case CACHE_UPDATE_INVALIDATE:
      result = event->_ctrl->invalidate(event->_key);
      break;
    case CACHE_UPDATE_CLEAR:
      result = event->_ctrl->clear();
      break;
    default:
      break;
    }
    LOG4CXX_INFO(cacheLogger, writeResult(event, result));
  }
  catch (const std::exception& e)
  {
    const std::string& message(writeResult(event) + ":exception:" + e.what());
    LOG4CXX_ERROR(logger, message);
    LOG4CXX_ERROR(cacheLogger, message);
  }
  catch (...)
  {
    const std::string& message(writeResult(event) + ":unknown exception");
    LOG4CXX_ERROR(logger, message);
    LOG4CXX_ERROR(cacheLogger, message);
  }
}

void processEvent(CacheUpdateEventPtr event,
                  CacheUpdateEventPtrSet& ptrSet)
{
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    ptrSet.erase(event);
  }
  processEvent(event);
}

struct ProcessUpdate
{
  ProcessUpdate(CacheUpdateEventPtrSet& ptrSet)
    : _ptrSet(ptrSet)
  {
  }

  void operator ()(CacheUpdateEventPtr event)
  {
    processEvent(event, _ptrSet);
  }

  CacheUpdateEventPtrSet& _ptrSet;
};

template <typename T>
bool saveUniqueEvent(CacheUpdateEventPtr event,
                     T& container,
                     CacheUpdateEventPtrSet& ptrSet)
{
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    std::pair<CacheUpdateEventPtrSet::iterator, bool> pr(ptrSet.insert(event));
    if (pr.second)
    {
      container.push_back(event);
      return true;
    }
  }
  LOG4CXX_DEBUG(cacheLogger, writeResult(event, 0, true));
  return false;
}

bool saveEvent(CacheUpdateEventPtr event)
{
  bool rcEnabled(RemoteCache::ReadConfig::isEnabled());
  const auto& serverAttributes(RemoteCache::ReadConfig::getServer(event->_key.tableName()));
  const auto& hp(serverAttributes.hostPort());
  const auto& hppr(serverAttributes.primary());
  const auto& hpsec(serverAttributes.secondary());
  bool immediate(!rcEnabled || hp.empty() || hppr._sameHost || hpsec._sameHost);
  if (immediate)
  {
    return saveUniqueEvent(event, immediateUpdates, immediateUpdateSet);
  }
  else
  {
    event->_ctrl->updateLDC(event->_key, event->_action);
    return saveUniqueEvent(event, delayedUpdates, delayedUpdateSet);
  }
  return false;
}

class CacheUpdateTask : public TseCallableTask
{
public:

  CacheUpdateTask(CacheUpdateEventPtr event,
                  CacheUpdateEventPtrSet& ptrSet)
    : _event(event)
    , _ptrSet(ptrSet)
  {
    ++numberTasks;
  }

  virtual ~CacheUpdateTask()
  {
    --numberTasks;
  }

  virtual void run() override { processEvent(_event, _ptrSet); }

private:
  CacheUpdateEventPtr _event;
  CacheUpdateEventPtrSet& _ptrSet;
};

struct PushTask
{
  PushTask(CacheUpdateEventPtrSet& ptrSet)
    : _ptrSet(ptrSet)
  {
  }

  void operator ()(CacheUpdateEventPtr event) const
  {
    static TseRunnableExecutor executor(TseThreadingConst::CACHE_UPDATE_TASK);
    CacheUpdateTask* task(new CacheUpdateTask(event, _ptrSet));
    if (fallback::fixed::fallbackRCLoadOnUpdate())
    {
      executor.execute(task, true);
    }
    else// new code
    {
      static TseRunnableExecutor louExecutor(TseThreadingConst::LOAD_ON_UPDATE_TASK);
      if (event && event->_ctrl)
      {
        const std::string& id(event->_ctrl->getID());
        if (!id.empty())
        {
          const auto& cm(CacheManager::instance());
          bool loadOnUpdate(cm.cacheLoadOnUpdate(id));
          const std::string& serverPort(RemoteCache::ReadConfig::getServerPort());
          bool masterAny(!serverPort.empty());
          bool masterType(RemoteCache::RCServerAttributes::isMaster(id));
          if (masterAny && !masterType && loadOnUpdate)
          {
            louExecutor.execute(task, true);
          }
          else
          {
            executor.execute(task, true);
          }
        }
      }
    }
  }

  CacheUpdateEventPtrSet& _ptrSet;
};

}// namespace

Logger getCacheUpdateLogger()
{
  return cacheLogger;
}

std::time_t DelayedUpdates::serverStartTime()
{
  static std::time_t started(std::time(nullptr));
  return started;
}

bool DelayedUpdates::delayUpdate(const ObjectKey& key,
                                 CacheUpdateAction action)
{
  CacheRegistry& registry(CacheRegistry::instance());
  CacheControl* ctrl(registry.getCacheControl(key.tableName()));
  if (UNLIKELY(nullptr == ctrl))
  {
    LOG4CXX_INFO(cacheLogger, "table " << key.tableName() << " isn't in registry");
    return true;
  }
  CacheUpdateEventPtr event(new CacheUpdateEvent(key, action, ctrl));
  switch(event->_action)
  {
  case CACHE_UPDATE_WARMUP:
    processEvent(event);
    return true;
  default:
    break;
  }
  return saveEvent(event);
}

void DelayedUpdates::processDelayedUpdates()
{
  int delay(RemoteCache::ReadConfig::getClientCacheUpdateDelay());
  TseSrvStats::cacheUpdateTimestamp(true);
  LOG4CXX_INFO(cacheLogger, "thread pool queue size=" << numberTasks);
  std::time_t currentTime(std::time(nullptr));
  std::time_t oldestToKeep(currentTime - delay);
  size_t pushed(0);
  CacheUpdatePtrList localList;
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    CacheUpdatePtrList::iterator it(
      std::lower_bound(delayedUpdates.begin(),
                       delayedUpdates.end(),
                       oldestToKeep,
                       CompareTimes()));
    delayedUpdates.splice(localList.end(), delayedUpdates, delayedUpdates.begin(), it);
  }
  PushTask delayedTask(delayedUpdateSet);
  std::for_each(localList.begin(), localList.end(), delayedTask);
  pushed += localList.size();
  CacheUpdatePtrVector localVector;
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    immediateUpdates.swap(localVector);
  }
  PushTask immediateTask(immediateUpdateSet);
  std::for_each(localVector.begin(), localVector.end(), immediateTask);
  pushed += localVector.size();
  if (pushed)
  {
    LOG4CXX_INFO(cacheLogger, "pushed to queue " << pushed << " thread tasks");
  }
}

}// tse
