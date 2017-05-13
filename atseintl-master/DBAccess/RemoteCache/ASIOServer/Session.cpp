#include "DBAccess/RemoteCache/ASIOServer/Session.h"

#include "Common/Logger.h"
#include "DBAccess/RemoteCache/RCStartStop.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOServer/RequestHandler.h"

#include <boost/bind.hpp>

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace tse
{

static Logger logger("atseintl.DBAccess.RemoteCache.Session");

namespace RemoteCache
{

namespace
{

std::atomic<int> numberSessions(0);

bool serverReady()
{
  static std::atomic<bool> ready(false);
  if (ready)
  {
    return ready;
  }
  static std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  ready = ::difftime(std::time(nullptr), serverStartTime()) >= ReadConfig::getLdcDelay();
  return ready;
}

void logWarning(const char* where,
                StatusType status,
                const boost::system::error_code& err)
{
  if (boost::asio::error::eof == err || boost::asio::error::connection_reset == err)
  {
    LOG4CXX_INFO(logger, where << ' ' << err.message());
  }
  else if (err.message().empty())
  {
    LOG4CXX_WARN(logger, where << ' ' << statusToString(status));
  }
  else
  {
    LOG4CXX_WARN(logger, where << ' ' << err.message());
  }
}

}// namespace

Session::Session()
  : _socket(_ioService)
  , _timer(_ioService)
{
  ++numberSessions;
  LOG4CXX_DEBUG(logger, __FUNCTION__ << " numberSessions=" << numberSessions);
}

Session::~Session()
{
  try
  {
    if (!_ioService.stopped())
    {
      _ioService.stop();
    }
    if (_thread.joinable())
    {
      _thread.join();
    }
  }
  catch (...)
  {
    // ignore
  }
  --numberSessions;
}

int Session::getNumberSessions()
{
  return numberSessions;
}

void Session::handleStop()
{
  boost::system::error_code ignore;
  _timer.cancel(ignore);
  _socket.cancel(ignore);
  if (!_ioService.stopped())
  {
    _ioService.stop();
  }
}

void Session::stop()
{
  _stopped = true;
  _ioService.dispatch(boost::bind(&Session::handleStop, this));
}

bool Session::stopped() const
{
  return _ioService.stopped();
}

void Session::run()
{
  try
  {
    boost::system::error_code e;
    _ioService.run(e);
    if (boost::system::errc::success != e.value())
    {
      LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << e.message());
    }
  }
  catch (const boost::system::system_error& err)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << ' ' << err.what());
  }
  catch (const std::exception& ex)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << ' ' << ex.what());
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << " UNKNOWN)");
  }
  _ioService.stop();
}

void Session::start()
{
  boost::system::error_code error;
  boost::asio::ip::tcp::no_delay option(true);
  _socket.set_option(option, error);
  if (error)
  {
    LOG4CXX_WARN(logger, " RC:" << error.message());
  }
  int threadPoolSize(ReadConfig::getThreadPoolSize());
  if (!serverReady())
  {
    _status = RC_SERVER_NOT_READY;
  }
  else if (numberSessions > threadPoolSize)
  {
    _status = RC_SERVER_BUSY;
  }
  _ioService.post(boost::bind(&Session::processRequest, this));
  std::thread tmp(boost::bind(&Session::run, this));
  _thread.swap(tmp);
}

void Session::handleIdleTimer(const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted && !_stopped)
  {
    LOG4CXX_INFO(logger, __FUNCTION__);
  }
}

void Session::asyncIdleWait()
{
  unsigned idleTimeout(ReadConfig::getIdleMasterTimeout());
  if (idleTimeout > 0)
  {
    boost::system::error_code ignore;
    _timer.expires_from_now(boost::posix_time::seconds(idleTimeout), ignore);
    _timer.async_wait(boost::bind(&Session::handleIdleTimer, this, boost::asio::placeholders::error));
  }
}

void Session::processRequest()
{
  boost::system::error_code ignore;
  _timer.expires_from_now(boost::posix_time::seconds(0), ignore);
  asyncIdleWait();
  RequestPtr request(new Request(-1));
  ReplyPtr reply(new Reply(-1));
  request->_headerVect.resize(_headerSz);
  boost::asio::async_read(
    _socket,
    boost::asio::buffer(request->_headerVect, _headerSz),
    boost::bind(&Session::handleReadHeader,
                this,
                request,
                reply,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void Session::onError(ReplyPtr reply,
                      StatusType status)
{
  // process first error
  StatusType previousStatus(reply->_header._status);
  bool previousStateOk(RC_NONE == previousStatus
                       || RC_COMPRESSED_VALUE == previousStatus
                       || RC_UNCOMPRESSED_VALUE == previousStatus);
  if (previousStateOk)
  {
    reply->stockReply(status);
    reply->convertToBuffers();
    boost::asio::async_write(_socket,
                             reply->_buffers,
                             boost::bind(&Session::handleWrite,
                                         this,
                                         reply,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    LOG4CXX_WARN(logger, statusToString(status));
  }
}

void Session::handleReadHeader(RequestPtr request,
                               ReplyPtr reply,
                               const boost::system::error_code& err,
                               std::size_t bytesTransferred)
{
  boost::system::error_code ignore;
  size_t numberCanceled(_timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err && numberCanceled > 0 && _headerSz == bytesTransferred)
  {
    assert(_headerSz == bytesTransferred);
    if (readHeader(request->_header, request->_headerVect)
        && request->_header._requestId != static_cast<uint64_t>(-1))
    {
      reply->_header._requestId = request->_header._requestId;
      request->_payload.resize(request->_header._payloadSize);
    }
    else
    {
      LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << statusToString(RC_BAD_REQUEST));
      return;
    }
    if (request->_header._status != RC_NONE)
    {
      LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << statusToString(request->_header._status));
      return;
    }
    request->_payload.resize(request->_header._payloadSize);
    boost::asio::async_read(
      _socket,
      boost::asio::buffer(request->_payload, request->_payload.size()),
      boost::bind(&Session::handleReadPayload,
                  this,
                  request,
                  reply,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
    unsigned receiveTimeout(ReadConfig::getServerReceiveTimeout());
    if (receiveTimeout > 0)
    {
      boost::system::error_code ignore;
      _timer.expires_from_now(boost::posix_time::seconds(receiveTimeout), ignore);
      asyncReceiveWait(reply);
    }
  }
  else
  {
    logWarning(__FUNCTION__, RC_READ_ERROR, err);
  }
}

void Session::handleReadPayload(RequestPtr request,
                                ReplyPtr reply,
                                const boost::system::error_code& err,
                                std::size_t bytesTransferred)
{
  boost::system::error_code ignore;
  size_t numberCanceled(_timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err && numberCanceled > 0 && request->_payload.size() == bytesTransferred)
  {
    assert(request->_payload.size() == bytesTransferred);
    if (_status != RC_NONE)
    {
      onError(reply, _status);
      _status = RC_NONE;
    }
    else
    {
      boost::system::error_code ignore;
      _timer.expires_at(boost::posix_time::pos_infin, ignore);
      RequestHandler::handleRequest(_socket, *request, *reply);
      reply->convertToBuffers();
      unsigned sendTimeout(ReadConfig::getServerSendTimeout());
      if (sendTimeout > 0)
      {
        _timer.expires_from_now(boost::posix_time::seconds(sendTimeout), ignore);
        asyncSendWait(reply);
      }
      boost::asio::async_write(_socket,
                               reply->_buffers,
                               boost::bind(&Session::handleWrite,
                                           this,
                                           reply,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
  }
  else
  {
    logWarning(__FUNCTION__, RC_READ_ERROR, err);
  }
}

void Session::asyncSendWait(ReplyPtr reply)
{
  _timer.async_wait(boost::bind(&Session::handleSendTimer,
                                this,
                                reply,
                                boost::asio::placeholders::error));
}

void Session::asyncReceiveWait(ReplyPtr reply)
{
  _timer.async_wait(boost::bind(&Session::handleReceiveTimer,
                                this,
                                reply,
                                boost::asio::placeholders::error));
}

void Session::handleReceiveTimer(ReplyPtr reply,
                                 const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted && !_stopped)
  {
    uint64_t requestId(0);
    if (reply)
    {
      requestId = reply->_header._requestId;
    }
    LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << statusToString(RC_SERVER_TIMEOUT)
                 << ",requestId=" << requestId);
  }
}

void Session::handleSendTimer(ReplyPtr reply,
                              const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted && !_stopped)
  {
    onError(reply, RC_SERVER_TIMEOUT);
  }
}

void Session::handleWrite(ReplyPtr reply,
                          const boost::system::error_code& err,
                          std::size_t bytesTransferred)
{
  boost::system::error_code ignore;
  bool timeout(0 == _timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err && !timeout && _headerSz + reply->_payloadSize == bytesTransferred)
  {
    switch (reply->_header._status)
    {
    case RC_COMPRESSED_VALUE:
    case RC_UNCOMPRESSED_VALUE:
      if (!_stopped)
      {
        _ioService.post(boost::bind(&Session::processRequest, this));
      }
      break;
    default:
      break;
    }
  }
  else
  {
    LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << err.message());
  }
}

}// RemoteCache

}// tse
