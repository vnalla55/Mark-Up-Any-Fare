//----------------------------------------------------------------------------
//  File: FDMinStayApplication.cpp
//
//  Author: Partha Kumar Chakraborti
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

#include "Rules/FDMinStayApplication.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/MinStayRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/DayTimeApplication.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.Rules.FDMinStayApplication");

//-------------------------------------------------------------------
//   @method validateDate
//
//   Description: Verifies whether the date is a valid date or not.
//
//   @param const DateTime&           - date
//
// -------------------------------------------------------------------
bool
FDMinStayApplication::validateDate(const DateTime& date)
{
  return !date.isEmptyDate() && date.isValid();
}

//-------------------------------------------------------------------
//   @method fillInfoObject
//
//   Description: Coppies FareDisplayInfo object with values.
//
//   @param FareDisplayInfo&           - FareDisplayTrx object
//   @param const MinStayRestriction&  - rule object
//	 @return	: true or false.
//
// -------------------------------------------------------------------
void
FDMinStayApplication::fillInfoObject(FareDisplayInfo& fareDisplayInfo,
                                     const MinStayRestriction& minStayRule,
                                     const PaxTypeFare& paxTypeFare)
{
  const std::string threeDollar = "$$$"; // 3 dollar

  PaxTypeFareRuleData* ptfRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::MINIMUM_STAY_RULE);

  if (ptfRuleData)
  {
    if ((ptfRuleData->categoryRuleItemInfoVec()) &&
        (ptfRuleData->categoryRuleItemInfoVec()->size() > 1))
    {
      fareDisplayInfo.minStay() = threeDollar;
      return;
    }
  }

  PeriodOfStay minStayPeriod(minStayRule.minStay(), minStayRule.minStayUnit());

  LOG4CXX_DEBUG(logger,
                "MinStayPeriod: " << std::string(minStayPeriod)
                                  << " and MinStayUnit: " << minStayPeriod.unit());

  if (minStayPeriod.isDayOfWeek() || (minStayPeriod.unit() == PeriodOfStay::DAYS))
    fareDisplayInfo.minStay() = minStayRule.minStay();
  else
    fareDisplayInfo.minStay() = threeDollar; // put $$$.
}
//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a FareMarket and fillup
//                                FareDisplayInfo object with appropriate value.
//
//   @param PricingTrx           - Pricing transaction
//   @param Itin                 - itinerary
//   @param PaxTypeFare          - reference to Pax Type Fare
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param FareMarket           -  Fare Market
//
//   @return Record3ReturnTypes - possible values are:
//                                NOT_PROCESSED
//                                FAIL
//                                PASS
//                                SKIP
//                                STOP
//
//-------------------------------------------------------------------

Record3ReturnTypes
FDMinStayApplication::validate(PricingTrx& trx,
                               Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               const RuleItemInfo* rule,
                               const FareMarket& fareMarket,
                               bool isQualifiedCategory)
{
  LOG4CXX_INFO(logger, " Entered FDMinStayApplication::validate()");

  Record3ReturnTypes ret = FAIL;

  static const std::string threeBlankSpace = "   "; // 3 blank space

  // ---------------------------------------------------
  //         Get pointer to FareDisplayTrx object dynamic_cast ing from trx
  // ---------------------------------------------------
  FareDisplayTrx* fareDisplayTrx = nullptr;
  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fareDisplayTrx))
  {
    LOG4CXX_DEBUG(
        logger,
        "Unable to get FareDislayTrx object from PricingTrx object by dynamic cast. - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - FAIL");
    return FAIL;
  }

  // ---------------------------------------------------
  //         Get the MinStayRestriction dynamic_cast ing from rule
  // ---------------------------------------------------
  const MinStayRestriction* minStayRule = dynamic_cast<const MinStayRestriction*>(rule);

  if (!minStayRule)
  {
    LOG4CXX_DEBUG(logger, "Unable to get MinStayRestriction from the rule  - SKIP");
    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - SKIP");
    return SKIP;
  }

  LOG4CXX_DEBUG(logger,
                "Got pointer to MinStayRestriction, minStay="
                    << minStayRule->minStay() << ",minStayUnit:" << minStayRule->minStayUnit()
                    << ",");

  //----------------------------------------------------------------
  // Check if it is a request to exclude Min/Max Stay restricted
  // fares or to exclude any restricted fares
  //----------------------------------------------------------------
  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);

  if (diagPtr == nullptr)
    LOG4CXX_DEBUG(logger, "Diagnostic pointer is null");

  DiagCollector& diag = *diagPtr;
  LOG4CXX_DEBUG(logger, "Retrieved Diagnostic");

  if (UNLIKELY(diag.diagnosticType() == Diagnostic306))
    diag.enable(Diagnostic306);

  const FareDisplayOptions* fdOptions = fareDisplayTrx->getOptions();

  if (fdOptions && (fdOptions->isExcludeMinMaxStayFares() || fdOptions->isExcludeRestrictedFares()))
  {
    LOG4CXX_DEBUG(logger, "Exclude fares with Min/Max Stay restrictions - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - FAIL");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MINSTAY: FAILED - EXCLUDE FARES WITH MIN STAY RESTRICTIONS\n";
      diag.flushMsg();
    }

    return FAIL;
  }

  //----------------------------------------------------
  // Get the FareDisplayInfo Object .
  //----------------------------------------------------

  LOG4CXX_DEBUG(
      logger,
      "Trying to get FareDisplayInfo and determine if there is any Multiple Min Stay Condition");

  FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());

  if (!fareDisplayInfo)
  {
    LOG4CXX_DEBUG(logger, "Unable to get FareDisplayInfo object  - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - FAIL");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MINSTAY: FAILED - UNABLE TO GET FAREDISPLAYINFO\n";
      diag.flushMsg();
    }
    return FAIL;
  }

  LOG4CXX_DEBUG(logger,
                "FareDisplayInfo object contains, minStay=" << fareDisplayInfo->minStay() << ",");

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (minStayRule->unavailTag() == dataUnavailable)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

    fareDisplayInfo->setUnavailableR3Rule(RuleConst::MINIMUM_STAY_RULE);

    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - SKIP");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MINSTAY: SKIPPED - UNAVAILABLE RULE DATA\n";
      diag.flushMsg();
    }

    return SKIP;
  }

  //----------------------------------------------------
  // Verify that we have valid date
  //----------------------------------------------------
  if (!validateDate(fareDisplayTrx->getRequest()->returnDate()))
  {
    LOG4CXX_DEBUG(
        logger,
        "An invalid return date or empty return date found in FareDisplayTrx object - SKIP ");
    LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - SKIP");

    if (!isQualifiedCategory)
    {
      fillInfoObject(*fareDisplayInfo, *minStayRule, paxTypeFare);
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << " MINSTAY: SKIPPED - INVALID OR EMPTY RETURN DATE\n";
      diag.flushMsg();
    }

    return SKIP;
  }

  //-----------------------------------------------------------
  // Initializing FareDisplayInfo Object with 3 blank spaces
  //-----------------------------------------------------------
  fareDisplayInfo->minStay() = threeBlankSpace; // set "   " ( 3 blank space characters )

  //----------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //----------------------------------------------------------------
  if (fdOptions && !fdOptions->isValidateRules())
  {
    LOG4CXX_INFO(logger, " No Validation qualifier - skip Minimum Stay Validation");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MINSTAY: SKIPPED - NO VALIDATION QUALIFIER\n";
      diag.flushMsg();
    }
    ret = SKIP;
  }
  else
  {
    //-----------------------------------------------------------
    // Call base class ( PricingTrx ) function to validate for Fare Display.
    //-----------------------------------------------------------
    ret =
        MinimumStayApplication::validate(trx, fareDisplayTrx, fareMarket, itin, paxTypeFare, rule);

    if (ret == FAIL)
    {
      LOG4CXX_DEBUG(logger, "Base class validate method failed. - FAIL");
      LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - FAIL");
      if (UNLIKELY(diag.isActive()))
      {
        diag << " MINSTAY: FAILED\n";
        diag.flushMsg();
      }
      return FAIL;
    }
  }

  //-----------------------------------------------------------
  // Fillup FareDisplayInfo object with appropriate values.
  //-----------------------------------------------------------
  if (!isQualifiedCategory)
  {
    fillInfoObject(*fareDisplayInfo, *minStayRule, paxTypeFare);
  }

  //-----------------------------------------------------------
  // Leaving FDMinStayApplication :: Validate()
  //-----------------------------------------------------------
  LOG4CXX_INFO(logger, " Leaving FDMinStayApplication::validate() - " << ret);
  if (UNLIKELY(diag.isActive()))
  {
    if (ret == PASS)
    {
      diag << " MINSTAY: PASSED\n";
      diag.flushMsg();
    }
  }

  return ret;
}

} // tse
