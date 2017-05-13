
//----------------------------------------------------------------------------
//  File: FDDayTimeApplication.cpp
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

#include "Rules/FDDayTimeApplication.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.Rules.FDDayTimeApplication");

//----------------------------------------------------------------------------
//
// @Function    Record3ReturnType DayTimeApplication::validate
//
// Description: This function is to validate Record 3 Category 2
//
// @param       ruleArgs - Object that contains the Fare, Request, Journey,
//                         SubJourney, Itinerary and FareUsage objects, and
//                         the rule checker phase
//
// @return      pass     - fare is validated
//              fail     - fare fails this rule category restrictions
//              skip     - skip this rule category
//
// </PRE>
//----------------------------------------------------------------------------

Record3ReturnTypes
FDDayTimeApplication::validate(PricingTrx& trx,
                               Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               const RuleItemInfo* rule,
                               const FareMarket& fareMarket,
                               bool isQualifiedCategory,
                               bool isInbound)
{
  LOG4CXX_INFO(logger,
               " Entered FDDayTimeApplication::validate() - " << paxTypeFare.fareClass()
                                                              << " Rule: " << rule);

  Record3ReturnTypes retval = FAIL;

  //-------------------------------------------------------------------------
  // Get pointer to FareDisplayTrx object dynamic_cast ing from trx
  //-------------------------------------------------------------------------
  FareDisplayTrx* fareDisplayTrx = nullptr;

  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fareDisplayTrx))
  {
    LOG4CXX_DEBUG(
        logger,
        "Unable to get FareDislayTrx object from PricingTrx object by dynamic cast. - FAIL");
    LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - FAIL");
    return FAIL;
  }

  DiagManager diag(trx, Diagnostic302);

  // Temporary check to bypass Cat 2 validation for Intair Fun Sun agencies
  PseudoCityCode pcc = fareDisplayTrx->getRequest()->ticketingAgent()->tvlAgencyPCC();

  if (pcc == "BB50" || pcc == "7WOB" || pcc == "9RN1" || pcc == "BU31" || pcc == "FY57" ||
      pcc == "U557" || pcc == "LG4A")
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " DAY/TIME APPL: NOT PROCESSED - AGENCY EXCEPTION\n";
    }

    LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - NOTPROCESSED");

    return NOTPROCESSED;
  }

  //-------------------------------------------------------------------------
  // Get the DayTimeApplication dynamic_cast ing from rule
  //-------------------------------------------------------------------------
  const DayTimeAppInfo* dayTimeApp = dynamic_cast<const DayTimeAppInfo*>(rule);

  if (!dayTimeApp)
  {
    LOG4CXX_DEBUG(logger, "Unable to get DayTimeInfo from the rule  - FAIL " << rule);
    LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - FAIL");
    return FAIL;
  }

  initialize(dayTimeApp);

  if (UNLIKELY(diag.isActive()))
  {
    if (!isInbound)
    {
      diag << "--------------- OUTBOUND VALIDATION ---------------------------\n";
    }
    else
    {
      diag << "--------------- INBOUND VALIDATION ----------------------------\n";
    }
  }

  //-------------------------------------------------------------------------
  // Check if data is unavailable or text only
  //-------------------------------------------------------------------------
  if (dayTimeApp->unavailtag() == dataUnavailable)
  {
    FareDisplayInfo* fdInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());

    if (!fdInfo)
    {
      LOG4CXX_DEBUG(logger, "Unable to get FareDisplayInfo object");
      LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - FAIL");
      return FAIL;
    }

    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

    fdInfo->setUnavailableR3Rule(RuleConst::DAY_TIME_RULE);

    if (UNLIKELY(diag.isActive()))
    {
      diag << " DAY/TIME APPL: NOT PROCESSED - DATA UNAVAILABLE\n";
    }

    LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - NOTPROCESSED");

    return NOTPROCESSED;
  }
  else if (dayTimeApp->unavailtag() == textOnly)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " DAY/TIME APPL: SKIP - TEXT ONLY\n";
    }

    LOG4CXX_INFO(logger, " Leaving FDDayTimeApplication::validate() - SKIP");

    return SKIP;
  }

  //-------------------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //-------------------------------------------------------------------------
  FareDisplayOptions* fdOptions = fareDisplayTrx->getOptions();

  if (fdOptions && (!fdOptions->isValidateRules() || fdOptions->isExcludeDayTimeValidation()))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " DAY/TIME APPL: NOT PROCESSED - SKIP RULES VALIDATION\n";
    }

    LOG4CXX_INFO(logger, " No Validation qualifier - skip Day/Time Application Validation");

    return NOTPROCESSED;
  }

  //-------------------------------------------------------------------------
  // Validate the Geographic Specification Table (995)
  //-------------------------------------------------------------------------

  // Define a pointer for the Application Point
  const TravelSeg* pItinAP = nullptr;

  // We should support arrival TSI. According to latest ATPCO document,
  // Cat2 does not use TSI that check both departure and arrival,
  // so we do not check arrival time

  RuleUtil::TravelSegWrapperVector appTravelSegs;

  if (dayTimeApp->geoTblItemNo() != 0)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "GEO SPECIFICATION:\n";
      RuleUtil::diagGeoTblItem(
          dayTimeApp->geoTblItemNo(), paxTypeFare.fare()->vendor(), trx, diag.collector());
      diag << "\n";
    }

    // System assumption
    RuleConst::TSIScopeParamType scope = (_itemInfo->dayTimeAppl() == subJourneyBased)
                                             ? RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY
                                             : RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    PricingUnit* pm = nullptr;
    FarePath* farePath = nullptr;

    fareDisplayTrx->dataHandle().get(farePath);

    if (farePath == nullptr)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " DAY/TIME APPL: FAIL - UNABLE TO CREATE FAREPATH\n";
      }

      LOG4CXX_DEBUG(logger,
                    "FDDayTimeApplication::validate() - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
      LOG4CXX_DEBUG(logger, "Leaving FDDayTimeApplication::validate()");
      return FAIL;
    }

    if (!FareDisplayUtil::initializeFarePath(*fareDisplayTrx, farePath))
    {
      farePath = nullptr;
    }

    retval = callGeoSpecTable(
        *(paxTypeFare.fare()), &fareMarket, pm, farePath, trx, appTravelSegs, scope);

    if (retval != PASS && retval != SOFTPASS)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " RETURN FROM GEO TABLE CALL:";

        if (retval == SKIP)
        {
          diag << " SKIP\n";
          diag << " DAYTIMEAPP - PASS\n";
        }
        else
          diag << " FAIL\n";
      }

      return retval;
    }

    if (appTravelSegs.begin() != appTravelSegs.end())
    {
      pItinAP = (*(appTravelSegs.begin()))->travelSeg();

      if (UNLIKELY(diag.isActive()))
      {
        diag << " GEO RETURN FIRST TRAVEL SEG: " << pItinAP->departureDT().toIsoExtendedString()
             << " " << pItinAP->origAirport() << " " << pItinAP->destAirport() << "\n";
      }
    }
    else
    {
      pItinAP = *(fareMarket.travelSeg().begin());

      if (UNLIKELY(diag.isActive()))
      {
        diag << " GEO RETURN TRAVEL SEG EMPTY\n";
      }
    }
  }
  else // No Geo Spec Table
  {
    // Set up the Application Point: origin of the F.C. (fareUsage)
    pItinAP = *(fareMarket.travelSeg().begin());
  }

  // We validate outbound date for one way fare, when it's requested by
  // FareDisplayUserPreference table only
  if (processOWSingleDateFare(fareDisplayTrx, paxTypeFare, pItinAP, diag, retval))
    return retval;

  // Check for no Return Date
  if (!fareDisplayTrx->getRequest()->returnDate().isValid() &&
      paxTypeFare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED && !isQualifiedCategory)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " NO RTN DATE - PASS\n";
    }
    return PASS;
  }

  //-------------------------------------------------------------------------
  // Validate Day of Week - Outbound Travel Date
  //-------------------------------------------------------------------------
  if (!isInbound)
  {
    DateTime earliestDepartureDate = pItinAP->earliestDepartureDT();
    DateTime latestDepartureDate = pItinAP->latestDepartureDT();

    if (UNLIKELY(diag.isActive()))
    {
      if (earliestDepartureDate.date() == latestDepartureDate.date())
      {
        diag << " TVL DATE " << earliestDepartureDate.toIsoExtendedString() << "\n";
      }
      else
      {
        diag << " TVL DATE RANGE ";
        diag << earliestDepartureDate.toIsoExtendedString() << " - ";
        diag << latestDepartureDate.toIsoExtendedString() << "\n";
      }
    }

    retval = validateDayOfWeekInRange(earliestDepartureDate, latestDepartureDate);
  }

  //-------------------------------------------------------------------------
  // Validate Day of Week - Return Date
  //
  // Note: Only validate if we meet all these conditions:
  // - Outbound Date validation failed
  // - Return Date specified
  // - Fare is not OW that may not be doubled
  // - Not a qualified category (IF condition)
  //-------------------------------------------------------------------------
  if (isInbound && fareDisplayTrx->getRequest()->returnDate().isValid() &&
      paxTypeFare.owrt() != ONE_WAY_MAYNOT_BE_DOUBLED && !isQualifiedCategory)
  {
    DateTime returnDate = fareDisplayTrx->getRequest()->returnDate();

    if (UNLIKELY(diag.isActive()))
    {
      diag << " RTN DATE " << returnDate.toIsoExtendedString() << "\n";
    }
    retval = validateDateDayOfWeek(returnDate);
  }

  // Validation is done
  diagReturnType(diag, retval);

  return retval;
}

Record3ReturnTypes
FDDayTimeApplication::validateDayOfWeekInRange(DateTime& earliestDepartureDate,
                                               DateTime& latestDepartureDate)
{
  Record3ReturnTypes retval = FAIL;
  DateTime& travelDateTime = earliestDepartureDate;
  uint32_t travelDOW = 0;
  bool skipTODCheck = true;

  for (uint16_t i = 0; i < 7 && travelDateTime <= latestDepartureDate; ++i)
  {
    travelDOW = travelDateTime.date().day_of_week().as_number();

    if (travelDOW == 0)
    {
      travelDOW = 7; // In ATPCo Sunday is 7, not 0
    }

    bool displayWqWarning = false;
    retval = validateDayOfWeek(travelDateTime, travelDOW, skipTODCheck, displayWqWarning);

    if (retval == PASS)
    {
      return retval;
    }

    travelDateTime = travelDateTime.nextDay();
  }

  return retval;
}

Record3ReturnTypes
FDDayTimeApplication::validateDateDayOfWeek(DateTime& valDate)
{
  bool skipTODCheck = true;
  uint32_t returnDOW = valDate.date().day_of_week().as_number();
  bool displayWqWarning = false;

  if (returnDOW == 0)
  {
    returnDOW = 7; // In ATPCo Sunday is 7, not 0
  }

  return (validateDayOfWeek(valDate, returnDOW, skipTODCheck, displayWqWarning));
}

void
FDDayTimeApplication::diagReturnType(DiagManager& diag, const Record3ReturnTypes& retval)
{
  if (UNLIKELY(diag.isActive()))
  {
    if (retval == FAIL)
    {
      diag << " FAIL - DAY OF WEEK\n";
    }
    else
    {
      diag << " PASS\n";
    }
  }
}
bool
FDDayTimeApplication::processOWSingleDateFare(FareDisplayTrx* fareDisplayTrx,
                                              const PaxTypeFare& paxTypeFare,
                                              const TravelSeg* tvSeg,
                                              DiagManager& diag,
                                              Record3ReturnTypes& retval)
{
  // only validate OW
  if (fareDisplayTrx->getOptions()->applyDOWvalidationToOWFares() != YES)
    return false;

  DateTime earliestDepartureDate = tvSeg->earliestDepartureDT();
  DateTime latestDepartureDate = tvSeg->latestDepartureDT();

  // only valid for OW fare, when no return date and no date range
  if ((paxTypeFare.owrt() != ONE_WAY_MAY_BE_DOUBLED) ||
      fareDisplayTrx->getRequest()->returnDate().isValid() ||
      (earliestDepartureDate.date() != latestDepartureDate.date()))
    return false;

  if (UNLIKELY(diag.isActive()))
  {
    uint32_t dow = earliestDepartureDate.date().day_of_week().as_number();
    static const char* weekDays[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    diag << " TVL DATE " << earliestDepartureDate.toIsoExtendedString() << " DOW: " << weekDays[dow]
         << "\n";
  }
  retval = validateDateDayOfWeek(earliestDepartureDate);
  diagReturnType(diag, retval);

  return true;
}
}
