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

#include <vector>
#include <ostream>
#include <iostream>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

namespace io = boost::iostreams;

namespace tax
{
class TestServerRequestRecorder
{
public:
  static TestServerRequestRecorder* Init()
  {
    if (!_this)
      return new TestServerRequestRecorder();
    return _this;
  }

  bool const& verbose() const
  {
    return _active;
  }

  bool& verbose()
  {
    return _verbose;
  }
  bool const& active() const
  {
    return _verbose;
  }

  bool& active()
  {
    return _active;
  }
  void addNumber()
  {
    (*_out) << _Index++ << std::endl;
  }

  void newRequest(const std::string& reqXML)
  {
    if (!_active)
      return;
    addNumber();
    std::string recXml = removeBadChar(reqXML);
    (*_out) << recXml << std::endl;
  }

  void newResponse(const std::string& respXML)
  {
    if (!_active)
      return;
    std::string recXml = removeBadChar(respXML);
    (*_out) << recXml << std::endl;
  }

  void setFile(const char* filePath)
  {
    _out->flush();
    _buf->close();
    delete _out;
    delete _buf;
    _buf = new io::stream_buffer<io::file_sink>(filePath);
    _out = new std::ostream(_buf);
  }

  void recordRequest(const std::string& xmlRequest)
  {
    if (verbose())
      (*_out) << "Received xml:" << std::endl << xmlRequest << std::endl;
    newRequest(xmlRequest);
  }

  void recordPayloadSizeWithFullMessageSize(uint32_t payloadSize, size_t fullMessageSize)
  {
    if (verbose())
    {
      std::cout << "payloadSize: " << payloadSize << std::endl;
      std::cout << "fullMessageSize: " << fullMessageSize << std::endl;
    }
  }

  void recordPayloadSize(uint32_t payloadSize)
  {
    if (verbose())
    {
      std::cout << "payloadSize: " << payloadSize << std::endl;
    }
  }


  void recordReceivedMessageSize(std::vector<unsigned char>::size_type receivedMessageSize)
  {
    if (verbose())
    {
      std::cout << "parseReceivedMessage size: " << receivedMessageSize << std::endl;
    }
    else
    {
      std::cout << "Request received" << std::endl;
    }
  }

  static std::string removeBadChar(const std::string& XML)
  {
    char chars[] = "\r\n";
    std::string recXml = XML;
    for (unsigned int i = 0; i < strlen(chars); ++i)
    {
      // you need include <algorithm> to use general algorithms like std::remove()
      recXml.erase(std::remove(recXml.begin(), recXml.end(), chars[i]), recXml.end());
    }
    return recXml;
  }

private:
  std::ostream const& ostream() const
  {
    return *_out;
  }

  std::ostream& ostream()
  {
    return *_out;
  }

  TestServerRequestRecorder() : _active(false), _Index(0), _verbose(false)
  {
    _this = this;
    _buf = new io::stream_buffer<io::file_sink>("RequestsLog.xml");
    _out = new std::ostream(_buf);
  }
  ~TestServerRequestRecorder()
  {
    _out->flush();
    _buf->close();
  }
  static TestServerRequestRecorder* _this;
  io::stream_buffer<io::file_sink>* _buf;
  std::ostream* _out;
  bool _active;
  int _Index;
  bool _verbose;
};

} // namespace tax
