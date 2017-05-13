//----------------------------------------------------------------------------
//
//  File:               XraySender.h
//  Description:        Sender for Xray
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

#include "IXraySender.h"

#include "Common/Logger.h"
#include "Util/Base64.h"
#include "Xray/XrayUtil.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <chrono>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

using boost::asio::ip::tcp;

namespace tse
{
namespace xray
{
template <class ClientType>
class XraySender final : public IXraySender
{
public:
  XraySender();
  void send(const std::string& message) override;

private:
  void updateToken();
  std::string prepareHeaderForTokenUpdate() const;
  std::string prepareHeaderForXray(const std::string& message) const;
  std::string getEncodedTokenCredential() const;
  int32_t sendHttpMessage(const std::string& serverUrl,
                          const uint16_t port,
                          const bool isHttps,
                          const std::string& message,
                          std::string& response);

  std::string _xrayServerUrlAddress;
  std::string _xrayServerUrlPath;
  uint16_t _xrayServerPort;
  bool _isHttpsXrayServer = false;
  std::string _tokenServerUrlAddress;
  std::string _tokenServerUrlPath;
  uint16_t _tokenServerPort;
  bool _isHttpsTokenServer = false;
  std::string _tokenUserName;
  std::string _tokenUserPass;
  std::chrono::time_point<std::chrono::system_clock> _lastTokenUpdate;
  std::chrono::duration<int> _tokenUpdatePeriod = std::chrono::seconds(0);
  std::string _token;
};
}
}

namespace tse
{
namespace
{
constexpr int32_t HTTP_OK = 200;
constexpr int32_t HTTP_UNAUTHORIZED = 401;
constexpr int32_t CONNECT_TIMEOUT_MS = 4000;
constexpr int32_t NO_HTTP_ERROR = -1;

const std::string httpsProtocolString = "https://";

static Logger
loggerSender("atseintl.Xray.XraySender");

std::string
getJsonValue(const std::string& json, const std::string& key)
{
  std::stringstream sstream(json);
  boost::property_tree::ptree tree;
  boost::property_tree::read_json(sstream, tree);
  return tree.get<std::string>(key);
}

std::string
extractAddrFromUrl(const std::string& url)
{
  const size_t addressSubstringBeginId = url.find("://");
  if (addressSubstringBeginId == std::string::npos)
  {
    const size_t addressSubstringEndId = url.find('/');
    return url.substr(0, addressSubstringEndId);
  }
  else
  {
    const size_t addressSubstringEndId = url.find('/', addressSubstringBeginId + 3);
    return url.substr(addressSubstringBeginId + 3,
                      addressSubstringEndId - addressSubstringBeginId - 3);
  }
}

std::string
extractPathFromUrl(const std::string& url)
{
  const size_t addressSubstringBeginId = url.find("://");
  if (addressSubstringBeginId == std::string::npos)
  {
    const size_t substringBegin = url.find('/');
    if (substringBegin == std::string::npos)
      return "";
    return url.substr(substringBegin);
  }
  else
  {
    const size_t substringBegin = url.find('/', addressSubstringBeginId + 3);
    if (substringBegin == std::string::npos)
      return "";
    return url.substr(substringBegin);
  }
}

bool
checkIfHttps(const std::string& url)
{
  return url.compare(0, httpsProtocolString.length(), httpsProtocolString) == 0;
}

int32_t
processHttpResponse(const std::string& responseWithHead, std::string& response)
{
  std::istringstream responseStream(responseWithHead);
  std::string httpVersion;
  responseStream >> httpVersion;
  int32_t responseCode;
  responseStream >> responseCode;

  if (httpVersion.substr(0, 5) != "HTTP/")
  {
    LOG4CXX_ERROR(loggerSender, "Invalid HTTP response");
    return -1;
  }

  if (responseCode == HTTP_OK)
  {
    LOG4CXX_INFO(loggerSender, std::string("HTTP response code: ") + std::to_string(responseCode));
    const size_t bodyBeginPos = responseWithHead.find("\r\n\r\n");
    if (bodyBeginPos != std::string::npos)
      response = responseWithHead.substr(bodyBeginPos + 4);
  }
  else
  {
    LOG4CXX_ERROR(loggerSender, std::string("HTTP response code: ") + std::to_string(responseCode));
  }
  return responseCode;
}
}

namespace xray
{
template <class ClientType>
XraySender<ClientType>::XraySender()
  : _xrayServerUrlAddress(extractAddrFromUrl(xrayServerAddrCfg.getValue())),
    _xrayServerUrlPath(extractPathFromUrl(xrayServerAddrCfg.getValue())),
    _xrayServerPort(xrayServerPortCfg.getValue()),
    _isHttpsXrayServer(checkIfHttps(xrayServerAddrCfg.getValue())),
    _tokenServerUrlAddress(extractAddrFromUrl(authServerAddrCfg.getValue())),
    _tokenServerUrlPath(extractPathFromUrl(authServerAddrCfg.getValue())),
    _tokenServerPort(authServerPortCfg.getValue()),
    _isHttpsTokenServer(checkIfHttps(authServerAddrCfg.getValue())),
    _tokenUserName(authUserCfg.getValue()),
    _tokenUserPass(authPassCfg.getValue())
{
  if (directConnection.getValue())
    _token = Base64::encode(_tokenUserName + ":" + _tokenUserPass);
}

template <class ClientType>
void
XraySender<ClientType>::send(const std::string& message)
{
  if (!directConnection.getValue())
  {
    if (std::chrono::system_clock::now() - _lastTokenUpdate > _tokenUpdatePeriod)
      updateToken();
  }

  LOG4CXX_DEBUG(loggerSender, "Sending " << message);
  const std::string requestMessage = prepareHeaderForXray(message);
  std::string response;
  int32_t responseCode = sendHttpMessage(
      _xrayServerUrlAddress, _xrayServerPort, _isHttpsXrayServer, requestMessage, response);

  if (!directConnection.getValue())
  {
    if (responseCode == HTTP_UNAUTHORIZED)
    {
      LOG4CXX_INFO(loggerSender, "Trying to update token due to 401 error");
      updateToken();
      const std::string requestMessage = prepareHeaderForXray(message);
      responseCode = sendHttpMessage(
          _xrayServerUrlAddress, _xrayServerPort, _isHttpsXrayServer, requestMessage, response);
    }
  }

  if (responseCode == HTTP_OK)
  {
    LOG4CXX_INFO(loggerSender, "Message sent successfully");
  }
  else if (responseCode != NO_HTTP_ERROR)
  {
    LOG4CXX_ERROR(loggerSender,
                  "HTTP error: " + std::to_string(responseCode) + " during sending message.");
  }
  else
  {
    LOG4CXX_ERROR(loggerSender, "Error during sending message.");
  }
}

template <class ClientType>
void
XraySender<ClientType>::updateToken()
{
  const std::string requestMessage = prepareHeaderForTokenUpdate();
  std::string response;
  int32_t responseCode = sendHttpMessage(
      _tokenServerUrlAddress, _tokenServerPort, _isHttpsTokenServer, requestMessage, response);

  if (responseCode == HTTP_OK)
  {
    try
    {
      std::string token = getJsonValue(response, "access_token");
      std::string expiresTime = getJsonValue(response, "expires_in");

      _token = token;
      _tokenUpdatePeriod = std::chrono::seconds(std::stol(expiresTime));
      _lastTokenUpdate = std::chrono::system_clock::now();
    }
    catch (const std::exception& e)
    {
      LOG4CXX_ERROR(loggerSender, "Exception during obtaining token: " + std::string(e.what()));
      return;
    }
    LOG4CXX_INFO(loggerSender, "Token updated successfully: " + _token);
  }
  else if (responseCode != NO_HTTP_ERROR)
  {
    LOG4CXX_ERROR(loggerSender,
                  "HTTP error: " + std::to_string(responseCode) + " during updating token.");
  }
  else
  {
    LOG4CXX_ERROR(loggerSender, "Error during updating token.");
  }
}

template <class ClientType>
std::string
XraySender<ClientType>::prepareHeaderForTokenUpdate() const
{
  std::ostringstream request_stream;

  std::string message = "grant_type=client_credentials";

  request_stream << "POST " << _tokenServerUrlPath << " HTTP/1.0\r\n"
                 << "Host: " << _tokenServerUrlAddress << " \r\n"
                 << "Content-Type: application/x-www-form-urlencoded\r\n"
                 << "Content-Length: " << message.length() << "\r\n"
                 << "Authorization: Basic " << getEncodedTokenCredential() << "\r\n"
                 << "Connection: close\r\n\r\n" << message;

  return request_stream.str();
}

template <class ClientType>
std::string
XraySender<ClientType>::prepareHeaderForXray(const std::string& message) const
{
  std::ostringstream request_stream;
  std::string authType(directConnection.getValue() ? "Basic" : "Bearer");

  request_stream << "POST " << _xrayServerUrlPath << " HTTP/1.0\r\n"
                 << "Host: " << _xrayServerUrlAddress << " \r\n"
                 << "Content-Type: application/json \r\n"
                 << "Content-Length: " << message.length() << "\r\n"
                 << "Authorization: " << authType << " " << _token << "\r\n"
                 << "Connection: close\r\n\r\n" << message;

  return request_stream.str();
}

template <class ClientType>
std::string
XraySender<ClientType>::getEncodedTokenCredential() const
{
  return Base64::encode(Base64::encode(_tokenUserName) + ":" + Base64::encode(_tokenUserPass));
}

template <class ClientType>
int32_t
XraySender<ClientType>::sendHttpMessage(const std::string& serverUrlAddress,
                                        const uint16_t serverPort,
                                        const bool isHttps,
                                        const std::string& message,
                                        std::string& response)
{
  int32_t responseCode = NO_HTTP_ERROR;
  try
  {
    boost::asio::ssl::context context(boost::asio::ssl::context::sslv23);
    context.set_default_verify_paths();

    ClientType client(serverUrlAddress, serverPort, isHttps, message, CONNECT_TIMEOUT_MS, context);
    client.process();
    const std::string responseWithHead = client.getResponse();

    responseCode = processHttpResponse(responseWithHead, response);
  }
  catch (const std::exception& e)
  {
    LOG4CXX_ERROR(loggerSender, "Exception: " + std::string(e.what()));
  }

  return responseCode;
}
}
}
