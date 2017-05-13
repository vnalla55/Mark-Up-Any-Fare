#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "DBAccess/RemoteCache/ASIOServer/Reply.h"
#include "DBAccess/RemoteCache/ASIOServer/Request.h"

namespace tse
{

namespace RemoteCache
{

typedef boost::shared_ptr<boost::asio::io_service> IOServicePtr;

// represents a single connection from a client
typedef boost::shared_ptr<class Connection> ConnectionPtr;

class Connection : public boost::enable_shared_from_this<Connection>, boost::noncopyable
{
public:

  explicit Connection(IOServicePtr ioServicePtr);

  ~Connection();

  boost::asio::ip::tcp::socket& socket();

  // start the first asynchronous operation for the connection
  void start();

  static int getNumberConnections();

private:

  // handle completion of a read operation
  void handleRead(const boost::system::error_code& e,
                  std::size_t bytesTransferred);

  void handleWrite(const boost::system::error_code& e,
                   std::size_t bytesTransferred);

  void onError(StatusType status,
               const std::string& text = "");

  void asyncWait();

  void handleTimer(const boost::system::error_code& e);

  IOServicePtr _ioServicePtr;

  boost::asio::ip::tcp::socket _socket;

  // buffer for incoming data
  boost::array<char, 8192> _buffer;

  Request _request;

  Reply _reply;

  size_t _responseSize{0};

  boost::asio::deadline_timer _timer;

};

}// RemoteCache

}// tse
