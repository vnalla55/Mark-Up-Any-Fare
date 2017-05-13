#include "DBAccess/RemoteCache/ASIOServer/Connection.h"

#include "Common/Logger.h"
#include "DBAccess/RemoteCache/RCStartStop.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOServer/RequestHandler.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/logic/tribool.hpp>

#include <atomic>
#include <vector>

namespace tse
{

static Logger logger("atseintl.DBAccess.RemoteCache.Connection");

namespace RemoteCache
{

namespace
{

// Parse some data. The tribool return value is true when a complete request
// has been parsed, false if the data is invalid, indeterminate when more
// data is required. The InputIterator return value indicates how much of the
// input has been consumed

boost::tribool parseRequest(Request& req,
                            Reply& rep,
                            const char* begin,
                            const char* end)
{
  size_t size(end - begin);
  if (req._headerVect.size() < _headerSz)
  {
    size_t need(_headerSz - req._headerVect.size());
    size_t copied(std::min(size, need));
    req._headerVect.insert(req._headerVect.end(), begin, begin + copied);
    if (copied <= size)
    {
      if (!readHeader(req._header, req._headerVect))
      {
        return false;
      }
      rep._header._requestId = req._header._requestId;
      size_t remains(size - copied);
      if (remains > req._header._payloadSize)
      {
        return false;
      }
      req._payload.reserve(req._header._payloadSize);
      req._payload.insert(req._payload.end(), begin + copied, end);
      if (req._payload.size() == req._header._payloadSize)
      {
        return true;
      }
    }
  }
  else
  {
    size_t need(req._header._payloadSize - req._payload.size());
    if (size > need)
    {
      return false;
    }
    req._payload.insert(req._payload.end(), begin, end);
    if (size == need)
    {
      return true;
    }
  }
  return boost::indeterminate;
}

std::atomic<int> numberConnections(0);

bool serverReady()
{
  static std::atomic<bool> ready(false);
  if (ready)
  {
    return ready;
  }
  static boost::mutex mutex;
  boost::mutex::scoped_lock lock(mutex);
  ready = ::difftime(std::time(0), serverStartTime()) > ReadConfig::getLdcDelay();
  return ready;
}

}// namespace

Connection::Connection(IOServicePtr ioServicePtr)
  : _ioServicePtr(ioServicePtr)
  , _socket(*ioServicePtr)
  , _request(-1)
  , _reply(-1)
  , _timer(*ioServicePtr)
{
  ++numberConnections;
  boost::system::error_code ignore;
  _timer.expires_at(boost::posix_time::pos_infin, ignore);
  _request._headerVect.reserve(_headerSz);
}

Connection::~Connection()
{
  LOG4CXX_DEBUG(logger, __FUNCTION__);
  --numberConnections;
}

int Connection::getNumberConnections()
{
  return numberConnections;
}

boost::asio::ip::tcp::socket& Connection::socket()
{
  return _socket;
}

void Connection::start()
{
  int threadPoolSize(ReadConfig::getThreadPoolSize());
  LOG4CXX_INFO(logger, __FUNCTION__ << " numberConnections=" << numberConnections);
  if (!serverReady())
  {
    onError(RC_SERVER_NOT_READY);
  }
  else if (numberConnections >= threadPoolSize)
  {
    onError(RC_SERVER_BUSY);
  }
  else
  {
    int receiveTimeout(ReadConfig::getServerReceiveTimeout());
    if (receiveTimeout > 0)
    {
      boost::system::error_code ignore;
      _timer.expires_from_now(boost::posix_time::seconds(receiveTimeout), ignore);
      asyncWait();
    }
    boost::system::error_code error;
    _socket.set_option(boost::asio::ip::tcp::no_delay(), error);
    if (error)
    {
      LOG4CXX_WARN(logger, " RC:" << error.message());
    }
    _socket.async_read_some(
      boost::asio::buffer(_buffer),
      boost::bind(&Connection::handleRead,
                  shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }
}

void Connection::onError(StatusType status,
                         const std::string& text)
{
  // process first error
  StatusType previousStatus(_reply._header._status);
  bool previousStateOk(RC_NONE == previousStatus
                       || RC_COMPRESSED_VALUE == previousStatus
                       || RC_UNCOMPRESSED_VALUE == previousStatus);
  if (previousStateOk)
  {
    _reply.stockReply(status);
    _responseSize = _reply.toBuffers();
    boost::asio::async_write(_socket,
                             _reply._buffers,
                             boost::bind(&Connection::handleWrite,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    LOG4CXX_WARN(logger, ' ' << statusToString(status) << ':' << text);
  }
}

void Connection::handleRead(const boost::system::error_code& e,
                            std::size_t bytesTransferred)
{
  if (!e)
  {
    boost::tribool result(parseRequest(_request,
                                       _reply,
                                       _buffer.data(),
                                       _buffer.data() + bytesTransferred));
    if (result)
    {
      if (_request._header._status != RC_NONE)
      {
        // bad request
        onError(_request._header._status);
      }
      boost::system::error_code ignore;
      _timer.expires_at(boost::posix_time::pos_infin, ignore);
      RequestHandler::handleRequest(_socket, _request, _reply);
      _responseSize = _reply.toBuffers();
      int sendTimeout(ReadConfig::getServerSendTimeout());
      if (sendTimeout > 0)
      {
        _timer.expires_from_now(boost::posix_time::seconds(sendTimeout), ignore);
        asyncWait();
      }
      boost::asio::async_write(_socket,
                               _reply._buffers,
                               boost::bind(&Connection::handleWrite,
                                           shared_from_this(),
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
    else if (!result)
    {
      // bad request
      onError(_request._header._status);
    }
    else// indeterminate
    {
      _socket.async_read_some(
        boost::asio::buffer(_buffer),
        boost::bind(&Connection::handleRead,
                    shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
  }
  else
  {
    onError(RC_READ_ERROR, e.message());
  }
}

void Connection::asyncWait()
{
  _timer.async_wait(boost::bind(&Connection::handleTimer,
                                shared_from_this(),
                                boost::asio::placeholders::error));
}

void Connection::handleTimer(const boost::system::error_code& e)
{
  if (e != boost::asio::error::operation_aborted
      && _timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
  {
    onError(RC_SERVER_TIMEOUT);
  }
}

void Connection::handleWrite(const boost::system::error_code& e,
                             std::size_t bytesTransferred)
{
  if (e)
  {
    onError(RC_WRITE_ERROR, e.message());
  }
  else
  {
    if (_responseSize != bytesTransferred)
    {
      LOG4CXX_ERROR(logger, __FUNCTION__ << " _responseSize=" << _responseSize
                    << ",bytesTransferred=" << bytesTransferred);
    }
    assert(_responseSize == bytesTransferred);
    if (_socket.is_open())
    {
      boost::system::error_code ignore;
      _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    }
  }
  boost::system::error_code ignore;
  _timer.cancel(ignore);
  // no new asynchronous operations are started. This means that all shared_ptr
  // references to the Connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The Connection class's
  // destructor closes the socket
}

}// RemoteCache

}// tse
