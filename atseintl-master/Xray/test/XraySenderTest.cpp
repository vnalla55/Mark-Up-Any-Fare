//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#include "Xray/XraySender.h"
#include "Xray/XrayClient.h"

#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/regex.hpp>

#include <cstddef>
#include <string>

using ::testing::_;
using ::testing::Eq;
using ::testing::Truly;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::Mock;

namespace tse
{
namespace xray
{
class MockClient
{
public:
  MOCK_METHOD6(constructor,
               void(const std::string& host,
                    const uint16_t port,
                    const bool isHttps,
                    const std::string& request,
                    const uint32_t connectTimeout,
                    boost::asio::ssl::context& context));
  MOCK_METHOD0(process, void());
  MOCK_METHOD0(getResponse, std::string());
};

MockClient mockClient;

class MockClientWrapper
{
public:
  MockClientWrapper(const std::string& host,
                    const uint16_t port,
                    const bool isHttps,
                    const std::string& request,
                    const uint32_t connectTimeout,
                    boost::asio::ssl::context& context)
  {
    mockClient.constructor(host, port, isHttps, request, connectTimeout, context);
  }
  void process() { mockClient.process(); }
  std::string getResponse() { return mockClient.getResponse(); }
};

constexpr uint16_t HTTPS_PORT = 443;

class XraySenderTest : public ::testing::Test
{
protected:
  TestConfigInitializer cfg;
  XraySenderTest()
  {
    TestConfigInitializer::setValue<std::string>("SERVER_ADDRESS", "xray.addr/xray.path", "XRAY");
    TestConfigInitializer::setValue<std::string>("SERVER_PORT", std::to_string(HTTPS_PORT), "XRAY");
    TestConfigInitializer::setValue<std::string>(
        "AUTH_SERVER_ADDRESS", "auth.addr/auth.path", "XRAY");
    TestConfigInitializer::setValue<std::string>(
        "AUTH_SERVER_PORT", std::to_string(HTTPS_PORT), "XRAY");
    TestConfigInitializer::setValue<std::string>("AUTH_USER", "auth.user", "XRAY");
    TestConfigInitializer::setValue<std::string>("AUTH_PASS", "auth.pass", "XRAY");
    TestConfigInitializer::setValue<std::string>("DIRECT_CONNECTION", "N", "XRAY");
  }
};

bool
checkXrayMessageHeader(const std::string& header)
{
  if (!boost::regex_search(header, boost::regex("POST +/xray.path +HTTP/1.0 *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Host: +xray.addr *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Content-Type: +application/json *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Content-Length: +4 *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Authorization: +Bearer token *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Connection: +close\r\n\r\n *\\[\\{\\}\\]")))
    return false;

  return true;
}

bool
checkAuthMessageHeader(const std::string& header)
{
  if (!boost::regex_search(header, boost::regex("POST +/auth.path +HTTP/1.0 *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Host: +auth.addr *\r\n")))
    return false;
  if (!boost::regex_search(header,
                           boost::regex("Content-Type: +application/x-www-form-urlencoded *\r\n")))
    return false;
  if (!boost::regex_search(header, boost::regex("Content-Length: +29 *\r\n")))
    return false;
  if (!boost::regex_search(
          header, boost::regex("Authorization: +Basic WVhWMGFDNTFjMlZ5OllYVjBhQzV3WVhOeg== *\r\n")))
    return false;
  if (!boost::regex_search(
          header, boost::regex("Connection: +close *\r\n\r\ngrant_type=client_credentials")))
    return false;

  return true;
}

TEST_F(XraySenderTest, testClassMethodExecution)
{
  InSequence dummy;
  std::string authResponse = "HTTP/1.1 200 OK\r\n Connection: close\r\n\r\n{\"access_token\": "
                             "\"token\" ,\r\n \"token_type\": \"bearer\" ,\r\n \"expires_in\": "
                             "900}";
  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("auth.addr")), HTTPS_PORT, false, Truly(checkAuthMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1).WillOnce(Return(authResponse));
  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("xray.addr")), HTTPS_PORT, false, Truly(checkXrayMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1);
  XraySender<MockClientWrapper> sender;
  sender.send("[{}]");
  Mock::VerifyAndClearExpectations(&mockClient);
}

TEST_F(XraySenderTest, testFirstUpdateFailure)
{
  std::string authResponse = "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n{\"access_token\": "
                             "\"token\" ,\r\n \"token_type\": \"bearer\" ,\r\n \"expires_in\": "
                             "900}";
  std::string xrayResponse = "HTTP/1.1 401 Unauthorized\r\nConnection: close\r\n\r\n";

  InSequence dummy;
  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("auth.addr")), HTTPS_PORT, false, Truly(checkAuthMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1).WillOnce(Return(authResponse));

  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("xray.addr")), HTTPS_PORT, false, Truly(checkXrayMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1).WillOnce(Return(xrayResponse));

  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("auth.addr")), HTTPS_PORT, false, Truly(checkAuthMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1).WillOnce(Return(authResponse));

  EXPECT_CALL(
      mockClient,
      constructor(
          Eq(std::string("xray.addr")), HTTPS_PORT, false, Truly(checkXrayMessageHeader), _, _))
      .Times(1);
  EXPECT_CALL(mockClient, process()).Times(1);
  EXPECT_CALL(mockClient, getResponse()).Times(1);

  XraySender<MockClientWrapper> sender;
  sender.send("[{}]");
  Mock::VerifyAndClearExpectations(&mockClient);
}
}
}
