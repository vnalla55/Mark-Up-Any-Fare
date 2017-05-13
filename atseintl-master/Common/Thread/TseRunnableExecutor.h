#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/TseTaskMetrics.h"

#include <boost/atomic.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <exception>
#include <memory>

namespace tse
{
class TseThreadPool;
class TseCallableTask;

class TseRunnableExecutor : boost::noncopyable
{
public:
  TseRunnableExecutor(TseThreadingConst::TaskId taskId);

  virtual ~TseRunnableExecutor();

  void cancel() { _canceled = true; }

  bool isCanceled() const { return _canceled; }

  void execute(TseCallableTask& task);

  virtual void execute(TseCallableTask* task, bool destroyRunnable = false);

  void wait(bool dorethrow = true);

  void recordFirstException(const ErrorResponseException& e);

  void recordFirstException(const std::exception& e);

  void recordFirstException();

  TseTaskMetrics getMetrics();

  void decrementTaskCount();

  size_t size() const;

protected:
  TseRunnableExecutor(TseThreadingConst::TaskId taskId, size_t maxActiveSize);

  void recordStats(uint64_t startMillis = 0) const;

  const TseThreadingConst::TaskId _taskId;
  std::unique_ptr<TseThreadPool> _scopedThreadPool;
  boost::atomic<bool> _canceled{false};
  unsigned _count = 0;
  TseThreadPool* _threads;
  boost::mutex _mutex;
  boost::condition_variable _condition;
  ErrorResponseException::ErrorResponseCode _exceptionCode;
  std::string _exceptionMessage;
  TseTaskMetrics _metrics;
};
}// tse
