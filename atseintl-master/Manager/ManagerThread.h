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

#include "Common/Config/ConfigurableValue.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/Thread/TseCallableTask.h"
#include "Server/TseServer.h"

#include <exception>
#include <string>

namespace tse
{
struct ManagerThreadConfig
{
  ManagerThreadConfig(const std::string& name)
    : adapterName(name, "SERVICE_ADAPTER"), serviceName(name, "SERVICE_NAME")
  {
  }

  ConfigurableValue<std::string> adapterName;
  ConfigurableValue<std::string> serviceName;
};

template <class AdapterType>
class ManagerThread : public TseCallableTask
{
  ManagerThread(const ManagerThread& rhs) = delete;
  ManagerThread& operator=(const ManagerThread& rhs) = delete;

private:
  const std::string _name;
  TseServer& _srv;
  ConfigMan& _config;
  AdapterType* _adapter;
  Service* _service;

  static Logger _logger;
  static ManagerThreadConfig _configVariables;

public:
  ManagerThread(TseServer& srv, const std::string& name)
    : _name(name), _srv(srv), _config(srv.config()), _adapter(nullptr), _service(nullptr)
  {
  }

  virtual void run() override
  {
    if (_service == nullptr)
    {
      LOG4CXX_ERROR(_logger, "Service pointer is null");
      return; // failure
    }

    try
    {
      LOG4CXX_DEBUG(_logger, _name + " calling run");

      _adapter->run(*_service);
    }
    catch (ErrorResponseException& ere)
    {
      LOG4CXX_FATAL(_logger, _name + " ErrorResponseException - " << ere.what());
    }
    catch (std::exception& e)
    {
      LOG4CXX_FATAL(_logger, _name + " std::exception - " << e.what());
    }
    catch (...)
    {
      LOG4CXX_FATAL(_logger, _name + " excepted");
    }

    LOG4CXX_DEBUG(_logger, _name + " exiting");
  }

  virtual bool initialize(int argc, char* argv[])
  {
    // Get the adapter
    const std::string adapterName = _configVariables.adapterName.getValue();

    _adapter = static_cast<AdapterType*>(_srv.adapter(adapterName));

    if (_adapter == nullptr)
    {
      LOG4CXX_ERROR(_logger, "Couldnt retrieve Adapter '" << adapterName << "' from server");
      return false; // failure
    }

    // Get the service
    const std::string serviceName = _configVariables.serviceName.getValue();

    _service = _srv.service(serviceName);
    if (_service == nullptr)
    {
      LOG4CXX_ERROR(_logger, "Couldnt retrieve Service '" << serviceName << "' from server");
      return false; // failure
    }

    return true;
  }

  virtual void finish()
  {
    LOG4CXX_DEBUG(_logger, _name + " calling Adapter::shutdown");
    _adapter->shutdown();
  }

}; // End class ManagerThread
} // End namespace tse

