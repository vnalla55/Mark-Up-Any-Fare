#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include "DBAccess/RemoteCache/ASIOServer/IOServicePool.h"
#include "DBAccess/RemoteCache/ASIOServer/Connection.h"
#include "DBAccess/RemoteCache/ASIOServer/Session.h"

namespace tse
{

namespace RemoteCache
{

class ASIOServer : boost::noncopyable
{
public:
  ASIOServer();
  explicit ASIOServer(int threadPoolSize);

  static bool start();
  static void stop();
  void run();
private:
  void constructorBody();
  void startAccept();
  void handleAccept(const boost::system::error_code& e);
  void handleAcceptPersistent(SessionPtr session,
                              const boost::system::error_code& e);
  int _threadPoolSize;
  /// The pool of io_service objects used to perform asynchronous operations.
  IOServicePoolPtr _ioServicePool;
  IOServicePtr _ioServicePtr;
  boost::asio::ip::tcp::acceptor _acceptor;
  ConnectionPtr _newConnection;
  std::atomic<bool> _stopped{false};
  static boost::thread _serverThread;
};

} // RemoteCache

} // tse
