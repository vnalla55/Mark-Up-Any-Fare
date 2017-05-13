//----------------------------------------------------------------------------
//
//  File:               ServerSocketThread.cpp
//  Description:        Thread dedicated to process incoming client request
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

#include "Manager/ServerSocketThread.h"

#include "Adapter/ServerSocketAdapter.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/Thread/TseTransactionExecutor.h"
#include "Common/TrxUtil.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "Manager/SocketRequestImpl.h"
#include "Manager/TseManagerUtil.h"

#include <ZThreads/zthread/CountingSemaphore.h>
#include <ZThreads/zthread/Exceptions.h>

#include <cstring>
#include <exception>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

namespace tse
{
namespace
{
Logger logger("atseintl.Manager.ServerSocketThread");
// TODO - may want to use the ServerSocketThread logger because of the scripts
// referencing these messages
Logger shutdownLogger("atseintl.Manager.ServerSocketShutdownHandler");
Logger profilerLogger("atseintl.Manager.Profiler");

// class which manages profiling of the application using the
// Google Performance Tools. InitProfiler() checks settings to make sure the
// profiler is enabled, and if it is, returns a pointer to the profiler. Then,
// every time a transaction is received, processTrx() will update the profiler
// if necessary.
//
// The profiler is only used if the following three environment variables are
// set:
//
// GOOGLE_PROFILE_FILENAME: the path to write the profile to
// GOOGLE_PROFILE_START: the transaction number to begin profiling on.
// GOOGLE_PROFILE_STOP: the transaction number to finish profiling with.
//
// The libprofiler.so library will be opened.  If it is not present, then
// the profiler won't be used.
class ProfilerInfo
{
  typedef int (*ProfilerStartFn)(const char*);
  typedef void (*ProfilerStopFn)();
  std::string _filename; // filename to output to.
  int _trxCount; // number of transactions we've seen.
  int _startAt, _stopAt; // which transactions to start/stop profiling at.
  ProfilerStartFn _profilerStart; // function pointer to start profiling.
  ProfilerStopFn _profilerStop; // function pointer to stop profiling.

public:
  static ProfilerInfo* InitProfiler()
  {
    const char* filename = getenv("GOOGLE_PROFILE_FILENAME");
    const char* beginProf = getenv("GOOGLE_PROFILE_START");
    const char* endProf = getenv("GOOGLE_PROFILE_STOP");
    if (!filename || strlen(filename) == 0 || !beginProf || !endProf)
    {
      // if any environment variables aren't set, don't use the profiler.
      // this is the normal case for most runs, so we return without
      // a message.
      return nullptr;
    }

    // search for the profiler library.
    void* lib = dlopen("libprofiler.so", RTLD_LAZY | RTLD_LOCAL);
    if (!lib)
    {
      // the profiler couldn't be loaded, and the environment variables indicate
      // the user wanted to profile, so log an error.
      LOG4CXX_ERROR(profilerLogger, "Could not open libprofiler.so. Not profiling");
      return nullptr;
    }

    ProfilerStartFn profilerStart = (ProfilerStartFn)(dlsym(lib, "ProfilerStart"));
    ProfilerStopFn profilerStop = (ProfilerStopFn)(dlsym(lib, "ProfilerStop"));
    if (!profilerStart || !profilerStop)
    {
      // the functions we expected in the library couldn't be found, profiling can't be done.
      dlclose(lib);
      LOG4CXX_ERROR(profilerLogger, "Could not load functions from libprofiler.so. Not profiling");
      return nullptr;
    }

    // create the profiler object and return it. This object will persist for the life of
    // the application, so no reason to delete it.
    ProfilerInfo* info = new ProfilerInfo;
    info->_filename = filename;
    info->_trxCount = 0;
    info->_startAt = atoi(beginProf);
    info->_stopAt = atoi(endProf);
    info->_profilerStart = profilerStart;
    info->_profilerStop = profilerStop;
    LOG4CXX_WARN(profilerLogger, "Google Profiler initialized");
    return info;
  }

  void processTrx()
  {
    if (_trxCount == _startAt)
    {
      // we've reached the transaction we should begin profiling on, so start
      // the profiling.
      _profilerStart(_filename.c_str());
      LOG4CXX_WARN(profilerLogger, "Started Google Profile");
    }

    if (_trxCount == _stopAt)
    {
      // we've reached the transaction we should stop profiling on, so
      // terminate the profiling.
      _profilerStop();
      LOG4CXX_WARN(profilerLogger, "Finished Google Profile");
    }

    ++_trxCount;
  }
};
}

void
ServerSocketThread::run()
{
  LOG4CXX_INFO(logger, "ServerSocketThread run - index [" << _index << "]");

  // Initialize the profiler. Will be NULL if the profiler isn't going to
  // be used (the common/production case)
  ProfilerInfo* profiler = ProfilerInfo::InitProfiler();

  const MallocContextDisabler context;

  ZThread::CountingSemaphore* processedTransactionsCSp =
      _shutdownHandler->processedTransactionsCS();

  TseTransactionExecutor transactionExecutor;

  // Get Adapter values for IO_BUF_SIZE and TIMEOUT
  uint32_t ioBufSize = _adapter->getIOBufSize();
  uint16_t timeout = _adapter->getTimeout();
  eo::Socket* clientSocket = nullptr;
  tse::ConfigMan& config(Global::config());
  std::string type;
  if (!config.getValue("SERVER_TYPE", type, "APPLICATION_CONSOLE"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "SERVER_TYPE", "APPLICATION_CONSOLE");
  }
  int port(_adapter->getPort());
  char buffer[32] = "";
  sprintf(buffer, "%s%d", ",port=", port);
  std::string serverid("type:" + type + buffer);
  std::string msg(serverid + ":Thread accepting transactions caught exception:");
  while (!_shutdownHandler->exiting())
  {
    if (profiler)
    {
      // if we are profiling, then tell the profiler about this transaction.
      profiler->processTrx();
    }

    _shutdownHandler->checkShutdownThresholds();

    try
    {
      DateTime startTime;
      clientSocket = _adapter->run(_index, startTime);
      if (clientSocket == nullptr)
      {
        if (!_shutdownHandler->exiting())
        {
          msg.append("Errors obtaining client socket, shutting down adapter");
          _shutdownHandler->finish();
        }
      }
      else
      {
        SocketRequestImpl* socketRequestImpl(new SocketRequestImpl(startTime,
                                                                   clientSocket,
                                                                   *_service,
                                                                   *_xform,
                                                                   ioBufSize,
                                                                   timeout,
                                                                   nullptr,
                                                                   processedTransactionsCSp));
        transactionExecutor.execute(socketRequestImpl);
      }
    }
    catch (const ZThread::Synchronization_Exception& e)
    {
      msg.append(e.what());
      _shutdownHandler->finish();
    }
    catch (const boost::thread_interrupted&)
    {
      msg.append("interrupted");
      _shutdownHandler->finish();
    }
    catch (const std::exception& e)
    {
      msg.append(e.what());
      _shutdownHandler->finish();
    }
    catch (...)
    {
      msg.append("unknown exception");
      _shutdownHandler->finish();
    }
  }
  LOG4CXX_INFO(logger, "ServerSocketThread exiting");
  TseUtil::alert(msg.c_str());
  _adapter->shutdown();
  LOG4CXX_FATAL(logger, msg);
}

bool
ServerSocketThread::initialize(ServerSocketAdapter* adapter,
                               Service* service,
                               Xform* xform,
                               ServerSocketShutdownHandler* shutdownHandler)
{
  const MallocContextDisabler context;

  // Set the adapter
  _adapter = adapter;
  if (_adapter == nullptr)
  {
    LOG4CXX_FATAL(logger, "NULL Adapter specified");
    return false; // failure
  }

  // Set the service
  _service = service;
  if (_service == nullptr)
  {
    LOG4CXX_FATAL(logger, "NULL Service specified");
    return false; // failure
  }

  // Set the Xform
  _xform = xform;
  if (_xform == nullptr)
  {
    LOG4CXX_FATAL(logger, "NULL Xform specified");
    return false; // failure
  }

  // Set the shutdown Handler
  _shutdownHandler = shutdownHandler;
  if (_shutdownHandler == nullptr)
  {
    LOG4CXX_FATAL(logger, "NULL ShutdownHandler specified");
    return false; // failure
  }

  return true;
}

//----------------------------------------------------------------------------
// warmServer()
//----------------------------------------------------------------------------
void
ServerSocketThread::warmServer(Xform& xform, Service& service)
{
  tse::ConfigMan& config = Global::config();

  std::string dir;
  if (!config.getValue("WARMING_REQUESTS", dir, "TSE_SERVER"))
  {
    LOG4CXX_INFO(logger, "Could not find warming dir");
    return;
  }

  DIR* const dirHandle = opendir(dir.c_str());
  if (dirHandle == nullptr)
  {
    LOG4CXX_WARN(logger, "Could not open warming directory '" << dir << "'");
    return;
  }

  const std::string dirname = dir + "/";
  const std::string ext = ".xml";

  size_t nfiles = 0;

  while (dirent* entry = readdir(dirHandle))
  {
    const std::string fname = dirname + entry->d_name;
    if (fname.size() < ext.size() ||
        std::equal(ext.begin(), ext.end(), fname.end() - ext.size()) == false)
    {
      continue;
    }

    try
    {
      std::string request, response;
      std::ifstream input(fname.c_str());

      while (input.good() && !input.eof())
      {
        request.push_back(char(input.get()));
      }

      if (request.empty())
      {
        LOG4CXX_WARN(logger, "Could not read file '" << fname << "' to warm server");
        continue;
      }

      LOG4CXX_INFO(logger, "Warming server with '" << fname << "'");
      {
        DateTime startTime = boost::posix_time::microsec_clock::local_time();
        TseManagerUtil::TrxCounter trxCounter;
        TseManagerUtil::process(startTime, trxCounter, request, response, xform, service, false);
      }
    }
    catch (...)
    {
      LOG4CXX_WARN(logger, "Error while warming the server");
      continue;
    }

    ++nfiles;
  }

  closedir(dirHandle);

  LOG4CXX_INFO(logger, "Warmed the server with " << nfiles << " requests");
}

//=======================================================================
// ServerSocketShutdownHandler class implementation
//=======================================================================
ServerSocketShutdownHandler::ServerSocketShutdownHandler(
    ServerSocketAdapter* adapter,
    const std::string& executableName,
    const uint32_t maxTransactions,
    const uint32_t maxVirtualMemory,
    const int minAvailableMemory,
    const unsigned maxRSSPercentage,
    const uint32_t maxTransactionWait,
    const uint32_t maxTransactionRetryTime,
    const uint32_t maxBigIpDeregistrationWait,
    const uint32_t maxBigIpDeregistrationRetryTime)
  : _adapter(adapter),
    _maxTransactions(maxTransactions),
    _maxVirtualMemory(maxVirtualMemory),
    _minAvailableMemory(minAvailableMemory),
    _maxRSSPercentage(maxRSSPercentage),
    _maxTransactionWait(maxTransactionWait),
    _maxTransactionRetryTime(maxTransactionRetryTime),
    _maxBigIpDeregistrationWait(maxBigIpDeregistrationWait),
    _maxBigIpDeregistrationRetryTime(maxBigIpDeregistrationRetryTime),
    _processedTransactionsCS(maxTransactions)
{
  _bigIPExtDeregistrationTriggerFile = executableName + ".bigip_disable.trigger";
  _bigIPDeregistrationDoneTriggerFile = executableName + ".bigip_disable.complete";
}

void
ServerSocketShutdownHandler::checkTransactionLimit()
{
  if (((_maxTransactions - _processedTransactionsCS.count()) % 10000) == 0)
  {
    LOG4CXX_INFO(shutdownLogger,
                 "Max Transactions: " << _maxTransactions << " no of transactions: "
                                      << _maxTransactions - _processedTransactionsCS.count());
  }
  else
  {
    LOG4CXX_DEBUG(shutdownLogger,
                  "Max Transactions: " << _maxTransactions << " no of transactions: "
                                       << _maxTransactions - _processedTransactionsCS.count());
  }

  if (_processedTransactionsCS.count() <= 0)
  {
    const std::string reason = "Max Transactions exceeded";
    shutdown(reason);
  }

  LOG4CXX_DEBUG(shutdownLogger,
                "Remaining trx that can be processed = " << _processedTransactionsCS.count());
}

void
ServerSocketShutdownHandler::checkVirtualMemoryLimit()
{
  size_t mem;
  if (!Memory::changesFallback)
  {
    mem = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize() / (1024 * 1024);
  }
  else
  {
    mem = MemoryUsage::getVirtualMemorySize() / (1024 * 1024);
  }

  if (mem >= _maxVirtualMemory)
  {
    const std::string reason = "Max Virtual Memory exceeded";
    shutdown(reason);
  }
}

void ServerSocketShutdownHandler::checkAvailableMemory()
{
  if (Memory::changesFallback)
  {
    if (TrxUtil::isEnoughAvailableMemory(static_cast<size_t>(_minAvailableMemory),
                                         static_cast<size_t>(_maxRSSPercentage)))
      return;
  }
  else
  {
    if (LIKELY(Memory::GlobalManager::instance()->isEnoughAvailableMemory(
                                          static_cast<size_t>(_minAvailableMemory),
                                          static_cast<size_t>(_maxRSSPercentage))))
      return;
  }
  shutdown("Available memory below limit");
}

void
ServerSocketShutdownHandler::shutdown(const std::string& reason)
{
  // if it is already in exiting state, don't process the shutdown
  if (exiting())
    return;

  LOG4CXX_FATAL(shutdownLogger, reason);
  LOG4CXX_FATAL(shutdownLogger, "Active Transactions: " << TseManagerUtil::TrxCounter::count());
  uint32_t retryCount =
      _maxTransactionRetryTime ? (_maxTransactionWait / _maxTransactionRetryTime) : 0;
  finish();
  _adapter->onTrxLimitExceeded();
  _adapter->shutdown();
  LOG4CXX_WARN(shutdownLogger, "Adapter was shutdown");
  TseSrvStats::recordSocketClose();
  LOG4CXX_WARN(shutdownLogger, "TseSrvStats::recordSocketClose - done");
  ZThread::CountingSemaphore delayCs(1);
  delayCs.tryAcquire(0);

  if (_adapter->isAnyBigIPRegistered() && _maxBigIpDeregistrationWait > 0)
  {
    // Create trigger file for the external script to know that we are in trouble with
    // deregistering
    //  so it can help us by external deregister
    unlink(_bigIPDeregistrationDoneTriggerFile.c_str());

    std::ofstream fileHandle(_bigIPExtDeregistrationTriggerFile.c_str());
    if (!fileHandle)
    {
      LOG4CXX_ERROR(shutdownLogger, "Unable to create BIGIP deregistration Trigger File")
    }
    else
    {
      fileHandle.close();
      // Wait configured period of time for external script action before we will allow to kill
      // server
      uint32_t bigIpDeregistrationRetryCount =
          _maxBigIpDeregistrationRetryTime
              ? (_maxBigIpDeregistrationWait / _maxBigIpDeregistrationRetryTime)
              : 0;
      while (bigIpDeregistrationRetryCount-- > 0)
      {
        if (access(_bigIPDeregistrationDoneTriggerFile.c_str(), F_OK) && errno == ENOENT)
          delayCs.tryAcquire(_maxBigIpDeregistrationRetryTime);
        else
          break;
      }
      unlink(_bigIPExtDeregistrationTriggerFile.c_str());
    }
  }

  while (TseManagerUtil::TrxCounter::count() > 0 && retryCount > 0)
  {
    LOG4CXX_WARN(shutdownLogger, "Sleeping for  " << _maxTransactionRetryTime);
    delayCs.tryAcquire(_maxTransactionRetryTime);
    retryCount--;
    LOG4CXX_WARN(shutdownLogger,
                 "Remaining trx = " << TseManagerUtil::TrxCounter::count()
                                    << " Remaining retries = " << retryCount);
  }

  LOG4CXX_FATAL(shutdownLogger,
                reason << "...Shutting down server with active transactions: "
                       << TseManagerUtil::TrxCounter::count());
  struct rlimit rl;
  rl.rlim_cur = 0;
  rl.rlim_max = 0;
  std::cerr << "set core limit to 0,returned:" << setrlimit(RLIMIT_CORE, &rl) << std::endl;
  abort();
}
}
