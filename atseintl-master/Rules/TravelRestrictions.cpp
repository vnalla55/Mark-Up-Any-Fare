//-------------------------------------------------------------------
//
//  File:         TravelRestrictions.cpp
//  Author:       Simon Li
//  Created:      08/02/2004
//  Description:  Travel Restriction category is used when travel dates
//                are specifically stated in a rule. It contains travel
//                commencement, expiration and completion dates. It also
//                indicates whether travel completion must commence or
//                be completed by a specific date/time.
//                This is a Rule Application class that supports two public
//                validate functions for Fare Market and Pricing Unit.
//
//  Copyright Sabre 2004
//
//        The copyright to the computer program(s) herein
//        is the property of Sabre.
//        The program(s) may be used and/or copied only with
//        the written permission of Sabre or in accordance
//        with the terms and conditions stipulated in the
//        agreement/contract under which the program(s)
//        have been supplied.
//
//-------------------------------------------------------------------

#include "Rules/TravelRestrictions.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/TravelRestriction.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/UpdaterObserver.h"
#include "Util/BranchPrediction.h"

#include <memory>
#include <vector>

namespace tse
{

Logger
TravelRestrictions::_logger("atseintl.Rules.TravelRestrictions");

static const int NO_TIME = -1;

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validate()
//
// validate()   Performs rule validations for travel restrictions
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
//                               NOT_PROCESSED = 1
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//                               STOP          = 5
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validate(PricingTrx& trx,
                             Itin& itin,
                             const PaxTypeFare& paxTypeFare,
                             const RuleItemInfo* rule,
                             const FareMarket& fareMarket)
{
  LOG4CXX_INFO(_logger, " Entered TravelRestrictions::validate() for FareMarket");

  const TravelRestriction* tvlRestrictionInfo = dynamic_cast<const TravelRestriction*>(rule);

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  paxTypeFare.warningMap().set(WarningMap::cat14_warning, true);

  if (UNLIKELY(!tvlRestrictionInfo))
  {
    LOG4CXX_ERROR(_logger, "Not valid TravelRestriction");
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
    return FAIL;
  }
  const TravelRestriction& travelRestrictionInfo = *tvlRestrictionInfo;

  DiagManager diag(trx, Diagnostic314);

  if (UNLIKELY(diag.isActive()))
  {
    displayRuleDataToDiag(travelRestrictionInfo, diag.collector());

    if (paxTypeFare.selectedFareNetRemit())
    {
      diag << " FOOTNOTES RULE FOR SELECTED CAT35 NET REMIT FARE\n";
    }
  }

  //----------------------------------------------------------------------
  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  //----------------------------------------------------------------------
  const Record3ReturnTypes unavailableFlag =
      validateUnavailableDataTag(travelRestrictionInfo.unavailTag());
  if (UNLIKELY(unavailableFlag == FAIL))
  {
    diag << " TRAVEL RESTRICTIONS: FAIL - DATA UNAVAIBLE\n";
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
    return FAIL;
  }
  else if (unavailableFlag == SKIP)
  {
    diag << " TRAVEL RESTRICTIONS: SKIP - TXT DATA ONLY\n";
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - SKIP");
    return SKIP;
  }

  bool needPUValidation = false;
  int16_t nvaStartSegOrder = 0; // travel seg order that start date applies
  int16_t nvaStopSegOrder = 0; // travel seg order that stop date applies

  //-----------------------------------------------------------------
  // Earliest/latest start date is applied to travel
  // from origin of pricing unit, unless further modified by a
  // geographic specification table
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
  if (isTravelIndNotApplicable(travelRestrictionInfo.returnTvlInd()) ||
      ((const PricingTrx&)trx).getOptions()->fareX() ||
      (paxTypeFare.isCmdPricing() &&
       ((const PricingTrx&)trx).getRequest()->ticketingAgent()->abacusUser()))
  {
    needStopDTCheck = false;
  }

  //-----------------------------------------------------------------
  // Identify the travel segments and find the reference dates
  // for validation
  // Different approaches on if there is valid GEO Table
  //-----------------------------------------------------------------
  DateTime refStartDT;
  DateTime refStopDT;
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  // do not check geo995 table for the selected Cat35 net Remit fare
  if (travelRestrictionInfo.geoTblItemNo() && !paxTypeFare.selectedFareNetRemit())
  {
    RuleConst::TSIScopeType tsiScopeRtn = RuleConst::TSI_SCOPE_FARE_COMPONENT;
    if (UNLIKELY(!RuleUtil::getTSIScopeFromGeoRuleItem(
            travelRestrictionInfo.geoTblItemNo(), paxTypeFare.vendor(), trx, tsiScopeRtn)))
    {
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

      diag << " TRAVEL RESTRICTIONS: FAIL - NO TSI SCOPE FROM GEO\n";
      return FAIL;
    }

    //---------------------------------------------------------------
    //  This is how TSI Scope overrides applications:
    //
    //  TSI           TRAVEL FROM/TO     TRAVEL COMMENCE/COMPLETE
    //  NONE            SJ                  FC
    //  FC              FC                  FC
    //  SJ              SJ                  SJ
    //  SJ_AND_FC       SJ                  FC
    //  J               J                   J
    //---------------------------------------------------------------

    //---------------------------------------------------------------
    // In faremarket, we do not have SUB JOURNEY(P.U) information
    // We only have Fare Component and Journey information
    //---------------------------------------------------------------
    if (isGeoScopeSubJourney(tsiScopeRtn))
    {
      needPUValidation = true;
      needStartDTCheck = false; // let it pass, validate later
      if (tsiScopeRtn != RuleConst::TSI_SCOPE_SJ_AND_FC)
      {
        needStopDTCheck = false; // let it pass, validate later
      }
    }

    if (!needStartDTCheck && !needStopDTCheck)
    {
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - PASS");

      if (UNLIKELY(diag.isActive()))
      {
        if (needPUValidation)
          diag << " TRAVEL RESTRICTIONS: SOFTPASS - NO SUB JOURNEY INFO\n";
        else
          diag << " TRAVEL RESTRICTIONS: PASS - NO SUB JOURNEY INFO\n";
      }

      if (LIKELY(needPUValidation))
        return SOFTPASS;
      else
        return PASS;
    }

    //-----------------------------------------------------------------
    // Indentify travel segments by GEO table item No.
    //-----------------------------------------------------------------
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;

    RuleConst::TSIScopeParamType scopeParamDefault = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
    int32_t itemNo = travelRestrictionInfo.geoTblItemNo();

    if (UNLIKELY(!RuleUtil::validateGeoRuleItem(itemNo,
                                       paxTypeFare.vendor(),
                                       scopeParamDefault,
                                       true,
                                       true,
                                       true,
                                       trx,
                                       nullptr, // fare path,
                                       &itin,
                                       nullptr, // pricing unit,
                                       &fareMarket,
                                       TrxUtil::getTicketingDT(trx),
                                       applTravelSegment, // this will contain the results
                                       true,
                                       false,
                                       fltStopCheck,
                                       tsiReturn,
                                       locKey1Return,
                                       locKey2Return,
                                       Diagnostic314)))
    {
      //------------------------------------------------------
      // Because we allow scope override by TSI, if the TSI scope is FC,
      // apply system assumption: no restriction;
      // Otherwise (SJ, J, FC_AND_SJ), we need re-validation, SOFTPASS
      //------------------------------------------------------
      if (needPUValidation)
        diag << " TRAVEL RESTRICTIONS: SOFTPASS ";
      else
        diag << " TRAVEL RESTRICTIONS: SKIP ";

      diag << "- FALSE FROM VALIDATE GEO RULE ITEM\n";

      if (needPUValidation)
      {
        LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - SOFTPASS");
        return SOFTPASS;
      }
      else
      {
        LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - PASS");
        return PASS;
      }
    }

    // If there is a TSI attached to the Table 995, the TSI can
    //  override the origin and destination checks.
    // The default is check origin not destination.
    bool origCheck = true;
    bool destCheck = false;
    bool tsiOverrideOrigDestChk = true;

    if (UNLIKELY(!RuleUtil::getTSIOrigDestCheckFromGeoRuleItem(
            itemNo, paxTypeFare.vendor(), trx, origCheck, destCheck)))
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
    //              if this TSIScope is for JOURNEY, otherwise
    //              (FARE_COMPONENT) the same as stop date
    //------------------------------------------------------
    if (needStopDTCheck)
    {
      if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
      {
        nvaStopSegOrder = itin.segmentOrder(applTravelSegment.back()->travelSeg());

        RuleUtil::getLastValidTvlDT(
            refStopDT, applTravelSegment, tsiOverrideOrigDestChk ? (!destCheck) : false, noPNRTrx);
        // will still check against arrival if TSI did not
        // override origCheck, destCheck
      }
      else // CommencedBy
      {
        nvaStopSegOrder = itin.segmentOrder(applTravelSegment.front()->travelSeg());

        RuleUtil::getFirstValidTvlDT(refStopDT, applTravelSegment, origCheck, noPNRTrx);
      }
      if (UNLIKELY(!refStopDT.isValid()))
      {
        needStopDTCheck = false;
      }
    }

    if (needStartDTCheck)
    {
      RuleUtil::getFirstValidTvlDT(refStartDT, applTravelSegment, origCheck, noPNRTrx);

      if (UNLIKELY(!refStartDT.isValid()))
      {
        // system assumption, pass now, validate later
        needStartDTCheck = false;
      }
      else if (isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
      {
        nvaStartSegOrder = itin.segmentOrder(applTravelSegment.front()->travelSeg());
      }
    } // end of getting refStartDT and refStopDT when geoTbleItemNo != 0
  } // geoTblItemNo != 0
  else
  {
    //----------------------------------------------------------------
    // StopDT
    //   TVLMUSTBECOMMENCEDBY: stop date against earliest flight depart
    //   TVLMUSTBECOMPLETEDBY: stop date against latlest flight arrival
    // StartDT
    //   Always against earliest flight departure, but it is for validation
    //   against Pricing Unit, we should not validate now
    //   H/W, to improve combination performance, we can fail some fares
    //   based on
    //   JourneyStartDT <= SubJourneyStartDT <= FareComponentStartDT
    //   (SubJourney is for PU)
    //----------------------------------------------------------------
    if (needStopDTCheck)
    {
      //-----------------------------------------------------------
      // Get all airseg from the fare market and put it into the
      // applTravelSegment vector because there was no 995 for populate it
      //-----------------------------------------------------------
      if (LIKELY(!fareMarket.travelSeg().empty()))
      {
        std::vector<TravelSeg*>::const_iterator travelSegmentI;
        travelSegmentI = fareMarket.travelSeg().begin();
        for (; travelSegmentI != fareMarket.travelSeg().end(); ++travelSegmentI)
        {
          if (dynamic_cast<AirSeg*>(*travelSegmentI))
          {
            RuleUtil::TravelSegWrapper* tsw;
            // lint --e{530}
            trx.dataHandle().get(tsw);
            tsw->travelSeg() = (*travelSegmentI);
            tsw->origMatch() = true;
            tsw->destMatch() = false;
            applTravelSegment.push_back(tsw);
          }
        }
      }

      if (UNLIKELY(applTravelSegment.empty()))
      {
        diag << " TRAVEL RESTRICTION: FAIL "
             << "- FARE MARKET TRAVEL SEG EMPTY\n";

        LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
        return FAIL;
      }

      if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
      {
        nvaStopSegOrder = itin.segmentOrder(applTravelSegment.back()->travelSeg());

        RuleUtil::getLastValidTvlDT(refStopDT, applTravelSegment, false, noPNRTrx);
      }
      else
      {
        nvaStopSegOrder = itin.segmentOrder(applTravelSegment.front()->travelSeg());

        RuleUtil::getFirstValidTvlDT(refStopDT, applTravelSegment, true, noPNRTrx);
      }

      if (!refStopDT.isValid())
      {
        needStopDTCheck = false;
      }
    }
  }

  // do not check earliestTvlStartDate/latestTvlStartDate for cat35 net Remit
  // when geo table is filed
  if (needStartDTCheck)
  {
    Record3ReturnTypes rtn = PASS;

    if (travelRestrictionInfo.geoTblItemNo() && !paxTypeFare.selectedFareNetRemit())
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " TRAVEL DATE FOR FROM/TO VALIDATION " << refStartDT.toIsoExtendedString() << "\n";
      }
      rtn = validateTvlStartDate(refStartDT, travelRestrictionInfo, diag);
    }
    else
    {
      // For PU combination performance, fail fares with what we have
      DateTime refJourneyStartDT;
      DateTime refFareCompStartDT;

      RuleUtil::getFirstValidTvlDT(refJourneyStartDT, itin.travelSeg(), true, noPNRTrx);
      RuleUtil::getFirstValidTvlDT(refFareCompStartDT, fareMarket.travelSeg(), true, noPNRTrx);

      if (LIKELY(refFareCompStartDT.isValid()))
      {
        if (UNLIKELY(diag.isActive()))
        {
          if (refJourneyStartDT == refFareCompStartDT)
          {
            diag << " TRAVEL DATE FOR FROM/TO VALIDATION "
                 << refJourneyStartDT.toIsoExtendedString() << "\n";
          }
          else
          {
            diag << " TRAVEL DATE FOR FROM/TO VALIDATION IS BTW \n"
                 << refJourneyStartDT.toIsoExtendedString() << " AND "
                 << refFareCompStartDT.toIsoExtendedString() << "\n";
          }
        }

        if (refJourneyStartDT != refFareCompStartDT)
        {
          needPUValidation = true; // PU start time is still unknown
        }
        else if (travelRestrictionInfo.latestTvlStartDate().isValid())
        {
          nvaStartSegOrder = 1;
        }
        rtn = validateTvlStartDate(
            refJourneyStartDT, refFareCompStartDT, travelRestrictionInfo, diag);
      }
      else
      {
        needPUValidation = true; // start time validation is for PU
      }
    }

    if (rtn == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - START DATE\n";
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

      return FAIL;
    }
  }

  if (needStopDTCheck)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " TVL DATE FOR COMMENCE/COMPLETE VALID " << refStopDT.toIsoExtendedString() << "\n";
    }

    if (validateTvlStopTime(refStopDT, travelRestrictionInfo, diag) == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - STOP DATE\n";
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

      return FAIL;
    }
  }
  LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - PASS");

  if (needPUValidation)
  {
    diag << "\n TRAVEL RESTRICTIONS: SOFTPASS \n";
    return SOFTPASS;
  }
  else
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "\n TRAVEL RESTRICTIONS: PASS \n";
    }

    if (nvaStartSegOrder > 0)
    {
      const LocCode location = itin.travelSeg()[nvaStartSegOrder - 1]->boardMultiCity();
      notifyObservers(NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMMENCE,
                      nvaStartSegOrder,
                      travelRestrictionInfo.latestTvlStartDate(),
                      location,
                      NO_TIME);
    }

    if (nvaStopSegOrder > 0)
    {
      const LocCode location = itin.travelSeg()[nvaStopSegOrder - 1]->boardMultiCity();
      notifyObservers(travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMMENCEDBY
                          ? (NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMMENCE)
                          : (NotificationType::PAX_TYPE_FARE_TRAVEL_MUST_COMPLETE),
                      nvaStopSegOrder,
                      travelRestrictionInfo.stopDate(),
                      location,
                      travelRestrictionInfo.stopTime());
    }

    return PASS;
  }
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validate()
//
// validate() Performs rule validations for travel restrictions
//              on a PricingUnit.
//
//  @param PricingTrx&          - Pricing transaction
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                need dynamic_cast to <TravelRestriction*>
//  @param FarePath&            - Fare Path
//  @param PricingUnit&         - Pricing unit
//  @param FareUsage&           - Fare Usage
//
//  @return Record3ReturnTypes - possible values are:
//                               NOT_PROCESSED = 1
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//                               STOP          = 5
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validate(PricingTrx& trx,
                             const RuleItemInfo* rule,
                             const Itin& itin,
                             const PricingUnit& pricingUnit,
                             const FareUsage& fareUsage)
{
  FarePath fakeFarePath;
  fakeFarePath.itin() = const_cast<Itin*>(&itin);

  return validate(trx, rule, fakeFarePath, pricingUnit, fareUsage);
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validate()
//
// validate() Performs rule validations for travel restrictions
//              on a PricingUnit.
//
//  @param PricingTrx&          - Pricing transaction
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                need dynamic_cast to <TravelRestriction*>
//  @param FarePath&            - Fare Path
//  @param PricingUnit&         - Pricing unit
//  @param FareUsage&           - Fare Usage
//
//  @return Record3ReturnTypes - possible values are:
//                               NOT_PROCESSED = 1
//                               FAIL          = 2
//                               PASS          = 3
//                               SKIP          = 4
//                               STOP          = 5
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validate(PricingTrx& trx,
                             const RuleItemInfo* rule,
                             const FarePath& farePath,
                             const PricingUnit& pricingUnit,
                             const FareUsage& fareUsage)
{
  LOG4CXX_INFO(_logger, " Entered TravelRestrictions::validate() for Pricing Unit");

  const TravelRestriction* tvlRestrictionInfo = dynamic_cast<const TravelRestriction*>(rule);

  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  if (UNLIKELY(!tvlRestrictionInfo))
  {
    LOG4CXX_ERROR(_logger, "Not valid TravelRestriction");
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
    return FAIL;
  }
  const TravelRestriction& travelRestrictionInfo = *tvlRestrictionInfo;

  DiagManager diag(trx, Diagnostic314);

  if (UNLIKELY(diag.isActive()))
  {
    displayRuleDataToDiag(travelRestrictionInfo, diag.collector());
  }

  //----------------------------------------------------------------
  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  //----------------------------------------------------------------
  const Record3ReturnTypes unavailableFlag =
      validateUnavailableDataTag(travelRestrictionInfo.unavailTag());
  if (UNLIKELY(unavailableFlag == FAIL))
  {
    diag << " TRAVEL RESTRICTIONS: FAIL - DATA UNAVAIBLE\n";
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
    return FAIL;
  }
  else if (unavailableFlag == SKIP)
  {
    diag << " TRAVEL RESTRICTIONS: SKIP - TXT DATA ONLY\n";
    LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - SKIP");
    return SKIP;
  }

  //-----------------------------------------------------------------
  bool needStartDTCheck = true; // TRAVEL FROM/TRAVEL TO CHECK FOR P.U.
  bool needStopDTCheck = true; // TRAVEL COMMENCE/COMPLETE CHECK FOR F.C.

  if (!isValidATSEDate(travelRestrictionInfo.earliestTvlStartDate()) &&
      !isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
  {
    needStartDTCheck = false;
  }
  if ((travelRestrictionInfo.returnTvlInd() == NOTAPPLY) ||
      ((const PricingTrx&)trx).getOptions()->fareX() ||
      (fareUsage.paxTypeFare()->isCmdPricing() &&
       ((const PricingTrx&)trx).getRequest()->ticketingAgent()->abacusUser()))
  {
    needStopDTCheck = false;
  }

  int16_t nvaStartSegOrder = 0;
  int16_t nvaStopSegOrder = 0;

  const Itin* itin = farePath.itin();
  const bool isFPFake = farePath.pricingUnit().empty();

  //----------------------------------------------------------------
  // For Pricing Unit, There are two or three dates we need to validate
  // Must validate:     earliest departure date on outbound travel
  //                    latest departure date on outbound travel
  // May need validate: last departure or arrival date on return travel
  //----------------------------------------------------------------
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  bool origCheck = true;
  bool destCheck = false;
  DateTime refStartDT;
  DateTime refStopDT;

  const PaxTypeFare& paxTypeFare = *fareUsage.paxTypeFare();

  paxTypeFare.warningMap().set(WarningMap::cat14_warning, true);

  if (travelRestrictionInfo.geoTblItemNo())
  {
    //---------------------------------------------------------------
    // In PringUnit validation, we do not skip validation even if scope
    // is for fare component, because we might have skipped FC validation
    // on record2 InOutInd or Directionality 3,4
    //---------------------------------------------------------------

    // Now, identify the travel segments
    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;
    // By default StartDate is SUB JOURNEY(PU) based rule;
    // StopDate is FARE COMPONENT based rule;
    RuleConst::TSIScopeParamType scopeParam = (needStartDTCheck)
                                                  ? RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY
                                                  : RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    if (!RuleUtil::validateGeoRuleItem(travelRestrictionInfo.geoTblItemNo(),
                                       paxTypeFare.vendor(),
                                       scopeParam,
                                       true,
                                       true,
                                       true,
                                       trx,
                                       nullptr,
                                       itin,
                                       &pricingUnit,
                                       paxTypeFare.fareMarket(),
                                       TrxUtil::getTicketingDT(trx),
                                       applTravelSegment, // this will contain the results
                                       true,
                                       false,
                                       fltStopCheck,
                                       tsiReturn,
                                       locKey1Return,
                                       locKey2Return,
                                       Diagnostic314)) // lint !e603
    {
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - SKIP");

      diag << " TRAVEL RESTRICTIONS: SKIP "
           << "- FALSE FROM VALIDATE GEO RULE ITEM\n";

      return SKIP;
    }

    // If there is a TSI attached to the Table 995, the TSI can
    //  override the origin and destination checks.
    // The default is check origin not destination.
    int32_t itemNo = travelRestrictionInfo.geoTblItemNo();
    bool tsiOverrideOrigDestChk = true;

    if (UNLIKELY(!RuleUtil::getTSIOrigDestCheckFromGeoRuleItem(
            itemNo, paxTypeFare.vendor(), trx, origCheck, destCheck)))
    {
      tsiOverrideOrigDestChk = false;
    }

    //------------------------------------------------------
    // GEO table might overwrite assumptions
    // Do some diagnostic
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

    if (needStartDTCheck)
    {
      RuleUtil::getFirstValidTvlDT(refStartDT, applTravelSegment, origCheck, noPNRTrx);

      if (UNLIKELY(!refStartDT.isValid()))
      {
        needStartDTCheck = false;
      }
      else if (isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
      {
        nvaStartSegOrder = itin->segmentOrder(applTravelSegment.front()->travelSeg());
      }
    }

    if (needStopDTCheck)
    {
      // If TSI scope is TSI_SCOPE_SJ_AND_FC, StartDate uses PU scope,
      // StopDate uses FC scope, otherwise both use same TravelSegs
      bool stopDTOnDifferentTvlSeg = false;
      bool needNVADataOnFareUsage = true;

      if (scopeParam == RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY)
      {
        RuleConst::TSIScopeType tsiScopeRtn = RuleConst::TSI_SCOPE_FARE_COMPONENT;
        if (!RuleUtil::getTSIScopeFromGeoRuleItem(
                travelRestrictionInfo.geoTblItemNo(), paxTypeFare.vendor(), trx, tsiScopeRtn))
        {
          LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

          diag << " TRAVEL RESTRICTIONS: FAIL - NO TSI SCOPE FROM GEO\n";
          return FAIL;
        }
        if (tsiScopeRtn == RuleConst::TSI_SCOPE_SJ_AND_FC)
        {
          stopDTOnDifferentTvlSeg = true;
          needNVADataOnFareUsage = false;
        }
        else if (tsiScopeRtn == RuleConst::TSI_SCOPE_FARE_COMPONENT)
        {
          needNVADataOnFareUsage = false;
        }
      }

      if (stopDTOnDifferentTvlSeg)
      {
        // Default should be FC, we need reget applTravelSegment
        applTravelSegment.clear();
        RuleConst::TSIScopeParamType scopeParamDefault = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
        int32_t itemNo = travelRestrictionInfo.geoTblItemNo();

        if (!RuleUtil::validateGeoRuleItem(itemNo,
                                           paxTypeFare.vendor(),
                                           scopeParamDefault,
                                           true,
                                           true,
                                           true,
                                           trx,
                                           isFPFake ? nullptr : &farePath,
                                           itin,
                                           &pricingUnit,
                                           paxTypeFare.fareMarket(),
                                           TrxUtil::getTicketingDT(trx),
                                           applTravelSegment, // this will contain the results
                                           true,
                                           false,
                                           fltStopCheck,
                                           tsiReturn,
                                           locKey1Return,
                                           locKey2Return,
                                           Diagnostic314))
        {
          diag << " TRAVEL RESTRICTIONS: SKIP STOP DT CHECK"
               << "- FALSE FROM VALIDATE GEO RULE ITEM\n";

          needStopDTCheck = false;
        }
      }

      if (LIKELY(needStopDTCheck))
      {
        //-------------------------------------------------------------
        // Depends on travelRestrictionInfo.returnTvlInd(),
        // we need to either validate against orig return departure
        // or final return arrival;
        // But if GEO table is of not a F.C. TSI, this can be override
        // to a P.U. application (TVLMUSTBECOMPLETEDBY will be ignore)
        //-------------------------------------------------------------
        if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
        {
          if (needNVADataOnFareUsage || trx.getRequest()->isSFR())
          {
            nvaStopSegOrder = itin->segmentOrder(applTravelSegment.back()->travelSeg());
          }

          RuleUtil::getLastValidTvlDT(refStopDT,
                                      applTravelSegment,
                                      tsiOverrideOrigDestChk ? (!destCheck) : false,
                                      noPNRTrx);
          // will still check against arrival if TSI did not
          // override origCheck, destCheck
        }
        else
        {
          if (needNVADataOnFareUsage || trx.getRequest()->isSFR())
          {
            nvaStopSegOrder = itin->segmentOrder(applTravelSegment.front()->travelSeg());
          }

          RuleUtil::getFirstValidTvlDT(refStopDT, applTravelSegment, origCheck, noPNRTrx);
        }

        if (!refStopDT.isValid())
        {
          needStopDTCheck = false;
        }
      }
    }
  } // geoTblItemNo
  else
  {
    //----------------------------------------------------------------------
    // Start From/To date validation is for Pricing Unit
    //----------------------------------------------------------------------
    if (needStartDTCheck)
    {
      //------------------------------------------------------------------
      // Take all airseg from the pricing unit and put it into the
      // applTravelSegment vector because there was no 995 for populate it
      //------------------------------------------------------------------
      if (UNLIKELY(pricingUnit.travelSeg().empty()))
      {
        diag << " TRAVEL RESTRICTIONS: FAIL "
             << "- FARE MARKET TRAVEL SEG EMPTY\n";

        LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");
        return FAIL;
      }

      origCheck = true;
      RuleUtil::getFirstValidTvlDT(refStartDT, pricingUnit.travelSeg(), origCheck, noPNRTrx);
      if (UNLIKELY(!refStartDT.isValid()))
      {
        needStartDTCheck = false;
      }
      else if (isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
      {
        nvaStartSegOrder = itin->segmentOrder(pricingUnit.travelSeg().front());
      }
    } // needStartDTCheck

    //----------------------------------------------------------------------
    // Commence/Complete date validation is for Fare Component
    //----------------------------------------------------------------------
    if (needStopDTCheck)
    {
      const std::vector<TravelSeg*>& tvlSegs = paxTypeFare.fareMarket()->travelSeg();

      //----------------------------------------------------------------
      // Depends on travelRestrictionInfo.returnTvlInd(),
      // we need to either validate against orig return departure
      // or final return arrival;
      //----------------------------------------------------------------
      if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMPLETEDBY)
      {
        nvaStopSegOrder = itin->segmentOrder(tvlSegs.back());
        RuleUtil::getLastValidTvlDT(refStopDT, tvlSegs, false, noPNRTrx);
      }
      else
      {
        nvaStopSegOrder = itin->segmentOrder(tvlSegs.front());
        RuleUtil::getFirstValidTvlDT(refStopDT, tvlSegs, true, noPNRTrx);
      }

      if (UNLIKELY(!refStopDT.isValid()))
      {
        needStopDTCheck = false;
      }
    }
  }

  if (needStartDTCheck)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " TRAVEL DATE FOR FROM/TO VALIDATION " << refStartDT.toIsoExtendedString() << "\n";
    }

    if (validateTvlStartDate(refStartDT, travelRestrictionInfo, diag) == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - START DATE\n";
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

      (const_cast<FareUsage&>(fareUsage)).setFailedFound();

      return FAIL;
    }
  }

  if (needStopDTCheck)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " TVL DATE FOR COMMENCE/COMPLETE VALID " << refStopDT.toIsoExtendedString() << "\n";
    }

    if (validateTvlStopTime(refStopDT, travelRestrictionInfo, diag) == FAIL)
    {
      diag << " TRAVEL RESTRICTIONS: FAIL - COMMENCE/COMPLETE DATE\n";
      LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - FAIL");

      return FAIL;
    }
  }

  LOG4CXX_INFO(_logger, " Leaving TravelRestrictions::validate() - PASS");
  diag << "\n TRAVEL RESTRICTIONS: PASS \n";

  if (nvaStartSegOrder > 0)
  {
    const LocCode location = itin->travelSeg()[nvaStartSegOrder - 1]->boardMultiCity();
    notifyObservers(NotificationType::TRAVEL_MUST_COMMENCE,
                    nvaStartSegOrder,
                    travelRestrictionInfo.latestTvlStartDate(),
                    location,
                    NO_TIME);
  }

  if (nvaStopSegOrder > 0)
  {
    const LocCode location = itin->travelSeg()[nvaStopSegOrder - 1]->boardMultiCity();
    notifyObservers(travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMMENCEDBY
                        ? (NotificationType::TRAVEL_MUST_COMMENCE)
                        : (NotificationType::TRAVEL_MUST_COMPLETE),
                    nvaStopSegOrder,
                    travelRestrictionInfo.stopDate(),
                    location,
                    travelRestrictionInfo.stopTime());
  }

  return PASS;
}

template <class T>
class TravelRestrictionObservers
{
  using TRObserverType = TravelRestrictionsObserverType<T>;

public:
  TravelRestrictionObservers(T& object, TravelRestrictions& travelRestriction, PricingTrx& trx)
    : _object(object), _travelRestriction(travelRestriction)
  {
    _observerVec.push_back(TRObserverType::create(trx.getRequest()->isSFR()
                                                      ? (ObserverType::TRV_REST_COMMENCE_SFR)
                                                      : (ObserverType::TRV_REST_COMMENCE),
                                                  trx.dataHandle(),
                                                  &_travelRestriction));
    _observerVec.push_back(TRObserverType::create(trx.getRequest()->isSFR()
                                                      ? (ObserverType::TRV_REST_COMPLETE_SFR)
                                                      : (ObserverType::TRV_REST_COMPLETE),
                                                  trx.dataHandle(),
                                                  &_travelRestriction));
  }

  void updateIfNotified()
  {
    for (auto& observer : _observerVec)
      observer->updateIfNotified(_object);
  }

private:
  T& _object;
  TravelRestrictions& _travelRestriction;
  std::vector<std::unique_ptr<TRObserverType>> _observerVec;
};

Record3ReturnTypes
TravelRestrictionsObserverWrapper::validate(PricingTrx& trx,
                                            Itin& itin,
                                            PaxTypeFare& paxTypefare,
                                            const RuleItemInfo* ruleInfo,
                                            const FareMarket& fareMarket)
{
  TravelRestrictionObservers<PaxTypeFare> observers(paxTypefare, _travelRestriction, trx);
  Record3ReturnTypes ret =
      _travelRestriction.validate(trx, itin, paxTypefare, ruleInfo, fareMarket);
  observers.updateIfNotified();

  return ret;
}
Record3ReturnTypes
TravelRestrictionsObserverWrapper::validate(PricingTrx& trx,
                                            const RuleItemInfo* ruleInfo,
                                            const Itin& itin,
                                            const PricingUnit& pricingUnit,
                                            FareUsage& fareUsage)
{
  TravelRestrictionObservers<FareUsage> observers(fareUsage, _travelRestriction, trx);

  Record3ReturnTypes ret = _travelRestriction.validate(trx, ruleInfo, itin, pricingUnit, fareUsage);

  observers.updateIfNotified();
  return ret;
}

Record3ReturnTypes
TravelRestrictionsObserverWrapper::validate(PricingTrx& trx,
                                            const RuleItemInfo* ruleInfo,
                                            const FarePath& farePath,
                                            const PricingUnit& pricingUnit,
                                            FareUsage& fareUsage)
{
  TravelRestrictionObservers<FareUsage> observers(fareUsage, _travelRestriction, trx);

  Record3ReturnTypes ret =
      _travelRestriction.validate(trx, ruleInfo, farePath, pricingUnit, fareUsage);
  observers.updateIfNotified();
  return ret;
}

void
TravelRestrictionsObserverWrapper::setRuleDataAccess(RuleControllerDataAccess* ruleDataAccess)
{
  _travelRestriction.setRuleDataAccess(ruleDataAccess);
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validateTvlStartDate()
//
// validateTvlStartDate()  validate travel start time meets requirement
//
//  @param DateTime&  refDT     - Reference DateTime, could be of departure
//                                or arrival
//  @param TravelRestriction&   - Travel Restriction Rule Item Info
//  @param DiagCollector&       - Diagnostic Collector
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validateTvlStartDate(const DateTime& refDT,
                                         const TravelRestriction& travelRestrictionInfo,
                                         DiagManager& diag) const
{
  const DateTime& earlistDT = travelRestrictionInfo.earliestTvlStartDate();

  // The Hour:Minute we get would be 0:00 as they are not defined in rule data
  if (isValidATSEDate(earlistDT) && (refDT < earlistDT))
  {
    diag << "TRAVEL DATE IS EARLIER THAN FIRST DATE PERMITTED\n";
    return FAIL;
  }

  const DateTime& latestDT = travelRestrictionInfo.latestTvlStartDate();

  if (isValidATSEDate(latestDT) && (refDT.date() > latestDT.date()))
  {
    diag << "TRAVEL DATE IS LATER THAN LAST DATE PERMITTED\n";
    return FAIL;
  }

  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validateTvlStartDate()
//
// validateTvlStartDate()  validate PU travel start time meets requirement
//   when we have not got PU information yet.
//   Failure is based on
//     JourneyStartDT <= SubJourneyStartDT <= FareComponentStartDT
//   (SubJourney is for PU)
//
//  @param DateTime&  refJourneyDT  - Reference Journey Start DateTime
//  @param DateTime&  refFareCompDT - Reference Fare Component DateTime
//  @param TravelRestriction&   - Travel Restriction Rule Item Info
//  @param DiagCollector&       - Diagnostic Collector
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validateTvlStartDate(const DateTime& refJourneyDT,
                                         const DateTime& refFareCompDT,
                                         const TravelRestriction& travelRestrictionInfo,
                                         DiagManager& diag) const
{
  const DateTime& earlistDT = travelRestrictionInfo.earliestTvlStartDate();

  // The Hour:Minute we get would be 0:00 as they are not defined in rule data
  if ((earlistDT.isValid()) && (refFareCompDT < earlistDT))
  {
    diag << "TRAVEL DATE IS EARLIER THAN FIRST DATE PERMITTED\n";
    return FAIL;
  }

  const DateTime& latestDT = travelRestrictionInfo.latestTvlStartDate();

  if ((latestDT.isValid()) && (refJourneyDT.date() > latestDT.date()))
  {
    diag << "TRAVEL DATE IS LATER THAN LAST DATE PERMITTED\n";
    return FAIL;
  }

  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    TravelRestrictions::validateTvlStopTime()
//
// validateTvlStopTime()  validate travel return time meets requirement
//
//  @param DateTime&  refDT     - Reference DateTime, could be of departure
//                                or arrival
//  @param TravelRestriction&   - Travel Restriction Rule Item Info
//  @param DiagCollector&       - Diagnostic Collector
//
//  @return Record3ReturnTypes - possible values are:
//                               FAIL          = 2
//                               PASS          = 3
//
// </PRE>
//-------------------------------------------------------------------
Record3ReturnTypes
TravelRestrictions::validateTvlStopTime(const DateTime& rtnRefDT,
                                        const TravelRestriction& travelRestrictionInfo,
                                        DiagManager& diag) const
{
  int32_t hrs = 23;
  int32_t mins = 59;

  if (LIKELY(isValidTOD(travelRestrictionInfo.stopTime())))
  {
    hrs = travelRestrictionInfo.stopTime() / 60;
    mins = travelRestrictionInfo.stopTime() % 60;
  }

  DateTime rtnStopDT = DateTime(travelRestrictionInfo.stopDate().date(), hrs, mins, 0);

  if (rtnRefDT > rtnStopDT)
  {
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
// @MethodName    TravelRestrictions::displayRuleDataToDiag()
//
// displayRuleDataToDiag()   read fields in TravelRestriction so to put details
//                           about the rule item to diagnostic collector
//
//  @param TravelRestriction&   - Travel Restriction Rule Item Info
//  @param DiagCollector*       - Diagnostic Collector
//
//  @return void
//
// </PRE>
//-------------------------------------------------------------------
void
TravelRestrictions::displayRuleDataToDiag(const TravelRestriction& travelRestrictionInfo,
                                          DiagCollector& diag)
{
  //------------------------------------------------------------------------
  //   CATEGORY 14 RULE DATA
  //------------------------------------------------------------------------
  diag << "***************************************************************" << std::endl;
  diag << "CATEGORY 14 RULE DATA:" << std::endl;

  //--------------------------
  // Unavailable Tag
  //--------------------------
  if (travelRestrictionInfo.unavailTag() != NOTAPPLY)
  {
    if (travelRestrictionInfo.unavailTag() == dataUnavailable)
    {
      diag << " UNAVAILABLE DATA" << std::endl;
    }
    else
    {
      diag << " TEXT DATA ONLY" << std::endl;
    }
  }

  //--------------------------------------
  // Geographic Specification Table 995
  //--------------------------------------
  diag << " GEOGRAPHIC SPECIFICATION TABLE 995: " << travelRestrictionInfo.geoTblItemNo()
       << std::endl;

  //--------------------------------------
  // Travel From / Travel To
  //--------------------------------------
  diag << " FIRST DATE TRAVEL IS PERMITTED TO COMMENCE: ";
  if (isValidATSEDate(travelRestrictionInfo.earliestTvlStartDate()))
    diag << travelRestrictionInfo.earliestTvlStartDate().toIsoExtendedString() << std::endl;
  else
    diag << "N/A" << std::endl;

  // Latest Travel Commence Date
  diag << " LAST  DATE TRAVEL IS PERMITTED TO COMMENCE: ";
  if (isValidATSEDate(travelRestrictionInfo.latestTvlStartDate()))
    diag << travelRestrictionInfo.latestTvlStartDate().toIsoExtendedString() << std::endl;
  else
    diag << "N/A" << std::endl;

  //--------------------------
  // Travel Commence/Complete
  //--------------------------

  if (travelRestrictionInfo.returnTvlInd() != NOTAPPLY &&
      isValidATSEDate(travelRestrictionInfo.stopDate()))
  {
    if (travelRestrictionInfo.returnTvlInd() == TVLMUSTBECOMMENCEDBY)
    {
      // Last Date Travel Must Commence
      diag << " LAST DATE TRAVEL MUST COMMENCE: "
           << travelRestrictionInfo.stopDate().dateToIsoExtendedString();
    }
    else
    {
      // Last Date Travel Must be Completed
      diag << " LAST DATE TRAVEL MUST BE COMPLETED: "
           << travelRestrictionInfo.stopDate().dateToIsoExtendedString();
    }
    // TimeOfDay
    if (isValidTOD(travelRestrictionInfo.stopTime()))
    {
      diag << "T" << std::setw(2) << std::setfill('0') << travelRestrictionInfo.stopTime() / 60
           << ":" << std::setw(2) << std::setfill('0') << travelRestrictionInfo.stopTime() % 60
           << ":00";
    }
    diag << std::endl;
  }

  //------------------------------------------------------------------------
  //    End of Display
  //------------------------------------------------------------------------
  diag << "***************************************************************" << std::endl;

  diag.flushMsg();
}

bool
TravelRestrictions::isGeoScopeSubJourney(RuleConst::TSIScopeType geoScope) const
{
  return geoScope != RuleConst::TSI_SCOPE_FARE_COMPONENT &&
         geoScope != RuleConst::TSI_SCOPE_JOURNEY;
}

bool
TravelRestrictions::isTravelIndNotApplicable(Indicator tvlInd) const
{
  return tvlInd == NOTAPPLY;
}

} // tse
