#include "Common/Thread/TseRunnableExecutor.h"

#include "Common/Logger.h"
#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseThreadPool.h"

namespace tse
{
namespace
{
Logger logger("atseintl.Common.TseRunnableExecutor");

Logger statsLogger("atseintl.ThreadTaskStatistics");

uint64_t _numberSamples[TseThreadingConst::NUMBER_TASK_TYPE] = {};

uint64_t
getCurrentMillis()
{
  return clock() * 1000 / CLOCKS_PER_SEC;
}

}// namespace

TseRunnableExecutor::TseRunnableExecutor(TseThreadingConst::TaskId taskId)
  : _taskId(taskId)
  , _threads(ThreadPoolFactory::instance(taskId))
  , _exceptionCode(ErrorResponseException::NO_ERROR)
{
}

TseRunnableExecutor::TseRunnableExecutor(TseThreadingConst::TaskId taskId,
                                         size_t maxActiveSize)
  : _taskId(taskId)
  , _scopedThreadPool(new TseThreadPool(TseThreadingConst::SCOPED_EXECUTOR_TASK, maxActiveSize))
  , _threads(_scopedThreadPool.get())
  , _exceptionCode(ErrorResponseException::NO_ERROR)
{
}

TseRunnableExecutor::~TseRunnableExecutor()
{
  try
  {
    wait(false);
  }
  catch(...)
  {
    // ignore
  }
}

void TseRunnableExecutor::execute(TseCallableTask& runnable)
{
  execute(&runnable);
}

void TseRunnableExecutor::execute(TseCallableTask* runnable,
                                  bool destroyRunnable)
{
  if (0 == _threads)
  {
    if (LIKELY(runnable))
    {
      runnable->run();

      if (UNLIKELY(destroyRunnable))
      {
        delete runnable;
      }
    }
    return;
  }
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    ++_count;
  }
  TseRunnableWrapper task(runnable, this, destroyRunnable);
  _threads->enqueue(task);
}

void TseRunnableExecutor::wait(bool dorethrow)
{
  if (0 == _threads)
  {
    return;
  }
  uint64_t millis(getCurrentMillis());
  boost::unique_lock<boost::mutex> lock(_mutex);
  if (dorethrow) // remove logging in the destructor
  {
    LOG4CXX_DEBUG(logger, __FUNCTION__ << ":_count=" << _count);
  }
  while (_count != 0)
  {
    _condition.wait(lock);
  }
  assert(0 == _count);
  if (dorethrow)
  {
    recordStats(millis);
    if (_exceptionCode != ErrorResponseException::NO_ERROR)
    {
      throw ErrorResponseException(_exceptionCode, _exceptionMessage.c_str());
    }
  }
}

void TseRunnableExecutor::recordFirstException(const ErrorResponseException& e)
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  if (ErrorResponseException::NO_ERROR == _exceptionCode)
  {
    _exceptionCode = e.code();
    _exceptionMessage = e.message();
  }
}

void TseRunnableExecutor::recordFirstException(const std::exception& e)
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  if (ErrorResponseException::NO_ERROR == _exceptionCode)
  {
    _exceptionCode = ErrorResponseException::SYSTEM_EXCEPTION;
    _exceptionMessage = e.what();
  }
}

void TseRunnableExecutor::recordFirstException()
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  if (ErrorResponseException::NO_ERROR == _exceptionCode)
  {
    _exceptionCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    _exceptionMessage = "unknown exception";
  }
}

TseTaskMetrics TseRunnableExecutor::getMetrics()
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  _metrics._currentActiveTasks = _count;
  return _metrics;
}

void TseRunnableExecutor::decrementTaskCount()
{
  boost::lock_guard<boost::mutex> lock(_mutex);
  --_count;
  _condition.notify_one();
}

size_t TseRunnableExecutor::size() const
{
  return _threads ? _threads->size() : 0;
}

void
TseRunnableExecutor::recordStats(uint64_t startMillis) const
{
  static int period(TseThreadingConst::getStatsPeriod());
  if (LIKELY(_threads && period > 0))
  {
    ++_numberSamples[_taskId];
    static unsigned threshold(TseThreadingConst::getStatsThreshold());
    static size_t period2(period > 10 ? period / 10 : 1);
    size_t queueSize(_threads->getQueueSize());
    if (0 == _numberSamples[_taskId] % period
        || (queueSize >= threshold && 0 == _numberSamples[_taskId] % period2))
    {
      size_t activeThreads(_threads->getNumberActiveThreads());
      size_t poolSize(_threads->size());
      uint64_t executionTime(0);
      if (startMillis != 0)
      {
        executionTime = getCurrentMillis() - startMillis;
      }
      const BoosterThreadPool* booster(_threads->getBooster());
      size_t boosterEffPoolSize(booster ? booster->getEffPoolSize() : 0);
      size_t boosterActiveThreads(booster ? booster->getNumberActiveThreads() : 0);
      LOG4CXX_INFO(statsLogger,
                   TseThreadingConst::getTaskName(_taskId)
                   << ":samples=" << _numberSamples[_taskId]
                   << ",combinedTasks=" << _threads->getNumberCombinedTasks()
                   << ",poolSize=" << poolSize
                   << ",queueSize=" << queueSize
                   << ",activeThreads=" << activeThreads
                   << ",executionTime=" << executionTime << "ms"
                   << ",concurrency=" << TseThreadPool::getConcurrency()
                   << ",boosterEffPoolSize=" << boosterEffPoolSize
                   << ",boosterActiveThreads=" << boosterActiveThreads);
    }
  }
}

}// tse
