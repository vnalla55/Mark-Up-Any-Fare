// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/Thread/TimerTask.h"

namespace tse
{
class TimerTask;

class TimerTaskExecutor
{
public:
  using Clock = TimerTask::Clock;

  static TimerTaskExecutor& instance();
  static bool hasInstance();
  static void setInstance(TimerTaskExecutor* instance);
  static void destroyInstance();

  virtual ~TimerTaskExecutor() {}

  // Schedule a task for immediate execution.
  virtual void scheduleNow(TimerTask& task) = 0;

  // Schedule a task for execution after some period of time.
  virtual void scheduleAfter(Clock::duration dur, TimerTask& task) = 0;

  // Schedule a task for execution at a specific point of time.
  virtual void scheduleAt(Clock::time_point tp, TimerTask& task) = 0;

  // Cancel a task.
  // NOTE: The task may still be executing on another thread. It means that you CANNOT deallocate
  // it after this call; use cancelAndWait instead.
  virtual void cancel(TimerTask& task) = 0;

  // Cancel a task and make sure it is not executing.
  // NOTE: Don't call this function from within task's run() method; it will deadlock.
  virtual void cancelAndWait(TimerTask& task) = 0;

protected:
  static void notifyTaskCancelled(TimerTask& task) { task.onCancel(); }
  static void changeTaskState(TimerTask& task, TimerTask::State state) { task.setState(state); }
  static void* getTaskData(TimerTask& task) { return task.data(); }
  static void setTaskData(TimerTask& task, void* data) { task.setData(data); }
};
}
