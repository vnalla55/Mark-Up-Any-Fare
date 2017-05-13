#include "AppConsole/ReceiveThread.h"

#include "AppConsole/AppConsole.h"
#include "AppConsole/ProcessThread.h"
#include "ClientSocket/EOSocket.h"
#include "Util/BranchPrediction.h"

#include <boost/thread.hpp>

#include <algorithm>
#include <iostream>

#include <syslog.h>
#include <unistd.h>

namespace
{
void
alert(const char* msg)
{
  openlog("TseServer", LOG_PID, LOG_USER);
  syslog(LOG_CRIT, "CVGALERT: %s", msg);
  closelog();
}
}

namespace ac
{
ReceiveThread::ReceiveThread(AppConsole& appConsole, unsigned short port)
  : _appConsole(appConsole), _port(port), _executor(tse::TseThreadingConst::SCOPED_EXECUTOR_TASK, 0)
{
}

bool
ReceiveThread::start()
{
  if (_socket != nullptr)
    return false;

  if (_port <= 0)
    return false;

  _socket = new eo::Socket;

  if (!_socket->listen(_port))
  {
    delete _socket;
    _socket = nullptr;

    return false;
  }

  return true;
}

bool
ReceiveThread::stop()
{
  // the accept() call is not a cancellation point
  // so we have to do this fake connection to fall
  // out of accept()

  if (_socket != nullptr)
  {
    eo::Socket sock;
    sock.connect("127.0.0.1", _port);
  }

  if (_socket != nullptr)
  {
    delete _socket;
    _socket = nullptr;
  }

  return true;
}

void
ReceiveThread::run()
{
  if (_socket == nullptr)
    return;

  while (1)
  {
    if (UNLIKELY(_appConsole.state() == AppConsole::stopping || _appConsole.state() == AppConsole::stopped))
    {
      break;
    }

    int newSock = _socket->accept();

    if (UNLIKELY(_appConsole.state() == AppConsole::stopping || _appConsole.state() == AppConsole::stopped))
    {
      break;
    }

    if (UNLIKELY(newSock < 0))
      continue;
    std::string msg("AppConsole thread caught exception:");
    try
    {
      eo::Socket* clientSocket = new eo::Socket(newSock, &_socket->getClientSocket());
      tse::TseCallableTask* processThread(new ProcessThread(_appConsole, clientSocket));
      _executor.execute(processThread, true);
    }
    catch (boost::thread_interrupted&)
    {
      msg.append("interrupted");
      alert(msg.c_str());
    }
    catch (const std::exception& e)
    {
      msg.append(e.what());
      alert(msg.c_str());
    }
    catch (...)
    {
      msg.append("unknown exception");
      alert(msg.c_str());
    }
  } // lint !e429
}
}
