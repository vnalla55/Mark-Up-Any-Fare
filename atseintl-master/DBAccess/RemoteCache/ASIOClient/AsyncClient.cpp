#include "DBAccess/RemoteCache/ASIOClient/AsyncClient.h"
#include <iostream>
#include <boost/bind.hpp>
#include "DBAccess/RemoteCache/ASIOClient/ASIORequest.h"

namespace tse
{

namespace
{

std::atomic<int> _numberClients(0);

}// namespace

AsyncClient::AsyncClient(const std::string& host,
                         const std::string& port,
                         unsigned long connectTimeout,
                         unsigned long processTimeout,
                         bool linger,
                         int lingerTime,
                         size_t receiveBufferSize)
  : _receiveBufferPtr(new std::vector<char>(receiveBufferSize))
  , _socket(_ioService)
  , _timer(_ioService)
  , _receiveBuffer(*_receiveBufferPtr)
  , _host(host)
  , _port(port)
  , _connectTimeout(connectTimeout)
  , _processTimeout(processTimeout)
  , _linger(linger)
  , _lingerTime(lingerTime)
  , _resolver(_ioService)
  , _resolveQuery(_host, _port)
  , _requestSize(0)
{
  ++_numberClients;
  boost::system::error_code ignore;
  _timer.expires_at(boost::posix_time::pos_infin, ignore);
}

AsyncClient::AsyncClient(const std::string& host,
                         const std::string& port,
                         unsigned long connectTimeout,
                         unsigned long processTimeout,
                         bool linger,
                         int lingerTime,
                         std::vector<char>& buffer)
  : _socket(_ioService)
  , _timer(_ioService)
  , _receiveBuffer(buffer)
  , _host(host)
  , _port(port)
  , _connectTimeout(connectTimeout)
  , _processTimeout(processTimeout)
  , _linger(linger)
  , _lingerTime(lingerTime)
  , _resolver(_ioService)
  , _resolveQuery(_host, _port)
  , _requestSize(0)
{
  ++_numberClients;
  boost::system::error_code ignore;
  _timer.expires_at(boost::posix_time::pos_infin, ignore);
}

AsyncClient::~AsyncClient()
{
  --_numberClients;
  //std::cerr << __FUNCTION__ << " #### _numberClients=" << _numberClients << std::endl;
}

int AsyncClient::getNumberClients()
{
  return _numberClients;
}

bool AsyncClient::stopped() const
{
  return !_socket.is_open();
}

void AsyncClient::stop()
{
  boost::system::error_code ignore;
  _timer.cancel(ignore);
  if (_socket.is_open())
  {
    _socket.close(ignore);
  }
  if (!_ioService.stopped())
  {
    _ioService.stop();
  }
}

void AsyncClient::process(ASIORequest& request)
{
  boost::system::error_code ignore;
  _timer.expires_from_now(boost::posix_time::milliseconds(_connectTimeout), ignore);
  asyncConnectWait(request);
  // start an asynchronous resolve to translate the server and service names
  // into a list of endpoints
  _resolver.async_resolve(_resolveQuery,
                          boost::bind(&AsyncClient::handleResolve,
                                      this,
                                      boost::ref(request),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::iterator));
  boost::system::error_code ec;
  try
  {
    _ioService.run(ec);
  }
  catch (const boost::system::system_error& err)
  {
    stop();
    static const std::string prefix("boost::system::system_error:");
    request.onError(prefix + err.what());
  }
  catch (const std::exception& ex)
  {
    stop();
    static const std::string prefix("std::exception:");
    request.onError(prefix + ex.what());
  }
  catch (...)
  {
    stop();
    static const std::string prefix("unknown");
    request.onError(prefix);
  }
  if (ec)
  {
    static const std::string prefix("boost::system::error_code:");
    request.onError(prefix + ec.message());
  }
}

void AsyncClient::handleResolve(ASIORequest& request,
                                const boost::system::error_code& err,
                                boost::asio::ip::tcp::resolver::iterator endpointIter)
{
  if (!err)
  {
    // close socket from possible previous attempt
    if (_socket.is_open())
    {
      boost::system::error_code ignore;
      _socket.close(ignore);
    }
    // attempt a connection to each endpoint in the list until we
    // successfully establish a connection
    boost::asio::async_connect(_socket,
                               endpointIter,
                               boost::bind(&AsyncClient::handleConnect,
                                           this,
                                           boost::ref(request),
                                           boost::asio::placeholders::error));
  }
  else
  {
    stop();
    request.onConnectError(err);
  }
}

void AsyncClient::handleConnect(ASIORequest& request,
                                const boost::system::error_code& err)
{
  if (err)
  {
    stop();
    request.onConnectError(err);
  }
  else
  {
    // the connection was successful. Send the request
    boost::system::error_code error;
    _socket.set_option(boost::asio::ip::tcp::no_delay(), error);
    if (error)
    {
      request.onError(err.message());
    }
    if (_linger)
    {
      boost::asio::socket_base::linger option(true, _lingerTime);
      boost::system::error_code error;
      _socket.set_option(option, error);
      if (error)
      {
        request.onError(err.message());
      }
    }
    boost::system::error_code ignore;
    _timer.expires_from_now(boost::posix_time::milliseconds(_processTimeout), ignore);
    asyncProcessingWait(request);
    _requestSize = request.fillSendBuffers(_sendBuffers);
    boost::asio::async_write(_socket,
                             _sendBuffers,
                             boost::bind(&AsyncClient::handleWriteRequest,
                                         this,
                                         boost::ref(request),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
  }
}

void AsyncClient::handleWriteRequest(ASIORequest& request,
                                     const boost::system::error_code& err,
                                     size_t bytesTransferred)
{
  if (stopped())
    return;
  if (!err)
  {
    if (_requestSize != bytesTransferred)
    {
      std::ostringstream oss;
      oss <<  "ERROR:_requestSize=" << _requestSize
          << ",bytesTransferred=" << bytesTransferred;
      request.debugOutput(oss.str());
    }
    assert(_requestSize == bytesTransferred);
    _socket.async_read_some(
      boost::asio::buffer(_receiveBuffer),
      boost::bind(&AsyncClient::handleReadResponse,
                  this,
                  boost::ref(request),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    stop();
    request.onWriteError(err);
  }
}

void AsyncClient::handleReadResponse(ASIORequest& request,
                                     const boost::system::error_code& err,
                                     size_t bytesTransferred)
{
  if (stopped())
    return;
  if (!err)
  {
    boost::tribool result(request.readResponse(_receiveBuffer, bytesTransferred));
    if (result)
    {
      //stop();
      boost::system::error_code ignore;
      _timer.cancel(ignore);
      return;
    }
    else if (!result)// error
    {
      stop();
      request.debugOutput("request.readResponse detected error");
    }
    else// boost::indeterminate - read more
    {
      _socket.async_read_some(
        boost::asio::buffer(_receiveBuffer),
        boost::bind(&AsyncClient::handleReadResponse,
                    this,
                    boost::ref(request),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
    }
  }
  else if (boost::asio::error::eof != err)
  {
    stop();
    request.onReadError(err);
  }
}

void AsyncClient::handleConnectTimer(ASIORequest& request,
                                     const boost::system::error_code& err)
{
  if (err != boost::asio::error::operation_aborted)
  {
    if (_timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
      stop();
      request.onConnectTimeout();
    }
  }
}

void AsyncClient::handleProcessingTimer(ASIORequest& request,
                                        const boost::system::error_code& err)
{
  if (stopped())
    return;
  if (err != boost::asio::error::operation_aborted)
  {
    if (_timer.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
      stop();
      request.onProcessingTimeout();
    }
  }
}

void AsyncClient::asyncConnectWait(ASIORequest& request)
{
  _timer.async_wait(boost::bind(&AsyncClient::handleConnectTimer,
                                this,
                                boost::ref(request),
                                boost::asio::placeholders::error));
}

void AsyncClient::asyncProcessingWait(ASIORequest& request)
{
  if (stopped())
    return;
  _timer.async_wait(boost::bind(&AsyncClient::handleProcessingTimer,
                                this,
                                boost::ref(request),
                                boost::asio::placeholders::error));
}

}// tse
