#ifndef TSE_READ_WRITE_LOCK
#define TSE_READ_WRITE_LOCK

#ifndef CONFIG_SMP
#define CONFIG_SMP
#endif

#include "Common/Thread/TSEAtomic.h"

class TSEReadWriteLock
{
  static constexpr int NumMutex = 5;

  union AtomicMutex
  {
    atomic_t mutex;
    char cacheLine[64];
  };

  AtomicMutex _m[NumMutex];

  int acquireReadSearch();

public:
  TSEReadWriteLock();

  int acquireRead();

  void releaseRead(int index) { atomic_inc(&_m[index].mutex); }

  void acquireWrite();

  void releaseWrite();
};

#endif
