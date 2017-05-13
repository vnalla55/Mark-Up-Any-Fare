#include "Common/Thread/TSEReadWriteLock.h"

#include <pthread.h>
#include <unistd.h>

int
TSEReadWriteLock::acquireReadSearch()
{
  for (;;)
  {
    for (int n = 0; n != NumMutex; ++n)
    {
      if (atomic_dec_and_test(&_m[n].mutex))
      {
        return n;
      }
      else
      {
        atomic_inc(&_m[n].mutex);
      }
    }

    usleep(1);
  }
}

TSEReadWriteLock::TSEReadWriteLock()
{
  atomic_t m = ATOMIC_INIT(1);
  for (int n = 0; n != NumMutex; ++n)
  {
    _m[n].mutex = m;
  }
}

int
TSEReadWriteLock::acquireRead()
{
  const int index = static_cast<int>(pthread_self() % NumMutex);
  if (atomic_dec_and_test(&_m[index].mutex))
  {
    return index;
  }

  atomic_inc(&_m[index].mutex);

  return acquireReadSearch();
}

void
TSEReadWriteLock::acquireWrite()
{
  for (int n = 0; n != NumMutex; ++n)
  {
    while (!atomic_dec_and_test(&_m[n].mutex))
    {
      atomic_inc(&_m[n].mutex);
      usleep(1);
    }
  }
}

void
TSEReadWriteLock::releaseWrite()
{
  for (int n = 0; n != NumMutex; ++n)
  {
    atomic_inc(&_m[n].mutex);
  }
}
