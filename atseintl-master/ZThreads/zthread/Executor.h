#pragma once

#include "ZThreads/zthread/Waitable.h"
#include "ZThreads/zthread/Cancelable.h"

namespace ZThread
{

class Task;

class Executor:
  public Cancelable,
  public Waitable
{
public:
  Executor() {}
  virtual ~Executor() {}
  virtual void interrupt() = 0;
  virtual void execute(const Task& task) = 0;

private:
  Executor(const Executor&);
  void operator=(const Executor&);
};

} // namespace ZThread

