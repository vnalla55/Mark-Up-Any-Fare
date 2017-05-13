//--------------------------------------------------------------------
//
//  File:         AdvanceResTkt.cpp
//  Author:       Simon Li
//  Created:      07/12/2004
//  Description:  This is Advance Reservation/Ticketing Rule Application class
//                that supports two public validate functions for Fare Market
//                and Pricing Unit respectivelly, based on passed in
//                AdvResTktInfo and other parameters required by RuleApplicationBase
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

#include "Rules/AdvanceResTkt.h"

#include "Common/ClassOfService.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FlownStatusCheck.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RtwUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/ReissueSequence.h"
#include "Diagnostic/DiagManager.h"
#include "RexPricing/RepriceSolutionValidator.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/RuleValidationChancelor.h"
#include "Rules/RuleValidationMonitor.h"
#include "Rules/UpdaterObserver.h"
#include "Util/BranchPrediction.h"

#include <utility>

namespace tse
{
Logger
AdvanceResTkt::_logger("atseintl.Rules.AdvanceResTkt");

const AdvResTktUnitName AdvanceResTkt::DAY_UNIT_NAME = "DAYS";
const AdvResTktUnitName AdvanceResTkt::MONTH_UNIT_NAME = "MTS";
const AdvResTktUnitName AdvanceResTkt::MINUTE_UNIT_NAME = "MINS";
const AdvResTktUnitName AdvanceResTkt::HOUR_UNIT_NAME = "HRS";
const AdvResTktUnitName AdvanceResTkt::UNKNOWN_UNIT_NAME = "UNKNOWN UNIT";

const TimeDuration AdvanceResTkt::SAMETIME_WINDOW = TimeDuration(0, 30, 0);

FALLBACK_DECL(rexCat5APO41906)

Record3ReturnTypes
AdvanceResTkt::validateUnavailableDataTag(PricingTrx& trx,
                                          const RuleItemInfo& rule,
                                          DiagManager& diag)
{
  const AdvResTktInfo* advResTktInfo = dynamic_cast<const AdvResTktInfo*>(&rule);

  if (advResTktInfo == nullptr)
  {
    LOG4CXX_ERROR(_logger, "Not valid AdvResTktInfo");
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - FAIL");
    return FAIL;
  }

  Record3ReturnTypes r3rtn = validateUnavailableDataTag(advResTktInfo->unavailtag());
  if (UNLIKELY(diag.isActive()))
  {
    if (r3rtn == FAIL)
      diag << " ADVANCE RESTKT: FAIL - DATA UNAVAIBLE\n";
    else if (r3rtn == SKIP)
      diag << " ADVANCE RESTKT: SKIP - TXT DATA ONLY\n";
  }
  return r3rtn;
}

Record3ReturnTypes
AdvanceResTkt::validate(PricingTrx& trx,
                        Itin& itin,
                        const PaxTypeFare& paxTypeFare,
                        const RuleItemInfo* rule,
                        const FareMarket& fareMarket)
{
  LOG4CXX_INFO(_logger, " Entered AdvanceResTkt::validate() for FareMarket");

  DiagManager diag(trx, Diagnostic305);
  DataHandle& dh = trx.dataHandle();
  _dataHandle = &dh;

  const AdvResTktInfo* advResTktInfo = dynamic_cast<const AdvResTktInfo*>(rule);

  if (UNLIKELY(advResTktInfo == nullptr))
  {
    LOG4CXX_ERROR(_logger, "Not valid AdvResTktInfo");
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - FAIL");
    return FAIL;
  }

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(trx) &&
      (paxTypeFare.retrievalInfo() == nullptr || paxTypeFare.retrievalInfo()->keep())))
  {
    LOG4CXX_INFO(_logger, " Bypass for Rex Pricing AdvanceResTkt::validate() - SOFTPASS");
    return SOFTPASS;
  }

  bool mayPassAfterRebook = false;

  const AdvResTktInfo& advanceResTktInfo = *advResTktInfo;

  if (UNLIKELY(diag.isActive()))
  {
    if (trx.diagnostic().filterRulePhase(RuleItem::FCOPhase) &&
        trx.diagnostic().filterRulePhase(RuleItem::FarePhase))
      diag.deActivate();
  }

  if (diag.isActive())
  {
    displayRuleDataToDiag(advanceResTktInfo, diag.collector());
  }

  // Validate for the ND - qualifier in the Pricing Entry ( Nulify Date )
  if (UNLIKELY(trx.getRequest()->isPriceNullDate()))
  {
    diag << " ADVANCE RESTKT: PASS - ND QUALIFIER IS USED\n";

    LOG4CXX_INFO(_logger, " Nulify Date qualifier - skip Advance Validation");

    return RexPricingTrx::isRexTrxAndNewItin(trx) ? SOFTPASS : PASS;
  }

  bool allowRebook = false; // Flag set at processing WPNC(S)
  uint16_t currentNumOfRbkSeg = 0;
  std::map<const TravelSeg*, bool> segsRebookStatus;

  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  Record3ReturnTypes r3rtn = validateUnavailableDataTag(advanceResTktInfo.unavailtag());
  if (UNLIKELY(r3rtn == FAIL))
  {
    diag << " ADVANCE RESTKT: FAIL - DATA UNAVAIBLE\n";
  }
  else if (r3rtn == SKIP)
  {
    diag << " ADVANCE RESTKT: SKIP - TXT DATA ONLY\n";
  }
  else
  { // r3rtn == PASS
    if (trx.getRequest()->isLowFareNoAvailability() || // WPNCS
        trx.getRequest()->isLowFareRequested()) // WPNC
    {
      diag << ".ALLOW REBOOKING..\n";
      allowRebook = true;
      getCurrentRebookStatus(paxTypeFare, nullptr, segsRebookStatus);
      currentNumOfRbkSeg = segsRebookStatus.size();
    }

    if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(trx)))
    {
      const RexPricingTrx& rexTrx = static_cast<const RexPricingTrx&>(trx);
      _newTicketIssueDate = rexTrx.departureDateValidator().getDepartureDate(fareMarket, itin);
    }

    r3rtn = identifyAndValidateTvlSeg(trx,
                                      itin,
                                      advanceResTktInfo,
                                      (const FarePath*)nullptr,
                                      (PricingUnit*)nullptr,
                                      &fareMarket,
                                      paxTypeFare,
                                      allowRebook ? &segsRebookStatus : nullptr,
                                      diag,
                                      mayPassAfterRebook);
  }

  if (UNLIKELY(isValidationNeeded(RuleConst::ADVANCE_RESERVATION_RULE, trx)))
  {
    updateStatus(RuleConst::ADVANCE_RESERVATION_RULE, r3rtn);
  }

  if (r3rtn == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - FAIL");
    if (mayPassAfterRebook)
      const_cast<PaxTypeFare&>(paxTypeFare).reProcessCat05NoMatch() = true;
  }
  else if (r3rtn == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - SKIP");
  }
  else if (r3rtn == PASS)
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - PASS");
  }
  else
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate()");

  // change paxTypeFare.segmentStatus()
  if (UNLIKELY((allowRebook == true) && ((r3rtn == PASS) || (r3rtn == SOFTPASS) || (r3rtn == SKIP)) &&
      (currentNumOfRbkSeg != segsRebookStatus.size())))
  {
    // have something need to be rebooked
    Record3ReturnTypes rtn =
        updateRebookStatus(trx, &itin, const_cast<PaxTypeFare&>(paxTypeFare), nullptr, segsRebookStatus);

    if (rtn == FAIL)
    {
      diag << ".FAIL CHECKAVAIL.. \n";
      r3rtn = FAIL;
    }
    else if (rtn == SKIP)
    {
      diag << ".CAN NOT SET REBOOK FLAG AT THIS STEP.. \n";
      r3rtn = SOFTPASS;
    }
  }

  if (_matchedTktExptTime)
  {
    if (UNLIKELY(r3rtn == FAIL && advanceResTktInfo.eachSector() != notApply))
      return STOP; // FM scope, sure fail
    else if (LIKELY(r3rtn == SOFTPASS))
      return STOP_SOFT;
  }

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(trx) && r3rtn == PASS))
    return SOFTPASS;

  return r3rtn;
}

Record3ReturnTypes
AdvanceResTkt::validate(PricingTrx& trx,
                        const RuleItemInfo* rule,
                        const FarePath& farePath,
                        const PricingUnit& pricingUnit,
                        const FareUsage& fareUsage)
{
  LOG4CXX_INFO(_logger, " Entered AdvanceResTkt::validate() for Pricing Unit");
  DataHandle& dh = trx.dataHandle();
  _dataHandle = &dh;
  _farePath = &farePath;

  const AdvResTktInfo* advResTktInfo = dynamic_cast<const AdvResTktInfo*>(rule);

  if (UNLIKELY(advResTktInfo == nullptr))
  {
    LOG4CXX_ERROR(_logger, "Not valid AdvResTktInfo");
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - FAIL");
    return FAIL;
  }

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(trx);
    if (rexTrx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE &&
        !farePath.forceCat5Validation() && !rexTrx.departureDateValidator().isValid())
    {
      LOG4CXX_INFO(_logger, " Bypass for Rex Pricing AdvanceResTkt::validate() - PASS");
      return PASS;
    }
  }

  bool mayPassAfterRebook = false;
  const AdvResTktInfo& advanceResTktInfo = *advResTktInfo;

  if (advanceResTktInfo.ticketed() != notApply)
    (const_cast<FareUsage&>(fareUsage)).simultaneousResTkt() = true;

  DiagManager diag(trx, Diagnostic305);
  if (UNLIKELY(diag.isActive()))
  {
    if ((_useFakeFP && trx.diagnostic().filterRulePhase(RuleItem::PricingUnitFactoryPhase)) ||
        (!_useFakeFP && trx.diagnostic().filterRulePhase(RuleItem::FarePathFactoryPhase)))
      diag.deActivate();

    if (diag.isActive() && !_useFakeFP && RexPricingTrx::isRexTrxAndNewItin(trx) &&
        pricingUnit.volChangesAdvResOverride() &&
        pricingUnit.volChangesAdvResOverride()->diag305OFF())
      diag.deActivate();
  }

  if (diag.isActive())
  {
    const FareMarket& fareMarket = *fareUsage.paxTypeFare()->fareMarket(); // lint !e530
    diag << "FARE USAGE: " << fareMarket.origin()->loc() << "-" << fareMarket.destination()->loc()
         << "  ";
    if (pricingUnit.dynamicValidationPhase())
      diag << "DYNAMIC  ";

    if (pricingUnit.travelSeg().empty())
    {
      LOG4CXX_ERROR(_logger, "AdvanceResTkt::validate(...) - Empty travel segment vector");
      diag << "PU: "
           << "Empty travel segment vector";
    }
    else
    {
      diag << "PU: " << pricingUnit.travelSeg().front()->origAirport();
    }

    if (pricingUnit.turnAroundPoint())
    {
      diag << "-" << pricingUnit.turnAroundPoint()->origAirport();
    }
    diag << "-" << pricingUnit.travelSeg().back()->destAirport() << '\n';

    displayRuleDataToDiag(advanceResTktInfo, diag.collector());
  }

  // Validate for the ND - qualifier in the Pricing Entry ( Nulify Date )
  if (UNLIKELY(trx.getRequest()->isPriceNullDate()))
  {
    diag << " ADVANCE RESTKT: PASS - ND QUALIFIER IS USED\n";

    LOG4CXX_INFO(_logger, " Nulify Date qualifier - skip Advance Validation");
    return PASS;
  }

  bool allowRebook = false; // Flag set at processing WPNC(S)
  uint16_t currentNumOfRbkSeg = 0;
  std::map<const TravelSeg*, bool> segsRebookStatus;

  // Check if this rule item is eligible for validation with function of
  // super class (RuleApplicationBase)
  Record3ReturnTypes r3rtn = validateUnavailableDataTag(advanceResTktInfo.unavailtag());
  if (UNLIKELY(r3rtn == FAIL))
  {
    diag << " ADVANCE RESTKT: FAIL \n";
  }
  else if (r3rtn == SKIP)
  {
    diag << " ADVANCE RESTKT: SKIP \n";
  }
  else
  { // r3rtn == PASS
    if (trx.getRequest()->isLowFareNoAvailability() || // WPNCS
        (trx.getRequest()->isLowFareRequested() && // WPNC
         !pricingUnit.dynamicValidationPhase()))
    {
      diag << ".ALLOW REBOOKING..\n";
      allowRebook = true;
      // gathering all segment booking status we can have
      // For now, the whole PU
      getCurrentRebookStatus(farePath, segsRebookStatus);
      currentNumOfRbkSeg = segsRebookStatus.size();
    }

    if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(trx)))
    {
      if(!pricingUnit.volChangesAdvResOverride())
      {
        //during fallback removing consider remove departureDateValidator
        const RexPricingTrx& rexTrx = static_cast<const RexPricingTrx&>(trx);
        _newTicketIssueDate =
            rexTrx.departureDateValidator().getDepartureDate(pricingUnit, *farePath.itin());
      }
      else
      {
        _newTicketIssueDate = pricingUnit.volChangesAdvResOverride()->fromDate();
      }
    }

    r3rtn = identifyAndValidateTvlSeg(trx,
                                      *farePath.itin(),
                                      advanceResTktInfo,
                                      &farePath,
                                      &pricingUnit,
                                      nullptr,
                                      *fareUsage.paxTypeFare(),
                                      allowRebook ? &segsRebookStatus : nullptr,
                                      diag,
                                      mayPassAfterRebook);
  }

  if (r3rtn == FAIL)
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - FAIL");
  }
  else if (r3rtn == SKIP)
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - SKIP");
  }
  else if (LIKELY(r3rtn == PASS))
  {
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate() - PASS");
  }
  else
    LOG4CXX_INFO(_logger, " Leaving AdvanceResTkt::validate()");

  // change paxTypeFare.segmentStatus()
  if (UNLIKELY((allowRebook == true) && ((r3rtn == PASS) || (r3rtn == SOFTPASS) || (r3rtn == SKIP)) &&
      (currentNumOfRbkSeg != segsRebookStatus.size())))
  {
    // have something need to be rebooked
    if (!updateRebookStatus(trx, farePath, const_cast<FareUsage&>(fareUsage), segsRebookStatus))
    {
      diag << ".FAIL CHECKAVAIL.. \n";
      r3rtn = FAIL;
    }
    else
    {
      diag << ".PASS CHECKAVAIL FOR REBOOKED SEGS \n";
    }
  }

  if (UNLIKELY(r3rtn == FAIL && _matchedTktExptTime))
    return STOP;

  return r3rtn;
}

bool
AdvanceResTkt::nonFlexFareValidationNeeded(PricingTrx& trx) const
{
  return (!trx.isFlexFare() && trx.getOptions()->isNoAdvPurchRestr());
}

namespace
{
void
pushToApplTvlSegment(PricingTrx& trx,
                     TravelSeg* tvlSeg,
                     RuleUtil::TravelSegWrapperVector& applTravelSegment)
{
  if (tvlSeg->isAir())
  {
    RuleUtil::TravelSegWrapper* tsw = &trx.dataHandle().safe_create<RuleUtil::TravelSegWrapper>();
    tsw->travelSeg() = tvlSeg;
    tsw->origMatch() = true;
    tsw->destMatch() = false;

    applTravelSegment.push_back(tsw);
  }
}
}

Record3ReturnTypes
AdvanceResTkt::identifyAndValidateTvlSeg(PricingTrx& trx,
                                         const Itin& itin,
                                         const AdvResTktInfo& advanceResTktInfo,
                                         const FarePath* farePath,
                                         const PricingUnit* pricingUnit,
                                         const FareMarket* fareMarket,
                                         const PaxTypeFare& paxTypeFare,
                                         std::map<const TravelSeg*, bool>* segsRbkStat,
                                         DiagManager& diag,
                                         bool& mayPassAfterRebook)
{
  const VendorCode& vendor = paxTypeFare.vendor();
  // bool displayWqWarning = false;

  //--------------------------------------------------------------
  // Check if it is a request with no Advance Purchase Restriction
  //--------------------------------------------------------------
  const bool nonFlexFareValidation = nonFlexFareValidationNeeded(trx);
  if (nonFlexFareValidation || isValidationNeeded(RuleConst::ADVANCE_RESERVATION_RULE, trx))
  {
    if (existResTktRestr(advanceResTktInfo))
    {
      diag << " ADVANCE RESTKT:FAIL - RESTRICTION APPLY\n";
      updateStatus(RuleConst::ADVANCE_RESERVATION_RULE, FAIL);
      if (nonFlexFareValidation || shouldReturn(RuleConst::ADVANCE_RESERVATION_RULE))
        return FAIL;
    }
  }

  //-----------------------------------------------------------
  // fareMarket must not be 0 for fare market validation
  // farePath and pricingUnit must not be 0 for pricing unit validation
  //-----------------------------------------------------------
  bool forFareMarket = (fareMarket != nullptr);

  if (!forFareMarket)
  {
    if (UNLIKELY(pricingUnit == nullptr))
      return FAIL;

    if (UNLIKELY(farePath == nullptr))
    {
      // @TODO if there is TSI or Geo under JOURNEY_SCOPE
      // we need softpass
    }
  }

  //-----------------------------------------------------------
  // Ticketing Date is commonly used, get it here.
  //-----------------------------------------------------------
  DateTime tktDT = TrxUtil::getTicketingDTCat5(trx);

  //-----------------------------------------------------------
  // Gather the travel segments that we need to validate
  // GeoTblItemNo will be used to identify segments when value is not 0;
  //-----------------------------------------------------------
  RuleUtil::TravelSegWrapperVector applTravelSegment;

  if (advanceResTktInfo.geoTblItemNo() != 0)
  {
    bool confOrigCheck = true;
    bool confDestCheck = false;

    bool fltStopCheck = false;
    TSICode tsiReturn;
    LocKey locKey1Return;
    LocKey locKey2Return;
    RuleConst::TSIScopeParamType defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;

    if (advanceResTktInfo.eachSector() != notApply)
    {
      defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT; // Against F.C.
    }

    const FareMarket* fmForGeo = fareMarket;
    if (!forFareMarket)
    {
      fmForGeo = paxTypeFare.fareMarket();
    }

    if (!RuleUtil::validateGeoRuleItem(advanceResTktInfo.geoTblItemNo(),
                                       vendor,
                                       defaultScopeParam,
                                       false,
                                       false,
                                       true,
                                       trx,
                                       _useFakeFP ? nullptr : farePath,
                                       &itin,
                                       pricingUnit,
                                       fmForGeo,
                                       tktDT,
                                       applTravelSegment, // this will contain the results
                                       confOrigCheck,
                                       confDestCheck,
                                       fltStopCheck,
                                       tsiReturn,
                                       locKey1Return,
                                       locKey2Return,
                                       Diagnostic305,
                                       RuleUtil::LOGIC_AND))
    {
      Record3ReturnTypes ret = FAIL;

      if ((pricingUnit == nullptr) && (defaultScopeParam != RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT)
          // not default against F.C
          )
      {
        // can be that it is for PU or FP scope, that we do not have yet
        // we need to return SOFTPASS in that case
        RuleConst::TSIScopeType geoScope;

        if (RuleUtil::getTSIScope(tsiReturn, trx, geoScope))
        {
          if (LIKELY((pricingUnit == nullptr) && (geoScope == RuleConst::TSI_SCOPE_SUB_JOURNEY ||
                                     geoScope == RuleConst::TSI_SCOPE_SJ_AND_FC ||
                                     geoScope == RuleConst::TSI_SCOPE_JOURNEY)))
          {
            ret = SOFTPASS;
          }
        }
      }

      diag << " ADVANCE RESTKT: - FALSE FROM CONF GEO RULE ITEM\n";
      if (ret == FAIL)
      {
        diag << " ADVANCE RESTKT: FAIL\n";
      }
      else // SOFTPASS
      {
        diag << " ADVANCE RESTKT: SOFTPASS - REVALID PENDING\n";
      }
      return ret;
    }
  } // geoTblItemNo != 0
  else
  {
    //----------------------------------------------------------------------
    // we need to identify the travel segments
    // by fareMarket.travelSeg or pricingUnit.traveSeg
    //----------------------------------------------------------------------
    //--------------------------------------------------------------
    // For not outbound fareMarket we do not need to check
    // if only sectors prior to PU turnaround point is required.
    //--------------------------------------------------------------
    if (advanceResTktInfo.eachSector() == notApply)
    {
      // PU scope, when PU is not ready, we still try to fail
      // what we can, do not want to fail what we are not sure.
      // Inbound and Unknown faremarket does not need to be
      // verified if we have  requirement that only segments
      // prior to PU turnaround point need to be checked
      if ((advanceResTktInfo.confirmedSector() != returnSectorNoOpen) &&
          (advanceResTktInfo.confirmedSector() != allSector))
      {
        if (forFareMarket)
        {
          if (fareMarket->direction() != FMDirection::OUTBOUND)
          {
            diag << " ADVANCE RESTKT: SOFTPASS - ONLY CHECK OUTBOUND\n";
            return SOFTPASS;
          }
          if (advanceResTktInfo.confirmedSector() == firstSector &&
              fareMarket->travelSeg().front() != itin.travelSeg().front())
          {
            diag << " ADVANCE RESTKT: SOFTPASS - UNSURE FIRST SECTOR IN PU\n";
            return SOFTPASS;
          }
        }
      }
    }

    if (forFareMarket)
    {
      if (LIKELY(!fareMarket->travelSeg().empty()))
      {
        for (TravelSeg* tvlSeg : fareMarket->travelSeg())
          pushToApplTvlSegment(trx, tvlSeg, applTravelSegment);
      }
      else
      {
        diag << " ADVANCE RESTKT: FAIL "
             << "- FARE MARKET TRAVEL SEG EMPTY\n";

        return FAIL;
      }
    }
    else
    {
      if (advanceResTktInfo.eachSector() != notApply)
      {
        // Fare Component Scope
        const FareMarket* fm = paxTypeFare.fareMarket();
        for (TravelSeg* tvlSeg : fm->travelSeg())
          pushToApplTvlSegment(trx, tvlSeg, applTravelSegment);
      }
      else if (UNLIKELY(pricingUnit->travelSeg().empty()))
      {
        diag << " ADVANCE RESTKT: FAIL "
             << "- PRICING UNIT TRAVEL SEG EMPTY\n";

        return FAIL;
      }
      else
      {
        for (TravelSeg* tvlSeg : pricingUnit->travelSeg())
          pushToApplTvlSegment(trx, tvlSeg, applTravelSegment);
      }
    }
  }

  // based on rule data and FC/PU scope we are in, decide what we need
  // to validate
  bool isSoftPass = false; // set true later when validation need to be done
  // in PU, softpass in FC
  const TSICode resTSI = advanceResTktInfo.resTSI();
  const TSICode tktTSI = advanceResTktInfo.tktTSI();

  bool needResDTChk = false;

  if (advanceResTktInfo.permitted() == notPermitted)
  {
    needResDTChk = true; // res must be at time of departure of FC orig
  }
  else if ((advanceResTktInfo.firstResUnit().length() != 0) ||
           (advanceResTktInfo.lastResUnit().length() != 0))
  {
    // should be meatured from departure of PU or TSI specified
    if (forFareMarket && (resTSI == 0))
    {
      isSoftPass = true;
      if (fareMarket->travelSeg().front() == itin.travelSeg().front())
      {
        // departure of PU will be departure of FM, so we can have
        // hard failure
        needResDTChk = true;
        if (advanceResTktInfo.eachSector() != notApply)
        {
          // check is only on this fare component, pass will be
          // a hard pass
          isSoftPass = false;
        }
      }
      else
      {
        // Date before departure of PU should be earlier or same
        // as departure of FC, so for last reservation date before
        // departure restriction, we can have hard fail and soft pass.
        if ((advanceResTktInfo.firstResUnit().length() != 0))
          needResDTChk = false;
        else
          needResDTChk = true;
      }
    }
    else
    {
      needResDTChk = true;
    }
  }

  bool needTktAgainstResCheck = false;

  if ((advanceResTktInfo.ticketed() != notApply) ||
      (getAdvTktUnit(advanceResTktInfo).length() != 0))
  {
    needTktAgainstResCheck = true;
  }

  bool needTktAgainstDepartCheck = false;
  if (getAdvTktOpt(advanceResTktInfo) != notApply)
  {
    if (forFareMarket && (tktTSI == 0))
    {
      isSoftPass = true;
      if (fareMarket->travelSeg().front() == itin.travelSeg().front())
      {
        // departure of PU will be departure of FM, so we can have
        // hard failure
        needTktAgainstDepartCheck = true;
        if (advanceResTktInfo.eachSector() != notApply)
        {
          // check is only on this fare component, pass will be
          // a hard pass
          isSoftPass = false;
        }
      }
      else
      {
        needTktAgainstDepartCheck = false;
      }
    }
    else
    {
      needTktAgainstDepartCheck = true;
    }
  }
  bool needTktDTChk = needTktAgainstResCheck || needTktAgainstDepartCheck;

  if (UNLIKELY(RexPricingTrx::isRexTrxAndNewItin(trx) && forFareMarket))
  {
    needResDTChk = false;
    needTktDTChk = true;
  }

  //-----------------------------------------------------------
  // Determine what Ticketing Date will be used
  // Not Exchange Trx, Ticketing Date normally is TrxUtil::getTicketingDTCat5(trx);
  // For Exchange Trx, Ticketing Date(s) is determined on conditions.
  //-----------------------------------------------------------
  DateTime tktDate[MAX_NUM_ELIGIBLE_TKTDT]; // 2
  int numOfTryTktDT = 1; // default
  if (needTktDTChk)
  {
    numOfTryTktDT = getEligibleTktDates(trx, *paxTypeFare.fareMarket(), pricingUnit, tktDate);
  }
  else
  {
    if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX || trx.excTrxType() == PricingTrx::AR_EXC_TRX))
      tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
  }
  bool needConfStatCheck = true;

  //-----------------------------------------------------------
  // Then we get date that reservation will validate against with.
  // If resTSI exists, identify the travel segment by TSI
  // and go ahead validating reservation
  // Else, we use the default - first travel segment
  // If ticketing and reservation use same TSI, validate ticketing
  // with identified segments as well
  //-----------------------------------------------------------
  const FareMarket* fm = forFareMarket ? fareMarket : paxTypeFare.fareMarket();
  const bool optionB = trx.getOptions()->AdvancePurchaseOption() == 'T';
  const bool tvlSegChange = checkTravelSegChange(*fm);
  const bool isoptionBanyChange = (trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
                                   trx.excTrxType() == PricingTrx::AR_EXC_TRX) &&
                                  optionB && tvlSegChange;
  bool doResTktValidSameTvlSegs = (tktTSI == resTSI && !isoptionBanyChange) ? true : false;

  if (needResDTChk && (resTSI != 0))
  {
    needResDTChk = false; // after this, no duplicated check

    // see if we will validate Tkt Reqs the same time
    bool checkTktDTHere = false;

    if (doResTktValidSameTvlSegs)
    {
      checkTktDTHere = true;
      needTktDTChk = false; // after this, no duplicated check
    };

    RuleUtil::TravelSegWrapperVector resApplTravelSegment;

    LocKey loc1;
    LocKey loc2;
    RuleConst::TSIScopeParamType defaultScopeParam =
        RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY; // Use default scope param - sub jouney

    if (advanceResTktInfo.eachSector() != notApply)
    {
      defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT; // Against F.C.
    }

    const FareMarket* fmForTSI = fareMarket;
    if (!forFareMarket)
    {
      fmForTSI = paxTypeFare.fareMarket();
    }

    /// Pricing Unit or  Fare Component (FareMarket) that a TSI applies to.
    if (!RuleUtil::scopeTSIGeo(resTSI,
                               loc1,
                               loc2,
                               defaultScopeParam,
                               false,
                               false,
                               true,
                               trx,
                               _useFakeFP ? nullptr : farePath,
                               &itin,
                               pricingUnit,
                               fmForTSI,
                               tktDT,
                               resApplTravelSegment, // this will contain the results
                               Diagnostic305,
                               vendor))
    {
      diag << " ADVANCE RESTKT- FALSE FROM SCOPE TSI GEO\n";
      if (defaultScopeParam == RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT)
      {
        diag << " ADVANCE RESTKT:FAIL - NOT ABLE TO GET FARECOMPS BY RES TSI\n";
        return FAIL;
      }
      else if (!forFareMarket)
      {
        diag << " ADVANCE RESTKT:FAIL - NOT ABLE TO GET TVLSEG BY RES TSI\n";
        return FAIL;
      }

      // forFareMarket but we probably need PU or FP, skip
      diag << " ADVANCE RES:SOFTPASS - NOT FC SCOPE RESTSI  REVALID LATER\n";
      isSoftPass = true;
    }
    else if (UNLIKELY(resApplTravelSegment.empty()))
    {
      diag << " ADVANCE RESTKT: FAIL  - NO MATCH FROM RES TSI\n";
      return FAIL;
    }
    else
    {
      const RuleUtil::TravelSegWrapper& refTvlSegW = *resApplTravelSegment.front();

      bool displayWqWarning1 = false;
      bool displayWqWarning2 = false;

      Record3ReturnTypes result = FAIL;
      for (int tktDTOrder = 0; (tktDTOrder < numOfTryTktDT && FAIL == result); tktDTOrder++)
      {
        bool failedOnCfmStat = false;

        result = validateTvlSegs(trx,
                                 itin,
                                 pricingUnit,
                                 applTravelSegment,
                                 advanceResTktInfo,
                                 refTvlSegW,
                                 true, // needResDTChk
                                 checkTktDTHere,
                                 false, // needConfStatChk
                                 segsRbkStat,
                                 diag,
                                 mayPassAfterRebook,
                                 displayWqWarning1,
                                 displayWqWarning2,
                                 tktDate[tktDTOrder],
                                 false,
                                 failedOnCfmStat,
                                 &paxTypeFare);
      }

      if (displayWqWarning1)
      {
        paxTypeFare.warningMap().set(WarningMap::cat5_warning_1, true);
      }
      if (UNLIKELY(displayWqWarning2))
      {
        paxTypeFare.warningMap().set(WarningMap::cat5_warning_2, true);
      }

      if (result == FAIL)
        return FAIL;
      else if (result == SKIP)
      {
        if (isSoftPass)
        {
          diag << "\n ADVANCE RESTKT: SOFTPASS \n";
          return SOFTPASS;
        }
        else
          return SKIP;
      }

      if (UNLIKELY(resCanNotBeMadeNow(refTvlSegW, advanceResTktInfo)))
      {
        needConfStatCheck = false;
      }
    }
  } // resTSI != 0

  //-----------------------------------------------------------
  // Next:  if tktTSI exists and diff from resTSI, identify segments
  //  by tktTSI, and do the validation if succeeded
  //-----------------------------------------------------------
  if (tktTSI != 0 && !doResTktValidSameTvlSegs)
  {
    needTktDTChk = false;
    RuleUtil::TravelSegWrapperVector tktApplTravelSegment;

    LocKey loc1;
    LocKey loc2;
    RuleConst::TSIScopeParamType defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY;

    if (UNLIKELY(isoptionBanyChange))
    {
      defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT; // Against F.C.
    }
    else if (advanceResTktInfo.eachSector() != notApply)
    {
      defaultScopeParam = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT; // Against F.C.
    }

    const FareMarket* fmForTSI = fareMarket;
    if (!forFareMarket)
    {
      fmForTSI = paxTypeFare.fareMarket();
    }

    if (!RuleUtil::scopeTSIGeo(tktTSI,
                               loc1,
                               loc2,
                               defaultScopeParam,
                               false,
                               false,
                               true,
                               trx,
                               _useFakeFP ? nullptr : farePath,
                               &itin,
                               pricingUnit,
                               fmForTSI,
                               tktDT,
                               tktApplTravelSegment, // this will contain the results
                               Diagnostic305,
                               vendor))
    {
      diag << " ADVANCE RESTKT - FALSE FROM SCOPE TKT TSI\n";
      if (defaultScopeParam == RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT)
      {
        if (!isoptionBanyChange)
        {
          diag << " ADVANCE RESTKT:FAIL - NOT ABLE TO GET FARECOMPS BY TKT TSI\n";
          return FAIL;
        }
      }
      else if (!forFareMarket)
      {
        diag << " ADVANCE RESTKT:FAIL - NOT ABLE TO GET TVLSEG BY TKT TSI\n";
        return FAIL;
      }

      // forFareMarket but we probably need PU or FP
      diag << " ADVANCE TKT:SOFTPASS - NOT FC SCOPE TKTTSI  REVALID LATER\n";
      isSoftPass = true;
    }
    else if (tktApplTravelSegment.empty())
    {
      if (!isoptionBanyChange)
      {
        diag << " ADVANCE RESTKT: FAIL  - NO MATCH FROM TKT TSI\n";
        return FAIL;
      }

      if (!fmForTSI->travelSeg().empty())
      {
        for (TravelSeg* tvlSeg : fareMarket->travelSeg())
        {
          if (tvlSeg->isAir())
          {
            pushToApplTvlSegment(trx, tvlSeg, tktApplTravelSegment);
            break;
          }
        }
      }
    }
    else
    {
      const RuleUtil::TravelSegWrapper& refTvlSegW = *tktApplTravelSegment.front();

      bool displayWqWarning1 = false;
      bool displayWqWarning2 = false;

      Record3ReturnTypes result = FAIL;
      for (int tktDTOrder = 0; (tktDTOrder < numOfTryTktDT && FAIL == result); tktDTOrder++)
      {
        bool failedOnCfmStat = false;

        result = validateTvlSegs(trx,
                                 itin,
                                 pricingUnit,
                                 applTravelSegment,
                                 advanceResTktInfo,
                                 refTvlSegW,
                                 false, // needResDTCheck
                                 true, // needTktDTCheck,
                                 false, // needConfStatCheck,
                                 segsRbkStat,
                                 diag,
                                 mayPassAfterRebook,
                                 displayWqWarning1,
                                 displayWqWarning2,
                                 tktDate[tktDTOrder],
                                 false,
                                 failedOnCfmStat,
                                 &paxTypeFare);
      }

      if (LIKELY(displayWqWarning1))
      {
        paxTypeFare.warningMap().set(WarningMap::cat5_warning_1, true);
      }
      if (UNLIKELY(displayWqWarning2))
      {
        paxTypeFare.warningMap().set(WarningMap::cat5_warning_2, true);
      }

      if (result == FAIL)
        return FAIL;
      else if (result == SKIP)
      {
        if (isSoftPass)
        {
          diag << "\n ADVANCE RESTKT: SOFTPASS \n";
          return SOFTPASS;
        }
        else
          return SKIP;
      }
    }
  }

  if (UNLIKELY(applTravelSegment.empty()))
  {
    LOG4CXX_ERROR(_logger,
                  "AdvanceResTkt::identifyAndValidateTvlSeg(...) - Empty travel segment vector");
    diag << " ADVANCE RESTKT: FAIL  - NO MATCH FROM GEO\n";
    return FAIL;
  }

  // the first segment of PU or FC be the default departure reference
  RuleUtil::TravelSegWrapper defaultRefTvlSegW;
  if (advanceResTktInfo.eachSector() != notApply || pricingUnit == nullptr || isoptionBanyChange)
    defaultRefTvlSegW.travelSeg() = paxTypeFare.fareMarket()->travelSeg().front();
  else
    defaultRefTvlSegW.travelSeg() = pricingUnit->travelSeg().front();

  defaultRefTvlSegW.origMatch() = true;
  defaultRefTvlSegW.destMatch() = false;

  // Skip the confirmed status check for WPNCS;
  // Skip the confirmed status check for WPNC on fare market validation, skip
  // confirmed status check for segments we are able to the rebook
  // during PU/FP validation.
  // if we can not make reservation at current time, we skip the confirmed
  // status check;
  if (trx.getRequest()->isLowFareNoAvailability() // WPNCS
      ||
      ((advanceResTktInfo.resTSI() == 0) &&
       resCanNotBeMadeNow(defaultRefTvlSegW, advanceResTktInfo)))
  {
    needConfStatCheck = false;
  }
  else if (forFareMarket && trx.getRequest()->isLowFareRequested()) // WPNC
  {
    needConfStatCheck = false;
    isSoftPass = true;
  }

  if (needResDTChk || needTktDTChk || needConfStatCheck)
  {
    Record3ReturnTypes result = FAIL;

    bool displayWqWarning1 = false;
    bool displayWqWarning2 = false;

    for (int tktDTOrder = 0; (tktDTOrder < numOfTryTktDT && FAIL == result); tktDTOrder++)
    {
      bool failedOnCfmStat = false;

      result = validateTvlSegs(trx,
                               itin,
                               pricingUnit,
                               applTravelSegment,
                               advanceResTktInfo,
                               defaultRefTvlSegW,
                               needResDTChk,
                               needTktDTChk,
                               needConfStatCheck,
                               segsRbkStat,
                               diag,
                               mayPassAfterRebook,
                               displayWqWarning1,
                               displayWqWarning2,
                               tktDate[tktDTOrder],
                               false,
                               failedOnCfmStat,
                               &paxTypeFare);

      if (UNLIKELY(failedOnCfmStat))
        break; // new ticketing date would not help
    }

    if (displayWqWarning1)
    {
      paxTypeFare.warningMap().set(WarningMap::cat5_warning_1, true);
    }
    if (UNLIKELY(displayWqWarning2))
    {
      paxTypeFare.warningMap().set(WarningMap::cat5_warning_2, true);
    }

    if (result == FAIL)
      return FAIL;
    else if (result == SKIP)
    {
      if (isSoftPass)
      {
        diag << "\n ADVANCE RESTKT: SOFTPASS \n";
        return SOFTPASS;
      }
      else
        return SKIP;
    }
  }

  if (isSoftPass)
  {
    diag << "\n ADVANCE RESTKT: SOFTPASS \n";
    return SOFTPASS;
  }

  if (forFareMarket)
  {
    if (!needPUValidation(itin, *fareMarket, advanceResTktInfo))
    {
      if (_cat05overrideDisplayFlag)
      {
        diag << "\n ADVANCE RESTKT: PASS - ";
        displayCat05OverrideDataToDiag(diag);
      }
      else
        diag << "\n ADVANCE RESTKT: PASS \n";
      return PASS;
    }
    else
    {
      diag << "\n ADVANCE RESTKT: SOFTPASS \n";
      return SOFTPASS;
    }
  }
  else
  {
    if (UNLIKELY(_cat05overrideDisplayFlag))
    {
      diag << "\n ADVANCE RESTKT: PASS - ";
      displayCat05OverrideDataToDiag(diag);
    }
    else
      diag << "\n ADVANCE RESTKT: PASS \n";
    return PASS;
  }
}

bool
AdvanceResTkt::needPUValidation(const Itin& itin,
                                const FareMarket& fareMarket,
                                const AdvResTktInfo& advanceResTktInfo) const
{
  // Cat5 Reservation Restriction and Ticketing Restriction is by default
  // on each components of PU.
  //
  // We should only be able to HARDPASS if the fare market and Itinary
  // contain same segments (so PU must contain same segments as well),
  // and there is no Ticket Date restriction so that we do not have to do
  // PU revalidation to get PU->latestTktDT.

  return !(((itin.travelSeg().size() == fareMarket.travelSeg().size()) ||
            (advanceResTktInfo.eachSector() != notApply)) &&
           advanceResTktInfo.ticketed() == notApply &&
           advanceResTktInfo.advTktExcpUnit() == notApply &&
           getAdvTktUnit(advanceResTktInfo).empty() && getAdvTktOpt(advanceResTktInfo) == notApply);
}

inline static const TravelSeg*
getTurnaroundPoint(const PricingTrx& trx, const Itin& itin, const PricingUnit* pu)
{
  if (UNLIKELY(RtwUtil::isRtw(trx)))
  {
    const int16_t tsAfterFurthest = itin.furthestPointSegmentOrder();
    if (tsAfterFurthest > 0 && tsAfterFurthest < int16_t(itin.travelSeg().size()))
      return itin.travelSeg()[tsAfterFurthest];
  }
  else if (pu)
    return pu->turnAroundPoint();
  return nullptr;
}

Record3ReturnTypes
AdvanceResTkt::validateTvlSegs(PricingTrx& trx,
                               const Itin& itin,
                               const PricingUnit* pricingUnit,
                               const RuleUtil::TravelSegWrapperVector& originalApplTravelSegment,
                               const AdvResTktInfo& advanceResTktInfo,
                               const RuleUtil::TravelSegWrapper& refOrigTvlSegW,
                               const bool& needResDTChk,
                               const bool& needTktDTChk,
                               const bool& needConfStatChk,
                               std::map<const TravelSeg*, bool>* segsRbkStat,
                               DiagManager& diag,
                               bool& mayPassAfterRebook,
                               bool& displWqWarning1,
                               bool& displWqWarning2,
                               const DateTime& tktDT,
                               bool skipResPermittedChk,
                               bool& failedOnCfmStat,
                               const PaxTypeFare* paxTypeFare) const
{
  RuleUtil::TravelSegWrapperVector newApplTvlSegs;
  const RuleUtil::TravelSegWrapperVector& applTravelSegment =
      deleteTvlSegIgnoreCat5(trx, pricingUnit, originalApplTravelSegment, newApplTvlSegs)
          ? newApplTvlSegs
          : originalApplTravelSegment;

  bool needResDTCheck =
      (RexPricingTrx::isRexTrxAndNewItin(trx) && pricingUnit == nullptr) ? false : needResDTChk;

  bool needTktDTCheck = needTktDTChk;
  bool needConfStatCheck = needConfStatChk;
  const bool allowRebook = (segsRbkStat != nullptr);
  const PricingTrx::ExcTrxType& excTrxType = trx.excTrxType();
  bool isWQTrx = trx.noPNRPricing();

  failedOnCfmStat = false;

  if (UNLIKELY(applTravelSegment.empty()))
  {
    return PASS;
  }
  if (needTktDTChk || needResDTCheck)
    _utcOffset = checkUTC(trx, *refOrigTvlSegW.travelSeg());

  bool optionA = (trx.getOptions()->AdvancePurchaseOption() == ' ' ||
                  trx.getOptions()->AdvancePurchaseOption() == 'F');
  bool optionB = (trx.getOptions()->AdvancePurchaseOption() == 'T');
  DateTime adjustedDateResRestriction = DateTime::emptyDate();

  if (UNLIKELY(excTrxType == PricingTrx::AR_EXC_TRX))
  {
    RexPricingTrx* excTrx = static_cast<RexPricingTrx*>(&trx);

    if (excTrx != nullptr && !excTrx->exchangeItin().empty() &&
        excTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE &&
        (_farePath && _farePath->forceCat5Validation()))
    {
      adjustedDateResRestriction = excTrx->currentTicketingDT();
    }
  }
  else if (UNLIKELY(excTrxType == PricingTrx::PORT_EXC_TRX))
  {
    ExchangePricingTrx* excTrx = static_cast<ExchangePricingTrx*>(&trx);

    if (excTrx != nullptr && excTrx->reqType() != AGENT_PRICING_MASK && !excTrx->exchangeItin().empty() &&
        (optionA || optionB))
    {
      adjustedDateResRestriction = TrxUtil::getTicketingDTCat5(trx);
    }
  }

  const TravelSeg* turnAroundTvlSeg = getTurnaroundPoint(trx, itin, pricingUnit);
  const TravelSeg& refOrigTvlSeg = *refOrigTvlSegW.travelSeg();
  DateTime refOrigDT =
      refOrigTvlSegW.origMatch() ? refOrigTvlSeg.departureDT() : refOrigTvlSeg.arrivalDT();

  DateTime depRefOrigDT = refOrigDT; // msd
  if (_utcOffset)
  {
    if (advanceResTktInfo.advTktExcpUnit() == HOUR_UNIT_INDICATOR ||
        advanceResTktInfo.advTktExcpUnit() == MINUTE_UNIT_INDICATOR ||
        advanceResTktInfo.firstResUnit().c_str()[0] == HOUR_UNIT_INDICATOR ||
        advanceResTktInfo.firstResUnit().c_str()[0] == MINUTE_UNIT_INDICATOR ||
        advanceResTktInfo.lastResUnit().c_str()[0] == HOUR_UNIT_INDICATOR ||
        advanceResTktInfo.lastResUnit().c_str()[0] == MINUTE_UNIT_INDICATOR ||
        getAdvTktDepartUnit(advanceResTktInfo) == HOUR_UNIT_INDICATOR ||
        getAdvTktDepartUnit(advanceResTktInfo) == MINUTE_UNIT_INDICATOR)

      refOrigDT = refOrigDT.subtractSeconds(_utcOffset * 60);
  }
  DateTime refStartDT = refOrigDT;
  DateTime refEndDT = refOrigDT;

  if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
  {
    refStartDT = refOrigTvlSegW.origMatch() ? refOrigTvlSeg.earliestDepartureDT()
                                            : refOrigTvlSeg.earliestArrivalDT();
    refEndDT = refOrigTvlSegW.origMatch() ? refOrigTvlSeg.latestDepartureDT()
                                          : refOrigTvlSeg.latestArrivalDT();
  }

  // We must set todays date for all segments when itenerery
  // do not have any date.
  if (UNLIKELY(isWQTrx && itin.dateType() == Itin::NoDate))
  {
    const TravelSeg* pTvlSeg = *itin.travelSeg().begin();

    refOrigDT = pTvlSeg->departureDT();
    refStartDT = pTvlSeg->departureDT();
    refEndDT = pTvlSeg->departureDT();
  }

  if (UNLIKELY(diag.isActive() && refOrigTvlSegW.origMatch()))
  {
    diag << "  ORIG " << refOrigTvlSeg.origAirport()
         << " DEPART DATE: " << depRefOrigDT.toIsoExtendedString() << "\n";

    if (refOrigDT != depRefOrigDT)
      diag << "  ORIG " << refOrigTvlSeg.origAirport()
           << " DEPART DATE AT POS: " << refOrigDT.toIsoExtendedString() << "\n";
  }
  else if (diag.isActive() && refOrigTvlSegW.destMatch())
  {
    diag << "  DEST " << refOrigTvlSeg.destAirport()
         << "  ARRIVE DATE: " << depRefOrigDT.toIsoExtendedString() << "\n";

    diag << "  DEST " << refOrigTvlSeg.destAirport()
         << " ARRIVE DATE AT POS: " << depRefOrigDT.toIsoExtendedString() << "\n";
  }

  bool needResTktSameDay = false;
  bool needResTktSameTime = false;

  if (advanceResTktInfo.ticketed() != notApply)
  {
    if (itin.geoTravelType() == GeoTravelType::International || itin.geoTravelType() == GeoTravelType::ForeignDomestic)
    {
      needResTktSameDay = true;
    }
    else
    {
      needResTktSameTime = true;
    }
  }

  // loop through the travel segs and validate everything
  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI = applTravelSegment.begin();
  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentEndI = applTravelSegment.end();
  const RuleUtil::TravelSegWrapper* tsw = *applTravelSegmentI;

  DateTime bookingDT;
  DateTime ticketDT = TrxUtil::getTicketingDTCat5(trx);

  for (; applTravelSegmentI != applTravelSegmentEndI; applTravelSegmentI++)
  {
    tsw = *applTravelSegmentI;
    const TravelSeg* ts = tsw->travelSeg();

    if (ts->segmentType() == Arunk)
      continue;

    // Openseg + 1 logic, for open segment not followed by any
    // dated segment, we will only check confirmed status from now on
    if (UNLIKELY(ts->openSegAfterDatedSeg() && !isWQTrx))
    {
      needResDTCheck = false;
      needTktDTCheck = false;
    }

    if (!needConfStatCheck && !needResDTCheck && !needTktDTCheck)
    {
      break;
    }

    if (UNLIKELY(diag.isActive()))
    {
      diag << "TRAVEL SEGMENT\n";
      diag << "  ORIG: " << ts->origAirport() << " "
           << "  DEST: " << ts->destAirport() << "\n";
    }

    bool isTvlSegRebooked = false;
    getBookingDate(trx, *ts, bookingDT, isTvlSegRebooked, segsRbkStat);

    if (UNLIKELY(diag.isActive()))
    {
      diag << "  RESERVATION DATE: " << bookingDT.toIsoExtendedString() << "\n";
    }

    // if notPermitted and status not confirmed, we skip reservation permitted check
    bool checkConfStat =
        checkConfirmedStatusNotPermitted(*ts, advanceResTktInfo, ticketDT, refStartDT);
    bool skipResPermitChk = checkConfStat ? true : skipResPermittedChk;

    Record3ReturnTypes result = validateResTkt(bookingDT,
                                               tktDT,
                                               needResDTCheck,
                                               needTktDTCheck,
                                               advanceResTktInfo,
                                               needResTktSameDay,
                                               needResTktSameTime,
                                               refOrigDT,
                                               refStartDT,
                                               refEndDT,
                                               const_cast<PricingUnit*>(pricingUnit),
                                               diag,
                                               mayPassAfterRebook,
                                               skipResPermitChk,
                                               isWQTrx,
                                               displWqWarning1,
                                               adjustedDateResRestriction,
                                               trx);
    if (result == FAIL)
    {
      if (!allowRebook || isTvlSegRebooked)
      {
        return FAIL;
      }
      // allow rebook and has not tried yet, try rebook
      isTvlSegRebooked = true;

      getBookingDate(trx, *ts, bookingDT, isTvlSegRebooked, segsRbkStat);
      if (UNLIKELY(diag.isActive()))
      {
        diag << "  REBOOKING RESERVATION DATE: " << bookingDT.toIsoExtendedString() << "\n";
      }

      Record3ReturnTypes newResult = validateResTkt(bookingDT,
                                                    tktDT,
                                                    needResDTCheck,
                                                    needTktDTCheck,
                                                    advanceResTktInfo,
                                                    needResTktSameDay,
                                                    needResTktSameTime,
                                                    refOrigDT,
                                                    refStartDT,
                                                    refEndDT,
                                                    const_cast<PricingUnit*>(pricingUnit),
                                                    diag,
                                                    mayPassAfterRebook,
                                                    skipResPermitChk,
                                                    isWQTrx,
                                                    displWqWarning1,
                                                    adjustedDateResRestriction,
                                                    trx);
      if (newResult == FAIL)
      {
        return FAIL;
      }

      if (segsRbkStat)
      {
        (*segsRbkStat)[ts] = true; // rebooking made us pass
      }

      if (newResult == SKIP)
      {
        return SKIP;
      }
    }
    else if (result == SKIP)
    {
      return SKIP;
    }

    // Confirmed Sector, rebooked segments are always confirmed, except
    // open segment
    if (needConfStatCheck && ((ts->segmentType() == Open) || !isTvlSegRebooked))
    {
      if (UNLIKELY(diag.isActive()))
      {
        if (isWQTrx)
            diag << "CONFIRM STATUS : AUTOPASS\n";
        else
            diag << "CONFIRM STATUS : " << ts->resStatus() << "\n";
      }

      if (UNLIKELY(isWQTrx && paxTypeFare))
      {
        // browse through applicable travel segments
        RuleUtil::TravelSegWrapperVectorCI iter = applTravelSegment.begin();
        RuleUtil::TravelSegWrapperVectorCI iterEnd = applTravelSegment.end();

        while (iter != iterEnd)
        {
          bool setWarning = false;
          const TravelSeg* tseg = (*iter)->travelSeg();

          if (tseg->segmentOrder() == ts->segmentOrder())
          {
            const FareMarket* fm = nullptr;

            // try to find fare market of current travel segment
            if (pricingUnit)
            {
              for (const auto fu : pricingUnit->fareUsage())
              {
                if (fu &&
                    std::find(fu->travelSeg().begin(), fu->travelSeg().end(), tseg) !=
                        fu->travelSeg().end())
                {
                  fm = fu->paxTypeFare()->fareMarket();
                  break;
                }
              }
            }
            else
            {
              fm = paxTypeFare->fareMarket();
            }

            if (fm)
            {
              if (fm->direction() == FMDirection::OUTBOUND)
              {
                if (advanceResTktInfo.confirmedSector() != returnSectorNoOpen)
                {
                  setWarning = true;
                }
              }
              else if (fm->direction() == FMDirection::INBOUND)
              {
                if (advanceResTktInfo.confirmedSector() != firstSector)
                {
                  setWarning = true;
                }
              }
              else
              {
                if (advanceResTktInfo.confirmedSector() != notApply)
                {
                  setWarning = true;
                }
              }
            }

            if (setWarning)
            {
              displWqWarning2 = true;
              paxTypeFare->warningMap().setCat5WqWarning(tseg->segmentOrder(), true);
            }
          }
          ++iter;
        }
      }

      if (LIKELY(advanceResTktInfo.permitted() != notPermitted))
      {
        checkConfStat = checkConfirmedStatus(*ts);
        // if confirm segment failed bypass it for AA if segment status is DS and not 3.5 cmd
        // pricing.
        if (UNLIKELY(!checkConfStat))
        {
          if ((trx.billing()) && (trx.billing()->aaaCity().size() < 4) &&
              (trx.billing()->partitionID() == SPECIAL_CARRIER_AA) && (trx.getRequest()) &&
              (trx.getRequest()->ticketingAgent()->sabre1SUser()) &&
              (ts->realResStatus() == DS_REAL_RES_STATUS) && (ts->segmentType() != Open) &&
              (paxTypeFare) && (!paxTypeFare->isCmdPricing()))
          {
            LOG4CXX_INFO(_logger, "Cat05 sector confirmation: BYPASSED");
            diag << "BYPASSING CAT05  SEGMENT CONFIRMATION STATUS DS: ";
            diag << ts->origAirport() << " " << ts->destAirport() << "\n";
            checkConfStat = true;
          }
        }
      }

      if (UNLIKELY(!checkConfStat))
      {
        failedOnCfmStat = true;

        if (UNLIKELY(!isWQTrx && diag.isActive()))
        {
          diag << " ADVANCE RESTKT: FAIL - TRAVEL SEG ";
          diag << ts->origAirport() << ts->destAirport();
          diag << " NOT CONFIRMED  \n";
        }

        if (!isWQTrx && ts->segmentType() == Open)
          return FAIL;

        if (!isWQTrx && !allowRebook)
        {
          mayPassAfterRebook = true;
          return FAIL;
        }

        // for WPNC (allowRebook), the segment may be available now
        if (needResDTCheck || needTktDTCheck)
        {
          if (UNLIKELY(!isWQTrx && diag.isActive()))
          {
            diag << " WILL RETRY AVAILABILITY AFTER REBOOK\n";
          }

          // allow rebook and has not tried yet, try rebook
          isTvlSegRebooked = true;

          getBookingDate(trx, *ts, bookingDT, isTvlSegRebooked, segsRbkStat);
          if (UNLIKELY(!isWQTrx && diag.isActive()))
          {
            diag << "  REBOOKING RESERVATION DATE: " << bookingDT.toIsoExtendedString() << "\n";
          }

          Record3ReturnTypes newResult = validateResTkt(bookingDT,
                                                        tktDT,
                                                        needResDTCheck,
                                                        needTktDTCheck,
                                                        advanceResTktInfo,
                                                        needResTktSameDay,
                                                        needResTktSameTime,
                                                        refOrigDT,
                                                        refStartDT,
                                                        refEndDT,
                                                        const_cast<PricingUnit*>(pricingUnit),
                                                        diag,
                                                        mayPassAfterRebook,
                                                        skipResPermitChk,
                                                        isWQTrx,
                                                        displWqWarning1,
                                                        adjustedDateResRestriction,
                                                        trx);
          if (!isWQTrx && newResult == FAIL)
          {
            return FAIL;
          }
        }

        failedOnCfmStat = false;

        if (segsRbkStat)
        {
          (*segsRbkStat)[ts] = true; // rebooking may make us pass
          // depending on availability
        }
      }
    }

    // Now we see if we still need status check for
    // further travel segments
    if (advanceResTktInfo.eachSector() != notApply)
    {
      // validate all sectors
      continue;
    }

    if (advanceResTktInfo.confirmedSector() == firstSector)
    {
      //  FIRST SECTOR OF THE FIRST F.C. OF THE P.U. MUST BE CONFIRMED
      needConfStatCheck = false; // no more in the loop
      if (LIKELY(advanceResTktInfo.permitted() != notPermitted))
      {
        // notPermitted will force reservation for all segs
        // be same day as orig departure
        needResDTCheck = false;
        needTktDTCheck = false;
      }
    }
    else if (advanceResTktInfo.confirmedSector() == notApply) // Blank
    {
      //  ALL SECTORS OF ALL F.C. WITHIN THE P.U. PRIOR TO THE
      //   POINT OF TURNAROUND MUST BE CONFIRMED

      RuleUtil::TravelSegWrapperVectorCI nextTSWI = applTravelSegmentI + 1;
      if (nextTSWI != applTravelSegmentEndI)
      {
        ts = (*nextTSWI)->travelSeg();
        if (ts == turnAroundTvlSeg)
        {
          needConfStatCheck = false; // no more in the loop
          if (advanceResTktInfo.permitted() != notPermitted)
          {
            needResDTCheck = false;
            needTktDTCheck = false;
          }
        }
      }
    }
    // else: allSector || returnSectorNoOpen
    //  ALL SECTORS OF ALL F.C. WITHIN THE P.U. MUST BE CONFIRMED
  }
  return PASS;
}

Record3ReturnTypes
AdvanceResTkt::validateAdvanceTktTime(PricingUnit* pricingUnit,
                                      const DateTime& ticketDT,
                                      const DateTime& bookDT,
                                      const DateTime& departureDT,
                                      const AdvResTktInfo& advanceResTktInfo,
                                      DiagManager& diag,
                                      bool& mayPassAfterRebook) const
{
  // Adjust ticketDT time to match departure city time
  DateTime tktdate = ticketDT;

  // Ticketing Waiver Time
  if (UNLIKELY(isValidATSEDate(advanceResTktInfo.waiverTktDate()) &&
      (ticketDT < advanceResTktInfo.waiverTktDate())))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "TKT DATE " << ticketDT.toIsoExtendedString() << " EARLIER THAN WAIVER DATE "
           << advanceResTktInfo.waiverTktDate().toIsoExtendedString() << "\n";
      diag << "NO TICKET RESTRICTION APPLY\n";
    }
    return NOTPROCESSED; // Each sector ticketDT are the same, so by default no ticket restriction
    // check any more for others
  }

  // Ticketing exception date, if reservation made after the date
  // no ticketing restriction apply
  if (advanceResTktInfo.advTktExcpUnit() != notApply)
  {
    DateTime dt1;
    DateTime& limitDT = dt1;

    if (UNLIKELY(getLimitDateTime(limitDT,
                         departureDT,
                         0,
                         advanceResTktInfo.advTktExcpTime(),
                         advanceResTktInfo.advTktExcpUnit(),
                         BEFORE_REF_TIME) == false))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << " ADVANCE RESTKT: FAILED "
             << "- DID NOT UNDERSTAND TKT EXCEPT TIME OR UNIT \n";
        diag << "TIME " << advanceResTktInfo.advTktExcpTime() << "\n";
        diag << "UNIT " << advanceResTktInfo.advTktExcpUnit() << "\n";
      }
      return FAIL;
    }

    if (bookDT < limitDT)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "RES DATE EARLIER THAN TKT EXCEPT TIME " << limitDT.toIsoExtendedString() << "\n";
        diag << "  TKT REQS APPLY\n";
      }
      (const_cast<AdvanceResTkt*>(this))->_matchedTktExptTime = true;
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "RES DATE LATER THAN TKT EXCEPT TIME " << limitDT.toIsoExtendedString() << "\n";
        diag << "  TKT REQS NOT APPLY\n";
      }
      return SKIP;
    }
  }

  // OK, we do need ticketing validation
  // We will validate whatever is appliable
  if (UNLIKELY(diag.isActive()))
  {
    diag << "  TICKET DATE: ";
    diag << ticketDT.toIsoExtendedString() << "\n";

    if (this->_diagFromToDate)
    {
      diag << "  FROM DATE: " << ticketDT.toIsoExtendedString();
      diag << "  TO DATE: " << departureDT.toIsoExtendedString() << "\n";
    }
  }

  bool checkDTAgainstRes = false;
  DateTime dt1;
  DateTime& limitDTAfterRes = dt1;

  const ResUnit& advTktUnit = getAdvTktUnit(advanceResTktInfo);

  if (advTktUnit.length() != 0)
  {
    if (UNLIKELY(_ignoreTktAfterResRestriction))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "LIMIT AFTER RESERVATION IGNORED\n";
      }
    }
    else
    {
      // limitDTAfterRes is same as ticketingDt if bookingDatevalidationSkip is set in itin
      bool resultGetRes = (_itin) && (_itin->cat05BookingDateValidationSkip())
                              ? getLimitDateTime(limitDTAfterRes,
                                                 ticketDT,
                                                 getAdvTktTod(advanceResTktInfo),
                                                 getAdvTktPeriod(advanceResTktInfo),
                                                 advTktUnit,
                                                 AFTER_REF_TIME)
                              : getLimitDateTime(limitDTAfterRes,
                                                 bookDT,
                                                 getAdvTktTod(advanceResTktInfo),
                                                 getAdvTktPeriod(advanceResTktInfo),
                                                 advTktUnit,
                                                 AFTER_REF_TIME);
      if (UNLIKELY(_itin && _itin->cat05BookingDateValidationSkip()))
      {
        (const_cast<AdvanceResTkt*>(this))->setcat05overrideDisplayFlag();
        if (pricingUnit)
          pricingUnit->isOverrideCxrCat05TktAftRes() = true;
      }

      if (UNLIKELY(!resultGetRes))
      {
        diag << "WRONG INPUT ABOUT TICKETING TIME LIMIT AFTER RESERVATION - IGNORE RULE\n";
        checkDTAgainstRes = false;
      }
      else
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << "LIMIT AFTER RESERVATION ";
          diag << limitDTAfterRes.toIsoExtendedString() << "\n";
        }
        checkDTAgainstRes = true;
      }
    }
  }

  if (UNLIKELY(_pricingTrx && RexPricingTrx::isRexTrxAndNewItin(*_pricingTrx) && pricingUnit == nullptr))
    checkDTAgainstRes = false;

  bool checkDTAgainstDepart = false;
  DateTime dt2;
  DateTime& limitDTBeforeDepart = dt2;
  const Indicator& advTktOpt = getAdvTktOpt(advanceResTktInfo);

  if (UNLIKELY(advTktOpt == firstTimePermit && advanceResTktInfo.tktTSI() == 0 && !_isPTFBeginJxr &&
      pricingUnit == nullptr && advanceResTktInfo.eachSector() == notApply))
  {
    checkDTAgainstDepart = false;
    if (UNLIKELY(diag.isActive()))
    {
      diag << "EALIEST TKTDT AGAINST PU DEPARTURE REVALIDATE AT PU SCOPE\n";
    }
  }
  else if (advTktOpt != notApply)
  {
    const bool forLatestTOD = (advTktOpt != firstTimePermit);

    if (UNLIKELY(_ignoreTktDeforeDeptRestriction))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "LIMIT BEFORE DEPARTURE IGNORED\n";
      }
    }
    else
    {
      if (UNLIKELY(getLimitDateTime(limitDTBeforeDepart,
                           departureDT,
                           0,
                           getAdvTktDepart(advanceResTktInfo),
                           getAdvTktDepartUnit(advanceResTktInfo),
                           BEFORE_REF_TIME,
                           forLatestTOD) == false))
      {
        diag << "WRONG INPUT ABOUT TICKETING TIME LIMIT AFTER BEFORE DEPARTURE - IGNORE RULE\n";
        checkDTAgainstDepart = false;
      }
      else
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << "LIMIT BEFORE DEPARTURE ";
          diag << limitDTBeforeDepart.toIsoExtendedString() << "\n";
        }
        checkDTAgainstDepart = true;
      }
    }
  }

  const Indicator& advTktBoth = getAdvTktBoth(advanceResTktInfo);

  // We may need only to check one of the dates
  if ((advTktBoth != notApply) && checkDTAgainstRes && checkDTAgainstDepart)
  {
    if (advTktBoth == earlierTime)
    {
      // Earlier of the two ticket issuance requirements apply
      if (limitDTAfterRes < limitDTBeforeDepart)
        checkDTAgainstDepart = false;
      else
        checkDTAgainstRes = false;

      if (UNLIKELY(diag.isActive()))
      {
        diag << "EARLIER OF THE TWO TICKET ISSUANCE ";
        if (checkDTAgainstRes == true)
          diag << limitDTAfterRes.toIsoExtendedString() << "\n";
        else
          diag << limitDTBeforeDepart.toIsoExtendedString() << "\n";
      }
    }
    else
    {
      // Later of the two ticket issuance requirements apply
      if (limitDTAfterRes < limitDTBeforeDepart)
        checkDTAgainstRes = false;
      else
        checkDTAgainstDepart = false;

      if (UNLIKELY(diag.isActive()))
      {
        diag << "LATER OF THE TWO TICKET ISSUANCE ";
        if (checkDTAgainstRes == true)
          diag << limitDTAfterRes.toIsoExtendedString() << "\n";
        else
          diag << limitDTBeforeDepart.toIsoExtendedString() << "\n";
      }
    }
  }

  if (checkDTAgainstRes)
  {
    if (UNLIKELY(ticketDT > limitDTAfterRes))
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "TICKETING TIME  " << ticketDT.toIsoExtendedString() << "\n";
        diag << "     PAST TIME LIMIT AFTER RESERVATION  " << limitDTAfterRes.toIsoExtendedString()
             << "\n";
      }
      mayPassAfterRebook = true;
      return FAIL;
    }
    if (pricingUnit != nullptr && !pricingUnit->dynamicValidationPhase())
    {
      pricingUnit->latestTktDT().setWithEarlier(limitDTAfterRes);
    }
    notifyObservers(LATEST_TKT_DAY, limitDTAfterRes);
  }

  if (checkDTAgainstDepart)
  {
    if (UNLIKELY(advTktOpt == firstTimePermit))
    {
      // Earliest time ticketing is required before departure of flight
      if (tktdate < limitDTBeforeDepart)
      {
        diag << " TICKETING TIME MUST NOT BE EARLIER THAN REQUIRED TIME BEFORE DEPARTURE\n";
        return FAIL;
      }
    }
    else
    {
      if (tktdate >= limitDTBeforeDepart)
      {
        diag << " TICKETING TIME MUST NOT BE LATER THAN REQUIRED TIME BEFORE DEPARTURE\n";
        return FAIL;
      }

      if (pricingUnit != nullptr && !pricingUnit->dynamicValidationPhase())
      {
        pricingUnit->latestTktDT().setWithEarlier(limitDTBeforeDepart);
      }
      notifyObservers(LATEST_TKT_DAY, limitDTBeforeDepart);
    }
  }

  return PASS;
}

Record3ReturnTypes
AdvanceResTkt::validateAdvanceResTime(const DateTime& bookDT,
                                      const DateTime& refStartDT,
                                      const DateTime& refEndDT,
                                      const AdvResTktInfo& advanceResTktInfo,
                                      DiagManager& diag,
                                      bool& mayPassAfterRebook) const
{
  if (UNLIKELY(isValidATSEDate(advanceResTktInfo.waiverResDate()) &&
      (bookDT < advanceResTktInfo.waiverResDate())))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << "RES DATE " << bookDT.toIsoExtendedString() << " EARLIER THAN WAIVER DATE "
           << advanceResTktInfo.waiverResDate().toIsoExtendedString() << "\n";
      diag << "NO RESERVATION RESTRICTION APPLY\n";
    }
    return PASS;
  }

  if (advanceResTktInfo.firstResUnit().length() != 0)
  {
    DateTime dt1;
    DateTime& firstLimitDT = dt1;

    if (getLimitDateTime(firstLimitDT,
                         refStartDT,
                         advanceResTktInfo.firstResTod(),
                         advanceResTktInfo.firstResPeriod(),
                         advanceResTktInfo.firstResUnit(),
                         BEFORE_REF_TIME,
                         false // forLatestTOD
                         ) == false)
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "WRONG INPUT ABOUT THE EARLIEST RESERVATION TIME ALLOWED - IGNORED RULE\n";
      }
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "THE EARLIEST RESERVATION TIME ALLOWED - " << firstLimitDT.toIsoExtendedString()
             << "\n";
      }

      if (bookDT < firstLimitDT)
      {
        diag << "RESERVATION TIME IS EARLIER THAN THE EARLIEST TIME ALLOWED - FAIL\n";
        mayPassAfterRebook = true;
        return FAIL;
      }
    }
  }

  if (advanceResTktInfo.lastResUnit().length() != 0)
  {
    DateTime dt2;
    DateTime& lastLimitDT = dt2;

    if (UNLIKELY(getLimitDateTime(lastLimitDT,
                         refEndDT,
                         advanceResTktInfo.lastResTod(),
                         advanceResTktInfo.lastResPeriod(),
                         advanceResTktInfo.lastResUnit(),
                         BEFORE_REF_TIME) == false))
    {
      diag << "WRONG INPUT ABOUT THE LATEST RESERVATION TIME ALLOWED - IGNORED RULE\n";
    }
    else
    {
      if (UNLIKELY(diag.isActive()))
      {
        diag << "THE LATEST RESERVATION TIME ALLOWED - " << lastLimitDT.toIsoExtendedString()
             << "\n";
      }

      if (bookDT >= lastLimitDT)
      {
        diag << "RESERVATION TIME IS LATER THAN THE LATEST TIME ALLOWED - FAIL\n";
        // do not set mayPassAfterRebook = true here
        // because we know it will not pass even after rebook
        return FAIL;
      }
      notifyObservers(LAST_DATE_TO_BOOK, lastLimitDT);
    }
  }

  return PASS;
}

bool
AdvanceResTkt::getLimitDateTime(DateTime& limitTimeReturn,
                                const DateTime& referenceDT,
                                const int16_t& resTod,
                                const ResPeriod& resPeriod,
                                const ResUnit& resUnit,
                                const BeforeOrAfter& beforeOrAfter,
                                const bool forLatestTOD,
                                const bool keepTime)
{
  try
  {
    if (UNLIKELY(resPeriod.length() == 0))
      return false;

    if (UNLIKELY(!referenceDT.isValid()))
      return false;

    if (LIKELY(!isalpha(resPeriod.c_str()[0])))
    {
      // Should be period(000-999), unit(N, H, D, M)
      int period = -1;

      period = atoi(resPeriod.c_str());
      if (UNLIKELY(period < 0))
        return false;

      Indicator unitInd = static_cast<Indicator>(resUnit.c_str()[0]);

      // call override functions
      return getLimitDateTime(limitTimeReturn,
                              referenceDT,
                              resTod,
                              static_cast<uint16_t>(period),
                              unitInd,
                              beforeOrAfter,
                              forLatestTOD,
                              keepTime);
    }

    // Period measured by how many times Week Day occurred.

    // find out which DayOfWeek(SUN .. SAT) ResPeriod is
    uint32_t limitDayOfWeek = 0;

    while (limitDayOfWeek < DAYS_PER_WEEK)
    {
      if (resPeriod == WEEKDAYS_UPPER_CASE[limitDayOfWeek])
        break;

      limitDayOfWeek++;
    }

    if (limitDayOfWeek == DAYS_PER_WEEK) // no match
      return false;

    int32_t accurrence = 0;
    accurrence = atoi(resUnit.c_str());

    if (accurrence <= 0) // does not make sense
      return false;

    int32_t numDaysDiff =
        static_cast<int32_t>(referenceDT.dayOfWeek()) - static_cast<int32_t>(limitDayOfWeek);

    if (numDaysDiff > 0)
      numDaysDiff += (accurrence - 1) * DAYS_PER_WEEK;
    else
      numDaysDiff += accurrence * DAYS_PER_WEEK;

    TimeDuration td(numDaysDiff * HOURS_PER_DAY, 0, 0);
    DateTime tmpDT = referenceDT;

    if (beforeOrAfter == BEFORE_REF_TIME)
      tmpDT -= td;
    else
      tmpDT += td;

    if (keepTime)
    {
      limitTimeReturn = tmpDT;
      return true;
    }

    if (!isValidTOD(resTod))
    {
      // 23:59 or 00:00 is the default time
      const int32_t hour = forLatestTOD ? 23 : 0;
      const int32_t minute = forLatestTOD ? 59 : 0;

      limitTimeReturn = DateTime(tmpDT.year(), tmpDT.month(), tmpDT.day(), hour, minute, 0);
    }
    else
    {
      // as OCT23 24:00 is OCT24 0:00, and we will display
      // OCT24 as LAST DAY TO PURCHASE, to avoid misleading, overwrite
      // 24:00 (1440) to 23:59.
      int32_t newHour = 0;
      int32_t newMinute = 0;

      if (resTod == HIGHEND_TIMEOFDAY)
      {
        newHour = 23;
        newMinute = 59;
      }
      else
      {
        newHour = resTod / 60;
        newMinute = resTod % 60;
      }
      limitTimeReturn = DateTime(tmpDT.year(), tmpDT.month(), tmpDT.day(), newHour, newMinute, 0);
    }

    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool
AdvanceResTkt::getLimitDateTime(DateTime& limitTimeReturn,
                                const DateTime& referenceDT,
                                const int16_t& resTod,
                                const uint16_t& period,
                                const Indicator& unitInd,
                                const BeforeOrAfter& beforeOrAfter,
                                const bool forLatestTOD,
                                const bool keepTime)
{
  if (unitInd == MONTH_UNIT_INDICATOR)
  {
    // Since days of month varies, do special process and return
    uint16_t monthsDiff = period;
    uint16_t yearsDiff = monthsDiff / 12;
    monthsDiff %= 12;

    uint16_t newMonth = static_cast<uint16_t>(referenceDT.month());
    uint16_t newYear = static_cast<uint16_t>(referenceDT.year());

    if (beforeOrAfter == BEFORE_REF_TIME)
    {
      if (monthsDiff < newMonth)
      {
        newMonth -= monthsDiff;
      }
      else
      {
        newMonth += (12 - monthsDiff);
        yearsDiff++;
      }
      newYear -= yearsDiff;
    }
    else
    {
      newMonth += monthsDiff;
      yearsDiff += (newMonth - 1) / 12;
      newMonth %= 12;
      if (newMonth == 0)
        newMonth = 12;
      newYear += yearsDiff;
    }
    int16_t newDay =
        (DateTime::endOfMonthDay(newYear, newMonth) < static_cast<int16_t>(referenceDT.day()))
            ? DateTime::endOfMonthDay(newYear, newMonth)
            : static_cast<int16_t>(referenceDT.day());

    if (keepTime)
    {
      limitTimeReturn = DateTime(Year(newYear),
                                 Month(newMonth),
                                 Day(newDay),
                                 referenceDT.hours(),
                                 referenceDT.minutes(),
                                 0);
      return true;
    }

    // Unless the resTod is defined, 23:59 or 00:00 will be the time
    int32_t newHour = 0;
    int32_t newMinute = 0;

    if (isValidTOD(resTod))
    {
      // as OCT23 24:00 is OCT24 0:00, and we will display
      // OCT24 as LAST DAY TO PURCHASE, to avoid misleading, overwrite
      // 24:00 (1440) to 23:59.
      if (resTod == HIGHEND_TIMEOFDAY)
      {
        newHour = 23;
        newMinute = 59;
      }
      else
      {
        newHour = resTod / 60;
        newMinute = resTod % 60;
      }
    }
    else
    {
      newHour = forLatestTOD ? 23 : 0;
      newMinute = forLatestTOD ? 59 : 0;
    }

    limitTimeReturn = DateTime(Year(newYear), Month(newMonth), Day(newDay), newHour, newMinute, 0);

    return true;
  } // MONTH_UNIT_INDICATOR

  uint32_t minutes = 0; // In all cases, we use minutes to represent duration

  if (unitInd == DAY_UNIT_INDICATOR)
    minutes = period * MINUTES_PER_DAY;
  else if (unitInd == HOUR_UNIT_INDICATOR)
  {
    minutes = period == 0 ? 59 : period * MINUTES_PER_HOUR;
  }
  else if (unitInd == MINUTE_UNIT_INDICATOR)
    minutes = period;
  else
    return false;

  TimeDuration td(0, minutes, 0);
  DateTime tmpDT = referenceDT;

  if (beforeOrAfter == BEFORE_REF_TIME)
    tmpDT -= td;
  else
    tmpDT += td;

  if (UNLIKELY(keepTime))
  {
    limitTimeReturn = tmpDT;
    return true;
  }

  if (!isValidTOD(resTod))
  {
    // Unless unit is Hour or Minute, default time will be
    // 23:59 for latest comparison or 00:00 for earliest comparison
    if ((unitInd == HOUR_UNIT_INDICATOR) || (unitInd == MINUTE_UNIT_INDICATOR))
    {
      limitTimeReturn = tmpDT;
    }
    else
    {
      const int32_t hour = forLatestTOD ? 23 : 0;
      const int32_t minute = forLatestTOD ? 59 : 0;

      limitTimeReturn = DateTime(tmpDT.year(), tmpDT.month(), tmpDT.day(), hour, minute, 0);
    }
  }
  else
  {
    int32_t newHour = 0;
    int32_t newMinute = 0;

    if (resTod == HIGHEND_TIMEOFDAY)
    {
      newHour = 23;
      newMinute = 59;
    }
    else
    {
      newHour = resTod / 60;
      newMinute = resTod % 60;
    }

    limitTimeReturn = DateTime(tmpDT.year(), tmpDT.month(), tmpDT.day(), newHour, newMinute, 0);
  }

  return true;
}

bool
AdvanceResTkt::checkConfirmedStatus(const TravelSeg& travelSeg) const
{
  return (travelSeg.resStatus() == CONFIRM_RES_STATUS) ||
         (travelSeg.resStatus() == NOSEAT_RES_STATUS);
}

bool
AdvanceResTkt::checkConfirmedStatusNotPermitted(const TravelSeg& travelSeg,
                                                const AdvResTktInfo& advanceResTktInfo,
                                                const DateTime& ticketDT,
                                                const DateTime& departureDT) const
{
  if (advanceResTktInfo.permitted() != notPermitted)
    return false;

  // if status is CONFIRM_RES_STATUS we pass when ticket date is same as departure date
  if (travelSeg.resStatus() == CONFIRM_RES_STATUS)
    return ticketDT.date() == departureDT.date();

  // if status is not CONFIRM_RES_STATUS, we pass
  return true;
}

const AdvResTktUnitName
AdvanceResTkt::unitName(Indicator unit) const
{
  switch (unit)
  {
  case DAY_UNIT_INDICATOR: // Days
    return DAY_UNIT_NAME;

  case MONTH_UNIT_INDICATOR: // Months
    return MONTH_UNIT_NAME;

  case MINUTE_UNIT_INDICATOR: // Minutes
    return MINUTE_UNIT_NAME;

  case HOUR_UNIT_INDICATOR: // Hours
    return HOUR_UNIT_NAME;
  default:
    break;
  }
  return UNKNOWN_UNIT_NAME;
}

bool
AdvanceResTkt::existResTktRestr(const AdvResTktInfo& advanceResTktInfo) const
{
  return (
      advanceResTktInfo.permitted() != notApply || !advanceResTktInfo.firstResUnit().empty() ||
      !advanceResTktInfo.lastResUnit().empty() || advanceResTktInfo.ticketed() != notApply ||
      advanceResTktInfo.advTktExcpUnit() != notApply || !getAdvTktUnit(advanceResTktInfo).empty() ||
      getAdvTktOpt(advanceResTktInfo) != notApply || getAdvTktBoth(advanceResTktInfo) != notApply);
}

bool
AdvanceResTkt::resCanNotBeMadeNow(const RuleUtil::TravelSegWrapper& refOrigTvlSegW,
                                  const AdvResTktInfo& advanceResTktInfo) const
{
  if (advanceResTktInfo.firstResUnit().length() == 0)
    return false;

  const TravelSeg& refOrigTvlSeg = *refOrigTvlSegW.travelSeg();
  const DateTime& refDT =
      refOrigTvlSegW.origMatch() ? refOrigTvlSeg.departureDT() : refOrigTvlSeg.arrivalDT();

  DateTime firstLimitDT;

  if (getLimitDateTime(firstLimitDT,
                       refDT,
                       advanceResTktInfo.firstResTod(),
                       advanceResTktInfo.firstResPeriod(),
                       advanceResTktInfo.firstResUnit(),
                       BEFORE_REF_TIME,
                       false // forLatestTOD
                       ) == false)
  {
    return false;
  }

  return firstLimitDT > DateTime::localTime();
}
void
AdvanceResTkt::displayCat05OverrideDataToDiag(DiagManager& diagm) const
{
  if (!diagm.isActive())
    return;
  if ((_itin) && (_itin->cat05BookingDateValidationSkip()))
  {
    diagm.collector() << "ATTN OVERRIDDEN WITH PRICING DATE" << std::endl;
  }
  return;
}
void
AdvanceResTkt::displayRuleDataToDiag(const AdvResTktInfo& advanceResTktInfo, DiagCollector& diag)
{
  //   CATEGORY 5 RULE DATA
  //-------------------------------------------------------------------------
  diag << "CATEGORY 5 RULE DATA:" << std::endl;

  //--------------------------
  // Unavailable Tag
  //--------------------------
  if (advanceResTktInfo.unavailtag() != notApply)
  {
    if (advanceResTktInfo.unavailtag() == dataUnavailable)
    {
      diag << " UNAVAILABLE DATA" << std::endl;
    }
    else
    {
      diag << " TEXT DATA ONLY" << std::endl;
    }
    //----------------------------------------------------------------------
    //    End of Display
    //----------------------------------------------------------------------
    diag << "**************************************************************\n";

    diag.flushMsg();
    return;
  }

  //--------------------------
  // Waiver Reservation Date
  //--------------------------
  diag << " LAST DATE WAIVER APPLIES TO ADV RES REQUIREMENT: ";
  if (isValidATSEDate(advanceResTktInfo.waiverResDate()))
  {
    diag << advanceResTktInfo.waiverResDate().dateToIsoExtendedString() << std::endl;
  }
  else
  {
    diag << "N/A" << std::endl;
  }

  //--------------------------
  // Waiver Ticketing Date
  //--------------------------
  diag << " LAST DATE WAIVER APPLIES TO ADV TKT REQUIREMENT: ";
  if (isValidATSEDate(advanceResTktInfo.waiverTktDate()))
  {
    diag << advanceResTktInfo.waiverTktDate().dateToIsoExtendedString() << std::endl;
  }
  else
  {
    diag << "N/A" << std::endl;
  }

  bool anyRestriction = false; // at some points, if anyRestriction is still
  // false, "NOT APPLICABLE" will be wriiten to
  // diagnostic as no restriction applied

  //--------------------------
  // Advance Reservation
  //--------------------------
  diag << " ADVANCE RESERVATION:" << std::endl;

  // Permitted Indicator
  if (advanceResTktInfo.permitted() == notPermitted)
  {
    diag << "  ADVANCE RESERVATION IS NOT PERMITTED" << std::endl;
    // Res TSI
    if (advanceResTktInfo.resTSI() != 0)
    {
      anyRestriction = true;
      diag << "  RESERVATION TSI: " << advanceResTktInfo.resTSI() << std::endl;
    }
  }
  else
  {
    if (advanceResTktInfo.firstResUnit().length() != 0)
    {
      anyRestriction = true;
      ;

      // Earliest time reservation is permitted before departure of flight
      diag << "  FIRST TIME RES PERMITTED BEFORE DEPARTURE: ";
      diag << advanceResTktInfo.firstResPeriod() << " ";

      if (isalpha(advanceResTktInfo.firstResPeriod().c_str()[0]))
      {
        diag << advanceResTktInfo.firstResUnit() << "times in front" << std::endl;
      }
      else
      {
        diag << unitName(advanceResTktInfo.firstResUnit().c_str()[0]) << std::endl;
      }

      if (isValidTOD(advanceResTktInfo.firstResTod()))
        diag << "                                         AS OF" << advanceResTktInfo.firstResTod()
             << std::endl;
    }

    // Latest time reservation is permitted before departure of flight
    if (advanceResTktInfo.lastResUnit().length() != 0)
    {
      anyRestriction = true;
      ;

      diag << "  LAST TIME RES PERMITTED BEFORE DEPARTURE: ";
      diag << advanceResTktInfo.lastResPeriod() << " ";

      if (isalpha(advanceResTktInfo.lastResPeriod().c_str()[0]))
      {
        diag << advanceResTktInfo.lastResUnit() << "TIMES OCCUR" << std::endl;
      }
      else
      {
        diag << unitName(advanceResTktInfo.lastResUnit().c_str()[0]) << std::endl;
      }

      if (isValidTOD(advanceResTktInfo.lastResTod()))
        diag << "                                         UNTIL " << advanceResTktInfo.lastResTod()
             << std::endl;
    }

    // Res TSI
    if ((advanceResTktInfo.firstResUnit().length() != 0 ||
         advanceResTktInfo.lastResUnit().length() != 0) &&
        (advanceResTktInfo.resTSI() != 0))
    {
      anyRestriction = true;
      diag << "  RESERVATION TSI: " << advanceResTktInfo.resTSI() << std::endl;
    }
  } // Permitted indicator

  // Ticketed Indicator
  if (advanceResTktInfo.ticketed() != notApply)
  {
    anyRestriction = true;
    ;
    diag << "  RESERVATION MUST BE MADE AND TICKET ISSUED AT SAME TIME" << std::endl;
  }

  // Standby
  if (advanceResTktInfo.standby() != notApply)
  {
    anyRestriction = true;
    ;
    switch (advanceResTktInfo.standby())
    {
    case noWaitAndStandby:
      diag << "  WAITLIST AND STANDBY NOT PERMITTED" << std::endl;
      break;
    case noWait:
      diag << "  WAITLIST NOT PERMITTED" << std::endl;
      break;
    case noOrigWaitAndStandby:
      diag << "  WAITLIST AND STANDBY NOT PERMITTED ON ORIG FLIGHT" << std::endl;
      break;
    case noOrigStandby:
      diag << "  STANDBY NOT PERMITTED ON ORIG FLIGHT" << std::endl;
      break;
    case noOrigWait:
      diag << "  WAITLIST NOT PERMITTED ON ORIG FLIGHT" << std::endl;
      break;
    case noStandbyOrOrigWait:
      diag << "  STANDBY OR ORIG WAITLIST NOT PERMITTED" << std::endl;
      break;
    case noWaitOrOrigStandby:
      diag << "  WAITLIST OR ORIG STANDBY NOT PERMITTED" << std::endl;
      break;
    case onlyStandbyOnSameDayFlightNoOthers:
      diag << "  STANDBY PERMITED FOR EARLIER/LATER SAME DAY FLIGHTS" << std::endl;
      diag << "  OTHERWISE WAITLIST AND STANDBY NOT PERMITTED" << std::endl;
      break;
    case onlyStandbyOnSameDayFlightNoOtherStandby:
      diag << "  STANDBY PERMITED FOR EARLIER/LATER SAME DAY FLIGHTS" << std::endl;
      diag << "  OTHERWISE STANDBY NOT PERMITTED" << std::endl;
      break;
    case onlyStandbyOnSameDayFlightNoOthersOnOrig:
      diag << "  STANDBY PERMITED FOR EARLIER/LATER SAME DAY FLIGHTS" << std::endl;
      diag << "  OTHERWISE WAITLIST AND STANDBY NOT PERMITTED ON ORIG" << std::endl;
      break;
    case onlyWaitOnSameDayFlightNoOthersOnOrig:
      diag << "  WAITLIST PERMITED FOR EARLIER/LATER SAME DAY FLIGHTS" << std::endl;
      diag << "  OTHERWISE WAITLIST AND STANDBY NOT PERMITTED ON ORIG" << std::endl;
      break;
    case standbyOnSameDateTicketed:
      diag << "  STANDBY PERMITED FOR SAME DATE AS ORIGINALLY TICKETED" << std::endl;
      diag << "  PROVIDED ANY FLIGHT/TIME SPECIFIC PROVISIONS ARE MET" << std::endl;
      break;
    case noStandby:
      diag << "  STANDBY NOT PERMITED" << std::endl;
      break;
    default:
      break;
    }
  } // standby

  if (!anyRestriction)
    diag << "  NOT APPLICABLE" << std::endl;

  //--------------------------
  // Confirmed Status
  //--------------------------

  // Note: Keep anyRestriction indicator set for Advance Res as it is

  diag << std::endl << " CONFIRMED STATUS:" << std::endl;
  const int geoTbl = advanceResTktInfo.geoTblItemNo();

  // Each Sector
  if (advanceResTktInfo.eachSector() != notApply)
  {
    diag << "  ADVANCE RES IS MEASURED FROM THE ORIGINATING FLIGHT OF THE" << std::endl;
    diag << "  FARE COMPONENT BEING VALIDATED AND CONFIRMED RES IS REQUIRED" << std::endl;
    if (geoTbl == 0)
    {
      diag << "  FOR EACH SECTOR OF THE FARE COMPONENT BEING VALIDATED" << std::endl;
    }
    else
    {
      diag << "  FOR EACH GEO SECTOR OF THE FARE COMPONENT BEING VALIDATED" << std::endl;
    }

    anyRestriction = true;
  }
  else
  {
    // Confirmed Sector

    if (advanceResTktInfo.confirmedSector() == firstSector)
    {
      if (geoTbl == 0)
      {
        diag << "  FIRST SECTOR OF THE FIRST F.C. OF THE P.U. MUST BE CONFIRMED" << std::endl;
      }
      else
      {
        diag << "  FIRST SECTOR OF THE GEO SECTORS MUST BE CONFIRMED" << std::endl;
      }

      anyRestriction = true;
    }
    else if (advanceResTktInfo.confirmedSector() == allSector ||
             advanceResTktInfo.confirmedSector() == returnSectorNoOpen)
    {
      if (geoTbl == 0)
      {
        diag << "  ALL SECTORS OF ALL F.C. WITHIN THE P.U. MUST BE CONFIRMED" << std::endl;
      }
      else
      {
        diag << "  ALL GEO SECTORS MUST BE CONFIRMED" << std::endl;
      }
      anyRestriction = true;
    }
    else // Blank
    {
      if (geoTbl != 0)
      {
        diag << "  ALL SECTORS SPEC BY GEO TBL MUST BE CONFIRMED" << std::endl;
      }
      else
      {
        diag << "  ALL SECTORS OF ALL F.C. WITHIN THE P.U. PRIOR TO THE" << std::endl;
        diag << "   POINT OF TURNAROUND MUST BE CONFIRMED" << std::endl;
      }
      anyRestriction = true;
    }
  }

  // Geographic Specification Table 995
  if (geoTbl != 0)
  {
    diag << "  GEOGRAPHIC SPECIFICATION TABLE 995: " << geoTbl;
  }

  //--------------------------
  // Advance Ticketing
  //--------------------------

  anyRestriction = false;

  diag << std::endl << " ADVANCE TICKETING:" << std::endl;

  // Exception Time
  // Ticketing requirements are only applicable if reservation are made
  // by the time specified (before departure)
  if (advanceResTktInfo.advTktExcpUnit() != notApply)
  {
    diag << "  TKT REQS APPLY IF RES IS MADE " << advanceResTktInfo.advTktExcpTime() << " ";
    diag << unitName(advanceResTktInfo.advTktExcpUnit());
    diag << " BEFORE DEPARTURE" << std::endl;

    anyRestriction = true;
  }

  const ResUnit& advTktUnit = getAdvTktUnit(advanceResTktInfo);
  // Amount of time after reservation is made that ticket issuance is required
  if (advTktUnit.length() != 0)
  {
    const ResPeriod& advTktPeriod = getAdvTktPeriod(advanceResTktInfo);

    anyRestriction = true;
    ;

    diag << "  TIME WHEN TKT IS REQUIRED AFTER RES: ";
    diag << advTktPeriod << " ";

    if (isalpha(advTktPeriod.c_str()[0]))
    {
      diag << advTktUnit << "TIMES OCCUR" << std::endl;
    }
    else
    {
      diag << unitName(advTktUnit.c_str()[0]) << std::endl;
    }

    const int& advTktTod = getAdvTktTod(advanceResTktInfo);
    if (isValidTOD(advTktTod))
      diag << "                                         UNTIL " << std::setw(2) << std::setfill('0')
           << advTktTod / 60 << ":" << std::setw(2) << std::setfill('0') << advTktTod % 60 << ":00"
           << std::endl;

    anyRestriction = true;
  }

  const Indicator& advTktOpt = getAdvTktOpt(advanceResTktInfo);
  // Time ticketing is required before departure of flight
  // Option field: P = first time and R = last time
  if (advTktOpt != notApply)
  {
    if (advTktOpt == firstTimePermit)
    {
      // Earliest time ticketing is required before departure of flight
      diag << "  FIRST TIME TKT IS PERMITTED PRIOR TO DEPARTURE: ";
    }
    else
    {
      // Latest time ticketing is required before departure of flight
      diag << "  TIME PRIOR TO DEPARTURE TKT IS REQUIRED: ";
    }
    diag << getAdvTktDepart(advanceResTktInfo) << " ";
    diag << unitName(getAdvTktDepartUnit(advanceResTktInfo)) << std::endl;

    anyRestriction = true;
  }

  // Tkt TSI
  if (advanceResTktInfo.tktTSI() != 0)
  {
    diag << "  TICKETING TSI: " << advanceResTktInfo.tktTSI() << std::endl;
    anyRestriction = true;
  }

  const Indicator& advTktBoth = getAdvTktBoth(advanceResTktInfo);
  // Both field: E = earlier time and L = later time
  if (advTktBoth != notApply)
  {
    // Both a time before departure and time after reservation are specified

    if (advTktBoth == earlierTime)
    {
      // Earlier of the two ticket issuance requirements apply
      diag << "  EARLIER OF THE TWO TICKET ISSUANCE REQUIREMENTS APPLY" << std::endl;
    }
    else
    {
      // Later of the two ticket issuance requirements apply
      diag << "  LATER OF THE TWO TICKET ISSUANCE REQUIREMENTS APPLY" << std::endl;
    }
  }

  if (!anyRestriction)
    diag << "  NOT APPLICABLE" << std::endl;

  //-------------------------------------------------------------------------
  //    End of Display
  //-------------------------------------------------------------------------
  diag << "**************************************************************" << std::endl;

  diag.flushMsg();
}

bool
AdvanceResTkt::checkAvail(const TravelSeg& tvlSeg,
                          const std::vector<ClassOfService*>& cosVec,
                          PricingTrx& trx,
                          const PaxTypeFare& paxTypeFare) const
{
  const uint16_t numSeatsRequired = PaxTypeUtil::numSeatsForFare(trx, paxTypeFare);

  return std::any_of(cosVec.begin(),
                     cosVec.end(),
                     [&tvlSeg, numSeatsRequired](const ClassOfService* const cs)
                     { return cs->isAvailable(tvlSeg.getBookingCode(), numSeatsRequired); });
}

void
AdvanceResTkt::getCurrentRebookStatus(const PaxTypeFare& paxTypeFare,
                                      FareUsage* fu,
                                      std::map<const TravelSeg*, bool>& segsRebookStatus) const
{
  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());

  PaxTypeFare::SegmentStatusVec& segStatVec =
      getSegStatusVec(const_cast<PaxTypeFare&>(paxTypeFare), fu);

  // segmentStatus must have same size of fareMarket travel segments.
  // otherwise, the vector is not initialized right, we should do nothing
  if (segStatVec.size() != fareMarket.travelSeg().size())
    return;

  uint16_t airIndex = 0;

  for (const TravelSeg* tvlSeg : fareMarket.travelSeg())
  {
    if (segStatVec[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      // already rebooked
      segsRebookStatus[tvlSeg] = false;
    ++airIndex;
  }
}

void
AdvanceResTkt::getCurrentRebookStatus(const FarePath& farePath,
                                      std::map<const TravelSeg*, bool>& segsRebookStatus) const
{
  // get rebook status from all PaxTypeFare in the FarePath
  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      getCurrentRebookStatus(*(fareUsage->paxTypeFare()), fareUsage, segsRebookStatus);
      // ignore the failure from the call
    }
  }
}

bool
AdvanceResTkt::isFlownSectorForNewItin(const PricingTrx& trx, const TravelSeg& seg) const
{
  return RexBaseTrx::isRexTrxAndNewItin(trx) && !seg.unflown();
}

Record3ReturnTypes
AdvanceResTkt::updateRebookStatus(PricingTrx& trx,
                                  const Itin* itin,
                                  PaxTypeFare& paxTypeFare,
                                  FareUsage* fu,
                                  const std::map<const TravelSeg*, bool>& segsRebookStatus) const
{
  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
  const FareMarket* flowFareMarket = nullptr;
  std::vector<ClassOfService*>* flowAvail = nullptr;

  PaxTypeFare::SegmentStatusVec& segStatVec = getSegStatusVec(paxTypeFare, fu);

  PaxTypeFare::SegmentStatusVec& segStatVecRule2 = getSegStatusVecRule2(paxTypeFare, fu);

  uint16_t rule2Size = segStatVecRule2.size();

  // segmentStatus must have same size of fareMarket travel segments.
  // otherwise, the vector is not initialized right, we should do nothing
  if (segStatVec.size() != fareMarket.travelSeg().size())
  {
    return SKIP;
  }

  std::vector<TravelSeg*>::const_iterator tvlSegI = fareMarket.travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEndI = fareMarket.travelSeg().end();
  uint16_t airIndex = 0;
  uint16_t cosVecSize = fareMarket.classOfServiceVec().size();
  bool availCheckPass = false;
  bool flowAvailCheckPass = false;
  bool considerJourney = false;

  for (; tvlSegI != tvlSegEndI; tvlSegI++, airIndex++)
  {
    if (isFlownSectorForNewItin(trx, **tvlSegI))
      continue;

    considerJourney = false;
    availCheckPass = false;
    if (segStatVec[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      if (partOfLocalJourney(trx, *tvlSegI) && rule2Size > 0 && airIndex < rule2Size)
      {
        if (!segStatVecRule2[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
            !rebookNotRequired(segsRebookStatus, *tvlSegI))
        {
          setRebook(*(*tvlSegI), segStatVecRule2[airIndex]);
        }
      }
      continue;
    }

    if (rebookNotRequired(segsRebookStatus, *tvlSegI))
      continue;

    // need newly rebook
    if (trx.getRequest()->isLowFareNoAvailability()) // WPNCS
    {
      setRebook(*(*tvlSegI), segStatVec[airIndex]);
      continue;
    }

    if (airIndex >= cosVecSize)
      return FAIL;

    if (fareMarket.classOfServiceVec()[airIndex] == nullptr)
      return FAIL;

    availCheckPass =
        checkAvail(*(*tvlSegI), *(fareMarket.classOfServiceVec()[airIndex]), trx, paxTypeFare);
    if (availCheckPass)
      setRebook(*(*tvlSegI), segStatVec[airIndex]);

    if (partOfLocalJourney(trx, *tvlSegI) && rule2Size > 0 && airIndex < rule2Size)
    {
      considerJourney = true;
      if (!segStatVecRule2[airIndex]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
      {
        considerJourney = false;
        flowFareMarket = getFlowMarket(trx, itin, *tvlSegI);
        if (flowFareMarket != nullptr)
        {
          flowAvail = getFlowAvail(*tvlSegI, flowFareMarket);
          if (flowAvail != nullptr)
          {
            flowAvailCheckPass = checkAvail(*(*tvlSegI), *flowAvail, trx, paxTypeFare);
            if (flowAvailCheckPass)
            {
              setRebook(*(*tvlSegI), segStatVecRule2[airIndex]);
              considerJourney = true;
            }
          }
        }
      }
    }

    if (!availCheckPass)
    {
      if (!considerJourney)
        return FAIL;
    }
  } // for loop for each travelSeg

  return PASS;
}

bool
AdvanceResTkt::updateRebookStatus(PricingTrx& trx,
                                  const FarePath& farePath,
                                  FareUsage& fuInProcess,
                                  const std::map<const TravelSeg*, bool>& segsRebookStatus) const
{
  // get rebook status from all PaxTypeFare in the FarePath
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pu->fareUsage())
    {
      if (trx.getRequest()->originBasedRTPricing() && fareUsage->paxTypeFare()->isDummyFare())
        continue;

      if (updateRebookStatus(
              trx, farePath.itin(), *(fareUsage->paxTypeFare()), fareUsage, segsRebookStatus) ==
          FAIL)
      {
        // the indicator failedCat5InAnotherFu must be set only if the FaqreUsage we are
        // processing right is is failing because of segment status or availability in another
        // Fare Usage
        if (&fuInProcess != fareUsage)
          fuInProcess.failedCat5InAnotherFu() = true;

        // should be that NCB failed availability
        return false;
      }
    }
  }
  return true;
}

void
AdvanceResTkt::updateBookingDate(DateTime& bookingDT, PricingTrx& trx) const
{
  if (UNLIKELY(isHistorical(trx) && bookingDT > trx.ticketingDate()))
  {
    bool overrideBkgDate = trx.isNotExchangeTrx();
    if (!overrideBkgDate && trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
    {
      if ((static_cast<ExchangePricingTrx&>(trx)).reqType() == AGENT_PRICING_MASK && _itin &&
          _itin->validatingCarrier().equalToConst("WN"))
        overrideBkgDate = true;
    }
    if (overrideBkgDate)
      bookingDT = trx.ticketingDate();
  }
}

void
AdvanceResTkt::getBookingDate(PricingTrx& trx,
                              const TravelSeg& tvlSeg,
                              DateTime& bookingDT,
                              bool& isRebooked,
                              std::map<const TravelSeg*, bool>* segsBkgStat) const
{
  bool isWQTrx = trx.noPNRPricing();

  if (!isRebooked && segsBkgStat)
  {
    if (segsBkgStat->find(&tvlSeg) != segsBkgStat->end())
    {
      isRebooked = true;
    }
  }

  if (UNLIKELY(!_tktFromDateOverride.isEmptyDate()))
  {
    bookingDT = _tktFromDateOverride;
    return;
  }

  if (isRebooked && !isWQTrx)
  {
    // When the command is WPNCS and WPNC for low fare search, we would use
    // bookingDT same as ticketingDT, but if ticketingDT is overrided as
    // future date, we would use local time. Because we do not have agent
    // time zone information, it is the safest way now.
    bookingDT = TrxUtil::getTicketingDTCat5(trx);
    updateBookingDate(bookingDT, trx);

    const bool isTktDTOverride = (bookingDT.totalSeconds() == 60);
    // hard-coded here, do not want to but it is the only way of
    // knowing we are having ticketing date override, in which case
    // 'B01OCT05 ticket date would be 01/10/2005-00:01:00

    if (UNLIKELY(isTktDTOverride))
    {
      DateTime today = trx.transactionStartTime();
      if (bookingDT > today)
      {
        // means that we intended to ticketing at time in future
        bookingDT = today; // w/o knowing agent local time zone,
        // this is best we can do
      }
    }
  }
  else
  {
    bookingDT = tvlSeg.bookingDT();
    updateBookingDate(bookingDT, trx);
  }
}

Record3ReturnTypes
AdvanceResTkt::validateResTkt(const DateTime& bookingDT,
                              const DateTime& tktDT,
                              bool needResDTCheck,
                              bool needTktDTCheck,
                              const AdvResTktInfo& advanceResTktInfo,
                              bool needResTktSameDayChk,
                              bool needResTktSameTimeChk,
                              const DateTime& refOrigDT,
                              const DateTime& refStartDT,
                              const DateTime& refEndDT,
                              PricingUnit* pricingUnit,
                              DiagManager& diag,
                              bool& mayPassAfterRebook,
                              bool skipResPermittedChk,
                              const bool isWQTrx,
                              bool& displWqWarning1,
                              const DateTime& adjustedDateResRestriction,
                              PricingTrx& trx) const
{
  const bool autoPassResDT = isWQTrx;

  if (needResDTCheck)
  {
    if (UNLIKELY(true == autoPassResDT))
    {
      diag << " ADVANCE RES: AUTOPASS \n";
    }

    // Check if advance reservation is permitted
    if (advanceResTktInfo.permitted() == notPermitted)
    {
      if (!skipResPermittedChk)
      {
        // Reservations must be at same day of departure of the originating flight of the fare
        // component
        if (isValidATSEDate(bookingDT) && (bookingDT.date() != refOrigDT.date()))
        {
          mayPassAfterRebook = true;

          if (true == autoPassResDT)
          {
            displWqWarning1 = true;
          }
          else
          {
            diag << " ADVANCE RESTKT: FAIL "
                 << "- RES ONLY PERMITTED ON SAME DAY OF ORIG\n";

            return FAIL; // Yes, fail this fare
          }
        }
      }
      notifyObservers(LAST_DATE_TO_BOOK, refOrigDT.date());
    }
    else if (LIKELY(isValidATSEDate(bookingDT)))
    {
      // Validate reservation
      DateTime startDT = refStartDT;
      DateTime endDT = refEndDT;
      std::string diagFailMsg;

      if (fallback::rexCat5APO41906(&trx))
      {
        if (UNLIKELY(!adjustedDateResRestriction.isEmptyDate()))
        {
          int64_t diffTimeSecond = DateTime::diffTime(DateTime(adjustedDateResRestriction.date()),
                                                      DateTime(tktDT.date()));
          if (diffTimeSecond < 0)
            diffTimeSecond = 0; // Reissue date is after Orginal Tkt date

          if (diffTimeSecond > 0)
          {
            startDT = startDT.addSeconds(diffTimeSecond);
            endDT = endDT.addSeconds(diffTimeSecond);
          }

          if (UNLIKELY(diag.isActive()))
          {
            diag << "THE ADJUSTED DEPARTURE DATE: " << startDT.toIsoExtendedString() << "\n";
          }
          diagFailMsg = " ADVANCE RESTKT: FAIL - ADJUSTED NOT BETWEEN RESERVATION TIME PERMITTED\n";
        }
        else
        {
          diagFailMsg = " ADVANCE RESTKT: FAIL - NOT BETWEEN RESERVATION TIME PERMITTED\n";
        }
      }

      if (validateAdvanceResTime(
              bookingDT, startDT, endDT, advanceResTktInfo, diag, mayPassAfterRebook) == FAIL)
      {
        if (UNLIKELY(autoPassResDT))
        {
          displWqWarning1 = true;
        }
        else
        {
          diag << diagFailMsg;
          return FAIL;
        }
      }
    }

    if (LIKELY(false == autoPassResDT))
    {
      diag << " ADVANCE RES: PASS \n";
    }
  } // needResDTCheck

  if (needTktDTCheck)
  {
    DateTime departureDT = (_tktToDateOverride.isEmptyDate() ? refEndDT : _tktToDateOverride);
    DateTime ticketDT;
    if(!pricingUnit || !pricingUnit->volChangesAdvResOverride())
    {
      ticketDT =
          (_pricingTrx && RexPricingTrx::isRexTrxAndNewItin(*_pricingTrx)
               ? determineTicketDateForRex(
                     static_cast<const RexPricingTrx&>(*_pricingTrx), pricingUnit, tktDT, departureDT)
               : determineTicketDate(tktDT, departureDT));
    }
    else
    {
      ticketDT = pricingUnit->volChangesAdvResOverride()->fromDate();
    }

    // Ticketing
    Record3ReturnTypes resultValidation = validateAdvanceTktTime(
        pricingUnit, ticketDT, bookingDT, departureDT, advanceResTktInfo, diag, mayPassAfterRebook);

    displWqWarning1 = true;

    if (resultValidation == FAIL)
    {
      diag << " ADVANCE RESTKT: FAIL "
           << "- NOT BETWEEN TICKETING TIME PERMITTED\n";

      return FAIL;
    }
    else if (resultValidation == SKIP)
    {
      diag << " ADVANCE RESTKT: SKIP\n";
      return SKIP;
    }
    else if (UNLIKELY(resultValidation == NOTPROCESSED))
    {
      needTktDTCheck = false; // no further ticket restriction check any more
    }

    if (LIKELY(!TrxUtil::isPricingInfiniCat05ValidationSkipActivated(trx)))
      diag << " ADVANCE TKT: PASS\n";
  }

  // If Res and Ticketing need to be same time
  if (needResTktSameDayChk || needResTktSameTimeChk)
  {
    DateTime latestTktTime;

    // use pricingDT for ttl if bookingDateValidation is set
    DateTime cat05BookingDT = bookingDT;

    if (UNLIKELY((_itin) && (_itin->cat05BookingDateValidationSkip())))
    {
      cat05BookingDT = TrxUtil::getTicketingDTCat5(trx);
      (const_cast<AdvanceResTkt*>(this))->setcat05overrideDisplayFlag();
      if (pricingUnit != nullptr)
        pricingUnit->isOverrideCxrCat05TktAftRes() = true;
    }

    if (needResTktSameDayChk)
    {
      if (UNLIKELY(tktDT.date() > cat05BookingDT.date()))
      {
        if (UNLIKELY(diag.isActive()))
        {
          diag << " ADVANCE RESTKT: FAIL - TKT DATE NOT SAME AS RES\n";
        }
        mayPassAfterRebook = true;
        return FAIL;
      }
      latestTktTime = DateTime(cat05BookingDT.date(), 23, 59, 0);
    }
    else
    {
      // needResTktSameTimeChk
      latestTktTime = cat05BookingDT + SAMETIME_WINDOW;
      if (tktDT > latestTktTime)
      {
        diag << " ADVANCE RESTKT: FAIL - TKT TIME NOT WITHIN 30MIN RES TIME\n";
        mayPassAfterRebook = true;
        return FAIL;
      }
    }

    if (pricingUnit != nullptr && !pricingUnit->dynamicValidationPhase())
    {
      pricingUnit->latestTktDT().setWithEarlier(latestTktTime);
    }

    // We want to be consistent with Cat5 Data Application
    notifyObservers(LATEST_TKT_DAY, cat05BookingDT);
    notifyObservers(LAST_DATE_TO_BOOK, cat05BookingDT);
  }
  if (UNLIKELY(TrxUtil::isPricingInfiniCat05ValidationSkipActivated(trx)))
  {
    if (_cat05overrideDisplayFlag)
    {
      diag << " ADVANCE TKT: PASS - ";
      displayCat05OverrideDataToDiag(diag);
    }
    else
      diag << " ADVANCE TKT: PASS\n";
  }
  return PASS;
}

const DateTime&
AdvanceResTkt::determineTicketDate(const DateTime& ticketDate, const DateTime& departureDate) const
{
  if (UNLIKELY(_pricingTrx && (_pricingTrx->excTrxType() == PricingTrx::PORT_EXC_TRX ||
                      (_pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX &&
                       !(static_cast<const RexPricingTrx*>(_pricingTrx)->isAnalyzingExcItin()))) &&
      ticketDate > departureDate))
  {
    return departureDate;
  }

  return ticketDate;
}

void
AdvanceResTkt::setRebook(const TravelSeg& tvlSeg, PaxTypeFare::SegmentStatus& segStat) const
{
  segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
  segStat._bkgCodeReBook = tvlSeg.getBookingCode();
  segStat._reBookCabin = tvlSeg.bookedCabin();
}

bool
AdvanceResTkt::partOfLocalJourney(PricingTrx& trx, TravelSeg* tvlSeg) const
{
  if (trx.getTrxType() != PricingTrx::PRICING_TRX)
    return false;

  if (!(trx.getOptions()->journeyActivatedForPricing()))
    return false;

  if (!(trx.getOptions()->applyJourneyLogic()))
    return false;

  AirSeg* airSeg = tvlSeg->toAirSeg();
  if (!airSeg)
    return false;

  if (!airSeg->localJourneyCarrier())
    return false;

  if (!trx.getRequest()->isLowFareRequested())
    return false;

  Itin& itin = *(trx.itin()[0]);

  return itin.hasTvlSegInFlowFareMarkets(tvlSeg);
}

bool
AdvanceResTkt::rebookNotRequired(const std::map<const TravelSeg*, bool>& segsRebookStatus,
                                 const TravelSeg* tvlSeg) const
{
  const auto segBkgStatIter = segsRebookStatus.find(tvlSeg);
  return (segBkgStatIter == segsRebookStatus.end() || !segBkgStatIter->second);
}

const FareMarket*
AdvanceResTkt::getFlowMarket(PricingTrx& trx, const Itin* itinPtr, TravelSeg* tvlSeg) const
{
  const Itin* const itin = trx.getOptions()->isCarnivalSumOfLocal() ? itinPtr : trx.itin().front();
  return JourneyUtil::getFlowMarketFromSegment(tvlSeg, itin);
}

std::vector<ClassOfService*>*
AdvanceResTkt::getFlowAvail(TravelSeg* tvlSeg, const FareMarket* fm) const
{
  uint16_t cosIndex = 0;
  for (TravelSeg* travelSeg : fm->travelSeg())
  {
    if (travelSeg->isAir() && tvlSeg == travelSeg && cosIndex < fm->classOfServiceVec().size())
      return fm->classOfServiceVec()[cosIndex];
    ++cosIndex;
  }
  return nullptr;
}

int
AdvanceResTkt::getEligibleTktDates(PricingTrx& trx,
                                   const FareMarket& fm,
                                   const PricingUnit* pu,
                                   DateTime* tktDate) const
{
  int numOfTktDT = 1;
  tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
  bool optionA = (trx.getOptions()->AdvancePurchaseOption() == ' ' ||
                  trx.getOptions()->AdvancePurchaseOption() == 'F');
  bool optionB = (trx.getOptions()->AdvancePurchaseOption() == 'T');
  bool tvlSegChange = false;
  bool excTrxValid = false;
  bool arTrxValid = false;

  ExchangePricingTrx* excTrx = nullptr;
  RexPricingTrx* arTrx = nullptr;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    excTrx = static_cast<ExchangePricingTrx*>(&trx);

    if (excTrx->reqType() != AGENT_PRICING_MASK && excTrx->reqType() != MULTIPLE_EXCHANGE &&
        !excTrx->exchangeItin().empty() && (optionA || optionB))
      excTrxValid = true;
  }

  else if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    arTrx = static_cast<RexPricingTrx*>(&trx);

    if (arTrx->reqType() == AUTOMATED_REISSUE && !arTrx->exchangeItin().empty())
      arTrxValid = true;
  }

  if (UNLIKELY(excTrxValid && excTrx->reqType() != AGENT_PRICING_MASK &&
      trx.getRequest()->ticketingAgent() &&
      !trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()))
  {
    tktDate[0] = excTrx->purchaseDT();
    return 1;
  }

  else if (UNLIKELY(excTrxValid || arTrxValid))
  {
    const std::vector<ExcItin*>& excItins =
        (excTrxValid ? excTrx->exchangeItin() : arTrx->exchangeItin());

    const std::vector<Itin*>& newItins = (excTrxValid ? excTrx->newItin() : arTrx->newItin());

    const DateTime& originalTktIssueDT =
        (excTrxValid ? excTrx->originalTktIssueDT() : arTrx->originalTktIssueDT());

    if (&fm != nullptr)
      tvlSegChange = (checkTravelSegChange(fm));
    if (optionA) // Option A
    {
      const Itin* newItin =
          (trx.getTrxType() == PricingTrx::MIP_TRX && pu) ? arTrx->curNewItin() : newItins.front();

      bool USCAitin =
          (newItin->geoTravelType() == GeoTravelType::Domestic || newItin->geoTravelType() == GeoTravelType::Transborder);

      bool destinationChange = false;
      TravelSeg* excLastTvl = excItins.front()->travelSeg().back();
      TravelSeg* itinLastTvl = newItin->travelSeg().back();

      if (excLastTvl->destination()->loc() != itinLastTvl->destination()->loc())
        destinationChange = true;

      if (pu == nullptr) // fare component level
      {
        if ((fm.direction() == FMDirection::OUTBOUND && tvlSegChange) ||
            (destinationChange && USCAitin))
        {
          tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
          numOfTktDT = 1;
        }
        else if ((fm.direction() != FMDirection::INBOUND) &&
                 ((!tvlSegChange && !excItins.front()->stopOverChange() && USCAitin) ||
                  (!tvlSegChange && !USCAitin)))
        {
          tktDate[0] = originalTktIssueDT;
          numOfTktDT = 1;
        }
        else
        {
          tktDate[0] = originalTktIssueDT;
          tktDate[1] = TrxUtil::getTicketingDTCat5(trx);
          numOfTktDT = 2;
        }
      }
      else // PU level
      {
        bool travelPRVI = false;
        bool fareBreakChanged = false;

        FareUsage* fUsage = (pu->fareUsage().front());
        TravelSeg* tvlSeg = *fUsage->travelSeg().begin();

        if ((tvlSeg->changeStatus() == TravelSeg::CHANGED) ||
            (tvlSeg->changeStatus() == TravelSeg::INVENTORYCHANGED))
        {
          tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
          numOfTktDT = 1;
          return (numOfTktDT);
        }

        if (USCAitin)
        {
          std::vector<FareCompInfo*>::const_iterator fcIter =
              excItins.front()->fareComponent().begin();
          const std::vector<FareCompInfo*>::const_iterator fcIterEnd =
              excItins.front()->fareComponent().end();

          for (FareUsage* fareUsage : pu->fareUsage())
          {
            for (TravelSeg* tvlSeg : fareUsage->travelSeg())
            {
              if ((tvlSeg->origin()->state() == PUERTORICO) ||
                  (tvlSeg->origin()->state() == VIRGIN_ISLAND) ||
                  (tvlSeg->destination()->state() == PUERTORICO) ||
                  (tvlSeg->destination()->state() == VIRGIN_ISLAND))
              {
                travelPRVI = true;
                break;
              }
            }

            if (travelPRVI)
              break;

            // Check for changes in farebreakes
            if (!fareBreakChanged)
            {
              fareBreakChanged = true;

              for (; fcIter != fcIterEnd; ++fcIter)
              {
                if ((fareUsage->travelSeg().front()->origAirport() ==
                     (*fcIter)->fareMarket()->travelSeg().front()->origAirport()) &&
                    (fareUsage->travelSeg().back()->destAirport() ==
                     (*fcIter)->fareMarket()->travelSeg().back()->destAirport()))
                {
                  fareBreakChanged = false;
                  break;
                }
              }
            }
          }
        }
        if (!USCAitin || travelPRVI)
        {
          tktDate[0] = getPrevExchangeOrOriginTktIssueDT(trx); // use original ticket issue date
          numOfTktDT = 1;
          return (numOfTktDT);
        }
        if (destinationChange)
        {
          tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
          numOfTktDT = 1;
          return (numOfTktDT);
        }
        else if (!excItins.front()->stopOverChange())
        {
          tktDate[0] = getPrevExchangeOrOriginTktIssueDT(trx); // use original ticket issue date
          numOfTktDT = 1;
          return (numOfTktDT);
        }
        else if (fareBreakChanged) //(excItins.front()->stopOverChange())?
        {
          tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
          numOfTktDT = 1;
          return (numOfTktDT);
        }
        else
        {
          tktDate[0] = getPrevExchangeOrOriginTktIssueDT(trx); // use original ticket issue date
          numOfTktDT = 1;
          return (numOfTktDT);
        }
      }
    }
    else if (optionB)
    {
      if (!tvlSegChange)
        tktDate[0] = getPrevExchangeOrOriginTktIssueDT(trx); // use original ticket issue date
    }
    else // CAT 31 option PCG = N
    {
      if (pu && !_tktFromDateOverride.isEmptyDate())
      {
        tktDate[0] = _tktFromDateOverride;
      }
    }
  }
  else
  {
    tktDate[0] = TrxUtil::getTicketingDTCat5(trx);
    numOfTktDT = 1; // not Exchange Trx
  }
  return (numOfTktDT);
}


struct IsChanged : public std::unary_function<const TravelSeg*, bool>
{
  bool operator()(const TravelSeg* seg) const
  {
    return seg->changeStatus() == TravelSeg::CHANGED ||
           seg->changeStatus() == TravelSeg::INVENTORYCHANGED;
  }
};

bool
AdvanceResTkt::checkTravelSegChange(const FareMarket& fm) const
{
  return std::any_of(fm.travelSeg().cbegin(), fm.travelSeg().cend(), IsChanged());
}

void
AdvanceResTkt::initialize(const PricingTrx& trx,
                          const AdvResTktInfo& advResTktInfo,
                          const PaxTypeFare& paxTypeFare,
                          const PricingUnit* pricingUnit,
                          const Itin* itin)
{
  _pricingTrx = &trx;
  _newTicketIssueDate = DateTime::emptyDate();
  _itin = itin;

  if (LIKELY(itin))
  {
    _isPTFBeginJxr = (paxTypeFare.fareMarket()->travelSeg().front() == itin->travelSeg().front());
  }

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
      (trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
       (!pricingUnit || !pricingUnit->volChangesAdvResOverride()))))
  {
    if (itin)
    {
      FlownStatusCheck flStatChk(*itin);
      if (flStatChk.isPartiallyFlown())
        _ignoreTktAfterResRestriction = true;
      else if (flStatChk.isTotallyUnflown())
      {
        if (!pricingUnit)
        {
          std::vector<TravelSeg*>::const_iterator posOfSeg =
              find(itin->travelSeg().begin(),
                   itin->travelSeg().end(),
                   paxTypeFare.fareMarket()->travelSeg().front());
          if (posOfSeg == itin->travelSeg().end())
            _ignoreTktAfterResRestriction = true;
          else
          {
            ++posOfSeg;
            // any sector from first of itin till first of Fare Market is not changed, we can ignore
            // at FC as it might be first of PU;
            // if all are changed, we are sure that first sector of PU is changed, so we can not
            // ignore
            _ignoreTktAfterResRestriction =
                (std::find_if(itin->travelSeg().begin(), posOfSeg, std::not1(IsChanged())) !=
                 posOfSeg);
          }
        }
        else
        {
          if (!(pricingUnit->travelSeg().front()->changeStatus() == TravelSeg::CHANGED ||
                pricingUnit->travelSeg().front()->changeStatus() == TravelSeg::INVENTORYCHANGED))
            _ignoreTktAfterResRestriction = true;
        }
      }
    }
  }

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    if (trx.getOptions()->AdvancePurchaseOption() == 'T' && (0 == advResTktInfo.tktTSI()))
    {
      const auto& tvlSegVector = paxTypeFare.fareMarket()->travelSeg();
      if (std::any_of(tvlSegVector.begin(), tvlSegVector.end(), IsChanged()))
        _tktToDateOverride = paxTypeFare.fareMarket()->travelSeg().front()->departureDT();
    }
  }
  else if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX && pricingUnit))
  {
    _diagFromToDate = true;

    const AdvResOverride* advResOverride = pricingUnit->volChangesAdvResOverride();

    if (advResOverride)
    {
      // call from RexAdvResTktValidator !
      const ReissueSequenceW* reissueSequence = advResOverride->reissueSequence();

      // always present and not empty
      _tktToDateOverride = advResOverride->toDate();
      _tktFromDateOverride = advResOverride->fromDate();

      if (reissueSequence) // means succesfully mapped FC
      {
        _advTktBoth = advResOverride->reissueSequence()->departureInd();

        if (advResOverride->ignoreTktAfterResRestriction())
          _ignoreTktAfterResRestriction = true;
        else
        {
          _advTktTod = advResOverride->reissueSequence()->reissueTOD();
          _advTktPeriod = advResOverride->reissueSequence()->reissuePeriod();
          _advTktUnit = advResOverride->reissueSequence()->reissueUnit();
        }

        if (advResOverride->ignoreTktDeforeDeptRestriction())
          _ignoreTktDeforeDeptRestriction = true;
        else
        {
          _advTktOpt = advResOverride->reissueSequence()->optionInd();
          _advTktDepart = advResOverride->reissueSequence()->departure();
          _advTktDepartUnit = advResOverride->reissueSequence()->departureUnit();
        }
      }
    }
  }
  else
  {
    if (pricingUnit && advResTktInfo.eachSector() != notApply)
    {
      if (advResTktInfo.tktTSI() == 0)
      {
        _tktToDateOverride = pricingUnit->travelSeg().front()->departureDT();
      }
    }
  }
}

PaxTypeFare::SegmentStatusVec&
AdvanceResTkt::getSegStatusVec(PaxTypeFare& paxTypeFare, FareUsage* fu) const
{
  if (fu != nullptr)
  {
    if (fu->segmentStatus().empty())
      fu->segmentStatus().insert(fu->segmentStatus().end(),
                                 fu->paxTypeFare()->segmentStatus().begin(),
                                 fu->paxTypeFare()->segmentStatus().end());
    return fu->segmentStatus();
  }
  return paxTypeFare.segmentStatus();
}

PaxTypeFare::SegmentStatusVec&
AdvanceResTkt::getSegStatusVecRule2(PaxTypeFare& paxTypeFare, FareUsage* fu) const
{
  if (fu != nullptr)
  {
    if (fu->segmentStatusRule2().empty())
      fu->segmentStatusRule2().insert(fu->segmentStatusRule2().end(),
                                      fu->paxTypeFare()->segmentStatusRule2().begin(),
                                      fu->paxTypeFare()->segmentStatusRule2().end());
    return fu->segmentStatusRule2();
  }
  return paxTypeFare.segmentStatusRule2();
}

bool
AdvanceResTkt::isHistorical(PricingTrx& trx) const
{
  if (UNLIKELY(trx.dataHandle().isHistorical()))
    return true;
  return TrxUtil::isHistorical(trx);
}

// We did not delete directly on original WrapperVector
//
// return true   if found Travel Segment belonging to a Keep Fare that is set to ignore
//               cat5 validation, and save the rest Travel Segments into a new
//               WrapperVector, this function would not change the original WrapperVector
//
//        false  if not found Travel Segment belonging to a Keep Fare that is set to
//               ignore cat5 validation, the original WrapperVector should be used
bool
AdvanceResTkt::deleteTvlSegIgnoreCat5(PricingTrx& trx,
                                      const PricingUnit* pricingUnit,
                                      const RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                      RuleUtil::TravelSegWrapperVector& newApplTvlSegs) const
{
  if (LIKELY(!pricingUnit ||
      (!RexPricingTrx::isRexTrxAndNewItin(trx) &&
       (trx.excTrxType() != PricingTrx::PORT_EXC_TRX ||
        TAG_10_EXCHANGE != (static_cast<const ExchangePricingTrx&>(trx)).reqType()))))
    return false;

  std::vector<TravelSeg*> ignoredTvlSegs;
  for (FareUsage* fu : pricingUnit->fareUsage())
  {
    if (fu->categoryIgnoredForKeepFare().find(RuleConst::ADVANCE_RESERVATION_RULE) !=
        fu->categoryIgnoredForKeepFare().end())
    {
      ignoredTvlSegs.insert(ignoredTvlSegs.end(), fu->travelSeg().begin(), fu->travelSeg().end());
    }
    else
    {
      if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
      {
        const ExchangePricingTrx& excTrx = static_cast<const ExchangePricingTrx&>(trx);
        if (excTrx.reqType() == TAG_10_EXCHANGE)
        {
          if (fu->paxTypeFare()->isDummyFare())
          {
            ignoredTvlSegs.insert(
                ignoredTvlSegs.end(), fu->travelSeg().begin(), fu->travelSeg().end());
          }
        }
      }
    }
  }
  if (ignoredTvlSegs.empty())
    return false;

  for (auto& applTvlSeg : applTravelSegment)
  {
    if (std::find(ignoredTvlSegs.begin(), ignoredTvlSegs.end(), applTvlSeg->travelSeg()) ==
        ignoredTvlSegs.end())
      newApplTvlSegs.push_back(applTvlSeg);
  }

  return true;
}

const DateTime&
AdvanceResTkt::getPrevExchangeOrOriginTktIssueDT(const PricingTrx& trx) const
{
  const BaseExchangeTrx& exchTrx = static_cast<const BaseExchangeTrx&>(trx);
  if (exchTrx.applyReissueExchange() && !exchTrx.previousExchangeDT().isEmptyDate())
    return exchTrx.previousExchangeDT();
  else
    return exchTrx.originalTktIssueDT();
}

const DateTime&
AdvanceResTkt::determineTicketDateForRex(const RexPricingTrx& rexTrx,
                                         bool isPricingUnit,
                                         const DateTime& tktDT,
                                         const DateTime& departureDT) const
{
  if (!isPricingUnit)
    return _newTicketIssueDate.isEmptyDate() ? rexTrx.originalTktIssueDT() : _newTicketIssueDate;

  const DateTime& tktDate = _newTicketIssueDate.isEmptyDate() ? tktDT : _newTicketIssueDate;
  const DateTime& issueDate =
      ((rexTrx.applyReissueExchange() && tktDate == rexTrx.originalTktIssueDT())
           ? getPrevExchangeOrOriginTktIssueDT(rexTrx)
           : tktDate);
  return determineTicketDate(issueDate, departureDT);
}

const Loc*
AdvanceResTkt::getCommonReferenceLoc(const PricingTrx& trx)
{
  DateTime localTime = DateTime::localTime();
  if (trx.getRequest()->PricingRequest::salePointOverride().size())
    return trx.dataHandle().getLoc(trx.getRequest()->PricingRequest::salePointOverride(),
                                   localTime);
  else
    return trx.getRequest()->ticketingAgent()->agentLocation();
}

short
AdvanceResTkt::checkUTC(const PricingTrx& trx, const TravelSeg& tvlSeg) const
{
  const LocCode& tvlLocT = tvlSeg.origAirport();
  DateTime localTime = DateTime::localTime();
  short utcOffset = 0;
  const Loc* saleLoc = getCommonReferenceLoc(trx);
  const Loc* tvlLoc = trx.dataHandle().getLoc(tvlLocT, localTime);

  if (LIKELY(saleLoc && tvlLoc))
  {
    if (UNLIKELY(!LocUtil::getUtcOffsetDifference(
            *tvlLoc, *saleLoc, utcOffset, trx.dataHandle(), localTime, localTime)))
      utcOffset = 0;
  }
  return utcOffset;
}

template <typename T>
class AdvanceResTktObservers
{
public:
  AdvanceResTktObservers(T& object, PricingTrx& trx, AdvanceResTkt& advanceResTkt)
    : _object(&object)
  {
    if (trx.getRequest() && trx.getRequest()->isSFR())
    {
      _objectObservers.push_back(AdvanceResAndTktObserverType<T>::create(
          ADVANCE_RESERVATION_SFR, trx.dataHandle(), &advanceResTkt));
      _objectObservers.push_back(AdvanceResAndTktObserverType<T>::create(
          ADVANCE_TICKETING_SFR, trx.dataHandle(), &advanceResTkt));
    }
  }

  void updateIfNotified()
  {
    for (auto& observer : _objectObservers)
      observer->updateIfNotified(*_object);
  }

private:
  std::vector<std::unique_ptr<AdvanceResAndTktObserverType<T>>> _objectObservers;
  T* _object = nullptr;
};

// Called in FVO
Record3ReturnTypes
AdvanceResTktWrapper::validate(PricingTrx& trx,
                               Itin& itin,
                               PaxTypeFare& paxTypeFare,
                               const RuleItemInfo* rule,
                               const FareMarket& fareMarket)
{
  AdvanceResTktObservers<PaxTypeFare> observers(paxTypeFare, trx, _advanceResTkt);
  const Record3ReturnTypes retval =
      _advanceResTkt.validate(trx, itin, paxTypeFare, rule, fareMarket);

  if (retval == PASS)
    observers.updateIfNotified();

  return retval;
}

Record3ReturnTypes
AdvanceResTktWrapper::validate(PricingTrx& trx,
                               const RuleItemInfo* rule,
                               const FarePath& farePath,
                               const PricingUnit& pricingUnit,
                               FareUsage& fareUsage)
{
  AdvanceResTktObservers<FareUsage> observers(fareUsage, trx, _advanceResTkt);

  const Record3ReturnTypes retval =
      _advanceResTkt.validate(trx, rule, farePath, pricingUnit, fareUsage);

  if (retval == PASS)
    observers.updateIfNotified();

  return retval;
}
} // tse
