#pragma once

#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <atomic>
#include <memory>

namespace tse
{

namespace RemoteCache
{

typedef std::shared_ptr<struct RCRequest> RCRequestPtr;

class ClientPool;

class Client : boost::noncopyable
{
public:

  explicit Client(ClientPool* pool);
  ~Client();

  void stop();
  void cancel();
  bool stopped() const;
  bool willStop() const { return _stop; }

  static int getNumberClients();

private:
  void run();
  void connect();
  void processRequest();
  void handleConnect(const boost::system::error_code& err);
  void handleWriteRequest(RCRequestPtr request,
                          const boost::system::error_code& err,
                          std::size_t bytesTransferred);
  void handleReadHeader(RCRequestPtr request,
                        const boost::system::error_code& err,
                        std::size_t bytesTransferred);
  void handleReadPayload(RCRequestPtr request,
                         const boost::system::error_code& err,
                         std::size_t bytesTransferred);
  void handleConnectTimer(const boost::system::error_code& err);
  void handleProcessingTimer(RCRequestPtr request,
                             const boost::system::error_code& err);
  void asyncConnectWait();
  void asyncProcessingWait(RCRequestPtr request);
  void onError(RCRequestPtr request,
               StatusType status,
               const boost::system::error_code& err);
  void handleStop();

  ClientPool* _pool{nullptr};
  boost::asio::io_service _ioService;
  boost::asio::ip::tcp::socket _socket;
  boost::asio::deadline_timer _timer;
  boost::thread _thread;
  std::atomic<bool> _stop{false};
};

}// RemoteCache

}// tse
