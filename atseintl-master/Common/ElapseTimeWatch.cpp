// ElapseTimeWatch.C
//
#include "Common/ElapseTimeWatch.h"

#include "Util/BranchPrediction.h"

#ifdef __linux__
#include <time.h>

#endif

namespace tse
{

ElapseTimeWatch::ElapseTimeWatch()
{
  reset();
}

void
ElapseTimeWatch::reset()
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
}

bool
ElapseTimeWatch::start()
{
  if (UNLIKELY(_state == STARTED))
    return false; // failure

  if (LIKELY(_state == STOPPED))
    reset();

  gettimeofday(&_start, nullptr);
  times(&_tmsStart);

  _state = STARTED;
  return true; // success
}

bool
ElapseTimeWatch::stop()
{
  if (_state == STOPPED)
    return false; // failure

  if (LIKELY(_state == STARTED))
    update();

  _state = STOPPED;
  return true; // success
}

bool
ElapseTimeWatch::pause()
{
  if (_state == STOPPED || _state == PAUSED)
    return false; // failure

  update();

  _state = PAUSED;
  return true; // success
}

void
ElapseTimeWatch::update()
{
  struct timeval stop;
  struct tms tmsStop;

  // Get time stamps
  //
  gettimeofday(&stop, nullptr);
  times(&tmsStop);

  // Real elapsed
  //
  if ((stop.tv_usec - _start.tv_usec) < 0)
  {
    stop.tv_sec -= 1;
    stop.tv_usec += USEC_PER_SEC;
  }

  // Elapsed
  //
  _elapsedTime += double(stop.tv_sec - _start.tv_sec) +
                  double(stop.tv_usec - _start.tv_usec) / double(USEC_PER_SEC);
}

double
ElapseTimeWatch::elapsedTime()
{
  if (UNLIKELY(_state == STARTED))
  {
    pause();
    start();
  }

  return _elapsedTime;
}

std::ostream& operator<<(std::ostream& stream, ElapseTimeWatch& etw)
{
  const bool paused = (etw._state == etw.STARTED);
  if (paused)
    etw.pause();

  stream << "     Elapsed Time (Sec): " << etw._elapsedTime << std::endl;

  if (paused)
    etw.start();

  return stream;
}

} // namespace tse
