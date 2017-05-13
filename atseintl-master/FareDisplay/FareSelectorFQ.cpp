//-------------------------------------------------------------------
//
//  File:        FareSelectorFQ.h
//  Created:     October 7, 2005
//  Authors:     Doug Batchelor
//
//  Updates:
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FareSelectorFQ.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "BookingCode/RBData.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TSEException.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/NegFareRest.h"
#include "FareDisplay/InclusionCodeConsts.h"

#include <vector>

namespace tse
{
// log4cxx::LoggerPtr
// FareSelector::_logger(log4cxx::Logger::getLogger("atseintl.FareDisplay.FareSelector"));

// -------------------------------------------------------------------
//
// @MethodName  FareSelectorFQ::selectFares()
//
// Iterate through all PaxTypefares to determine
// which ones are valid.
//
// @param trx - a reference to a FareDisplayTrx.
//
// @return bool
//
// -------------------------------------------------------------------------

bool
FareSelectorFQ::selectFares(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(_logger, " Entered FareSelectorFQ::selectFares()");

  DataHandle dataHandle(trx.ticketingDate());
  TseRunnableExecutor taskExecutor(TseThreadingConst::FARE_SELECTOR_TASK);
  std::vector<QualifyFareTask*> taskVec;

  // Set Alternate Currency
  CurrencyCode alternateCurrency = EMPTY_STRING();
  if (trx.getOptions() != nullptr && !trx.getOptions()->alternateCurrency().empty())
  {
    alternateCurrency = trx.getOptions()->alternateCurrency();
  }

  Itin* itin = trx.itin().front();

  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    QualifyFareTask* task = nullptr;
    dataHandle.get(task);
    taskVec.push_back(task);

    task->_alternateCurrency = alternateCurrency;
    task->_fareMarket = &fareMarket;
    task->_fareSelector = this;
    task->trx(&trx);

    taskExecutor.execute(*task);
  }
  taskExecutor.wait();

  std::vector<QualifyFareTask*>::const_iterator taskIt = taskVec.begin();
  std::vector<QualifyFareTask*>::const_iterator taskItEnd = taskVec.end();

  for (; taskIt != taskItEnd; taskIt++)
  {
    if ((*taskIt)->_fareSelected)
      return true;
  }

  return false;
}

void
FareSelectorFQ::QualifyFareTask::performTask()
{
  FareDisplayTrx* fdTrx = dynamic_cast<FareDisplayTrx*>(trx());

  if (fdTrx == nullptr)
  {
    LOG4CXX_ERROR(_logger, "performTask: NULL trx pointer!");
    return;
  }

  std::vector<PaxTypeFare*>::const_iterator ptfItr = _fareMarket->allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfEnd = _fareMarket->allPaxTypeFare().end();

  for (; ptfItr != ptfEnd; ptfItr++)
  {
    PaxTypeFare* ptFare = (*ptfItr);

    if (ptFare == nullptr || !ptFare->isValid())
      continue;

    if (_fareSelector->qualifyThisFare(*fdTrx, *ptFare))
      _fareSelected = true;
  }

  _fareSelector->qualifyOutboundCurrency(*fdTrx, *_fareMarket, _alternateCurrency);
}
}
