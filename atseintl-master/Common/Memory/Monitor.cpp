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

#include "Common/Memory/Monitor.h"

#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/Thread/TimerTaskExecutor.h"

#include <sstream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

namespace tse
{
namespace Memory
{

namespace
{
Logger
logger("atseintl.Common.Memory.MemoryMonitor");

enum StatFields
{
  STAT_UNKNOWN,
  STAT_PID,
  STAT_COMM,
  STAT_STATE,
  STAT_VSIZE = 23,
  STAT_RSS,

  STAT_END
};

const std::vector<std::string> meminfoAvail{"MemFree:", "Buffers:", "Cached:"};
const std::vector<std::string> meminfoTotal{"MemTotal:"};

const size_t PAGE_SIZE = 4096;

class UpdateMemoryMonitorTask : public TimerTask
{
public:
  UpdateMemoryMonitorTask(MemoryMonitor& monitor)
    : TimerTask(TimerTask::REPEATING,
                std::chrono::milliseconds(monitorUpdatePeriod),
                std::chrono::milliseconds(monitorUpdatePeriodGranularity)),
      _memoryMonitor(monitor)
  {
  }

  void performTask() override
  {
    _memoryMonitor.updateMonitor();
  }

private:
  MemoryMonitor& _memoryMonitor;
};

class CachedFileBuffer
{
public:
  CachedFileBuffer(const std::string& path)
  {
    _file = open(path.c_str(), O_RDONLY);
  }

  std::string readFileContents()
  {
    std::string buffer;
    buffer.resize(4096);

    int bytes =  pread(_file, &(buffer.front()), buffer.size(), 0);
    if (bytes != -1)
    {
      buffer.resize(bytes);
      return buffer;
    }

    return std::string();
  }

private:
  int _file;
};

inline std::string getStatPath()
{
  std::string path ("/proc/");
  path += std::to_string((int)getpid());
  path += "/stat";

  return path;
}

CachedFileBuffer stat(getStatPath());
CachedFileBuffer meminfo("/proc/meminfo");
}

std::unique_ptr<MemoryMonitor> MemoryMonitor::_instance;
size_t MemoryMonitor::_totalMemorySizeCache = 0;

MemoryMonitor::MemoryMonitor()
{
  if (monitorUpdatePeriod)
  {
    _task = std::unique_ptr<TimerTask>(new UpdateMemoryMonitorTask(*this));
    TimerTaskExecutor::instance().scheduleNow(*_task);
  }
  size_t memSize(0);
  if (parseMeminfo(meminfoTotal, memSize))
  {
    _totalMemorySizeCache = memSize;
  }
  else
  {
    // hang on to your hat!
    abort();
  }
}

MemoryMonitor::~MemoryMonitor()
{
  if (_task)
  {
    TimerTaskExecutor::instance().cancelAndWait(*_task);
    _task.reset();
  }
}

void
MemoryMonitor::createInstance()
{
  _instance = std::unique_ptr<MemoryMonitor>(new MemoryMonitor());
}

void
MemoryMonitor::destroyInstance()
{
  _instance.reset();
}

void
MemoryMonitor::updateMonitor()
{
  parseStat();
  size_t memSize(0);
  if (parseMeminfo(meminfoAvail, memSize))
    _availableMemorySizeInCache.store(memSize, std::memory_order_relaxed);

  LOG4CXX_INFO(logger, "Memory monitor was updated:"
               << " VM = " << _virtualMemorySizeCache.load(std::memory_order_relaxed)
               << " RSS = " << _residentMemorySizeInCache.load(std::memory_order_relaxed)
               << " AVM = " << _availableMemorySizeInCache.load(std::memory_order_relaxed));
}

size_t
MemoryMonitor::getUpdatedVirtualMemorySize()
{
  parseStat();
  return _virtualMemorySizeCache.load(std::memory_order_relaxed);
}

size_t
MemoryMonitor::getUpdatedResidentMemorySize()
{
  parseStat();
  return _residentMemorySizeInCache.load(std::memory_order_relaxed);
}

size_t
MemoryMonitor::getUpdatedAvailableMemorySize()
{
  size_t memSize(0);
  if (parseMeminfo(meminfoAvail, memSize))
    _availableMemorySizeInCache.store(memSize , std::memory_order_relaxed);

  return _availableMemorySizeInCache.load(std::memory_order_relaxed);
}

bool
MemoryMonitor::parseStat()
{
  std::string buffer(stat.readFileContents());

  if (UNLIKELY(buffer.empty()))
  {
    LOG4CXX_ERROR(logger, "Memory monitor: cannot parse stat, empty file");
    return false;
  }

  std::istringstream iss(std::move(buffer));

  for (int statIdx = 1; statIdx < STAT_END; ++statIdx)
  {
    size_t value(0);

    if (statIdx == STAT_COMM || statIdx == STAT_STATE)
    {
      std::string dummy;
      iss >> dummy;
    }
    else
    {
      iss >> value;
    }

    if (!iss || !iss.good())
    {
      LOG4CXX_ERROR(logger, "Memory monitor: cannot parse stat file");
      return false;
    }

    switch (statIdx)
    {
      case STAT_VSIZE:
      {
        _virtualMemorySizeCache.store(value, std::memory_order_relaxed);
        break;
      }
      case STAT_RSS:
      {
        _residentMemorySizeInCache.store(value * PAGE_SIZE, std::memory_order_relaxed);
        break;
      }
    }
  }

  return true;
}

bool
MemoryMonitor::parseMeminfo(const std::vector<std::string> fields, size_t& memsize)
{
  std::string buffer(meminfo.readFileContents());

  if (UNLIKELY(buffer.empty()))
  {
    LOG4CXX_ERROR(logger, "Memory monitor: cannot parse meminfo, empty file");
    return false;
  }

  size_t value(0);

  for(const std::string& field : fields)
  {
    auto idx = buffer.find(field);
    if (idx == std::string::npos)
    {
      LOG4CXX_ERROR(logger, "Memory monitor: cannot parse meminfo");
      return false;
    }

    value += std::strtoull(buffer.c_str() + field.size() + idx + 1, NULL, 10);
  }
  memsize = value * 1024;
  return true;
}

}
}
