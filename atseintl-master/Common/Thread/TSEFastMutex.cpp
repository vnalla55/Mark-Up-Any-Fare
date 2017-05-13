#include "Common/Thread/TSEFastMutex.h"

void
TSEFastMutex::acquire()
{
  while (!atomic_dec_and_test(&_m))
  {
    atomic_inc(&_m);
    usleep(1);
  }
}
