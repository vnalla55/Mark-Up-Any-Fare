#pragma once

#include "Common/Thread/TSELockGuards.h"
#include "Common/Thread/TSEReadWriteLock.h"

#include <algorithm>

#include <linux/version.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <tr1/unordered_map>

// #define TIMERCLOCK( enabled, object, action ) ::tse::TimeUtil::TimerClock( enabled, object,
// action )
// #define TIMERCLOCK( object, action )

namespace tse
{
class Logger;
namespace TimeUtil
{
struct TimeInterval
{
  size_t m_start = 0;
  size_t m_end = 0;
  size_t m_startcpu = 0;
  size_t m_endcpu = 0;
};

struct Time
{
  Time(const std::string& name, const std::string& operation) : m_name(name), m_operation(operation)
  {
  }
  std::string m_name;
  std::string m_operation;
  size_t time = 0;
  size_t count = 0;
  size_t cputime = 0;
  size_t maxtime = 0;
  size_t maxcputime = 0;
};

using TimeMap = std::vector<Time>;

struct TimerClock : public TimeInterval
{
  bool m_enabled;

  const size_t TIMER_NUMBER;
  TimerClock(size_t timer_number, bool enabled);

  virtual bool checkpoint(size_t& utime, size_t& stime);
  size_t user() { return m_end - m_start; }
  size_t system() { return m_endcpu - m_startcpu; }
  void start() { checkpoint(m_start, m_startcpu); }
  void stop() { checkpoint(m_end, m_endcpu); }
  static TimeMap m_map;
  static bool m_updated;
  static TSEReadWriteLock m_mutex;
  static bool updated() { return m_updated; }
  static void statistics(Logger& logger);
  static void statistics(std::ostream& os);
  virtual ~TimerClock();
};

struct Replace : public std::string
{
  Replace(const std::string& rhs, char from, char to);
};

struct Split : public std::string
{
  Split(const std::string& rhs, const std::string& delimiters);
  virtual ~Split() {}
};

struct Upper : public std::string
{

  Upper(const std::string& rhs) : std::string(rhs)
  {
    std::transform(begin(), end(), begin(), (int (*)(int))std::toupper);
  }
  virtual ~Upper() {}
};
}
}

#define MONITOR(m) TSEWriteGuard<> g##__LINE__(m)
#define TIMER_INITIALIZER(enabled, name, operation)                                                \
  volatile static bool TIMER_BOOL(false);                                                          \
  volatile static size_t TIMER_NUMBER(0);                                                          \
  if (enabled && !TIMER_BOOL)                                                                      \
  {                                                                                                \
    MONITOR(::tse::TimeUtil::TimerClock::m_mutex);                                                 \
    if (!TIMER_BOOL)                                                                               \
    {                                                                                              \
      TIMER_BOOL = true;                                                                           \
      TIMER_NUMBER = ::tse::TimeUtil::TimerClock::m_map.size();                                    \
      ::tse::TimeUtil::TimerClock::m_map.push_back(::tse::TimeUtil::Time(name, operation));        \
    }                                                                                              \
  }                                                                                                \
  ::tse::TimeUtil::TimerClock Clock(TIMER_NUMBER, enabled)

#define TIMERCLOCK(enable_timers, name, operation) TIMER_INITIALIZER(enable_timers, name, operation)

