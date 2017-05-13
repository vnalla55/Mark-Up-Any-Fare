#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace tse
{

namespace RemoteCache
{

typedef boost::shared_ptr<boost::asio::io_service> IOServicePtr;
typedef boost::shared_ptr<class IOServicePool> IOServicePoolPtr;

// A pool of io_service objects.
class IOServicePool : private boost::noncopyable
{
public:
  // Construct the io_service pool.
  explicit IOServicePool(std::size_t poolSize);

  // Run all io_service objects in the pool.
  void run();

  // Stop all io_service objects in the pool.
  void stop();

  // Get an io_service to use.
  IOServicePtr getIOService();

  IOServicePtr addIOService();

private:

  // wrapper for io_service run()
  void run(std::size_t i);

  typedef boost::shared_ptr<boost::asio::io_service::work> WorkPtr;

  // The pool of io_services.
  std::vector<IOServicePtr> _ioServices;

  // The work that keeps the io_services running.
  std::vector<WorkPtr> _work;

  const std::size_t _maxPoolSize;
  std::vector<boost::shared_ptr<boost::thread> > _threads;

  // The next io_service to use for a connection.
  std::size_t _nextIOService{};
};

}// RemoteCache

}// tse
