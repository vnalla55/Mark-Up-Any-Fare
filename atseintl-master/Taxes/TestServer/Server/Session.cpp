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
#include <iostream>
#include <algorithm>
#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

#include "TestServer/Facades/TaxStringTestProcessor.h"
#include "TestServer/Server/MsgHeader.h"
#include "TestServer/Server/Session.h"
#include "TestServer/Server/TestServerRequestRecorder.h"

using namespace tax;
using namespace std;
using boost::asio::ip::tcp;

Session::Session(boost::asio::io_service& io_service)
  : _socket(io_service), _requestRecorder(*(TestServerRequestRecorder::Init()))
{
}

void Session::start()
{
  TaxStringTestProcessor taxStringProcessor;
  std::string xmlRequest = readXmlString();
  _requestRecorder.recordRequest(xmlRequest);

  std::string& responseXml = taxStringProcessor.processString(xmlRequest).getResponseMessage();

  TestServerRequestRecorder* sr = TestServerRequestRecorder::Init();
  if (sr->verbose())
    std::cout << "ResponseToStringBuilder::buildFrom::responseXml: " << std::endl << responseXml << std::endl;

  std::string responseMessage;
  addResponseHeader(responseXml.size(), responseMessage);

  responseMessage.append(responseXml);
  sr->newResponse(responseXml);

  if (!responseMessage.empty())
    boost::asio::write(_socket,
                       boost::asio::buffer(responseMessage.c_str(), responseMessage.size() + 1));

  _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  _socket.close();
}

std::string Session::readXmlString()
{
  bool firstMessage = true;
  uint32_t payloadSize = 0;
  size_t fullMessageSize = 0;
  size_t partialMessageSize = 0;
  try
  {
    for (;;)
    {
      boost::array<unsigned char, 1024> buffer;
      boost::system::error_code error;

      if (fullMessageSize < 20 + payloadSize)
      {
        partialMessageSize = _socket.read_some(boost::asio::buffer(buffer), error);

        std::copy(buffer.begin(), buffer.begin() + partialMessageSize,
                  std::back_inserter(_receivedMessage));

        fullMessageSize += partialMessageSize;

        if (firstMessage)
        {
          payloadSize = getPayloadSize();
          firstMessage = false;
        }
      }
      else
      {
        break;
      }

      if (error == boost::asio::error::eof)
      {
        break;
      }
      else if (error)
      {
        throw boost::system::system_error(error);
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
  _requestRecorder.recordPayloadSizeWithFullMessageSize(payloadSize, fullMessageSize);

  std::string result;
  if (_receivedMessage.size() > 28)
  {
    result = std::string(_receivedMessage.begin() + 28, _receivedMessage.end());
  }
  return result;
}

uint32_t Session::getPayloadSize()
{

  uint32_t payloadSize = 0;
  _requestRecorder.recordReceivedMessageSize(_receivedMessage.size());

  if (_receivedMessage.size() < 20)
  {
    return payloadSize;
  }

  memcpy(&payloadSize, &_receivedMessage[4], 4);
  payloadSize = ntohl(payloadSize);

  _requestRecorder.recordPayloadSize(payloadSize);

  return payloadSize;
}

void
Session::addResponseHeader(size_t payloadSize, std::string& responseMessage)
{
  MsgHeader responseHeader(uint32_t(payloadSize + 1));
  char* header = reinterpret_cast<char*>(&responseHeader);
  responseMessage.append(header, sizeof(responseHeader));
}

