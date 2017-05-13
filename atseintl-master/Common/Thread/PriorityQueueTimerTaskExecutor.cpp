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

#include "Common/Thread/PriorityQueueTimerTaskExecutor.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Util/BranchPrediction.h"
#include "Util/unlock_guard.h"

#include <algorithm>

namespace tse
{
namespace
{
Logger
logger("atseintl.Common.Thread.TimerTaskExecutor");
}

PriorityQueueTimerTaskExecutor::PriorityQueueTimerTaskExecutor(bool delayThreadCreation)
  : _exiting(false)
{
  if (!delayThreadCreation)
    createThread();
}

PriorityQueueTimerTaskExecutor::~PriorityQueueTimerTaskExecutor()
{
  stopThread();
}

void
PriorityQueueTimerTaskExecutor::scheduleNow(TimerTask& task)
{
  scheduleAt(TimerTask::Clock::now(), task);
}

void
PriorityQueueTimerTaskExecutor::scheduleAfter(Clock::duration dur, TimerTask& task)
{
  scheduleAt(TimerTask::Clock::now() + dur, task);
}

void
PriorityQueueTimerTaskExecutor::scheduleAt(Clock::time_point tp, TimerTask& task)
{
  TSE_ASSERT(task.state() == TimerTask::State::UNUSED);

  std::unique_lock<std::mutex> lock(_mutex);

  _queue.push(QueuedTask(&task, tp));
  changeTaskState(task, TimerTask::State::SCHEDULED);

  _threadCV.notify_one();
}

void
PriorityQueueTimerTaskExecutor::cancel(TimerTask& task)
{
  cancelFromOutside(task, false);
}

void
PriorityQueueTimerTaskExecutor::cancelAndWait(TimerTask& task)
{
  cancelFromOutside(task, true);
}

void
PriorityQueueTimerTaskExecutor::createThread()
{
  _thread = std::thread(&PriorityQueueTimerTaskExecutor::run, this);
}

void
PriorityQueueTimerTaskExecutor::stopThread()
{
  if (!_thread.joinable())
    return;

  {
    std::unique_lock<std::mutex> lock(_mutex);

    while (!_queue.empty())
    {
      notifyTaskCancelled(*_queue.top().task);
      _queue.pop();
    }

    _exiting = true;
    _threadCV.notify_one();
  }

  _thread.join();
}

void
PriorityQueueTimerTaskExecutor::run()
{
  std::unique_lock<std::mutex> lock(_mutex);

  while (!_exiting)
  {
    if (UNLIKELY(_queue.empty()))
    {
      _threadCV.wait(lock);
      continue;
    }

    QueuedTask top = _queue.top();
    TimerTask& task = *top.task;
    const bool repeating = task.isRepeating();
    TSE_ASSERT(task.state() == TimerTask::State::SCHEDULED);

    if (top.earliestNextRun() > TimerTask::Clock::now())
    {
      _threadCV.wait_for(lock, top.nextRun - TimerTask::Clock::now());
      continue;
    }

    _queue.pop();

    if (repeating)
    {
      top.nextRun += task.period();
      _queue.push(top);
    }

    changeTaskState(task, TimerTask::State::EXECUTING);

    bool cancel = false;
    try
    {
      unlock_guard<std::mutex> unlock(_mutex);
      task.run();
    }
    catch (std::exception& e)
    {
      LOG4CXX_ERROR(logger, "TimerTask ended with an exception: " << e.what());
      cancel = true;
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger, "TimerTask ended with an unknown exception!");
      cancel = true;
    }

    TSE_ASSERT(task.state() == TimerTask::State::EXECUTING ||
               task.state() == TimerTask::State::EXECUTING_CANCELLED);

    if (task.state() == TimerTask::State::EXECUTING_CANCELLED)
    {
      changeTaskState(task, TimerTask::State::UNUSED);
      _cancelCV.notify_all();
      continue;
    }

    if (cancel || !repeating)
    {
      if (repeating)
        eraseInQueue(&task);
      notifyTaskCancelled(task);
      changeTaskState(task, TimerTask::State::UNUSED);
      continue;
    }

    changeTaskState(task, TimerTask::State::SCHEDULED);
  }
}

void
PriorityQueueTimerTaskExecutor::eraseInQueue(TimerTask* key)
{
  // NOTE A work-around for the lack of erase() in std::priority_queue.
  const size_t size = _queue.size();

  Vector<QueuedTask> tasks;
  tasks.reserve(size);

  for (size_t i = 0; i < size; ++i)
  {
    if (_queue.top().task != key)
      tasks.push_back(_queue.top());
    _queue.pop();
  }

  Queue queue({}, std::move(tasks));
  _queue.swap(queue);
}

void
PriorityQueueTimerTaskExecutor::cancelFromOutside(TimerTask& task, bool wait)
{
  const bool repeating = task.isRepeating();
  std::unique_lock<std::mutex> lock(_mutex);

  if (task.state() == TimerTask::State::UNUSED)
    return;

  if (task.state() != TimerTask::State::EXECUTING_CANCELLED)
  {
    if (repeating || task.state() == TimerTask::State::SCHEDULED)
      eraseInQueue(&task);

    notifyTaskCancelled(task);

    if (task.state() != TimerTask::State::EXECUTING)
      _threadCV.notify_one();
  }

  if (task.state() == TimerTask::State::SCHEDULED)
  {
    changeTaskState(task, TimerTask::State::UNUSED);
  }
  else
  {
    if (task.state() == TimerTask::State::EXECUTING)
      changeTaskState(task, TimerTask::State::EXECUTING_CANCELLED);

    if (wait)
    {
      while (task.state() != TimerTask::State::UNUSED)
        _cancelCV.wait(lock);
    }
  }
}
}
