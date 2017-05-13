//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef TIMER_H
#define TIMER_H

#include <ctime>

class Timer
{
public:
  Timer() : _begin(0), _time(0.0) {}

  void start() { _begin = std::clock(); }
  void stop() { _time = double(std::clock() - _begin) / CLOCKS_PER_SEC; }

  const double& time() const { return _time; }

private:
  std::clock_t _begin;
  double _time;
};

#endif // TIMER_H
