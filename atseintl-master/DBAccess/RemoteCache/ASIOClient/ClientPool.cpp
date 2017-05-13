#include "DBAccess/RemoteCache/ASIOClient/ClientPool.h"

#include "Common/Logger.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOClient/Client.h"
#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"

#include <chrono>
#include <cmath>

namespace tse
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.ClientPool");

class ClearFlag
{
public:
  ClearFlag(std::atomic<bool>& flag)
    : _flag(flag)
  {
  }

  ~ClearFlag()
  {
    _flag = false;
  }
private:
  std::atomic<bool>& _flag;
};

}// namespace

namespace RemoteCache
{

std::pair<bool, StatusType> ClientPool::PoolStatus::isEnabled()
{
  if (_enabled)
  {
    return { true, RC_NONE };
  }
  std::time_t currentTime(std::time(nullptr));
  if (currentTime >= _errorTime + _retryInterval)
  {
    reset();
    _needHealthcheck = true;
    return { _enabled, RC_NONE };
  }
  return { _enabled, _status };
}

bool ClientPool::PoolStatus::set(StatusType status)
{
  if (ReadConfig::isHostStatus(status))
  {
    _errorTime = std::time(nullptr);
    _enabled = false;
    _needHealthcheck = true;
    if (_status != status)
    {
      _status = status;
      static const std::time_t minRetryInterval(10);
      std::time_t configRetryInterval(ReadConfig::getClientRetryInterval(status));
      _retryInterval = configRetryInterval >= minRetryInterval ? configRetryInterval : minRetryInterval;
      LOG4CXX_WARN(logger, "status=" << status << ',' << statusToString(status)
                   << ",retry in " << _retryInterval << 's');
    }
    return true;
  }
  return false;
}

ClientPool::ClientPool(const std::string& host,
                       const std::string& port)
  : _host(host)
  , _port(port)
  , _lastHealthcheck(std::time(nullptr))
  , _lastAdjustment(std::time(nullptr))
{
  if (ReadConfig::getAsynchronousHealthcheck())
  {
    std::thread t(std::bind(&ClientPool::runHealthchecks, this));
    _healthcheckThread.swap(t);
  }
}

ClientPool::~ClientPool()
{
  stop();
}

void ClientPool::runHealthchecks()
{
  while (!_stopped)
  {
    ClearFlag clearFlag(_needHealthcheck);
    try
    {
      std::unique_lock<std::mutex> lock(_healthcheckMutex);
      while (!_needHealthcheck)
      {
        _healthcheckCondition.wait(lock);
        if (_stopped)
        {
          return;
        }
      }
      if (_stopped)
      {
        break;
      }
      std::string msg;
      _healthcheckResult = healthcheck(msg);
      LOG4CXX_WARN(logger, __FUNCTION__ << ':' << msg);
    }
    catch (...)
    {
      // ignore
    }
  }
  LOG4CXX_DEBUG(logger, __FUNCTION__ << ":exiting...");
}

std::pair<bool, StatusType> ClientPool::isEnabled()
{
  std::unique_lock<std::mutex> lock(_mutex);
  return _poolStatus.isEnabled();
}

void ClientPool::setStatus(StatusType status)
{
  std::unique_lock<std::mutex> lock(_mutex);
  if (_poolStatus.set(status) && !_poolStatus.isEnabled().first)
  {
    for (auto request : _requestQueue)
    {
      request->cancel();
    }
    _requestQueue.clear();
    for (auto client : _clients)
    {
      client->cancel();
    }
    _condition.notify_all();
  }
}

bool ClientPool::enqueue(RCRequestPtr request,
                         bool healthcheck)
{
  if (!_stopped)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_poolStatus.isEnabled().first)
    {
      ++_numberRequests;
      ++_sample;
      _totalQueue += _requestQueue.size();
      if (clientAvailable(healthcheck))
      {
        _requestQueue.push_back(request);
        _condition.notify_one();
        return true;
      }
    }
  }
  request->cancel();
  return false;
}

RCRequestPtr ClientPool::getRequest()
{
  RCRequestPtr request;
  unsigned idleTime(ReadConfig::getIdleSlaveTimeout());
  std::cv_status status(std::cv_status::no_timeout);
  auto exp(std::chrono::steady_clock::now() + std::chrono::seconds(idleTime));
  std::unique_lock<std::mutex> lock(_mutex);
  try
  {
    while (_requestQueue.empty() && std::cv_status::no_timeout == status && !_stopped)
    {
      status = _condition.wait_until(lock, exp);
    }
  }
  catch (...)
  {
    // ignore
  }
  if (std::cv_status::timeout == status)
  {
    LOG4CXX_INFO(logger, __FUNCTION__ << ":slave idle timeout");
    return request;
  }
  if (!_requestQueue.empty())
  {
    request = _requestQueue.front();
    _requestQueue.pop_front();
  }
  return request;
}

size_t ClientPool::getNumberClients()
{
  std::size_t size(0);
  for (auto it(_clients.begin()); it != _clients.end(); )
  {
    if ((*it)->stopped())
    {
      it = _clients.erase(it);
    }
    else
    {
      if (!(*it)->willStop())
      {
        ++size;
      }
      ++it;
    }
  }
  return size;
}

bool ClientPool::clientAvailable(bool healthcheck)
{
  unsigned maxNumberClients(ReadConfig::getMaxNumberClients());
  double ratio(ReadConfig::getMinClientsRatio());
  unsigned minNumberClients(std::max(std::lround(maxNumberClients * ratio), 1L));
  double queueTolerance(ReadConfig::getQueueTolerance());
  unsigned numberClients(getNumberClients());
  if (healthcheck)
  {
    if (numberClients < maxNumberClients)
    {
      if (!_requestQueue.empty() || numberClients < minNumberClients)
      {
        _clients.emplace_back(new Client(this));
      }
      return true;
    }
    else
    {
      return false;
    }
  }
  if (numberClients < minNumberClients)
  {
    _clients.emplace_back(new Client(this));
    return true;
  }
  unsigned threshold(std::lround(numberClients * queueTolerance));
  if (_requestQueue.size() + 1 > threshold)
  {
    if (numberClients < maxNumberClients)
    {
      _clients.emplace_back(new Client(this));
      return true;
    }
    else
    {
      // throttle
      return false;
    }
  }
  std::size_t samplingInterval(ReadConfig::getClientPoolSamplingInterval());
  int adjustInterval(ReadConfig::getClientPoolAdjustInterval());
  bool adjust(false);
  if (adjustInterval > 0)
  {
    std::time_t currentTime(std::time(nullptr));
    if ((adjust = std::difftime(currentTime, _lastAdjustment) > adjustInterval))
    {
      _lastAdjustment = currentTime;
    }
  }
  else if (samplingInterval)
  {
    adjust = _sample == samplingInterval;
  }
  if (adjust)
  {
    double average(static_cast<double>(_totalQueue) / _sample);
    if (numberClients > minNumberClients && average < threshold - .5)// hist
    {
      _clients.back()->stop();
    }
    LOG4CXX_WARN(logger, _host << ':' << _port
                 << ",requests=" << _numberRequests
                 << ",queue=" << _requestQueue.size()
                 << ",average=" << average << ",active clients=" << numberClients
                 << ",all clients=" << _clients.size()
                 << ",maxClients=" << maxNumberClients
                 << ",threshold=" << threshold);
    _totalQueue = 0;
    _sample = 0;
  }
  return !_clients.empty();
}

void ClientPool::stop()
{
  try
  {
    bool joinable(false);
    {
      std::unique_lock<std::mutex> lock(_healthcheckMutex);
      _stopped = true;
      joinable = _healthcheckThread.joinable();
      _healthcheckCondition.notify_one();
    }
    if (joinable)
    {
      _healthcheckThread.join();
    }
    LOG4CXX_WARN(logger, __FUNCTION__ << ":healthcheckThread joined");
    std::unique_lock<std::mutex> lock(_mutex);
    LOG4CXX_WARN(logger, __FUNCTION__ << ":_clients.size()=" << _clients.size());
    for (auto client : _clients)
    {
      client->cancel();
    }
    while (!_clients.empty())
    {
      for (auto it(_clients.begin()); it != _clients.end(); )
      {
        if ((*it)->stopped())
        {
          it = _clients.erase(it);
        }
        else
        {
          ++it;
        }
      }
      _condition.notify_all();
      _condition.wait_for(lock, std::chrono::milliseconds(10));
    }
    LOG4CXX_WARN(logger, __FUNCTION__ << ":exiting,_clients.size()=" << _clients.size());
  }
  catch (...)
  {
    // ignore
  }
}

bool ClientPool::needHealthcheck()
{
  bool asyncHealthcheck(ReadConfig::getAsynchronousHealthcheck());
  long period(ReadConfig::getHealthcheckPeriod());
  if (period > 0)
  {
    std::time_t now(std::time(nullptr));
    if (std::difftime(now, _lastHealthcheck) > period)
    {
      _lastHealthcheck = now;
      if (asyncHealthcheck)
      {
        _needHealthcheck = true;
        _healthcheckCondition.notify_one();
        return false;
      }
      return true;
    }
  }
  bool healthcheckEnabled(ReadConfig::isHealthcheckEnabled());
  if (healthcheckEnabled)
  {
    std::pair<bool, StatusType> enabled(isEnabled());
    bool need(enabled.first ? _poolStatus.getNeedHealthcheck() : false);
    if (need && asyncHealthcheck)
    {
      _needHealthcheck = true;
      _healthcheckCondition.notify_one();
    }
    return need;
  }
  return false;
}

bool ClientPool::healthcheck(std::string& msg)
{
  bool result(false);
  RCRequestHealthcheckPtr request(RCRequestHealthcheck::createRequest());
  if (!request)
  {
    return false;
  }
  try
  {
    std::ostringstream output;
    if (enqueue(request, true))
    {
      request->wait();
      StatusType status(request->_responseHeader._status);
      if (RC_COMPRESSED_VALUE == status || RC_UNCOMPRESSED_VALUE == status)
      {
        output << _host << ':' << _port
               << " success:" << statusToString(status);
        _poolStatus.setNeedHealthcheck(false);
        result = true;
      }
      else
      {
        std::pair<bool, StatusType> enabled(_poolStatus.isEnabled());
        std::string failure;
        if (enabled.first)
        {
          failure.assign(statusToString(status));
        }
        else
        {
          failure.assign(statusToString(enabled.second));
        }
        output << _host << ':' << _port
               << " failed:" << failure;
        if (RC_CLIENT_PROCESSING_TIMEOUT == status)
        {
          _poolStatus.set(RC_HEALTHCHECK_TIMEOUT);
        }
      }
    }
    else if (_poolStatus.isEnabled().first)
    {
      _poolStatus.set(RC_QUEUE_LIMIT_EXCEEDED);
      output << _host << ':' << _port
             << " failed:" << statusToString(RC_QUEUE_LIMIT_EXCEEDED);
    }
    msg.assign(output.str());
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << ":exception");
    msg.assign(" exception");
  }
  request->cancel();
  return result;
}

}// RemoteCache

}// tse
