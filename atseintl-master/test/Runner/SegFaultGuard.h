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

#ifndef SEGFAULTGUARD_H
#define SEGFAULTGUARD_H

#include <cstdio>
#include <csignal>
#include <execinfo.h>

#include <stdint.h>

class SegFaultGuard
{
public:
  SegFaultGuard() : _prevSignalHandle(setupHandler(printStackAndThrowException)) {}

  ~SegFaultGuard() { setupHandler(_prevSignalHandle); }

protected:
  typedef void (*SignalHandle)(int);

  class Exception : public std::exception
  {
  public:
    const char* what() const throw() { return "Segmentation fault has been occurred"; }
  };

  SignalHandle setupHandler(SignalHandle handle) const { return std::signal(SIGSEGV, handle); }

  static void printStackAndThrowException(int)
  {
    printStack();
    throw Exception();
  }

  static void printStack()
  {
    const uint32_t MAX_SIZE = 20;
    void* frames[MAX_SIZE];

    uint32_t size = backtrace(frames, MAX_SIZE); // get backtrace frames to 'frames'
    std::cerr << "Execution path:" << std::endl;
    backtrace_symbols_fd(frames, size, 2); // print decoded frames to stderr
  }

  SignalHandle _prevSignalHandle;
};

#endif
