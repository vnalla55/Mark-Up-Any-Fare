#include "AppConsole/ProcessThread.h"

#include "AppConsole/AppConsole.h"
#include "AppConsole/SocketUtils.h"
#include "ClientSocket/EOSocket.h"
#include "Util/BranchPrediction.h"

#include <unistd.h>

namespace
{
const int _defaultLinger = 5; // 5 seconds
}

namespace ac
{
ProcessThread::~ProcessThread()
{
  if (LIKELY(_socket != nullptr))
  {
    delete _socket;
    _socket = nullptr;
  }
}

void
ProcessThread::run()
{
  if (UNLIKELY(_socket == nullptr))
    return;

  if (LIKELY(_defaultLinger > 0)) // lint !e506
    _socket->setLinger(true, _defaultLinger);

  while (1)
  {
    SocketUtils::Message req;
    SocketUtils::Message rsp;

    if (UNLIKELY(_appConsole.state() == AppConsole::stopping || _appConsole.state() == AppConsole::stopped))
    {
      break;
    }

    if (!SocketUtils::readMessage(*_socket, req))
      break;

    // Once we've received the message go ahead and process
    // the result before checking for shutdown
    if (UNLIKELY(!_appConsole.processMessage(req, rsp)))
      break;

    if (UNLIKELY(!SocketUtils::writeMessage(*_socket, rsp)))
      break;
  }
}
}
