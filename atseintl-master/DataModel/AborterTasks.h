//------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Thread/TimerTask.h"
#include "Common/Thread/TimerTaskExecutor.h"

#include <chrono>

namespace tse
{
enum class AborterTaskMode
{
  ABORT,
  HURRY,
  HURRY_COND
};

class TrxAborter;

template <AborterTaskMode Mode>
class AborterTask : public TimerTask
{
public:
  AborterTask(TrxAborter* aborter, TimerTask::Clock::duration duration)
    : TimerTask(ONE_SHOT, std::chrono::milliseconds(50)), _aborter(aborter)
  {
    if (duration.count())
      TimerTaskExecutor::instance().scheduleAfter(duration, *this);
  }

  ~AborterTask()
  {
    TimerTaskExecutor::instance().cancelAndWait(*this);
    resetFlag();
  }

  AborterTask(const AborterTask&) = delete;
  AborterTask& operator=(const AborterTask&) = delete;

  void run() override;
  void resetFlag();

private:
  TrxAborter* _aborter = nullptr;
};
}
