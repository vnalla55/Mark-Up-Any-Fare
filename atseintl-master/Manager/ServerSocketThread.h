//----------------------------------------------------------------------------
//
//  File:               ServerSocketThread.h
//
//  Description:        Thread to handle client requests
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

#include "Common/Thread/TseCallableTask.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/detail/atomic_count.hpp>
#include <ZThreads/zthread/CountingSemaphore.h>

#include <string>

namespace ZThread
{
class CountingSemaphore;
}

namespace tse
{
class ServerSocketAdapter;
class Service;
class Xform;
class ServerSocketShutdownHandler;

class ServerSocketThread : public tse::TseCallableTask
{
protected:
  std::string _name;
  ServerSocketAdapter* _adapter = nullptr;
  Service* _service = nullptr;
  Xform* _xform = nullptr;
  ServerSocketShutdownHandler* _shutdownHandler = nullptr;
  uint32_t _index = 0;

public:
  ServerSocketThread(const std::string& name, uint32_t index) : _name(name), _index(index) {}
  ServerSocketThread(const ServerSocketThread&) = delete;
  ServerSocketThread& operator=(const ServerSocketThread&) = delete;

  virtual void run() override;

  virtual bool initialize(ServerSocketAdapter* adapter,
                          Service* service,
                          Xform* xform,
                          ServerSocketShutdownHandler* shutdownHandler);

public:
  // functions that are not specific to thread instance
  static void warmServer(Xform& xform, Service& service);
}; // End class ServerSocketThread

class ServerSocketShutdownHandler
{
public:
  ServerSocketShutdownHandler(ServerSocketAdapter* adapter,
                              const std::string& executableName,
                              uint32_t maxTransactions,
                              uint32_t maxVirtualMemory,
                              int minAvailableMemory,
                              unsigned maxRSSPercentage,
                              uint32_t maxTransactionWait,
                              uint32_t maxTransactionRetryTime,
                              uint32_t maxBigIpDeregistrationWait,
                              uint32_t maxBigIpDeregistrationRetryTime);

  ServerSocketShutdownHandler(const ServerSocketShutdownHandler&) = delete;
  ServerSocketShutdownHandler& operator=(const ServerSocketShutdownHandler&) = delete;

  void finish() { ++_exiting; }
  bool exiting() const { return ((long)_exiting ? true : false); }
  ZThread::CountingSemaphore* processedTransactionsCS()
  {
    return (_maxTransactions > 0 ? &_processedTransactionsCS : nullptr);
  }

  void checkShutdownThresholds()
  {
    if (_maxTransactions > 0)
      checkTransactionLimit();
    if (_minAvailableMemory > 0)
      checkAvailableMemory();
    else if (_maxVirtualMemory > 0)
      checkVirtualMemoryLimit();
  }

protected:
  void checkTransactionLimit();
  void checkVirtualMemoryLimit();
  void checkAvailableMemory();
  void shutdown(const std::string& reason);

private:
  ServerSocketAdapter* _adapter = nullptr;

  uint32_t _maxTransactions;
  uint32_t _maxVirtualMemory;
  int _minAvailableMemory;
  unsigned _maxRSSPercentage;
  uint32_t _maxTransactionWait;
  uint32_t _maxTransactionRetryTime;
  uint32_t _maxBigIpDeregistrationWait;
  uint32_t _maxBigIpDeregistrationRetryTime;
  ZThread::CountingSemaphore _processedTransactionsCS;
  boost::detail::atomic_count _exiting{0};

  std::string _bigIPExtDeregistrationTriggerFile;
  std::string _bigIPDeregistrationDoneTriggerFile;

  // TODO - use of boost::atomic_count is temporary, it will be replaced
}; // End class SeverSocketShutdownHandler

} // End namespace tse

