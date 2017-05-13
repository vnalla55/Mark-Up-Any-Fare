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

#include "Common/Thread/TimerTaskExecutor.h"
#include "Util/Vector.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace tse
{
class PriorityQueueTimerTaskExecutor final : public TimerTaskExecutor
{
  friend class PriorityQueueTimerTaskExecutorTest;

public:
  PriorityQueueTimerTaskExecutor() : PriorityQueueTimerTaskExecutor(false) {}
  ~PriorityQueueTimerTaskExecutor() override;

  void scheduleNow(TimerTask& task) override;
  void scheduleAfter(Clock::duration dur, TimerTask& task) override;
  void scheduleAt(Clock::time_point tp, TimerTask& task) override;

  void cancel(TimerTask& task) override;
  void cancelAndWait(TimerTask& task) override;

protected:
  PriorityQueueTimerTaskExecutor(bool delayThreadCreation);

  void createThread();
  void stopThread();

private:
  struct QueuedTask
  {
    QueuedTask() {}
    QueuedTask(TimerTask* task, TimerTask::Clock::time_point nextRun) : task(task), nextRun(nextRun)
    {
    }

    TimerTask* task = nullptr;
    TimerTask::Clock::time_point nextRun;

    TimerTask::Clock::time_point earliestNextRun() const { return nextRun - task->granularity(); }

    friend bool operator<(const QueuedTask& l, const QueuedTask& r)
    {
      return l.earliestNextRun() > r.earliestNextRun();
    }
  };

  using Queue = std::priority_queue<QueuedTask, Vector<QueuedTask>>;

  Queue _queue;
  std::mutex _mutex;
  std::condition_variable _threadCV, _cancelCV;
  std::thread _thread;
  bool _exiting;

  void run();

  void eraseInQueue(TimerTask* key);
  void cancelFromOutside(TimerTask& task, bool wait);
};
}
