//-------------------------------------------------------------------
//
//  File:        FDEligibility.cpp
//
//  Authors:     Marco cartolano
//  Created:     May 23, 2005
//  Description: Validate method for Fare Display
//
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
//------------------------------------------------------------------

#include "Rules/FDEligibility.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/EligibilityInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{

static Logger
logger("atseintl.Rules.FDEligibility");

//----------------------------------------------------------------------------
// validate()
//----------------------------------------------------------------------------
Record3ReturnTypes
FDEligibility::validate(PricingTrx& trx,
                        Itin& itin,
                        PaxTypeFare& paxTypeFare,
                        const RuleItemInfo* rule,
                        const FareMarket& fareMarket,
                        const bool& isQualifyingCat,
                        const bool& isCat15Qualifying)
{
  LOG4CXX_INFO(logger, " Entered FDEligibility::validate()");

  const EligibilityInfo* eligibilityInfo = dynamic_cast<const EligibilityInfo*>(rule);

  if (!eligibilityInfo)
    return FAIL;

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (eligibilityInfo->unavailTag() == dataUnavailable)
  {
    //--------------------------------------------------------------
    // Get a Fare Display Transaction from the Pricing Transaction
    //--------------------------------------------------------------

    if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
    {
      LOG4CXX_DEBUG(logger, "Unable to get FareDisplayTrx");
      LOG4CXX_INFO(logger, " Leaving FDEligibility::validate() - FAIL");

      return FAIL;
    }

    //-------------------------------------------------------------------
    // Get Fare Display Info object
    //-------------------------------------------------------------------
    FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

    if (!fdInfo)
    {
      LOG4CXX_DEBUG(logger, "Unable to get FareDisplayInfo object");
      LOG4CXX_INFO(logger, " Leaving FDEligibility::validate() - FAIL");

      return FAIL;
    }
    else
    {
      //-------------------------------------------------------------------
      // Update FareDisplayInfo object: Unavailable rule data
      //-------------------------------------------------------------------
      LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

      fdInfo->setUnavailableR3Rule(RuleConst::ELIGIBILITY_RULE);
    }

    //-------------------------------------------------------------------
    // For Diagnostic display
    //-------------------------------------------------------------------
    DCFactory* factory = nullptr;
    DiagCollector* diagPtr = nullptr;
    bool diagEnabled = false;

    updateDiagnostic301(trx, paxTypeFare, eligibilityInfo, factory, diagPtr, diagEnabled);

    if (diagEnabled)
    {
      (*diagPtr) << " ELIGIBILITY: NOT PROCESSED - DATA UNAVAILABLE" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving FDEligibility::validate() - NOTPROCESSED");

    return NOTPROCESSED;
  }

  //----------------------------------------------------------------------
  // Call base class (Eligibility) function to do the validation
  //
  // 11/3/05  Changed to call method in derived class.
  //----------------------------------------------------------------------
  LOG4CXX_INFO(logger, " Calling FDEligibility::validate()");

  Record3ReturnTypes ret =
      fdValidate(trx, paxTypeFare, rule, fareMarket, isQualifyingCat, isCat15Qualifying);

  if (ret == SOFTPASS)
  {
    // For Fare Display there's no softpass
    ret = PASS;
  }

  if (ret == PASS && !eligibilityInfo->acctCode().empty())
  {
    paxTypeFare.setMatchedCorpID();
  }

  LOG4CXX_INFO(logger, " Leaving FDEligibility::validate() - " << ret);
  return ret;
}

//----------------------------------------------------------------------
// 11/3/05  Clone validate method from base class until it can be
//          refactored.  Match pax type status method must be bypassed
//          for Fare Display.  It is too risky to refactor the base
//          class this close to pricing's prod implementation.
//----------------------------------------------------------------------
Record3ReturnTypes
FDEligibility::fdValidate(PricingTrx& trx,
                          PaxTypeFare& paxTypeFare,
                          const RuleItemInfo* rule,
                          const FareMarket& fareMarket,
                          const bool& isQualifyingCat,
                          const bool& isCat15Qualifying)
{
  const EligibilityInfo* eligibilityInfo = dynamic_cast<const EligibilityInfo*>(rule);

  Record3ReturnTypes retval;

  //-------------------------------------------------------------------
  // For Diagnostic display
  //-------------------------------------------------------------------
  DCFactory* factory = nullptr;
  DiagCollector* diagPtr = nullptr;
  bool diagEnabled = false;

  updateDiagnostic301(trx, paxTypeFare, eligibilityInfo, factory, diagPtr, diagEnabled);

  //-------------------------------------------------------------------
  // Validate Unavailable Tag
  //-------------------------------------------------------------------
  retval = checkUnavailableAndText(eligibilityInfo);

  if (retval == FAIL)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED - CHECK UNAVAILABLE" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");
    return FAIL;
  }
  else if (retval == SKIP)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP - TEXT ONLY" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");
    return SKIP;
  }

  //-------------------------------------------------------------------
  // Validate Passenger Type
  //-------------------------------------------------------------------
  retval =
      checkPTC(eligibilityInfo, paxTypeFare, trx, factory, diagPtr, diagEnabled, isQualifyingCat);

  if (retval == FAIL)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED - CHECK PTC" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");
    return FAIL;
  }
  else if (retval == SKIP)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: SKIP - NO MATCH ON PSGR TYPE" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - SKIP");
    return SKIP;
  }

  //-------------------------------------------------------------------
  // Validate Account Code
  //-------------------------------------------------------------------
  if (checkAccountCode(
          eligibilityInfo, paxTypeFare, trx, factory, isCat15Qualifying, diagPtr, diagEnabled) ==
      FAIL)
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diagPtr) << " ELIGIBILITY: FAILED - ACCOUNT CODE" << std::endl;
      diagPtr->flushMsg();
    }

    LOG4CXX_INFO(logger, " Leaving Eligibility::validate() - FAIL");
    return FAIL;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diagPtr) << "PASS" << std::endl;
    diagPtr->flushMsg();
  }

  LOG4CXX_INFO(logger, " Leaving FDEligibility::validate() - PASS");
  return PASS;
}

void
FDEligibility::updateDiagnostic301(PricingTrx& trx,
                                   PaxTypeFare& paxTypeFare,
                                   const EligibilityInfo*& eligibilityInfo,
                                   DCFactory*& factory,
                                   DiagCollector*& diagPtr,
                                   bool& diagEnabled)
{
  if (trx.diagnostic().diagnosticType() == Diagnostic301)
  {
    factory = DCFactory::instance();
    diagPtr = factory->create(trx);
    diagPtr->enable(Diagnostic301);
    diagEnabled = true;

    (*diagPtr) << "FARE PAX TYPE : " << paxTypeFare.fcasPaxType()
               << " MIN AGE : " << paxTypeFare.fcasMinAge()
               << " MAX AGE : " << paxTypeFare.fcasMaxAge() << std::endl
               << "R3 PAX TYPE   : " << eligibilityInfo->psgType()
               << " MIN AGE : " << eligibilityInfo->minAge()
               << " MAX AGE : " << eligibilityInfo->maxAge() << std::endl
               << "R3 ACCOUNT CODE/CORP ID : " << eligibilityInfo->acctCode() << std::endl;
  }
}

} // tse
