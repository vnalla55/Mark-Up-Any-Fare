//-------------------------------------------------------------------
//
//  File:        Trx.cpp
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//
//  Description: Transaction's root object.
//
//  Updates:
//          10/01/05 - Val Perov - file created.
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

#include "DataModel/Trx.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/TrxAborter.h"
#include "Util/BranchPrediction.h"
#include "Xform/XMLConvertUtils.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace tse
{
FALLBACK_DECL(unifyMemoryAborter)

namespace
{
Logger
logger("atseintl.DataModel.Trx");

// TODO Remove these three with unifyMemoryAborter.
void
setAbortBit(std::atomic<uint8_t>& shouldAbort)
{
  shouldAbort.store(shouldAbort.load(std::memory_order_relaxed) | 1, std::memory_order_relaxed);
}
void
setAbortByHurryBit(std::atomic<uint8_t>& shouldAbort)
{
  shouldAbort.store(shouldAbort.load(std::memory_order_relaxed) | 2, std::memory_order_relaxed);
}
void
disableAbortByHurryBit(std::atomic<uint8_t>& shouldAbort)
{
  shouldAbort.store(shouldAbort.load(std::memory_order_relaxed) & ~2, std::memory_order_relaxed);
}

void
setBit(std::atomic<uint8_t>& shouldAbort, TrxAborter::Reason bit)
{
  shouldAbort.store(shouldAbort.load(std::memory_order_relaxed) | uint8_t(bit),
                    std::memory_order_relaxed);
}

void
clearBit(std::atomic<uint8_t>& shouldAbort, TrxAborter::Reason bit)
{
  shouldAbort.store(shouldAbort.load(std::memory_order_relaxed) & ~uint8_t(bit),
                    std::memory_order_relaxed);
}

#ifdef CONFIG_HIERARCHY_REFACTOR
ConfigBundle
getGlobalConfigBundle()
{
  const auto ptr = Global::configBundle().load(std::memory_order_acquire);
  if (LIKELY(ptr))
    return *ptr;

  // Should happen only in unit tests that do not use TestConfigInitializer.

  // Start with allocating dynamic values.
  allocateAllConfigBundles();

  ConfigBundle dummy;

  // This will init with defaults.
  ConfigMan empty;
  dummy.fill(empty);

  return dummy;
}
#endif
}

Trx::Trx(Memory::TrxManager* manager)
  : BaseTrx(manager),
    _dataHandle(DateTime::localTime(),
                MultiThreadedDeleteListSize,
                static_cast<int>(getBaseIntId()),
                manager),
    _latencyDataThread(nlatencyDataThread),
    _dynamicCfg(Global::configUpdateInProgress() ? Global::newDynamicCfg() : Global::dynamicCfg()),
#ifdef CONFIG_HIERARCHY_REFACTOR
    _configBundle(getGlobalConfigBundle()),
#endif
    _transactionSignature(pthread_self())
{
}

inline Trx::TransactionSignature::TransactionSignature(const pthread_t thread)
  : _mainThread(thread), _transactionId(generateTransactionId())
{
}

inline uint64_t
Trx::TransactionSignature::generateTransactionId() const
{
  DateTime startTime = boost::posix_time::microsec_clock::local_time();
  uint64_t dttm64 = startTime.get64BitRep();
  uint64_t transactionId = (((uint64_t)_mainThread) << 48) | // 1st 2 bytes
                           (dttm64 & 0x0000FFFFFFFFFFFF); // last 6 bytes
  return transactionId;
}

bool
Trx::recordMetrics(const char* metricsDescription) const
{
  if (!_recordMetrics)
    return false;
  if (!TrxUtil::needMetricsInResponse())
    return true;

  return (TrxUtil::needMetricsInResponse(metricsDescription));
}

bool
Trx::recordCPUTime(const char* metricsDescription) const
{
  if (!_recordCPUTime)
    return false;
  if (!TrxUtil::needMetricsInResponse())
    return true;

  return (TrxUtil::needMetricsInResponse(metricsDescription));
}

void
Trx::convert(tse::ErrorResponseException& ere, std::string& response)
{
  std::string xmlResponse;
  response.insert(0, ere.message());
  XMLConvertUtils::formatResponse(response, xmlResponse);
  response = xmlResponse;
}

bool
Trx::convert(std::string& response)
{
  LOG4CXX_WARN(logger, "Response conversion not supported for this transaction!");
  return false;
}

void
Trx::abort()
{
  if (fallback::unifyMemoryAborter(this))
  {
    aborter()->abort();
    return;
  }

  const uint8_t reason = _shouldAbort.load(std::memory_order_relaxed);
  if (reason)
    aborter()->abort(reason);
}

void
Trx::hurry()
{
  aborter()->hurry();
}

void
Trx::setAbortOnHurry(bool setting)
{
  if (_abortOnHurry == setting)
    return;
  std::lock_guard<std::mutex> lock(_aborterMutex);
  _abortOnHurry = setting;
  if (!_shouldHurry.load(std::memory_order_relaxed))
    return;

  if (fallback::unifyMemoryAborter(this))
  {
    if (_abortOnHurry)
      setAbortByHurryBit(_shouldAbort);
    else
      disableAbortByHurryBit(_shouldAbort);
  }
  else
  {
    if (_abortOnHurry)
      setBit(_shouldAbort, TrxAborter::Reason::HURRY);
    else
      clearBit(_shouldAbort, TrxAborter::Reason::HURRY);
  }
}

void
Trx::setTimeout()
{
  std::lock_guard<std::mutex> lock(_aborterMutex);
  if (fallback::unifyMemoryAborter(this))
    setAbortBit(_shouldAbort);
  else
    setBit(_shouldAbort, TrxAborter::Reason::TIMEOUT);
}

void
Trx::setOutOfMemory()
{
  std::lock_guard<std::mutex> lock(_aborterMutex);
  setBit(_shouldAbort, TrxAborter::Reason::MEMORY);
}

void
Trx::setHurry()
{
  std::lock_guard<std::mutex> lock(_aborterMutex);
  _shouldHurry.store(true, std::memory_order_relaxed);
  if (_abortOnHurry)
  {
    if (fallback::unifyMemoryAborter(this))
      setAbortByHurryBit(_shouldAbort);
    else
      setBit(_shouldAbort, TrxAborter::Reason::HURRY);
  }
}

void
Trx::setHurryWithCond()
{
  _shouldHurryWithCond.store(true, std::memory_order_relaxed);
}

void
Trx::setAborterFlagsAs(Trx* other)
{
  // We assume that no other thread will mess with our trx.
  std::lock_guard<std::mutex> lock(other->_aborterMutex);
  _shouldAbort.store(
      other->_shouldAbort.load(std::memory_order_relaxed), std::memory_order_relaxed);
  _shouldHurry.store(
      other->_shouldHurry.load(std::memory_order_relaxed), std::memory_order_relaxed);
  _shouldHurryWithCond.store(
      other->_shouldHurryWithCond.load(std::memory_order_relaxed), std::memory_order_relaxed);
  _abortOnHurry = other->_abortOnHurry;
}

void
Trx::resetTimeoutFlag()
{
  std::lock_guard<std::mutex> lock(_aborterMutex);
  clearBit(_shouldAbort, TrxAborter::Reason::TIMEOUT);
}

void
Trx::resetHurryFlag()
{
  std::lock_guard<std::mutex> lock(_aborterMutex);
  _shouldHurry.store(false, std::memory_order_relaxed);
  if (_abortOnHurry)
    clearBit(_shouldAbort, TrxAborter::Reason::HURRY);
}

void
Trx::resetHurryWithCondFlag()
{
  _shouldHurryWithCond.store(false, std::memory_order_relaxed);
}
} // tse namespace
