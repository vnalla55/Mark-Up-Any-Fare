#pragma once

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/asio.hpp>

namespace tse
{

struct ASIORequest : boost::noncopyable
{
  virtual ~ASIORequest() {}

  virtual size_t fillSendBuffers(std::vector<boost::asio::const_buffer>& buffers) = 0;

  virtual size_t convertToBuffers(std::vector<boost::asio::const_buffer>& buffers) = 0;

  virtual boost::tribool readResponse(const std::vector<char>& buffer,
                                      size_t bytesTransferred) = 0;

  virtual void onConnectTimeout() = 0;

  virtual long calculateMaxProcessingTime() const = 0;

  virtual void onProcessingTimeout() = 0;

  virtual void onReadError(const boost::system::error_code& err) = 0;

  virtual void onWriteError(const boost::system::error_code& err) = 0;

  virtual void onConnectError(const boost::system::error_code& err) = 0;

  virtual void onError(const std::string& message = "") = 0;

  virtual void debugOutput(const std::string& text) = 0;

  virtual void onClientError(int status,
                             const boost::system::error_code& err,
                             const std::string& message = "") = 0;
};

}// tse
