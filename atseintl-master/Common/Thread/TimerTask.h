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

#include "Common/Assert.h"
#include "Common/Thread/TseCallableTask.h"
#include "Util/Time.h"

#include <stdint.h>

namespace tse
{
class TimerTaskExecutor;

class TimerTask : public TseCallableTask
{
  friend class TimerTaskExecutor;

public:
  static constexpr int DEFAULT_GRANULARITY_FACTOR = 10;
  using Clock = Time::Clock;

  enum class State
  {
    // Free to schedule.
    UNUSED,
    // Waiting to be executed.
    SCHEDULED,
    // Executing at the moment.
    EXECUTING,
    // Executing, but already cancelled on another thread; will be UNUSED soon.
    EXECUTING_CANCELLED,
  };

  enum OneShotT { ONE_SHOT };
  enum RepeatingT { REPEATING };

  Clock::duration period() const { return _period; }
  Clock::duration granularity() const { return _granularity; }
  State state() const { return _state; }

  bool isRepeating() const { return _period != Clock::duration::zero(); }

protected:
  TimerTask() = default;
  TimerTask(Clock::duration granularity) : _granularity(granularity) {}

  TimerTask(OneShotT) {}
  TimerTask(OneShotT, Clock::duration granularity) : _granularity(granularity) {}

  TimerTask(RepeatingT, Clock::duration period)
    : _period(period), _granularity(period / int(DEFAULT_GRANULARITY_FACTOR))
  {
  }
  TimerTask(RepeatingT, Clock::duration period, Clock::duration granularity)
    : _period(period), _granularity(granularity)
  {
    TSE_ASSERT(period >= granularity);
  }

  // NOTE BEGIN: Don't call these directly other than from TimerTaskExecutor.

  virtual void onCancel() {}

  void setState(State state) { _state = state; }

  void* data() const { return _data; }
  void setData(void* data) { _data = data; }

  // NOTE END:   Don't call these directly other than from TimerTaskExecutor.

private:
  const Clock::duration _period = Clock::duration::zero();
  const Clock::duration _granularity = std::chrono::seconds(1);
  void* _data = nullptr;
  State _state = State::UNUSED;
};
}
