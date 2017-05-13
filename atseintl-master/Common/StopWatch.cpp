#include "Common/StopWatch.h"

#include <ostream>

#include <time.h>
#include <unistd.h>

namespace tse
{

StopWatch::StopWatch() : _state(STOPPED) { reset(); }

void
StopWatch::reset()
{
  // struct timeval
  //
  _start.tv_sec = 0;
  _start.tv_usec = 0;

  // struct tms
  //
  _tmsStart.tms_utime = 0;
  _tmsStart.tms_stime = 0;
  _tmsStart.tms_cutime = 0;
  _tmsStart.tms_cstime = 0;

  // elapsed times
  //
  _elapsedTime = 0;
  _cpuTime = 0;
  _userTime = 0;
  _systemTime = 0;
  _childUserTime = 0;
  _childSystemTime = 0;
}

bool
StopWatch::start()
{
  if (_state == STARTED)
  {
    return false; // failure
  }

  if (_state == STOPPED)
  {
    reset();
  }

  ::gettimeofday(&_start, nullptr);
  ::times(&_tmsStart);

  _state = STARTED;
  return true; // success
}

bool
StopWatch::stop()
{
  if (_state == STOPPED)
  {
    return false; // failure
  }

  if (_state == STARTED)
  {
    update();
  }

  _state = STOPPED;
  return true; // success
}

bool
StopWatch::pause()
{
  if (_state == STOPPED || _state == PAUSED)
  {
    return false; // failure
  }

  update();

  _state = PAUSED;
  return true; // success
}

void
StopWatch::update()
{
  enum
  { SW_USEC_PER_SEC = 1000000 };

  ::timeval stop;
  ::tms tmsStop;

  // Get time stamps
  //
  ::gettimeofday(&stop, nullptr);
  ::times(&tmsStop);

  // Real elapsed
  //
  if ((stop.tv_usec - _start.tv_usec) < 0)
  {
    stop.tv_sec -= 1;
    stop.tv_usec += SW_USEC_PER_SEC;
  }

  // Elapsed
  _elapsedTime += double(stop.tv_sec - _start.tv_sec) +
                  double(stop.tv_usec - _start.tv_usec) / double(SW_USEC_PER_SEC);

  // User elapsed
  _userTime += double(tmsStop.tms_utime - _tmsStart.tms_utime) / ClkTck();

  // System elapsed
  _systemTime += double(tmsStop.tms_stime - _tmsStart.tms_stime) / ClkTck();

  // Child User elapsed
  _childUserTime += double(tmsStop.tms_cutime - _tmsStart.tms_cutime) / ClkTck();

  // Child System elapsed
  _childSystemTime += double(tmsStop.tms_cstime - _tmsStart.tms_cstime) / ClkTck();
}

double
StopWatch::elapsedTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _elapsedTime;
}

double
StopWatch::cpuTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _userTime + _systemTime;
}

double
StopWatch::userTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _userTime;
}

double
StopWatch::systemTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _systemTime;
}

double
StopWatch::childUserTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _childUserTime;
}

double
StopWatch::childSystemTime()
{
  if (_state == STARTED)
  {
    pause();
    start();
  }

  return _childSystemTime;
}

std::ostream& operator<<(std::ostream& stream, StopWatch& sw)
{
  const bool paused = (sw._state == sw.STARTED);
  if (paused)
  {
    sw.pause();
  }

  stream << "    User CPU Time (Sec): " << sw._userTime << std::endl;
  stream << "  System CPU Time (Sec): " << sw._systemTime << std::endl;
  stream << "   Total CPU Time (Sec): " << sw._userTime + sw._systemTime << std::endl;
  stream << "     Elapsed Time (Sec): " << sw._elapsedTime << std::endl;
  stream << "  Child User Time (Sec): " << sw._childUserTime << std::endl;
  stream << "Child System Time (Sec): " << sw._childSystemTime << std::endl;

  if (paused)
  {
    sw.start();
  }

  return stream;
}

double
StopWatch::ClkTck()
{
  static const ::clock_t clk = ::sysconf(_SC_CLK_TCK);
  return static_cast<double>(clk);
}

} // namespace sfc
