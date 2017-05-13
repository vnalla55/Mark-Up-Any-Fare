#pragma once

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/version.hpp>

#include <queue>

namespace tse
{
template <typename T>
class concurrent_queue
{
private:
  std::queue<T> _queue;
  mutable boost::mutex _mutex;
#if BOOST_VERSION <= 103400
  boost::condition _condition;
#else
  boost::condition_variable _condition;
#endif // BOOST_VERSION
  // not implemented
  concurrent_queue(const concurrent_queue&);
  concurrent_queue& operator=(const concurrent_queue&);

public:
  concurrent_queue() {}
  ~concurrent_queue() {}

  void push(const T& data)
  {
    boost::mutex::scoped_lock lock(_mutex);
    _queue.push(data);
    lock.unlock();
    _condition.notify_one();
  }
  bool empty() const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _queue.empty();
  }
  bool try_pop(T& value)
  {
    boost::mutex::scoped_lock lock(_mutex);
    if (_queue.empty())
    {
      return false;
    }
    value = _queue.front();
    _queue.pop();
    return true;
  }
  void wait_and_pop(T& value)
  {
    boost::mutex::scoped_lock lock(_mutex);
    while (_queue.empty())
    {
      _condition.wait(lock);
    }
    value = _queue.front();
    _queue.pop();
  }
  size_t size() const
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _queue.size();
  }
};
} // tse
