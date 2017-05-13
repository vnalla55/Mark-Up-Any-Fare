#include "Common/Thread/ThreadPoolInfo.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/Logger.h"

#include <boost/tokenizer.hpp>

namespace tse
{

namespace
{

Logger logger("atseintl.Common.ThreadPoolInfo");

int getConfigValue(const std::string& name,
                   int defaultValue,
                   const std::string& group)
{
  int valueCfg(defaultValue);
  if (Global::hasConfig())
  {
    Global::config().getValue(name, valueCfg, group);
  }
  LOG4CXX_DEBUG(logger, __FUNCTION__ << " name: " << name
                << " group: " << group << " valueCfg: " << valueCfg);
  return valueCfg;
}

}// namespace

ThreadPoolInfo::ThreadPoolInfo(TseThreadingConst::TaskId taskId,
                               int size)
  : _taskId(taskId)
  , _size(size)
{
}

ThreadPoolInfo::ThreadPoolInfo(TseThreadingConst::TaskId taskId,
                               const std::string& group,
                               const std::string& keySize)
  : _taskId(taskId)
  , _size(getConfigValue(keySize, -1, group))
{
}

namespace
{

template<TseThreadingConst::TaskId n> const ThreadPoolInfo& getPoolInfo(const std::string& group,
                                                                        const std::string& keySize)
{
  static const ThreadPoolInfo poolInfo(n, group, keySize);
  return poolInfo;
}

bool getPoolSize(const std::string& group,
                 const std::string& key,
                 int& size)
{
  bool result(false);
  if (Global::hasConfig())
  {
    result = Global::config().getValue(key, size, group);
  }
  LOG4CXX_DEBUG(logger, __FUNCTION__ << ",key: " << key
                << ",group:" << group << ",size:" << size);
  return result;
}

}// namespace

unsigned ThreadPoolInfo::getTransactionThreshold()
{
  unsigned threshold(16);
  if (Global::hasConfig())
  {
    Global::config().getValue("TRX_THRESHOLD", threshold, "TSE_SERVER");
  }
  return threshold;
}

const ThreadPoolInfo& ThreadPoolInfo::get(TseThreadingConst::TaskId taskId)
{
  bool boosterEnabled(getBoosterMaxSize() > 0);
  switch (taskId)
  {
  case TseThreadingConst::TRANSACTION_TASK:
    {
      if (!boosterEnabled)
      {
        return getPoolInfo<TseThreadingConst::TRANSACTION_TASK>("TSE_SERVER", "TRX_POOLINGTHRESHOLD");
      }
      else
      {
        int size(0);
        bool exist(getPoolSize("TSE_SERVER", "TRX_POOLSIZE", size));
        if (!exist)
        {
          size = getTransactionThreshold();
        }
        static ThreadPoolInfo poolInfo(TseThreadingConst::TRANSACTION_TASK, size);
        return poolInfo;
      }
    }
    break;
  case TseThreadingConst::TRANSACTION_ASYNCHRONOUS_TASK:
    {
      static const std::string key(boosterEnabled ? "ASYNC_TASK_POOLSIZE" : "ASYNC_TASK_NUM_THREADS");
      return getPoolInfo<TseThreadingConst::TRANSACTION_ASYNCHRONOUS_TASK>("TSE_MANAGER_UTIL", key);
    }
    break;
  case TseThreadingConst::PRICING_ITIN_TASK:
    {
      static const std::string key(boosterEnabled ? "ITIN_POOLSIZE" : "ITIN_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::PRICING_ITIN_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::PRICING_ITIN_ASE_TASK:
    {
      static const std::string key(boosterEnabled ? "ITIN_ASE_POOLSIZE" : "ITIN_ASE_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::PRICING_ITIN_ASE_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::FAREPATHPQ_TASK:
    {
      static const std::string key(boosterEnabled ? "FAREPATHPQ_POOLSIZE" : "FAREPATHPQ_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FAREPATHPQ_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::PRICINGUNITPQ_TASK:
    {
      static const std::string key(boosterEnabled ? "PRICINGUNITPQ_POOLSIZE" : "PRICINGUNITPQ_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::PRICINGUNITPQ_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::FAREPATHCOLLECTOR_TASK:
    {
      static const std::string key(boosterEnabled ? "FAREPATHCOLLECTOR_POOLSIZE" : "FAREPATHCOLLECTOR_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FAREPATHCOLLECTOR_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::INITFAREPATH_TASK:
    {
      static const std::string key(boosterEnabled ? "INITFAREPATH_POOLSIZE" : "INITFAREPATH_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::INITFAREPATH_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::NETFAREMARKETS_TASK:
    {
      static const std::string key(boosterEnabled ? "NETFAREMARKETS_POOLSIZE" : "NETFAREMARKETS_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::NETFAREMARKETS_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::PRICINGPQ_TASK:
    {
      static const std::string key(boosterEnabled ? "PRICINGPQ_POOLSIZE" : "PRICINGPQ_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::PRICINGPQ_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::PUPATHMATRIX_TASK:
    {
      static const std::string key(boosterEnabled ? "PUPATHMATRIX_POOLSIZE" : "PUPATHMATRIX_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::PUPATHMATRIX_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::SURCHARGES_TASK:
    {
      static const std::string key(boosterEnabled ? "SURCHARGES_POOLSIZE" : "SURCHARGES_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::SURCHARGES_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::GATEWAY_TASK:
    {
      static const std::string key(boosterEnabled ? "GATEWAY_POOLSIZE" : "GATEWAY_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::GATEWAY_TASK>("ADDON_CONSTRUCTION", key);
    }
    break;
  case TseThreadingConst::VENDOR_TASK:
    {
      static const std::string key(boosterEnabled ? "VENDOR_POOLSIZE" : "VENDOR_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::VENDOR_TASK>("ADDON_CONSTRUCTION", key);
    }
    break;
  case TseThreadingConst::FARESC_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FARESC_TASK>("FARESC_SVC", key);
    }
    break;
  case TseThreadingConst::FARESV_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FARESV_TASK>("FARESV_SVC", key);
    }
    break;
  case TseThreadingConst::FARE_CALC_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FARE_CALC_TASK>("FARE_CALC_SVC", key);
    }
    break;
  case TseThreadingConst::FARE_SELECTOR_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::FARE_SELECTOR_TASK>("FARE_SELECTOR_SVC", key);
    }
    break;
  case TseThreadingConst::BAGGAGE_TASK:
    {
      static const std::string key(boosterEnabled ? "BAGGAGETASK_POOLSIZE" : "BAGGAGETASK_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::BAGGAGE_TASK>("FREE_BAGGAGE", key);
    }
    break;
  case TseThreadingConst::REX_FARE_SELECTOR_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::REX_FARE_SELECTOR_TASK>("REX_FARE_SELECTOR_SVC", key);
    }
    break;
  case TseThreadingConst::SVCFEESCOLLECTOR_TASK:
    {
      static const std::string key(boosterEnabled ? "SVCFEESCOLLECTOR_POOLSIZE" : "SVCFEESCOLLECTOR_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::SVCFEESCOLLECTOR_TASK>("SERVICE_FEES_SVC", key);
    }
    break;
  case TseThreadingConst::SHOPPING_TASK:
    {
      static const std::string key(boosterEnabled ? "SHOPPINGPQ_POOLSIZE" : "SHOPPINGPQ_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::SHOPPING_TASK>("SHOPPING_SVC", key);
    }
    break;
  case TseThreadingConst::SHOPPINGPQ_TASK:
    {
      static const std::string key(boosterEnabled ? "SHOPPINGPQ_POOLSIZE" : "SHOPPINGPQ_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::SHOPPINGPQ_TASK>("PRICING_SVC", key);
    }
    break;
  case TseThreadingConst::TAX_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::TAX_TASK>("TAX_SVC", key);
    }
    break;
  case TseThreadingConst::TO_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::TO_TASK>("TO_SVC", key);
    }
    break;
  case TseThreadingConst::LDC_TASK:
    {
      static const std::string key(boosterEnabled ? "SERIALIZE_QUEUE_POOLSIZE" : "SERIALIZE_QUEUE_THREAD_MAX");
      return getPoolInfo<TseThreadingConst::LDC_TASK>("DISK_CACHE_OPTIONS", key);
    }
    break;
  case TseThreadingConst::LDC_FULL_VERIFY_TASK:
  case TseThreadingConst::LDC_CACHE_EVENT_TASK:
  case TseThreadingConst::LDC_ACTION_QUEUE_TASK:
    {
      static const std::string key(boosterEnabled ? "QUEUE_POOLSIZE" : "QUEUE_THREAD_MAX");
      return getPoolInfo<TseThreadingConst::LDC_TASK>("DISK_CACHE_OPTIONS", key);
    }
    break;
  case TseThreadingConst::CACHE_UPDATE_TASK:
    {
      static const std::string key(boosterEnabled ? "POOLSIZE" : "POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::CACHE_UPDATE_TASK>("CACHE_ADP", key);
    }
    break;
  case TseThreadingConst::THROTTLING_TASK:
    {
      static const std::string key(boosterEnabled ? "THROTTLING_POOLSIZE" : "THROTTLING_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::THROTTLING_TASK>("TSE_SERVER", key);
    }
    break;
  case TseThreadingConst::LOAD_ON_UPDATE_TASK:
    {
      static const std::string key(boosterEnabled ? "LOAD_ON_UPDATE_POOLSIZE" : "LOAD_ON_UPDATE_POOLINGTHRESHOLD");
      return getPoolInfo<TseThreadingConst::LOAD_ON_UPDATE_TASK>("CACHE_ADP", key);
    }
    break;
  case TseThreadingConst::SYNCHRONOUS_TASK:
    {
      static ThreadPoolInfo poolInfo(TseThreadingConst::SYNCHRONOUS_TASK, -1);
      return poolInfo;
    }
    break;
  case TseThreadingConst::TASK_ID_ANY:
  default:
    {
      static ThreadPoolInfo poolInfo(TseThreadingConst::TASK_ID_ANY, 13);
      return poolInfo;
    }
    break;
  }
}

namespace
{

int getDefaultBoosterThreshold()
{
  int threshold(1);
  int value(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_THRESHOLD", value, "TSE_SERVER")
      && value > 0)
  {
    threshold = value;
  }
  return threshold;
}

int getTaskBoosterThreshold(const std::string& name,
                            const std::string& group)
{
  static int defaultThreshold(getDefaultBoosterThreshold());
  int value(0);
  if (Global::hasConfig()
      && Global::config().getValue(name, value, group)
      && value > 0)
  {
    return value;
  }
  return defaultThreshold;
}

int getBoosterMaxSizeImpl()
{
  int maxSize(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_POOL_MAX", maxSize, "TSE_SERVER")
      && maxSize > -1)
  {
    return maxSize;
  }
  return 0;
}

}// namespace

int ThreadPoolInfo::getBoosterMaxSize()
{
  static int maxSize(getBoosterMaxSizeImpl());
  return maxSize;
}

bool ThreadPoolInfo::getBoosterExcludedTasks(std::set<int>& excludedTasks)
{
  if (Global::hasConfig())
  {
    std::string excludes;
    Global::config().getValue("BOOSTER_EXCLUDE_TASKS", excludes, "TSE_SERVER");
    boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char> > tokens(excludes, sep);
    for (const auto& task : tokens)
    {
      int excluded(atoi(task.c_str()));
      excludedTasks.insert(excluded);
    }
  }
  return true;
}

bool ThreadPoolInfo::getBoosterCombineTasks(TseThreadingConst::TaskId taskId)
{
  std::set<int> set;
  if (Global::hasConfig())
  {
    std::string combined;
    if (Global::config().getValue("BOOSTER_COMBINED_TASKS", combined, "TSE_SERVER"))
    {
      boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
      boost::tokenizer<boost::char_separator<char> > tokens(combined, sep);
      for (const auto& task : tokens)
      {
        int id(atoi(task.c_str()));
        set.insert(id);
      }
    }
  }
  return set.find(taskId) != set.end();
}

int ThreadPoolInfo::getBoosterThreshold(TseThreadingConst::TaskId taskId)
{
  static int defaultThreshold(getDefaultBoosterThreshold());
  switch (taskId)
  {
  case TseThreadingConst::TRANSACTION_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_TRX_THRESHOLD", "TSE_SERVER"));
      return threshold;
    }
    break;
  case TseThreadingConst::FARESC_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THRESHOLD", "FARESC_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::FARESV_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THRESHOLD", "FARESV_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::BAGGAGE_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THRESHOLD", "FREE_BAGGAGE"));
      return threshold;
    }
    break;
  case TseThreadingConst::FAREPATHPQ_TASK:
    {
      static int threshold(getTaskBoosterThreshold("FAREPATHPQ_BOOSTER_THRESHOLD", "PRICING_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::PRICINGUNITPQ_TASK:
    {
      static int threshold(getTaskBoosterThreshold("PRICINGUNITPQ_BOOSTER_THRESHOLD", "PRICING_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::SHOPPING_TASK:
    {
      static int threshold(getTaskBoosterThreshold("SHOPPING_BOOSTER_THRESHOLD", "SHOPPING_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::SHOPPINGPQ_TASK:
    {
      static int threshold(getTaskBoosterThreshold("SHOPPINGPQ_BOOSTER_THRESHOLD", "PRICING_SVC"));
      return threshold;
    }
    break;
  case TseThreadingConst::THROTTLING_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THROTTLING_THRESHOLD", "TSE_SERVER"));
      return threshold;
    }
    break;
  case TseThreadingConst::CACHE_UPDATE_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THRESHOLD", "CACHE_ADP"));
      return threshold;
    }
    break;
  case TseThreadingConst::TRANSACTION_ASYNCHRONOUS_TASK:
    {
      static int threshold(getTaskBoosterThreshold("BOOSTER_THRESHOLD", "TSE_MANAGER_UTIL"));
      return threshold;
    }
    break;
  default:
    break;
  }
  return defaultThreshold;
}

int ThreadPoolInfo::getBoosterAdjustmentPeriod()
{
  int period(10000);
  int value(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_ADJUSTMENT_PERIOD", value, "TSE_SERVER")
      && value > -1)
  {
    period = value;
  }
  return period;
}

double ThreadPoolInfo::getBoosterSizeSlope()
{
  double slope(14);
  double value(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_SIZE_SLOPE", value, "TSE_SERVER")
      && value > 1)
  {
    slope = value;
  }
  return slope;
}

// BOOSTER_POOL_SIZE_SET = 5|10|17|23|28
bool ThreadPoolInfo::getBoosterSizes(std::vector<double>& sizes)
{
  if (Global::hasConfig())
  {
    std::string configStr;
    if (Global::config().getValue("BOOSTER_POOL_SIZE_SET", configStr, "TSE_SERVER"))
    {
      boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
      boost::tokenizer<boost::char_separator<char> > tokens(configStr, sep);
      for (const auto& token : tokens)
      {
        double sz(atof(token.c_str()));
        sizes.push_back(sz);
      }
    }
  }
  return true;
}

bool ThreadPoolInfo::getBoosterConsumeBacklog(TseThreadingConst::TaskId taskId)
{
  std::set<int> set;
  if (Global::hasConfig())
  {
    std::string configTasks;
    if (Global::config().getValue("BOOSTER_CONSUME_BACKLOG", configTasks, "TSE_SERVER"))
    {
      boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
      boost::tokenizer<boost::char_separator<char> > tokens(configTasks, sep);
      for (const auto& task : tokens)
      {
        int id(atoi(task.c_str()));
        set.insert(id);
      }
    }
  }
  return set.empty() || set.find(taskId) != set.end();
}

bool ThreadPoolInfo::getBoosterConsumeBacklog2(TseThreadingConst::TaskId taskId)
{
  std::set<int> set;
  if (Global::hasConfig())
  {
    std::string configTasks;
    if (Global::config().getValue("BOOSTER_CONSUME_BACKLOG_2", configTasks, "TSE_SERVER"))
    {
      boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
      boost::tokenizer<boost::char_separator<char> > tokens(configTasks, sep);
      for (const auto& task : tokens)
      {
        int id(atoi(task.c_str()));
        set.insert(id);
      }
    }
  }
  return set.find(taskId) != set.end();
}

int ThreadPoolInfo::getIdleThreadTimeout()
{
  int seconds(300);
  int value(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_IDLE_THREAD_TIMEOUT", value, "TSE_SERVER")
      && value > 0)
  {
    seconds = value;
  }
  return seconds;
}

namespace
{

unsigned getBoosterCombineThresholdCfg(const std::string& name,
                                       const std::string& group)
{
  unsigned threshold(0);
  unsigned value(0);
  if (Global::hasConfig()
      && Global::config().getValue(name, value, group))
  {
    threshold = value;
  }
  return threshold;
}

}// namespace

unsigned ThreadPoolInfo::getBoosterCombineThreshold(TseThreadingConst::TaskId taskId)
{
  switch(taskId)
  {
  case TseThreadingConst::FARESV_TASK:
    {
      static unsigned threshold(getBoosterCombineThresholdCfg("BOOSTER_COMBINE_THRESHOLD", "FARESV_SVC"));
      return threshold;
    }
    break;
  default:
    break;
  }
  return 0;
}

unsigned ThreadPoolInfo::getBoosterMinSize()
{
  unsigned minSize(5);
  int value(0);
  if (Global::hasConfig()
      && Global::config().getValue("BOOSTER_POOL_MIN_SIZE", value, "TSE_SERVER")
      && value > 1)
  {
    minSize = value;
  }
  return minSize;
}

}// tse
