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

#include <gmock/gmock.h>

namespace tse
{
class MockTimerTask : public TimerTask
{
public:
  MockTimerTask() = default;
  MockTimerTask(Clock::duration granularity) : TimerTask(granularity) {}

  MockTimerTask(OneShotT x) : TimerTask(x) {}
  MockTimerTask(OneShotT x, Clock::duration granularity) : TimerTask(x, granularity) {}

  MockTimerTask(RepeatingT x, Clock::duration period) : TimerTask(x, period) {}
  MockTimerTask(RepeatingT x, Clock::duration period, Clock::duration granularity)
    : TimerTask(x, period, granularity)
  {
  }

  MOCK_METHOD0(onCancel, void());
  MOCK_METHOD0(run, void());
};
}
