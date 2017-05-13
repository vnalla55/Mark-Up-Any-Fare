// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "TestServer/Server/Settings.h"

namespace tax
{
class TestServerRequestRecorder;

class Session
{
public:
  Session(boost::asio::io_service& io_service);

  boost::asio::ip::tcp::socket& getSocket()
  {
    return _socket;
  }

  void start();
  std::string readXmlString();
  uint32_t getPayloadSize();

private:
  void recordRequest(const std::string& xmlRequest);
  void addResponseHeader(size_t payloadSize, std::string& responseMessage);
  boost::asio::ip::tcp::socket _socket;
  std::vector<unsigned char> _receivedMessage;
  TestServerRequestRecorder& _requestRecorder;
};
} // namespace tax
