#pragma once

#include <boost/asio.hpp>

namespace tse
{

struct ASIORequest;

class AsyncClient : boost::noncopyable
{
public:

  AsyncClient(const std::string& host,
              const std::string& port,
              unsigned long connectTimeout,
              unsigned long processTimeout,
              bool linger,
              int lingerTime,
              size_t receiveBufferSize = 8192);

  AsyncClient(const std::string& host,
              const std::string& port,
              unsigned long connectTimeout,
              unsigned long processTimeout,
              bool linger,
              int lingerTime,
              std::vector<char>& buffer);

  ~AsyncClient();

  void process(ASIORequest& request);

  static int getNumberClients();

protected:

  bool stopped() const;

  void stop();

  void handleResolve(ASIORequest& request,
                     const boost::system::error_code& err,
                     boost::asio::ip::tcp::resolver::iterator endpointIter);

  void handleConnect(ASIORequest& request,
                     const boost::system::error_code& err);

  void handleWriteRequest(ASIORequest& request,
                          const boost::system::error_code& err,
                          size_t bytesTransferred);

  void handleReadResponse(ASIORequest& request,
                          const boost::system::error_code& err,
                          size_t bytesTransferred);

  void handleConnectTimer(ASIORequest& request,
                          const boost::system::error_code& err);

  void handleProcessingTimer(ASIORequest& request,
                             const boost::system::error_code& err);

  void asyncConnectWait(ASIORequest& request);

  void asyncProcessingWait(ASIORequest& request);

  boost::shared_ptr<std::vector<char> > _receiveBufferPtr;
  boost::asio::io_service _ioService;
  boost::asio::ip::tcp::socket _socket;
  boost::asio::deadline_timer _timer;
  std::vector<char>& _receiveBuffer;
  const std::string _host;
  const std::string _port;
  const unsigned long _connectTimeout;
  const unsigned long _processTimeout;
  const bool _linger;
  const int _lingerTime;
  boost::asio::ip::tcp::resolver _resolver;
  const boost::asio::ip::tcp::resolver::query _resolveQuery;
  std::vector<boost::asio::const_buffer> _sendBuffers;
  size_t _requestSize;
};

}// tse
