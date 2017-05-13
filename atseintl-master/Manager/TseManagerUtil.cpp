//----------------------------------------------------------------------------
//
//  File:               TseManagerUtil.cpp
//  Description:        Base execution routine for Manager objects
//  Created:            03/24/2005
//  Authors:            Mark Kasprowicz
//
//  Description:
//
//  Return types:
//
//  Copyright Sabre 2005
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
#include "Manager/TseManagerUtil.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Gauss.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/MetricsUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/Thread/TseThreadingConst.h"
#include "Common/Throttling.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "Common/TSELatencyData.h"
#include "Common/TseSrvStats.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/AborterTasks.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Response.h"
#include "DataModel/StatusTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Manager/SocketRequestImpl.h"
#include "Service/Service.h"
#include "Util/Time.h"
#include "Xform/Xform.h"

#include <memory>
#include <exception>
#include <sstream>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <ZThreads/zthread/CountingSemaphore.h>

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrActivation);
FIXEDFALLBACK_DECL(throttleEarly);
FIXEDFALLBACK_DECL(chargerDontSkipReqHeader);
FALLBACK_DECL(fallbackFareSelctionActivateMIP);
FALLBACK_DECL(fallbackTraditionalValidatingCxr)

volatile uint16_t TseManagerUtil::TrxCounter::_count = 0;
boost::mutex TseManagerUtil::TrxCounter::_mutex;

namespace
{
Logger
_logger("atseintl.Manager.TseManagerUtil");
Logger
_loggerMetrics("atseintl.Metrics");
Logger
_loggerRequest("atseintl.Request");
Logger
_loggerLongRunningTrx("atseintl.LongRunningTrx");

ConfigurableValue<uint16_t> serverPort("SERVER_SOCKET_ADP", "PORT", 53501);

template <typename T> T getConfigValue(const char* name,
                                       const char* group,
                                       const T& defaultValue = T())
{
  T value(defaultValue);
  if (!Global::config().getValue(name, value, group))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, name, group);
  }
  return value;
}

ConfigurableValue<uint32_t> trxThresholdDelay("TSE_SERVER", "TRX_THRESHOLD_DELAY", 0);

std::shared_ptr<TseRunnableExecutor> runnableExecutor;

Service* billingService = nullptr;
Service* requestResponseService = nullptr;

struct QueueSizeLimits
{
  QueueSizeLimits() : nThreads(0), maxThreads(-1) {}

  std::atomic<int> nThreads;
  int maxThreads;
};

struct QueueSizeLimitsExceededError
{
};

class QueueSizeLimitChecker
{
  QueueSizeLimits& _limits;
  int _numThreads;

public:
  QueueSizeLimitChecker(QueueSizeLimits& limits) : _limits(limits)
  {
    if (_limits.nThreads >= _limits.maxThreads && _limits.maxThreads >= 0)
    {
      throw QueueSizeLimitsExceededError();
    }

    ++_limits.nThreads;

    _numThreads = _limits.nThreads;
  }

  virtual ~QueueSizeLimitChecker()
  {
    --_limits.nThreads;
  }

  int numThreads() const { return _numThreads; }
};

QueueSizeLimits asyncTaskLimits, billingLimits, requestResponseLimits, asapLimits;

const char* GROUP_KEY = "TSE_MANAGER_UTIL";
// const char * ASYNC_KEY = "USE_ASYNC";
const char* BILLING_KEY = "BILLING_SERVICE";
const char* REQUEST_RESPONSE_KEY = "REQUEST_RESPONSE_SERVICE";
const char* ASYNC_TASK_NUM_THREADS = "ASYNC_TASK_NUM_THREADS";
const char* MAX_ASYNC_TASKS = "MAX_ASYNC_TASKS";
const char* MAX_BILLING_TASKS = "MAX_BILLING_TASKS";
const char* MAX_REQUEST_RESPONSE_TASKS = "MAX_REQUEST_RESPONSE_TASKS";
const char* MAX_ASAP_TASKS = "MAX_ASAP_TASKS";

struct InitAsyncTasks
{

InitAsyncTasks()
{
  std::string billingSvc;
  if (Global::config().getValue(BILLING_KEY, billingSvc, GROUP_KEY))
  {
    billingService = Global::service(billingSvc);
  }
  else
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, BILLING_KEY, GROUP_KEY);
  }

  std::string reqRspSvc;
  if (Global::config().getValue(REQUEST_RESPONSE_KEY, reqRspSvc, GROUP_KEY))
  {
    requestResponseService = Global::service(reqRspSvc);
  }
  else
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, REQUEST_RESPONSE_KEY, GROUP_KEY);
  }

  int asyncTaskThreads = 10;
  Global::config().getValue(ASYNC_TASK_NUM_THREADS, asyncTaskThreads, GROUP_KEY);

  runnableExecutor.reset(new TseRunnableExecutor(TseThreadingConst::TRANSACTION_ASYNCHRONOUS_TASK));

  if (!Global::config().getValue(MAX_ASYNC_TASKS, asyncTaskLimits.maxThreads, GROUP_KEY))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, MAX_ASYNC_TASKS, GROUP_KEY);
  }
  if (!Global::config().getValue(MAX_BILLING_TASKS, billingLimits.maxThreads, GROUP_KEY))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, MAX_BILLING_TASKS, GROUP_KEY);
  }
  if (!Global::config().getValue(
          MAX_REQUEST_RESPONSE_TASKS, requestResponseLimits.maxThreads, GROUP_KEY))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, MAX_REQUEST_RESPONSE_TASKS, GROUP_KEY);
  }
  if (!Global::config().getValue(MAX_ASAP_TASKS, asapLimits.maxThreads, GROUP_KEY))
  {
    CONFIG_MAN_LOG_KEY_ERROR(_logger, MAX_ASAP_TASKS, GROUP_KEY);
  }
}
};//InitAsyncTasks

// Used for the async thread
class AsyncTask : public TseCallableTask
{
public:
  AsyncTask(std::shared_ptr<DataHandle>& dhp,
            Trx& trx,
            bool successful,
            size_t reqSize,
            size_t rspSize,
            const std::string& request)
    : _queueSizeLimitChecker(asyncTaskLimits),
      _apdh(dhp),
      _trx(trx),
      _successful(successful),
      _reqSize(reqSize),
      _rspSize(rspSize),
      _request(request)
  {
  }

  void run() override
  {
    const MallocContext allocatorContext;
    LOG4CXX_INFO(_loggerMetrics, "Started AsyncTask: " << _queueSizeLimitChecker.numThreads() << " AsyncTasks running");

    std::string txn;
    {
      double elapsedTime = 0;
      double cpuTime = 0;

      // Write out the metrics
      if (PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&_trx))
      {
        std::ostringstream tmp;

        MetricsUtil::trxLatency(tmp, *pricingTrx, &elapsedTime, &cpuTime);

        LOG4CXX_INFO(_loggerMetrics, "\n" << tmp.str() << "\n");

        txn = TrxUtil::getTransId(*pricingTrx);

        LOG4CXX_INFO(_loggerMetrics,
                     "Start Time: '" << boost::posix_time::to_iso_extended_string(
                                            pricingTrx->transactionStartTime()) << "'");
        LOG4CXX_INFO(_loggerMetrics,
                     "  End Time: '" << boost::posix_time::to_iso_extended_string(
                                            pricingTrx->transactionEndTime()) << "'");
        LOG4CXX_INFO(_loggerMetrics, "       PNR: '" << TrxUtil::getPNR(*pricingTrx) << "'");
        LOG4CXX_INFO(_loggerMetrics, "     Entry: '" << TrxUtil::getLineEntry(*pricingTrx) << "'");
        LOG4CXX_INFO(_loggerMetrics, "       PCC: '" << TrxUtil::getPCC(*pricingTrx) << "'");
        LOG4CXX_INFO(_loggerMetrics, "   TransID: '" << txn << "'");
        LOG4CXX_INFO(_loggerMetrics, "    LNIATA: '" << TrxUtil::getLNIATA(*pricingTrx) << "'");

        if (pricingTrx->getXrayJsonMessage() != nullptr)
        {
          LOG4CXX_DEBUG(_logger, "XrayJsonMessage is existing. Filling fields.");

          pricingTrx->getXrayJsonMessage()->setStartDate(pricingTrx->transactionStartTime());
          pricingTrx->getXrayJsonMessage()->setEndDate(pricingTrx->transactionEndTime());
          pricingTrx->getXrayJsonMessage()->setService(pricingTrx->billing()->serviceName());
          pricingTrx->getXrayJsonMessage()->setInstance(std::to_string(serverPort.getValue()));

          if (pricingTrx->aborter() && pricingTrx->aborter()->getHurryLogicActivatedFlag())
          {
            DateTime hurryUpEndDate(pricingTrx->aborter()->getHurryAt());
            pricingTrx->getXrayJsonMessage()->setHurryUpEndDate(hurryUpEndDate);
          }

          pricingTrx->pushXrayJsonMessageToContainer();

          LOG4CXX_DEBUG(_logger, "XrayJsonMessage pushed to container.");
        }
      }
      else if (CurrencyTrx* currencyTrx = dynamic_cast<CurrencyTrx*>(&_trx))
      {
        std::ostringstream tmp;

        MetricsUtil::trxLatency(tmp, *currencyTrx, &elapsedTime, &cpuTime);

        LOG4CXX_INFO(_loggerMetrics, "\n" << tmp.str() << "\n");
      }
      else if (MultiExchangeTrx* multiTrx = dynamic_cast<MultiExchangeTrx*>(&_trx))
      {
        std::ostringstream tmp;

        MetricsUtil::trxLatency(tmp, *multiTrx, &elapsedTime, &cpuTime);
        LOG4CXX_INFO(_loggerMetrics, "\n" << tmp.str() << "\n");
        LOG4CXX_INFO(_loggerMetrics,
                     "Start Time: '" << boost::posix_time::to_iso_extended_string(
                                            multiTrx->transactionStartTime()) << "'");
        LOG4CXX_INFO(_loggerMetrics,
                     "  End Time: '" << boost::posix_time::to_iso_extended_string(
                                            multiTrx->transactionEndTime()) << "'");
        LOG4CXX_INFO(_loggerMetrics,
                     "       PCC: '" << TrxUtil::getPCC(*multiTrx->newPricingTrx()) << "'");
        LOG4CXX_INFO(_loggerMetrics,
                     "    LNIATA: '" << TrxUtil::getLNIATA(*multiTrx->newPricingTrx()) << "'");

        TseSrvStats::mergeTrxStats(*multiTrx);
      }

      // The elapsed time from the Metrics doesnt work if its off, getting it
      // from the transaction is better
      {
        boost::posix_time::time_period tp(_trx.transactionStartTime(), _trx.transactionEndTime());
        boost::posix_time::time_duration td = tp.length();

        elapsedTime = double(td.total_microseconds()) / 1000000.0;
      }
      // Setup the long running transaction time threshold values
      static const int trxTimeThreshold(getConfigValue("TRX_TIME_THRESHOLD", "TSE_SERVER", 0));

      if (trxTimeThreshold > 0 && elapsedTime > trxTimeThreshold)
      {
        // if elapsed time is greater than a threshold, write it to a long running transactions log
        // file
        // skip the first 8 bytes some type of trans id

        LOG4CXX_INFO(_loggerLongRunningTrx,
                     "TseManagerUtil Request with Elapsed Time " << elapsedTime << " Received - "
                                                                 << (_request.c_str() + 8));
      }

      TseSrvStats::recordTrxCall(_successful, _reqSize, _rspSize, elapsedTime, cpuTime, _trx);
    }

    // Write the billing records
    //
    // Note - we dont care if this process fails, do not return false or overwrite the
    // previous rc if there's a problem, just log it.

    if (billingService != nullptr)
    {
      try
      {
        QueueSizeLimitChecker check(billingLimits);

        LOG4CXX_DEBUG(_logger, "Calling Billing for TXN '" << txn << "'");
        bool rc =  _trx.process(*billingService);
        SUPPRESS_UNUSED_WARNING(rc);
        LOG4CXX_DEBUG(_logger, "Billing returned " << rc);
      }
      catch (QueueSizeLimitsExceededError&)
      {
        TseSrvStats::recordBillingCall(false, 0, 0, 0, _trx);
        LOG4CXX_ERROR(_loggerMetrics, "Could not call billing service: queue full");
      }
    }

    // Write the request/response records
    //
    // Note - we dont care if this process fails, do not return false or overwrite the
    // previous rc if there's a problem, just log it.
    if (requestResponseService != nullptr)
    {
      try
      {
        QueueSizeLimitChecker check(requestResponseLimits);

        LOG4CXX_DEBUG(_logger, "Calling RequestResponse for TXN '" << txn << "'");
        bool rc = _trx.process(*requestResponseService);
        SUPPRESS_UNUSED_WARNING(rc);
        LOG4CXX_DEBUG(_logger, "RequestResponse returned " << rc);
      }
      catch (QueueSizeLimitsExceededError&)
      {
        TseSrvStats::recordRequestResponseCall(false, 0, 0, 0, _trx);
        LOG4CXX_ERROR(_loggerMetrics, "Could not call request response service: queue full");
      }
    }

    {
      // Had to do these here when all the services were finished
      std::ostringstream tmp;

      TseSrvStats::dumpTrxStats(tmp, _trx);
      LOG4CXX_INFO(_loggerMetrics, "\n" << tmp.str() << "\n");
    }

    try
    {
      QueueSizeLimitChecker check(asapLimits);
      TseSrvStats::publishStats(_trx);
    }
    catch (QueueSizeLimitsExceededError&)
    {
      LOG4CXX_ERROR(_loggerMetrics, "Could not call asap service: queue full");
      TseSrvStats::publishStats(_trx, false /*no asap*/);
    }
  }

private:
  QueueSizeLimitChecker _queueSizeLimitChecker;
  std::shared_ptr<DataHandle> _apdh;
  Trx& _trx;
  bool _successful;
  size_t _reqSize;
  size_t _rspSize;
  std::string _request;

  // Placed here so they wont be called
  //
  AsyncTask(const AsyncTask& rhs);
  AsyncTask& operator=(const AsyncTask& rhs);
};
}

//==================================================================================
// process()
//==================================================================================
bool
TseManagerUtil::process(DateTime startTime,
                        TrxCounter& trxCounter,
                        std::string& req,
                        std::string& rsp,
                        Xform& xform,
                        Service& service,
                        bool throttled,
                        ZThread::CountingSemaphore* processedRequestsCS,
                        int requestID)
{
  static const InitAsyncTasks init;

  // make the transaction use its own region for allocations
  const MallocContext allocatorContext;

  LOG4CXX_DEBUG(_logger, "Entered process()");

  rsp.clear();

  if (!fallback::fixed::throttleEarly())
  {
    if (req.substr(0, 8) == "<Shoppin")
    {
      LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request "
                      << (throttled ? "Throttled" : "Received") << " - "
                      << "Request: ID(" << requestID << ") {{{\n" << (req.c_str()) << "\n}}}\n");
    }
    // skip the first 8 bytes some type of trans id
    else
    {
      if (!fallback::fixed::chargerDontSkipReqHeader())
      {
        std::string reqStart(req.c_str(), 0, 5);
        if (reqStart == "<TRQ>" || reqStart == "<TAX>")
        {
          LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request "
                          << (throttled ? "Throttled" : "Received") << " - " << (req.c_str()));
        }
        else
        {
          LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request "
                          << (throttled ? "Throttled" : "Received") << " - " << (req.c_str() + 8));
        }
      }
      else
      {
        LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request "
                        << (throttled ? "Throttled" : "Received") << " - " << (req.c_str() + 8));
      }
    }

    if (throttled)
    {
      xform.throttle(req, rsp);
      TseSrvStats::updateErrorCounts(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);
      LOG4CXX_WARN(_logger, "*******TRANSACTION THROTTLED*******");
      Time::sleepFor(std::chrono::milliseconds(trxThresholdDelay.getValue()));
      return !(rsp.empty());
    }
  }

  LOG4CXX_DEBUG(_logger,
                "Trx Start Time: " << boost::posix_time::to_iso_extended_string(startTime));

  Trx* trx = nullptr;
  std::shared_ptr<DataHandle> dh(new DataHandle);

  bool rc = false;

  // Threading Diagnostic
  DCFactory* factory = nullptr;
  PricingTrx* pricingTrx = nullptr;
  MultiExchangeTrx* multiTrx = nullptr;
  pthread_t pid = pthread_self();


  try
  {
    if (fallback::fixed::throttleEarly())
    {
      std::string reqStr(req.c_str(), 0, 8);
      if (reqStr == "<Shoppin")
      {
        LOG4CXX_INFO(_loggerRequest,
                    "TseManagerUtil Request Received - "
                        << "Request: ID(" << requestID << ") {{{\n" << (req.c_str()) << "\n}}}\n");
      }
      // skip the first 8 bytes some type of trans id
      else
      {
        if (!fallback::fixed::chargerDontSkipReqHeader())
        {
          std::string reqStart(req.c_str(), 0, 5);
          if (reqStart == "<TRQ>" || reqStart == "<TAX>")
          {
            LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request Received - " << (req.c_str()));
          }
          else
          {
            LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request Received - " << (req.c_str() + 8));
          }
        }
        else
        {
          LOG4CXX_INFO(_loggerRequest, "TseManagerUtil Request Received - " << (req.c_str() + 8));
        }
      }
    }

    LOG4CXX_DEBUG(_logger, "TseManagerUtil.process() - creating Trx");

    {
      LOG4CXX_DEBUG(_logger, "TseManagerUtil.process() - calling XForm.requestToTrx()");

      {
        if (!xform.convert(*dh, req, trx, throttled))
        {
          LOG4CXX_ERROR(_logger, "Xform::requestToTrx() failed for request [" << req << "]");
          throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);
        }
      }

      if (trx == nullptr)
      {
        LOG4CXX_ERROR(_logger, "Xform::requestToTrx() returned a null Trx pointer");
        throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);
      }

      Throttling throttling(*trx);
      throttling.checkThrottlingConditions();

      // We are about to process the transaction, so update the counter of
      // processed transactions
      if (processedRequestsCS != nullptr)
      {
        processedRequestsCS->tryAcquire(0);

        LOG4CXX_DEBUG(_logger, "Remaining trx to process = " << processedRequestsCS->count());
      }

      trx->transactionStartTime() = startTime;

      const TseCallableTrxTask::CurrentTrxSetter trxSetter(trx);

      pricingTrx = dynamic_cast<PricingTrx*>(trx);

      if (_logger->isInfoEnabled())
      {
        std::ostringstream tmp;
        tmp << "Start Trx";

        if (pricingTrx != nullptr)
        {
          const std::string& trxId = TrxUtil::getTransId(*pricingTrx);
          if (!trxId.empty())
            tmp << " ID:" << trxId;

          const std::string& clientTrxId = TrxUtil::getClientTransId(*pricingTrx);
          if (!clientTrxId.empty())
            tmp << " CID:" << clientTrxId;

          const std::string& pnr = TrxUtil::getPNR(*pricingTrx);
          if (!pnr.empty())
            tmp << " PNR:" << pnr;

          const std::string& entry = TrxUtil::getLineEntry(*pricingTrx);
          if (!entry.empty())
            tmp << " Entry:" << entry;

          std::string pcc = TrxUtil::getPCC(*pricingTrx);
          if (!pcc.empty())
            tmp << " PCC:" << pcc;

          const std::string& lniata = TrxUtil::getLNIATA(*pricingTrx);
          if (!lniata.empty())
            tmp << " LNIATA:" << lniata;
        }

        LOG4CXX_INFO(_logger, tmp.str());
      }

      // Setup the timeout values
      setTrxTimeout(*trx);
      setTrxActivationFlags(pricingTrx);

      setShoppingGsaActivationFlags(pricingTrx);

      // Throttle all transactions on memory critical condition.
      if (Memory::GlobalManager::instance() &&
          Memory::GlobalManager::instance()->isCriticalCondition())
      {
        throttled = true;
      }

      checkTrxThreshold(trxCounter.localCount(), throttled);

      static const std::string recordTimeVal(
          getConfigValue("RECORD_TIME", "METRICS", EMPTY_STRING()));
      trx->setRecordCPUTime(
            ((recordTimeVal == "Y") || (recordTimeVal == "YES") || (recordTimeVal == "1")));
      static const std::string recordMemVal(
          getConfigValue("RECORD_MEMORY", "METRICS", EMPTY_STRING()));
      trx->setRecordMemory(
            ((recordMemVal == "Y") || (recordMemVal == "YES") || (recordMemVal == "1")));

      LOG4CXX_DEBUG(_logger, "TseManagerUtil.process() - calling Service.process()");

      // Threading Diagnostic
      // Diagnostic belong to PricingTrx
      pricingTrx = dynamic_cast<PricingTrx*>(trx);
      if (pricingTrx != nullptr)
      {
        factory = DCFactory::instance();
        if (factory)
        {
          factory->createThreadDiag(*pricingTrx, pid);
        }
      }
      multiTrx = dynamic_cast<MultiExchangeTrx*>(trx);
      if (multiTrx)
      {
        factory = DCFactory::instance();
        if (factory)
        {
          factory->createThreadDiag(*(multiTrx->diagPricingTrx()), pid);
        }
      }

      {
        TSELatencyData latency(*trx, "TSEMANAGERUTIL SERVICE", true);
        // rc = _service.process(*trx);
        rc = trx->process(service);
      }

      if (!rc)
      {
        LOG4CXX_ERROR(_logger, "process() for request [" << (req.c_str() + 8) << "] failed!");
        throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);
      }

      if (StatusTrx* statusTrx = dynamic_cast<StatusTrx*>(trx))
      {
        statusTrx->response() << "YES" << std::endl;
      }
      else if (MetricsTrx* metricsTrx = dynamic_cast<MetricsTrx*>(trx))
      {
        std::ostringstream& tmp = metricsTrx->response();

        MetricsUtil::header(tmp, "TseManagerUtil Metrics");
        MetricsUtil::lineItemHeader(tmp);

        // MetricsUtil::lineItem(tmp, "TSEMANAGERUTIL SERVICE");
        // MetricsUtil::lineItem(tmp, "TSEMANAGERUTIL RSPXFORM");
      }

      LOG4CXX_DEBUG(_logger, "TseManagerUtil.process() - calling Xform.convert(Trx& trx, std::string& response)");

      {
        TSELatencyData latency(*trx, "TSEMANAGERUTIL RSPXFORM");
        rc = xform.convert(*trx, rsp);

        if (!rc)
          LOG4CXX_ERROR(_logger, "Xform.convert(Trx& trx, std::string& response) failed");
      }
    }
  }
  catch (NonFatalErrorResponseException& ere)
  {
    // rc made true since these tranactions should not be counted as failed
    rc = true;
    handleException(rsp, xform, trx, ere);
  }

  catch (ErrorResponseException& ere)
  {
    // rc should already be false, leave it alone
    handleException(rsp, xform, trx, ere);
  }
  catch (TSEException& e)
  {
    LOG4CXX_ERROR(_logger, "Error Exception caught " << e.what());
    LOG4CXX_ERROR(_logger, e.where());

    // rc should already be false, leave it alone
    ErrorResponseException ere(ErrorResponseException::UNKNOWN_EXCEPTION);
    handleException(rsp, xform, trx, ere);
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Error std::exception caught " << e.what());
    std::cerr << "Error std::Exception caught " << e.what() << std::endl;

    // rc should already be false, leave it alone
    ErrorResponseException ere(ErrorResponseException::UNKNOWN_EXCEPTION);
    handleException(rsp, xform, trx, ere);
  }
  catch (...) // everything
  {
    LOG4CXX_ERROR(_logger, "Error Unknown exception caught ");
    std::cerr << "Error: Unknown exception" << std::endl;

    // rc should already be false, leave it alone
    ErrorResponseException ere(ErrorResponseException::UNKNOWN_EXCEPTION);
    handleException(rsp, xform, trx, ere);
  }

  // clean up pid diagnostic map
  if (pricingTrx && factory)
  {
    factory->endThreadDiag(*pricingTrx, pid);
  }
  else if (multiTrx && factory)
  {
    factory->endThreadDiag(*(multiTrx->diagPricingTrx()), pid);
  }

  DateTime endTime = boost::posix_time::microsec_clock::local_time();
  LOG4CXX_DEBUG(_logger, "Trx End Time: " << boost::posix_time::to_iso_extended_string(endTime));

  if (trx != nullptr)
  { // We set this again in case an exception got thrown and the start isn't correct.
    trx->transactionStartTime() = startTime;
    trx->transactionEndTime() = endTime;
    if (trx->aborter())
      trx->aborter()->terminateAborterTasks();
    trx->setForceNoTimeout(true); // We dont want to accidentally timeout in the AsyncTask

    if (_logger->isInfoEnabled())
    {
      std::ostringstream tmp;
      tmp << "End Trx";

      if (pricingTrx != nullptr)
      {
        const std::string& transId = TrxUtil::getTransId(*pricingTrx);
        if (!transId.empty())
          tmp << " ID:" << transId;
      }

      LOG4CXX_INFO(_logger, tmp.str());
    }

    try
    {
      if (runnableExecutor != nullptr)
      {
        AsyncTask* task(new AsyncTask(dh, *trx, rc, req.size(), rsp.size(), req));
        runnableExecutor->execute(task, true);
      }
    }
    catch (QueueSizeLimitsExceededError&)
    {
      // we couldn't start the async task, because there
      // are too many already....nothing we can do.
      LOG4CXX_ERROR(_loggerMetrics,
                    "Could not start AsyncTask: "
                    "Too many AsyncTasks already in progress");
    }
  }

  LOG4CXX_DEBUG(_logger, "Leaving process()");
  if (!rc)
  {
    LOG4CXX_WARN(_logger, "*******TRANSACTION FAILED************");
  }

  // If the response has anything in it, write it back to PSS, returning true does that
  return (!rsp.empty());
}

//==================================================================================
// setTrxTimeout
//==================================================================================
void
TseManagerUtil::setTrxTimeout(Trx& trx)
{

  ShoppingTrx* shpTrx = dynamic_cast<ShoppingTrx*>(&trx);
  PricingTrx* prcTrx = dynamic_cast<PricingTrx*>(&trx);
  if (prcTrx && (shpTrx || (prcTrx->getTrxType() == PricingTrx::MIP_TRX)))
  {
    const bool nonDiagTrx = (prcTrx->diagnostic().diagnosticType() == DiagnosticNone);

    static const int trxTimeout(getConfigValue("TIMEOUT", "SHOPPING_SVC", 0));

    if (trxTimeout > 0)
    {
      if (trx.aborter() != nullptr)
      {
        TrxAborter* aborter = trx.aborter();
        if (aborter && aborter->timeout() > (2 * trxTimeout) && nonDiagTrx && !trx.isTestRequest())
        {
          ShoppingUtil::setTimeout(*prcTrx, 2 * trxTimeout);
        }
      }
      else
      {
        TrxAborter* aborter = &trx.dataHandle().safe_create<TrxAborter>(&trx);
        trx.aborter() = aborter;
        ShoppingUtil::setTimeout(*prcTrx, trxTimeout);
      }
    }
  }
  else
  {
    TrxUtil::createTrxAborter(trx);
  }
}

//==================================================================================
// checkTrxThreshold()
//==================================================================================
void
TseManagerUtil::checkTrxThreshold(uint16_t count,
                                  bool throttled)
{
  if (throttled)
  {
    static const uint16_t threshold(getConfigValue("TRX_THRESHOLD", "TSE_SERVER", 0));

    LOG4CXX_WARN(_logger, "TRANSACTION THRESHOLD REACHED, FAILING TRANSACTION. threshold: " << threshold);

    static const unsigned long trxThresholdDelay(getConfigValue("TRX_THRESHOLD_DELAY", "TSE_SERVER", 2000));
    Time::sleepFor(std::chrono::milliseconds(trxThresholdDelay));
    throw ErrorResponseException(ErrorResponseException::TRANSACTION_THRESHOLD_REACHED);
  }
}

//==================================================================================
// TrxCounter()
//==================================================================================
TseManagerUtil::TrxCounter::TrxCounter()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  TseSrvStats::recordConcurrentTrx(_count);
  _localCount = _count;
  ++_count;
}

//==================================================================================
// ~TrxCounter()
//==================================================================================
TseManagerUtil::TrxCounter::~TrxCounter()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  if (_count > 0)
    _count--;
  TseSrvStats::recordConcurrentTrx(_count);
};

uint16_t
TseManagerUtil::TrxCounter::count()
{
  boost::lock_guard<boost::mutex> g(_mutex);
  return _count;
}

//==================================================================================
// handleException()
//==================================================================================
void
TseManagerUtil::handleException(
    std::string& rsp, Xform& xform, Trx* trx, ErrorResponseException& ere, bool rc)
{
  rsp.clear();
  if (trx != nullptr)
  {
    xform.convert(ere, *trx, rsp);
    trx->transactionRC() = ere.code();
  }
  else
  {
    xform.convert(ere, rsp);
  }
  if (!rc)
  {
    TseSrvStats::updateErrorCounts(ere.code());
  }
}

//==================================================================================
// deinitialize()
//==================================================================================
void
TseManagerUtil::deinitialize()
{
  if (runnableExecutor != nullptr)
    runnableExecutor->wait();

  while (asyncTaskLimits.nThreads || billingLimits.nThreads || requestResponseLimits.nThreads ||
         asapLimits.nThreads)
  {
    timespec tspec;
    tspec.tv_sec = 0;
    tspec.tv_nsec = 100000000;
    nanosleep(&tspec, nullptr);
  }
}

void
TseManagerUtil::setTrxActivationFlags(Trx* trx)
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx);

  if (pricingTrx)
  {
    const PricingTrx::TrxType trxType = pricingTrx->getTrxType();
    bool isIataShoppingRequestActive = true;

    if (fallback::fallbackFareSelctionActivateMIP(pricingTrx))
    {
      isIataShoppingRequestActive =
      ((PricingTrx::IS_TRX == trxType) || (PricingTrx::MIP_TRX == trxType)) ?
        pricingTrx->getProjectActivationFlag() & PricingTrx::FareSelection_Active : true;
    }

    if ( pricingTrx->getRequest() &&
         TrxUtil::isIataFareSelectionActivated( *pricingTrx ) &&
         isIataShoppingRequestActive  &&
         TrxUtil::isIataFareSelectionApplicable( pricingTrx ) )
    {
      pricingTrx->setIataFareSelectionApplicable( true );
    }

    if (!fallback::fallbackValidatingCxrActivation(pricingTrx))
    {
      const PricingRequest*      request = pricingTrx->getRequest();
      const Agent*                 agent = request ? request->ticketingAgent() : nullptr;
      const bool           isTravelAgent = agent && !agent->tvlAgencyPCC().empty();
      const PricingTrx::TrxType  trxType = pricingTrx->getTrxType();

      const bool isBaggageTrx = dynamic_cast<BaggageTrx*>(trx) != nullptr;
      const bool isGsaTrx = (PricingTrx::PRICING_TRX == trxType &&
         (dynamic_cast<AncillaryPricingTrx*>(trx) == nullptr) &&
         (dynamic_cast<TktFeesPricingTrx*>(trx) == nullptr)   &&
         !isBaggageTrx)   ||
        PricingTrx::IS_TRX == trxType ||
        PricingTrx::MIP_TRX == trxType ||
        PricingTrx::FF_TRX == trxType ||
        PricingTrx::RESHOP_TRX == trxType ||
        PricingTrx::REPRICING_TRX  == trxType;

      pricingTrx->setValidatingCxrGsaApplicable(false);
      if (isGsaTrx && isTravelAgent)
      {
        if (request->ticketingAgent()->agentTJR())
        {
          if (request->ticketingAgent()->agentTJR()->pricingApplTag5() == 'Y')
          {
            DateTime ticketingDate = ValidatingCxrUtil::determineTicketDate(*pricingTrx);
            if (pricingTrx->isCustomerActivatedByDateAndFlag("GSA", ticketingDate))
            {
              pricingTrx->setValidatingCxrGsaApplicable(true);

              if (!fallback::fallbackTraditionalValidatingCxr(pricingTrx) &&
                  request->ticketingAgent()->agentTJR()->pricingApplTag7() == 'Y' &&
                  request->validatingCarrier().empty())
              {
                 pricingTrx->setTraditionalValidatingCxr(true);
                 pricingTrx->setSkipNeutral(true);
              }
            }
            else
            {
              DCFactory* factory = DCFactory::instance();
              DiagCollector* diag = factory->create(*pricingTrx);
              if (diag)
              {
                diag->enable(Diagnostic191);
                *diag << "GSA NOT ACTIVE IN CUSTOMER ACTV TABLE FOR "
                  << TrxUtil::getPCC(*pricingTrx);
                diag->flushMsg();
              }
            }

          }
          else
          {
            DCFactory* factory = DCFactory::instance();
            DiagCollector* diag = factory->create(*pricingTrx);
            if (diag)
            {
              diag->enable(Diagnostic191);
              *diag << "GSA DISABLED BASED ON TJR FOR PCC "
                << TrxUtil::getPCC(*pricingTrx);
              diag->flushMsg();
            }
          }
        }
      }
    }
  }
}

void
TseManagerUtil::setShoppingGsaActivationFlags(Trx* trx)
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx);

  if (pricingTrx && pricingTrx->isValidatingCxrGsaApplicable())
  {
    const PricingRequest* request       = pricingTrx->getRequest();
    const PricingTrx::TrxType  trxType  = pricingTrx->getTrxType();
    const bool      isShoppingTrx       = PricingTrx::IS_TRX  == trxType ||
                                          PricingTrx::MIP_TRX == trxType ||
                                          PricingTrx::FF_TRX  == trxType;

    if (request && isShoppingTrx)
    {
      if (!request->isValidatingCarrierRequest())
        pricingTrx->setValidatingCxrGsaApplicable(false);
    }
  }
}

} // tse
