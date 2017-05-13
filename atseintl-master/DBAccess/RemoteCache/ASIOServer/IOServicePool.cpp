#include "IOServicePool.h"
#include <boost/bind.hpp>
#include "Common/Logger.h"

namespace tse
{

static Logger logger("atseintl.DBAccess.RemoteCache.IOServicePool");

namespace RemoteCache
{

namespace
{

boost::mutex mutex;
boost::condition_variable condition;
bool stopped(false);

}

IOServicePool::IOServicePool(std::size_t maxPoolSize)
  : _maxPoolSize(maxPoolSize)
{
  if (0 == maxPoolSize)
  {
    LOG4CXX_FATAL(logger, __FUNCTION__ << " #### maxPoolSize must be > 0");
  }
}

void IOServicePool::run(std::size_t i)
{
  for (;;)
  {
    try
    {
      boost::system::error_code e;
      _ioServices[i]->run(e);
      if (boost::system::errc::success != e.value())
      {
        LOG4CXX_WARN(logger, __FUNCTION__ << " #### " << e.message());
      }
      break;
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger, __FUNCTION__ << " #### UNKNOWN");
      break;
    }
  }
}

void IOServicePool::run()
{
  {
    boost::mutex::scoped_lock lock(mutex);
    while (!stopped)
    {
      condition.wait(lock);
    }
  }
  // wait for all threads in the pool to exit
  for (auto& thread : _threads)
  {
    if (thread && thread->joinable())
    {
      try
      {
        thread->join();
      }
      catch(...)
      {
        // ignore
      }
    }
  }
}

void IOServicePool::stop()
{
  // explicitly stop all io_services
  for (auto& service : _ioServices)
  {
    service->stop();
  }
  boost::mutex::scoped_lock lock(mutex);
  stopped = true;
  condition.notify_one();
}

IOServicePtr IOServicePool::addIOService()
{
  IOServicePtr newService;
  // +10 for throttling, +1 for accept
  if (_ioServices.size() < _maxPoolSize + 10 + 1)
  {
    // Give the io_service work to do so that its run() function will not
    // exit until it is explicitly stopped.
    newService.reset(new boost::asio::io_service);
    WorkPtr workPtr(new boost::asio::io_service::work(*newService));
    _ioServices.push_back(newService);
    _work.push_back(workPtr);
    boost::shared_ptr<boost::thread> thread(new boost::thread(
      boost::bind(&IOServicePool::run, this, _threads.size())));
    _threads.push_back(thread);
  }
  return newService;
}

IOServicePtr IOServicePool::getIOService()
{
  // find idle io_service, keep the first one for accept
  for (std::size_t i = 1; i < _ioServices.size(); ++i)
  {
    if (_ioServices[i].unique())
    {
      return _ioServices[i];
    }
  }
  IOServicePtr newService(addIOService());
  if (newService)
  {
    return newService;
  }
  LOG4CXX_WARN(logger, __FUNCTION__ << " #### idle io_service not found,returning next");
  // round-robin scheme to choose the next io_service to use
  if (_ioServices.size() > 1)
  {
    _nextIOService = _nextIOService % _ioServices.size();
    if (0 == _nextIOService)
    {
      _nextIOService = 1;
    }
    return _ioServices[_nextIOService++];
  }
  return IOServicePtr();
}

}// RemoteCache

}// tse
