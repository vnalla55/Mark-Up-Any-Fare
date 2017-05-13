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

#include "Common/Thread/TimerTaskExecutor.h"

#include <memory>

namespace tse
{
namespace
{
std::unique_ptr<TimerTaskExecutor> _instance;
}

TimerTaskExecutor&
TimerTaskExecutor::instance()
{
  return *_instance;
}

bool
TimerTaskExecutor::hasInstance()
{
  return static_cast<bool>(_instance);
}

void
TimerTaskExecutor::setInstance(TimerTaskExecutor* instance)
{
  _instance.reset(instance);
}

void
TimerTaskExecutor::destroyInstance()
{
  _instance.reset();
}
}
