#include "DBAccess/RemoteCache/ASIOClient/Client.h"

#include "Common/Logger.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientPool.h"
#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"

#include <boost/bind.hpp>

#include <iostream>

namespace tse
{

namespace RemoteCache
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.Client");

std::atomic<int> _numberClients(0);

StatusType translateError(const boost::system::error_code& err)
{
  switch (err.value())
  {
  case boost::asio::error::connection_refused:
  case boost::asio::error::host_unreachable:
    return RC_CONNECTION_REFUSED;
    break;
  default:
    return RC_CLIENT_ERROR;
    break;
  }
  return RC_NONE;
}

}// namespace

Client::Client(ClientPool* pool)
  : _pool(pool)
  , _socket(_ioService)
  , _timer(_ioService)
  , _thread(boost::bind(&Client::run, this))
{
  ++_numberClients;
}

Client::~Client()
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
  --_numberClients;
}

void Client::handleStop()
{
  boost::system::error_code ignore;
  _timer.cancel(ignore);
  _socket.cancel(ignore);
  if (!_ioService.stopped())
  {
    _ioService.stop();
  }
}

void Client::cancel()
{
  _stop = true;
  _ioService.dispatch(boost::bind(&Client::handleStop, this));
}

void Client::stop()
{
  _stop = true;
}

void Client::connect()
{
  if (_pool)
  {
    boost::system::error_code ignore;
    unsigned long connectTimeout(ReadConfig::getClientConnectionTimeout());
    _timer.expires_from_now(boost::posix_time::seconds(connectTimeout), ignore);
    asyncConnectWait();
    boost::asio::ip::tcp::resolver resolver(_ioService);
    boost::asio::ip::tcp::resolver::query query(_pool->getHost(), _pool->getPort());
    boost::asio::ip::tcp::resolver::iterator iterator(resolver.resolve(query));
    boost::asio::async_connect(_socket,
                               iterator,
                               boost::bind(&Client::handleConnect,
                                           this,
                                           boost::asio::placeholders::error));
  }
}

void Client::run()
{
  try
  {
    _ioService.post(boost::bind(&Client::connect, this));
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

int Client::getNumberClients()
{
  return _numberClients;
}

bool Client::stopped() const
{
  return _ioService.stopped();
}

void Client::processRequest()
{
  RCRequestPtr request(_pool->getRequest());
  boost::system::error_code ignore;
  _timer.expires_from_now(boost::posix_time::seconds(0), ignore);
  if (request)
  {
    if (RC_NONE == request->_responseHeader._status)
    {
      unsigned long sendTimeout(ReadConfig::getClientSendTimeout());
      _timer.expires_from_now(boost::posix_time::milliseconds(sendTimeout), ignore);
      asyncProcessingWait(request);
      std::vector<boost::asio::const_buffer> sendBuffers;
      request->convertToBuffers(sendBuffers);
      boost::asio::async_write(_socket,
                               sendBuffers,
                               boost::bind(&Client::handleWriteRequest,
                                           this,
                                           request,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      request->notify();
    }
  }
}

void Client::handleConnect(const boost::system::error_code& err)
{
  boost::system::error_code error;
  bool timeout(0 == _timer.expires_from_now(boost::posix_time::seconds(0), error));
  if (!err && !timeout)
  {
    boost::asio::ip::tcp::no_delay option(true);
    _socket.set_option(option, error);
    if (error)
    {
      LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << err.message());
    }
    bool linger(ReadConfig::isLinger());
    if (linger)
    {
      unsigned lingerTime(ReadConfig::getLingerTime());
      boost::asio::socket_base::linger option(true, lingerTime);
      boost::system::error_code error;
      _socket.set_option(option, error);
      if (error)
      {
        LOG4CXX_WARN(logger, __FUNCTION__ << ' ' << err.message());
      }
    }
    if (!_stop)
    {
      _ioService.post(boost::bind(&Client::processRequest, this));
    }
  }
  else if (_pool)
  {
    _pool->setStatus(RC_CONNECTION_REFUSED);
  }
}

void Client::handleWriteRequest(RCRequestPtr request,
                                const boost::system::error_code& err,
                                size_t bytesTransferred)
{
  boost::system::error_code ignore;
  size_t numberCanceled(_timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err
      && numberCanceled > 0
      && request->_requestVector.size() == bytesTransferred)
  {
    _timer.expires_at(request->_expiresAt, ignore);
    asyncProcessingWait(request);
    request->_responseHeaderVect.resize(_headerSz);
    boost::asio::async_read(
      _socket,
      boost::asio::buffer(request->_responseHeaderVect, _headerSz),
      boost::bind(&Client::handleReadHeader,
                  this,
                  request,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    onError(request, translateError(err), err);
  }
}

void Client::handleReadHeader(RCRequestPtr request,
                              const boost::system::error_code& err,
                              std::size_t bytesTransferred)
{
  boost::system::error_code ignore;
  boost::posix_time::ptime expiresAt(_timer.expires_at());
  bool timeout(0 == _timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err && !timeout)
  {
    assert(bytesTransferred == _headerSz);
    if (readHeader(request->_responseHeader, request->_responseHeaderVect))
    {
      assert(request->_responseHeader._requestId == request->_requestId);
      _timer.expires_at(expiresAt, ignore);
      asyncProcessingWait(request);
      std::size_t payloadSz(request->_responseHeader._payloadSize);
      request->_responsePayloadVect.resize(payloadSz);
      boost::asio::async_read(
        _socket,
        boost::asio::buffer(request->_responsePayloadVect, payloadSz),
        boost::bind(&Client::handleReadPayload,
                    this,
                    request,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
  }
  else if (err)
  {
    onError(request, translateError(err), err);
  }
}

void Client::handleReadPayload(RCRequestPtr request,
                               const boost::system::error_code& err,
                               std::size_t bytesTransferred)
{
  boost::system::error_code ignore;
  bool timeout(0 == _timer.expires_from_now(boost::posix_time::seconds(0), ignore));
  if (!err && !timeout && request->_responseHeader._payloadSize == bytesTransferred)
  {
    switch (request->_responseHeader._status)
    {
    case RC_NONE:
      break;
    case RC_COMPRESSED_VALUE:
    case RC_UNCOMPRESSED_VALUE:
      if (!_stop)
      {
        _ioService.post(boost::bind(&Client::processRequest, this));
      }
      break;
    default:
      onError(request, request->_responseHeader._status, err);
      break;
    }
  }
  else
  {
    onError(request, RC_READ_ERROR, err);
  }
  request->notify();
}

void Client::handleConnectTimer(const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << ":connect timeout");
    if (_pool)
    {
      _pool->setStatus(RC_CONNECTION_REFUSED);
    }
    _ioService.stop();
  }
}

void Client::handleProcessingTimer(RCRequestPtr request,
                                   const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted)
  {
    onError(request, RC_CLIENT_PROCESSING_TIMEOUT, err);
  }
}

void Client::asyncConnectWait()
{
  _timer.async_wait(boost::bind(&Client::handleConnectTimer,
                                this,
                                boost::asio::placeholders::error));
}

void Client::asyncProcessingWait(RCRequestPtr request)
{
  _timer.async_wait(boost::bind(&Client::handleProcessingTimer,
                                this,
                                request,
                                boost::asio::placeholders::error));
}

void Client::onError(RCRequestPtr request,
                     StatusType status,
                     const boost::system::error_code& err)
{
  if (request)
  {
    request->onClientError(status, err);
  }
  if (_pool)
  {
    _pool->setStatus(status);
  }
}

}// RemoteCache

}// tse
