// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
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
#include "Common/Thread/WheelTimerTaskExecutor.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Util/Algorithm/Container.h"
#include "Util/Algorithm/Integer.h"
#include "Util/BranchPrediction.h"
#include "Util/unlock_guard.h"

#include <algorithm>
#include <type_traits>

namespace tse
{
namespace
{
Logger
logger("atseintl.Common.Thread.TimerTaskExecutor");

bool
executeTask(TimerTask& task)
{
  try
  {
    task.run();
    return true;
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(logger, "TimerTask ended with an exception: " << e.what());
    return false;
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, "TimerTask ended with an unknown exception!");
    return false;
  }
}
}

namespace wheel
{
namespace
{
// Each jiffy is 1/4 ms long.
constexpr Jiffies JIFFY_NANOSECONDS = 1000000 / 4;

Jiffies
fromDuration(Clock::duration dur)
{
  const int64_t count = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  return count / JIFFY_NANOSECONDS;
}

// NOTE Unused.
//
// Clock::duration
// toDuration(Jiffies jiffies)
// {
//   const auto nanos = std::chrono::nanoseconds(jiffies * JIFFY_NANOSECONDS);
//   return std::chrono::duration_cast<Clock::duration>(nanos);
// }

Jiffy
fromTimePoint(Clock::time_point tp)
{
  const int64_t count =
      std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
  return count / JIFFY_NANOSECONDS;
}

Clock::time_point
toTimePoint(Jiffy jiffy)
{
  const auto nanos = std::chrono::nanoseconds(jiffy * JIFFY_NANOSECONDS);
  return Clock::time_point(std::chrono::duration_cast<Clock::duration>(nanos));
}

Jiffy
now()
{
  return fromTimePoint(Clock::now());
}

Jiffy
alignJiffyUp(Jiffy jiffy, Jiffies granularity)
{
  jiffy += granularity - 1;
  jiffy &= -Jiffy(granularity);
  return jiffy;
}

bool
isDurationTooLarge(Clock::duration dur)
{
  const int64_t count = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  return (count / JIFFY_NANOSECONDS) > JIFFIES_MAX;
}

unsigned
wheelIndexFromJiffies(Jiffies jiffies)
{
  return (alg::significantBits(uint64_t(jiffies)) + WHEEL_SIZE_LOG - 1) / WHEEL_SIZE_LOG - 1;
}

unsigned
slotFromJiffy(Jiffy jiffy, unsigned wheelIndex)
{
  return (jiffy >> (wheelIndex * WHEEL_SIZE_LOG)) & WHEEL_MASK;
}

Jiffies
granularityFromDuration(Clock::duration granularityDur)
{
  if (UNLIKELY(granularityDur <= Clock::duration::zero()))
    return 1;

  if (UNLIKELY(isDurationTooLarge(granularityDur)))
    return SLEEP_MAX;

  const Jiffy granularity = fromDuration(granularityDur);
  if (UNLIKELY(granularity == 0))
    return 1;

  // Align down to the nearest power of two.
  return Jiffies(1) << (alg::significantBits(granularity) - 1);
}

unsigned
scanMask(Mask& mask)
{
  const unsigned slot = alg::trailingZeros(mask);
  mask &= ~(Mask(1) << slot);
  return slot;
}
}

struct QueuedTask
{
  TimerTask* task;
  QueuedTask** prevPlace;
  QueuedTask* next;
  Jiffy nextJiffy;
  unsigned wheelIndex;
  Jiffies overSleep;
  Jiffies period;
  Jiffies granularity;

  bool isRepeating() const { return period != 0; }

  Jiffy nextJiffyExact() const { return nextJiffy - overSleep; }

  void setNextJiffy(Jiffy jiffy)
  {
    nextJiffy = alignJiffyUp(jiffy, granularity);
    overSleep = nextJiffy - jiffy;
  }

  void insert(QueuedTask** place)
  {
    const auto next = *place;
    this->prevPlace = place;
    this->next = next;
    if (next)
      next->prevPlace = &this->next;
    *place = this;
  }

  void unlink()
  {
    *prevPlace = next;
    if (next)
      next->prevPlace = prevPlace;

    prevPlace = nullptr;
    next = nullptr;
  }

  static void moveHead(QueuedTask*& to, QueuedTask*& from)
  {
    to = from;
    if (from)
      to->prevPlace = &to;

    from = nullptr;
  }
};

static_assert(std::is_trivially_destructible<QueuedTask>::value,
              "QueuedTask must be trivially destructible");
}

WheelTimerTaskExecutor::WheelTimerTaskExecutor(bool delayThreadCreation)
  : _threadJiffy(wheel::now()),
    _wakeUpJiffy(_threadJiffy + wheel::SLEEP_MAX),
    _pool(sizeof(wheel::QueuedTask))
{
  if (!delayThreadCreation)
    createThread();
}

WheelTimerTaskExecutor::~WheelTimerTaskExecutor()
{
  stopThread();
}

void
WheelTimerTaskExecutor::scheduleNow(TimerTask& task)
{
  scheduleAt(TimerTask::Clock::now(), task);
}

void
WheelTimerTaskExecutor::scheduleAfter(Clock::duration dur, TimerTask& task)
{
  scheduleAt(TimerTask::Clock::now() + dur, task);
}

void
WheelTimerTaskExecutor::scheduleAt(Clock::time_point tp, TimerTask& task)
{
  TSE_ASSERT(task.state() == TimerTask::State::UNUSED);
  const wheel::Jiffy jiffy = wheel::fromTimePoint(tp);

  std::unique_lock<std::mutex> lock(_mutex);

  const bool expired = hasExpired(jiffy);
  if (!expired)
    TSE_ASSERT(!isJiffiesRemainingTooLarge(jiffy));

  if (task.isRepeating())
    TSE_ASSERT(!wheel::isDurationTooLarge(task.period()));

  const auto queuedTask = allocateQueuedTask(task, jiffy);

  if (expired)
  {
    queuedTask->insert(&_ready);
    changeTaskState(task, TimerTask::State::EXECUTING);
  }
  else
  {
    dispatchTask(queuedTask);
  }

  checkWakeUp(jiffy);
}

void
WheelTimerTaskExecutor::cancel(TimerTask& task)
{
  cancelFromOutside(task, false);
}

void
WheelTimerTaskExecutor::cancelAndWait(TimerTask& task)
{
  cancelFromOutside(task, true);
}

void
WheelTimerTaskExecutor::createThread()
{
  _thread = std::thread(&WheelTimerTaskExecutor::run, this);
}

void
WheelTimerTaskExecutor::stopThread()
{
  if (!_thread.joinable())
    return;

  {
    std::unique_lock<std::mutex> lock(_mutex);
    _exiting = true;
    _runnerCV.notify_one();
  }

  _thread.join();
}

void
WheelTimerTaskExecutor::run()
{
  std::unique_lock<std::mutex> lock(_mutex);

  while (!_exiting)
  {
    _runnerCV.wait_until(lock, wheel::toTimePoint(_wakeUpJiffy));

    const wheel::Jiffy nextJiffy = wheel::now();
    advanceWheels(nextJiffy);

    if (_ready)
      processReadyTasks();

    _wakeUpJiffy = nextJiffy + jiffiesToSleep();
  }

  cancelEverything();
}

void
WheelTimerTaskExecutor::advanceWheels(wheel::Jiffy nextJiffy)
{
  wheel::QueuedTask* rescheduleList = nullptr;

  const wheel::Jiffy prevJiffy = _threadJiffy;
  _threadJiffy = nextJiffy;
  const wheel::Jiffies elapsed = nextJiffy - prevJiffy;
  TSE_ASSERT(elapsed <= wheel::JIFFIES_MAX);
  if (UNLIKELY(elapsed == 0))
    return;

  const unsigned wheelIndexFullUpdateEnd = advanceFullWheels(elapsed, rescheduleList);
  advancePartialWheels(wheelIndexFullUpdateEnd, prevJiffy, rescheduleList);

  while (rescheduleList)
  {
    wheel::QueuedTask* next = rescheduleList->next;
    dispatchTask(rescheduleList);
    rescheduleList = next;
  }
}

unsigned
WheelTimerTaskExecutor::advanceFullWheels(wheel::Jiffies elapsed,
                                          wheel::QueuedTask*& rescheduleList)
{
  const unsigned end = wheel::wheelIndexFromJiffies(elapsed);

  for (unsigned wheelIndex = 0; wheelIndex < end; ++wheelIndex)
  {
    if (!_masks[wheelIndex])
      continue;

    // These wheels have all of the slots expired.
    processWheel(wheelIndex, wheel::MASK_FULL, rescheduleList);
    _masks[wheelIndex] = 0;
  }

  return end;
}

void
WheelTimerTaskExecutor::advancePartialWheels(unsigned start,
                                             wheel::Jiffy prevJiffy,
                                             wheel::QueuedTask*& rescheduleList)
{
  const unsigned end = wheel::wheelIndexFromJiffies(_threadJiffy ^ prevJiffy) + 1;

  for (unsigned wheelIndex = start; wheelIndex < end; ++wheelIndex)
  {
    if (!_masks[wheelIndex])
      continue;

    // These wheels could be expired partially.
    const unsigned beforeSlot = wheel::slotFromJiffy(prevJiffy, wheelIndex);
    const unsigned afterSlot = wheel::slotFromJiffy(_threadJiffy, wheelIndex);

    wheel::Mask mask = (wheel::Mask(1) << ((afterSlot - beforeSlot) & wheel::WHEEL_MASK)) - 1;
    mask = alg::rotateLeft(mask, ((beforeSlot + 1) & wheel::WHEEL_MASK));

    processWheel(wheelIndex, mask, rescheduleList);
    _masks[wheelIndex] &= ~mask;
  }
}

void
WheelTimerTaskExecutor::processWheel(unsigned wheelIndex,
                                     wheel::Mask mask,
                                     wheel::QueuedTask*& rescheduleList)
{
  mask &= _masks[wheelIndex];
  while (mask)
  {
    const unsigned slot = wheel::scanMask(mask);
    processSlot(_wheels[wheelIndex][slot], rescheduleList);
  }
}

void
WheelTimerTaskExecutor::processSlot(wheel::QueuedTask*& slot, wheel::QueuedTask*& rescheduleList)
{
  wheel::QueuedTask* queuedTask = slot;
  while (queuedTask)
  {
    wheel::QueuedTask* next = queuedTask->next;

    if (hasExpired(queuedTask->nextJiffy))
    {
      queuedTask->insert(&_ready);
      changeTaskState(*queuedTask->task, TimerTask::State::EXECUTING);
    }
    else
    {
      queuedTask->insert(&rescheduleList);
    }

    queuedTask = next;
  }

  slot = nullptr;
}

void
WheelTimerTaskExecutor::processReadyTasks()
{
  wheel::QueuedTask* rescheduleList = nullptr;
  wheel::QueuedTask* toRemove = nullptr;

  executeReadyTasks(rescheduleList, toRemove);
  reschedulePeriodicTasks(rescheduleList, toRemove);
  removeTasks(toRemove);
}

void
WheelTimerTaskExecutor::executeReadyTasks(wheel::QueuedTask*& readyList,
                                          wheel::QueuedTask*& toRemove)
{
  wheel::QueuedTask::moveHead(readyList, _ready);
  wheel::QueuedTask* ready = readyList;

  unlock_guard<std::mutex> unlock(_mutex);
  do
  {
    wheel::QueuedTask* next = ready->next;

    const bool reschedule = executeTask(*ready->task) && ready->isRepeating();

    if (!reschedule)
    {
      ready->unlink();
      ready->insert(&toRemove);
    }
    else
    {
      ready->setNextJiffy(ready->nextJiffyExact() + ready->period);

      // In case the next timer invocation has already expired, repeat.
      // NOTE This uses _threadJiffy without a mutex, but since only our thread may modify
      //      it, we are safe.
      if (UNLIKELY(hasExpired(ready->nextJiffy)))
        continue;
    }

    ready = next;
  } while (ready);
}

void
WheelTimerTaskExecutor::reschedulePeriodicTasks(wheel::QueuedTask* rescheduleList,
                                                wheel::QueuedTask*& toRemove)
{
  while (rescheduleList)
  {
    wheel::QueuedTask* next = rescheduleList->next;

    TimerTask& task = *rescheduleList->task;
    if (UNLIKELY(task.state() == TimerTask::State::EXECUTING_CANCELLED))
      rescheduleList->insert(&toRemove);
    else
      dispatchTask(rescheduleList);

    rescheduleList = next;
  }
}

void
WheelTimerTaskExecutor::removeTasks(wheel::QueuedTask* toRemove)
{
  bool notifyCancelCV = false;

  while (toRemove)
  {
    wheel::QueuedTask* next = toRemove->next;

    TimerTask& task = *toRemove->task;

    // In case we cancel an EXEUCUTING_CANCELLED task, we have to notify someone that waits
    // on _cancelCV in cancelFromOutside().
    notifyCancelCV |= (task.state() == TimerTask::State::EXECUTING_CANCELLED);

    notifyTaskCancelled(task);
    changeTaskState(task, TimerTask::State::UNUSED);

    deallocateQueuedTask(toRemove);

    toRemove = next;
  }

  if (notifyCancelCV)
    _cancelCV.notify_all();
}

wheel::Jiffies
WheelTimerTaskExecutor::jiffiesToSleep()
{
  const unsigned firstWheel = alg::find_index_if(_masks, [](wheel::Mask mask) { return mask != 0;});

  if (UNLIKELY(firstWheel == wheel::WHEELS))
    return wheel::SLEEP_MAX;

  const wheel::Mask mask = _masks[firstWheel];
  const unsigned slot = wheel::slotFromJiffy(_threadJiffy, firstWheel);

  // Find how for is the first pending slot.
  wheel::Jiffies jiffies = alg::trailingZeros(alg::rotateRight(mask, slot));

  // Shift by the granularity.
  jiffies <<= (firstWheel * wheel::WHEEL_SIZE_LOG);

  // Deduce the amount already progressed in the lower wheels.
  jiffies -= (_threadJiffy & ((wheel::Jiffies(1) << (firstWheel * wheel::WHEEL_SIZE_LOG)) - 1));

  return jiffies;
}

void
WheelTimerTaskExecutor::cancelEverything()
{
  for (unsigned wheelIndex = 0; wheelIndex < wheel::WHEELS; ++wheelIndex)
  {
    wheel::Mask mask = _masks[wheelIndex];
    while (mask)
    {
      const unsigned slot = wheel::scanMask(mask);

      wheel::QueuedTask* queuedTask = _wheels[wheelIndex][slot];
      do
      {
        wheel::QueuedTask* next = queuedTask->next;

        TimerTask& task = *queuedTask->task;
        notifyTaskCancelled(task);
        changeTaskState(task, TimerTask::State::UNUSED);

        deallocateQueuedTask(queuedTask);
        queuedTask = next;
      } while (queuedTask);
    }
  }
}

wheel::QueuedTask*
WheelTimerTaskExecutor::allocateQueuedTask(TimerTask& task, wheel::Jiffy jiffy)
{
  const auto queuedTask = reinterpret_cast<wheel::QueuedTask*>(_pool.malloc());

  setTaskData(task, queuedTask);

  queuedTask->task = &task;
  queuedTask->period = wheel::fromDuration(task.period());
  queuedTask->granularity = wheel::granularityFromDuration(task.granularity());
  queuedTask->setNextJiffy(jiffy);

  return queuedTask;
}

void
WheelTimerTaskExecutor::deallocateQueuedTask(wheel::QueuedTask* task)
{
  _pool.free(reinterpret_cast<void*>(task));
}

unsigned
WheelTimerTaskExecutor::dispatchTask(wheel::QueuedTask* queuedTask)
{
  const wheel::Jiffies remaining = jiffiesRemaining(queuedTask->nextJiffy);
  const unsigned wheelIndex = wheel::wheelIndexFromJiffies(remaining);
  const unsigned slot = wheel::slotFromJiffy(queuedTask->nextJiffy, wheelIndex);

  queuedTask->wheelIndex = wheelIndex;
  queuedTask->insert(&_wheels[wheelIndex][slot]);
  _masks[wheelIndex] |= (wheel::Mask(1) << slot);

  changeTaskState(*queuedTask->task, TimerTask::State::SCHEDULED);

  return wheelIndex;
}

void
WheelTimerTaskExecutor::checkWakeUp(wheel::Jiffy jiffy)
{
  if (jiffy >= _wakeUpJiffy)
    return;

  _wakeUpJiffy = 0;
  _runnerCV.notify_one();
}

void
WheelTimerTaskExecutor::cancelFromOutside(TimerTask& task, bool wait)
{
  std::unique_lock<std::mutex> lock(_mutex);

  if (task.state() == TimerTask::State::UNUSED)
    return;

  const auto queuedTask = reinterpret_cast<wheel::QueuedTask*>(getTaskData(task));

  if (LIKELY(task.state() == TimerTask::State::SCHEDULED))
  {
    // Find where does the task currently reside.
    const unsigned wheelIndex = queuedTask->wheelIndex;
    const unsigned slot = wheel::slotFromJiffy(queuedTask->nextJiffy, wheelIndex);

    queuedTask->unlink();
    deallocateQueuedTask(queuedTask);

    // Clear mask if it was the last task from the slot.
    if (!_wheels[wheelIndex][slot])
      _masks[wheelIndex] &= ~(wheel::Mask(1) << slot);

    notifyTaskCancelled(task);
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
