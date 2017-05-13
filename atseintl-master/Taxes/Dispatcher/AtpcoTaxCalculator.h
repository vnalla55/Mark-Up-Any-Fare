// -------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------
#pragma once

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseThreadingConst.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "DataModel/PricingTrx.h"

#include "Taxes/AtpcoTaxes/DomainDataObjects/Request.h"
#include "Taxes/AtpcoTaxes/Processor/BusinessRulesProcessor.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "Taxes/Dispatcher/RequestToV2CacheBuilder.h"

namespace tax
{

class AtpcoTaxProcessingTask : public tse::TseCallableTrxTask
{
  DefaultServices _services;
  BusinessRulesProcessor* _processor = nullptr;
  const Request* _request = nullptr;
  const AtpcoTaxesActivationStatus* _activationStatus = nullptr;
  type::Index _itinIndex = 0;
  const OrderedTaxes* _orderedTaxes = nullptr;
  ItinsRawPayments* _itinsRawPayments = nullptr;
  ItinsPayments* _itinsPayments = nullptr;

public:
  void initialize(const Request& request,
                  const AtpcoTaxesActivationStatus& activationStatus,
                  type::Index itinIndex,
                  const OrderedTaxes& orderedTaxes,
                  ItinsRawPayments& itinsRawPayments,
                  ItinsPayments& itinsPayments,
                  tse::PricingTrx& trx)
  {
      tse::TseCallableTrxTask::trx(&trx);
      desc("ATPCO TAXES TASK");
      _request = &request;
      RequestToV2CacheBuilder().buildCache(_services, XmlCache(), *_request, trx.dataHandle());
      _processor = new BusinessRulesProcessor(_services);
      _activationStatus = &activationStatus;
      _itinIndex = itinIndex;
      _orderedTaxes = &orderedTaxes;
      _itinsRawPayments = &itinsRawPayments;
      _itinsPayments = &itinsPayments;
  }

  ~AtpcoTaxProcessingTask()
  {
    delete _processor;
  }

  void performTask() override
  {
    _processor->runSingleItin(*_request, _itinIndex, *_orderedTaxes, *_itinsRawPayments, *_itinsPayments);
  }
};

class AtpcoTaxCalculator
{
  uint32_t _remainingItin;
  tse::TseRunnableExecutor _pooledExecutor;
  tse::TseRunnableExecutor _synchronousExecutor;
  const OrderedTaxes& _orderedTaxes;
  ItinsRawPayments& _itinsRawPayments;
  ItinsPayments& _itinsPayments;
  const Request& _request;
  const AtpcoTaxesActivationStatus& _activationStatus;
  tse::PricingTrx _trx;

public:
  AtpcoTaxCalculator(const tse::TseThreadingConst::TaskId taskId,
                     uint32_t itinCount,
                     const OrderedTaxes& orderedTaxes,
                     ItinsRawPayments& itinsRawPayments,
                     ItinsPayments& itinsPayments,
                     const Request& request,
                     const AtpcoTaxesActivationStatus& activationStatus)
    : _remainingItin{itinCount},
      _pooledExecutor{taskId},
      _synchronousExecutor{tse::TseThreadingConst::SYNCHRONOUS_TASK},
      _orderedTaxes{orderedTaxes},
      _itinsRawPayments{itinsRawPayments},
      _itinsPayments{itinsPayments},
      _request{request},
      _activationStatus{activationStatus}
  {
    _trx.transactionStartTime() = tse::DateTime::localTime();
  }

  AtpcoTaxCalculator(const AtpcoTaxCalculator&) = delete;
  AtpcoTaxCalculator& operator=(const AtpcoTaxCalculator&) = delete;

  tse::TseRunnableExecutor& getTseExecutor()
  {
    return (_remainingItin > 1) ? _pooledExecutor : _synchronousExecutor;
  }

  void calculateTaxes(type::Index itinIndex)
  {
    AtpcoTaxProcessingTask* task = nullptr;
    _trx.dataHandle().get(task);
    TSE_ASSERT(task);

    task->initialize(_request,
                     _activationStatus,
                     itinIndex,
                     _orderedTaxes,
                     _itinsRawPayments,
                     _itinsPayments,
                     _trx);

    getTseExecutor().execute(task);

    --_remainingItin;
  }

  void wait()
  {
    _pooledExecutor.wait();
  }
};

}
