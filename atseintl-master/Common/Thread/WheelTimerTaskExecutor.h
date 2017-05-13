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

#pragma once

#include "Common/Thread/TimerTaskExecutor.h"

#include <condition_variable>
#include <limits>
#include <mutex>
#include <thread>

#include <boost/pool/pool.hpp>

namespace tse
{
namespace wheel
{
using Clock = TimerTask::Clock;

using Mask = uint64_t;
using Jiffy = uint64_t;
using Jiffies = uint32_t;

constexpr unsigned WHEEL_SIZE_LOG = 6;
constexpr unsigned WHEEL_SIZE = 1 << WHEEL_SIZE_LOG;
constexpr unsigned WHEEL_MASK = WHEEL_SIZE - 1;
constexpr unsigned WHEELS = 5;

constexpr Mask MASK_FULL = std::numeric_limits<Mask>::max();

constexpr Jiffies JIFFIES_MAX = (Jiffies(1) << (WHEEL_SIZE_LOG * WHEELS)) - 1;
constexpr Jiffies SLEEP_MAX = (Jiffies(1) << (WHEEL_SIZE_LOG * WHEELS - 1));

struct QueuedTask;

static_assert(WHEEL_SIZE_LOG * WHEELS < 8 * sizeof(Jiffies) - 1, "Jiffies is too small");
static_assert(WHEEL_SIZE == 8 * sizeof(Mask), "Mask should be exactly WHEEL_SIZE bits");
}

class WheelTimerTaskExecutor final : public TimerTaskExecutor
{
  friend class WheelTimerTaskExecutorTest;

public:
  WheelTimerTaskExecutor() : WheelTimerTaskExecutor(false) {}
  ~WheelTimerTaskExecutor() override;

  void scheduleNow(TimerTask& task) override;
  void scheduleAfter(Clock::duration dur, TimerTask& task) override;
  void scheduleAt(Clock::time_point tp, TimerTask& task) override;

  void cancel(TimerTask& task) override;
  void cancelAndWait(TimerTask& task) override;

protected:
  WheelTimerTaskExecutor(bool delayThreadCreation);

  void createThread();
  void stopThread();

private:
  using Wheel = wheel::QueuedTask*[wheel::WHEEL_SIZE];

  std::mutex _mutex;
  wheel::Jiffy _threadJiffy;
  wheel::Jiffy _wakeUpJiffy;

  wheel::QueuedTask* _ready = nullptr;
  bool _exiting = false;
  wheel::Mask _masks[wheel::WHEELS] = {};
  Wheel _wheels[wheel::WHEELS] = {};

  boost::pool<boost::default_user_allocator_malloc_free> _pool;
  std::condition_variable _runnerCV, _cancelCV;
  std::thread _thread;

  void run();
  void advanceWheels(wheel::Jiffy nextJiffy);
  unsigned advanceFullWheels(wheel::Jiffies elapsed, wheel::QueuedTask*& rescheduleList);
  void
  advancePartialWheels(unsigned start, wheel::Jiffy prevJiffy, wheel::QueuedTask*& rescheduleList);
  void processWheel(unsigned wheelIndex, wheel::Mask mask, wheel::QueuedTask*& rescheduleList);
  void processSlot(wheel::QueuedTask*& slot, wheel::QueuedTask*& rescheduleList);
  void processReadyTasks();
  void executeReadyTasks(wheel::QueuedTask*& readyList, wheel::QueuedTask*& toRemove);
  void reschedulePeriodicTasks(wheel::QueuedTask* rescheduleList, wheel::QueuedTask*& toRemove);
  void removeTasks(wheel::QueuedTask* toRemove);
  wheel::Jiffies jiffiesToSleep();
  void cancelEverything();

  wheel::QueuedTask* allocateQueuedTask(TimerTask& task, wheel::Jiffy jiffy);
  void deallocateQueuedTask(wheel::QueuedTask* task);

  unsigned dispatchTask(wheel::QueuedTask* queuedTask);
  void checkWakeUp(wheel::Jiffy jiffy);
  void cancelFromOutside(TimerTask& task, bool wait);

  bool hasExpired(wheel::Jiffy jiffy) const { return jiffy <= _threadJiffy; }
  bool isJiffiesRemainingTooLarge(wheel::Jiffy jiffy) const
  {
    return jiffy - _threadJiffy > wheel::JIFFIES_MAX;
  }
  wheel::Jiffies jiffiesRemaining(wheel::Jiffy jiffy) const { return jiffy - _threadJiffy; }
};
}
