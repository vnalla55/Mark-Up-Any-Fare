#ifndef THREAD_TEST_UTIL_H
#define THREAD_TEST_UTIL_H

#include "ZThreads/zthread/Runnable.h"
#include "ZThreads/zthread/Task.h"
#include "ZThreads/zthread/Exceptions.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

#include <boost/atomic.hpp>

namespace tse
{

inline
unsigned long fib(unsigned long n)
{
  if (n == 1 || n == 2)
  {
    return 1;
  }
  else
  {
    return fib(n - 2) + fib(n - 1);
  }
}

template<typename ValueT>
struct ThreadValue
{
  ThreadValue()
    : _isSet(false)
  {
  }

  ValueT get()
  {
    boost::mutex::scoped_lock lock(_mutex);
    while (!_isSet)
    {
      _condition.wait(lock);
    }

    return _value;
  }

  void set(const ValueT& value)
  {
    boost::mutex::scoped_lock lock(_mutex);
    _value = value;
    _isSet = true;
    _condition.notify_one();
  }

private:
  boost::mutex _mutex;
  boost::condition_variable _condition;
  ValueT _value;
  bool _isSet;
};

struct FibTask:
  public ZThread::Runnable
{
  FibTask(unsigned long n, unsigned long& value)
    : _n(n), _value(value)
  {
  }

  virtual void run()
  {
    _value = fib(_n);
  }

private:
  unsigned long _n;
  unsigned long& _value;
};

struct FibTask2:
  public ZThread::Runnable
{
  FibTask2(unsigned long n, ThreadValue<unsigned long>& value)
    : _n(n), _value(value)
  {
  }

  virtual void run()
  {
    _value.set(fib(_n));
  }

private:
  unsigned long _n;
  ThreadValue<unsigned long>& _value;
};

struct WaitTask:
  public ZThread::Runnable
{
  explicit
  WaitTask(ThreadValue<int>& value)
    : _value(value)
  {
  }

  virtual void run()
  {
    (void)_value.get();
  }

private:
  ThreadValue<int>& _value;
};

struct InterruptTask:
  public ZThread::Runnable
{
  InterruptTask(ThreadValue<int>& start,
                ThreadValue<int>& value,
                bool& interrupted)
    : _start(start), _value(value), _interrupted(interrupted)
  {
  }

  virtual void run()
  {
    try
    {
      _start.set(1);
      (void)_value.get();
    }
    catch (ZThread::Interrupted_Exception&)
    {
      _interrupted = true;
    }
  }

private:
  ThreadValue<int>& _start;
  ThreadValue<int>& _value;
  bool& _interrupted;
};

struct IncrementTask:
  public ZThread::Runnable
{
  IncrementTask(boost::atomic<int>& counter)
    : _counter(counter)
  {
  }

  virtual void run()
  {
    _counter.fetch_add(1, boost::memory_order_release);
  }

private:
  boost::atomic<int>& _counter;
};

} // namespace tse

#endif
