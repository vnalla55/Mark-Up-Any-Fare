//-------------------------------------------------------------------
//
//  File:        FareDisplayBlackoutDates.cpp
//  Authors:     Lipika Bardalai
//  Created:     March 2005
//  Description: This class contains blackout dates validation
//               for Fare Display, reusing existing functionality.
//               Method - process is overidden to reuse functionality
//               from base class and alse adds new semantics for
//               Fare Display.
//
//  Copyright Sabre 2001
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//---------------------------------------------------------------------

#include "Rules/FareDisplayBlackoutDates.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BlackoutInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag311Collector.h"
#include "Rules/BlackoutDates.h"
#include "Rules/DatePredicates.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

namespace tse
{
// -----------------------------------------------------------------------
//
// @MethodName FareDisplayBlackoutDates::process()
//
// process() is overriden in this class to reuse the common validation
// of BlackoutInfo attributes. Will have specific logic when date
// ranges are feasible. Also, return a PASS for a SOFTPASS from base
// functionality
//
// @param paxTypeFare - a PaxTypeFare object
// @param trx         - a PricingTrx object
//
// @return            - a Record3ReturnTypes value to determine if this
//                      category validation for FD is PASS or FAIL
//
// -----------------------------------------------------------------------

Record3ReturnTypes
FareDisplayBlackoutDates::process(PaxTypeFare& paxTypeFare, PricingTrx& trx, bool isInbound)
{
  LOG4CXX_INFO(_logger, "Entered FareDisplayBlackoutDates::process()");

  //--------------------------------------------------------------
  // Get a Fare Display Transaction from the Pricing Transaction
  //--------------------------------------------------------------
  // lint -e{1502}
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_DEBUG(_logger, "Unable to get FareDisplayTrx");
    LOG4CXX_INFO(_logger, " Leaving FareDisplayBlackoutDates::process() - FAIL");

    return FAIL;
  }
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);

  if (diagPtr == nullptr)
    LOG4CXX_DEBUG(_logger, "Diagnostic pointer is null");

  DiagCollector& diag = *diagPtr;
  LOG4CXX_DEBUG(_logger, "Retrieved Diagnostic");

  if (UNLIKELY(diag.diagnosticType() == Diagnostic311))
    diag.enable(Diagnostic311);

  const BlackoutInfo& blackoutInfo = info();

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (blackoutInfo.unavailtag() == UNAVAILABLE)
  {
    //-------------------------------------------------------------------
    // Get Fare Display Info object
    //-------------------------------------------------------------------
    FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

    if (!fdInfo)
    {
      LOG4CXX_DEBUG(_logger, "Unable to get FareDisplayInfo object");
      LOG4CXX_INFO(_logger, " Leaving FareDisplayBlackoutDates::process() - FAIL");

      if (UNLIKELY(diag.isActive()))
      {
        diag << "BLACKOUT: FAILED - UNABLE TO GET FAREDISPLAY INFO\n";
        diag.flushMsg();
      }
      return FAIL;
    }
    else
    {
      //-------------------------------------------------------------------
      // Update FareDisplayInfo object: Unavailable rule data
      //-------------------------------------------------------------------
      LOG4CXX_INFO(_logger, " Updating FareDisplayInfo object");

      fdInfo->setUnavailableR3Rule(RuleConst::BLACKOUTS_RULE);
    }

    LOG4CXX_INFO(_logger, " Leaving FareDisplayBlackoutDates::process() - NOTPROCESSED");
    if (UNLIKELY(diag.isActive()))
    {
      diag << "BLACKOUT: NOT PROCESSED - UNAVAILABLE RULE INFO\n";
      diag.flushMsg();
    }
    return NOTPROCESSED;
  }

  //----------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //----------------------------------------------------------------
  FareDisplayOptions* fdOptions = fdTrx->getOptions();

  if (fdOptions && !fdOptions->isValidateRules())
  {
    LOG4CXX_INFO(_logger, " No Validation qualifier - skip Blackout Dates Validation");
    return NOTPROCESSED;
  }

  // validate common functionality
  RuleConst::TSIScopeParamType scope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  _travelSegs = &paxTypeFare.fareMarket()->travelSeg();
  Record3ReturnTypes retval1 = processCommon(trx, paxTypeFare, blackoutInfo, scope, isInbound);

  LOG4CXX_INFO(_logger, "Leaving FareDisplayBlackoutDates::process()");

  if (retval1 == SOFTPASS)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "BLACKOUT: PASSED\n";
      diag.flushMsg();
    }

    return PASS;
  }
  if (UNLIKELY(diag.isActive()))
  {
    if (retval1 == PASS)
    {
      diag << "BLACKOUT: PASSED\n";
      diag.flushMsg();
    }
  }
  return retval1;
}

IsDateBetween*
FareDisplayBlackoutDates::createDateBetween()
{
  IsDateRangeBetween* dateBetween;
  _dataHandle.get(dateBetween);

  return dateBetween;
}

Predicate*
FareDisplayBlackoutDates::getDateBetweenPredicate(const BlackoutInfo& info)
{
  return getDatePredicate<IsDateRangeBetween>(info);
}

} // tse
