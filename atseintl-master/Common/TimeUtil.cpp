#include "Common/TimeUtil.h"

#include "Common/Logger.h"
#include "Util/BranchPrediction.h"

#include <cstdio>

namespace tse
{
namespace TimeUtil
{
bool
TimeUtil::TimerClock::m_updated(false);
TimeUtil::TimeMap TimeUtil::TimerClock::m_map;
TSEReadWriteLock TimeUtil::TimerClock::m_mutex;

#define KERNEL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#if 0
// #if KERNEL_VERSION(2,6,16) <= LINUX_VERSION_CODE
#error "Building with HRT do you really mean to do this?"
// #if 1
// #if defined(__GNUC__) && __GNUC__ >= 4 && KERNEL_VERSION(2,6,16) <= LINUX_VERSION_CODE
    // If we're on RHEL4 or higher and using GCC 4 or higher, we can use
    // the HRT (High Resolution Timer) API to get CPU time down to 1
    // nanosecond resolution.  Otherwise we're stuck with 10 millisecond
    // resolution which is pretty much useless.
    static const size_t nano = 1000000000 ;
    static const size_t micro= 1000000 ;
    static const size_t milli= 1000 ;
#warning "Building with HRT"
    bool TimeUtil::TimerClock::checkpoint(size_t & utime, size_t& stime)
    {
      timespec tscpu;
      timespec tsusr;
      bool rc( false );
      rc = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tscpu) == 0 && clock_gettime(CLOCK_REALTIME, &tsusr) == 0;
      // Return the time in microseconds
      if ( rc )
      {
        utime = (tsusr.tv_sec * milli) + (tsusr.tv_nsec / 1000000) ;
        stime = (tscpu.tv_sec * milli) + (tscpu.tv_nsec / 1000000) ;
      }
      return rc;
    }

#else // if 0 // don't use HRT yet.
// #if KERNEL_VERSION(2,6,16) <= LINUX_VERSION_CODE
// #warning "Linux kernel version doesn't support HRT High resolution timers[ ! usec cpu resolution
// ]"
// #warning "Building without HRT, cputime is not accurate"
//     unsigned long TimeUtil::Timer::now()
//     {
//       struct timeval tv;
//       gettimeofday(&tv, NULL);
//       return(tv.tv_usec + (tv.tv_sec * 1000000));
//     }

bool
TimeUtil::TimerClock::checkpoint(size_t& utime, size_t& stime)
{
  //       stime = utime = now() / 1000;
  static size_t CLOCK_TICKS(100);
  timeb user;
  tms cpu;
  times(&cpu);
  ftime(&user);
  utime = (static_cast<long long>(user.time) * 1000) + user.millitm;
  stime = (cpu.tms_utime + cpu.tms_stime * 1000) / CLOCK_TICKS;

  return true;
}
#endif

TimeUtil::TimerClock::TimerClock(size_t timer_number, bool enabled)
  : m_enabled(enabled), TIMER_NUMBER(timer_number)
{
  if (UNLIKELY(m_enabled))
    start();
}

void
TimeUtil::TimerClock::statistics(std::ostream& os)
{
  TSEWriteGuard<> lock(m_mutex);
  if (TimeUtil::TimerClock::updated())
  {
    TimeUtil::TimerClock::m_updated = false;
    TimeUtil::TimeMap::const_iterator mi;
    TimeUtil::TimeMap& map(tse::TimeUtil::TimerClock::m_map);
    os << "DiskCache::TimerClock::Stats " // greppable string
       << "Object,"
       << "action,"
       << "count,"
       << "time,"
       << "cputime" << std::endl;
    for (mi = map.begin(); mi != map.end(); ++mi)
    {

      os << "DiskCache::TimerClock::Stats " // greppable string
         << mi->m_name << "," << mi->m_operation << "," << mi->count << "," << mi->time << ","
         << mi->cputime << std::endl;
    }
  }
}

void
TimeUtil::TimerClock::statistics(Logger& logger)
{
  TSEWriteGuard<> lock(m_mutex);
  if (TimeUtil::TimerClock::updated())
  {
    TimeUtil::TimerClock::m_updated = false;
    TimeUtil::TimeMap::const_iterator mi;
    TimeUtil::TimeMap& map(tse::TimeUtil::TimerClock::m_map);
    LOG4CXX_INFO(logger,
                 "DiskCache::TimerClock::Stats " // greppable string
                     << "Object,"
                     << "action,"
                     << "count,"
                     << "time,"
                     << "cputime");
    for (mi = map.begin(); mi != map.end(); ++mi)
    {
      //             LOG4CXX_DEBUG( logger
      //                          ,  "DiskCache::TimerClock ["
      //                          << ti->first << "] [" << mi->first
      //                          << "] number ["
      //                          << ti->second.count
      //                          << "] time ["
      //                          << ti->second.time
      //                          << "] cputime ["
      //                          << ti->second.cputime
      //                          ) ;
      LOG4CXX_INFO(logger,
                   "DiskCache::TimerClock::Stats " // greppable string
                       << mi->m_name << "," << mi->m_operation << "," << mi->count << ","
                       << mi->time << "," << mi->cputime);
    }
  }
}

TimeUtil::TimerClock::~TimerClock()
{
  if (UNLIKELY(m_enabled))
  {
    stop();
    TSEWriteGuard<> lock(m_mutex);
    TimerClock::m_updated = true;
    Time& time(TimerClock::m_map[TIMER_NUMBER]);
    time.time += user();
    time.cputime += system();
    time.count++;
  }
}

TimeUtil::Replace::Replace(const std::string& rhs, char from, char to) : std::string(rhs)
{
  std::string& lhs(*this);
  for (auto& lh : lhs)
    if (lh == from)
      lh = to;
}

TimeUtil::Split::Split(const std::string& rhs, const std::string& delimiters) : std::string(rhs)
{
  std::size_t pos = find_last_not_of(delimiters);
  if (pos != std::string::npos)
    erase(pos + 1);
  for (size_t i = 0; i < length(); i++)
    if (operator[](i) == ' ')
      operator[](i) = '^';
}
}
}
