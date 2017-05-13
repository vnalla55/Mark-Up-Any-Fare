#pragma once

#include "ZThreads/zthread/Task.h"
#include "ZThreads/zthread/Executor.h"

#include <boost/atomic.hpp>

#include <memory>

namespace ZThread
{

class ThreadedExecutor:
  public Executor
{
  class ThreadStorage;

public:
  virtual ~ThreadedExecutor();
  ThreadedExecutor();
  virtual void interrupt();
  virtual void execute(const Task& task);
  virtual void cancel();
  virtual bool isCanceled();
  virtual void wait();
  virtual bool wait(unsigned long timeout);

private:
  static void threadFunction(Task task, std::shared_ptr<ThreadStorage> storage);

private:
  std::shared_ptr<ThreadStorage> _storage;
  boost::atomic<bool> _cancel;
};

} // namespace ZThread

