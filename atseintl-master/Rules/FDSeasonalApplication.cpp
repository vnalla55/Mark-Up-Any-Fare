//-------------------------------------------------------------------
//
//  File:        FDSeasonalApplication.cpp
//  Authors:     Lipika Bardalai
//  Created:     March 2005
//  Description: This class contains SeasonalApplication validation
//               for Fare Display, reusing existing functionality.
//               Method - validate is overidden to reuse functionality
//               from base class, adding new semantics for
//               Fare Display.
//
//               08/08/05 Tony Lam - Add new methods to validate date range
//
//  Copyright Sabre 2001
//              The copyright to the computer program(s) herein
//              is the property of Sabre.
//              The program(s) may be used and/or copied only with
//              the written permission of Sabre or in accordance
//              with the terms and conditions stipulated in the
//              agreement/contract under which the program(s)
//              have been supplied.
//
//---------------------------------------------------------------------

#include "Rules/FDSeasonalApplication.h"

#include "Common/Logger.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/SeasonalAppl.h"
#include "Diagnostic/DiagManager.h"
#include "Util/BranchPrediction.h"

namespace tse
{

Logger
FDSeasonalApplication::_logger("atseintl.Rules.FDSeasonalApplication");

Record3ReturnTypes
FDSeasonalApplication::validate(PricingTrx& trx,
                                Itin& itin,
                                const PaxTypeFare& paxTypeFare,
                                const RuleItemInfo* rule,
                                const FareMarket& fareMarket,
                                bool isQualifiedCategory,
                                bool isInbound)
{
  LOG4CXX_INFO(_logger, " Entered FDSeasonalApplication::validate() " << paxTypeFare.fareClass());

  const FareDisplayTrx* fareDisplayTrx = dynamic_cast<const FareDisplayTrx*>(&trx);

  if (!fareDisplayTrx)
    return FAIL;

  //-------------------------------------------------------------------
  // Get Fare Display Info object
  //-------------------------------------------------------------------

  DiagManager diag(trx, Diagnostic303);

  FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());

  if (!fareDisplayInfo)
  {
    LOG4CXX_ERROR(_logger, "Unable to get FareDisplayInfo object");
    if (diag.isActive())
    {
      diag << "SEASONS: FAIL "
           << "- UNABLE TO GET FAREDISPLAY INFO\n";
    }
    return FAIL;
  }

  //-------------------------------------------------------------------
  // Add Seasonal Application information to FareDisplayInfo object
  //-------------------------------------------------------------------
  const SeasonalAppl* seasonRule = dynamic_cast<const SeasonalAppl*>(rule);
  if (!seasonRule)
  {
    if (diag.isActive())
    {
      diag << "SEASONS: FAIL "
           << "- UNABLE TO GET SEASON RULE INFO\n";
    }
    return FAIL;
  }

  const SeasonalAppl& seasonalRuleInfo = *seasonRule;

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (seasonalRuleInfo.unavailtag() == dataUnavailable)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(_logger, " Updating FareDisplayInfo object");

    fareDisplayInfo->setUnavailableR3Rule(RuleConst::SEASONAL_RULE);

    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::validate() - NOTPROCESSED");
    if (diag.isActive())
    {
      diag << "SEASONS: NOT PROCESSED "
           << "- CHECK UNAVAILABLE TAG\n";
    }

    // return NOTPROCESSED;
    // Skip this Record 3 and continue with the next one
    return SKIP;
  }
  //----------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //----------------------------------------------------------------
  const FareDisplayOptions* fdOptions = fareDisplayTrx->getOptions();

  if (fdOptions && !fdOptions->isValidateRules())
  {
    LOG4CXX_INFO(_logger, " No Validation qualifier - skip Seasonal Application Validation");
    if (diag.isActive())
    {
      diag << "SEASONS: NOT PROCESSED "
           << "- NO VALIDATION QUALIFIER\n";
    }

    // Collect seasons for VN display
    if (!isQualifiedCategory)
    {
      updateFareDisplayInfo(seasonalRuleInfo, fareDisplayInfo);
    }

    return SKIP;
  }

  //-------------------------------------------------------------------
  // Validate Seasonal Application data
  //-------------------------------------------------------------------
  Record3ReturnTypes ret = checkUnavailableAndText(seasonRule);
  if (ret == FAIL || ret == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::checkUnavailableAndText() - " << ret);
    if (diag.isActive())
    {
      if (ret == FAIL)
      {
        diag << "SEASONS: FAIL "
             << "- CHECK UNAVAILABLE\n";
      }
      else if (ret == SKIP)
      {
        diag << "SEASONS: SKIP "
             << "- TEXT ONLY\n";
      }
    }
    return ret;
  }

  if (UNLIKELY(diag.isActive()))
    diagRestriction(*seasonRule, diag);

  const FareMarket& fm = *fareDisplayTrx->inboundFareMarket();

  //---------------------------------------------------------------------------
  // Set scope type
  // System assumption:
  // - Validate Seasons dates against the departure date from the origin
  //   of the Pricing Unit (subjourney);
  // - Assumption Override Tag may be set indicate the fare component trip date
  //   must be use to validate against season dates
  //---------------------------------------------------------------------------
  // validate any system assumption override
  Record3ReturnTypes retval = FAIL;

  // Check for bad data
  if (seasonRule->tvlstartyear() == 0 && seasonRule->tvlstartmonth() == 0 &&
      seasonRule->tvlstartDay() == 0 && seasonRule->tvlStopyear() == 0 &&
      seasonRule->tvlStopmonth() == 0 && seasonRule->tvlStopDay() == 0)
  {
    if (diag.isActive())
    {
      diag << "SEASONS: SKIP - BAD DATA\n";
    }
    return SKIP;
  }

  if (isInbound)
  {
    LOG4CXX_INFO(_logger, " Entering FDSeasonalApplication::checkOverrideForDateRange() - Inbound");
    if (seasonRule->assumptionOverride() != 'X')
      retval = FAIL;
    else
      retval = checkOverrideForDateRange(seasonRule, fm);
  }
  else
  {
    LOG4CXX_INFO(_logger,
                 " Entering FDSeasonalApplication::checkOverrideForDateRange() - Outbound");
    retval = checkOverrideForDateRange(seasonRule, fareMarket);
  }
  if (retval == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::checkOverrideForDateRange() - FAIL");
    if (diag.isActive())
    {
      diag << " SEASONS: FAIL - NO DATES IN SEASONS\n";
    }
    return FAIL;
  }
  else if (retval == PASS && !isInbound)
  {
    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::checkAssumptionOverride() - PASS");

    if (!isQualifiedCategory)
    {
      updateFareDisplayInfo(seasonalRuleInfo, fareDisplayInfo);
    }

    if (diag.isActive())
    {
      diag << " SEASONS: PASS - ALL DATES IN SEASONS\n";
    }

    return PASS;
  }

  // validate table 995
  if (isInbound)
  {
    LOG4CXX_INFO(_logger, " FDSeasonalApplication::checkGeoTblItemNo() For Inbound");
    retval = checkGeoForDateRange(seasonRule, paxTypeFare, fm, itin, trx);
  }
  else
  {
    LOG4CXX_INFO(_logger, " FDSeasonalApplication::checkGeoTblItemNo() For Outbound");
    retval = checkGeoForDateRange(seasonRule, paxTypeFare, fareMarket, itin, trx);
  }
  if (retval == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::checkGeoTblItemNo() - SKIP");
    if (diag.isActive())
    {
      diag << " SEASONS: SKIP - NO GEO MATCH FROM TABLE 995 \n";
    }

    return SKIP;
  }
  else if (retval == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::checkGeoTblItemNo() - FAIL");
    if (diag.isActive())
    {
      diag << " SEASONS: FAILED -  NO DATES IN SEASONS\n";
    }
    return FAIL;
  }

  LOG4CXX_INFO(_logger,
               " Leaving FDSeasonalApplication::validate() ALL ATTRIBUTES VALIDATED - PASS");

  if (diag.isActive())
  {
    diag << " SEASONS: PASSED - COMPLETED ALL VALIDATION CODE\n";
  }
  retval = PASS;

  if (!isQualifiedCategory)
  {
    updateFareDisplayInfo(seasonalRuleInfo, fareDisplayInfo);
  }

  return PASS;
}

void
FDSeasonalApplication::updateFareDisplayInfo(const SeasonalAppl& seasonalRuleInfo,
                                             FareDisplayInfo*& fdInfo) const
{
  LOG4CXX_INFO(_logger, " Entered FDSeasonalApplication::updateFareDisplayInfo()");
  Indicator dir;
  if (_rule->inOutInd() == RuleConst::ALWAYS_APPLIES)
  {
    dir = BLANK;
    LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo()::ALWAYS_APPLIES");
  }
  else
  {
    if (_rule->inOutInd() == OUTBOUND)
    {
      dir = OUTBOUND;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: OUTBOUND");
    }
    else if (_rule->inOutInd() == INBOUND)
    {
      dir = INBOUND;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: INBOUND");
    }
    else
    {
      dir = BLANK;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: BLANK");
    }
  }

  fdInfo->addSeason(dir,
                    seasonalRuleInfo.tvlstartyear(),
                    seasonalRuleInfo.tvlstartmonth(),
                    seasonalRuleInfo.tvlstartDay(),
                    seasonalRuleInfo.tvlStopyear(),
                    seasonalRuleInfo.tvlStopmonth(),
                    seasonalRuleInfo.tvlStopDay());

  LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::updateFareDisplayInfo()");
}

void
FDSeasonalApplication::updateFareDisplayInfo(const RuleItemInfo* rule, FareDisplayInfo*& fdInfo)
    const
{
  //-------------------------------------------------------------------
  // Add Seasonal Application information to FareDisplayInfo object
  //-------------------------------------------------------------------
  const SeasonalAppl* seasonRule = dynamic_cast<const SeasonalAppl*>(rule);
  if (!seasonRule)
    return;

  const SeasonalAppl& seasonalRuleInfo = *seasonRule;

  LOG4CXX_INFO(_logger, " Entered FDSeasonalApplication::updateFareDisplayInfo()");
  Indicator dir;
  if (_rule->inOutInd() == RuleConst::ALWAYS_APPLIES)
  {
    dir = BLANK;
    LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo()::ALWAYS_APPLIES");
  }
  else
  {
    if (_rule->inOutInd() == OUTBOUND)
    {
      dir = OUTBOUND;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: OUTBOUND");
    }
    else if (_rule->inOutInd() == INBOUND)
    {
      dir = INBOUND;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: INBOUND");
    }
    else
    {
      dir = BLANK;
      LOG4CXX_INFO(_logger, "FDSeasonalApplication::updateFareDisplayInfo():: BLANK");
    }
  }

  fdInfo->addSeason(dir,
                    seasonalRuleInfo.tvlstartyear(),
                    seasonalRuleInfo.tvlstartmonth(),
                    seasonalRuleInfo.tvlstartDay(),
                    seasonalRuleInfo.tvlStopyear(),
                    seasonalRuleInfo.tvlStopmonth(),
                    seasonalRuleInfo.tvlStopDay());

  LOG4CXX_INFO(_logger, " Leaving FDSeasonalApplication::updateFareDisplayInfo()");
}

} // tse
