//----------------------------------------------------------------------------
//
//  Description: Common Metrics Latency class for ATSE shopping/pricing.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TSELatencyData.h"

#include "Common/Global.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/MetricsUtil.h"
#include "DataModel/Trx.h"

#include <pthread.h>

namespace tse
{
namespace
{

std::deque<Trx::Latency>&
getLatencyData(Trx& trx)
{
  if (pthread_self() == trx.mainThread())
  {
    return trx.latencyData();
  }
  else
  {
    const size_t index = size_t(pthread_self()) % trx.latencyDataThread().size();
    boost::lock_guard<boost::mutex> g(trx.latencyDataThreadMutex()[index]);
    return trx.latencyDataThread()[index][pthread_self()];
  }
}
}

TSELatencyData::TSELatencyData(Trx& trx,
                               const char* const description,
                               Callback callback,
                               bool isTaskToplevelData)
  : _trx(*trx.mainTrx()),
    _description(description),
    _isTaskToplevelData(isTaskToplevelData),
    _timer(isTaskToplevelData,
           trx.recordMetrics() || trx.recordMemory() || trx.recordCPUTime(),
           trx.recordTopLevelMetricsOnly()),
    _callback(callback)
{
  if (!_timer.enabled())
    return;

  if (UNLIKELY(_trx.recordMemory()))
  {
    if (!Memory::changesFallback)
    {
      _startMem = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize();
    }
    else
    {
      _startMem = MemoryUsage::getVirtualMemorySize();
    }
  }

  {
    std::deque<Trx::Latency>& data = getLatencyData(_trx);
    _items = data.size();
  }

  _timer.start();
}

TSELatencyData::TSELatencyData(Trx& trx,
                               const char* const description,
                               bool isTaskToplevelData)
  : TSELatencyData(trx, description, Callback(), isTaskToplevelData)
{
}

TSELatencyData::TSELatencyData(Trx& trx,
                               const MetricsUtil::MetricsFactor factor,
                               bool isTaskToplevelData)
  : TSELatencyData(trx, MetricsUtil::factorDesc(factor).c_str(), isTaskToplevelData)
{
}

TSELatencyData::~TSELatencyData()
{
  if (!_timer.enabled())
    return;

  collect();
}

void
TSELatencyData::collect()
{
  const double childUserTime = double(0);
  const double childSystemTime = double(0);

  _timer.stop();

  const double wallTime = _timer.getElapsedTime();
  const double userTime = _timer.getCpuUserTime();
  const double sysTime = _timer.getCpuSystemTime();

  std::deque<Trx::Latency>& data = getLatencyData(_trx);
  const size_t items = data.size() - _items;

  // if the last function call was the same, then simply accumulate an
  // item onto that.
  if (items == 0 && data.empty() == false && data.front().service == _trx.getCurrentService() &&
      data.front().description == _description && data.front().nItems == 0)
  {
    Trx::Latency& obj = data.front();
    obj.nRepeats++;
    obj.wallTime += wallTime;
    obj.userTime += userTime;
    obj.systemTime += sysTime;
    obj.childUserTime += childUserTime;
    obj.childSystemTime += childSystemTime;
  }
  else
  {
    size_t mem(0);
    if (_trx.recordMemory())
    {
      if (!Memory::changesFallback)
      {
        mem = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize();
      }
      else
      {
        mem = MemoryUsage::getVirtualMemorySize();
      }
    }
    data.push_front(Trx::Latency(items,
                                 _description,
                                 wallTime,
                                 userTime,
                                 sysTime,
                                 childUserTime,
                                 childSystemTime,
                                 _startMem,
                                 mem,
                                 pthread_self(),
                                 _isTaskToplevelData, _trx.getCurrentService()));
  }

  if (UNLIKELY(_callback))
    _callback(wallTime, userTime, sysTime);
}
}
