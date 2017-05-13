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
#include "Common/Memory/Config.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"

namespace tse
{
FIXEDFALLBACK_DECL(memoryManagerChanges)

namespace Memory
{
namespace
{
Logger
logger("atseintl.Common.Memory.Config");

constexpr size_t MB = 1024 * 1024;
constexpr size_t GB = 1024 * MB;
const std::string section = "MEMORY";

ConfigurableValue<bool> managerEnabledCfg("TSE_SERVER", "USE_MEMORY_MANAGER", false);

// How often should a trx update its memory usage accurately.
ConfigurableValue<uint32_t> updatePeriodCfg(section, "UPDATE_PERIOD", 5);

// Initial capacity of global memory manager.
ConfigurableValue<uint32_t> initialCapacityCfg(section, "INITIAL_CAPACITY", 100);

// All of the following thresholds are in bytes. Please note that the memory manager doesn't count
// all of the memory the system is using, so these values must be lower than the operating system
// allows. In particular, the memory manager doesn't count cache memory usage just yet.

// Soft global threshold of memory consumption. Crossing this value stops the most expensive
// trx. Only this one trx is stopped, unless the next threshold is reached.
ConfigurableValue<size_t> globalThresholdCfg(section, "GLOBAL_THRESHOLD", 0);

// Crossing this threshold makes the server enter critical condition. All of the trx are stopped,
// new trx are immediately throttled. The server leaves critical condition once it's consumption
// drops below this watermark.
ConfigurableValue<size_t> criticalThresholdCfg(section, "CRITICAL_THRESHOLD", 0);

// Per-trx memory limit. Crossing it stops the trx.
ConfigurableValue<size_t> trxThresholdCfg(section, "TRX_THRESHOLD", 10 * GB);

// Note that without critical condition the stopped trx will still try to end successfully,
// although with less options returned.

// How often (in bytes allocated) should FarePathFactoryStorage inform the memory manager about
// it's memory usage.
ConfigurableValue<size_t> fpStorageCheckPeriodCfg(section, "FP_STORAGE_CHECK_PERIOD", MB);
constexpr size_t FP_STORAGE_CHECK_PERIOD_MIN = 10240;

// How often (in bytes allocated) should DataHandle DeleteList container inform the memory manager
// about it's memory usage.
ConfigurableValue<size_t> dhCheckPeriodCfg(section, "DH_CHECK_PERIOD", MB);
constexpr size_t DH_CHECK_PERIOD_MIN = 65536;

// How often should a memory monitor be updated.
// Period is specified in milliseconds.
ConfigurableValue<uint32_t>
memoryMonitorUpdatePeriod(section, "MEMORY_MONITOR_UPDATE_PERIOD", 1000);
ConfigurableValue<uint32_t>
memoryMonitorUpdatePeriodGranularity(section, "MEMORY_MONITOR_UPDATE_PERIOD_GRANULARITY", 200);

ConfigurableValue<size_t> critiFreeRssWatermark(section,"CRITICAL_FREE_RSS_WATERMARK", 0);

ConfigurableValue<size_t> warnFreeRssWatermark(section, "SOFT_FREE_RSS_WATERMARK", 0);
}

bool _managerEnabled = false;
uint32_t _updatePeriod;
uint32_t _initialCapacity;
size_t _globalThreshold = 20 * GB;
size_t _criticalThreshold = 25 * GB;
size_t _trxThreshold = 10 * GB;
size_t _fpStorageCheckPeriod;
size_t _dhCheckPeriod;
uint32_t _monitorUpdatePeriod;
uint32_t _monitorUpdatePeriodGranularity;
size_t _criticalFreeRssWatermark = 6 * GB;
size_t _softFreeRssWatermark = 9 * GB;
bool _changesFallback = true;

void checkThresholdsAndWatermarks()
{
  const bool areThresholdConfigured = _criticalThreshold && _globalThreshold;
  const bool areWatermarksConfigured = _criticalFreeRssWatermark && _softFreeRssWatermark;

  if (areWatermarksConfigured)
  {
    if (_criticalThreshold || _globalThreshold)
    {
      LOG4CXX_ERROR(logger, "Watermark and Threshold mechanizms are mutually exclusive. The latter "
                            "is disabled.");
    }
    _criticalThreshold = std::numeric_limits<size_t>::max();
    _globalThreshold = std::numeric_limits<size_t>::max();
    _criticalFreeRssWatermark = std::min(_criticalFreeRssWatermark, _softFreeRssWatermark);
  }
  else if (areThresholdConfigured)
  {
    if (_criticalFreeRssWatermark || _softFreeRssWatermark)
    {
      LOG4CXX_ERROR(logger, "Threshold and Watermark mechanizms are mutually exclusive. The latter "
                            "is disabled.");
    }
    _criticalFreeRssWatermark = 0;
    _softFreeRssWatermark = 0;
  }
  else
  {
    LOG4CXX_ERROR(logger, "Watermark and Threshold mechanizms are inactive and will not trigger "
                          "even when there's no more memory available.");
    _criticalThreshold = std::numeric_limits<size_t>::max();
    _globalThreshold = std::numeric_limits<size_t>::max();
    _criticalFreeRssWatermark = 0;
    _softFreeRssWatermark = 0;
  }
}

void
configure()
{
  _managerEnabled = managerEnabledCfg.getValue();
  _updatePeriod = updatePeriodCfg.getValue();
  _initialCapacity = initialCapacityCfg.getValue();
  _globalThreshold = globalThresholdCfg.getValue();
  _criticalThreshold = criticalThresholdCfg.getValue();
  _trxThreshold = trxThresholdCfg.getValue();
  _fpStorageCheckPeriod = fpStorageCheckPeriodCfg.getValue();
  _dhCheckPeriod = dhCheckPeriodCfg.getValue();
  _monitorUpdatePeriod = memoryMonitorUpdatePeriod.getValue();
  _monitorUpdatePeriodGranularity = memoryMonitorUpdatePeriodGranularity.getValue();
  _criticalFreeRssWatermark = critiFreeRssWatermark.getValue();
  _softFreeRssWatermark = warnFreeRssWatermark.getValue();

  checkThresholdsAndWatermarks();

  _fpStorageCheckPeriod = std::max(_fpStorageCheckPeriod, FP_STORAGE_CHECK_PERIOD_MIN);
  _dhCheckPeriod = std::max(_dhCheckPeriod, DH_CHECK_PERIOD_MIN);

  if (_managerEnabled)
    _changesFallback = fallback::fixed::memoryManagerChanges();
  else
    _changesFallback = true;
}
}
}
