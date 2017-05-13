//----------------------------------------------------------------------------
//
//  File:        ServerSocketManager.cpp
//
//  Description: This class is basically a loadable wrapper that exists just
//               to instantiate a ServiceThreadPool
//
//  Copyright Sabre 2003
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

#include "Manager/ServerSocketManager.h"

#include "Adapter/ServerSocketAdapter.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Manager/TseManagerUtil.h"
#include "Server/TseServer.h"
#include "Service/Service.h"

#include <unistd.h>

#include <functional>

namespace tse
{
namespace
{
Logger
logger("atseintl.Manager.ServerSocketManager");

struct ServerSocketManagerConfigurableValues
{
  std::string serviceName;
  ConfigurableValue<std::string> adapterName;
  ConfigurableValue<std::string> serviceNameCfg;
  ConfigurableValue<std::string> xformName;
  ConfigurableValue<std::string> iorFile;
  ConfigurableValue<uint32_t> maxTransactions;
  ConfigurableValue<uint32_t> transactionVariance;
  ConfigurableValue<uint32_t> maxVirtualMemory;
  ConfigurableValue<uint32_t> virtualMemoryVariance;
  ConfigurableValue<int> minAvailableMemory;
  ConfigurableValue<uint32_t> availableMemoryVariance;
  ConfigurableValue<uint32_t> maxRSSPercentage;
  ConfigurableValue<uint32_t> maxTransactionWait;
  ConfigurableValue<uint32_t> maxTransactionRetryTime;
  ConfigurableValue<uint32_t> maxBigIpDeregistrationWait;
  ConfigurableValue<uint32_t> maxBigIpDeregistrationRetryTime;

  ServerSocketManagerConfigurableValues(const std::string& name)
    : serviceName(name),
      adapterName(serviceName, "SERVICE_ADAPTER"),
      serviceNameCfg(serviceName, "SERVICE_NAME"),
      xformName(serviceName, "XFORM_NAME"),
      iorFile(serviceName, "IOR_FILE"),
      maxTransactions(serviceName, "MAX_TRANSACTIONS"),
      transactionVariance(serviceName, "MAX_TRANSACTION_VARIANCE"),
      maxVirtualMemory(serviceName, "MAX_VIRTUAL_MEMORY"),
      virtualMemoryVariance(serviceName, "MAX_VIRTUAL_MEMORY_VARIANCE"),
      minAvailableMemory(serviceName, "MIN_AVAILABLE_MEMORY", -1),
      availableMemoryVariance(serviceName, "MIN_AVAILABLE_MEMORY_VARIANCE"),
      maxRSSPercentage(serviceName, "MAX_RSS_PERCENTAGE", 100),
      maxTransactionWait(serviceName, "MAX_TRANSACTION_WAIT"),
      maxTransactionRetryTime(serviceName, "MAX_TRANSACTION_SHUTDOWN_RETRY_TIME"),
      maxBigIpDeregistrationWait(serviceName, "MAX_BIGIP_DEREGISTRATION_WAIT_TIME"),
      maxBigIpDeregistrationRetryTime(serviceName, "MAX_BIGIP_DEREGISTRATION_RETRY_TIME")
  {
  }
} serverSocketManagerCvs("TO_MAN");

}

static LoadableModuleRegister<Manager, ServerSocketManager>
_("libServerSocketManager.so");

ServerSocketManager::ServerSocketManager(const std::string& name, tse::TseServer& srv)
  : Manager(name, srv.config()), _name(name)
{
}

ServerSocketManager::~ServerSocketManager()
{
  LOG4CXX_DEBUG(logger, "ServerSocketManager destructing");
  try
  {
    _shutdownHandler->finish();

    for (boost::thread& thread : _threads)
      thread.join();

    TseManagerUtil::deinitialize();
  }
  catch (...)
  {
    // Nothing to do
  }

  for (auto& elem : _threadTasks)
  {
    delete elem;
  }
  delete _shutdownHandler;
  LOG4CXX_DEBUG(logger, "ServerSocketManager destructed");
}

bool
ServerSocketManager::initialize(int argc, char* argv[])
{
  const MallocContextDisabler context;
  // Get the adapter
  const std::string adapterName = serverSocketManagerCvs.adapterName.getValue();
  if (serverSocketManagerCvs.adapterName.isDefault())
    return false;

  _adapter = (ServerSocketAdapter*)Global::adapter(adapterName);

  if (_adapter == nullptr)
  {
    LOG4CXX_FATAL(logger, "Couldnt retrieve Adapter '" << adapterName << "' from server");
    return false; // failure
  }

  // Get the service
  const std::string serviceName = serverSocketManagerCvs.serviceNameCfg.getValue();
  if (serverSocketManagerCvs.serviceNameCfg.isDefault())
    return false;

  _service = Global::service(serviceName);
  if (_service == nullptr)
  {
    LOG4CXX_FATAL(logger, "Couldnt retrieve Service '" << serviceName << "' from server");
    return false; // failure
  }

  // Get the Xform
  const std::string xformName = serverSocketManagerCvs.xformName.getValue();
  if (serverSocketManagerCvs.xformName.isDefault())
    return false;
  _xform = Global::xform(xformName);

  if (_xform == nullptr)
  {
    LOG4CXX_FATAL(logger, "Couldnt retrieve Service '" << serviceName << "' from server");
    return false; // failure
  }

  // If they have a defined IOR for us, write it out for the landing zone to use
  const std::string iorFile = serverSocketManagerCvs.iorFile.getValue();
  _adapter->setIOR(iorFile);
  if (!initializeShutdownHandler())
    return false;

  // Initialize configured number of threads
  const uint32_t numListeners = _adapter->getNumListeners();
  for (uint32_t i = 0; i < numListeners; i++)
  {
    ServerSocketThread* t = new ServerSocketThread(_name, i);
    _threadTasks.push_back(t);

    if (!t->initialize(_adapter, _service, _xform, _shutdownHandler))
    {
      LOG4CXX_FATAL(logger,
                    "Unable to initalize ServerSocketThread"
                        << " for the index [" << i << "]");
      return false;
    }
  }

  return true;
}

bool
ServerSocketManager::initializeShutdownHandler()
{
  // Get restart limits from the config
  uint32_t maxTransactions = serverSocketManagerCvs.maxTransactions.getValue();
  uint32_t maxVirtualMemory = serverSocketManagerCvs.maxVirtualMemory.getValue();
  int minAvailableMemory = serverSocketManagerCvs.minAvailableMemory.getValue();
  const unsigned maxRSSPercentage = serverSocketManagerCvs.maxRSSPercentage.getValue();
  const uint32_t maxTransactionWait = serverSocketManagerCvs.maxTransactionWait.getValue();
  const uint32_t maxTransactionRetryTime =
      serverSocketManagerCvs.maxTransactionRetryTime.getValue();
  const uint32_t transactionVariance = serverSocketManagerCvs.transactionVariance.getValue();

  if (maxTransactions > 0)
  {
    // determine max possible variance based on percentage of the
    // transaction limit
    uint32_t lim = (maxTransactions * transactionVariance) / 100;

    if (lim)
    {
      // initialize random number generator
      srand(unsigned(time(nullptr)));

      // get the random number and add it to the fixed limit
      maxTransactions += (rand() % lim) + 1;
    }
    LOG4CXX_INFO(logger,
                 "Server configured to shut down after " << maxTransactions << " transactions.");
  }

  if (maxVirtualMemory > 0)
  {
    // determine max possible variance based on percentage of the
    // transaction limit
    const uint32_t lim =
        (maxVirtualMemory * serverSocketManagerCvs.virtualMemoryVariance.getValue()) / 100;

    if (lim)
    {
      // initialize random number generator
      srand(unsigned(time(nullptr)));

      // get the random number and add it to the fixed limit
      maxVirtualMemory += (rand() % lim) + 1;
    }
    LOG4CXX_INFO(logger, "Server configured to shut down after " << maxVirtualMemory << "MB.");
  }

  if (minAvailableMemory > 0)
  {
    const uint32_t lim(minAvailableMemory *
                       serverSocketManagerCvs.availableMemoryVariance.getValue() / 100);
    if (lim > 0)
    {
      srand(unsigned(time(nullptr)));
      minAvailableMemory += rand() % lim + 1;
    }
    LOG4CXX_INFO(logger,
                 "Server configured to shut down if minAvailableMemory < " << minAvailableMemory
                                                                           << "MB");
    // std::cerr << "minAvailableMemory=" << minAvailableMemory << std::endl;
  }

  if (maxTransactionWait < maxTransactionRetryTime)
    LOG4CXX_ERROR(logger,
                  "Inconsistency between MAX_TRANSACTION_WAIT and "
                  "MAX_TRANSACTION_SHUTDOWN_RETRY_TIME in the config file");

  const uint32_t maxBigIpDeregistrationWait =
      serverSocketManagerCvs.maxBigIpDeregistrationWait.getValue();
  const uint32_t maxBigIpDeregistrationRetryTime =
      serverSocketManagerCvs.maxBigIpDeregistrationRetryTime.getValue();
  TSE_ASSERT(_service);
  const std::string executableName = _service->server().getExecutableName();

  _shutdownHandler = new ServerSocketShutdownHandler(_adapter,
                                                     executableName,
                                                     maxTransactions,
                                                     maxVirtualMemory,
                                                     minAvailableMemory,
                                                     maxRSSPercentage,
                                                     maxTransactionWait,
                                                     maxTransactionRetryTime,
                                                     maxBigIpDeregistrationWait,
                                                     maxBigIpDeregistrationRetryTime);

  return true;
}

namespace
{
void
ignoreAllExceptions(TseCallableTask& task)
{
  try
  {
    task.run();
  }
  catch (...)
  {
    // Ignore
  }
}
}

void
ServerSocketManager::postInitialize()
{
  ServerSocketThread::warmServer(*_xform, *_service);

  for (uint32_t i = 0; i < _threadTasks.size(); i++)
  {
    LOG4CXX_DEBUG(logger, "Executing Thread with index [" << i << "]");
    _threads.emplace_back(std::bind(ignoreAllExceptions, std::ref(*_threadTasks[i])));
  }
  // TODO -- do we need sleep a second to make sure that all the sockets
  // are opened before generating IOR file?
  _adapter->generateIOR();
}

void
ServerSocketManager::preShutdown()
{
  const MallocContextDisabler context;

  LOG4CXX_DEBUG(logger, "ServerSocketManager preparing for shutdown");

  try
  {
    _shutdownHandler->finish();
    _adapter->shutdown();

    for (boost::thread& thread : _threads)
      thread.interrupt();
  }
  catch (...)
  {
    // Nothing to do
  }

  LOG4CXX_DEBUG(logger, "ServerSocketManager prepared for shutdown");
}
}
