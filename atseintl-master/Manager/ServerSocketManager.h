//----------------------------------------------------------------------------
//
//  File:               ServerSocketManager.h
//
//  Description:        A manager that creates a thread
//                      to process client socket requests
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/Config/ConfigurableValue.h"
#include "Manager/Manager.h"
#include "Manager/ServerSocketThread.h"

#include <boost/thread.hpp>

#include <string>
#include <vector>

namespace tse
{
class TseServer;
class ServerSocketManager;
class ServerSocketAdapter;

class ServerSocketManager : public Manager
{
public:
  ServerSocketManager(const std::string& name, TseServer& srv);
  virtual ~ServerSocketManager();

private:
  const std::string _name;
  ServerSocketAdapter* _adapter = nullptr;
  Service* _service = nullptr;
  Xform* _xform = nullptr;
  ServerSocketShutdownHandler* _shutdownHandler = nullptr;
  std::vector<boost::thread> _threads;

  std::vector<ServerSocketThread*> _threadTasks;

public:
  virtual bool initialize(int argc = 0, char* argv[] = nullptr) override;
  virtual void postInitialize() override;
  virtual void preShutdown() override;

private:
  bool initializeShutdownHandler();
}; // End class ServerSocketManager
} // end namespace tse
