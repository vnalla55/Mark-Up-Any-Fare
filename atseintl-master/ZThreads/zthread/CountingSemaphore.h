#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace ZThread
{

class CountingSemaphore
{
public:
  explicit
  CountingSemaphore(int initialCount = 0)
    : _counter(initialCount)
  {
  }

  void wait()
  {
    acquire();
  }

  bool tryWait(unsigned long timeout)
  {
    return tryAcquire(timeout);
  }

  void post()
  {
    release();
  }

  int count()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    return _counter;
  }

  bool tryAcquire(unsigned long timeout)
  {
    std::unique_lock<std::mutex> lock(_mutex);

    if (_counter <= 0)
    {
      const std::chrono::system_clock::time_point absoluteTime =
        std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);
      while (_counter <= 0)
      {
        if (_condition.wait_until(lock, absoluteTime) == std::cv_status::timeout)
        {
          return false;
        }
      }
    }

    --_counter;
    return true;
  }

  void acquire()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_counter <= 0)
    {
      _condition.wait(lock);
    }

    --_counter;
  }

  void release()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    ++_counter;

    if (_counter > 0)
    {
      _condition.notify_one();
    }
  }

private:
  CountingSemaphore(const CountingSemaphore&);
  void operator=(const CountingSemaphore&);

private:
  std::mutex _mutex;
  std::condition_variable _condition;
  int _counter;
};

} // namespace ZThread

