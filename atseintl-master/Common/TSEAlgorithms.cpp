//----------------------------------------------------------------------------
//
//  File:        TSEAlgorithms.cpp
//  Created:     03/01/2005
//  Authors:     Stephen Suggs
//
//  Description: Common algorithms for transaction processing
//
//  Updates:
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
//----------------------------------------------------------------------------
#include "Common/TSEAlgorithms.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"

#include <set>

namespace tse
{
FALLBACK_DECL(fallbackAPO45852ExcInItin);

namespace
{
Logger _logger("atseintl.Common.TseAlgorithms");

boost::mutex ItinResponseCodeMutex;

class ThreadSpecificResetter
{
  bool _tls;

public:
  ThreadSpecificResetter(bool tls) : _tls(tls) {}
  ~ThreadSpecificResetter()
  {
    if (UNLIKELY(_tls))
    {
      pthread_setspecific(tse::DataHandle::threadSpecificTicketDateKey(), nullptr);
      pthread_setspecific(tse::DataHandle::threadSpecificIsHistoricalKey(), nullptr);
    }
  }
};

class TIFMThread : public TseCallableTrxTask
{
public:
  TIFMThread() : _itin(nullptr), _fareMarket(nullptr), _func(0) { desc("ALGORITHM THREAD TASK"); }

  TIFMThread(Itin* itin, FareMarket* fareMarket, TrxItinFareMarketFunctor func)
    : _itin(itin), _fareMarket(fareMarket), _func(func)
  {
    desc("ALGORITHM THREAD TASK");
  }

  ~TIFMThread() {}
  void performTask() override
  {
    PricingTrx* _trx = trx();
    if (UNLIKELY(_trx == nullptr))
      LOG4CXX_ERROR(_logger, "NULL PricingTrx pointer!");
    if (UNLIKELY(_trx == nullptr || _itin == nullptr || _fareMarket == nullptr || _func == 0))
    {
      return;
    }

    bool tls = _trx->dataHandle().useTLS();

    if (UNLIKELY(tls))
    {
      const DateTime& retrivalDate = _fareMarket->retrievalDate();
      if (retrivalDate != DateTime::emptyDate())
      {
        pthread_setspecific(DataHandle::threadSpecificTicketDateKey(), &retrivalDate);
        if (_trx->dataHandle().isHistEnabled(retrivalDate))
          pthread_setspecific(DataHandle::threadSpecificIsHistoricalKey(), (void*)1);
        else
          pthread_setspecific(DataHandle::threadSpecificIsHistoricalKey(), (void*)0);
      }
      else
        return;
    }

    try
    {
      ThreadSpecificResetter tsr(tls);
      _func(*_trx, *_itin, *_fareMarket);
    }
    catch (ErrorResponseException& ere)
    {
      if (_itin != nullptr)
      {
        boost::lock_guard<boost::mutex> g(ItinResponseCodeMutex);

        _itin->errResponseCode() = ere.code();
        _itin->errResponseMsg() = ere.message();
      }
    }
  }

  Itin* _itin;
  FareMarket* _fareMarket;
  TrxItinFareMarketFunctor _func;
};

//----------------------------------------------------------------------------
// TIFMUtil
//----------------------------------------------------------------------------
class TIFMUtil
{
public:
  TIFMUtil(TseRunnableExecutor& exec,
           PricingTrx& trx,
           DataHandle& dataHandle,
           TrxItinFunctor itinFunc,
           TrxItinFareMarketFunctor fareMarketFunc,
           Itin* itin,
           bool validFareMarketsOnly,
           std::set<FareMarket*>* processedFareMarkets = nullptr,
           uint8_t serviceBit = 0,
           bool skipDummyFareMarkets = false)
    : _exec(exec),
      _trx(trx),
      _dataHandle(dataHandle),
      _itinFunc(itinFunc),
      _fareMarketFunc(fareMarketFunc),
      _itin(itin),
      _validFareMarketsOnly(validFareMarketsOnly),
      _processedFareMarkets(processedFareMarkets),
      _serviceBit(serviceBit),
      _skipDummyFareMarkets(skipDummyFareMarkets)
  {
  }
  ~TIFMUtil() {}

  void operator()(Itin* itin)
  {
    if (UNLIKELY(itin == nullptr))
      return;

    if (!fallback::fallbackAPO45852ExcInItin(&_trx))
    {
      if (UNLIKELY(itin->errResponseCode() != ErrorResponseException::NO_ERROR))
        return;
    }

    if (UNLIKELY(_itinFunc != 0))
      _itinFunc(_trx, *itin);

    // We wont have anything to do to the markets in this case
    if (UNLIKELY(_fareMarketFunc == 0))
      return;

    std::vector<FareMarket*>& fareMarkets = itin->fareMarket();
    std::for_each(fareMarkets.begin(),
                  fareMarkets.end(),
                  TIFMUtil(_exec,
                           _trx,
                           _dataHandle,
                           _itinFunc,
                           _fareMarketFunc,
                           itin,
                           _validFareMarketsOnly,
                           _processedFareMarkets,
                           _serviceBit,
                           _skipDummyFareMarkets));
  }

  void operator()(FareMarket* fm)
  {
    // We arent going to be able to run without these
    if (UNLIKELY(fm == nullptr))
      return;

    // If we have a serviceBit make sure we havent already processed this one
    if (_serviceBit != 0)
    {
      if (UNLIKELY(fm->serviceStatus().isSet((FareMarket::ServiceStatus)_serviceBit)))
        return; // We've already processed this one skip it
    }

    // Check if fareMarket is valid
    if (_validFareMarketsOnly && (fm->failCode() != ErrorResponseException::NO_ERROR))
      return;

    if (UNLIKELY(_skipDummyFareMarkets && fm->useDummyFare()))
      return;

    if (LIKELY(_processedFareMarkets != nullptr))
    {
      // Make sure we haven't already done this FM
      std::set<FareMarket*>::iterator i = _processedFareMarkets->find(fm);

      if (i != _processedFareMarkets->end())
      {
        LOG4CXX_DEBUG(_logger,
                      "Skipping FareMarket (" << FareMarketUtil::getDisplayString(*fm)
                                              << ")...already processed");
        return; // We've already done this one
      }

      _processedFareMarkets->insert(fm);
    }

    TIFMThread* thread = nullptr;
    _dataHandle.get(thread);

    //        thread->_trx = &_trx;
    // lint --e{413}
    thread->trx(&_trx);
    thread->_itin = _itin;
    thread->_fareMarket = fm;
    thread->_func = _fareMarketFunc;

    _exec.execute(*thread);
  }

protected:
  TseRunnableExecutor& _exec;
  PricingTrx& _trx;
  DataHandle& _dataHandle;
  TrxItinFunctor _itinFunc;
  TrxItinFareMarketFunctor _fareMarketFunc;
  Itin* _itin;
  bool _validFareMarketsOnly;
  std::set<FareMarket*>* _processedFareMarkets;
  uint8_t _serviceBit;
  bool _skipDummyFareMarkets;
};

//----------------------------------------------------------------------------
// TIFMSynchronousUtil
//
//         NOTE:  This class should be identical to TIFMUtil except
//                for lack of a DataHandle and the use of a stack based task
//
//----------------------------------------------------------------------------
class TIFMSynchronousUtil
{
public:
  TIFMSynchronousUtil(TseRunnableExecutor& exec,
                      PricingTrx& trx,
                      TrxItinFunctor itinFunc,
                      TrxItinFareMarketFunctor fareMarketFunc,
                      Itin* itin,
                      bool validFareMarketsOnly,
                      std::set<FareMarket*>* processedFareMarkets = nullptr,
                      uint8_t serviceBit = 0,
                      bool skipDummyFareMarkets = false)
    : _exec(exec),
      _trx(trx),
      _itinFunc(itinFunc),
      _fareMarketFunc(fareMarketFunc),
      _itin(itin),
      _validFareMarketsOnly(validFareMarketsOnly),
      _processedFareMarkets(processedFareMarkets),
      _serviceBit(serviceBit),
      _skipDummyFareMarkets(skipDummyFareMarkets)
  {
  }
  ~TIFMSynchronousUtil() {}

  void operator()(Itin* itin)
  {
    if (UNLIKELY(itin == nullptr))
      return;

    if (UNLIKELY(_itinFunc != 0))
      _itinFunc(_trx, *itin);

    // We wont have anything to do to the markets in this case
    if (UNLIKELY(_fareMarketFunc == 0))
      return;

    std::vector<FareMarket*>& fareMarkets = itin->fareMarket();
    std::for_each(fareMarkets.begin(),
                  fareMarkets.end(),
                  TIFMSynchronousUtil(_exec,
                                      _trx,
                                      _itinFunc,
                                      _fareMarketFunc,
                                      itin,
                                      _validFareMarketsOnly,
                                      _processedFareMarkets,
                                      _serviceBit,
                                      _skipDummyFareMarkets));
  }

  void operator()(FareMarket* fm)
  {
    // We arent going to be able to run without these
    if (UNLIKELY(fm == nullptr))
      return;

    // If we have a serviceBit make sure we havent already processed this one
    if (_serviceBit != 0)
    {
      if (fm->serviceStatus().isSet((FareMarket::ServiceStatus)_serviceBit))
        return; // We've already processed this one skip it
    }

    // Check if fareMarket is valid
    if (_validFareMarketsOnly && (fm->failCode() != ErrorResponseException::NO_ERROR))
      return;

    if (UNLIKELY(_skipDummyFareMarkets && fm->useDummyFare()))
      return;

    if (LIKELY(_processedFareMarkets != nullptr))
    {
      // Make sure we haven't already done this FM
      std::set<FareMarket*>::iterator i = _processedFareMarkets->find(fm);

      if (i != _processedFareMarkets->end())
      {
        LOG4CXX_DEBUG(_logger,
                      "Skipping FareMarket (" << FareMarketUtil::getDisplayString(*fm)
                                              << ")...already processed");
        return; // We've already done this one
      }

      _processedFareMarkets->insert(fm);
    }

    // Because this is invoked using a SynchronousExecutor, it is permissible to use a stack based
    // task object
    TIFMThread thread(_itin, fm, _fareMarketFunc);
    // lint --e{413}
    thread.trx(&_trx);

    _exec.execute(thread);
  }

protected:
  TseRunnableExecutor& _exec;
  PricingTrx& _trx;
  TrxItinFunctor _itinFunc;
  TrxItinFareMarketFunctor _fareMarketFunc;
  Itin* _itin;
  bool _validFareMarketsOnly;
  std::set<FareMarket*>* _processedFareMarkets;
  uint8_t _serviceBit;
  bool _skipDummyFareMarkets;
};

} // End anonymous namespace

//----------------------------------------------------------------------------
// exec_foreach_fareMarket()
//----------------------------------------------------------------------------
void
exec_foreach_fareMarket(TseThreadingConst::TaskId taskId,
                        PricingTrx& trx,
                        TrxItinFareMarketFunctor func)
{

  DataHandle dataHandle;
  TseRunnableExecutor taskExecutor(taskId);
  std::vector<Itin*>& itin = trx.itin();
  std::for_each(
      itin.begin(), itin.end(), TIFMUtil(taskExecutor, trx, dataHandle, 0, func, nullptr, false));
  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// exec_foreach_valid_fareMarket()
//----------------------------------------------------------------------------
void
exec_foreach_valid_fareMarket(TseThreadingConst::TaskId taskId,
                              PricingTrx& trx,
                              TrxItinFareMarketFunctor func,
                              uint8_t serviceBit)
{
  DataHandle dataHandle;
  TseRunnableExecutor taskExecutor(taskId);
  std::set<FareMarket*> fmSet;

  std::vector<Itin*>& itin = trx.itin();
  std::for_each(
      itin.begin(),
      itin.end(),
      TIFMUtil(taskExecutor, trx, dataHandle, 0, func, nullptr, true, &fmSet, serviceBit));
  taskExecutor.wait();
}

void
exec_foreach_valid_not_dummy_fareMarket(TseThreadingConst::TaskId taskId,
                                        PricingTrx& trx,
                                        TrxItinFareMarketFunctor func,
                                        uint8_t serviceBit)
{
  DataHandle dataHandle;
  TseRunnableExecutor taskExecutor(taskId);
  std::set<FareMarket*> fmSet;

  std::vector<Itin*>& itin = trx.itin();
  std::for_each(
      itin.begin(),
      itin.end(),
      TIFMUtil(taskExecutor, trx, dataHandle, 0, func, nullptr, true, &fmSet, serviceBit, true));
  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// exec_foreach_valid_fareMarket()
//----------------------------------------------------------------------------
void
exec_foreach_valid_fareMarket(TseThreadingConst::TaskId taskId,
                              PricingTrx& trx,
                              Itin& itin,
                              TrxItinFareMarketFunctor func,
                              uint8_t serviceBit)
{
  DataHandle dataHandle;
  TseRunnableExecutor taskExecutor(taskId);
  std::set<FareMarket*> fmSet;

  TIFMUtil util(taskExecutor, trx, dataHandle, 0, func, nullptr, true, &fmSet, serviceBit);
  util(&itin);
  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// exec_foreach_valid_common_fareMarket()
//----------------------------------------------------------------------------
void
exec_foreach_valid_common_fareMarket(TseThreadingConst::TaskId taskId,
                                     PricingTrx& trx,
                                     std::vector<FareMarket*>& fmv,
                                     TrxItinFareMarketFunctor func,
                                     uint8_t serviceBit)
{
  DataHandle dataHandle;
  TseRunnableExecutor taskExecutor(taskId);
  std::set<FareMarket*> fmSet;
  Itin* itin = trx.itin().front();
  std::for_each(fmv.begin(),
                fmv.end(),
                TIFMUtil(taskExecutor, trx, dataHandle, 0, func, itin, true, &fmSet, serviceBit));
  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// invoke_foreach_fareMarket()
//----------------------------------------------------------------------------
void
invoke_foreach_fareMarket(PricingTrx& trx, TrxItinFareMarketFunctor func)
{
  TseRunnableExecutor taskExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  std::set<FareMarket*> fmSet;

  std::vector<Itin*>& itin = trx.itin();
  std::for_each(itin.begin(),
                itin.end(),
                TIFMSynchronousUtil(taskExecutor, trx, 0, func, nullptr, false, &fmSet));

  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// invoke_foreach_valid_fareMarket()
//----------------------------------------------------------------------------
void
invoke_foreach_valid_fareMarket(PricingTrx& trx,
                                TrxItinFareMarketFunctor func,
                                uint8_t serviceBit)
{
  TseRunnableExecutor taskExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  std::set<FareMarket*> fmSet;

  std::vector<Itin*>& itin = trx.itin();
  std::for_each(itin.begin(),
                itin.end(),
                TIFMSynchronousUtil(taskExecutor, trx, 0, func, nullptr, true, &fmSet, serviceBit));

  taskExecutor.wait();
}

//----------------------------------------------------------------------------
// invoke_foreach_valid_fareMarket()
//----------------------------------------------------------------------------
void
invoke_foreach_valid_fareMarket(PricingTrx& trx,
                                Itin& itin,
                                TrxItinFareMarketFunctor func,
                                uint8_t serviceBit)
{
  TseRunnableExecutor taskExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  std::set<FareMarket*> fmSet;

  TIFMSynchronousUtil util(taskExecutor, trx, 0, func, nullptr, true, &fmSet, serviceBit);
  util(&itin);

  taskExecutor.wait();
}
}
