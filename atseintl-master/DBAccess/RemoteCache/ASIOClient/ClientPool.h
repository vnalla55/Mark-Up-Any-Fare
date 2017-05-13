#pragma once

#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

#include "DBAccess/RemoteCache/ASIOClient/ClientStatusBase.h"

#include <boost/noncopyable.hpp>

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace tse
{

namespace RemoteCache
{

struct RCRequest;

typedef std::shared_ptr<class Client> ClientPtr;

typedef std::shared_ptr<class Client> ClientPtr;

typedef std::shared_ptr<RCRequest> RCRequestPtr;

typedef std::shared_ptr<class ClientPool> ClientPoolPtr;

class ClientPool : boost::noncopyable
{
public:
  ClientPool(const std::string& host,
             const std::string& port);
  ~ClientPool();

  std::pair<bool, StatusType> isEnabled();
  void setStatus(StatusType status);
  bool enqueue(RCRequestPtr request,
               bool healthcheck = false);
  RCRequestPtr getRequest();
  const std::string& getHost() const { return _host; }
  const std::string& getPort() const { return _port; }
  bool needHealthcheck();
  bool healthcheck(std::string& msg);
  bool healthcheckResult() const { return _healthcheckResult; }
private:
  size_t getNumberClients();
  bool clientAvailable(bool healthcheck);
  void healthcheckRun();
  void runHealthchecks();
  void stop();

  std::mutex _mutex;
  std::condition_variable _condition;
  typedef std::list<ClientPtr> Clients;
  Clients _clients;
  typedef std::deque<RCRequestPtr> Requests;
  Requests _requestQueue;
  const std::string _host;
  const std::string _port;

  class PoolStatus : public ClientStatusBase
  {
  public:
    std::pair<bool, StatusType> isEnabled();
    bool set(StatusType status);
    bool getNeedHealthcheck() const { return _needHealthcheck; }
    void setNeedHealthcheck(bool need) { _needHealthcheck = need; }

  private:
    std::atomic<bool> _needHealthcheck{true};
  } _poolStatus;
  std::atomic<bool> _stopped{false};
  std::size_t _numberRequests{0};
  std::size_t _totalQueue{0};
  std::size_t _sample{0};
  std::atomic<bool> _healthcheckResult{true};
  std::atomic<bool> _needHealthcheck{false};
  std::mutex _healthcheckMutex;
  std::condition_variable _healthcheckCondition;
  std::thread _healthcheckThread;
  std::atomic<std::time_t> _lastHealthcheck;
  std::atomic<std::time_t> _lastAdjustment;
};

}// RemoteCache

}// tse
