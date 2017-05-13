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

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"
#include "test/include/ThreadFactory.h"

#include <atomic>
#include <cstring>

#include <pthread.h>
#include <signal.h>

namespace std
{
namespace chrono
{
template <typename Rep, typename Period>
void
PrintTo(const duration<Rep, Period>& duration, ostream* os)
{
  *os << duration_cast<nanoseconds>(duration).count();
}
}
}

namespace tse
{
using namespace ::testing;

class TimeTest : public Test
{
  static constexpr int SIGNO = SIGUSR2;

public:
  void TearDown() override { stopHindrance(); }

protected:
  ThreadFactory _threads;
  pthread_t _self;
  struct sigaction _osa;
  std::atomic<bool> _hindrance{false};

  void startHindrance()
  {
    _self = pthread_self();

    struct sigaction sa;
    std::memset(&sa, 0, sizeof sa);
    sa.sa_handler = [](int) {};
    sa.sa_flags = SA_RESTART;

    sigaction(SIGNO, &sa, &_osa);

    _hindrance = true;
    _threads.spawn([&]()
    {
      while (_hindrance)
      {
        pthread_kill(_self, SIGNO);
        usleep(100);
      }
    });
  }

  void stopHindrance()
  {
    _hindrance = false;
    _threads.join();

    sigaction(SIGNO, &_osa, nullptr);
  }
};

TEST_F(TimeTest, testSleepFor)
{
  auto before = Time::Clock::now();

  Time::sleepFor(std::chrono::milliseconds(50));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(50));
}

TEST_F(TimeTest, testSleepUntil)
{
  auto before = Time::Clock::now();

  Time::sleepUntil(before + std::chrono::milliseconds(60));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(60));
}

TEST_F(TimeTest, testSleepUntil_ForeignClock)
{
  auto before = Time::Clock::now();

  Time::sleepUntil(std::chrono::system_clock::now() + std::chrono::milliseconds(40));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(40));
}

TEST_F(TimeTest, testSleepFor_withHindrance)
{
  startHindrance();

  auto before = Time::Clock::now();

  Time::sleepFor(std::chrono::milliseconds(53));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(53));
}

TEST_F(TimeTest, testSleepUntil_withHindrance)
{
  startHindrance();

  auto before = Time::Clock::now();

  Time::sleepUntil(before + std::chrono::milliseconds(67));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(67));
}

TEST_F(TimeTest, testSleepUntil_foreignClock_withHindrance)
{
  startHindrance();

  auto before = Time::Clock::now();

  Time::sleepUntil(std::chrono::system_clock::now() + std::chrono::milliseconds(44));

  auto after = Time::Clock::now();

  ASSERT_GE(after - before, std::chrono::milliseconds(44));
}
}
