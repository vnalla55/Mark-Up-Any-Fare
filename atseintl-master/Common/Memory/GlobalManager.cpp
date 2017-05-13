//------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "Common/Memory/GlobalManager.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/Monitor.h"
#include "Common/Memory/TrxManager.h"
#include "Common/Thread/TimerTaskExecutor.h"
#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"

#include <algorithm>
#include <cassert>

namespace tse
{
FALLBACK_DECL(unifyMemoryAborter)

namespace Memory
{
namespace
{
Logger
logger("atseintl.Common.Memory.GlobalManager");

class UpdateTotalMemoryTask : public TimerTask
{
public:
  UpdateTotalMemoryTask(const uint32_t period, const uint16_t managerId, GlobalManager& manager)
    : TimerTask(TimerTask::REPEATING, std::chrono::seconds(period)),
      _globalManager(&manager),
      _managerId(managerId)
  {
  }

  void performTask() override
  {
    _globalManager->updateTotalMemory(_managerId);
  }

private:
  GlobalManager* _globalManager;
  uint16_t _managerId;
};

}

class GlobalManagerWatchdogTask : public TimerTask
{
public:
  GlobalManagerWatchdogTask()
    : TimerTask(TimerTask::REPEATING,
                std::chrono::milliseconds(monitorUpdatePeriod),
                std::chrono::milliseconds(monitorUpdatePeriodGranularity)),
      _running(false)
  {
  }
  void performTask() override
  {
    const size_t availableMemory = MemoryMonitor::instance()->getUpdatedAvailableMemorySize();
    if (availableMemory > criticalFreeRssWatermark)
    {
      std::unique_lock<std::recursive_mutex> guard(GlobalManager::instance()->_mutex);
      GlobalManager::instance()->setCriticalCondition(false);
      if (availableMemory > softFreeRssWatermark)
      {
        TimerTaskExecutor::instance().cancel(*this);
        GlobalManager::instance()->restoreOutOfMemoryFlag();
      }
    }
  }
  bool isRunning() const
  {
    return _running;
  }
  void start()
  {
    _running = true;
    TimerTaskExecutor::instance().scheduleNow(*this);
  }

protected:
  virtual void cancel()
  {
    _running = false;
  }

private:
  bool _running;
};

static std::unique_ptr<GlobalManagerWatchdogTask> globalManagerWatchdogTask;

GlobalManager::ManagerData::ManagerData(const uint16_t managerId, GlobalManager& manager)
  : task(new UpdateTotalMemoryTask(updatePeriod, managerId, manager))
{
}

GlobalManager* GlobalManager::_instance = nullptr;

GlobalManager::GlobalManager()
{
  setThreshold(globalThreshold);

  for (uint16_t i = 0; i < initialCapacity; ++i)
  {
    _managers.push_back(ManagerData(i, *this));
    _freeManagers.push(i);
  }

  _instance = this;
  LOG4CXX_INFO(logger, "Memory manager installed. Global threshold: " << threshold() << " bytes");

  globalManagerWatchdogTask.reset(new GlobalManagerWatchdogTask());
}

GlobalManager::~GlobalManager()
{
  for (auto& elem : _managers)
    if (elem.manager)
      unregisterManager(elem.manager);

  LOG4CXX_INFO(logger, "Memory manager uninstalled.");
  _instance = nullptr;

  for (const ManagerData& manager : _managers)
    delete manager.task;

  globalManagerWatchdogTask.reset();
}

void
GlobalManager::registerManager(TrxManager* manager)
{
  CompositeManager::registerManager(manager);
}

bool
GlobalManager::unregisterManager(TrxManager* manager)
{
  return CompositeManager::unregisterManager(manager);
}

void
GlobalManager::registerManagerImpl(Manager* manager)
{
  assert(dynamic_cast<TrxManager*>(manager));
  TrxManager* trxManager = static_cast<TrxManager*>(manager);

  uint16_t id(0);

  if (_freeManagers.empty())
  {
    id = _managers.size();
    _managers.push_back(ManagerData(id, *this));
  }
  else
  {
    id = _freeManagers.front();
    _freeManagers.pop();
  }

  _managers[id].manager = trxManager;
  trxManager->setId(id);
}

bool
GlobalManager::unregisterManagerImpl(Manager* manager)
{
  assert(dynamic_cast<TrxManager*>(manager));
  TrxManager* trxManager = static_cast<TrxManager*>(manager);

  const uint16_t id = trxManager->getId();

  if (id >= _managers.size())
    return false;

  if (_managers[id].manager != trxManager)
    return false;

  _managers[id].reset();

  _freeManagers.push(id);
  return true;
}

void
GlobalManager::updateNotificationThreshold()
{
  if (IS_DEBUG_ENABLED(logger) || _criticalCondition.load(std::memory_order_acquire) ||
      isOutOfMemory())
    return setNotificationThreshold(0);

  Manager::updateNotificationThreshold();
}

void
GlobalManager::notifyTotalMemoryChanged(ptrdiff_t by, size_t to)
{
  Manager::notifyTotalMemoryChanged(by, to);
  LOG4CXX_DEBUG(logger, "Current memory usage: " << to << " bytes");

  if (to <= criticalThreshold && _criticalCondition.load(std::memory_order_acquire))
  {
    _criticalCondition.store(false, std::memory_order_release);
    updateNotificationThreshold();
  }

  if (to <= threshold() && isOutOfMemory())
  {
    restoreOutOfMemoryFlag();
    updateNotificationThreshold();
  }
}

void
GlobalManager::updateTotalMemory()
{
  std::unique_lock<Mutex> guard(_mutex);

  for (uint16_t id = 0; id < _managers.size(); ++id)
    updateTotalMemoryUnsafe(id);
}

void
GlobalManager::updateTotalMemory(const uint16_t id)
{
  std::unique_lock<Mutex> guard(_mutex);
  updateTotalMemoryUnsafe(id);
}

void
GlobalManager::updateTotalMemoryUnsafe(const uint16_t id)
{
  ManagerData& managerData = _managers[id];

  if (!managerData.manager)
    return;

  managerData.manager->updateTotalMemory();
}

size_t
GlobalManager::getUpdatedAvailableMemorySize()
{
  size_t availableMemory = MemoryMonitor::instance()->getUpdatedAvailableMemorySize();

  if (UNLIKELY(availableMemory < softFreeRssWatermark))
  {
    std::unique_lock<Mutex> guard(_mutex);

    if (availableMemory < criticalFreeRssWatermark)
    {
      if (_criticalCondition.exchange(true, std::memory_order_acq_rel))
      {
        return availableMemory;
      }
      stopAllManagers();
      if (!globalManagerWatchdogTask->isRunning())
      {
        setOutOfMemoryFlag();
        globalManagerWatchdogTask->start();
      }
    }
    else
    {
      if (!setOutOfMemoryFlag())
      {
        return availableMemory;
      }
      stopHeaviestManager();
      globalManagerWatchdogTask->start();
    }
  }
  return availableMemory;
}

namespace
{
struct ResetFlag
{
  std::atomic<bool>& flag;
  ResetFlag(std::atomic<bool>& flag) : flag(flag) {}
  ~ResetFlag() { flag.store(false, std::memory_order_release); }
};
}

void
GlobalManager::setOutOfMemory()
{
  if (_isSettingOutOfMemory.exchange(true, std::memory_order_acq_rel))
    return;

  ResetFlag resetFlag(_isSettingOutOfMemory);
  std::unique_lock<Mutex> guard(_mutex);

  updateTotalMemory();
  const size_t totalMemory = getTotalMemory();

  if (totalMemory <= threshold())
    return;

  if (totalMemory > criticalThreshold)
  {
    // Don't stop all trx twice.
    if (_criticalCondition.exchange(true, std::memory_order_acq_rel))
      return;

    updateNotificationThreshold();

    setOutOfMemoryFlag();
    stopAllManagers();
  }
  else
  {
    // Don't stop 2 transactions at the same time, unless we reached the critical threshold.
    if (!setOutOfMemoryFlag())
      return;

    updateNotificationThreshold();

    stopHeaviestManager();
  }
}

void
GlobalManager::stopHeaviestManager()
{
  LOG4CXX_INFO(logger, "We are slowly running out of memory. Stopping the heaviest trx...");

  const auto it = std::max_element(
    _managers.begin(), _managers.end(), [](ManagerData& l, ManagerData& r)
    {
      return managerMemoryScore(l) < managerMemoryScore(r);
    });

  if (it == _managers.end() || !it->manager || it->manager->isOutOfMemory())
  {
    LOG4CXX_WARN(logger, "No trx to stop found!");
    return;
  }

  it->manager->setOutOfMemory();
  LOG4CXX_INFO(logger, "Done");
}

void
GlobalManager::stopAllManagers()
{
  LOG4CXX_INFO(logger, "We are completely out of memory! Stopping all the trx...");

  for (ManagerData& data : _managers)
    if (data.manager)
      data.manager->setOutOfMemory();

  LOG4CXX_INFO(logger, "Done");
}

size_t
GlobalManager::managerMemoryScore(const ManagerData& managerData)
{
  if (!managerData.manager)
    return 0;

  if (managerData.manager->isOutOfMemory())
    return 0;

  return managerData.manager->getTotalMemory();
}

TimerTask&
GlobalManager::getUpdateTotalMemoryTask(const uint16_t id)
{
  return *_managers[id].task;
}

void
GlobalManager::checkTrxMemoryLimits(PricingTrx& trx, const uint32_t count)
{
  const uint32_t memAvailCheckInterval = TrxUtil::getMemCheckTrxInterval(trx);

  if (memAvailCheckInterval > 0 && count % memAvailCheckInterval == 0)
    getUpdatedAvailableMemorySize();

  if (fallback::unifyMemoryAborter(&trx))
    TrxUtil::checkTrxMemoryFlag(trx);
  else
    trx.checkTrxAborted();
}

void
GlobalManager::checkTrxMemoryLimits(PricingTrx& trx)
{
  getUpdatedAvailableMemorySize();
  if (fallback::unifyMemoryAborter(&trx))
    TrxUtil::checkTrxMemoryFlag(trx);
  else
    trx.checkTrxAborted();
}

bool
GlobalManager::isEnoughAvailableMemory(const size_t minAvailableMemory,
                                       const size_t maxRSSPercentage)
{
  const size_t available = getUpdatedAvailableMemorySize();
  if (UNLIKELY(available < minAvailableMemory * 1024 * 1024))
  {
    if (maxRSSPercentage < 100)
    {
      const size_t total = MemoryMonitor::instance()->getTotalMemorySize();
      const size_t rss = MemoryMonitor::instance()->getUpdatedResidentMemorySize();
      const size_t percentage = rss * 100u / total;

      if (percentage < maxRSSPercentage)
      {
        return true;
      }
      LOG4CXX_ERROR(logger,
                    "Memory Total=" << total << " RSS=" << rss << " Percentage=" << percentage
                    << " MaxRSSPercentage=" << maxRSSPercentage);
    }
    LOG4CXX_ERROR(logger,
                  "Low memory level dropping transactions. Available="
                  << available << " MinAvailableMemory=" << minAvailableMemory * 1024);
    return false;
  }
  return true;
}
}
}
