#pragma once

#include "DBAccess/RemoteCache/ASIOServer/Reply.h"
#include "DBAccess/RemoteCache/ASIOServer/Request.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <atomic>
#include <thread>

namespace tse
{

namespace RemoteCache
{

typedef boost::shared_ptr<class Session> SessionPtr;

class Session : boost::noncopyable
{
public:

  Session();
  ~Session();

  boost::asio::ip::tcp::socket& socket() { return _socket; }
  void start();
  void stop();
  bool stopped() const;

  static int getNumberSessions();
private:
  void run();
  void processRequest();
  void handleReadHeader(RequestPtr request,
                        ReplyPtr reply,
                        const boost::system::error_code& e,
                        std::size_t bytesTransferred);
  void handleReadPayload(RequestPtr request,
                         ReplyPtr reply,
                         const boost::system::error_code& e,
                         std::size_t bytesTransferred);
  void handleWrite(ReplyPtr reply,
                   const boost::system::error_code& e,
                   std::size_t bytesTransferred);
  void onError(ReplyPtr reply,
               StatusType status);
  void asyncIdleWait();
  void asyncReceiveWait(ReplyPtr reply);
  void asyncSendWait(ReplyPtr reply);
  void handleIdleTimer(const boost::system::error_code& err);
  void handleReceiveTimer(ReplyPtr reply,
                          const boost::system::error_code& e);
  void handleSendTimer(ReplyPtr reply,
                       const boost::system::error_code& e);
  void handleStop();

  std::thread _thread;
  boost::asio::io_service _ioService;
  boost::asio::ip::tcp::socket _socket;
  boost::asio::deadline_timer _timer;
  StatusType _status{RC_NONE};
  std::atomic<bool> _stopped{false};
};

}// RemoteCache

}// tse
