#include "ZThreads/zthread/ThreadedExecutor.h"
#include "ZThreads/zthread/Exceptions.h"

#include <boost/thread/thread.hpp>


#include <boost/bind.hpp>

#include <cassert>
#include <memory>
#include <map>
#include <utility>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ZThread
{

class ThreadedExecutor::ThreadStorage
{
  typedef std::map<boost::thread::id, std::shared_ptr<boost::thread>> Threads;

public:
  ThreadStorage()
    : _detached(false)
  {
  }

  template<class FunctionT>
  void createThread(const FunctionT& function)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    std::shared_ptr<boost::thread> thread = std::make_shared<boost::thread>(function);
    const boost::thread::id id = thread->get_id();
    _threads.insert(std::make_pair(id, thread));
  }

  void threadDone()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_detached)
    {
      return;
    }

    auto threadIt = _threads.find(boost::this_thread::get_id());
    assert(threadIt != _threads.end());
    if (threadIt != _threads.end())
    {
      _threads.erase(threadIt);
    }

    _condition.notify_all();
  }

  void interrupt()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    for (auto threadIt : _threads)
    {
      boost::thread& thread = *(threadIt.second);
      thread.interrupt();
    }
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_threads.empty())
    {
      _condition.wait(lock);
    }
  }

  bool wait(unsigned long timeout)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_threads.empty())
    {
      return true;
    }

    const std::chrono::system_clock::time_point absoluteTime =
      std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);
    while (!_threads.empty())
    {
      if (_condition.wait_until(lock, absoluteTime) == std::cv_status::timeout)
      {
        return false;
      }
    }

    return true;
  }

  void detach()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _threads.clear();
    _detached = true;
  }

private:
  ThreadStorage(const ThreadStorage&);
  void operator=(const ThreadStorage&);

private:
  std::mutex _mutex;
  std::condition_variable _condition;
  Threads _threads;
  bool _detached;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadedExecutor::~ThreadedExecutor()
{
  _storage->detach();
}

ThreadedExecutor::ThreadedExecutor() : _storage(std::make_shared<ThreadStorage>()), _cancel(false)
{
}

void ThreadedExecutor::interrupt()
{
  _storage->interrupt();
}

void ThreadedExecutor::execute(const Task& task)
{
  _storage->createThread(boost::bind(&ThreadedExecutor::threadFunction, task, _storage));
}

void ThreadedExecutor::cancel()
{
  _cancel.store(true, boost::memory_order_relaxed);
}

bool ThreadedExecutor::isCanceled()
{
  return _cancel.load(boost::memory_order_relaxed);
}

void ThreadedExecutor::wait()
{
  _storage->wait();
}

bool ThreadedExecutor::wait(unsigned long timeout)
{
  return _storage->wait(timeout);
}

void
ThreadedExecutor::threadFunction(Task task, std::shared_ptr<ThreadStorage> storage)
{
  boost::this_thread::disable_interruption di;
  try
  {
    boost::this_thread::restore_interruption ri(di);
    task->run();
  }
  catch (Interrupted_Exception&)
  {
  }
  catch (...)
  {
    assert(false && "Unhandled exception in thread");
  }

  try
  {
    storage->threadDone();
  }
  catch (...)
  {
    assert(false && "Unhandled exception in thread");
  }
}

} // namespace ZThread
