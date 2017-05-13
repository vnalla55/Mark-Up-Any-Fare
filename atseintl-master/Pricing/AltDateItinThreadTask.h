/*---------------------------------------------------------------------------
 *  File:    AltDateItinThreadTask.h
 *
 *  Copyright Sabre 2010
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Memory/Config.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseEnums.h"
#include "DataModel/PricingTrx.h"
#include "Pricing/ItinMemoryManager.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PUPathMatrix.h"

#include <forward_list>

namespace tse
{

class AltDateItinThreadTask : public TseCallableTrxTask
{
public:
  AltDateItinThreadTask(PricingOrchestrator& po,
                        PricingTrx& trx,
                        PUPathMatrixVec& puMatrixVec,
                        std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
    : _po(po),
      _trx(trx),
      _puMatrixVec(puMatrixVec),
      _puFactoryBucketVect(puFactoryBucketVect)
  {
    TseCallableTrxTask::trx(&_trx);
    TseCallableTrxTask::desc("PRICE ALT DATE ITIN TASK");
  }

  void performTask() override
  {
    try
    {
      std::forward_list<ItinMemoryManager> managers;
      if (Memory::managerEnabled)
      {
        Memory::TrxManager* trxManager = _trx.getMemoryManager();
        if (trxManager)
        {
          for (PUPathMatrix* const matrix : _puMatrixVec)
            managers.emplace_front(*trxManager, *matrix->itin());
        }
      }

      _itinPassed = _po.priceADItinTask(_trx, _puMatrixVec, _puFactoryBucketVect);
    }
    catch (Memory::OutOfMemoryException& ere)
    {
      _error = ere;
      throw; // rethrow out of memory exception to stop all queues processing
    }
    catch (ErrorResponseException& ere) { _error = ere; }
    catch (...)
    {
      _error = ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                      "UNKNOWN ERROR DURING PRICING");
    }
  }

  bool itinPassed() const { return _itinPassed; }

  const ErrorResponseException& error() const { return _error; }

private:
  PricingOrchestrator& _po;
  PricingTrx& _trx;
  PUPathMatrixVec& _puMatrixVec;
  std::vector<PricingUnitFactoryBucket*>& _puFactoryBucketVect;
  ErrorResponseException _error = ErrorResponseException::NO_ERROR;
  bool _itinPassed = false;
};

} // tse

