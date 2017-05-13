//-------------------------------------------------------------------
//
//  File:        ProcessThread.h
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

namespace eo
{
class Socket;
}

namespace ac
{
class AppConsole;

class ProcessThread : public tse::TseCallableTask
{
public:
  ProcessThread(AppConsole& appConsole, eo::Socket* socket)
    : _appConsole(appConsole), _socket(socket)
  {
  }

  ProcessThread(const ProcessThread&) = delete;
  ProcessThread& operator=(const ProcessThread&) = delete;

  ~ProcessThread();

  void run() override;

private:
  AppConsole& _appConsole;
  eo::Socket* _socket = nullptr;
};
} // End namespace
