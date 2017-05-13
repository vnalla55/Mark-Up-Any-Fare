/*---------------------------------------------------------------------------
 *  File:    SurchargesThreadTask.h
 *
 *  Copyright Sabre 2013
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseEnums.h"
#include "Pricing/SurchargesPreValidator.h"

namespace tse
{

class FareMarket;
class PricingTrx;

class SurchargesThreadTask : public TseCallableTrxTask
{

public:
  SurchargesThreadTask(PricingTrx& trx, SurchargesPreValidator& validator, FareMarket& fareMarket)
    : _trx(trx),
      _validator(validator),
      _fareMarket(fareMarket),
      _error(ErrorResponseException::NO_ERROR)
  {
    TseCallableTrxTask::trx(&_trx);
    TseCallableTrxTask::desc("SUR AND YQYR TASK");
  }

  const ErrorResponseException& error() const { return _error; }

  void performTask() override
  {
    try { _validator.process(_fareMarket); }
    catch (ErrorResponseException& ere) { _error = ere; }
    catch (...)
    {
      _error = ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION,
                                      "UNKNOWN ERROR DURING PRICING");
    }
  }

private:
  PricingTrx& _trx;
  SurchargesPreValidator& _validator;
  FareMarket& _fareMarket;
  ErrorResponseException _error;
};
}
