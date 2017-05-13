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
#include "Util/Time.h"

#include <cassert>
#include <cerrno>
#include <ctime>

namespace tse
{
namespace Time
{
void sleepUntilImpl(TimePoint now, TimePoint until)
{
  struct timespec ts;
  do
  {
    auto diff = until - now;
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(diff - sec);

    ts.tv_sec = static_cast<std::time_t>(sec.count());
    ts.tv_nsec = static_cast<long>(nsec.count());

    if (::nanosleep(&ts, nullptr) == 0)
      break;

    assert(errno == EINTR);

    now = Clock::now();
  }
  while (now < until);
}
}
}
