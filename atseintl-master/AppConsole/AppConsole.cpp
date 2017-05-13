
#include "AppConsole/AppConsole.h"

#include "AppConsole/ReceiveThread.h"

namespace ac
{
AppConsole::AppConsole() = default;

AppConsole::~AppConsole()
{
  AppConsole::stop();
}

bool
AppConsole::start(unsigned short port)
{
  // We're already running
  if (_state != stopped)
    return false;

  if (port > 0)
  {
    _state = starting;
    _receiveThread.reset(new ReceiveThread(*this, port));

    if (!_receiveThread->start())
    {
      _state = stopped;
      _receiveThread.reset();

      return false;
    }

    _state = running;
    _executor.execute(_receiveThread.get());
  }

  return true;
}

bool
AppConsole::stop()
{
  if (_state != running)
    return false;

  _state = stopping;

  if (_receiveThread)
    _receiveThread->stop();

  _state = stopped;

  return true;
}
}
