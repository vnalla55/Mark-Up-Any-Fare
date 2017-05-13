

//----------------------------------------------------------------------------
//  File: FDMaxStayApplication.cpp
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

#include "Rules/FDMaxStayApplication.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Rules/PeriodOfStay.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.Rules.FDMaxStayApplication");

//-------------------------------------------------------------------
//   @method fillInfoObject
//
//   Description: Coppies FareDisplayInfo object with values.
//
//   @param FareDisplayInfo&           - FareDisplayTrx object
//   @param const MaxStayRestriction&  - rule object
//	 @return	: true or false.
//
// -------------------------------------------------------------------
void
FDMaxStayApplication::fillInfoObject(FareDisplayInfo* fareDisplayInfo,
                                     const MaxStayRestriction* maxStayRule,
                                     const PaxTypeFare& paxTypeFare)
{
  static const std::string threeDollar = "$$$"; // "$$$"
  static const std::string twoBlankSpace = "  "; // "  "
  static const std::string threeBlankSpace = "   "; // "   "

  PaxTypeFareRuleData* ptfRuleData = paxTypeFare.paxTypeFareRuleData(RuleConst::MAXIMUM_STAY_RULE);

  if (ptfRuleData)
  {
    if ((ptfRuleData->categoryRuleItemInfoVec()) &&
        (ptfRuleData->categoryRuleItemInfoVec()->size() > 1))
    {
      fareDisplayInfo->maxStay() = threeDollar;
      fareDisplayInfo->maxStayUnit() = twoBlankSpace;
      LOG4CXX_DEBUG(logger, "Multiple Max Stay Condition exist. maxStay of fareDisplayInfo object "
                             "has been filled up with $$$");
      return;
    }
  }
  PeriodOfStay maxStayPeriod(maxStayRule->maxStay(), maxStayRule->maxStayUnit());
  fareDisplayInfo->maxStay() = maxStayRule->maxStay();
  if (maxStayPeriod.isDayOfWeek())
    fareDisplayInfo->maxStayUnit() = twoBlankSpace;
  else
    fareDisplayInfo->maxStayUnit() = maxStayRule->maxStayUnit();
}

//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a FareMarket and fillup
//								FareDisplayInfo object with appropriate value.
//
//   @param PricingTrx           - Pricing transaction
//   @param Itin                 - itinerary
//   @param PaxTypeFare          - reference to Pax Type Fare
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param FareMarket           -  Fare Market
//
//   @return Record3ReturnTypes - possible values are:
//                                NOT_PROCESSED = 1
//                                FAIL          = 2
//                                PASS          = 3
//                                SKIP          = 4
//                                STOP          = 5
//
//-------------------------------------------------------------------
Record3ReturnTypes
FDMaxStayApplication::validate(PricingTrx& trx,
                               Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               const RuleItemInfo* rule,
                               const FareMarket& fareMarket,
                               bool isQualifiedCategory)
{
  LOG4CXX_INFO(logger, " Entered FDMaxStayApplication::validate() = " << paxTypeFare.fareClass());

  Record3ReturnTypes ret = FAIL;

  // ---------------------------------------------------------
  // 		Get FareDisplayTrx object by dynamic casting from trx
  // ---------------------------------------------------------
  FareDisplayTrx* fareDisplayTrx = nullptr;
  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fareDisplayTrx))
  {
    LOG4CXX_DEBUG(
        logger,
        "Unable to get FareDislayTrx object from PricingTrx object by dynamic cast. - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - FAIL");
    return FAIL;
  }

  // ---------------------------------------------------------
  // 		Get MaxStayRestriction from rule
  // ---------------------------------------------------------
  const MaxStayRestriction* maxStayRule = dynamic_cast<const MaxStayRestriction*>(rule);
  if (!maxStayRule)
  {
    LOG4CXX_DEBUG(logger, "Unable to get MaxStayRestriction from the rule  - SKIP");
    LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - SKIP");
    return SKIP;
  }

  LOG4CXX_DEBUG(logger,
                "Got pointer to MaxStayRestriction, maxStay="
                    << maxStayRule->maxStay() << ",maxStayUnit:" << maxStayRule->maxStayUnit()
                    << ",");

  DCFactory* factory = DCFactory::instance();
  DiagCollector* diagPtr = factory->create(trx);

  if (diagPtr == nullptr)
    LOG4CXX_DEBUG(logger, "Diagnostic pointer is null");

  DiagCollector& diag = *diagPtr;
  LOG4CXX_DEBUG(logger, "Retrieved Diagnostic");

  if (UNLIKELY(diag.diagnosticType() == Diagnostic307))
    diag.enable(Diagnostic307);
  // Check if it is a request to exclude Min/Max Stay restricted
  // fares or to exclude any restricted fares
  //----------------------------------------------------------------
  const FareDisplayOptions* fdOptions = fareDisplayTrx->getOptions();

  if (fdOptions && (fdOptions->isExcludeMinMaxStayFares() || fdOptions->isExcludeRestrictedFares()))
  {
    // XS or XR don't apply to Max Stay of one year
    // Check if restriction is 365 days or 12 months
    PeriodOfStay maxStayPeriod(maxStayRule->maxStay(), maxStayRule->maxStayUnit());

    if (!maxStayPeriod.isOneYear())
    {
      LOG4CXX_DEBUG(logger, "Exclude fares with Min/Max Stay restrictions - FAIL");
      LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - FAIL");
      if (UNLIKELY(diag.isActive()))
      {
        diag << " MAXSTAY: FAILED - EXCLUDE FARES WITH MIN STAY RESTRICTIONS\n";
        diag.flushMsg();
      }

      return FAIL;
    }
  }

  // ---------------------------------------------------------
  // 		Get FareDisplayInfo object
  // ---------------------------------------------------------

  LOG4CXX_DEBUG(
      logger,
      "Trying to get FareDisplayInfo and determine if there is any Multiple Max Stay Condition");

  FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());
  if (!fareDisplayInfo)
  {
    LOG4CXX_DEBUG(logger, "Unable to get FareDisplayInfo object  - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - FAIL");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MAXSTAY: FAILED - UNABLE TO GET FAREDISPLAYINFO\n";
      diag.flushMsg();
    }

    return FAIL;
  }

  LOG4CXX_DEBUG(logger,
                "FareDisplayInfo object contains, maxStay="
                    << fareDisplayInfo->maxStay()
                    << ",maxStayUnit:" << fareDisplayInfo->maxStayUnit() << ",");
  if (!isQualifiedCategory)
  {
    fillInfoObject(fareDisplayInfo, maxStayRule, paxTypeFare);
  }

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (maxStayRule->unavailTag() == dataUnavailable)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

    fareDisplayInfo->setUnavailableR3Rule(RuleConst::MAXIMUM_STAY_RULE);

    LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - NOTPROCESSED");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MAXSTAY: NOT PROCESSED - UNAVAILABLE RULE DATA\n";
      diag.flushMsg();
    }

    return NOTPROCESSED;
  }

  // ----------------------------------------------------
  // 	Verify that we have valid date
  // ----------------------------------------------------
  if (!fareDisplayTrx->getRequest()->returnDate().isValid())
  {
    LOG4CXX_DEBUG(
        logger,
        "An invalid return date or empty return date found in FareDisplayTrx object - PASS ");
    LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - PASS ");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MAXSTAY: PASSED - INVALID OR EMPTY RETURN DATE\n";
      diag.flushMsg();
    }

    return PASS;
  }

  //----------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //----------------------------------------------------------------
  if (fdOptions && !fdOptions->isValidateRules())
  {
    LOG4CXX_INFO(logger, " No Validation qualifier - skip Maximum Stay Validation");
    if (UNLIKELY(diag.isActive()))
    {
      diag << " MAXSTAY: NOT PROCESSED - NO VALIDATION QUALIFIER\n";
      diag.flushMsg();
    }

    ret = NOTPROCESSED;
  }
  else
  {
    // ---------------------------------------------------------
    // 		Call base class ( Pricingtrx ) validate method
    // ---------------------------------------------------------
    ret =
        MaximumStayApplication::validate(trx, fareDisplayTrx, fareMarket, itin, paxTypeFare, rule);
    if (ret == FAIL)
    {
      LOG4CXX_DEBUG(logger, "Base class validate method failed. - FAIL");
      LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - FAIL");
      if (UNLIKELY(diag.isActive()))
      {
        diag << " MAXSTAY: FAILED\n";
        diag.flushMsg();
      }
      return FAIL;
    }
  }

  LOG4CXX_INFO(logger, " Leaving FDMaxStayApplication::validate() - " << ret);

  if (UNLIKELY(diag.isActive()))
  {
    if (ret == PASS)
    {
      diag << " MAXSTAY: PASSED\n";
      diag.flushMsg();
    }
  }
  return ret;
}
}
