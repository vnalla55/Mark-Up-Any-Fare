/*---------------------------------------------------------------------------
 *  File:    ItinThreadTask.h
 *
 *  Copyright Sabre 2003
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
#include "Common/Memory/LocalManager.h"
#include "Common/Memory/OutOfMemoryException.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Pricing/ItinMemoryManager.h"
#include "Pricing/PricingOrchestrator.h"
#include "Pricing/PricingUnitFactoryBucket.h"

#include <memory>

namespace tse
{
class PUPathMatrix;

class ItinThreadTask : public TseCallableTrxTask
{
public:
  ItinThreadTask(PricingOrchestrator& po,
                 PricingTrx& trx,
                 Itin& itin,
                 PUPathMatrix& puMatrix,
                 std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
    : _po(po),
      _trx(trx),
      _itin(itin),
      _puMatrix(puMatrix),
      _puFactoryBucketVect(puFactoryBucketVect),
      _error(ErrorResponseException::NO_ERROR),
      _itinPassed(false)
  {
    TseCallableTrxTask::trx(&_trx);
    TseCallableTrxTask::desc("PRICE ITIN TASK");
  }

  void performTask() override
  {
    try
    {
      std::unique_ptr<ItinMemoryManager> memoryManager;
      if (Memory::managerEnabled)
      {
        if (_trx.getMemoryManager())
          memoryManager.reset(new ItinMemoryManager(*_trx.getMemoryManager(), _itin));
      }
      _itinPassed = _po.priceItinTask(_trx, _itin, _puMatrix, _puFactoryBucketVect);
    }
    catch (Memory::OutOfMemoryException& ere)
    {
      _error = ere;
      _itin.errResponseCode() = _error.code();
      _itin.errResponseMsg() = _error.message();
      throw; // rethrow out of memory exception to stop all queues processing
    }
    catch (ErrorResponseException& ere)
    {
      _error = ere;
    }
    catch (...)
    {
      _error = ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                      "UNKNOWN ERROR DURING PRICING");
    }

    _itin.errResponseCode() = _error.code();
    _itin.errResponseMsg() = _error.message();
  }

  bool itinPassed() const { return _itinPassed; }

  const ErrorResponseException& error() const { return _error; }

  const Itin& getItin() const { return _itin; }

private:
  PricingOrchestrator& _po;
  PricingTrx& _trx;
  Itin& _itin;
  PUPathMatrix& _puMatrix;
  std::vector<PricingUnitFactoryBucket*>& _puFactoryBucketVect;
  ErrorResponseException _error;
  bool _itinPassed;
};
}
