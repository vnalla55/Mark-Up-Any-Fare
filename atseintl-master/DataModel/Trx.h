//-------------------------------------------------------------------
//
//  File:        Trx.h
//  Created:     March 8, 2004
//  Design:      Doug Steeb
//  Authors:
//
//  Description: Transaction's root object.
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/15/04 - Mike Carroll - added options
//          06/02/04 - Mark Kasprowicz - rename from Trx to PricingTrx
//
//  Copyright Sabre 2004
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

#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigBundle.h"
#endif
#include "Common/ErrorResponseException.h"
#include "DBAccess/DataHandle.h"
#include "DataModel/BaseTrx.h"
#include "DataModel/SurchargeData.h"
#include "Diagnostic/Diagnostic.h"

#include <boost/pool/object_pool.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <pthread.h>

#include <atomic>
#include <deque>
#include <map>
#include <mutex>

namespace tse
{
class DiagCollector;
class Throttling;
class TrxAborter;
class Service;
class Billing;

class Trx : public BaseTrx
{
public:
  using TidDisabledDCMap = std::map<pthread_t, DiagCollector*>;

  DataHandle& dataHandle() { return _dataHandle; }
  DataHandle& dataHandle() const { return _dataHandle; }

  tse::SurchargeData* constructSurchargeData()
  {
    boost::lock_guard<boost::mutex> guard(_surchargeDataPoolMutex);
    return _surchargeDataPool.construct();
  }

  struct Latency
  {
    Latency(size_t nItems,
            const char* description,
            double wallTime,
            double userTime,
            double systemTime,
            double childUserTime,
            double childSystemTime,
            size_t startMem,
            size_t endMem,
            size_t thread,
            bool toplevelTaskData,
            const Service* service)
      : nRepeats(1),
        nItems(nItems),
        description(description),
        wallTime(wallTime),
        userTime(userTime),
        systemTime(systemTime),
        childUserTime(childUserTime),
        childSystemTime(childSystemTime),
        startMem(startMem),
        endMem(endMem),
        thread(thread),
        toplevelTaskData(toplevelTaskData),
        service(service)
    {
    }

    size_t nRepeats, nItems;
    const char* description;
    double wallTime, userTime, systemTime, childUserTime, childSystemTime;
    size_t startMem, endMem;
    size_t thread;
    bool toplevelTaskData;
    const Service* service;
  };

  std::deque<Latency>& latencyData() { return _latencyData; }
  const std::deque<Latency>& latencyData() const { return _latencyData; }

  std::vector<std::map<size_t, std::deque<Latency> > >& latencyDataThread()
  {
    return _latencyDataThread;
  }

  const std::vector<std::map<size_t, std::deque<Latency> > >& latencyDataThread() const
  {
    return _latencyDataThread;
  }

  boost::mutex* latencyDataThreadMutex() { return _latencyDataThreadMutex; }

  pthread_t mainThread() const { return _transactionSignature.getMainThread(); }

  virtual void setRecordCPUTime(bool newValue) { _recordCPUTime = newValue; }
  const bool& recordCPUTime() const { return _recordCPUTime; }
  bool recordCPUTime(const char* description) const;

  virtual void setRecordMemory(bool newValue) { _recordMemory = newValue; }
  const bool& recordMemory() const { return _recordMemory; }

  bool& xml2() { return _xml2; }
  const bool& xml2() const { return _xml2; }

  bool& recordTopLevelMetricsOnly() { return _recordTopLevelMetricsOnly; }
  const bool& recordTopLevelMetricsOnly() const { return _recordTopLevelMetricsOnly; }

  bool& recordMetrics() { return _recordMetrics; }
  const bool& recordMetrics() const { return _recordMetrics; }

  bool recordMetrics(const char* metricsDescription) const;

  TrxAborter*& aborter() { return _aborter; }
  const TrxAborter* aborter() const { return _aborter; }

  std::vector<double>& statData() { return _statData; }
  const std::vector<double>& statData() const { return _statData; }

  boost::mutex& statDataMutex() { return _statDataMutex; }
  const boost::mutex& statDataMutex() const { return _statDataMutex; }

  DateTime& transactionStartTime() { return _transactionStartTime; }
  const DateTime& transactionStartTime() const { return _transactionStartTime; }

  DateTime& transactionEndTime() { return _transactionEndTime; }
  const DateTime& transactionEndTime() const { return _transactionEndTime; }

  virtual const DateTime& ticketingDate() const { return _transactionStartTime; }

  ErrorResponseException::ErrorResponseCode& transactionRC() { return _transactionRC; }
  const ErrorResponseException::ErrorResponseCode& transactionRC() const { return _transactionRC; }

  Global::configPtr& dynamicCfg() { return _dynamicCfg; }
  const Global::configPtr& dynamicCfg() const { return _dynamicCfg; }

#ifdef CONFIG_HIERARCHY_REFACTOR
  ConfigBundle& mutableConfigBundle() { return _configBundle; }
  const ConfigBundle& configBundle() const { return _configBundle; }
#endif

  bool isDynamicCfgOverriden() const { return _dynamicCfgOverriden; }
  void setDynamicCfgOverriden(bool value) { _dynamicCfgOverriden = value; }

  virtual bool process(Service& srv) = 0;

  virtual void convert(tse::ErrorResponseException& ere, std::string& response);

  virtual bool convert(std::string& response);

  uint64_t transactionId() const { return _transactionSignature.getTransactionId(); }

  void setParentTrx(Trx* trx) { _parentTrx = trx; }
  const Trx* getParentTrx() const { return _parentTrx; }
  Trx* getParentTrx() { return _parentTrx; }

  Trx* mainTrx() { return ((_parentTrx == nullptr) ? this : _parentTrx->mainTrx()); }

  // diagnostic stuff
  TidDisabledDCMap& tidDisabledDCMap() { return _tidDisabledDCMap; }
  const TidDisabledDCMap& tidDisabledDCMap() const { return _tidDisabledDCMap; }

  Diagnostic& diagnostic() { return _redirectedDiagnostic ? *_redirectedDiagnostic : _diagnostic; }
  const Diagnostic& diagnostic() const
  {
    return _redirectedDiagnostic ? *_redirectedDiagnostic : _diagnostic;
  }

  Diagnostic*& redirectedDiagnostic() { return _redirectedDiagnostic; }
  const Diagnostic* redirectedDiagnostic() const { return _redirectedDiagnostic; }

  bool isDiagActive() const
  {
    const DiagnosticTypes diagType = diagnostic().diagnosticType();
    return (diagType >= PRICING_DIAG_RANGE_BEGIN && diagType <= PRICING_DIAG_RANGE_END);
  }

  boost::mutex& mutexDisabledDC() { return _mutexDisabledDC; }
  const boost::mutex& mutexDisabledDC() const { return _mutexDisabledDC; }

  bool& mcpCarrierSwap() { return _mcpCarrierSwap; }
  const bool& mcpCarrierSwap() const { return _mcpCarrierSwap; }

  bool& segmentFeeApplied() { return _segmentFeeApplied; }
  const bool& segmentFeeApplied() const { return _segmentFeeApplied; }

  bool& isTestRequest() { return _isTestRequest; }
  const bool isTestRequest() const { return _isTestRequest; }

  void setForceNoTimeout(bool force) { _forceNoTimeout = force; }
  bool isForceNoTimeout() const { return _forceNoTimeout; }

  void setCurrentService(const Service* service) { _currentService = service; }
  const Service* getCurrentService() const { return _currentService; }

  void setThrottling(Throttling* throttling) { _throttling = throttling; }
  Throttling* throttling() const { return _throttling; }

  bool& overrideFallbackValidationCXRMultiSP() { return _overrideFallbackValidationCXRMultiSP; }
  const bool& overrideFallbackValidationCXRMultiSP() const { return _overrideFallbackValidationCXRMultiSP; }

  void checkTrxAborted()
  {
    if (UNLIKELY(_shouldAbort.load(std::memory_order_relaxed)))
    {
      abort();
    }
  }

  bool checkTrxMustHurry()
  {
    if (UNLIKELY(_shouldHurry.load(std::memory_order_relaxed)))
    {
      hurry();
      return true;
    }
    return false;
  }

  bool checkTrxMustHurryWithCond()
  {
    if (UNLIKELY(_shouldHurryWithCond.load(std::memory_order_relaxed)))
    {
      hurry();
      return true;
    }
    return false;
  }

  void setAbortOnHurry(bool setting = true);

  void setTimeout();
  void setOutOfMemory();
  void setHurry();
  void setHurryWithCond();

  void setAborterFlagsAs(Trx* other);

  void resetTimeoutFlag();
  void resetHurryFlag();
  void resetHurryWithCondFlag();

  virtual const Billing* billing() const { return nullptr; }

protected:
  static const size_t nlatencyDataThread = 17;

  // a suggested size for delete lists that will be accessed from
  // multiple threads
  static const size_t MultiThreadedDeleteListSize = 97;

  Trx(Memory::TrxManager* manager = nullptr);

  mutable DataHandle _dataHandle;
  mutable boost::object_pool<tse::SurchargeData> _surchargeDataPool;
  boost::mutex _surchargeDataPoolMutex;

  std::deque<Latency> _latencyData;

  std::vector<std::map<size_t, std::deque<Latency> > > _latencyDataThread;
  boost::mutex _latencyDataThreadMutex[nlatencyDataThread];

  bool _recordCPUTime = true;
  bool _recordMemory = false;

  bool _xml2 = false;
  bool _dynamicCfgOverriden = false;
  bool _recordTopLevelMetricsOnly = false;
  bool _recordMetrics = false;

  std::atomic<uint8_t> _shouldAbort{0};
  std::atomic<bool> _shouldHurry{false};
  std::atomic<bool> _shouldHurryWithCond{false};
  bool _abortOnHurry = false;

  std::mutex _aborterMutex;

  TrxAborter* _aborter = nullptr;
  std::vector<double> _statData;
  boost::mutex _statDataMutex;

  // These are used for the billing record
  DateTime _transactionStartTime;
  DateTime _transactionEndTime;
  ErrorResponseException::ErrorResponseCode _transactionRC =
      ErrorResponseException::ErrorResponseCode::NO_ERROR;

  Trx* _parentTrx = nullptr;

  TidDisabledDCMap _tidDisabledDCMap;

  Diagnostic _diagnostic{DiagnosticTypes::DiagnosticNone};
  boost::mutex _mutexDisabledDC;
  Diagnostic* _redirectedDiagnostic = nullptr;
  bool _mcpCarrierSwap = false;

  Global::configPtr _dynamicCfg;
#ifdef CONFIG_HIERARCHY_REFACTOR
  ConfigBundle _configBundle;
#endif
  bool _segmentFeeApplied = false;

  bool _isTestRequest = false;

  bool _forceNoTimeout = false;

  const Service* _currentService = nullptr;

  bool _overrideFallbackValidationCXRMultiSP = false;

private:
  void hurry();
  void abort();
  class TransactionSignature
  {
    const pthread_t _mainThread;
    const uint64_t _transactionId;

    uint64_t generateTransactionId() const;

  public:
    explicit TransactionSignature(const pthread_t);

    pthread_t getMainThread() const { return _mainThread; }
    uint64_t getTransactionId() const { return _transactionId; }
  };

  const TransactionSignature _transactionSignature;

  Throttling* _throttling = nullptr;
}; // class Trx
} // tse namespace
