//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Util/BranchPrediction.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

namespace tse
{
class TimerTask;

namespace Memory
{

class MemoryMonitor
{
public:
  virtual ~MemoryMonitor();
  MemoryMonitor(const MemoryMonitor&) = delete;
  MemoryMonitor& operator=(const MemoryMonitor&) = delete;

  size_t getVirtualMemorySize()
  {
    return _virtualMemorySizeCache.load(std::memory_order_relaxed);
  }
  size_t getResidentMemorySize()
  {
    return _residentMemorySizeInCache.load(std::memory_order_relaxed);
  }
  size_t getAvailableMemorySize()
  {
    return _availableMemorySizeInCache.load(std::memory_order_relaxed);
  }
  static size_t getTotalMemorySize()
  {
    return _totalMemorySizeCache;
  }

  virtual size_t getUpdatedVirtualMemorySize();
  virtual size_t getUpdatedResidentMemorySize();
  virtual size_t getUpdatedAvailableMemorySize();
  virtual void updateMonitor();

  static void createInstance();
  static void destroyInstance();
  static MemoryMonitor* instance()
  {
    // This is only for UT
    if (UNLIKELY(_instance.get() == nullptr))
    {
      createInstance();
    }
    return _instance.get();
  }

protected:
  MemoryMonitor();

  static std::unique_ptr<MemoryMonitor> _instance;

private:

  bool parseStat();
  bool parseMeminfo(const std::vector<std::string> fields, size_t& memsize);

  std::unique_ptr<TimerTask> _task;
  std::atomic<size_t> _virtualMemorySizeCache{0};
  std::atomic<size_t> _residentMemorySizeInCache{0};
  std::atomic<size_t> _availableMemorySizeInCache{0};

  static size_t _totalMemorySizeCache;
};

}
}
