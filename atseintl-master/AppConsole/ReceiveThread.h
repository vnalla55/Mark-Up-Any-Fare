//-------------------------------------------------------------------
//
//  File:        ReceiveThread.h
//  Created:     May 12, 2005
//  Authors:     Mark Kasprowicz
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

#include "Common/Thread/TseCallableTask.h"
#include "Common/Thread/TseScopedExecutor.h"

#include <vector>

namespace eo
{
class Socket;
}

namespace ac
{
class AppConsole;

class ReceiveThread final : public tse::TseCallableTask
{
public:
  ReceiveThread(AppConsole& appConsole, unsigned short port);
  ReceiveThread(const ReceiveThread&) = delete;
  ReceiveThread& operator=(const ReceiveThread&) = delete;

  bool start();
  bool stop();
  void run() override;

private:
  AppConsole& _appConsole;
  const unsigned short _port;
  eo::Socket* _socket = nullptr;
  tse::TseScopedExecutor _executor;
};
} // End namespace
