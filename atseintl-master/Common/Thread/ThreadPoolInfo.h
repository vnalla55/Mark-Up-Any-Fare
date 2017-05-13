#pragma once

#include "Common/Thread/TseThreadingConst.h"

#include <set>
#include <string>
#include <vector>

namespace tse
{

struct ThreadPoolInfo
{
  ThreadPoolInfo(TseThreadingConst::TaskId taskId,
                 int size);

  ThreadPoolInfo(TseThreadingConst::TaskId taskId,
                 const std::string& group,
                 const std::string& keySize);

  static const ThreadPoolInfo& get(TseThreadingConst::TaskId taskId);
  static int getBoosterThreshold(TseThreadingConst::TaskId taskId);
  static int getBoosterAdjustmentPeriod();
  static int getBoosterMaxSize();
  static bool getBoosterExcludedTasks(std::set<int>& excludedTasks);
  static double getBoosterSizeSlope();
  static bool getBoosterSizes(std::vector<double>& sizes);
  static unsigned getBoosterMinSize();
  static bool getBoosterConsumeBacklog(TseThreadingConst::TaskId taskId);
  static bool getBoosterConsumeBacklog2(TseThreadingConst::TaskId taskId);
  static bool getBoosterCombineTasks(TseThreadingConst::TaskId taskId);
  static unsigned getTransactionThreshold();
  static int getIdleThreadTimeout();
  static unsigned getBoosterCombineThreshold(TseThreadingConst::TaskId taskId);

  const TseThreadingConst::TaskId _taskId;
  const int _size;
};

}// tse
