// ElapseTimeWatch.h
//
#pragma once

#include <limits>

#ifdef __linux__
#include <sys/time.h>
#include <sys/times.h>

#else
#include <sys/times.h>

#endif
#include <ostream>

namespace tse
{

class ElapseTimeWatch
{
  static constexpr int USEC_PER_SEC = 1000000;

  // Time structures
  //
  struct timeval _start;
  struct tms _tmsStart;

  // Elapsed times (usec resolution)
  //
  double _elapsedTime = 0;

  // Stopwatch state
  //
  int _state = STOPPED;

  void reset(); // clears elapsed times
  void update(); // updates elapsed times

public:
  enum
  { STOPPED = 0,
    STARTED,
    PAUSED };

  ElapseTimeWatch();
  virtual ~ElapseTimeWatch() = default;

  // Functional interface
  //
  bool start();
  bool stop();
  bool pause();

  // Access
  //
  double elapsedTime();

  friend std::ostream& operator<<(std::ostream& stream, ElapseTimeWatch& etw);
};
} // namespace tse
