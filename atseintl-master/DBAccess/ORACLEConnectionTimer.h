#pragma once
#include <cstdio>

#include <oci.h>

namespace tse
{
struct ORACLEConnectionTimer_t
{
  pthread_cond_t condition;
  pthread_mutex_t mutex;
  volatile int done;
  volatile int running;
  volatile long seconds;
  volatile long elapsed;
  volatile long milliseconds;
  volatile long ocibreak;
  dvoid* ssnhp;
  OCIError* errhp;
  bool debug;
  size_t id;
};

struct ORACLEConnectionTimer : public ORACLEConnectionTimer_t
{
private:
  pthread_t thread = 0;

  static size_t nextid()
  {
    static size_t id(0);
    return __sync_fetch_and_add(&id, 1);
  }

  void initialize(long milliseconds, dvoid* ssnhp, OCIError* errhp);
  static void* OracleTimeoutRoutine(void* oracleTimer_t);

public:
  ORACLEConnectionTimer(double fractional, dvoid* ssnhp, OCIError* errhp);
  ORACLEConnectionTimer(long milliseconds, dvoid* ssnhp, OCIError* errhp);
  ~ORACLEConnectionTimer();

  void start();
  void stop();
  void start(long milliseconds);

  void finish(); // set state to done and notify worker thread.
};
}
