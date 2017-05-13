//----------------------------------------------------------------------------
//
//  File:               XrayClient.h
//  Description:        HTTPS Client for Xray and auth server
//  Created:            07/29/2016
//  Authors:            Grzegorz Ryniak
//
//  Copyright Sabre 2016
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace tse
{
namespace xray
{
class XrayClient
{
public:
  XrayClient(const std::string& host,
             const uint16_t port,
             const bool isHttps,
             const std::string& request,
             const uint32_t connectTimeout,
             boost::asio::ssl::context& context);
  void process();
  std::string getResponse();

private:
  void handleResolve(const boost::system::error_code& error,
                     boost::asio::ip::tcp::resolver::iterator endpointIterator);
  void handleConnect(const boost::system::error_code& error);
  void handleHandshake(const boost::system::error_code& error);
  void handleWrite(const boost::system::error_code& error);
  void handleRead(const boost::system::error_code& error);
  void handleTimer(const boost::system::error_code& error);
  void asyncConnectWait();
  void stop();

  const std::string& _requestMessage;
  boost::asio::streambuf _responseMessage;
  bool _isHttps;
  bool _isError = false;
  boost::asio::io_service _ioService;
  boost::asio::ip::tcp::resolver _resolver;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> _socket;
  boost::asio::ip::tcp::resolver::query _query;
  boost::asio::deadline_timer _timer;
  uint32_t _connectTimeout;
};
}
}
