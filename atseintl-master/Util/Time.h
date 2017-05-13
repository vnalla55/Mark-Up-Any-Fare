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

#include <chrono>

namespace tse
{
namespace Time
{
using Clock = std::chrono::steady_clock;
using Duration = Clock::duration;
using TimePoint = Clock::time_point;

void sleepUntilImpl(TimePoint now, TimePoint until);

// These are the equivalents of std::this_thread::sleep_{for,until} other than they handle
// EINTR well.
template <typename Rep, typename Period>
inline void
sleepFor(const std::chrono::duration<Rep, Period>& duration)
{
  if (duration <= duration.zero())
    return;

  auto now = Clock::now();
  sleepUntilImpl(now, now + std::chrono::duration_cast<Duration>(duration));
}

namespace detail
{
template <typename Clock2>
struct SleepUntilHelper
{
  template <typename Duration2>
  static void sleep(const std::chrono::time_point<Clock2, Duration2>& point)
  {
    sleepFor(point - Clock2::now());
  }
};

template <>
struct SleepUntilHelper<Clock>
{
  template <typename Duration2>
  static void sleep(const std::chrono::time_point<Clock, Duration2>& point)
  {
    auto clockPoint = std::chrono::time_point_cast<Duration>(point);
    auto now = Clock::now();
    if (clockPoint <= now)
      return;

    sleepUntilImpl(now, clockPoint);
  }
};
}

template <typename Clock2, typename Duration2>
inline void
sleepUntil(const std::chrono::time_point<Clock2, Duration2>& point)
{
  detail::SleepUntilHelper<Clock2>::sleep(point);
}
}
}
