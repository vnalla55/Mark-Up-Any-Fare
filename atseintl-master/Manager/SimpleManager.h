//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/Logger.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Manager/Manager.h"
#include "Server/TseServer.h"

#include <string>

namespace tse
{
template <class ThreadType>
class SimpleManager : public Manager
{
public:
  SimpleManager(const std::string& name, TseServer& srv)
    : Manager(name, srv.config()),
      _srv(srv),
      _threadTask(srv, name),
      _scopedExecutor(TseThreadingConst::SCOPED_EXECUTOR_TASK, 1)
  {
  }

  virtual ~SimpleManager()
  {
    LOG4CXX_DEBUG(_logger, _name + " destructing");

    LOG4CXX_DEBUG(_logger, _name + " trying to shutdown thread.");
    try
    {
      _threadTask.finish();
      _scopedExecutor.wait();
    }
    catch (...)
    {
      // do nothing - we're exiting anyway
    }

    LOG4CXX_DEBUG(_logger, _name + " destructed");
  }

  const TseServer& server() const { return _srv; }

  virtual bool initialize(int argc, char* argv[]) override
  {
    if (!_threadTask.initialize(argc, argv))
    {
      LOG4CXX_ERROR(_logger, "Unable to initalize " + _name + " Thread");
      return false;
    }

    return true;
  }

  virtual void postInitialize() override { _scopedExecutor.execute(_threadTask); }

  virtual void preShutdown() override
  {
    LOG4CXX_DEBUG(_logger, _name + " preparing for destruction");

    LOG4CXX_DEBUG(_logger, _name + " trying to shutdown thread.");
    try
    {
      _threadTask.finish();
      _scopedExecutor.wait();
    }
    catch (...)
    {
      // do nothing - we're exiting anyway
    }

    LOG4CXX_DEBUG(_logger, _name + " prepared for destruction");
  }

private:
  TseServer& _srv;
  ThreadType _threadTask;
  TseScopedExecutor _scopedExecutor;
  static Logger _logger;
};
} // end namespace tse

