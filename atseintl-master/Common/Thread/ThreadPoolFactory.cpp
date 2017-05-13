#include "Common/Thread/ThreadPoolFactory.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Thread/ThreadPoolInfo.h"
#include "Common/Thread/TseThreadPool.h"

namespace tse
{

namespace ThreadPoolFactory
{
bool _metrics = false;

namespace
{

Logger logger("atseintl.Common.ThreadPoolFactory");

boost::mutex poolMutex;

TseThreadPool* getThreadPool(const ThreadPoolInfo& poolInfo)
{
  boost::lock_guard<boost::mutex> lock(poolMutex);
  auto it(TseThreadPool::_poolMap.find(poolInfo._taskId));
  if (it != TseThreadPool::_poolMap.end())
  {
    return it->second.get();
  }
  else
  {
    std::shared_ptr<TseThreadPool> ptr(new TseThreadPool(poolInfo._taskId, poolInfo._size));
    TseThreadPool::_poolMap.insert(std::make_pair(poolInfo._taskId, ptr));
    return ptr.get();
  }
}

std::shared_ptr<BoosterThreadPool>
getBoosterPoolImpl()
{
  int maxSize(ThreadPoolInfo::getBoosterMaxSize());
  if (maxSize && !BoosterThreadPool::_instance)
  {
    BoosterThreadPool::_instance.reset(new BoosterThreadPool(maxSize));
  }
  return BoosterThreadPool::_instance;
}

}// namespace

TseThreadPool* instance(TseThreadingConst::TaskId taskId)
{
  TseThreadPool* result(0);
  const ThreadPoolInfo& poolInfo(ThreadPoolInfo::get(taskId));
  LOG4CXX_INFO(logger, TseThreadingConst::getTaskName(poolInfo._taskId) << ':' << poolInfo._size);
  if (poolInfo._size > -1)// -1 means no threading
  {
    result = getThreadPool(poolInfo);
  }
  return result;
}

BoosterThreadPool* getBoosterPool(TseThreadingConst::TaskId taskId)
{
  if (!Global::hasConfig()
      || TseThreadingConst::TASK_NONE == taskId
      || TseThreadingConst::LDC_TASK == taskId
      || TseThreadingConst::CACHE_INITIALIZATION_TASK == taskId
      || TseThreadingConst::SYNCHRONOUS_TASK == taskId
      || TseThreadingConst::SCOPED_EXECUTOR_TASK == taskId
      || TseThreadingConst::TASK_ID_ANY == taskId
      || TseThreadingConst::BOOSTER_TASK == taskId)
  {
    return 0;
  }
  static std::shared_ptr<BoosterThreadPool> instance(getBoosterPoolImpl());
  static std::set<int> excludedTasks;
  static const bool init(ThreadPoolInfo::getBoosterExcludedTasks(excludedTasks));
  SUPPRESS_UNUSED_WARNING(init);
  LOG4CXX_DEBUG(logger, "init=" << init);
  if (excludedTasks.find(taskId) != excludedTasks.end())
  {
    return 0;
  }
  return BoosterThreadPool::_instance.get();
}

size_t
getNumberActiveThreads(TseThreadingConst::TaskId taskId)
{
  if (TseThreadPool* pool = instance(taskId))
    return pool->getNumberActiveThreads();
  return 0;
}

}// ThreadPoolFactory

}// tse
