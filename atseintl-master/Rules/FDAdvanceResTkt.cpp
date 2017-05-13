//-------------------------------------------------------------------
//
//  File:        FDAdvanceResTkt.cpp
//
//  Authors:     Marco Cartolano
//  Created:     March 15, 2005
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
//-------------------------------------------------------------------

#include "Rules/FDAdvanceResTkt.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/LocKey.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
static Logger
logger("atseintl.Rules.FDAdvanceResTkt");

const ResPeriod FDAdvanceResTkt::DOLLAR_SIGNS = "$$$";

// ---------------------------------
// Constructor, Destructor Section
// ---------------------------------

FDAdvanceResTkt::FDAdvanceResTkt() {}

FDAdvanceResTkt::~FDAdvanceResTkt() {}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDAdvanceResTkt::validate()
//
// validate() Performs rule validations for advanced reservation/ticketing
//              on a FareMarket for Fare Display transactions.
//
//  @param PricingTrx&          - Pricing transaction
//  @param Itin&                - itinerary
//  @param PaxTypeFare&         - reference to Pax Type Fare
//  @param RuleItemInfo*        - Record 2 Rule Item Segment Info
//                                need dynamic_cast to <AdvResTktInfo*>
//  @param FareMarket&          - Fare Market
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
FDAdvanceResTkt::validate(PricingTrx& trx,
                          Itin& itin,
                          const PaxTypeFare& paxTypeFare,
                          const RuleItemInfo* rule,
                          const FareMarket& fareMarket,
                          bool isQualifiedCategory)
{
  LOG4CXX_INFO(logger, " Entered FDAdvanceResTkt::validate()");

  //--------------------------------------------------------------
  // Get a Fare Display Transaction from the Pricing Transaction
  //--------------------------------------------------------------
  FareDisplayUtil fdUtil;
  FareDisplayTrx* fdTrx;

  if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
  {
    LOG4CXX_DEBUG(logger, "Unable to get FareDisplayTrx");
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - FAIL");

    return FAIL;
  }

  const AdvResTktInfo* advResTktInfo = dynamic_cast<const AdvResTktInfo*>(rule);

  if (advResTktInfo == nullptr)
  {
    LOG4CXX_DEBUG(logger, "Not valid AdvResTktInfo");
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - FAIL");

    return FAIL;
  }

  const AdvResTktInfo& advanceResTktInfo = *advResTktInfo;

  //-------------------------------------------------------------------
  // For Diagnostic display
  //-------------------------------------------------------------------
  DiagManager diag(trx, Diagnostic305);

  if (UNLIKELY(diag.isActive()))
  {
    displayRuleDataToDiag(advanceResTktInfo, diag.collector());
  }

  //----------------------------------------------------------------
  // Check if it is a request to exclude Advance Purchase restricted
  // fares or to exclude any restricted fares
  //----------------------------------------------------------------
  FareDisplayOptions* fdOptions = fdTrx->getOptions();

  if (fdOptions &&
      (fdOptions->isExcludeAdvPurchaseFares() || fdOptions->isExcludeRestrictedFares()))
  {
    if (existResTktRestr(advanceResTktInfo))
    {
      diag << " ADVANCE RESTKT: FAIL - RESTRICTION APPLY\n";

      return FAIL;
    }
    else
    {
      diag << " ADVANCE RESTKT: PASS - NO RESTRICTIONS";

      return PASS;
    }
  }

  //-------------------------------------------------------------------
  // Get Fare Display Info object
  //-------------------------------------------------------------------
  FareDisplayInfo* fdInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());

  if (!fdInfo)
  {
    LOG4CXX_DEBUG(logger, "Unable to get FareDisplayInfo object");
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - FAIL");

    return FAIL;
  }

  //--------------------------------------------------------------
  // Check if data is unavailable
  //--------------------------------------------------------------
  if (advanceResTktInfo.unavailtag() == dataUnavailable)
  {
    //-------------------------------------------------------------------
    // Update FareDisplayInfo object: Unavailable rule data
    //-------------------------------------------------------------------
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");

    fdInfo->setUnavailableR3Rule(RuleConst::ADVANCE_RESERVATION_RULE);

    diag << " ADVANCE RESTKT: NOT PROCESSED - DATA UNAVAILABLE\n";

    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - NOTPROCESSED");

    return NOTPROCESSED;
  }

  //-------------------------------------------------------------------
  // Update FareDisplayInfo object with Advance Res/Tkt information
  //-------------------------------------------------------------------
  if (!isQualifiedCategory)
  {
    LOG4CXX_INFO(logger, " Updating FareDisplayInfo object");
    updateFareDisplayInfo(advanceResTktInfo, fdInfo);
  }

  //----------------------------------------------------------------
  // Check if it is a request to skip rules validation
  //----------------------------------------------------------------
  if (fdOptions && !fdOptions->isValidateRules())
  {
    diag << " ADVANCE RESTKT: NOT PROCESSED - SKIP RULES VALIDATION\n";

    LOG4CXX_INFO(logger, " No Validation qualifier - skip Advance Res/Tkt Validation");

    return NOTPROCESSED;
  }

  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  Record3ReturnTypes r3rtn = validateUnavailableDataTag(advanceResTktInfo.unavailtag());

  if (r3rtn == FAIL)
  {
    diag << " ADVANCE RESTKT: FAIL - DATA UNAVAIBLE\n";

    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - FAIL");

    return FAIL;
  }
  else if (r3rtn == SKIP)
  {
    diag << " ADVANCE RESTKT: SKIP - TXT DATA ONLY\n";

    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - SKIP");

    return SKIP;
  }

  r3rtn = determineAndValidateTvlSeg(
      *fdTrx, itin, advanceResTktInfo, &fareMarket, paxTypeFare.vendor(), diag, paxTypeFare.owrt());

  if (r3rtn == SOFTPASS)
  {
    // There's no softpass for Fare Display
    r3rtn = PASS;
  }

  if (r3rtn == FAIL)
  {
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - FAIL");
  }
  else if (r3rtn == SKIP)
  {
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - SKIP");
  }
  else if (r3rtn == PASS)
  {
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate() - PASS");
  }
  else
    LOG4CXX_INFO(logger, " Leaving FDAdvanceResTkt::validate()");

  return r3rtn;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDAdvanceResTkt::determineAndValidateTvlSeg()
//
// determineAndValidateTvlSeg()   is the function to determine the
//                       travel segments that we need to perform perform
//                       rule validations for advanced reservation/ticketing
//                       on FareMarket for Fare Display.
//
//  @param FareDisplayTrx&      - Fare Display transaction
//  @param AdvResTktInfo&       - Advance ResTkt Rule Item Info
//  @param FareMarket*          - Fare Market
//  @param VendorCode&          - Vendor Code
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
FDAdvanceResTkt::determineAndValidateTvlSeg(FareDisplayTrx& trx,
                                            const Itin& itin,
                                            const AdvResTktInfo& advanceResTktInfo,
                                            const FareMarket* fareMarket,
                                            const VendorCode& vendor,
                                            DiagManager& diag,
                                            Indicator owrt) const
{
  bool displayWqWarning = false;

  //-----------------------------------------------------------
  // fareMarket must not be 0 for fare market validation
  //-----------------------------------------------------------
  if (!fareMarket)
  {
    return FAIL;
  }

  //-----------------------------------------------------------
  // Ticketing Date is commonly used, get it here.
  //-----------------------------------------------------------
  DateTime tktDT = TrxUtil::getTicketingDT(trx);

  //-----------------------------------------------------------
  // Gather the travel segments that we need to validate
  // resTSI, tktTSI and GeoTblItemNo will be used to identify
  // segments when value is not 0; otherwise, we use default
  // (fare market) segments
  //-----------------------------------------------------------
  bool resUseDefaultSegs = true;
  bool tktUseDefaultSegs = true;

  //-----------------------------------------------------------
  // First:  Segments for Reservation
  // If resTSI exists, identify the segments information by TSI
  // and go ahead validating reservation
  // If ticketing and reservation use same TSI, validate ticketing
  // with identified segments as well
  //-----------------------------------------------------------
  const TSICode resTSI = advanceResTktInfo.resTSI();
  const TSICode tktTSI = advanceResTktInfo.tktTSI();

  bool doResTktValidSameTvlSegs = (tktTSI == resTSI) ? true : false;

  // code change for cat 5
  bool skipResPermittedChk = true; // this will bypass the cat 5 validation when checking open
                                   // returns , true for FD and false for Pricing

  FarePath* farePath = nullptr;
  trx.dataHandle().get(farePath);
  if (farePath == nullptr)
  {
    diag << " ADVANCERESTKT: FAIL - UNABLE TO CREATE FAREPATH\n";

    LOG4CXX_DEBUG(
        logger,
        "FDAdvanceResTkt::determineAndValidateTvlSeg - UNABLE TO ALLOCATE MEMORY FOR FAREPATH");
    LOG4CXX_DEBUG(logger, "Leaving FDAdvanceResTkt::determineAndValidateTvlSeg - FAIL");
    return FAIL;
  }

  PricingUnit* pu = nullptr;
  if (!FareDisplayUtil::initializeFarePath(trx, farePath))
  {
    farePath = nullptr;
  }
  else
    pu = farePath->pricingUnit().front();

  if (resTSI != 0)
  {
    RuleUtil::TravelSegWrapperVector resApplTravelSegment;

    LocKey loc1;
    LocKey loc2;

    RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    // Fare Component (FareMarket) that a TSI applies to
    if (!RuleUtil::scopeTSIGeo(resTSI,
                               loc1,
                               loc2,
                               defaultScope,
                               false, // allowJourneyScopeOverride
                               false, // allowPUScopeOverride
                               true, // allowFCScopeOverride
                               trx,
                               //                                   0,           // farePath
                               //                                   0,           // pricingUnit
                               farePath, // farePath
                               nullptr,
                               pu, // pricingUnit
                               fareMarket,
                               tktDT,
                               resApplTravelSegment, // this will contain the results
                               DiagnosticNone,
                               vendor))
    {
      diag << " ADVANCE RESTKT- FAIL - FALSE FROM SCOPE TSI GEO\n";
      return FAIL;
    }
    else if (resApplTravelSegment.empty())
    {
      diag << " ADVANCE RESTKT: FAIL - NO MATCH FROM RES TSI\n";
      return FAIL;
    }
    else
    {
      // Go ahead validate everything
      resUseDefaultSegs = false;

      // see if we will validate Tkt Reqs the same time
      bool needTktDTChk = false;

      if (doResTktValidSameTvlSegs)
      {
        tktUseDefaultSegs = false;
        needTktDTChk = true;
      }

      bool mayPassAfterRebook = false;
      bool failedCfmStat = false;
      const Record3ReturnTypes result = validateTvlSegs(trx,
                                                        itin,
                                                        nullptr, // pricingUnit
                                                        resApplTravelSegment,
                                                        advanceResTktInfo,
                                                        *resApplTravelSegment.front(),
                                                        true, // needResDTChk
                                                        needTktDTChk,
                                                        false, // needConfStatChk
                                                        nullptr, // segsRebookStatus
                                                        diag,
                                                        mayPassAfterRebook,
                                                        displayWqWarning,
                                                        displayWqWarning,
                                                        tktDT,
                                                        skipResPermittedChk,
                                                        failedCfmStat,
                                                        (PaxTypeFare*)nullptr);

      if (result == FAIL || result == SKIP)
      {
        return result;
      }
    }
  } // resTSI != 0

  //-----------------------------------------------------------
  // Next:  if tktTSI exists and diff from resTSI, identify segments
  //  by tktTSI, and do the validation if succeeded
  //-----------------------------------------------------------
  if (tktTSI != 0 && !doResTktValidSameTvlSegs)
  {
    RuleUtil::TravelSegWrapperVector tktApplTravelSegment;

    LocKey loc1;
    LocKey loc2;

    RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;

    if (!RuleUtil::scopeTSIGeo(tktTSI,
                               loc1,
                               loc2,
                               defaultScope,
                               false, // allowJourneyScopeOverride
                               false, // allowPUScopeOverride
                               true, // allowFCScopeOverride
                               trx,
                               //                                   0,           // farePath
                               //                                   0,           // pricingUnit
                               farePath, // farePath
                               nullptr,
                               pu, // pricingUnit
                               fareMarket,
                               tktDT,
                               tktApplTravelSegment, // this will contain the results
                               DiagnosticNone,
                               vendor))
    {
      diag << " ADVANCE RESTKT - FAIL - FALSE FROM SCOPE TKT TSI\n";
      return FAIL;
    }
    else if (tktApplTravelSegment.empty())
    {
      diag << " ADVANCE RESTKT: FAIL - NO MATCH FROM TKT TSI\n";
      return FAIL;
    }
    else
    {
      tktUseDefaultSegs = false;
      bool mayPassAfterRebook = false;
      bool failedCfmStat = false;
      // validate for ticketing
      const Record3ReturnTypes result = validateTvlSegs(trx,
                                                        itin,
                                                        nullptr, // pricingUnit
                                                        tktApplTravelSegment,
                                                        advanceResTktInfo,
                                                        *tktApplTravelSegment.front(),
                                                        false, // needResDTCheck
                                                        true, // needTktDTCheck
                                                        false, // needConfStatCheck
                                                        nullptr, // segsRebookStatus
                                                        diag,
                                                        mayPassAfterRebook,
                                                        displayWqWarning,
                                                        displayWqWarning,
                                                        tktDT,
                                                        skipResPermittedChk,
                                                        failedCfmStat,
                                                        (PaxTypeFare*)nullptr);

      if (result == FAIL || result == SKIP)
      {
        return result;
      }
    }
  }

  //----------------------------------------------------------------------
  // If either of Res or Tkt is using default (fare market) segments to
  // validate, we need to identify the travel segments by
  // fareMarket.travelSeg
  //----------------------------------------------------------------------
  if (resUseDefaultSegs || tktUseDefaultSegs)
  {
    // Put all airseg from the fare market into the
    // applTravelSegment vector
    RuleUtil::TravelSegWrapperVector applTravelSegment;

    if (!fareMarket->travelSeg().empty())
    {
      std::vector<TravelSeg*>::const_iterator travelSegmentI;
      travelSegmentI = fareMarket->travelSeg().begin();
      for (; travelSegmentI != fareMarket->travelSeg().end(); ++travelSegmentI)
      {
        if (dynamic_cast<AirSeg*>(*travelSegmentI))
        {
          RuleUtil::TravelSegWrapper* tsw = nullptr;
          trx.dataHandle().get(tsw);
          // lint --e{413}
          tsw->travelSeg() = *travelSegmentI;
          tsw->origMatch() = true;
          tsw->destMatch() = false;

          applTravelSegment.push_back(tsw);
        }
      }
    }
    else
    {
      diag << " ADVANCE RESTKT: FAIL - FARE MARKET TRAVEL SEG EMPTY\n";
      return FAIL;
    }

    if (applTravelSegment.empty())
    {
      diag << " ADVANCE RESTKT: FAIL - NO MATCH FROM GEO\n";
      return FAIL;
    }
    bool mayPassAfterRebook = false;
    bool failedCfmStat = false;
    const Record3ReturnTypes result = validateTvlSegs(trx,
                                                      itin,
                                                      nullptr, // pricingUnit
                                                      applTravelSegment,
                                                      advanceResTktInfo,
                                                      *applTravelSegment.front(),
                                                      resUseDefaultSegs, // needResDTCheck
                                                      tktUseDefaultSegs, // needTktDTCheck
                                                      false, // needConfStatCheck
                                                      nullptr, // segsRebookStatus
                                                      diag,
                                                      mayPassAfterRebook,
                                                      displayWqWarning,
                                                      displayWqWarning,
                                                      tktDT,
                                                      skipResPermittedChk,
                                                      failedCfmStat,
                                                      (PaxTypeFare*)nullptr);

    if (result == FAIL)
    {
      if (needReturnSegment(trx, owrt))
      {
        applTravelSegment.clear();

        AirSeg* returnSeg = dynamic_cast<AirSeg*>(itin.travelSeg().back());
        returnSeg->bookingDT() = trx.bookingDate();

        RuleUtil::TravelSegWrapper* tsw = nullptr;
        trx.dataHandle().get(tsw);
        tsw->travelSeg() = returnSeg;
        tsw->origMatch() = true;
        tsw->destMatch() = false;

        applTravelSegment.push_back(tsw);

        const Record3ReturnTypes result = validateTvlSegs(trx,
                                                          itin,
                                                          nullptr, // pricingUnit
                                                          applTravelSegment,
                                                          advanceResTktInfo,
                                                          *applTravelSegment.front(),
                                                          resUseDefaultSegs, // needResDTCheck
                                                          tktUseDefaultSegs, // needTktDTCheck
                                                          false, // needConfStatCheck
                                                          nullptr, // segsRebookStatus
                                                          diag,
                                                          mayPassAfterRebook,
                                                          displayWqWarning,
                                                          displayWqWarning,
                                                          tktDT,
                                                          skipResPermittedChk,
                                                          failedCfmStat,
                                                          (PaxTypeFare*)nullptr);

        if (result == FAIL || result == SKIP)
          return result;
      }
      else
        return result;
    }

    if (result == SKIP)
      return result;
  } // resUseDefaultSegs || tktUseDefaultSegs

  diag << " ADVANCE RESTKT: PASS \n";
  return PASS;
}

//-------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDAdvanceResTkt::updateFareDisplayInfo()
//
// updateFareDisplayInfo()  update FareDisplayInfo object with Advance
//                      Res/Tkt information
//
//  @param AdvResTktInfo&      - reference to Advance Res/Tkt rule
//  @param FareDisplayInfo*&   - reference to Fare Display Info ptr
//
// </PRE>
//-------------------------------------------------------------------
void
FDAdvanceResTkt::updateFareDisplayInfo(const AdvResTktInfo& advanceResTktInfo,
                                       FareDisplayInfo*& fdInfo) const
{
  //-----------------------------------
  // Check Advance Res information
  //-----------------------------------

  // Check if there's "Earliest time reservation is permitted" restriction
  //  or "Advance Res not permitted" restriction
  if ((fdInfo->isMultiAdvRes() && !advanceResTktInfo.lastResPeriod().empty()) ||
      !advanceResTktInfo.firstResPeriod().empty() || advanceResTktInfo.permitted() != notApply)
  {
    fdInfo->lastResPeriod() = DOLLAR_SIGNS; // Indicate multiple conditions
  }

  else if (!advanceResTktInfo.lastResPeriod().empty())
  {
    // Check if 3-char day of week representation
    if (isalpha(advanceResTktInfo.lastResPeriod().c_str()[0]))
    {
      fdInfo->lastResPeriod() = DOLLAR_SIGNS; // Indicate irregular condition
    }

    fdInfo->lastResPeriod() = advanceResTktInfo.lastResPeriod();
    fdInfo->lastResUnit() = advanceResTktInfo.lastResUnit();
  }

  // Save Simultaneous Res and Tkt indicator
  fdInfo->ticketed() = advanceResTktInfo.ticketed();

  //-----------------------------------
  // Check Advance Tkt information
  //-----------------------------------

  if ((fdInfo->isMultiAdvTkt() && !advanceResTktInfo.advTktPeriod().empty()) ||
      advanceResTktInfo.advTktExcpUnit() != notApply ||
      advanceResTktInfo.advTktOpt() == firstTimePermit ||
      advanceResTktInfo.advTktBoth() == laterTime)
  {
    fdInfo->advTktPeriod() = DOLLAR_SIGNS; // Indicate multiple conditions
  }

  // Check if there's "Ticketing before departure" restriction
  else if (advanceResTktInfo.advTktDepartUnit() != notApply)
  {
    // Need to convert lastResPeriod and lastResUnit
    int period = 0;

    if (fdInfo->lastResPeriod() != DOLLAR_SIGNS)
    {
      period = atoi(fdInfo->lastResPeriod().c_str());
    }

    Indicator unitInd = static_cast<Indicator>(fdInfo->lastResUnit().c_str()[0]);

    // Check if Advance Res and Advance Tkt requirements are different
    if (advanceResTktInfo.advTktdepart() != period ||
        advanceResTktInfo.advTktDepartUnit() != unitInd)
    {
      fdInfo->advTktPeriod() = DOLLAR_SIGNS; // Indicate multiple conditions
    }
  }

  // Check if there's "Ticketing after reservation" restriction
  if (!advanceResTktInfo.advTktPeriod().empty() && fdInfo->advTktPeriod() != DOLLAR_SIGNS)
  {
    // Check if 3-char day of week representation
    if (isalpha(advanceResTktInfo.advTktPeriod().c_str()[0]))
    {
      fdInfo->advTktPeriod() = DOLLAR_SIGNS; // Indicate irregular condition
    }
    else
    {
      // Save regular data
      fdInfo->advTktPeriod() = advanceResTktInfo.advTktPeriod();
      fdInfo->advTktUnit() = advanceResTktInfo.advTktUnit();
    }
  }
}
//----------------------------------------------------------------------------------
// <PRE>
//
// @MethodName    FDAdvanceResTkt::needReturnSegment()
//
// needReturnSegment()   is the function to determine if the return travel segment
//                       needs to be validated for advanced reservation/ticketing
//                       on FareMarket for Fare Display.
//
//  @param FareDisplayTrx&      - Fare Display transaction
//-----------------------------------------------------------------------------------
bool
FDAdvanceResTkt::needReturnSegment(FareDisplayTrx& fdTrx, Indicator owrt) const
{
  return ((owrt == ONE_WAY_MAY_BE_DOUBLED) && fdTrx.getRequest()->returnDate().isValid());
}
}
