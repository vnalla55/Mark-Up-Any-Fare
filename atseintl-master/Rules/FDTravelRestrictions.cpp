//-----------------------------------------------------------------
//
//  File:        FDTravelRestrictions.cpp
//
//  Authors:     Marco Cartolano
//  Created:     March 22, 2005
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
//-----------------------------------------------------------------

#include "Rules/FDTravelRestrictions.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TravelRestriction.h"
#include "Diagnostic/DiagManager.h"
#include "Util/BranchPrediction.h"

namespace tse
{

Logger
FDTravelRestrictions::_logger("atseintl.Rules.FDTravelRestrictions");

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDTravelRestrictions::validate()
//
// validate()   Performs rule validations for Travel Restrictions
//              on a FareMarket.
//
//  @param PricingTrx&          - Pricing transaction
//  @param Itin&                - itinerary
//  @param PaxTypeFare&         - reference to Pax Type Fare
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                need dynamic_cast to <TravelRestriction*>
//  @param FareMarket&         -  Fare Market
//
//  @return Record3ReturnTypes - possible values are:
//                               NOTPROCESSED  = 1
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//                               STOP          = 5
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
FDTravelRestrictions::validate(PricingTrx& trx,
                               Itin& itin,
                               const PaxTypeFare& paxTypeFare,
                               const RuleItemInfo* rule,
                               const FareMarket& fareMarket,
                               bool isQualifiedCategory)
{
  LOG4CXX_INFO(_logger, " Entered FDTravelRestrictions::validate()");

  //--------------------------------------------------------------
  // Get a Fare Display Transaction from the Pricing Transaction
  //--------------------------------------------------------------
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_DEBUG(_logger, "Unable to get FareDisplayTrx");
    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");

    return FAIL;
  }

  const TravelRestriction* tvlRestrictionInfo = dynamic_cast<const TravelRestriction*>(rule);

  if (!tvlRestrictionInfo)
  {
    LOG4CXX_DEBUG(_logger, "Not valid TravelRestriction");
    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");

    return FAIL;
  }

  const TravelRestriction& travelRestrictionInfo = *tvlRestrictionInfo;

  //-------------------------------------------------------------------
  // Get Fare Display Info object
  //-------------------------------------------------------------------
  FareDisplayInfo* fdInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());

  if (!fdInfo)
  {
    LOG4CXX_DEBUG(_logger, "Unable to get FareDisplayInfo object");
    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");

    return FAIL;
  }

  //-------------------------------------------------------------------
  // For Diagnostic display
  //-------------------------------------------------------------------
  DiagManager diag(trx, Diagnostic314);

  if (UNLIKELY(diag.isActive()))
  {
    displayRuleDataToDiag(travelRestrictionInfo, diag.collector());
    if (paxTypeFare.selectedFareNetRemit())
    {
      diag << " FOOTNOTES RULE FOR SELECTED CAT35 NET REMIT FARE\n";
    }
  }

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (travelRestrictionInfo.unavailTag() == dataUnavailable)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(_logger, " Updating FareDisplayInfo object");

    fdInfo->setUnavailableR3Rule(RuleConst::TRAVEL_RESTRICTIONS_RULE);

    diag << " TRAVEL RESTRICTIONS: NOT PROCESSED - DATA UNAVAIBLE\n";

    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - NOTPROCESSED");
    return NOTPROCESSED;
  }

  //-------------------------------------------------------------------
  // Update FareDisplayInfo object with Travel Restrictions information
  //-------------------------------------------------------------------
  if (!isQualifiedCategory)
  {
    LOG4CXX_INFO(_logger, " Updating FareDisplayInfo object");
    updateFareDisplayInfo(travelRestrictionInfo, fdInfo, TrxUtil::getTicketingDT(trx));
  }

  //----------------------------------------------------------------------
  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  //----------------------------------------------------------------------
  const Record3ReturnTypes unavailableFlag =
      validateUnavailableDataTag(travelRestrictionInfo.unavailTag());

  if (unavailableFlag == FAIL)
  {
    diag << " TRAVEL RESTRICTIONS: FAIL - DATA UNAVAIBLE\n";

    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");
    return FAIL;
  }
  else if (unavailableFlag == SKIP)
  {
    diag << " TRAVEL RESTRICTIONS: SKIP - TXT DATA ONLY\n";

    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - SKIP");
    return SKIP;
  }

  //-----------------------------------------------------------------
  // Earliest/latest start date is applied to travel
  // from origin of pricing unit, unless further modified by a
  // geographic specification table.
  // Since Fare Display does not have a Pricing Unit, earliest/latest
  // start date will be applied to travel from origin of Fare Component.
  //
  // Travel commence/complete restriction is applied
  // to departure from origin of fare component/arrival at destination
  // of fare component, unless further modified by a geographic
  // specification table
  //-----------------------------------------------------------------
  bool needStartDTCheck = true; // TRAVEL FROM/TRAVEL TO CHECK FOR P.U.
  bool needStopDTCheck = true; // TRAVEL COMMENCE/COMPLETE CHECK FOR F.C.

  if (!isValidATSEDate(travelRestrictionInfo.earliestTvlStartDate()) &&
      !isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
  {
    needStartDTCheck = false;
  }

  if (travelRestrictionInfo.returnTvlInd() == NOTAPPLY ||
      (fdTrx->isERD() && (fdTrx->getRequest()->ticketingAgent()->abacusUser() ||
                          fdTrx->getRequest()->ticketingAgent()->infiniUser()) &&
       fdTrx->getOptions() && fdTrx->getOptions()->isErdAfterCmdPricing()))
  {
    needStopDTCheck = false;
  }

  if (!needStartDTCheck && !needStopDTCheck)
  {
    diag << " TRAVEL RESTRICTIONS: PASS - NO START/STOP DATES\n";

    LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - PASS");
    return PASS;
  }

  //-----------------------------------------------------------------
  // Identify the travel segments and find the reference dates
  // for validation
  // Different approaches on if there is valid GEO Table
  // For Fare Display only Fare Component scope applies
  //-----------------------------------------------------------------
  DateTime refStartDT;
  DateTime refEndDT;
  DateTime refStopDT;
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  if (travelRestrictionInfo.geoTblItemNo())
  {
    //-----------------------------------------------------------------
    // Indentify travel segments by GEO table item No.
    //-----------------------------------------------------------------
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;

    DateTime tktDT = TrxUtil::getTicketingDT(*fdTrx);

    RuleConst::TSIScopeParamType scopeParamDefault = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
    int32_t itemNo = travelRestrictionInfo.geoTblItemNo();

    FarePath* farePath = nullptr;
    fdTrx->dataHandle().get(farePath);
    if (farePath == nullptr)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - UNABLE TO CREATE FAREPATH\n";

      LOG4CXX_DEBUG(_logger,
                    "FDTravelRestriction::process() - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
      LOG4CXX_DEBUG(_logger, "Leaving FDTravelRestriction:process");
      return FAIL;
    }

    if (!FareDisplayUtil::initializeFarePath(*fdTrx, farePath))
    {
      farePath = nullptr;
    }

    if (fdTrx->getRequest()->returnDate().isValid() &&
        (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY ||
         travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMMENCEDBY))
    {
      FareMarket* fm = fdTrx->inboundFareMarket();
      farePath->itin()->fareMarket().push_back(fm);
      if (!RuleUtil::validateGeoRuleItem(itemNo,
                                         paxTypeFare.vendor(),
                                         scopeParamDefault,
                                         false,
                                         false,
                                         false,
                                         trx,
                                         farePath,
                                         nullptr,
                                         farePath->pricingUnit()[0],
                                         &fareMarket,
                                         tktDT,
                                         applTravelSegment,
                                         true,
                                         false,
                                         fltStopCheck,
                                         tsiReturn,
                                         locKey1Return,
                                         locKey2Return,
                                         Diagnostic314))
      {

        //------------------------------------------------------
        // Apply system assumption: no restriction
        //------------------------------------------------------
        diag << " TRAVEL RESTRICTIONS: SKIP - FALSE FROM VALIDATE GEO RULE ITEM\n";

        LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - PASS");
        return PASS;
      }
    }

    else
    {
      if (!RuleUtil::validateGeoRuleItem(itemNo,
                                         paxTypeFare.vendor(),
                                         scopeParamDefault,
                                         false, // allowJourneyScopeOverride
                                         false, // allowPUScopeOverride
                                         false, // allowFCScopeOverride
                                         trx,
                                         nullptr, // farePath
                                         nullptr, // itin
                                         nullptr, // pricingUnit
                                         &fareMarket,
                                         tktDT,
                                         applTravelSegment, // this will contain the results
                                         true, // origCheck
                                         false, // destCheck
                                         fltStopCheck,
                                         tsiReturn,
                                         locKey1Return,
                                         locKey2Return,
                                         Diagnostic314))
      {
        //------------------------------------------------------
        // Apply system assumption: no restriction
        //------------------------------------------------------
        diag << " TRAVEL RESTRICTIONS: SKIP - FALSE FROM VALIDATE GEO RULE ITEM\n";

        LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - PASS");
        return PASS;
      }
    }

    // If there is a TSI attached to the Table 995, the TSI can
    //  override the origin and destination checks.
    // The default is check origin not destination.
    bool origCheck = true;
    bool destCheck = false;
    bool tsiOverrideOrigDestChk = true;

    VendorCode vendor = paxTypeFare.vendor();

    if (!RuleUtil::getTSIOrigDestCheckFromGeoRuleItem(itemNo, vendor, trx, origCheck, destCheck))
    {
      tsiOverrideOrigDestChk = false;
    }

    //------------------------------------------------------
    // Diagnostic GEO return
    //------------------------------------------------------
    if (UNLIKELY(diag.isActive()))
    {
      diag << " GEO RETURN: TSI - " << tsiReturn << " ORIG-CHK - " << ((origCheck) ? "T" : "F")
           << " DEST-CHK - " << ((destCheck) ? "T" : "F") << "\n";

      if (locKey1Return.locType() != LOCTYPE_NONE)
        diag << "             LOC1 - " << locKey1Return.loc();
      if (locKey2Return.locType() != LOCTYPE_NONE)
        diag << " LOC2 - " << locKey2Return.loc() << "\n";
    }

    //-----------------------------------------------------
    // Get Ref Start Date and Stop Date
    // StopDT
    //   origCheck: stop date against earliest flight depart
    //   destCheck: stop date against latest flight arrival
    // StartDT
    //   origCheck: start date against earliest flight depart
    //   destCheck: start date against earliest flight arrival
    //------------------------------------------------------
    if (needStopDTCheck)
    {
      if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
      {
        RuleUtil::getLastValidTvlDT(
            refStopDT, applTravelSegment, tsiOverrideOrigDestChk ? (!destCheck) : false);
        // will still check against arrival if TSI did not
        // override origCheck, destCheck
      }
      else // CommencedBy
      {
        RuleUtil::getFirstValidTvlDT(refStopDT, applTravelSegment, origCheck);
      }
    }

    if (needStartDTCheck)
    {
      RuleUtil::getFirstValidTvlDT(refStartDT, refEndDT, applTravelSegment, origCheck);

      if (!refStartDT.isValid() || !refEndDT.isValid())
      {
        // system assumption, pass now
        needStartDTCheck = false;
      }
    } // end of getting refStartDT and refStopDT when geoTbleItemNo != 0
  } // geoTblItemNo != 0
  else
  {
    //----------------------------------------------------------------
    // StopDT
    //   tvlMustBeCommencedBy: stop date against earliest flight depart
    //   tvlMustBeCompletedBy: stop date against latest flight arrival
    // StartDT
    //   Always against earliest flight departure
    //----------------------------------------------------------------
    if (needStopDTCheck)
    {
      //-----------------------------------------------------------
      // Get all airseg from the fare market and put it into the
      // applTravelSegment vector because there was no 995 for populate it
      //-----------------------------------------------------------
      if (!fareMarket.travelSeg().empty())
      {
        std::vector<TravelSeg*>::const_iterator travelSegmentI;
        travelSegmentI = fareMarket.travelSeg().begin();
        for (; travelSegmentI != fareMarket.travelSeg().end(); ++travelSegmentI)
        {
          if (dynamic_cast<AirSeg*>(*travelSegmentI))
          {
            // lint --e{530}
            RuleUtil::TravelSegWrapper* tsw;
            fdTrx->dataHandle().get(tsw);
            tsw->travelSeg() = (*travelSegmentI);
            tsw->origMatch() = true;
            tsw->destMatch() = false;
            applTravelSegment.push_back(tsw);
          }
        }
      }

      if (applTravelSegment.empty())
      {
        diag << " TRAVEL RESTRICTION: FAIL - FARE MARKET TRAVEL SEG EMPTY\n";

        LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");
        return FAIL;
      }

      if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
      {
        RuleUtil::getLastValidTvlDT(refStopDT, applTravelSegment, false);
      }
      else
      {
        RuleUtil::getFirstValidTvlDT(refStopDT, applTravelSegment, true);
      }

      if (!refStopDT.isValid())
      {
        needStopDTCheck = false;
      }
    }
  }

  if (needStartDTCheck)
  {
    //----------------------------------------------------------------
    // Check if it's a request to skip Earliest Start Date check (#VN)
    //----------------------------------------------------------------
    FareDisplayOptions* fdOptions = fdTrx->getOptions();
    bool needEarliestStartDTCheck = true;

    if (!fdTrx->isLongRD() && !fdTrx->isERD())
    {
      if (fdOptions && !fdOptions->isValidateRules())
      {
        LOG4CXX_DEBUG(
            _logger,
            " No Validation qualifier - skip Travel Restrictions Earliest Start Date Validation");
        needEarliestStartDTCheck = false;
      }
    }

    Record3ReturnTypes rtn = PASS;

    if (travelRestrictionInfo.geoTblItemNo())
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " TRAVEL DATE FOR FROM/TO VALIDATION " << refStartDT.toIsoExtendedString() << "\n";
      }

      rtn = validateTvlStartDate(
          refStartDT, refEndDT, travelRestrictionInfo, needEarliestStartDTCheck, diag);
    }
    else
    {
      RuleUtil::getFirstValidTvlDT(refStartDT, refEndDT, fareMarket.travelSeg(), true);

      if (refStartDT.isValid() && refEndDT.isValid())
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " TRAVEL DATE FOR FROM/TO VALIDATION " << refStartDT.toIsoExtendedString()
               << "\n";
        }

        rtn = validateTvlStartDate(
            refStartDT, refEndDT, travelRestrictionInfo, needEarliestStartDTCheck, diag);
      }
    }

    if (rtn == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - START DATE\n";

      LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");
      return FAIL;
    }
  }

  if (needStopDTCheck)
  {
    if (fdTrx->getRequest()->returnDate().isValid() &&
        fdTrx->getRequest()->returnDate() > refStopDT)
    {
      RuleUtil::getLastValidTvlDT(refStopDT, fdTrx->inboundFareMarket()->travelSeg(), true);
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << " TVL DATE FOR COMMENCE/COMPLETE VALID " << refStopDT.toIsoExtendedString() << "\n";
    }

    if (!refStopDT.isValid() || validateTvlStopDate(refStopDT, travelRestrictionInfo, diag) == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - STOP DATE\n";

      LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - FAIL");
      return FAIL;
    }
  }

  if (UNLIKELY(diag.isActive()))
  {
    diag << " TRAVEL RESTRICTION: PASS\n";
  }

  LOG4CXX_INFO(_logger, " Leaving FDTravelRestrictions::validate() - PASS");
  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDTravelRestrictions::validateTvlStartDate()
//
// validateTvlStartDate()  validate travel start date range meets
//  requirement
//
//  @param DateTime&  refStartDT - Reference earliest Travel DateTime
//  @param DateTime&  refEndDT   - Reference latest Travel DateTime
//  @param TravelRestriction&    - Travel Restriction Rule Item Info
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
FDTravelRestrictions::validateTvlStartDate(const DateTime& refStartDT,
                                           const DateTime& refEndDT,
                                           const TravelRestriction& travelRestrictionInfo,
                                           const bool& needEarliestStartDTCheck,
                                           DiagManager& diag) const
{
  if (needEarliestStartDTCheck)
  {
    const DateTime& earliestDT = travelRestrictionInfo.earliestTvlStartDate();

    // The Hour:Minute we get would be 0:00 as they are not defined in rule data
    if ((earliestDT.isValid()) && (refEndDT < earliestDT))
    {
      diag << "TRAVEL DATE IS EARLIER THAN FIRST DATE PERMITTED\n";

      LOG4CXX_DEBUG(_logger, " TRAVEL DATE IS EARLIER THAN FIRST DATE PERMITTED");
      return FAIL;
    }
  }

  const DateTime& latestDT = travelRestrictionInfo.latestTvlStartDate();

  if ((latestDT.isValid()) && (refStartDT.date() > latestDT.date()))
  {
    diag << "TRAVEL DATE IS LATER THAN LAST DATE PERMITTED\n";

    LOG4CXX_DEBUG(_logger, " TRAVEL DATE IS LATER THAN LAST DATE PERMITTED");
    return FAIL;
  }

  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDTravelRestrictions::validateTvlStopDate()
//
// validateTvlStopDate()  validate travel return date meets requirement
//
//  @param DateTime&  refDT     - Reference DateTime, could be of departure
//                                or arrival
//  @param TravelRestriction&   - Travel Restriction Rule Item Info
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
FDTravelRestrictions::validateTvlStopDate(const DateTime& rtnRefDT,
                                          const TravelRestriction& travelRestrictionInfo,
                                          DiagManager& diag) const
{
  DateTime rtnStopDT = DateTime(travelRestrictionInfo.stopDate().date(), 0, 0, 0);

  if (rtnRefDT > rtnStopDT)
  {
    diag << "TRAVEL RETURN DATE IS LATER THAN RETURN DATE PERMITTED\n";

    LOG4CXX_DEBUG(_logger, " TRAVEL RETURN DATE IS LATER THAN RETURN DATE PERMITTED");
    return FAIL;
  }
  else
  {
    return PASS;
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDTravelRestrictions::updateFareDisplayInfo()
//
// updateFareDisplayInfo()  update FareDisplayInfo object with Travel
//                      Restrictions information
//
//  @param TravelRestriction&  - reference to Travel Restrictions rule
//  @param FareDisplayInfo*&   - reference to Fare Display Info ptr
//  @param DateTime&           - reference to ticketing date
//
// </PRE>
//-------------------------------------------------------------------
void
FDTravelRestrictions::updateFareDisplayInfo(const TravelRestriction& travelRestrictionInfo,
                                            FareDisplayInfo*& fdInfo,
                                            const DateTime& ticketingDate) const
{
  // update travelInfoRD vector for RD header

  DateTime earliestTvlStartDate = travelRestrictionInfo.earliestTvlStartDate();

  DateTime latestTvlStartDate = travelRestrictionInfo.latestTvlStartDate();

  DateTime stopDate = travelRestrictionInfo.stopDate();

  if (earliestTvlStartDate.isValid() || latestTvlStartDate.isValid() ||
      (travelRestrictionInfo.returnTvlInd() != NOTAPPLY && stopDate.isValid()))
  {
    fdInfo->addRDTravelInfo(
        earliestTvlStartDate, latestTvlStartDate, stopDate, travelRestrictionInfo.returnTvlInd());
  }
  else
    return; // if no valid dates just return.

  // Do not update the FareDisplayInfo object with past dates
  earliestTvlStartDate =
      (travelRestrictionInfo.earliestTvlStartDate().date() < ticketingDate.date())
          ? DateTime::emptyDate()
          : travelRestrictionInfo.earliestTvlStartDate();

  latestTvlStartDate = (travelRestrictionInfo.latestTvlStartDate().date() < ticketingDate.date())
                           ? DateTime::emptyDate()
                           : travelRestrictionInfo.latestTvlStartDate();

  stopDate = (travelRestrictionInfo.stopDate().date() < ticketingDate.date())
                 ? DateTime::emptyDate()
                 : travelRestrictionInfo.stopDate();

  // Check for valid dates
  if (earliestTvlStartDate.isValid() || latestTvlStartDate.isValid() ||
      (travelRestrictionInfo.returnTvlInd() != NOTAPPLY && stopDate.isValid()))
  {
    fdInfo->addTravelInfo(
        earliestTvlStartDate, latestTvlStartDate, stopDate, travelRestrictionInfo.returnTvlInd());
  }
}

} // tse
