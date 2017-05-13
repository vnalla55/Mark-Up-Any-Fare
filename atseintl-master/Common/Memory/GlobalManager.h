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
#pragma once

#include "Common/Memory/CompositeManager.h"

#include <queue>
#include <vector>
#include <stdint.h>

namespace tse
{
class PricingTrx;
class TimerTask;

namespace Memory
{
class TrxManager;

class GlobalManager : public CompositeManager
{
  struct ManagerData
  {
    TrxManager* manager = nullptr;
    TimerTask* task;

    ManagerData(const uint16_t managerId, GlobalManager& manager);

    void reset() { manager = nullptr; }
  };

public:
  static GlobalManager* instance()
  {
    return _instance;
  }

  GlobalManager();
  ~GlobalManager();

  bool isCriticalCondition() const
  {
    return _criticalCondition.load(std::memory_order_acquire);
  }

  void registerManager(TrxManager* manager);
  bool unregisterManager(TrxManager* manager);

  TimerTask& getUpdateTotalMemoryTask(const uint16_t id);

  virtual void updateTotalMemory() override final;
  void updateTotalMemory(const uint16_t id);

  size_t getUpdatedAvailableMemorySize();

  virtual void setOutOfMemory() override final;

  bool isEnoughAvailableMemory(const size_t minAvailableMemory, const size_t maxRSSPercentage);
  void checkTrxMemoryLimits(PricingTrx& trx, const uint32_t count);
  void checkTrxMemoryLimits(PricingTrx& trx);

protected:

  virtual void registerManagerImpl(Manager* manager) override final;
  virtual bool unregisterManagerImpl(Manager* manager) override final;

  using CompositeManager::registerManager;
  using CompositeManager::unregisterManager;
  using CompositeManager::archiveManager;

  virtual void updateNotificationThreshold() override final;
  virtual void notifyTotalMemoryChanged(ptrdiff_t by, size_t to) override final;

private:
  friend class GlobalManagerWatchdogTask;

  std::vector<ManagerData> _managers;
  std::queue<uint16_t> _freeManagers;
  std::atomic<bool> _isSettingOutOfMemory{false};
  std::atomic<bool> _criticalCondition{false};

  static GlobalManager* _instance;

  void updateTotalMemoryUnsafe(const uint16_t);
  void stopHeaviestManager();
  void stopAllManagers();
  void setCriticalCondition(const bool value)
  {
    _criticalCondition.store(value, std::memory_order_release);
  }

  static size_t managerMemoryScore(const ManagerData& managerData);
};
}
}
