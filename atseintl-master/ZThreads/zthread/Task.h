#pragma once

#include "ZThreads/zthread/Runnable.h"
#include "ZThreads/zthread/CountedPtr.h"

namespace ZThread
{

class Task
{
public:
  typedef CountedPtr<Runnable> RunnablePtr;

public:
  explicit
  Task(Runnable* runnable)
    : _runnable(runnable)
  {
  }

  Task(const Task& task)
    : _runnable(task._runnable)
  {
  }

  explicit
  Task(const RunnablePtr& runnable)
    : _runnable(runnable)
  {
  }

  void operator=(const Task& task)
  {
    _runnable = task._runnable;
  }

  const Runnable* get() const
  {
    return _runnable.get();
  }

  Runnable* get()
  {
    return _runnable.get();
  }

  Runnable& operator*()
  {
    return *_runnable;
  }

  const Runnable& operator*() const
  {
    return *_runnable;
  }

  Runnable* operator->()
  {
    return _runnable.get();
  }

  const Runnable* operator->() const
  {
    return _runnable.get();
  }

  void operator()() const
  {
    _runnable->run();
  }

private:
  RunnablePtr _runnable;
};

} // namespace ZThread

