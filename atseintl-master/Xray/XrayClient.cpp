// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "XrayClient.h"

#include "Common/Logger.h"

#include <boost/bind.hpp>

#include <string>
#include <iostream>

using boost::asio::buffer;
using boost::asio::deadline_timer;
using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::ssl::context;
using boost::asio::ssl::stream;
using boost::asio::ssl::stream_base;
using boost::asio::ssl::verify_peer;
using boost::asio::streambuf;
using boost::posix_time::milliseconds;
using boost::posix_time::pos_infin;
using boost::system::error_code;

namespace tse
{
namespace xray
{
static Logger
logger("atseintl.Xray.XrayClient");

XrayClient::XrayClient(const std::string& host,
                       const uint16_t port,
                       const bool isHttps,
                       const std::string& request,
                       const uint32_t connectTimeout,
                       context& context)
  : _requestMessage(request),
    _isHttps(isHttps),
    _resolver(_ioService),
    _socket(_ioService, context),
    _query(host, std::to_string(port)),
    _timer(_ioService),
    _connectTimeout(connectTimeout)
{
  error_code ignore;
  _timer.expires_at(pos_infin, ignore);
}

void
XrayClient::process()
{
  error_code ignore;
  _timer.expires_from_now(milliseconds(_connectTimeout), ignore);
  asyncConnectWait();
  _resolver.async_resolve(_query,
                          bind(&XrayClient::handleResolve,
                               this,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::iterator));
  _ioService.run();
}

void
XrayClient::handleResolve(const error_code& error, tcp::resolver::iterator endpointIterator)
{
  if (!error)
  {
    // Close socket from possible previous attempt
    if (_socket.lowest_layer().is_open())
    {
      error_code ignore;
      _socket.lowest_layer().close(ignore);
    }
    _socket.set_verify_mode(verify_peer);
    async_connect(_socket.lowest_layer(),
                  endpointIterator,
                  bind(&XrayClient::handleConnect, this, boost::asio::placeholders::error));
  }
  else
  {
    LOG4CXX_ERROR(logger, "Resolve failed: " + error.message());
    _isError = true;
    stop();
  }
}

void
XrayClient::handleConnect(const error_code& error)
{
  if (!error)
    if (_isHttps)
      _socket.async_handshake(
          stream_base::client,
          bind(&XrayClient::handleHandshake, this, boost::asio::placeholders::error));
    else
      async_write(_socket.next_layer(),
                  buffer(_requestMessage, _requestMessage.length()),
                  bind(&XrayClient::handleWrite, this, boost::asio::placeholders::error));

  else
  {
    LOG4CXX_ERROR(logger, "Connect failed: " + error.message());
    _isError = true;
    stop();
  }
}

void
XrayClient::handleHandshake(const error_code& error)
{
  if (!error)
    async_write(_socket,
                buffer(_requestMessage, _requestMessage.length()),
                bind(&XrayClient::handleWrite, this, boost::asio::placeholders::error));
  else
  {
    LOG4CXX_ERROR(logger, "Handshake failed: " + error.message());
    _isError = true;
    stop();
  }
}

void
XrayClient::handleWrite(const error_code& error)
{
  if (!error)
    if (_isHttps)
      async_read_until(_socket,
                       _responseMessage,
                       "\r\n",
                       bind(&XrayClient::handleRead, this, boost::asio::placeholders::error));
    else
      async_read_until(_socket.next_layer(),
                       _responseMessage,
                       "\r\n",
                       bind(&XrayClient::handleRead, this, boost::asio::placeholders::error));
  else
  {
    LOG4CXX_ERROR(logger, "Write request failed: " + error.message());
    _isError = true;
    stop();
  }
}

void
XrayClient::handleRead(const error_code& error)
{
  if (!error)
  {
    error_code ignore;
    _timer.cancel(ignore);
  }
  else
  {
    LOG4CXX_ERROR(logger, "response read failed: " + error.message());
    _isError = true;
    stop();
  }
}

void
XrayClient::handleTimer(const error_code& error)
{
  if (error == boost::asio::error::operation_aborted)
    return;

  if (_timer.expires_at() <= deadline_timer::traits_type::now())
  {
    LOG4CXX_ERROR(logger, "Connection timeout");
    _isError = true;
    stop();
  }
}

void
XrayClient::asyncConnectWait()
{
  _timer.async_wait(bind(&XrayClient::handleTimer, this, boost::asio::placeholders::error));
}

std::string
XrayClient::getResponse()
{
  if (_isError)
    return "";
  std::istream responseStream(&_responseMessage);
  const std::string content(std::istreambuf_iterator<char>(responseStream), {});
  return content;
}

void
XrayClient::stop()
{
  error_code ignore;
  _timer.cancel(ignore);
  if (_socket.lowest_layer().is_open())
    _socket.lowest_layer().close();

  if (!_ioService.stopped())
    _ioService.stop();
}
}
}
