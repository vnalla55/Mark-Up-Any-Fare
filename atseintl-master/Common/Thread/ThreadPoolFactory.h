#pragma once

#include "Common/Thread/TseThreadingConst.h"

#include <cstddef>

namespace tse
{
class TseThreadPool;
class BoosterThreadPool;

namespace ThreadPoolFactory
{
extern bool _metrics;

TseThreadPool* instance(TseThreadingConst::TaskId taskId);
BoosterThreadPool* getBoosterPool(TseThreadingConst::TaskId taskId);
size_t getNumberActiveThreads(TseThreadingConst::TaskId taskId);

inline void enableMetrics(bool shouldMonitor) { _metrics = shouldMonitor; }
inline bool isMetricsEnabled() { return _metrics; }

}// ThreadPoolFactory
}// tse
