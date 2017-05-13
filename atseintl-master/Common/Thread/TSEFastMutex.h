#pragma once

// this file contains spinlock based mutex and read/write locks.
// These run faster than traditional mutexes in areas where
// the critical section is small.

#include <pthread.h>
#include <unistd.h>

#ifndef CONFIG_SMP
#define CONFIG_SMP
#endif

#include "Common/Thread/TSEAtomic.h"

class TSEFastMutex
{
  atomic_t _m;

public:
  TSEFastMutex()
  {
    atomic_t m = ATOMIC_INIT(1);
    _m = m;
  }

  TSEFastMutex(const TSEFastMutex&) = delete;
  TSEFastMutex& operator=(const TSEFastMutex&) = delete;

  void acquire();

  void release() { atomic_inc(&_m); }
};
