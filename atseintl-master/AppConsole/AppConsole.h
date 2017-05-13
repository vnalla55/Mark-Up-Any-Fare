//-------------------------------------------------------------------
//
//  File:        AppConsole.h
//  Created:     Jan 12, 2005
//  Authors:     Mark Kasprowicz
//
//  Description: Command and Control Interface
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "AppConsole/SocketUtils.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/Thread/TseThreadingConst.h"

#include <memory>

namespace ac
{
class ReceiveThread;

class AppConsole
{
public:
  enum State
  { starting = 0,
    running,
    stopping,
    stopped };

  AppConsole();
  AppConsole(const AppConsole&) = delete;
  AppConsole& operator=(const AppConsole&) = delete;

  virtual ~AppConsole();

  bool start(unsigned short port);
  bool stop();

  State state() { return _state; }

  virtual bool processMessage(const SocketUtils::Message& req, SocketUtils::Message& rsp) = 0;

private:
  std::unique_ptr<ReceiveThread> _receiveThread;
  volatile State _state = State::stopped;
  tse::TseScopedExecutor _executor{tse::TseThreadingConst::SCOPED_EXECUTOR_TASK, 1};
}; // End class
} // End namespace
