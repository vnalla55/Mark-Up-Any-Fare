#pragma once

#include <iosfwd>

#include <sys/time.h>
#include <sys/times.h>

namespace tse
{

class StopWatch
{
public:
  enum
  { STOPPED = 0,
    STARTED,
    PAUSED };

  StopWatch();

  // Functional interface
  //
  bool start();
  bool stop();
  bool pause();

  // Access
  //
  double elapsedTime();
  double cpuTime();

  double userTime();
  double systemTime();

  double childUserTime();
  double childSystemTime();

  friend std::ostream& operator<<(std::ostream& stream, StopWatch& sw);

private:
  void reset(); // clears elapsed times
  void update(); // updates elapsed times

  static double ClkTck();

private:
  // Time structures
  ::timeval _start;
  ::tms _tmsStart;

  // Elapsed times (usec resolution)
  double _elapsedTime = 0;
  double _cpuTime = 0;

  double _userTime = 0;
  double _systemTime = 0;

  double _childUserTime = 0;
  double _childSystemTime = 0;

  // Stopwatch state
  int _state = STOPPED;
};

} // namespace tse

