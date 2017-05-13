#include "DBAccess/RemoteCache/ASIOServer/ASIOServer.h"

#include "Allocator/TrxMalloc.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <list>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace tse
{

FIXEDFALLBACK_DECL(fallbackRCChanges);

namespace RemoteCache
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.ASIOServer");

std::mutex _mutex;
std::condition_variable _condition;
bool _ready(false);
std::shared_ptr<ASIOServer> _instance;

struct ServerRunnable
{
  ServerRunnable(bool persistent,
                 int threadPoolSize)
    : _persistent(persistent)
    , _threadPoolSize(threadPoolSize)
  {
  }

  void operator()()
  {
    if (fallback::fixed::fallbackRCChanges())
    {
      _instance.reset(new ASIOServer(_threadPoolSize));
    }
    else
    {
      _instance.reset(_persistent ? new ASIOServer : new ASIOServer(_threadPoolSize));
    }
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _ready = true;
    }
    _condition.notify_all();
    // run the Server until stopped
    _instance->run();
  }
  bool _persistent{false};
  int _threadPoolSize{0};
};

}// namespace

boost::thread ASIOServer::_serverThread;

ASIOServer::ASIOServer()// persistent
  : _threadPoolSize(0)
  , _ioServicePtr(new boost::asio::io_service)
  , _acceptor(*_ioServicePtr)
{
  constructorBody();
}

ASIOServer::ASIOServer(int threadPoolSize)
  : _threadPoolSize(threadPoolSize)
  , _ioServicePool(new IOServicePool(threadPoolSize))
  , _ioServicePtr(_ioServicePool->getIOService())
  , _acceptor(*_ioServicePtr)
{
  constructorBody();
}

void ASIOServer::constructorBody()
{
  const std::string& port(ReadConfig::getServerPort());
  if (port.empty())
  {
    return;
  }
  try
  {
    // open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR)
    boost::asio::ip::tcp::resolver resolver(_acceptor.get_io_service());
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), port);
    boost::asio::ip::tcp::endpoint endpoint(*resolver.resolve(query));
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    if (ReadConfig::isKeepAlive())
    {
      _acceptor.set_option(boost::asio::socket_base::keep_alive(true));
    }
    _acceptor.bind(endpoint);
    _acceptor.listen();

    startAccept();
    LOG4CXX_INFO(logger, __FUNCTION__ << " RemoteCache server listening on port " << port);
  }
  catch (...)
  {
    LOG4CXX_ERROR(logger, __FUNCTION__ << " Failed to start RemoteCache server");
  }
}
bool ASIOServer::start()
{
  const MallocContextDisabler disableCustomAllocator;
  std::string port(ReadConfig::getServerPort());
  if (ReadConfig::isEnabled() && !port.empty() && !_instance)
  {
    if (ReadConfig::usePersistentConnections())
    {
      ServerRunnable runnable(true, 1);
      boost::thread(runnable).swap(_serverThread);
    }
    else
    {
      int threadPoolSize(ReadConfig::getThreadPoolSize());
      ServerRunnable runnable(false, threadPoolSize);
      boost::thread(runnable).swap(_serverThread);
    }
    LOG4CXX_DEBUG(logger, __FUNCTION__);
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_ready)
    {
      _condition.wait(lock);
    }
    return _instance ? true : false;
  }
  return false;
}

void ASIOServer::run()
{
  const MallocContextDisabler disableCustomAllocator;
  if (_ioServicePool)
  {
    _ioServicePool->run();
  }
  else if (_ioServicePtr)// persistent
  {
    try
    {
      boost::system::error_code e;
      _ioServicePtr->run(e);
      if (boost::system::errc::success != e.value())
      {
        LOG4CXX_WARN(logger, __FUNCTION__ << ':' << e.message());
      }
    }
    catch (...)
    {
      LOG4CXX_ERROR(logger, __FUNCTION__ << " UNKNOWN");
    }
  }
}

namespace
{
typedef std::list<SessionPtr> Sessions;
Sessions sessions;
std::mutex sessionMutex;

SessionPtr getSession()
{
  {
    std::unique_lock<std::mutex> lock(sessionMutex);
    for (auto it(sessions.begin()); it != sessions.end(); )
    {
      if ((*it)->stopped())
      {
        it = sessions.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }
  SessionPtr session(new Session);
  return session;
}

void handleStop(IOServicePtr ioService)
{
  if (ioService && !ioService->stopped())
  {
    ioService->stop();
  }
}

}// namespace

void ASIOServer::stop()
{
  const MallocContextDisabler disableCustomAllocator;
  if (_instance)
  {
    _instance->_stopped = true;
    if (_instance->_ioServicePool)
    {
      _instance->_ioServicePool->stop();
    }
    else// persistent
    {
      auto servicePtr(_instance->_ioServicePtr);
      if (servicePtr)
      {
        servicePtr->dispatch(boost::bind(&handleStop, servicePtr));
      }
    }
    if (_serverThread.joinable())
    {
      _serverThread.join();
    }
    std::unique_lock<std::mutex> lock(sessionMutex);
    for (auto& session : sessions)
    {
      session->stop();
    }
    while (!sessions.empty())
    {
      for (auto it(sessions.begin()); it != sessions.end(); )
      {
        if ((*it)->stopped())
        {
          it = sessions.erase(it);
        }
        else
        {
          ++it;
        }
      }
      _condition.wait_for(lock, std::chrono::milliseconds(10));
    }
    _instance.reset();
    LOG4CXX_INFO(logger, __FUNCTION__ << " RemoteCache server stopped,"
                 << Session::getNumberSessions() << " sessions,"
                 << "sessions.size()=" << sessions.size());
  }
}

void ASIOServer::startAccept()
{
  const MallocContextDisabler disableCustomAllocator;
  bool persistent(ReadConfig::usePersistentConnections());
  if (persistent)
  {
    SessionPtr session(getSession());
    _acceptor.async_accept(
      session->socket(),
      boost::bind(&ASIOServer::handleAcceptPersistent,
                  this,
                  session,
                  boost::asio::placeholders::error));
  }
  else if (_ioServicePool)
  {
    _newConnection.reset(new Connection(_ioServicePool->getIOService()));
    _acceptor.async_accept(
      _newConnection->socket(),
      boost::bind(&ASIOServer::handleAccept, this, boost::asio::placeholders::error));
  }
}

void ASIOServer::handleAccept(const boost::system::error_code& err)
{
  if (!err)
  {
    _newConnection->start();
  }
  if (!_stopped)
  {
    startAccept();
  }
}

void ASIOServer::handleAcceptPersistent(SessionPtr session,
                                        const boost::system::error_code& e)
{
  if (!e)
  {
    session->start();
  }
  {
    std::unique_lock<std::mutex> lock(sessionMutex);
    sessions.push_back(session);
  }
  if (!_stopped)
  {
    startAccept();
  }
}

}// RemoteCache

}// tse
