#include "DBAccess/ORACLEConnectionTimer.h"

#include "Common/Logger.h"
#include "DBAccess/CacheManager.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <oci.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <tr1/type_traits>
#include <unistd.h>

#define TRUE_FALSE_TEXT(x) ((x) ? "true " : "false")
// #define FULL_MEMORY_BARRIER __asm__ __volatile__ ("" : : : "memory")
#define FULL_MEMORY_BARRIER __sync_synchronize()

namespace
{
log4cxx::LoggerPtr&
getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEConnectionTimer"));
  return logger;
}
const bool logDebugEnabled = getLogger()->isEnabledFor(log4cxx::Level::getDebug());
const long billion = 1000000000;
const long million = 1000000;
const long thousand = 1000;
const long hundred = 100;
const long ten = 10;
const long one = 1;
}

namespace tse
{

// fractional seconds are (whole seconds).(fraction of seconds)
ORACLEConnectionTimer::ORACLEConnectionTimer(double fractional, dvoid* ssnhp, OCIError* errhp)
{
  initialize((long)(fractional * 1000), ssnhp, errhp);
}

ORACLEConnectionTimer::ORACLEConnectionTimer(long milliseconds, dvoid* ssnhp, OCIError* errhp)
{
  initialize(milliseconds, ssnhp, errhp);
}

void
ORACLEConnectionTimer::initialize(long milliseconds, dvoid* ssnhp, OCIError* errhp)
{
  if (pthread_mutex_init(&this->mutex, nullptr) != 0)
  {
    LOG4CXX_ERROR(getLogger(), "Error initializing mutex [" << strerror(errno) << "]");
    throw std::runtime_error(
        "ORACLEConnectionTimer::ORACLEConnectionTimer() mutex construction failed");
  }
  if (pthread_cond_init(&this->condition, nullptr))
  {
    LOG4CXX_ERROR(getLogger(), "Error initializing condition variable [" << strerror(errno) << "]");
    throw std::runtime_error(
        "ORACLEConnectionTimer::ORACLEConnectionTimer() condition construction failed");
  }
  FULL_MEMORY_BARRIER;
  pthread_mutex_lock(&this->mutex);

  this->elapsed = 0;
  this->seconds = 0;
  this->milliseconds = milliseconds;
  this->done = false;
  this->running = false;
  this->ocibreak = false;
  this->ssnhp = ssnhp;
  this->errhp = errhp;
  this->debug = logDebugEnabled;
  this->id = ORACLEConnectionTimer::nextid();
  ORACLEConnectionTimer_t* timer = this;
  void* data = timer;
  if (pthread_create(&thread, nullptr, OracleTimeoutRoutine, data))
  {
    LOG4CXX_ERROR(
        getLogger(),
        "ORACLEConnectionTimer::ORACLEConnectionTimer construction failed to create thread ["
            << strerror(errno) << "]");
    throw std::runtime_error("ORACLEConnectionTimer::ORACLEConnectionTimer() failed");
  }
  LOG4CXX_DEBUG(getLogger(), "object #[" << this->id << "] construction");
}

void
ORACLEConnectionTimer::stop()
{
  if (CacheManager::initialized())
  {
    LOG4CXX_DEBUG(getLogger(), "object #[" << this->id << "] cancel timer");
    pthread_mutex_lock(&this->mutex);
    this->running = false;
    pthread_cond_signal(&this->condition);
    pthread_mutex_unlock(&this->mutex);
  }
}

void
ORACLEConnectionTimer::start(long milliseconds)
{
  if (CacheManager::initialized())
  {
    this->milliseconds = milliseconds;
    start();
  }
}

void
ORACLEConnectionTimer::start()
{
  if (CacheManager::initialized())
  {
    LOG4CXX_DEBUG(getLogger(), "object #[" << this->id << "] start timer");
    pthread_mutex_lock(&this->mutex);
    this->ocibreak = false;
    this->running = true;
    FULL_MEMORY_BARRIER;
    pthread_cond_signal(&this->condition);
    pthread_mutex_unlock(&this->mutex);
  }
}

ORACLEConnectionTimer::~ORACLEConnectionTimer()
{
  LOG4CXX_DEBUG(getLogger(), "object #[" << this->id << "] destruction");

  try { finish(); }
  catch (const std::exception& e)
  {
    LOG4CXX_ERROR(getLogger(), __PRETTY_FUNCTION__ << ":" << e.what());
  }
  catch (...) { LOG4CXX_ERROR(getLogger(), __PRETTY_FUNCTION__ << ":Unkown exception"); }
}

void
ORACLEConnectionTimer::finish()
{
  if (pthread_mutex_lock(&mutex) == 0)
  {
    this->done = true;
    this->running = false;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
  }
  pthread_join(thread, nullptr);
  // Test for a sequence error as a result of an aborted unlock.
  int rc = pthread_mutex_trylock(&mutex);
  if (rc == EBUSY || rc == 0)
    pthread_mutex_unlock(&mutex);
  pthread_mutex_destroy(&mutex);
}

namespace
{
extern "C" void
logString(ORACLEConnectionTimer_t* timer)
{
  char text[4096];
  sprintf(text,
          "object #[%ld] OracleTimeoutRoutine waited[%6ld] ms query canceled[%s] at[%p]",
          timer->id,
          timer->elapsed,
          TRUE_FALSE_TEXT(timer->ocibreak),
          timer);
  LOG4CXX_DEBUG(getLogger(), text);
}
}

void*
ORACLEConnectionTimer::OracleTimeoutRoutine(void* oracleTimer_t)
{

  ORACLEConnectionTimer_t* timer = (ORACLEConnectionTimer_t*)oracleTimer_t;
  // Timeout of milliseconds.
  long timeout = timer->seconds * thousand + timer->milliseconds;
  LOG4CXX_DEBUG(getLogger(), "object #[" << timer->id << "] thread started");

  while (!timer->done)
  {
    FULL_MEMORY_BARRIER;
    if (!timer->running)
    {
      timespec ts = { 0, 0 };
      int seconds = 1;
      int milliseconds = 0;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_nsec += (seconds * billion + milliseconds * million);
      ts.tv_sec += ts.tv_nsec / billion;
      ts.tv_nsec = ts.tv_nsec % billion;
      pthread_cond_timedwait(&timer->condition, &timer->mutex, &ts);
    }

    FULL_MEMORY_BARRIER;
    if (timer->running)
    {
      int rc = 0;
      timespec ts = { 0, 0 };
      clock_gettime(CLOCK_REALTIME, &ts);

      timespec start = { 0, 0 };
      timespec now = { 0, 0 };
      now = start = ts;

      ts.tv_nsec += (timer->seconds * billion + timer->milliseconds * million);
      ts.tv_sec += ts.tv_nsec / billion;
      ts.tv_nsec = ts.tv_nsec % billion;

      FULL_MEMORY_BARRIER;
      timer->elapsed = 0;
      bool done = false;

      while (!done)
      {
        FULL_MEMORY_BARRIER;
        if (timer->done)
        {
          break;
        }
        rc = pthread_cond_timedwait(&timer->condition, &timer->mutex, &ts);
        clock_gettime(CLOCK_REALTIME, &now);
        long nanoseconds =
            (now.tv_sec * billion + now.tv_nsec) - (start.tv_sec * billion + start.tv_nsec);
        timer->elapsed = nanoseconds / million;

        FULL_MEMORY_BARRIER;
        done = not timer->running || rc == ETIMEDOUT || timer->elapsed > timeout || timer->done;
        if (timer->elapsed >= timeout)
        {
          if (timer->running)
          {
            timer->ocibreak = true;
            if (timer->debug)
            {
              logString(timer);
            }
            OCIBreak(timer->ssnhp, timer->errhp);
          }
        }
      }
    } // is running
  }
  pthread_cond_destroy(&timer->condition);
  pthread_mutex_unlock(&timer->mutex);
  pthread_exit(nullptr);
  return nullptr;
}
}
