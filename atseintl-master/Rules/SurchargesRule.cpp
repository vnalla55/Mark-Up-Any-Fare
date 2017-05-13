//-------------------------------------------------------------------
//
//  File:        SurchargesRule.cpp
//  Created:     August 12, 2004
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//
//  Copyright Sabre 2004
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

#include "Rules/SurchargesRule.h"

#include "BookingCode/FareDisplayBookingCode.h"

#include "Common/CarrierUtil.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RtwUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/SurchargeData.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Nation.h"
#include "DBAccess/SurchargesInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag312Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"


#include <functional>
#include <numeric>

namespace tse
{
FALLBACK_DECL(ssdsp1836surchWithGeo);

namespace
{
class CompareTsw : public std::unary_function<const RuleUtil::TravelSegWrapper*, bool>
{
public:
  bool operator()(const RuleUtilTSI::TravelSegWrapper* tsw)
  {
    if (tsw->travelSeg()->boardMultiCity() == _tsw->travelSeg()->boardMultiCity() &&
        tsw->travelSeg()->offMultiCity() == _tsw->travelSeg()->offMultiCity())
      return true;

    return false;
  }

  CompareTsw(const RuleUtil::TravelSegWrapper* tsw) : _tsw(tsw) {}

private:
  const RuleUtil::TravelSegWrapper* _tsw = nullptr;
};
}

bool
SurchargesRule::isDiagEnabled(PricingTrx& trx, const PaxTypeFare& paxTypeFare, uint32_t itemNo)
{
  const std::string& ruleNumber = trx.diagnostic().diagParamMapItem(Diagnostic::RULE_NUMBER);
  if (trx.diagnostic().shouldDisplay(paxTypeFare, 12) &&
      (ruleNumber.empty() || static_cast<uint32_t>(atoi(ruleNumber.c_str())) == itemNo))
    return true;
  return false;
}

Record3ReturnTypes
SurchargesRule::validate(PricingTrx& trx, const SurchargesInfo& surchInfo, const PaxTypeFare& fare)
{
  DiagManager diag(trx, Diagnostic312);
  const bool dg = diag.isActive() && isDiagEnabled(trx, fare, surchInfo.itemNo());

  if (UNLIKELY(dg))
    static_cast<Diag312Collector&>(diag.collector())
        .diag312Collector(trx, pickProperFare(fare), &surchInfo);

  if (surchInfo.unavailTag() == RuleConst::DATA_UNAVAILABLE)
  {
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - FAIL:"
           << "\n";
      diag << "                     DATA IS UNAVAILABLE"
           << "\n";
    }
    return FAIL;
  }
  else if (surchInfo.unavailTag() == RuleConst::TEXT_ONLY)
  {
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - SKIP:"
           << "\n";
      diag << "                     TEXT ONLY"
           << "\n";
    }
    return SKIP;
  }

  if ((surchInfo.surchargeType() == RuleConst::FUEL) && (trx.getOptions()->ignoreFuelSurcharge()))
  {
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - SKIP:"
           << "\n";
      diag << "                     FUEL SURCHARGE"
           << "\n";
    }
    return SKIP;
  }

  const PaxType& paxType = getPaxType(nullptr, fare);

  if (!validatePaxTypeData(trx,
                           &paxType,
                           fare.vendor(),
                           surchInfo,
                           dg,
                           diag.isActive() ? static_cast<Diag312Collector*>(&diag.collector()) : nullptr))
  {
    return SKIP;
  }

  if (surchInfo.surchargeType() == RuleConst::BLANK ||
      (surchInfo.surchargeType() == RuleConst::EQUIPMENT && surchInfo.equipType().empty()))
  {
    if (UNLIKELY(dg))
      diag << "    SURCHARGE RULE - SKIP BY TYPE"
           << "\n";
    return SKIP;
  }

  if (isNegativeSurcharge(&surchInfo, diag, dg))
    return SKIP;

  if (surchInfo.surchargeType() == RuleConst::SUPERSONIC)
  {
    if (UNLIKELY(dg))
      diag << "    SURCHARGE RULE - SKIP BY C TYPE"
           << "\n";

    return SKIP;
  }

  bool useSectorPortion = (surchInfo.sectorPortion() == SectorPortionSector ||
                           surchInfo.sectorPortion() == SectorPortionPortion);

  if (surchInfo.geoTblItemNoBtw() != 0 && !useSectorPortion)
  {
    if (surchInfo.geoTblItemNoBtw() && !isDataCorrect(surchInfo))
    {
      if (UNLIKELY(dg))
      {
        diag << "    SURCHARGE RULE - SKIP BY SIDETRIP DATA:"
             << "\n";
        diag << "                     INCORRECT RULE DATA "
             << "\n";
      }
      return SKIP;
    }
  }
  else
  {
    //=======================================================
    //  Non_Side_Trip processing
    //=======================================================
    if (useSectorPortion)
    {
      if (surchInfo.geoTblItemNoBtw() == 0 || surchInfo.geoTblItemNoAnd() == 0)
      {
        if (UNLIKELY(dg))
        {
          diag << "    SURCHARGE RULE - SKIP BY 995 TABLE:"
               << "\n";
          if (surchInfo.geoTblItemNoBtw())
            diag << "                     MISSING 995BTW TABLE"
                 << "\n";
          else
            diag << "                     MISSING 995AND TABLE"
                 << "\n";
        }
        return SKIP;
      }
    }
  }

  Diag312Collector* diagPtr = (diag.isActive() && isDiagEnabled(trx, fare, surchInfo.itemNo()))
                                  ? &static_cast<Diag312Collector&>(diag.collector())
                                  : nullptr;

  PaxTypeFare& nonConstPtf = pickProperFare(fare);

  std::vector<SurchargeSeg> surchargeSegments;
  Record3ReturnTypes retValue = getSurchargeSegments(
      surchargeSegments, trx, nullptr, nullptr, nullptr, nonConstPtf, &surchInfo, diagPtr);

  if (retValue != PASS)
  {
    return retValue;
  }

  std::vector<SurchargeData*>& surchargeDataVec = nonConstPtf.surchargeData();

  for (const SurchargeSeg& tsInfo : surchargeSegments)
  {
    SurchargeData* surchargeData = addSurcharge(trx,
                                                nullptr,
                                                fare,
                                                nullptr,
                                                surchInfo,
                                                nullptr,
                                                *tsInfo._ts,
                                                *tsInfo._fcBrdCity,
                                                *tsInfo._fcOffCity,
                                                tsInfo._singleSector);

    if (surchargeData->travelPortion() == RuleConst::PERTICKET ||
        surchargeData->travelPortion() == RuleConst::PERDIRECTION)
    {
      surchargeDataVec.clear();

      if (UNLIKELY(diagPtr))
        *diagPtr << "FC SCOPE VALIDATION SOFTPASS - TRAVEL PORTION";

      return SOFTPASS;
    }

    if (UNLIKELY(diagPtr))
    {
      if (!tsInfo._loc)
        diagPtr->displaySurchargeData(
            trx, surchargeData, surchInfo, fare.isPaperTktSurchargeMayApply());
      else
        diagPtr->displaySideTripSurchargeData(surchargeData, *tsInfo._loc);
    }

    surchargeDataVec.push_back(surchargeData);
  }

  nonConstPtf.needRecalculateCat12() = false;

  nonConstPtf.nucTotalSurchargeFareAmount() =
      std::accumulate(surchargeDataVec.begin(),
                      surchargeDataVec.end(),
                      0.0,
                      [](double sum, const SurchargeData* sd)
                      { return sum + sd->amountNuc(); });

  return PASS;
}

//----------------------------------------------------------------------------
// validate()     Final phase scope
//----------------------------------------------------------------------------
Record3ReturnTypes
SurchargesRule::validate(PricingTrx& trx,
                         FarePath& farePath,
                         const PricingUnit& pricingUnit,
                         FareUsage& fareUsage,
                         const SurchargesInfo* surchInfo)
{
  DiagManager diag(trx, Diagnostic312);

  const bool dg =
      diag.isActive() && isDiagEnabled(trx, *fareUsage.paxTypeFare(), surchInfo->itemNo());
  Diag312Collector* diagPtr = nullptr;
  if (UNLIKELY(dg))
    diagPtr = &static_cast<Diag312Collector&>(diag.collector());

  PaxTypeFare& nonConstPtf = pickProperFare(*fareUsage.paxTypeFare());

  if (UNLIKELY(!nonConstPtf.needRecalculateCat12()))
  {
    if (UNLIKELY(dg))
      diag << " FARE ALREADY PROCESSED IN FARE VALIDATOR PHASE\n";

    return PASS;
  }

  if (UNLIKELY(dg))
    diagPtr->diag312Collector(trx, *fareUsage.paxTypeFare(), surchInfo);

  if (UNLIKELY(surchInfo->unavailTag() == RuleConst::DATA_UNAVAILABLE))
  {
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - FAIL:"
           << "\n";
      diag << "                     DATA IS UNAVAILABLE"
           << "\n";
    }
    return FAIL;
  }
  else if (surchInfo->unavailTag() == RuleConst::TEXT_ONLY)
  {
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - SKIP:"
           << "\n";
      diag << "                     TEXT ONLY"
           << "\n";
    }
    return SKIP;
  }

  if (UNLIKELY((surchInfo->surchargeType() == RuleConst::FUEL) && (trx.getOptions()->ignoreFuelSurcharge())))
  {
    farePath.fuelSurchargeIgnored() = true;
    if (UNLIKELY(dg))
    {
      diag << "    SURCHARGE RULE - SKIP:"
           << "\n";
      diag << "                     FUEL SURCHARGE"
           << "\n";
    }
    return SKIP;
  }

  if (UNLIKELY(!validatePaxTypeData(
          trx, farePath.paxType(), fareUsage.paxTypeFare()->vendor(), *surchInfo, dg, diagPtr)))
  {
    return SKIP;
  }

  if (UNLIKELY(surchInfo->surchargeType() == RuleConst::BLANK ||
      (surchInfo->surchargeType() == RuleConst::EQUIPMENT && surchInfo->equipType().empty())))
  {
    if (UNLIKELY(dg))
      diag << "    SURCHARGE RULE - SKIP BY TYPE"
           << "\n";
    return SKIP;
  }

  if (UNLIKELY(isNegativeSurcharge(surchInfo, diag, dg)))
    return SKIP;

  if (UNLIKELY(surchInfo->surchargeType() == RuleConst::SUPERSONIC))
  ///       surchInfo->surchargeType() == RuleConst::NAVIGATION)
  {
    if (UNLIKELY(dg))
      diag << "    SURCHARGE RULE - SKIP BY C TYPE"
           << "\n";

    return SKIP;
  }

  std::vector<SurchargeSeg> surchargeSegments;
  Record3ReturnTypes retValue = getSurchargeSegments(surchargeSegments,
                                                     trx,
                                                     &farePath,
                                                     &pricingUnit,
                                                     &fareUsage,
                                                     *fareUsage.paxTypeFare(),
                                                     surchInfo,
                                                     diagPtr);

  for (const SurchargeSeg& tsInfo : surchargeSegments)
  {
    SurchargeData* surchargeData = addSurcharge(trx,
                                                &farePath,
                                                *fareUsage.paxTypeFare(),
                                                &fareUsage,
                                                *surchInfo,
                                                nullptr,
                                                *tsInfo._ts,
                                                *tsInfo._fcBrdCity,
                                                *tsInfo._fcOffCity,
                                                tsInfo._singleSector);

    if (UNLIKELY(dg))
    {
      if (!tsInfo._loc)
        diagPtr->displaySurchargeData(
            trx, surchargeData, *surchInfo, fareUsage.isPaperTktSurchargeMayApply());
      else
        diagPtr->displaySideTripSurchargeData(surchargeData, *tsInfo._loc);
    }

    fareUsage.surchargeData().push_back(surchargeData);
  }

  return retValue;
}

Record3ReturnTypes
SurchargesRule::getSurchargeSegments(std::vector<SurchargeSeg>& surchargeSegments,
                                     PricingTrx& trx,
                                     FarePath* farePath,
                                     const PricingUnit* pricingUnit,
                                     FareUsage* fareUsage,
                                     PaxTypeFare& fare,
                                     const SurchargesInfo* surchInfo,
                                     Diag312Collector* diagPtr)
{
  std::vector<TravelSeg*>::const_iterator itB = fare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itE = fare.fareMarket()->travelSeg().end();
  std::vector<TravelSeg*>::const_iterator itL = itE - 1;
  std::vector<TravelSeg*>::const_iterator itBegin = itB;

  _tsi = 0;
  _tsiBtw = 0;
  _tsiAnd = 0;

  // SITA - DEC 1st, 2003
  // If SurchargeType is surface, surcharge applied if there is surface
  // segment (ArunkSeg);
  if (UNLIKELY(surchInfo->surchargeType() == RuleConst::SURFACE))
  {
    for (; itB != itE; ++itB)
    {
      if ((*itB)->segmentType() == Arunk)
      {
        break;
      }
    }
    if (itB == itE)
    {
      if (UNLIKELY(diagPtr))
        *diagPtr << "    SURCHARGE RULE - SKIP BY SURFACE TYPE"
                 << "\n";

      return SKIP;
    }

    // continue to check other fields...
  } // surchargeType == SURFACE

  // If SurchargeType is 'Z' (class of service surcharge), surcharge applied if
  // the governing carrier sector is booked in the specified RBD.

  if (UNLIKELY((surchInfo->surchargeType() == RuleConst::RBD ||
       surchInfo->surchargeType() == RuleConst::BUSINESCLASS ||
       surchInfo->surchargeType() == RuleConst::SLEEPERETTE) &&
      surchInfo->bookingCode().empty()))
  {
    if (UNLIKELY(diagPtr))
    {
      *diagPtr << "    SURCHARGE RULE - SKIP BY RBD TYPE"
               << "\n";
      *diagPtr << "                     RBD IS REQUIRED WITH TYPES Z/B/L\n";
    }
    return SKIP;
  }

  RuleUtil::TravelSegWrapperVector applTravelSegment;

  bool useSectorPortion = (surchInfo->sectorPortion() == SectorPortionSector ||
                           surchInfo->sectorPortion() == SectorPortionPortion);

  if (UNLIKELY(surchInfo->geoTblItemNoBtw() != 0 && !useSectorPortion))
  {
    return validateSideTrip(surchargeSegments,
                            trx,
                            farePath,
                            pricingUnit,
                            fareUsage,
                            fare,
                            surchInfo,
                            applTravelSegment,
                            diagPtr);
  }
  else
  {
    //=======================================================
    //  Non_Side_Trip processing
    //=======================================================
    if (useSectorPortion)
    {
      if (!processSectionPortion(
              trx, farePath, pricingUnit, fareUsage, fare, surchInfo, applTravelSegment, diagPtr))
        return SKIP;
    }
    else if (surchInfo->geoTblItemNo() != 0)
    {
      bool origCheck = true;
      bool destCheck = true;
      bool fltStopCheck = false;

      RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
      _tsi = 0;
      bool retCode995 =
          RuleUtil::validateGeoRuleItem(surchInfo->geoTblItemNo(),
                                        fare.vendor(),
                                        defaultScope,
                                        false,
                                        false,
                                        false,
                                        trx,
                                        farePath,
                                        nullptr,
                                        pricingUnit,
                                        fare.fareMarket(),
                                        farePath->itin()->travelSeg().front()->departureDT(),
                                        applTravelSegment,
                                        origCheck,
                                        destCheck,
                                        fltStopCheck,
                                        _tsi,
                                        _locKey1,
                                        _locKey2,
                                        Diagnostic312);
      if (!retCode995 || applTravelSegment.empty())
      {
        if (UNLIKELY(diagPtr))
        {
          *diagPtr << "    SURCHARGE RULE - SKIP BY 995 TABLE DATA:"
                   << "\n";
          *diagPtr << "                     NO MATCH IN 995 TABLE "
                   << "\n";
        }

        return SKIP;
      }
    }
    else
    {
      _tsi = 0;
      // set "origCheck = true"
      // and populate "applTravelSegment" for the all segments per the current FC
      // origCheck = true;
      for (; itB != itE; ++itB)
      {
        // add to applTravelSegment
        RuleUtil::TravelSegWrapper* tsw = nullptr;
        trx.dataHandle().get(tsw);
        // lint --e{413}
        tsw->travelSeg() = *itB;
        tsw->origMatch() = true;
        tsw->destMatch() = true;

        applTravelSegment.push_back(tsw);
      }
    }

    _matched986Segs.clear();

    if (!matchT986(trx, surchInfo, fare.vendor(), applTravelSegment, diagPtr))
      return SKIP;

    _matchedRbdSegs.clear();
    if (!matchRBD(surchInfo, applTravelSegment, diagPtr, trx, fareUsage))
      return SKIP;

    return validateNonSideTrip(
        surchargeSegments, trx, farePath, fareUsage, fare, *surchInfo, applTravelSegment, diagPtr);
  } // Side_Trip/Non_Side_Trip

  return PASS;
}

Record3ReturnTypes
SurchargesRule::validateSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                                 PricingTrx& trx,
                                 FarePath* farePath,
                                 const PricingUnit* pricingUnit,
                                 FareUsage* fareUsage,
                                 PaxTypeFare& fare,
                                 const SurchargesInfo* surchInfo,
                                 RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                 Diag312Collector* diag)
{
  bool origCheck = true;
  bool destCheck = true;
  bool fltStopCheck = false;
  bool origCheckBtw = true;
  bool destCheckBtw = true;
  bool origCheckAnd = true;
  bool destCheckAnd = true;

  bool isSTSurcharge = false;

  RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  size_t size = fare.fareMarket()->travelSeg().size();

  std::vector<TravelSeg*>::const_iterator itB = fare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itE = fare.fareMarket()->travelSeg().end();
  std::vector<TravelSeg*>::const_iterator itL = itE - 1;
  std::vector<TravelSeg*>::const_iterator itBegin = itB;

  const bool isFVOSurcharges = TrxUtil::isFVOSurchargesEnabled();

  //=======================================================
  //  Side_Trip processing
  //=======================================================
  if (!isFVOSurcharges && !isDataCorrect(*surchInfo))
  {
    // moved to flight independent
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY SIDETRIP DATA:"
            << "\n";
      *diag << "                     INCORRECT RULE DATA "
            << "\n";
    }
    return SKIP;
  }
  // lint -e{506}
  if (size <= 3)
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY SIDETRIP DATA:"
            << "\n";
      *diag << "                     NO SIDETRIP "
            << "\n";
    }
    return SKIP;
  }

  RuleUtil::TravelSegWrapperVector applTravelSegmentBtw;
  RuleUtil::TravelSegWrapperVector applTravelSegmentAnd;

  bool match995 = false;
  bool retCode995 = false;

  // check 995 Geo table and 995Btw/995And Geo tables
  if (surchInfo->geoTblItemNo() == 0)
  {
    _tsi = 0;
    match995 = true;
  }
  else
  {
    _tsi = 0;
    retCode995 = RuleUtil::validateGeoRuleItem(surchInfo->geoTblItemNo(),
                                               fare.vendor(),
                                               defaultScope,
                                               false,
                                               false,
                                               false,
                                               trx,
                                               farePath,
                                               nullptr,
                                               pricingUnit,
                                               fare.fareMarket(),
                                               farePath->itin()->travelSeg().front()->departureDT(),
                                               applTravelSegment,
                                               origCheck,
                                               destCheck,
                                               fltStopCheck,
                                               _tsi,
                                               _locKey1,
                                               _locKey2,
                                               Diagnostic312);

    if (retCode995 && !applTravelSegment.empty())
    {
      // looping through each segment for the current Fare Component scope
      itB = fare.fareMarket()->travelSeg().begin();

      for (; itB != itE; ++itB)
      {
        RuleUtil::TravelSegWrapperVectorCI itBegAppl = applTravelSegment.begin();
        RuleUtil::TravelSegWrapperVectorCI itEndAppl = applTravelSegment.end();

        for (; itBegAppl != itEndAppl; ++itBegAppl)
        {
          if (*itB == (*itBegAppl)->travelSeg())
          {
            match995 = true;
            break;
          } // matching segment pointers
        } // internal loop through the vector of *segments from 995 table
        if (match995)
        {
          break;
        }
      } // loop through segments in FC
    }
  }

  if (!match995)
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY SIDE TRIP 995 TABLE:"
            << "\n";
      *diag << "                     NO MATCH IN 995 TABLE "
            << "\n";
    }
    return SKIP;
  }
  // check 995Btw
  _tsiBtw = 0;
  retCode995 = RuleUtil::validateGeoRuleItem(surchInfo->geoTblItemNoBtw(),
                                             fare.vendor(),
                                             defaultScope,
                                             false,
                                             false,
                                             false,
                                             trx,
                                             farePath,
                                             nullptr,
                                             pricingUnit,
                                             fare.fareMarket(),
                                             farePath->itin()->travelSeg().front()->departureDT(),
                                             applTravelSegmentBtw,
                                             origCheckBtw,
                                             destCheckBtw,
                                             fltStopCheck,
                                             _tsiBtw,
                                             _locKeyBtw1,
                                             _locKeyBtw2,
                                             Diagnostic312);

  if (!retCode995 || applTravelSegmentBtw.empty())
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY SIDE TRIP 995BTW TABLE:"
            << "\n";
      *diag << "                     NO MATCH IN 995BTW TABLE "
            << "\n";
    }
    return SKIP;
  }

  if (surchInfo->geoTblItemNoAnd() == 0)
  {
    // Do "Between" logic.
    // Apply FROM/TO the specified points from "applTravelSegmentBtw" vector
    // within the current Fare component.

    _tsiAnd = 0;

    for (; itB != itE; ++itB)
    {
      bool localSideTrip = false;

      RuleUtil::TravelSegWrapperVectorCI itBegAppl = applTravelSegmentBtw.begin();
      RuleUtil::TravelSegWrapperVectorCI itEndAppl = applTravelSegmentBtw.end();

      for (; itBegAppl != itEndAppl; ++itBegAppl)
      {
        if (*itB == (*itBegAppl)->travelSeg())
        {
          localSideTrip = false;
          std::vector<TravelSeg*>::const_iterator itRet;

          if ((*itBegAppl)->origMatch())
          {
            if (isSideTrip(itB, itE, itBegin, itL, true, itRet))
            {
              if (isFVOSurcharges)
                surchargeSegments.push_back(SurchargeSeg(*itB,
                                                         &(*itB)->boardMultiCity(),
                                                         &(*itRet)->offMultiCity(),
                                                         (*itB) == (*itRet),
                                                         &(**itB).origAirport()));
              else
              {
                SurchargeData* surchargeData = nullptr;
                if (trx.getTrxType() == PricingTrx::MIP_TRX || trx.isShopping())
                {
                  trx.dataHandle().get(surchargeData);
                }
                else
                {
                  surchargeData = trx.constructSurchargeData();
                }

                // haveSurcharge = true; // never used
                addSurcharge(trx,
                             farePath,
                             fare,
                             fareUsage,
                             *surchInfo,
                             surchargeData,
                             (**itB),
                             (*itB)->boardMultiCity(),
                             (*itRet)->offMultiCity(),
                             (*itB) == (*itRet));

                if (UNLIKELY(diag))
                  diag->displaySideTripSurchargeData(surchargeData, (*itB)->origAirport());

                // lint -e{413}
                fareUsage->surchargeData().push_back(surchargeData);
              }

              isSTSurcharge = true;
              localSideTrip = true;
            }
          }
          // some Geo995's (with TSI) return both "orig" and "dest" indicators
          if (!localSideTrip)
          {
            if ((*itBegAppl)->destMatch())
            {
              if (isSideTrip(itB, itE, itBegin, itL, false, itRet))
              {
                if (isFVOSurcharges)
                  surchargeSegments.push_back(SurchargeSeg(*itB,
                                                           &(*itB)->boardMultiCity(),
                                                           &(*itRet)->offMultiCity(),
                                                           (*itB) == (*itRet),
                                                           &(**itB).destAirport()));
                else
                {
                  SurchargeData* surchargeData = nullptr;
                  if (trx.getTrxType() == PricingTrx::MIP_TRX || trx.isShopping())
                  {
                    trx.dataHandle().get(surchargeData);
                  }
                  else
                  {
                    surchargeData = trx.constructSurchargeData();
                  }

                  // haveSurcharge = true; // never used
                  addSurcharge(trx,
                               farePath,
                               fare,
                               fareUsage,
                               *surchInfo,
                               surchargeData,
                               (**itB),
                               (*itB)->boardMultiCity(),
                               (*itRet)->offMultiCity(),
                               (*itB) == (*itRet));

                  if (UNLIKELY(diag))
                  {
                    diag->displaySideTripSurchargeData(surchargeData, (*itB)->destAirport());
                  }
                  // lint -e{413}
                  fareUsage->surchargeData().push_back(surchargeData);
                }

                isSTSurcharge = true;
                localSideTrip = true;
              }
            }
          }
          if (isSTSurcharge)
          {
            break; // job is done for the current travel segment within FC
          }
        } //*iT ==*itBegAppl
      } // for (++itBegappl)
    } // for (++itB)
    if (!isSTSurcharge)
    {
      if (UNLIKELY(diag))
      {
        *diag << "    SURCHARGE RULE - SKIP BY SIDE TRIP 995BTW TABLE:"
              << "\n";
        *diag << "                     NO MATCH TRAVEL SEGMENT"
              << "\n";
      }
      return SKIP;
    }
  } // 995_Between
  else
  {
    // Do "Between/And" logic
    _tsiAnd = 0;
    retCode995 = RuleUtil::validateGeoRuleItem(surchInfo->geoTblItemNoAnd(),
                                               fare.vendor(),
                                               defaultScope,
                                               false,
                                               false,
                                               false,
                                               trx,
                                               farePath,
                                               nullptr,
                                               pricingUnit,
                                               fare.fareMarket(),
                                               farePath->itin()->travelSeg().front()->departureDT(),
                                               applTravelSegmentAnd,
                                               origCheckAnd,
                                               destCheckAnd,
                                               fltStopCheck,
                                               _tsiAnd,
                                               _locKeyAnd1,
                                               _locKeyAnd2,
                                               Diagnostic312);
    if (!retCode995 || applTravelSegmentAnd.empty())
    {
      if (UNLIKELY(diag))
      {
        *diag << "    SURCHARGE RULE - SKIP BY SIDE TRIP 995AND TABLE:"
              << "\n";
        *diag << "                     NO MATCH IN 995AND TABLE "
              << "\n";
      }
      return SKIP;
    }
    //
    //  the same logic as "BETWEEN" except:
    //  - any one segment from "AND" vector should be within a side trip
    //  - or vice versa

    if (!processSideTrip(surchargeSegments,
                         trx,
                         farePath,
                         fareUsage,
                         fare,
                         *surchInfo,
                         applTravelSegmentBtw,
                         applTravelSegmentAnd,
                         diag) &&
        !processSideTrip(surchargeSegments,
                         trx,
                         farePath,
                         fareUsage,
                         fare,
                         *surchInfo,
                         applTravelSegmentAnd,
                         applTravelSegmentBtw,
                         diag))
    {
      if (UNLIKELY(diag))
      {
        *diag << "  SURCHARGE RULE - SKIP BY SIDE TRIP 995AND 995BTW TABLES:"
              << "\n";
        *diag << "                   NO MATCH TRAVEL SEGMENT"
              << "\n";
      }
      return SKIP;
    }
  }
  return PASS;
}

bool
SurchargesRule::processSectionPortion(PricingTrx& trx,
                                      FarePath* farePath,
                                      const PricingUnit* pricingUnit,
                                      FareUsage* fareUsage,
                                      PaxTypeFare& fare,
                                      const SurchargesInfo* surchInfo,
                                      RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                      Diag312Collector* diag)
{
  if (UNLIKELY(surchInfo->geoTblItemNoBtw() == 0 || surchInfo->geoTblItemNoAnd() == 0))
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY 995 TABLE:"
            << "\n";
      if (surchInfo->geoTblItemNoBtw())
        *diag << "                     MISSING 995BTW TABLE"
              << "\n";
      else
        *diag << "                     MISSING 995AND TABLE"
              << "\n";
    }
    return false;
  }

  RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
  std::vector<TravelSeg*> tsVec[2];
  bool fltStopCheck = false;
  const bool isSectorSurch = surchInfo->sectorPortion() == SectorPortionSector;
  bool isCat12Request = true;
  if (RuleUtil::getTvlSegBtwAndGeo(trx,
                                   surchInfo->geoTblItemNoBtw(),
                                   surchInfo->geoTblItemNoAnd(),
                                   fare.vendor(),
                                   tsVec[0],
                                   RtwUtil::isRtw(trx) ? tsVec + 1 : nullptr,
                                   defaultScope,
                                   farePath,
                                   pricingUnit,
                                   fare.fareMarket(),
                                   isSectorSurch,
                                   trx.ticketingDate(),
                                   fltStopCheck,
                                   diag, isCat12Request))
  {
    bool pass = populateApplTravelSegment(trx, applTravelSegment, tsVec[0], diag);
    if (UNLIKELY(tsVec[1].size()))
      pass |= populateApplTravelSegment(trx, applTravelSegment, tsVec[1], diag);

    if (UNLIKELY(!pass))
    {
      if (UNLIKELY(diag))
      {
        *diag << "    SURCHARGE RULE - SKIP BY 995 TABLE:\n";
        *diag << "                     NOT MATCHED SECTOR\n";
      }
      return false;
    }

    // This is new code for QSUR13
    if (surchInfo->geoTblItemNo() != 0)
    {
      RuleUtil::TravelSegWrapperVector tsVecItemNo;

      bool origCheck = true;
      bool destCheck = true;
      bool fltStopCheck = false;
      RuleConst::TSIScopeParamType defaultScope = RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT;
      _tsi = 0;
      bool retCode995 =
          RuleUtil::validateGeoRuleItem(surchInfo->geoTblItemNo(),
                                        fare.vendor(),
                                        defaultScope,
                                        false,
                                        false,
                                        false,
                                        trx,
                                        farePath,
                                        nullptr,
                                        pricingUnit,
                                        fare.fareMarket(),
                                        farePath->itin()->travelSeg().front()->departureDT(),
                                        tsVecItemNo,
                                        origCheck,
                                        destCheck,
                                        fltStopCheck,
                                        _tsi,
                                        _locKey1,
                                        _locKey2,
                                        Diagnostic312);

      if (!retCode995 || tsVecItemNo.empty())
      {
        if (UNLIKELY(diag))
        {
          *diag << "    SURCHARGE RULE - SKIP BY 995 TABLE DATA:"
                << "\n";
          *diag << "                     NO MATCH IN T995"
                << "\n";
        }
        return false;
      }

      if (surchInfo->tvlPortion() == RuleConst::PERTRANSFER)
        markTransferPoint(applTravelSegment, tsVecItemNo, trx);
      else
        commonTravelSeg(applTravelSegment, tsVecItemNo);

      if (applTravelSegment.empty())
      {
        if (UNLIKELY(diag))
        {
          *diag << "    SURCHARGE RULE - SKIP BY 995 TABLE DATA:"
                << "\n";
          *diag << "       NO MATCH T995 BTW/AND TO FROM/TO/VIA"
                << "\n";
        }
        return false;
      }
    }
    return true;
  }
  if (UNLIKELY(diag))
  {
    *diag << "    SURCHARGE RULE - SKIP BY 995 TABLE:"
          << "\n";
    *diag << "                     NOT MATCHED ON T995"
          << "\n";
  }
  return false;
}

bool
SurchargesRule::populateApplTravelSegment(PricingTrx& trx,
                                          RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                          const std::vector<TravelSeg*> tsVec,
                                          Diag312Collector* diag) const
{
  if (UNLIKELY(diag && tsVec.size() &&
               trx.diagnostic().diagParamMapItem(Diagnostic::ALL_VALID) == "SEG"))
  {
    *diag << "                   MATCHED SEGMENTS:\n";
    for (const TravelSeg* ts : tsVec)
    {
      *diag << "                     " << ts->origAirport();
      *diag << " - " << ts->destAirport() << std::endl;
    }
  }

  for (TravelSeg* tvlSeg : tsVec)
  {
    RuleUtil::TravelSegWrapper* tsw = nullptr;
    trx.dataHandle().get(tsw);
    tsw->travelSeg() = tvlSeg;
    tsw->origMatch() = true;
    tsw->destMatch() = true;
    applTravelSegment.push_back(tsw);
  }

  return tsVec.size();
}

void
SurchargesRule::commonTravelSeg(RuleUtil::TravelSegWrapperVector& tvlSegsFirst,
                                RuleUtil::TravelSegWrapperVector& tvlSegsSecond)
{
  RuleUtil::TravelSegWrapperVector commonElements;

  for (const RuleUtil::TravelSegWrapper* tsw : tvlSegsSecond)
  {
    RuleUtil::TravelSegWrapperVector::const_iterator it;
    it = std::find_if(tvlSegsFirst.begin(), tvlSegsFirst.end(), CompareTsw(tsw));

    if (it != tvlSegsFirst.end())
      commonElements.push_back(tsw);
  }

  tvlSegsFirst.clear();
  tvlSegsFirst.insert(tvlSegsFirst.begin(), commonElements.begin(), commonElements.end());
}

void
SurchargesRule::markTransferPoint(RuleUtil::TravelSegWrapperVector& tvlSegsBtwAnd,
                                  RuleUtil::TravelSegWrapperVector& tvlSegsToFromVia,
                                  PricingTrx& trx)
{
  RuleUtil::TravelSegWrapperVector matchedTransferPoint;
  for (const RuleUtil::TravelSegWrapper* tsw : tvlSegsBtwAnd)
  {
    RuleUtil::TravelSegWrapperVector::const_iterator it;
    it = std::find_if(tvlSegsToFromVia.begin(), tvlSegsToFromVia.end(), CompareTsw(tsw));

    RuleUtil::TravelSegWrapper* newTsw = nullptr;
    trx.dataHandle().get(newTsw);
    newTsw->travelSeg() = tsw->travelSeg();

    if (it != tvlSegsToFromVia.end())
    {
      newTsw->origMatch() = (*it)->origMatch();
      newTsw->destMatch() = (*it)->destMatch();
    }
    else
    {
      newTsw->origMatch() = false;
      newTsw->destMatch() = false;
    }
    matchedTransferPoint.push_back(newTsw);
  }

  tvlSegsBtwAnd.clear();
  tvlSegsBtwAnd.insert(
      tvlSegsBtwAnd.begin(), matchedTransferPoint.begin(), matchedTransferPoint.end());
}

bool
SurchargesRule::matchT986(PricingTrx& trx,
                          const SurchargesInfo* surchInfo,
                          const VendorCode& vendor,
                          RuleUtil::TravelSegWrapperVector& applTravelSegment,
                          Diag312Collector* diag)
{
  // nothing to validate
  if (surchInfo->carrierFltTblItemNo() == 0)
    return true;

  // no records in 986 -fail?
  const CarrierFlight* table986 =
      trx.dataHandle().getCarrierFlight(vendor, surchInfo->carrierFltTblItemNo());
  if (UNLIKELY(!table986 || table986->segCnt() == 0))
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY 986 TABLE:\n";
      *diag << "                     T986 RECORDS NOT FOUND\n";
    }
    return false;
  }

  RuleUtil::TravelSegWrapperVectorCI itB = applTravelSegment.begin();
  RuleUtil::TravelSegWrapperVectorCI itE = applTravelSegment.end();
  for (; itB != itE; ++itB)
  {
    AirSeg* aSeg = (*itB)->travelSeg()->toAirSeg();
    if (!aSeg)
      continue;

    std::vector<CarrierFlightSeg*>::const_iterator segI = table986->segs().begin();
    std::vector<CarrierFlightSeg*>::const_iterator segE = table986->segs().end();
    for (; segI != segE; ++segI)
    {
      const CarrierFlightSeg& t986Seg = **segI;
      const AirSeg& currentSeg = *aSeg;

      if (!CarrierUtil::carrierExactOrAllianceMatch(
              currentSeg.marketingCarrierCode(), t986Seg.marketingCarrier(), trx) ||
          !CarrierUtil::carrierExactOrAllianceMatch(
              currentSeg.operatingCarrierCode(), t986Seg.operatingCarrier(), trx))
        continue;


      // Only flt1 is set. Flight numbers have to be equal
      // flt1 = 0    No data in T986
      // flt1 = -1   Any flight Number
      if (t986Seg.flt1() > 0 && t986Seg.flt2() == 0 && t986Seg.flt1() != currentSeg.flightNumber())
        continue;

      // Both flt are set. Flight number should be in range.
      if (t986Seg.flt2() != 0 && t986Seg.flt1() != 0 &&
          (t986Seg.flt2() < currentSeg.flightNumber() ||
           t986Seg.flt1() > currentSeg.flightNumber()))
      {
        continue;
      }

      // save matched for transfer/coupon matching
      _matched986Segs.push_back((*itB)->travelSeg());
      break;
    }
  }
  // no match on 986 - fail
  if (_matched986Segs.empty())
  {
    if (UNLIKELY(diag))
    {
      *diag << "    SURCHARGE RULE - SKIP BY 986 TABLE:\n";
      *diag << "                     NOT MATCHED ON TABLE 986\n";
    }
    return false;
  }
  return true;
}

bool
SurchargesRule::matchRBD(const SurchargesInfo* surchInfo,
                         RuleUtil::TravelSegWrapperVector& applTravelSegment,
                         Diag312Collector* diag,
                         PricingTrx& trx,
                         FareUsage* fUsagep)
{
  // need validation if booking code resent and type 'Z', 'B', 'L'
  if (!surchInfo->bookingCode().empty())
  {
    RuleUtil::TravelSegWrapperVectorCI itB = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVectorCI itE = applTravelSegment.end();

    bool wpncReq = trx.getRequest()->isLowFareRequested();
    for (; itB != itE; ++itB)
    {
      if (wpncReq && fUsagep && (!fUsagep->segmentStatus().empty()))
      {
        size_t index = 0;
        for (TravelSeg* tvlSeg : fUsagep->travelSeg())
        {
          if (tvlSeg->pnrSegment() == (*itB)->travelSeg()->pnrSegment())
          {
            if ((!fUsagep->segmentStatus()[index]._bkgCodeReBook.empty()) &&
                (fUsagep->segmentStatus()[index]._bkgCodeSegStatus.isSet(
                    PaxTypeFare::BKSS_REBOOKED)))
            {
              if (fUsagep->segmentStatus()[index]._bkgCodeReBook == surchInfo->bookingCode())
                _matchedRbdSegs.push_back((*itB)->travelSeg());
            }
            else
            {
              if ((*itB)->travelSeg()->getBookingCode() == surchInfo->bookingCode())
                _matchedRbdSegs.push_back((*itB)->travelSeg());
            }
          }
          index++;
        }
      }
      else if ((*itB)->travelSeg()->getBookingCode() == surchInfo->bookingCode())
        _matchedRbdSegs.push_back((*itB)->travelSeg());
    }
    if (_matchedRbdSegs.empty())
    {
      if (UNLIKELY(diag))
      {
        *diag << "    SURCHARGE RULE - SKIP BY RBD:"
              << "\n";
        *diag << "                     NOT MATCHED ON RBD"
              << "\n";
      }
      return false;
    }
  }
  return true;
}

//=======================================================
//  check Date range
//=======================================================
bool
SurchargesRule::checkDateRange(PricingTrx& trx,
                               const SurchargesInfo& surchInfo,
                               TravelSeg& tSeg,
                               bool origin,
                               NoPNRPricingTrx* noPnrTrx)
{
  if (!surchInfo.startYear() && !surchInfo.startMonth() && !surchInfo.startDay() &&
      !surchInfo.stopYear() && !surchInfo.stopMonth() && !surchInfo.stopDay())
  {
    return true;
  }
  // check OPEN segment.
  // The rule should pass (without surcharges)
  //  if one or more open segments without a date
  //    after a segment with a date (at the end of an itinerary) and
  //    there are no more a dated TravelSegs in Itin.
  //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
  // doesn't apply for WQ transactions
  if (UNLIKELY(!noPnrTrx && tSeg.segmentType() == Open && tSeg.openSegAfterDatedSeg()))
  {
    return false;
  }
  DateTime checkedDT;
  if (origin)
  {
    checkedDT = tSeg.departureDT();
  }
  else
  {
    //Start/Stop Dates require a Departure validation to be made
    //unless used in conjunction with an arrival TSI

    bool checkOrig = true;
    bool checkDest = false;
    if (_tsi != 0)
    {
      if (RuleUtil::getTSIOrigDestCheck(_tsi, trx, checkOrig, checkDest) && !checkOrig)
      {
        checkedDT = tSeg.arrivalDT();
      }
      else
      {
        checkedDT = tSeg.departureDT();
      }
    }
  }

  // for WQ transactions, date for open segment may have to be updated
  if (UNLIKELY(noPnrTrx))
  {
    if (noPnrTrx->itin().front()->dateType() == Itin::NoDate)
      checkedDT = DateTime::localTime();
    else
      noPnrTrx->updateOpenDateIfNeccesary(&tSeg, checkedDT);
  }

  return checkedDT.isBetween(surchInfo.startYear(),
                             surchInfo.startMonth(),
                             surchInfo.startDay(),
                             surchInfo.stopYear(),
                             surchInfo.stopMonth(),
                             surchInfo.stopDay());
}

//=======================================================
//  check Day of Week and Time data
//=======================================================
bool
SurchargesRule::checkDOWandTime(PricingTrx& trx,
                                const SurchargesInfo& surchInfo,
                                TravelSeg& tSeg,
                                bool origin,
                                bool firstTravelSegment,
                                NoPNRPricingTrx* noPnrTrx)
{
  if (surchInfo.dow().empty() && surchInfo.todAppl() == RuleConst::BLANK &&
      surchInfo.startTime() == -1 && surchInfo.stopTime() == -1)
  {
    return true;
  }
  if (UNLIKELY(!surchInfo.dow().empty() && surchInfo.todAppl() == RuleConst::BLANK))
  {
    return false;
  }
  // check OPEN segment.
  // The rule should pass (without surcharges)
  //  if one or more open segments without a date
  //    after a segment with a date (at the end of an itinerary) and
  //    there are no more a dated TravelSegs in Itin.
  //  otherwise, validate as ussualy with date+1(done in ItinAnalizer).
  // doesn't apply for WQ transactions
  if (UNLIKELY(!noPnrTrx && tSeg.segmentType() == Open && tSeg.openSegAfterDatedSeg()))
  {
    return false;
  }

  DateTime checkedDT;
  if (origin)
  {
    checkedDT = tSeg.departureDT();
  }
  else
  {
    //According to ATPCO Validation of the Start/Stop dates is a departure application.
    bool checkOrig = true;
    bool checkDest = false;
    if (_tsi != 0)
    {
      if (RuleUtil::getTSIOrigDestCheck(_tsi, trx, checkOrig, checkDest) && !checkOrig)
      {
        checkedDT = tSeg.arrivalDT();
      }
      else
      {
        checkedDT = tSeg.departureDT();
      }
    }
  }
  // for WQ transactions, date for open segment may have to be updated
  if (UNLIKELY(noPnrTrx))
  {
    if (noPnrTrx->itin().front()->dateType() == Itin::NoDate)
      checkedDT = DateTime::localTime();
    else
      noPnrTrx->updateOpenDateIfNeccesary(&tSeg, checkedDT);
  }

  int dowSeg = checkedDT.dayOfWeek(); // 0...6
  int dowRule = 0;
  bool matchDow = false;

  std::string::const_iterator it = surchInfo.dow().begin();
  std::string::const_iterator itEnd = surchInfo.dow().end();
  std::string::const_iterator itLast = itEnd;

  if (LIKELY(!surchInfo.dow().empty()))
  {
    itLast = itEnd - 1;
    while (it != itEnd)
    {
      dowRule = (*it) - '0'; // 1---7  Note: 7=sunday!!!
      if (dowRule == 7)
      {
        dowRule = 0;
      }
      if (dowSeg == dowRule)
      {
        matchDow = true;
        break;
      }
      ++it;
    }
    if (!matchDow)
    {
      return false;
    }
  }
  if (LIKELY(surchInfo.startTime() == -1))
  {
    return true;
  }
  if (tSeg.segmentType() == Open)
  {
    return false;
  }
  // check a time
  const int minutesFromSeg = static_cast<int>(checkedDT.totalMinutes());

  const int minStartFromRule = surchInfo.startTime();

  // The format of surcharge time in the record 3 is total minutes from midnight
  // It is not in  Military Time format  SPR 17286

  int minStopFromRule = surchInfo.stopTime();

  if (minStopFromRule == 1440) // 24 hrs?
  {
    minStopFromRule = 1439; // If so, make it =23.59
  }

  if (surchInfo.todAppl() == RuleConst::DAYLY || surchInfo.todAppl() == RuleConst::BLANK)
  {
    return minutesFromSeg >= minStartFromRule && minutesFromSeg <= minStopFromRule;
  }
  else if (surchInfo.todAppl() == RuleConst::RANGE)
  {
    if (it == itEnd || surchInfo.dow().begin() == itLast) // no DOW or only one day in range
    {
      return minutesFromSeg >= minStartFromRule && minutesFromSeg <= minStopFromRule;
    }
    if (it == surchInfo.dow().begin())
    {
      return minutesFromSeg >= minStartFromRule;
    }
    else if (it == itLast)
    {
      return minutesFromSeg <= minStopFromRule;
    }
    else
      return true;
  }

  return false;
}

//=======================================================
//  add surchargeData in vector (or accumulate amount)
//=======================================================
SurchargeData*
SurchargesRule::addSurcharge(PricingTrx& trx,
                             const FarePath* farePath,
                             const PaxTypeFare& fare,
                             FareUsage* fareUsage,
                             const SurchargesInfo& surchInfo,
                             SurchargeData* surchargeData,
                             TravelSeg& tSeg,
                             const LocCode& fcBrdCity,
                             const LocCode& fcOffCity,
                             bool singleSector)

{
  const bool isFVOSurcharges = TrxUtil::isFVOSurchargesEnabled();
  if (UNLIKELY(isFVOSurcharges))
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX || trx.isShopping())
    {
      trx.dataHandle().get(surchargeData);
    }
    else
    {
      surchargeData = trx.constructSurchargeData();
    }
  }

  MoneyAmount fareAmount = 0.0;
  MoneyAmount surAmount = 0.0;
  CurrencyNoDec surCurNoDec = 0;
  MoneyAmount surAmountNuc = 0.0;
  CurrencyCode surCurrency;

  MoneyAmount surAmountNuc1 = 0.0;
  MoneyAmount surAmountNuc2 = 0.0;
  CurrencyConversionFacade ccFacade;

  bool isPercent = (surchInfo.surchargeCur1().empty() && surchInfo.surchargeCur2().empty());

  const Itin& itin = *farePath->itin();

  if (isPercent)
  {
    // check a percent
    // if (surchInfo.surchargePercentAppl() == FARE)
    surCurrency = fare.currency();
    surCurNoDec = fare.numDecimal();

    if (surCurNoDec == 0 )
    {
      const Currency* currency = nullptr;
      currency = trx.dataHandle().getCurrency(surCurrency);

      if (currency)
      {
        surCurNoDec = currency->noDec();
      }
    }

    // According to ATPCO data application
    // "The Comparison Fare and the Resulting Fare amount created by the Fare by Rule Fare
    // Calculation
    // fields (Bytes 60-103) will be used for comparison, prior to applying any rule validation
    // charges
    // such surcharges."

    if (fareUsage && fareUsage->cat25Fare())
    {
      fareAmount = fareUsage->cat25Fare()->originalFareAmount();
    }
    else
    {
      fareAmount = (fare.originalFareAmount());
    }

    // Save surcharge amount in local currency
    surAmount = (fareAmount * surchInfo.surchargePercent()) / 100.0f;

    const Money surAmt(fareAmount, fare.currency());
    Money nuc("NUC");
    // convertedAmount will contain either the NUC amount from
    // a single NUC conversion or the local currency amount
    // from a double NUC conversion:
    //
    // Double NUC conversion Example:
    // Surcharge Currency: GBP
    // Calculation Currency: INR
    // Convert GBP - NUC - GBP NUC amount is contained in nuc.value()
    // Convert NUC - INR - convertedAmount is in INR's
    // Surcharges uses the nuc.value() from the first NUC conversion
    // for comparison purposes. The nuc.value() from the first NUC
    // conversion is never overwritten with a subsequent conversion.
    // It is always a NUC.
    //
    // Single NUC conversion Example:
    // Surcharge Currency: GBP
    // Calculation Currency: NUC
    // Convert GBP - NUC - GBP NUC amount is contained in nuc.value()
    //                        and also convertedAmount
    //
    MoneyAmount convertedAmount = 0.0;

    // Perform either a single or double NUC conversion. This depends on whether or
    // not the calculation currency is NUC.

    if (LIKELY(ccFacade.convert(nuc,
                         surAmt,
                         trx,
                         itin.calculationCurrency(),
                         convertedAmount,
                         itin.useInternationalRounding())))
    {
      CurrencyUtil::truncateNUCAmount(convertedAmount);
      convertedAmount = (convertedAmount * surchInfo.surchargePercent()) / 100.0f;
      CurrencyUtil::truncateNUCAmount(convertedAmount);

      surAmountNuc = convertedAmount;
    }
  }
  else
  {
    // match the surcharge currency
    if (surchInfo.surchargeCur1() == fare.currency())
    {
      surCurrency = surchInfo.surchargeCur1();
      surCurNoDec = fare.numDecimal();
      surAmount = surchInfo.surchargeAmt1();
    }
    else if (surchInfo.surchargeCur2() == fare.currency())
    {
      surCurrency = surchInfo.surchargeCur2();
      surCurNoDec = fare.numDecimal();
      surAmount = surchInfo.surchargeAmt2();
    }
    else
    {
      Money nuc1("NUC");
      Money nuc2("NUC");

      if (LIKELY(!surchInfo.surchargeCur1().empty()))
      {
        const Money surAmt1(surchInfo.surchargeAmt1(), surchInfo.surchargeCur1());

        MoneyAmount convertedAmount = 0.0;

        if (LIKELY(ccFacade.convert(nuc1,
                             surAmt1,
                             trx,
                             itin.calculationCurrency(),
                             convertedAmount,
                             itin.useInternationalRounding())))
        {
          CurrencyUtil::truncateNUCAmount(convertedAmount);
          surAmountNuc1 = convertedAmount;
        }
      }
      if (UNLIKELY(!surchInfo.surchargeCur2().empty()))
      {
        const Money surAmt2(surchInfo.surchargeAmt2(), surchInfo.surchargeCur2());

        MoneyAmount convertedAmount = 0.0;

        if (ccFacade.convert(nuc2,
                             surAmt2,
                             trx,
                             itin.calculationCurrency(),
                             convertedAmount,
                             itin.useInternationalRounding()))
        {
          CurrencyUtil::truncateNUCAmount(convertedAmount);
          surAmountNuc2 = convertedAmount;
        }
      }
      if (LIKELY(nuc1.value() < nuc2.value() || surchInfo.surchargeCur2().empty()))
      {
        surAmountNuc = surAmountNuc1;
        surCurrency = surchInfo.surchargeCur1();
        surCurNoDec = surchInfo.surchargeNoDec1();
        surAmount = surchInfo.surchargeAmt1();
      }
      else
      {
        surAmountNuc = surAmountNuc2;
        surCurrency = surchInfo.surchargeCur2();
        surCurNoDec = surchInfo.surchargeNoDec2();
        surAmount = surchInfo.surchargeAmt2();
      }
    }
  }
  if (surAmountNuc == 0 && surAmount != 0 && !surCurrency.empty())
  {
    const Money surAmt(surAmount, surCurrency);
    Money nuc("NUC");

    MoneyAmount convertedAmount = 0.0;

    if (LIKELY(ccFacade.convert(nuc,
                         surAmt,
                         trx,
                         itin.calculationCurrency(),
                         convertedAmount,
                         itin.useInternationalRounding())))
    {
      CurrencyUtil::truncateNUCAmount(convertedAmount);
      surAmountNuc = convertedAmount;
    }
  }

  bool isCalculatedDisCountedFare = false;
  VendorCode vendor;
  MoneyAmount discPercent;
  if (fare.isDiscounted())
  {
    isCalculatedDisCountedFare = checkDiscountedInfo(fare, vendor, discPercent);
  }
  else
  {
    // just for sure when something goes wrong we can rollback
    // and take ptf from old place
    const PaxTypeFare* fbrPtf =
        (fareUsage && !isFVOSurcharges) ? fareUsage->cat25Fare() : fare.cat25Fare();

    if (fbrPtf && // Access to Cat 25 fare or Cat 19/25 fare
        fbrPtf->isDiscounted())
    {
      isCalculatedDisCountedFare = checkDiscountedInfo(*fbrPtf, vendor, discPercent);
    }
  }

  const PaxTypeCode& paxTypeCode = getPaxType(farePath, fare).paxType();

  if (isCalculatedDisCountedFare)
  {
    if ((vendor == Vendor::ATPCO &&
         surchInfo.surchargeAppl() == RuleConst::ADT_CHILD_DISC_INFANT_DISC_SURCHARGE &&
         (PaxTypeUtil::isChild(trx, paxTypeCode, vendor) ||
          PaxTypeUtil::isInfant(trx, paxTypeCode, vendor))) ||
        (vendor == Vendor::SITA &&
         ((PaxTypeUtil::isChild(trx, paxTypeCode, vendor) &&
           (surchInfo.surchargeAppl() == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC ||
            surchInfo.surchargeAppl() == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC ||
            surchInfo.surchargeAppl() == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE)) ||
          (PaxTypeUtil::isInfant(trx, paxTypeCode, vendor) &&
           (surchInfo.surchargeAppl() == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC ||
            surchInfo.surchargeAppl() == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC))))) // SITA (per
                                                                                     // BA):
    // 'APPL=3' the same as 'APPL=5'
    {
      surAmountNuc = (surAmountNuc * (discPercent / 100.0f));
      CurrencyUtil::truncateNUCAmount(surAmountNuc);
      surAmount = (surAmount * (discPercent / 100.0f));
    }
  }

  if (surchInfo.tvlPortion() == RuleConst::ROUNDTRIP)
  {
    surAmount = surAmount / 2;
    surAmountNuc = surAmountNuc / 2;
    CurrencyUtil::truncateNUCAmount(surAmountNuc);
  }
  CurrencyUtil::truncateNonNUCAmount(surAmount, surCurNoDec);

  surchargeData->carrier() = fare.carrier();
  surchargeData->itinItemCount() = 1;
  surchargeData->surchargeAppl() = surchInfo.surchargeAppl();
  surchargeData->surchargeType() = surchInfo.surchargeType();
  surchargeData->travelPortion() = surchInfo.tvlPortion();
  surchargeData->amountSelected() = surAmount;
  surchargeData->currSelected() = surCurrency;
  surchargeData->currNoDecSelected() = surCurNoDec;
  surchargeData->amountNuc() = surAmountNuc;
  surchargeData->brdAirport() = tSeg.origAirport();
  surchargeData->offAirport() = tSeg.destAirport();
  surchargeData->fcBrdCity() = fcBrdCity;
  surchargeData->fcOffCity() = fcOffCity;
  surchargeData->singleSector() = singleSector;
  surchargeData->geoTblNo() = surchInfo.geoTblItemNo();
  surchargeData->tsi() = _tsi;
  surchargeData->locKey1() = _locKey1;
  surchargeData->locKey2() = _locKey2;
  surchargeData->geoTblNoBtw() = surchInfo.geoTblItemNoBtw();
  surchargeData->tsiBtw() = _tsiBtw;
  surchargeData->locKeyBtw1() = _locKeyBtw1;
  surchargeData->locKeyBtw2() = _locKeyBtw2;
  surchargeData->geoTblNoAnd() = surchInfo.geoTblItemNoAnd();
  surchargeData->tsiAnd() = _tsiAnd;
  surchargeData->locKeyAnd1() = _locKeyAnd1;
  surchargeData->locKeyAnd2() = _locKeyAnd2;

  if (UNLIKELY(surchInfo.surchargeType() == RuleConst::OTHER && fareUsage &&
      //        surchInfo.tvlPortion()    == RuleConst::PERTICKET   &&
      //        fare.isElectronicTktRequired()
      fareUsage->isPaperTktSurchargeMayApply()))
  { // save a PAPER TKT surcharge
    surchargeData->surchargeDesc() = "PAPER TICKET SURCHARGE";
  }
  else // save a regular surcharge description
  {
    surchargeData->surchargeDesc() = surchInfo.surchargeDesc();
  }
  return surchargeData;
}

bool
SurchargesRule::checkDiscountedInfo(const PaxTypeFare& paxTypeFare,
                                    VendorCode& vendor,
                                    MoneyAmount& discPercent) const
{
  try
  {
    if (paxTypeFare.discountInfo().farecalcInd() == RuleConst::CALCULATED)
    {
      vendor = paxTypeFare.vendor();
      discPercent = paxTypeFare.discountInfo().discPercent();
      return true;
    }
    return false;
  }
  catch (...)
  {
  }
  return false;
}

//=======================================================
//  check surchargeData for Side_trip logic
//=======================================================
bool
SurchargesRule::isDataCorrect(const SurchargesInfo& surchInfo)
{
  if (!surchInfo.equipType().empty() || !surchInfo.dow().empty() ||
      surchInfo.todAppl() != RuleConst::BLANK || surchInfo.startYear() || surchInfo.startMonth() ||
      surchInfo.startDay() || surchInfo.stopYear() || surchInfo.stopMonth() ||
      surchInfo.stopDay() || surchInfo.surchargeType() != RuleConst::SIDETRIP ||
      surchInfo.tvlPortion() == RuleConst::PERTRANSFER)
  {
    return false;
  }
  return true;
}

//=======================================================
//  check a Side_Trip
//=======================================================
bool
SurchargesRule::isSideTrip(std::vector<TravelSeg*>::const_iterator itB,
                           std::vector<TravelSeg*>::const_iterator itE,
                           std::vector<TravelSeg*>::const_iterator itBegin,
                           std::vector<TravelSeg*>::const_iterator itLast,
                           bool checkOrigin,
                           std::vector<TravelSeg*>::const_iterator& itRet)
{
  if ((checkOrigin && itB == itBegin) || itB == itLast)
    return false;

  const Loc* locFirst = nullptr;

  if (checkOrigin) // check Origins
  {
    locFirst = (*itB)->origin();
    for (itB++; itB != itE; ++itB)
    {
      if ((*itB)->origin() == locFirst)
      {
        itRet = itB;
        return true;
      }
    }
  }
  else // check Destinations
  {
    locFirst = (*itB)->destination();
    for (itB++; itB != itLast; ++itB)
    {
      if ((*itB)->destination() == locFirst)
      {
        itRet = itB;
        return true;
      }
    }
  }
  return false;
}

//=======================================================
//  check a Side_Trip for the BETWEEN/AND Geo995
//=======================================================
bool
SurchargesRule::isSideTripBtwAnd(std::vector<TravelSeg*>::const_iterator itB,
                                 std::vector<TravelSeg*>::const_iterator itE,
                                 std::vector<TravelSeg*>::const_iterator itBegin,
                                 std::vector<TravelSeg*>::const_iterator itLast,
                                 RuleUtil::TravelSegWrapperVector& secondQualifiedVc,
                                 bool checkOrigin,
                                 std::vector<TravelSeg*>::const_iterator& itRet)
{
  if ((checkOrigin && itB == itBegin) || itB == itLast)
    return false;

  //  count               = 1;
  const Loc* locFirst = nullptr;
  bool matchST = false;
  RuleUtil::TravelSegWrapperVectorCI itBegSec = secondQualifiedVc.begin();
  RuleUtil::TravelSegWrapperVectorCI itEndSec = secondQualifiedVc.end();

  if (checkOrigin) // check Origins
  {
    locFirst = (*itB)->origin();
  }
  else // check Destinations
  {
    locFirst = (*itB)->destination();
  }
  //  for(itB++; itB != itE; ++itB)
  for (itB++; itB != itLast; ++itB) // use itLast when check destination()
  {
    //    count += 1;
    itBegSec = secondQualifiedVc.begin();

    for (; itBegSec != itEndSec; ++itBegSec)
    {
      if (*itB == (*itBegSec)->travelSeg())
      {
        matchST = true;
      }
    }
    if ((*itB)->destination() == locFirst)
    {
      if (matchST)
      {
        itRet = itB;
        return true;
      }
    }
  }
  return false;
}

//=======================================================
//  validatePaxTypeData()
//=======================================================
bool
SurchargesRule::validatePaxTypeData(PricingTrx& trx,
                                    const PaxType* aPax,
                                    const VendorCode vendor,
                                    const SurchargesInfo& surchInfo,
                                    bool dg,
                                    Diag312Collector* diag)
{
  const Indicator& appl = surchInfo.surchargeAppl();

  if (appl != RuleConst::ANY_PAX_SURCHARGE)
  {
    if (vendor == Vendor::ATPCO)
    {
      if (UNLIKELY(appl == RuleConst::CHILD_SURCHARGE && PaxTypeUtil::isAdult(trx, aPax->paxType(), vendor)))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - SKIP:" << std::endl;
          *diag << "                     FOR CHILD ONLY" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
      if (UNLIKELY(appl == RuleConst::ADT_SURCHARGE && !PaxTypeUtil::isAdult(trx, aPax->paxType(), vendor)))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - SKIP:" << std::endl;
          *diag << "                     FOR ADULT ONLY" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
    }
    if (vendor == Vendor::SITA)
    {
      if (PaxTypeUtil::isAdult(trx, aPax->paxType(), vendor) &&
          (appl == RuleConst::CHARGE_PAX_CHILD || appl == RuleConst::CHARGE_PAX_INFANT))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - SKIP:" << std::endl;
          *diag << "                     FOR CHD-INF ONLY" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
      if (PaxTypeUtil::isChild(trx, aPax->paxType(), vendor) &&
          (appl == RuleConst::CHARGE_PAX_ADULT || appl == RuleConst::CHARGE_PAX_INFANT))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - SKIP:" << std::endl;
          *diag << "                     FOR ADT-INF ONLY" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
      if (PaxTypeUtil::isInfant(trx, aPax->paxType(), vendor) &&
          (appl == RuleConst::CHARGE_PAX_ADULT || appl == RuleConst::CHARGE_PAX_CHILD ||
           appl == RuleConst::CHARGE_PAX_ADULT_CHILD))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - SKIP:" << std::endl;
          *diag << "                     FOR ADT-CHD ONLY" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
      if (PaxTypeUtil::isInfant(trx, aPax->paxType(), vendor) &&
          (appl == RuleConst::CHARGE_PAX_ADULT_CHILD_INFANT_FREE ||
           appl == RuleConst::CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE))
      {
        if (UNLIKELY(dg && diag->isActive()))
        {
          *diag << "    SURCHARGE RULE - PASS:" << std::endl;
          *diag << "                     INFANT FREE" << std::endl;
          diag->flushMsg();
        }
        return false;
      }
    }
  }
  return true;
}

//==================================================
//  process NON_Side_Trip
//==================================================
Record3ReturnTypes
SurchargesRule::validateNonSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                                    PricingTrx& trx,
                                    FarePath* farePath,
                                    FareUsage* fareUsage,
                                    const PaxTypeFare& fare,
                                    const SurchargesInfo& surchInfo,
                                    RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                    Diag312Collector* diag)
{
  bool haveSurcharge = false;
  bool collectionDone = false;
  bool ptsSkipped = false;
  bool ptsOverride = trx.getRequest()->isPtsOverride();

  // looping through each segment for the current Fare Component scope
  //
  std::vector<TravelSeg*>::const_iterator itB = fare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itE = fare.fareMarket()->travelSeg().end();
  std::vector<TravelSeg*>::const_iterator itL = itE - 1;
  std::vector<TravelSeg*>::const_iterator itBegin = itB;

  _perTransMatchedSeg = false; // the last matched Segment condition for  PERTRANSFER (refactoring)

  bool firstTravelSegment = true;
  NoPNRPricingTrx* noPnrTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  SurchargeData* surchargeData = nullptr;
  bool newSurchargeAndNoPerTransfer;

  const Indicator tvlPortion = getTravelPortion(surchInfo, trx);

  newSurchargeAndNoPerTransfer = (tvlPortion != RuleConst::PERTRANSFER);

  const bool hkgException = hongKongException(trx, fare.fareMarket()->travelSeg(), surchInfo);

  for (; itB != itE; ++itB)
  {
    // skip arunk segment if T995
    if ((*itB)->segmentType() == Arunk && tvlPortion == RuleConst::PERCOUPON)
      continue;

    RuleUtil::TravelSegWrapperVectorCI itBegAppl = applTravelSegment.begin();
    RuleUtil::TravelSegWrapperVectorCI itEndAppl = applTravelSegment.end();
    RuleUtil::TravelSegWrapperVectorCI iLastAppl = itEndAppl - 1;

    for (; itBegAppl != itEndAppl; ++itBegAppl)
    {
      bool matchedCurrentSeg = true;
      if (*itB == (*itBegAppl)->travelSeg())
      {
        bool isFirstSegment = firstTravelSegment;

        if (!surchInfo.equipType().empty())
        {
          AirSeg* segment = nullptr;
          segment = dynamic_cast<AirSeg*>(*itB);
          if (LIKELY(segment && segment->flightNumber() != 0 && !segment->equipmentType().empty()))
          {
            if (surchInfo.equipType() != segment->equipmentType() &&
                (fare.carrier() != RuleConst::AA_CARRIER ||
                 surchInfo.equipType() != RuleConst::M80_EQUIPMENT_TYPE ||
                 segment->equipmentType() != RuleConst::S80_EQUIPMENT_TYPE))
            {
              matchedCurrentSeg = false;
              if (UNLIKELY(noPnrTrx))
                fare.warningMap().set(WarningMap::cat12_warning);
            }
          }
          else
          {
            matchedCurrentSeg = false;
            if (noPnrTrx)
              fare.warningMap().set(WarningMap::cat12_warning);
          }
        }
        else
          firstTravelSegment = false;

        if (matchedCurrentSeg && hkgException)
        {
           //APO40975: the sector has matched. if the tvl portion is per coupon then add surcharge
           //for all surcharge types. for non coupon surcharges turn off the matched seg flag.
           if (tvlPortion == RuleConst::PERCOUPON)
           {
                // leave the segment as matched
           }
           else
           {
              if ((*itB)->origAirport() != "HKG")
                  matchedCurrentSeg = false;
           }
        }
        bool origCheck = (*itBegAppl)->origMatch();
        bool destCheck = (*itBegAppl)->destMatch();

        // check date range: Start/Stop dates
        if (matchedCurrentSeg && !checkDateRange(trx, surchInfo, (**itB), origCheck, noPnrTrx))
        {
          // WQ - if calculated date didn't match and no date entered for segment - log warning
          if (UNLIKELY(noPnrTrx && (**itB).hasEmptyDate()))
            fare.warningMap().set(WarningMap::cat12_warning);

          matchedCurrentSeg = false;
        }

        // check DayOfWeek and Times
        if (matchedCurrentSeg &&
            !checkDOWandTime(trx, surchInfo, (**itB), origCheck, isFirstSegment, noPnrTrx))
        {
          // WQ - if calculated date didn't match and no date entered for segment - log warning
          if (UNLIKELY(noPnrTrx && (**itB).hasEmptyDate()))
            fare.warningMap().set(WarningMap::cat12_warning);

          matchedCurrentSeg = false;
        }

        // if processed RBD/T986 - apply on each matched coupon/transfer
        if (UNLIKELY(matchedCurrentSeg && !_matchedRbdSegs.empty() &&
            std::find(_matchedRbdSegs.begin(), _matchedRbdSegs.end(), *itB) ==
                _matchedRbdSegs.end()))
        {
          matchedCurrentSeg = false;
        }

        if (matchedCurrentSeg && !_matched986Segs.empty() &&
            std::find(_matched986Segs.begin(), _matched986Segs.end(), *itB) ==
                _matched986Segs.end())
        {
          matchedCurrentSeg = false;
        }

        if (tvlPortion != RuleConst::PERTRANSFER)
        {
          // check next segment, otherwise add surcharge
          if (!matchedCurrentSeg)
            continue;
        }
        // for "PERTRANSFER"
        else
        {
          bool isToFromViaOnly = surchInfo.geoTblItemNo() && !surchInfo.geoTblItemNoAnd() &&
                                 !surchInfo.geoTblItemNoBtw();
          if (isToFromViaOnly)
          {
            if (!validatePerTransfer(
                    itB, itBegin, itL, matchedCurrentSeg, origCheck, destCheck, trx))
              continue;
          }
          else
          {
            RuleUtil::TravelSegWrapperVectorCI itFirst = applTravelSegment.begin();
            if (!validatePerTransfer(
                    itBegAppl, itFirst, iLastAppl, matchedCurrentSeg, origCheck, destCheck, trx))
              continue;
          }
        }

        // send a trailer MSG
        if (surchInfo.surchargeType() == RuleConst::OTHER)
        {
          if (UNLIKELY(!farePath && fare.isPaperTktSurchargeMayApply()))
          {
            if (UNLIKELY(diag))
              *diag << "FC SCOPE VALIDATION SOFTPASS - PAPER TICKET SURCHARGE";

            return SOFTPASS;
          }
          if (UNLIKELY(fareUsage && fareUsage->isPaperTktSurchargeMayApply()))
          {
            if (ptsOverride)
            {
              ptsSkipped = true;
              haveSurcharge = true;
              break; /// Exit the loop PTS must not be applied
            }

            if (farePath->paperTktSurcharge().empty())
            {
              farePath->paperTktSurcharge() = "PAPER TICKET SURCHARGE APPLIES";
            }
          }
        }

        if (!fallback::ssdsp1836surchWithGeo(&trx)) // solution with previous partial fixes.
        {
          if (!haveSurcharge && surchInfo.equipType().empty() && _matchedRbdSegs.empty() &&
              _matched986Segs.empty() &&  surchInfo.geoTblItemNo() == 0 &&
              itBegAppl != applTravelSegment.begin() &&
              (!surchInfo.geoTblItemNoAnd() || !surchInfo.geoTblItemNoBtw() ||
               surchInfo.sectorPortion() != SectorPortionPortion))
          {
            // Blank, 1, 2, 4, 6 Departure date from the origin of the fare component being validated.
            if (tvlPortion == RuleConst::ONEWAY || tvlPortion == RuleConst::ROUNDTRIP ||
                tvlPortion == RuleConst::PERTICKET || tvlPortion == RuleConst::PERDIRECTION ||
                tvlPortion == RuleConst::BLANK)
            {
              collectionDone = true;
              haveSurcharge = false;
              break;
            }
          }
        }
        else if (!haveSurcharge && surchInfo.equipType().empty() && _matchedRbdSegs.empty() &&
                 _matched986Segs.empty() &&  surchInfo.geoTblItemNo() == 0)
        {
          // Blank, 1, 2, 4, 6 Departure date from the origin of the fare component being validated.
          if (tvlPortion == RuleConst::ONEWAY || tvlPortion == RuleConst::ROUNDTRIP ||
              tvlPortion == RuleConst::PERTICKET || tvlPortion == RuleConst::PERDIRECTION ||
              tvlPortion == RuleConst::BLANK)
          {
            if (itBegAppl != applTravelSegment.begin())
            {
              collectionDone = true;
              haveSurcharge = false;
              break;
            }
          }
        }

        if (newSurchargeAndNoPerTransfer && !TrxUtil::isFVOSurchargesEnabled())
        {
          if (UNLIKELY(haveSurcharge && diag && diag->isActive()))
            diag->displaySurchargeData(
                trx, surchargeData, surchInfo, fareUsage->isPaperTktSurchargeMayApply());

          surchargeData = constructSurcharge(trx);
        }
        SurchargeSeg surchargeSeg;

        // really funny part below, gold medal to someone responsible
        // for those two projects implemented concurently...
        if (UNLIKELY(TrxUtil::isFVOSurchargesEnabled()))
        {
          if (!farePath &&
              (tvlPortion == RuleConst::PERTICKET || tvlPortion == RuleConst::PERDIRECTION))
          {
            if (UNLIKELY(diag))
            {
              *diag << "FC SCOPE VALIDATION SOFTPASS - "
                    << (tvlPortion == RuleConst::PERTICKET ? "PER TICKET SURCHARGE\n"
                                                           : "PER DIRECTION SURCHARGE\n");
            }
            return SOFTPASS;
          }
          if (tvlPortion == RuleConst::PERTRANSFER)
          {
            addSurchargeForPerTransfer(trx, surchargeSegments, itB, false, origCheck, destCheck);
          }
          else if (tvlPortion == RuleConst::PERCOUPON)
          {
            surchargeSeg._ts = *itB;
            surchargeSeg._fcBrdCity = &(*itB)->boardMultiCity();
            surchargeSeg._fcOffCity = &(*itB)->offMultiCity();
            surchargeSeg._singleSector = true;
          }
          else if (tvlPortion == RuleConst::PERTICKET)
          {
            travelSegConstIt seg =
                getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
            surchargeSeg._ts = *seg;
            surchargeSeg._fcBrdCity = &farePath->itin()->travelSeg().front()->boardMultiCity();
            surchargeSeg._fcOffCity = &farePath->itin()->travelSeg().back()->offMultiCity();
            surchargeSeg._singleSector = farePath->itin()->travelSeg().size() == 1;
          }
          else if (tvlPortion == RuleConst::PERDIRECTION)
          {
            travelSegConstIt seg =
                getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
            surchargeSeg._ts = *seg;
            surchargeSeg._fcBrdCity = &fare.fareMarket()->travelSeg().front()->boardMultiCity();
            surchargeSeg._fcOffCity = &fare.fareMarket()->travelSeg().back()->offMultiCity();
            surchargeSeg._singleSector = fare.fareMarket()->travelSeg().size() == 1;
          }
          else
          {
            if (!haveSurcharge)
            {
              travelSegConstIt seg =
                  getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
              surchargeSeg._ts = *seg;
              surchargeSeg._fcBrdCity = &(*itB)->boardMultiCity();
              surchargeSeg._fcOffCity = &(*itB)->offMultiCity();
              surchargeSeg._singleSector = true;
              surchargeSegments.push_back(surchargeSeg);
              newSurchargeAndNoPerTransfer = false;
            }
            else
            {
              surchargeSegments.back()._fcOffCity = &(*itB)->offMultiCity();
              surchargeSegments.back()._singleSector = false;
            }
          }
          haveSurcharge = true;
        }
        else
        {
          if (tvlPortion == RuleConst::PERTRANSFER)
          {
            addSurchargeForPerTransfer(
                trx, farePath, fare, fareUsage, surchInfo, itB, false, origCheck, destCheck, diag);
          }
          else if (tvlPortion == RuleConst::PERCOUPON)
          {
            addSurcharge(trx,
                         farePath,
                         fare,
                         fareUsage,
                         surchInfo,
                         surchargeData,
                         **itB,
                         (*itB)->boardMultiCity(),
                         (*itB)->offMultiCity(),
                         true);
          }
          else if (tvlPortion == RuleConst::PERTICKET)
          {
            travelSegConstIt seg =
                getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
            addSurcharge(trx,
                         farePath,
                         fare,
                         fareUsage,
                         surchInfo,
                         surchargeData,
                         **seg,
                         farePath->itin()->travelSeg().front()->boardMultiCity(),
                         farePath->itin()->travelSeg().back()->offMultiCity(),
                         farePath->itin()->travelSeg().size() == 1);
          }
          else if (tvlPortion == RuleConst::PERDIRECTION)
          {
            travelSegConstIt seg =
                getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
            addSurcharge(trx,
                         farePath,
                         fare,
                         fareUsage,
                         surchInfo,
                         surchargeData,
                         **seg,
                         fare.fareMarket()->travelSeg().front()->boardMultiCity(),
                         fare.fareMarket()->travelSeg().back()->offMultiCity(),
                         fare.fareMarket()->travelSeg().size() == 1);
          }
          else
          {
            if (!haveSurcharge)
            {
              travelSegConstIt seg =
                  getGoverningCarrierIfAvaliable(surchInfo, fare, itB, hkgException, true);
              addSurcharge(trx,
                           farePath,
                           fare,
                           fareUsage,
                           surchInfo,
                           surchargeData,
                           **seg,
                           (*itB)->boardMultiCity(),
                           (*itB)->offMultiCity(),
                           true);
              newSurchargeAndNoPerTransfer = false;
              fareUsage->surchargeData().push_back(surchargeData);
            }
            else
            {
              surchargeData->fcOffCity() = (*itB)->offMultiCity();
              surchargeData->singleSector() = false;
            }
          }
          haveSurcharge = true;
        }

        // lint --e{413}
        if (newSurchargeAndNoPerTransfer)
        {
          if (UNLIKELY(TrxUtil::isFVOSurchargesEnabled()))
            surchargeSegments.push_back(surchargeSeg);
          else
            fareUsage->surchargeData().push_back(surchargeData);
        }

        // stop only on PERTICKEt
        if (tvlPortion == RuleConst::PERTICKET || tvlPortion == RuleConst::PERDIRECTION)
        {
          collectionDone = true;
          break;
        }

      } // matching segment pointers
    } // internel loop through vector of *segments from 995 table
    if (collectionDone || ptsSkipped)
    {
      break;
    }
  } // loop through segments in FC

  if (UNLIKELY(diag && diag->isActive()))
  {
    if (haveSurcharge)
    {
      if (ptsSkipped)
      {
        *diag << "    PAPER TICKET SURCHARGE - SKIPPED " << std::endl;
      }
      else if (haveSurcharge && diag && diag->isActive() && !TrxUtil::isFVOSurchargesEnabled() &&
               tvlPortion != RuleConst::PERTRANSFER)
      {
        diag->displaySurchargeData(
            trx, surchargeData, surchInfo, fareUsage->isPaperTktSurchargeMayApply());
      }
      diag->flushMsg();
    }
    else
    {
      *diag << "    SURCHARGE RULE - SKIP " << std::endl;
      diag->flushMsg();
    }
  }

  // WQ - clear any cat12 warning messages if surcharge applied
  if (UNLIKELY(noPnrTrx && haveSurcharge))
    fare.warningMap().set(WarningMap::cat12_warning, false);

  return haveSurcharge ? PASS : SKIP;
}

//=======================================================
//  process Side_Trip for the BETWEEN/AND Geo995
//=======================================================
bool
SurchargesRule::processSideTrip(std::vector<SurchargeSeg>& surchargeSegments,
                                PricingTrx& trx,
                                const FarePath* farePath,
                                FareUsage* fareUsage,
                                const PaxTypeFare& fare,
                                const SurchargesInfo& surchInfo,
                                RuleUtil::TravelSegWrapperVector& firstQualifiedVc,
                                RuleUtil::TravelSegWrapperVector& secondQualifiedVc,
                                Diag312Collector* diag)
{
  bool isSTSurcharge = false;

  std::vector<TravelSeg*>::const_iterator itB = fare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator itE = fare.fareMarket()->travelSeg().end();
  std::vector<TravelSeg*>::const_iterator itL = itE - 1;
  std::vector<TravelSeg*>::const_iterator itBegin = itB;

  const bool isFVOSurcharges = TrxUtil::isFVOSurchargesEnabled();

  for (; itB != itE; ++itB)
  {
    bool localSideTrip = false;

    RuleUtil::TravelSegWrapperVectorCI itBegAppl = firstQualifiedVc.begin();
    RuleUtil::TravelSegWrapperVectorCI itEndAppl = firstQualifiedVc.end();

    for (; itBegAppl != itEndAppl; ++itBegAppl)
    {
      if (*itB == (*itBegAppl)->travelSeg())
      {
        localSideTrip = false;
        std::vector<TravelSeg*>::const_iterator itRet;

        if ((*itBegAppl)->origMatch())
        {
          if (isSideTripBtwAnd(itB, itE, itBegin, itL, secondQualifiedVc, true, itRet))
          {
            if (isFVOSurcharges)
              surchargeSegments.push_back(SurchargeSeg(*itB,
                                                       &(*itB)->boardMultiCity(),
                                                       &(*itRet)->offMultiCity(),
                                                       (*itB) == (*itRet),
                                                       &(**itB).origin()->loc()));
            else
            {
              SurchargeData* surchargeData = constructSurcharge(trx);
              addSurcharge(trx,
                           farePath,
                           fare,
                           fareUsage,
                           surchInfo,
                           surchargeData,
                           (**itB),
                           (*itB)->boardMultiCity(),
                           (*itRet)->offMultiCity(),
                           (*itB) == (*itRet));

              if (UNLIKELY(diag && diag->isActive()))
              {
                diag->displaySideTripSurchargeData(surchargeData, (*itB)->origin()->loc());
              }

              // lint -e{413}
              fareUsage->surchargeData().push_back(surchargeData);
            }

            isSTSurcharge = true;
            localSideTrip = true;
          }
        }
        // some Geo995's (with TSI) return both "orig" and "dest" indicators
        if (!localSideTrip)
        {
          if ((*itBegAppl)->destMatch())
          {
            if (isSideTripBtwAnd(itB, itE, itBegin, itL, secondQualifiedVc, false, itRet))
            {
              if (isFVOSurcharges)
                surchargeSegments.push_back(SurchargeSeg(*itB,
                                                         &(*itB)->boardMultiCity(),
                                                         &(*itRet)->offMultiCity(),
                                                         (*itB) == (*itRet),
                                                         &(**itB).destination()->loc()));
              else
              {
                SurchargeData* surchargeData = constructSurcharge(trx);
                addSurcharge(trx,
                             farePath,
                             fare,
                             fareUsage,
                             surchInfo,
                             surchargeData,
                             (**itB),
                             (*itB)->boardMultiCity(),
                             (*itRet)->offMultiCity(),
                             (*itB) == (*itRet));

                if (UNLIKELY(diag && diag->isActive()))
                {
                  diag->displaySideTripSurchargeData(surchargeData, (*itB)->destination()->loc());
                }
                // lint --e{413}
                fareUsage->surchargeData().push_back(surchargeData);
              }

              isSTSurcharge = true;
              localSideTrip = true;
            }
          }
        }
        if (isSTSurcharge)
        {
          break; // job is done for the current travel segment within FC
        }
      } // *iT ==*itBegAppl
    } // for (++itBegappl)
  } // for (++itB)

  return isSTSurcharge;
}

bool
SurchargesRule::matchSegmentGoverningCarrier(const CarrierCode& carrier, const TravelSeg* travelSeg)
    const
{
  if (LIKELY(travelSeg->isAir()))
  {
    return (carrier == (static_cast<const AirSeg*>(travelSeg))->carrier());
  }
  return true;
}

Indicator
SurchargesRule::getTravelPortion(const SurchargesInfo& surchInfo, const PricingTrx& trx) const
{
  if (UNLIKELY(trx.getOptions() && trx.getOptions()->isRtw() &&
      surchInfo.tvlPortion() == RuleConst::PERDIRECTION))
  {
    return RuleConst::ONEWAY;
  }

  return surchInfo.tvlPortion();
}

BookingCode
SurchargesRule::getBookingCode(PricingTrx& trx, PaxTypeFare& fare, const FareUsage* fUsagep) const
{
  FareDisplayTrx* fareDisplayTrx = dynamic_cast<FareDisplayTrx*>(&trx);

  if (!fareDisplayTrx)
  {
    // the primary sector could be rebooked on a wpnc. So return correct booking code
    if (trx.getRequest()->isLowFareRequested() && fUsagep)
    {
      int psectorPNRSegment = fare.fareMarket()->primarySector()->pnrSegment();
      size_t index = 0;
      for (TravelSeg* tvlSeg : fUsagep->travelSeg())
      {
        if ((tvlSeg->pnrSegment() == psectorPNRSegment) &&
            (!fUsagep->segmentStatus()[index]._bkgCodeReBook.empty()) &&
            (fUsagep->segmentStatus()[index]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED)))
          return fUsagep->segmentStatus()[index]._bkgCodeReBook;
        index++;
      }
    }
    return fare.fareMarket()->primarySector()->getBookingCode();
  }
  else
  {
    FareDisplayBookingCode fdbc;

    if (fare.bookingCode().empty())
    {
      fdbc.getBookingCode(*fareDisplayTrx, fare, fare.bookingCode());
    }
    return fare.bookingCode();
  }
}

bool
SurchargesRule::hongKongException(PricingTrx& trx,
                                  const std::vector<TravelSeg*>& travelSegs,
                                  const SurchargesInfo& surchInfo) const
{
  if (surchInfo.geoTblItemNo() ||
      surchInfo.geoTblItemNoAnd() ||
      surchInfo.geoTblItemNoBtw() ||
      !_matched986Segs.empty() ||
      !_matchedRbdSegs.empty() ||
      !surchInfo.equipType().empty() ||
      (surchInfo.surchargeCur1() != "HKD" && surchInfo.surchargeCur2() != "HKD"))
  {
    return false;
  }

  std::vector<TravelSeg*>::const_iterator itB = travelSegs.begin();
  std::vector<TravelSeg*>::const_iterator itE = travelSegs.end();

  for (; itB != itE; ++itB)
  {
    if ((*itB)->origAirport() == "HKG")
      return true;
  }
  return false;
}
bool
SurchargesRule::shouldMatchDOW(Indicator tvlPortion, bool firstSeg) const
{
  // Blank, 1, 2, 4, 6 - Departure day from the origin of the fare component being validated.
  if (tvlPortion == RuleConst::BLANK || tvlPortion == RuleConst::ONEWAY ||
      tvlPortion == RuleConst::ROUNDTRIP || tvlPortion == RuleConst::PERTICKET ||
      tvlPortion == RuleConst::PERDIRECTION)
    return firstSeg;

  // 3 - Departure day from each intermediate ticketed point on the fare component being validated.
  if (tvlPortion == RuleConst::PERTRANSFER)
    return !firstSeg;

  // 5 - Departure day from the origin of each sector on the fare component being validated.
  if (tvlPortion == RuleConst::PERCOUPON)
    return true;

  return false;
}
std::vector<TravelSeg*>::const_iterator
SurchargesRule::findGoverningCarrierSegment(const std::vector<TravelSeg*>& tvlSegs,
                                            std::vector<TravelSeg*>::const_iterator seg,
                                            const CarrierCode govCxr)
{
  bool findPassedSeg = false;
  std::vector<TravelSeg*>::const_iterator ret = seg;
  std::vector<TravelSeg*>::const_iterator it = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator ie = tvlSegs.end();
  for (; it != ie; it++)
  {
    if (it == seg)
      findPassedSeg = true;

    AirSeg* aSeg = dynamic_cast<AirSeg*>(*it);
    if (!aSeg)
      continue;

    if (aSeg->carrier() == govCxr)
    {
      // if we found segment, and match on governing carrier, then we finish
      if (LIKELY(findPassedSeg))
        return it;
      // if governing carrier segment is before passed segment
      ret = it;
    }
  }
  return ret;
}

SurchargesRule::travelSegConstIt
SurchargesRule::getGoverningCarrierIfAvaliable(const SurchargesInfo& surchInfo,
                                               const PaxTypeFare& fare,
                                               travelSegConstIt it,
                                               bool hkgExcept,
                                               bool checkGeoAndBtw)
{
  bool findGoverningCrxFlag;
  if (LIKELY(checkGeoAndBtw))
  {
    findGoverningCrxFlag =
        !hkgExcept
        // no TSI
        &&
        (!_tsi && !_tsiAnd && !_tsiBtw)
        // no GEOLOC (To/From/Via - Btw/And)
        &&
        (!surchInfo.geoTblItemNo() && !surchInfo.geoTblItemNoAnd() && !surchInfo.geoTblItemNoBtw())
        // no Equipment
        &&
        surchInfo.equipType().empty()
        // no Carrier/Flight Table
        &&
        !surchInfo.carrierFltTblItemNo()
         // no Sector/Portion
        &&
        (surchInfo.sectorPortion() == RuleConst::BLANK)
        // no RBD
        &&
        surchInfo.bookingCode().empty();
  }
  else
    findGoverningCrxFlag = !hkgExcept && !surchInfo.geoTblItemNo();

  travelSegConstIt seg = it;
  if (findGoverningCrxFlag)
  {
    if (!matchSegmentGoverningCarrier(fare.fareMarket()->governingCarrier(), *it))
    {
      seg = findGoverningCarrierSegment(
          fare.fareMarket()->travelSeg(), it, fare.fareMarket()->governingCarrier());
    }
  }
  return seg;
}

bool
SurchargesRule::isNegativeSurcharge(const SurchargesInfo* surchInfo,
                                    DiagManager& diag,
                                    bool isDiagActive) const
{
  bool isNegative = false;
  if (UNLIKELY(isalpha(surchInfo->surchargeAppl())))
  {
    isNegative = true;
    if (UNLIKELY(isDiagActive))
    {
      diag << "    SURCHARGE RULE - SKIP BY NEGATIVE SURCHARGE\n";
    }
  }
  return isNegative;
}

SurchargeData*
SurchargesRule::constructSurcharge(PricingTrx& trx)
{
  SurchargeData* surcharge;
  if (LIKELY(trx.getTrxType() == PricingTrx::MIP_TRX || trx.isShopping()))
  {
    trx.dataHandle().get(surcharge);
  }
  else
  {
    surcharge = trx.constructSurchargeData();
  }
  return surcharge;
}

void
SurchargesRule::addSurchargeForPerTransfer(PricingTrx& trx,
                                           const FarePath* farePath,
                                           const PaxTypeFare& fare,
                                           FareUsage* fareUsage,
                                           const SurchargesInfo& surchInfo,
                                           travelSegConstIt currentSegIt,
                                           bool singleSector,
                                           bool origCheck,
                                           bool destCheck,
                                           Diag312Collector* diag)
{
  if (origCheck &&
      (_transferPointDest.empty() || _transferPointDest != (*currentSegIt)->boardMultiCity()))
  {
    SurchargeData* surchargeData = constructSurcharge(trx);
    addSurcharge(trx,
                 farePath,
                 fare,
                 fareUsage,
                 surchInfo,
                 surchargeData,
                 **currentSegIt,
                 (*(currentSegIt - 1))->boardMultiCity(),
                 (*currentSegIt)->offMultiCity(),
                 false);

    fareUsage->surchargeData().push_back(surchargeData);

    if (UNLIKELY(diag && diag->isActive()))
      diag->displaySurchargeData(
          trx, surchargeData, surchInfo, fareUsage->isPaperTktSurchargeMayApply());
  }

  if (destCheck)
  {
    SurchargeData* surchargeData = constructSurcharge(trx);
    addSurcharge(trx,
                 farePath,
                 fare,
                 fareUsage,
                 surchInfo,
                 surchargeData,
                 **currentSegIt,
                 (*currentSegIt)->boardMultiCity(),
                 (*(currentSegIt + 1))->offMultiCity(),
                 false);

    fareUsage->surchargeData().push_back(surchargeData);
    _transferPointDest = (*currentSegIt)->offMultiCity();

    if (UNLIKELY(diag && diag->isActive()))
      diag->displaySurchargeData(
          trx, surchargeData, surchInfo, fareUsage->isPaperTktSurchargeMayApply());
  }
  else
    _transferPointDest.clear();
}

void
SurchargesRule::addSurchargeForPerTransfer(PricingTrx& trx,
                                           std::vector<SurchargeSeg>& surchargeSegments,
                                           travelSegConstIt currentSegIt,
                                           bool singleSector,
                                           bool origCheck,
                                           bool destCheck)
{
  if (origCheck &&
      (_transferPointDest.empty() || _transferPointDest != (*currentSegIt)->boardMultiCity()))
  {
    SurchargeSeg surchargeSeg;
    surchargeSeg._ts = *currentSegIt;
    surchargeSeg._fcBrdCity = &(*(currentSegIt - 1))->boardMultiCity();
    surchargeSeg._fcOffCity = &(*currentSegIt)->offMultiCity();
    surchargeSeg._singleSector = false;

    surchargeSegments.push_back(surchargeSeg);
  }

  if (destCheck)
  {
    SurchargeSeg surchargeSeg;
    surchargeSeg._ts = *currentSegIt;
    surchargeSeg._fcBrdCity = &(*currentSegIt)->boardMultiCity();
    surchargeSeg._fcOffCity = &(*(currentSegIt + 1))->offMultiCity();
    surchargeSeg._singleSector = false;

    surchargeSegments.push_back(surchargeSeg);
    _transferPointDest = (*currentSegIt)->offMultiCity();
  }
  else
    _transferPointDest.clear();
}
}
