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
#include <gmock/gmock.h>

namespace tse
{
class MockTimerTaskExecutor : public TimerTaskExecutor
{
public:
  static MockTimerTaskExecutor& instance()
  {
    return static_cast<MockTimerTaskExecutor&>(TimerTaskExecutor::instance());
  }

  MOCK_METHOD1(scheduleNow, void(TimerTask& task));
  MOCK_METHOD2(scheduleAfter, void(Clock::duration dur, TimerTask& task));
  MOCK_METHOD2(scheduleAt, void(Clock::time_point tp, TimerTask& task));
  MOCK_METHOD1(cancel, void(TimerTask& task));
  MOCK_METHOD1(cancelAndWait, void(TimerTask& task));
};
}
