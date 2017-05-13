//----------------------------------------------------------------------------
//
//  File:           RuleUtil.cpp
//
//  Copyright Sabre 2004
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------
#include "Rules/RuleUtil.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "Common/VecSet.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangeOverrides.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Itin.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/SurchargeData.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FareFocusDaytimeApplDetailInfo.h"
#include "DBAccess/FareFocusDaytimeApplInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/GeoRuleItem.h"
#include "DBAccess/HipMileageExceptInfo.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/JointCarrier.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/SamePoint.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/SectorSurcharge.h"
#include "DBAccess/Tours.h"
#include "DBAccess/TSIInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag202Collector.h"
#include "Diagnostic/Diag225Collector.h"
#include "Diagnostic/Diag335Collector.h"
#include "Diagnostic/Diag502Collector.h"
#include "Diagnostic/Diag512Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/CalcTotals.h"
#include "MinFares/MinFareLogic.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleConst.h"
#include "Rules/SecSurchargeAppl.h"
#include "Util/BranchPrediction.h"
#include "Util/IteratorRange.h"

#include <vector>
#include <algorithm>

#include <ctype.h>

namespace tse
{
FIXEDFALLBACK_DECL(APO29538_StopoverMinStay);
FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck);
FIXEDFALLBACK_DECL(fallbackDisableESVIS);
FALLBACK_DECL(fallbackFootNoteR2Optimization);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(perTicketSurchargeFix);

Logger
RuleUtil::_logger("atseintl.Rules.RuleUtil");

namespace
{
// Bound Fare usage
BindingResult
checkBindings(const PricingTrx& trx,
              const PaxTypeFare& paxTypeFare,
              const CategoryRuleInfo& item,
              bool& isLocationSwapped,
              MATCHTYPE matchType)
{
  BindingResult result(
      paxTypeFare.fare()->_fareInfo->checkBindings(trx, item, isLocationSwapped, matchType));
  if (UNLIKELY(result.second))
  {
    isLocationSwapped ^= paxTypeFare.isReversed();
  }
  return result;
}

template <typename T>
BindingResultCRIP
checkBindings(const PricingTrx& trx,
              const PaxTypeFare& paxTypeFare,
              int cat,
              const std::vector<T*>& catRuleInfoList,
              bool& bLocationSwapped,
              MATCHTYPE matchType)
{
  BindingResultCRIP result(paxTypeFare.fare()->_fareInfo->checkBindings(
      trx, cat, catRuleInfoList, bLocationSwapped, matchType));
  if (UNLIKELY(result.second != nullptr))
  {
    bLocationSwapped ^= paxTypeFare.isReversed();
  }
  return result;
}

// comparator to mimic ORDER BY SEQNO, CREATEDATE
// TODO: if common data is moved to base class CategoryRuleInfo, then no template is necessary
template <typename T>
struct CategoryRuleInfoComp
    : public std::binary_function<std::pair<const T*, bool>, std::pair<const T*, bool>, bool>
{
  bool operator()(const std::pair<const T*, bool>& lhs, const std::pair<const T*, bool>& rhs)
  {
    if (lhs.first->sequenceNumber() < rhs.first->sequenceNumber())
      return true;
    else if (lhs.first->sequenceNumber() == rhs.first->sequenceNumber())
    {
      if (lhs.first->createDate() < rhs.first->createDate())
        return true;
    }

    return false;
  }
};
}

const Indicator RuleUtil::BLANK_INDICATOR;
const int32_t RuleUtil::DEFAULT_TEXT_TBL_ITEM_NO;
const Indicator RuleUtil::LOGIC_AND;
const Indicator RuleUtil::LOGIC_OR;

int32_t
RuleUtil::getCat17Table996ItemNo(PricingTrx& trx,
                                 const PaxTypeFare& paxTypeFare,
                                 const PaxTypeFare*& paxFareForCat17)
{
  paxFareForCat17 = &paxTypeFare;

  // Special check for Cat25 fare
  if (paxTypeFare.isFareByRule())
  {
    const FBRPaxTypeFareRuleData* fbrPaxTypeFare =
        paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (LIKELY(fbrPaxTypeFare != nullptr))
    {
      const FareByRuleItemInfo* fbrItemInfo =
          dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());
      if (fbrItemInfo != nullptr && !fbrPaxTypeFare->isSpecifiedFare() && // Cat25 with base fare
          (fbrItemInfo->ovrdcat17() == 'B')) // Use base fare rule for Cat17
      {
        paxFareForCat17 = fbrPaxTypeFare->baseFare();
      }
    }
  }

  const std::vector<GeneralFareRuleInfo*>* gfrList =
      &(trx.dataHandle().getGeneralFareRule(paxFareForCat17->vendor(),
                                            paxFareForCat17->carrier(),
                                            paxFareForCat17->tcrRuleTariff(),
                                            paxFareForCat17->ruleNumber(),
                                            RuleConst::HIP_RULE, // Cat17
                                            paxFareForCat17->fareMarket()->travelDate()));

  if (gfrList->empty())
  {
    // Try General Rule if there is no Fare Rule found
    GeneralRuleApp* genRule =
        getGeneralRuleApp(trx, *(const_cast<PaxTypeFare*>(paxFareForCat17)), RuleConst::HIP_RULE);
    if (genRule != nullptr)
    {
      gfrList = &(trx.dataHandle().getGeneralFareRule(paxFareForCat17->vendor(),
                                                      paxFareForCat17->carrier(),
                                                      genRule->generalRuleTariff(),
                                                      genRule->generalRule(),
                                                      RuleConst::HIP_RULE, // Cat17
                                                      paxFareForCat17->fareMarket()->travelDate()));
    }
  }

  bool isLocationSwapped;
  std::vector<GeneralFareRuleInfo*>::const_iterator gfrIter = gfrList->begin();
  for (; gfrIter != gfrList->end(); gfrIter++)
  {
    // isLocationSwapped = false;
    GeneralFareRuleInfo& curGfr = **gfrIter;

    if (!RuleUtil::matchGeneralFareRule(
            trx, *(const_cast<PaxTypeFare*>(paxFareForCat17)), curGfr, isLocationSwapped))
      continue;

    // Check the matched Rec2 info
    const std::vector<CategoryRuleItemInfoSet*>& catRuleItemSet = curGfr.categoryRuleItemInfoSet();
    if (catRuleItemSet.size() != 1)
      return RuleUtil::DEFAULT_TEXT_TBL_ITEM_NO;

    const std::vector<CategoryRuleItemInfo>& catRuleItems =
        *catRuleItemSet.front();
    if (catRuleItems.size() == 1)
    {
      const RuleItemInfo* ruleItemInfo = getRuleItemInfo(trx, *gfrIter, &catRuleItems.front());
      if (ruleItemInfo != nullptr)
      {
        const HipMileageExceptInfo* hipMileageExceptInfo =
            dynamic_cast<const HipMileageExceptInfo*>(ruleItemInfo);
        if ((hipMileageExceptInfo != nullptr) &&
            (hipMileageExceptInfo->connectStopInd() == BLANK_INDICATOR) &&
            (hipMileageExceptInfo->noHipInd() == BLANK_INDICATOR) &&
            (hipMileageExceptInfo->loc1().locType() == BLANK_INDICATOR) &&
            hipMileageExceptInfo->loc1().loc().empty() &&
            (hipMileageExceptInfo->loc2().locType() == BLANK_INDICATOR) &&
            hipMileageExceptInfo->loc2().loc().empty())
          return ruleItemInfo->textTblItemNo();
      }
    }
    return RuleUtil::DEFAULT_TEXT_TBL_ITEM_NO;
  }

  return RuleUtil::DEFAULT_TEXT_TBL_ITEM_NO;
}

void
RuleUtil::travelSegWrapperVec2TSVec(const TravelSegWrapperVector& wrapperVec,
                                    std::vector<TravelSeg*>& vector,
                                    bool requireBothOrigDestMatch)
{
  vector.reserve(wrapperVec.size());

  for (const TravelSegWrapper* tsw : wrapperVec)
  {
    if (!requireBothOrigDestMatch || (tsw->origMatch() && tsw->destMatch()))
      vector.push_back(tsw->travelSeg());
  }
}

//..........................................................................
//  getTvlDateForTbl994Validation()
//..........................................................................
bool
RuleUtil::getTvlDateForTbl994Validation(DateTime& travelDate,
                                        const uint16_t category,
                                        const PaxTypeFare& paxTypeFare,
                                        const RuleItemInfo* ruleItemInfo,
                                        const PricingUnit* pricingUnit,
                                        NoPNRPricingTrx* noPNRPricingTrx)
{
  bool usePUTvlDate = false;

  switch (category)
  {
  case 5:
  case 6:
  case 7:
  case 8:
  case 10:
    usePUTvlDate = true;
    break;

  case 2:
  {
    const DayTimeAppInfo* dayTimeRule = dynamic_cast<const DayTimeAppInfo*>(ruleItemInfo);
    if (UNLIKELY(!dayTimeRule))
      return false; // should never happen, just for safety

    if (dayTimeRule->dayTimeAppl() == 'X')
    // DayTimeApplication::subJourneyBased
    {
      usePUTvlDate = true;
    }
  }
  break;

  case 3:
  {
    const SeasonalAppl* seasonRule = dynamic_cast<const SeasonalAppl*>(ruleItemInfo);
    if (!seasonRule)
      return false;

    if (seasonRule->assumptionOverride() != 'X')
      usePUTvlDate = true;
  }
  break;

  default:
    // use FC tvl date
    break;
  }

  // Set travelDate
  if (usePUTvlDate)
  {
    if (!pricingUnit)
      return false;

    getFirstValidTvlDT(travelDate, pricingUnit->travelSeg(), true, noPNRPricingTrx);
  }
  else
  {
    getFirstValidTvlDT(travelDate, paxTypeFare.fareMarket()->travelSeg(), true, noPNRPricingTrx);
  }

  if (UNLIKELY(!noPNRPricingTrx && travelDate.isOpenDate()))
  {
    // use first travelSeg departure date(+1 logic), setby PricingModelMap
    if (usePUTvlDate)
      travelDate = pricingUnit->travelSeg().front()->departureDT();
    else
      travelDate = paxTypeFare.fareMarket()->travelSeg().front()->departureDT();
  }

  return true;
}

bool
RuleUtil::matchOWRT(const Indicator owrtFromFareFocusRule,
                    const Indicator owrtFromFare)
{
  if (owrtFromFareFocusRule == ' ')
    return true;

  if ((owrtFromFareFocusRule == ONE_WAY_MAY_BE_DOUBLED) || (owrtFromFareFocusRule == ROUND_TRIP_MAYNOT_BE_HALVED) ||
      (owrtFromFareFocusRule == ONE_WAY_MAYNOT_BE_DOUBLED))
  {
    if (owrtFromFareFocusRule == owrtFromFare)
      return true;
  }

  if ((owrtFromFareFocusRule == ANY_ONE_WAY) && ((owrtFromFare == ONE_WAY_MAY_BE_DOUBLED) ||
      (owrtFromFare == ONE_WAY_MAYNOT_BE_DOUBLED)))
  {
    return true;
  }
  return false;
}

void
RuleUtil::getLatestBookingDate(PricingTrx& trx, DateTime& rtnDate, const PaxTypeFare& paxTypeFare)
{
  // Set reservationDate

  if (trx.getRequest()->isLowFareNoAvailability() // WPNCS
      ||
      trx.getRequest()->isLowFareRequested()) // WPNC
  {
    if (usingRebookedSeg(paxTypeFare))
    {
      if (LIKELY(trx.getRequest()->ticketingAgent()))
      {
        DateTime localTime = DateTime::localTime();
        short utcOffSet = 0;
        const Loc* saleLoc = trx.getRequest()->ticketingAgent()->agentLocation();
        const Loc* hdqLoc = trx.dataHandle().getLoc(RuleConst::HDQ_CITY, localTime);

        if (LIKELY(saleLoc && hdqLoc))
        {
          if (LIKELY(LocUtil::getUtcOffsetDifference(
                  *saleLoc, *hdqLoc, utcOffSet, trx.dataHandle(), localTime, localTime)))
          {
            localTime = localTime.addSeconds(utcOffSet * 60);
          }
        }

        rtnDate = localTime;
      }
      return;
    }
  }
  getLatestBookingDate(rtnDate, paxTypeFare.fareMarket()->travelSeg());
}

bool
RuleUtil::usingRebookedSeg(const PaxTypeFare& paxTypeFare)
{
  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());

  PaxTypeFare::SegmentStatusVec& segStatVec =
      (PaxTypeFare::SegmentStatusVec&)paxTypeFare.segmentStatus();

  // segmentStatus must have same size of fareMarket travel segments.
  // otherwise, the vector is not initialized right, we should do nothing
  if (segStatVec.size() != fareMarket.travelSeg().size())
  {
    return false;
  }

  PaxTypeFare::SegmentStatusVecCI segStatVecI = segStatVec.begin();
  PaxTypeFare::SegmentStatusVecCI segStatVecIEnd = segStatVec.end();

  for (; segStatVecI != segStatVecIEnd; segStatVecI++)
  {
    if ((*segStatVecI)._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
    {
      return true;
    }
  }

  return false;
}

void
RuleUtil::getLatestBookingDate(DateTime& rtnDate, const std::vector<TravelSeg*>& tvlSegs)
{
  rtnDate = DateTime::openDate();

  std::vector<TravelSeg*>::const_iterator tvlSegIter = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegIterEnd = tvlSegs.end();

  for (; tvlSegIter != tvlSegIterEnd; tvlSegIter++)
  {
    rtnDate.setWithLater((*tvlSegIter)->bookingDT());
  }
}
//..........................................................................
//  validateDateOverrideRuleItem()
//..........................................................................

bool
RuleUtil::checkT994Dates(const DateOverrideRuleItem& dorItem,
                         const DateTime& earliestDate,
                         const DateTime& latestDate,
                         const DateTime& ticketingDate,
                         const DateTime& reservationDate)
{
  return (
      (!dorItem.tvlEffDate().isValid() || (dorItem.tvlEffDate().date() <= latestDate.date())) &&
      (!dorItem.tvlDiscDate().isValid() || (dorItem.tvlDiscDate().date() >= earliestDate.date())) &&
      (!dorItem.tktEffDate().isValid() || (dorItem.tktEffDate().date() <= ticketingDate.date())) &&
      (!dorItem.tktDiscDate().isValid() ||
       (dorItem.tktDiscDate().date() >= ticketingDate.date())) &&
      (!dorItem.resEffDate().isValid() ||
       (dorItem.resEffDate().date() <= reservationDate.date())) &&
      (!dorItem.resDiscDate().isValid() ||
       (dorItem.resDiscDate().date() >= reservationDate.date())));
}
void
RuleUtil::printT994Dates(const DateOverrideRuleItem& dorItem, DiagCollector* diag)
{
  (*diag) << "    TVL EFF DTE  - " << dorItem.tvlEffDate().dateToString(DDMMMYY, "")
          << "  TVL DISC DTE - " << dorItem.tvlDiscDate().dateToString(DDMMMYY, "") << std::endl;
  (*diag) << "    TKT EFF DTE  - " << dorItem.tktEffDate().dateToString(DDMMMYY, "")
          << "  TKT DISC DTE - " << dorItem.tktDiscDate().dateToString(DDMMMYY, "") << std::endl;
  (*diag) << "    RES EFF DTE  - " << dorItem.resEffDate().dateToString(DDMMMYY, "")
          << "  RES DISC DTE - " << dorItem.resDiscDate().dateToString(DDMMMYY, "") << std::endl;
}
void
RuleUtil::printT994Dates(const DateOverrideRuleItem& dorItem,
                         DiagCollector* diag,
                         const DiagnosticTypes& callerDiag)
{
  std::ostringstream oss;
  oss << "    TVL EFF DTE  - " << dorItem.tvlEffDate().dateToString(DDMMMYY, "")
      << "  TVL DISC DTE - " << dorItem.tvlDiscDate().dateToString(DDMMMYY, "") << std::endl;
  oss << "    TKT EFF DTE  - " << dorItem.tktEffDate().dateToString(DDMMMYY, "")
      << "  TKT DISC DTE - " << dorItem.tktDiscDate().dateToString(DDMMMYY, "") << std::endl;
  oss << "    RES EFF DTE  - " << dorItem.resEffDate().dateToString(DDMMMYY, "")
      << "  RES DISC DTE - " << dorItem.resDiscDate().dateToString(DDMMMYY, "") << std::endl;
  Diag335Collector* dc335 = static_cast<Diag335Collector*>(diag);
  dc335->displayMessage(oss.str(), false);
}
//..........................................................................
//  validateDateOverrideRuleItem()
//..........................................................................
bool
RuleUtil::validateDateOverrideRuleItem(PricingTrx& trx,
                                       uint32_t itemNo,
                                       const VendorCode& vendor,
                                       const DateTime& travelDate,
                                       const DateTime& ticketingDate,
                                       const DateTime& reservationDate,
                                       DiagCollector* diag,
                                       const DiagnosticTypes& callerDiag)
{
  DataHandle dataHandle(trx.ticketingDate());
  dataHandle.setParentDataHandle(&trx.dataHandle());
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  bool diagEnabled = false;
  std::ostringstream oss;
  if (UNLIKELY(diag && (callerDiag != DiagnosticNone) && (callerDiag == diagType)))
  {
    diag->enable(diagType);
    diagEnabled = true;
    diag->setf(std::ios::left, std::ios::adjustfield);

    if (callerDiag == Diagnostic335)
    {
      oss << "  -: TABLE 994 -OVERRIDE DATE DATA :- " << vendor << " - " << itemNo << std::endl;
      Diag335Collector* dc335 = static_cast<Diag335Collector*>(diag);
      dc335->displayMessage(oss.str(), false);
    }
    else
    {
      (*diag) << "  -: TABLE 994 -OVERRIDE DATE DATA :- " << vendor << " - " << itemNo << std::endl;
    }
  }

  const std::vector<DateOverrideRuleItem*>& dorItemList =
      dataHandle.getDateOverrideRuleItem(vendor, itemNo);

  std::vector<DateOverrideRuleItem*>::const_iterator it = dorItemList.begin();

  while (it != dorItemList.end())
  {
    DateOverrideRuleItem& dorItem = **it;

    if (UNLIKELY(diagEnabled))
    {
      if (callerDiag == Diagnostic335)
      {
        printT994Dates(dorItem, diag, callerDiag);
      }
      else
      {
        printT994Dates(dorItem, diag);
      }
    }
    // check if dates match
    if (checkT994Dates(dorItem, travelDate, travelDate, ticketingDate, reservationDate))
    {
      if (UNLIKELY(diagEnabled))
      {
        if (callerDiag == Diagnostic335)
        {
          oss << "  TABLE 994: PASS" << std::endl;
          Diag335Collector* dc335 = static_cast<Diag335Collector*>(diag);
          dc335->displayMessage(oss.str(), false);
        }
        else
        {
          (*diag) << "  TABLE 994: PASS" << std::endl;
          diag->flushMsg();
        }
      }
      return true;
    }
    ++it;
  }

  if (UNLIKELY(diagEnabled))
  {
    if (callerDiag == Diagnostic335)
    {
      oss << "  TABLE 994: NOT MATCH" << std::endl;
      Diag335Collector* dc335 = static_cast<Diag335Collector*>(diag);
      dc335->displayMessage(oss.str(), false);
    }
    else
    {
      (*diag) << "  TABLE 994: NOT MATCH" << std::endl;
      diag->flushMsg();
    }
  }
  return false;
}

//..........................................................................
//  validateDateOverrideRuleItem() for Fare Display
//..........................................................................
bool
RuleUtil::validateDateOverrideRuleItem(PricingTrx& trx,
                                       bool& isInbound,
                                       uint32_t itemNo,
                                       const VendorCode& vendor,
                                       const DateTime& ticketingDate,
                                       const DateTime& reservationDate,
                                       DiagCollector* diag,
                                       const DiagnosticTypes& callerDiag)
{
  FareDisplayTrx* fdTrx;

  if (!FareDisplayUtil::getFareDisplayTrx(&trx, fdTrx))
  {
    return false;
  }

  DataHandle dataHandle(trx.ticketingDate());
  dataHandle.setParentDataHandle(&trx.dataHandle());
  FareMarket* fm = trx.fareMarket().front();
  if (isInbound)
  {
    fm = fdTrx->inboundFareMarket();
  }

  const DateTime& earliestDate = fm->travelSeg().front()->earliestDepartureDT();
  const DateTime& latestDate = fm->travelSeg().front()->latestDepartureDT();

  bool diagEnabled = false;
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  if (UNLIKELY(diag && (callerDiag != DiagnosticNone) && (callerDiag == diagType)))
  {
    diag->enable(diagType);
    diagEnabled = true;
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << "  -: TABLE 994 -OVERRIDE DATE DATA :- " << vendor << " - " << itemNo << std::endl;
  }

  const std::vector<DateOverrideRuleItem*>& dorItemList =
      dataHandle.getDateOverrideRuleItem(vendor, itemNo);

  std::vector<DateOverrideRuleItem*>::const_iterator it = dorItemList.begin();

  while (it != dorItemList.end())
  {
    DateOverrideRuleItem& dorItem = **it;

    if (UNLIKELY(diagEnabled))
    {
      printT994Dates(dorItem, diag);
    }
    // check if dates match
    if (checkT994Dates(dorItem, earliestDate, latestDate, ticketingDate, reservationDate))
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diag) << "  TABLE 994: PASS" << std::endl;
        diag->flushMsg();
      }
      return true;
    }
    ++it;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diag) << "  TABLE 994: NOT MATCH" << std::endl;
    diag->flushMsg();
  }
  return false;
}
//----------------------------------------------------------------------------
// matchJointCarrier()
//----------------------------------------------------------------------------
bool
RuleUtil::matchJointCarrier(PricingTrx& trx, const PaxTypeFare& paxTypeFare, const int itemNumber)
{
  if (!fallback::fallbackFootNoteR2Optimization(&trx))
    return RuleUtil::matchJointCarrier(
        trx, *paxTypeFare.fareMarket(), paxTypeFare.vendor(), paxTypeFare.carrier(), itemNumber);
  // to provide validation based on market, not on particular single fare

  if (LIKELY(itemNumber == 0))
  {
    return true;
  }
  bool ret = false;

  VecSet<CarrierCode> jcs; // joint carrier set

  // create Joint carrier vector with up to 3 different carriers
  // for the part of itinerary on the current FareMarket.

  const std::vector<TravelSeg*>& tSegments = paxTypeFare.fareMarket()->travelSeg();

  std::vector<TravelSeg*>::const_iterator tsIb = tSegments.begin();
  std::vector<TravelSeg*>::const_iterator tsIe = tSegments.end();

  uint16_t maxNo = 3;
  while ((tsIb != tsIe) && (jcs.size() < maxNo)) // done, if nubmer of joint carriers =3
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tsIb);
    if (airSeg != nullptr)
    {
      jcs.insert(airSeg->carrier()); // add an unidue carrier code to JCset
    }
    ++tsIb;
  }

  if (jcs.size() == 0 || jcs.size() > maxNo)
  {
    return false; // fail it
  }

  CarrierCode cxr = paxTypeFare.fareMarket()->governingCarrier();

  const DateTime& travelDate = paxTypeFare.fareMarket()->travelDate();
  // retrieve a vector of JointCarrier records
  const std::vector<const JointCarrier*>& jcList =
      trx.dataHandle().getJointCarrier(paxTypeFare.vendor(), itemNumber, travelDate);

  std::vector<const JointCarrier*>::const_iterator bIt = jcList.begin();

  // scan the vector

  while (bIt != jcList.end())
  {
    if (paxTypeFare.carrier() != INDUSTRY_CARRIER)
    {
      if ((*bIt)->carrier2()[0] == RuleConst::DOLLAR_SIGN ||
          (*bIt)->carrier3()[0] == RuleConst::DOLLAR_SIGN)
      {
        if (jcs.find((*bIt)->carrier1()) != jcs.end() || jcs.find((*bIt)->carrier2()) != jcs.end())
        {
          return true; // pass
        }
      }
      // do normal check
      else
      {
        // all up to 3 joint carriers should be match with 997 table data

        VecSet<CarrierCode>::iterator it = jcs.begin();

        while (it != jcs.end())
        {
          CarrierCode carrierCode = *it;

          if ((*bIt)->carrier1() != carrierCode && (*bIt)->carrier2() != carrierCode &&
              (*bIt)->carrier3() != carrierCode)
          {
            ret = false;
            break; // fail this table, check next JointCarrier table
          }
          ret = true;
          ++it;
        }
      }
    }
    // do YY carrier logic:
    // the publishing carrier need only match one carrier in the table.
    else
    {
      if (cxr == (*bIt)->carrier1() || cxr == (*bIt)->carrier2() || cxr == (*bIt)->carrier3())
      {
        return true; // pass table
      }
    }
    if (ret)
    {
      return true;
    }
    ++bIt;
  }
  return ret;
}

bool
RuleUtil::matchJointCarrier(PricingTrx& trx,
                            const FareMarket& fareMarket,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const int itemNumber)
{
  // if (finalPricing)  --> return true;            // logic will be added

  if (_LIKELY(itemNumber == 0))
  {
    return true;
  }
  bool ret = false;

  VecSet<CarrierCode> jcs; // joint carrier set

  // create Joint carrier vector with up to 3 different carriers
  // for the part of itinerary on the current FareMarket.

  const std::vector<TravelSeg*>& tSegments = fareMarket.travelSeg();

  std::vector<TravelSeg*>::const_iterator tsIb = tSegments.begin();
  std::vector<TravelSeg*>::const_iterator tsIe = tSegments.end();

  uint16_t maxNo = 3;
  while ((tsIb != tsIe) && (jcs.size() < maxNo)) // done, if nubmer of joint carriers =3
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tsIb);
    if (airSeg != nullptr)
    {
      jcs.insert(airSeg->carrier()); // add an unidue carrier code to JCset
    }
    ++tsIb;
  }

  if (jcs.size() == 0 || jcs.size() > maxNo)
  {
    return false; // fail it
  }

  CarrierCode cxr = fareMarket.governingCarrier();

  const DateTime& travelDate = fareMarket.travelDate();
  // retrieve a vector of JointCarrier records
  const std::vector<const JointCarrier*>& jcList =
      trx.dataHandle().getJointCarrier(vendor, itemNumber, travelDate);

  std::vector<const JointCarrier*>::const_iterator bIt = jcList.begin();

  // scan the vector

  while (bIt != jcList.end())
  {
    if (carrier != INDUSTRY_CARRIER)
    {
      if ((*bIt)->carrier2()[0] == RuleConst::DOLLAR_SIGN ||
          (*bIt)->carrier3()[0] == RuleConst::DOLLAR_SIGN)
      {
        if (jcs.find((*bIt)->carrier1()) != jcs.end() || jcs.find((*bIt)->carrier2()) != jcs.end())
        {
          return true; // pass
        }
      }
      // do normal check
      else
      {
        // all up to 3 joint carriers should be match with 997 table data

        VecSet<CarrierCode>::iterator it = jcs.begin();

        while (it != jcs.end())
        {
          CarrierCode carrierCode = *it;

          if ((*bIt)->carrier1() != carrierCode && (*bIt)->carrier2() != carrierCode &&
              (*bIt)->carrier3() != carrierCode)
          {
            ret = false;
            break; // fail this table, check next JointCarrier table
          }
          ret = true;
          ++it;
        }
      }
    }
    // do YY carrier logic:
    // the publishing carrier need only match one carrier in the table.
    else
    {
      if (cxr == (*bIt)->carrier1() || cxr == (*bIt)->carrier2() || cxr == (*bIt)->carrier3())
      {
        return true; // pass table
      }
    }
    if (ret)
    {
      return true;
    }
    ++bIt;
  }
  return ret;
}
//..........................................................................
//  validateSamePoint()
//..........................................................................

bool
RuleUtil::validateSamePoint(PricingTrx& trx,
                            const VendorCode& vendor,
                            const uint16_t itemNumber,
                            const LocCode& mkt1,
                            const LocCode& mkt2,
                            const DateTime& travelDate,
                            DiagCollector* diag,
                            const DiagnosticTypes& callerDiag)
{
  DataHandle dataHandle(trx.ticketingDate());
  dataHandle.setParentDataHandle(&trx.dataHandle());

  bool diagEnabled = false;
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (UNLIKELY(diag && (callerDiag != DiagnosticNone) && (callerDiag == diagType)))
  {
    diag->enable(diagType);
    diagEnabled = true;
    (*diag) << "  -: TABLE 993-SAME POINTS DATA :- " << vendor << " - " << itemNumber << std::endl;
  }

  if (UNLIKELY(itemNumber == 0))
  {
    return false;
  }

  // retrieve a vector of SamePoint records

  const std::vector<const SamePoint*>& spList =
      trx.dataHandle().getSamePoint(vendor, itemNumber, travelDate);

  // scan the vector

  std::vector<const SamePoint*>::const_iterator bIt = spList.begin();

  while (bIt != spList.end())
  {
    const SamePoint& samePoint = **bIt;

    if (UNLIKELY(diagEnabled))
    {
      (*diag) << "  MARKET1: " << samePoint.mkt1() << "   MARKET2: " << samePoint.mkt2()
              << std::endl;
    }

    if ((mkt1 == samePoint.mkt1() && mkt2 == samePoint.mkt2()) ||
        (mkt1 == samePoint.mkt2() && mkt2 == samePoint.mkt1()))
    {
      if (UNLIKELY(diagEnabled))
      {
        (*diag) << "  TABLE 993: MATCH" << std::endl;
        diag->flushMsg();
      }
      return true;
    }
    ++bIt;
  }

  if (UNLIKELY(diagEnabled))
  {
    (*diag) << "  TABLE 993: NOT MATCH" << std::endl;
    diag->flushMsg();
  }
  return false;
}

//----------------------------------------------------------------------------
// matchFareType()
//----------------------------------------------------------------------------

bool
RuleUtil::matchFareType(const FareType& ftFromRule, const FareType& ftFromFareC)
{
  if (ftFromRule.empty() || (ftFromFareC == ftFromRule))
  {
    return true;
  }
  else if ((ftFromRule.length() > 1) && (ftFromRule[0] == ALL_TYPE))
  {
    switch (ftFromRule[1])
    {
    case R_TYPE:
    case F_TYPE:
    case B_TYPE:
    case E_TYPE:
    case W_TYPE:
    case X_TYPE:
    case S_TYPE:
    case P_TYPE:
    case A_TYPE:
    case Z_TYPE:
    case J_TYPE:
      if (ftFromRule[1] == ftFromFareC[0])
        return true;
      break;

    case Y_TYPE:
      if ((ftFromFareC[0] == 'E') || (ftFromFareC[0] == 'X') || (ftFromFareC[0] == 'A') ||
          (ftFromFareC[0] == 'P') || (ftFromFareC[0] == 'S'))
        return true;
      break;

    case ALL_TYPE:
      return true;

    default:
      break;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
// matchGenericFareType()
//----------------------------------------------------------------------------

bool
RuleUtil::matchGenericFareType(const FareType& ftFromRule, const FareType& ftFromFareC)
{
  if (ftFromRule.empty() || (ftFromFareC == ftFromRule))
  {
    return true;
  }

  else if (ftFromRule.length() > 3)
  {
    return false;
  }

  else if ((ftFromRule.length() == 2) && (ftFromRule[0] == ALL_TYPE))
  {
    switch (ftFromRule[1])
    {

    case Y_TYPE:
      if ((ftFromFareC[0] == 'E') || (ftFromFareC[0] == 'X') ||
          (ftFromFareC[0] == 'P') || (ftFromFareC[0] == 'S'))
        return true;
      break;

    case W_TYPE:
      if ((ftFromFareC[0] == 'W') ||  (ftFromFareC[0] == 'Z'))
        return true;
      break;

    case E_TYPE:
    case B_TYPE:
    case F_TYPE:
    case J_TYPE:
    case X_TYPE:
    case S_TYPE:
    case P_TYPE:
    case R_TYPE:
      if (ftFromRule[1] == ftFromFareC[0])
        return true;
      break;

    default:
      break;
    }
  }

  else if ((ftFromRule.length() == 3) && (ftFromRule[0] == ALL_TYPE))
  {

    if (ftFromRule == FR_TYPE)
    {
      if ((ftFromFareC[0] == 'F') ||  (ftFromFareC[0] == 'R'))
        return true;
    }

    else if (ftFromRule == BJ_TYPE)
    {
      if ((ftFromFareC[0] == 'B') ||  (ftFromFareC[0] == 'J'))
        return true;
    }

    else if (ftFromRule == EW_TYPE)
    {
      if ((ftFromFareC[0] == 'E') || (ftFromFareC[0] == 'X') || (ftFromFareC[0] == 'S') ||
          (ftFromFareC[0] == 'P') || (ftFromFareC[0] == 'W') || (ftFromFareC[0] == 'Z'))
        return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
// matchSeasins()
//----------------------------------------------------------------------------

bool
RuleUtil::matchSeasons(Indicator sFromRule, Indicator sFromFareC)
{
  if ((sFromRule == ' ') || (sFromRule == sFromFareC))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// matchDayOfWeek()
//----------------------------------------------------------------------------

bool
RuleUtil::matchDayOfWeek(Indicator dowFromRule, Indicator dowFromFareC)
{
  if ((dowFromRule == ' ') || (dowFromRule == dowFromFareC))
  {
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtil::table995WhollyWithin
//
// Description:
//
// Compare the first locale of two GeoRuleItem's (Table 995).
//
// @param  itemNo1            - First GeoRuleItem (Table995) number
// @param  itemNo2            - Second GeoRuleItem (Table995) number
// @param  vendorCode         -
//
// @return true if the locales of the GeoRuleItem's match
//
// ----------------------------------------------------------------------------

bool
RuleUtil::table995WhollyWithin(PricingTrx& trx,
                               const uint32_t itemNo1,
                               const uint32_t itemNo2,
                               const VendorCode& vendorCode)
{
  // const TSELatencyData metrics(trx,"GET GEO RULE ITEM 3");
  const std::vector<GeoRuleItem*>& itemList1 = trx.dataHandle().getGeoRuleItem(vendorCode, itemNo1);

  if (itemList1.empty())
  {
    LOG4CXX_ERROR(_dataErrLogger,
                  "Could not get GeoRuleItem " << itemNo1 << " Vendor " << vendorCode);
    return false;
  }

  const std::vector<GeoRuleItem*>& itemList2 = trx.dataHandle().getGeoRuleItem(vendorCode, itemNo2);

  if (itemList2.empty())
  {
    LOG4CXX_ERROR(_dataErrLogger,
                  "Could not get GeoRuleItem " << itemNo2 << " Vendor " << vendorCode);
    return false;
  }

  const GeoRuleItem* geoRuleItem1 = itemList1.front();
  const GeoRuleItem* geoRuleItem2 = itemList2.front();

  return ((geoRuleItem1->loc1().loc() == geoRuleItem2->loc1().loc()) &&
          (geoRuleItem1->loc1().locType() == geoRuleItem2->loc1().locType()) &&
          (geoRuleItem1->tsi() == geoRuleItem2->tsi()));
}

bool
RuleUtil::validateGeoRuleItemCreateDiag(PricingTrx& trx,
                                        const DiagnosticTypes& callerDiag,
                                        DCFactory*& factory,
                                        DiagCollector*& diag,
                                        const uint32_t itemNo,
                                        const VendorCode& vendorCode,
                                        const RuleConst::TSIScopeParamType defaultScope,
                                        const FareMarket* fm)
{
  // get the diag type from transaction
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  // Check if the diagnostic request included the /DDGEO option.
  if (LIKELY(trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) !=
      RuleConst::DIAGNOSTIC_INCLUDE_GEO))
    return false;

  //
  // Check for Diagnostics 300 thru 350 then initialize the
  //  DCFactory and DiagCollector
  //
  if ((callerDiag != DiagnosticNone) && (callerDiag == diagType) && (diagType >= Diagnostic300) &&
      (diagType <= Diagnostic350))
  {
    factory = DCFactory::instance();
    diag = factory->create(trx);

    diag->enable(diagType);

    (*diag) << " " << std::endl << "---- TABLE995 INPUT DATA ----" << std::endl
            << "ITEM NO: " << itemNo << "  VENDOR: " << vendorCode << "  SCOPE: ";

    switch (defaultScope)
    {
    case RuleConst::TSI_SCOPE_PARAM_JOURNEY:
      (*diag) << "JOURNEY";
      break;

    case RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY:
      (*diag) << "SUB JOURNEY";
      break;

    case RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT:
      (*diag) << "FARE";
      break;

    default:
      (*diag) << "ERROR";
      break;
    }

    (*diag) << std::endl << "FARE MARKET- ";

    if (fm)
    {
      std::string orig("NULL"), dest("NULL");
      if (fm->origin())
      {
        orig = fm->origin()->loc();
      }
      if (fm->destination())
      {
        dest = fm->destination()->loc();
      }

      (*diag) << "  ORIG: " << orig << "  DEST: " << dest << "  CARRIER: " << fm->governingCarrier()
              << std::endl;
    }
    else
    {
      (*diag) << " NULL" << std::endl;
    }
    return true;
  }
  return false;
}

bool
RuleUtil::validateGeoRuleItem(const RuleConst::TSIScopeParamType defaultScope,
                              const FarePath* fp,
                              const Itin* itin,
                              const PricingUnit* pu,
                              const FareMarket* fm,
                              std::vector<TravelSeg*>::const_iterator& i,
                              std::vector<TravelSeg*>::const_iterator& j)
{
  if (defaultScope == RuleConst::TSI_SCOPE_PARAM_FARE_COMPONENT)
  {
    if (LIKELY(fm))
    {
      i = fm->travelSeg().begin();
      j = fm->travelSeg().end();
    }
    else
    {
      LOG4CXX_ERROR(_logger,
                    "defaultScope -s set to TSI_SCOPE_PARAM_FARE_COMPONENT but fm is null");
      return false;
    }
  }
  else if (LIKELY(defaultScope == RuleConst::TSI_SCOPE_PARAM_SUB_JOURNEY))
  {
    if (LIKELY(pu))
    {
      i = pu->travelSeg().begin();
      j = pu->travelSeg().end();
    }
    else
    {
      LOG4CXX_ERROR(_logger, "defaultScope -s set to TSI_SCOPE_PARAM_SUB_JOURNEY but pu is null");
      return false;
    }
  }
  else if (defaultScope == RuleConst::TSI_SCOPE_PARAM_JOURNEY)
  {
    const Itin* itinerary = nullptr;
    if (fp && fp->itin())
    {
      itinerary = fp->itin();
    }
    else if (itin)
    {
      itinerary = itin;
    }
    if (itinerary)
    {
      i = itinerary->travelSeg().begin();
      j = itinerary->travelSeg().end();
    }
    else
    {
      LOG4CXX_ERROR(
          _logger, "defaultScope -s set to TSI_SCOPE_PARAM_JOURNEY but FarePath and Itin are null");
      return false;
    }
  }
  else
  {
    LOG4CXX_ERROR(_logger, "Scope Invalid");
    return false;
  }
  return true;
}

void
RuleUtil::validateGeoRuleItemWriteGeoRule(GeoRuleItem* geo,
                                          DiagnosticTypes diagType,
                                          DCFactory* factory,
                                          DiagCollector& diag)
{
  diag << std::endl << "---- TABLE995 DB DATA ----" << std::endl;

  diag << "LOC1- ";
  if (LOCTYPE_NONE != geo->loc1().locType())
  {
    diag << geo->loc1().locType() << "-" << geo->loc1().loc() << std::endl;
  }
  else
  {
    diag << "   NULL" << std::endl;
  }

  diag << "LOC2- ";
  if (LOCTYPE_NONE != geo->loc2().locType())
  {
    diag << geo->loc2().locType() << "-" << geo->loc2().loc() << std::endl;
  }
  else
  {
    diag << "   NULL" << std::endl;
  }

  diag << "TSI-     NO: " << geo->tsi() << std::endl;
  diag << " " << std::endl << std::endl;

  diag.flushMsg();
  diag.disable(diagType);
}

bool
RuleUtil::validateGeoRuleItem(const uint32_t itemNo,
                              const VendorCode& vendorCode,
                              const RuleConst::TSIScopeParamType defaultScope,
                              const bool allowJourneyScopeOverride,
                              const bool allowPUScopeOverride,
                              const bool allowFCScopeOverride,
                              PricingTrx& trx,
                              const FarePath* fp,
                              const Itin* itin,
                              const PricingUnit* pu,
                              const FareMarket* fm,
                              const DateTime& ticketingDate,
                              RuleUtil::TravelSegWrapperVector& applTravelSegment,
                              const bool origCheck,
                              const bool destCheck,
                              bool& fltStopCheck,
                              TSICode& tsi,
                              LocKey& locKey1,
                              LocKey& locKey2,
                              const DiagnosticTypes& callerDiag,
                              const Indicator chkLogic,
                              const FareUsage* fareUsage)
{
  const DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  DCFactory* factory = nullptr;
  DiagCollector* diag = nullptr;

  bool diagEnabled = false;

  GeoTravelType geoTvlType = GeoTravelType::International;
  if (fp && fp->itin())
  {
    geoTvlType = fp->itin()->geoTravelType();
  }
  else if (itin)
  {
    geoTvlType = itin->geoTravelType();
  }
  else if (pu)
  {
    geoTvlType = pu->geoTravelType();
  }
  else if (LIKELY(fm))
  {
    geoTvlType = fm->geoTravelType();
  }

  diagEnabled = validateGeoRuleItemCreateDiag(
      trx, callerDiag, factory, diag, itemNo, vendorCode, defaultScope, fm);
  // get item Geo RUle Item (table995) from the DB
  //
  const std::vector<GeoRuleItem*>* geoRuleItemListPtr = nullptr;
  {
    // const TSELatencyData metrics(trx,"GET GEO RULE ITEM 4");
    geoRuleItemListPtr = &trx.dataHandle().getGeoRuleItem(vendorCode, itemNo);
  }
  const std::vector<GeoRuleItem*>& geoRuleItemList = *geoRuleItemListPtr;

  if (UNLIKELY(geoRuleItemList.size() != 1))
  {
    // no Geo Rule Item, logg error and return fales
    //
    LOG4CXX_ERROR(_dataErrLogger,
                  "Geo Rule Item vector did not contain 1 item"
                      << " ItemNo: " << itemNo << " VendorCode: " << vendorCode);
    if (UNLIKELY(diagEnabled))
    {
      diag->flushMsg();
      diag->disable(diagType);
    }
    return false;
  }

  std::vector<GeoRuleItem*>::const_iterator it = geoRuleItemList.begin();

  // populate tsi, geo1Return, and geo2Return
  //
  tsi = (*it)->tsi();

  locKey1 = (*it)->loc1();
  //    const Loc* loc1 = trx.dataHandle().getLoc(locKey1.loc(), ticketingDate);

  locKey2 = (*it)->loc2();
  //    const Loc* loc2 = trx.dataHandle().getLoc(locKey2.loc(), ticketingDate);

  // if tsi in the 995 table set the fltStopCheck to false because it does not make sense
  tsi = (*it)->tsi();

  if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX && fm->direction() == FMDirection::INBOUND &&
      tsi > 0 && !pu))
  {
    return false;
  }

  if (tsi)
  {
    fltStopCheck = false;
  }

  if (UNLIKELY(diagEnabled))
  {
    validateGeoRuleItemWriteGeoRule(*it, diagType, factory, *diag);
  }

  if (tsi > 0)
  {
    if (fallback::fixed::APO29538_StopoverMinStay())
    {
      scopeTSIGeo(tsi,
                  locKey1,
                  locKey2,
                  defaultScope,
                  allowJourneyScopeOverride,
                  allowPUScopeOverride,
                  allowFCScopeOverride,
                  trx,
                  fp,
                  itin,
                  pu,
                  fm,
                  ticketingDate,
                  applTravelSegment,
                  callerDiag,
                  vendorCode);
    }
    else
    {
      scopeTSIGeo(tsi,
                  locKey1,
                  locKey2,
                  defaultScope,
                  allowJourneyScopeOverride,
                  allowPUScopeOverride,
                  allowFCScopeOverride,
                  trx,
                  fp,
                  itin,
                  pu,
                  fm,
                  ticketingDate,
                  applTravelSegment,
                  callerDiag,
                  vendorCode,
                  fareUsage);
    }

    if (applTravelSegment.size() > 0)
      return true;
    else
      return false;
  }

  // loop through the travel segs and delete the ones that do not apply
  //
  std::vector<TravelSeg*>::const_iterator i;
  std::vector<TravelSeg*>::const_iterator j;

  if (!validateGeoRuleItem(defaultScope, fp, itin, pu, fm, i, j))
    return false;

  bool fltStopMatched = false;

  for (; i != j; ++i)
  {
    // check for arunk segment if so continue

    if (dynamic_cast<ArunkSeg*>((*i)))
      continue;

    // check travel seg
    if (LIKELY(origCheck || destCheck || fltStopCheck))
    {
      bool origCk = origCheck;

      bool destCk = destCheck;
      bool fltStopCh = fltStopCheck;

      if (checkTravelSeg(
              trx, (*it), (*i), origCk, destCk, fltStopCh, chkLogic, vendorCode, geoTvlType))
      {
        // add to applTravelSegment
        TravelSegWrapper* tsw = nullptr;
        trx.dataHandle().get(tsw);

        // lint --e{413}
        tsw->travelSeg() = *i;
        tsw->origMatch() = origCk;
        tsw->destMatch() = destCk;

        applTravelSegment.push_back(tsw);

        if (fltStopCheck)
        {
          fltStopMatched = true;
        }
      }
    }
  }

  fltStopCheck = fltStopMatched;

  if (applTravelSegment.size() > 0)
    return true;
  else
    return false;
}

bool
RuleUtil::getTvlSegBtwAndGeoDirectConflicted(bool& checkOrig1,
                                             bool& checkDest1,
                                             bool& checkOrig2,
                                             bool& checkDest2)
{
  if (!checkOrig1 || !checkDest1)
  {
    // Single direction, make sure there is no conflict resulting
    // from checkOrig2
    if (checkOrig1)
    {
      checkOrig2 = false;
      if (checkDest2 == false)
        return true;
    }
    else // checkDest1
    {
      checkDest2 = false;
      if (checkOrig2 == false)
        return true;
    }
  }
  return false;
}
bool
RuleUtil::getTvlSegBtwAndGeo(PricingTrx& trx,
                             const uint32_t itemNoBtw,
                             const uint32_t itemNoAnd,
                             const VendorCode& vendorCode,
                             std::vector<TravelSeg*>& tvlSegsRtn,
                             std::vector<TravelSeg*>* tvlSegsRtn2ndDirection,
                             const RuleConst::TSIScopeParamType defaultScope,
                             const FarePath* fp,
                             const PricingUnit* pu,
                             const FareMarket* fm,
                             bool lookForSingleSegments,
                             const DateTime& ticketingDate,
                             bool& fltStopCheck,
                             DiagCollector* callerDiag,
                             bool isCat12Request)
{
  if (UNLIKELY(itemNoBtw == 0 || itemNoAnd == 0))
    return false;

  // Try to get TSI of itemNoBtw, if there is one, we check its directionary;
  // Try to get TSI of itemNoAnd, if there is one, we check its directionary;
  // The directionary of both result should not be conflict
  bool checkOrigBtw = true;
  bool checkDestBtw = true;
  TSICode tsiBtw;
  LocKey locKey1Btw;
  LocKey locKey2Btw;
  bool checkOrigAnd = true;
  bool checkDestAnd = true;
  TSICode tsiAnd;
  LocKey locKey1And;
  LocKey locKey2And;

  GeoTravelType geoTvlType = GeoTravelType::International;
  if (fp != nullptr && fp->itin() != nullptr)
  {
    geoTvlType = fp->itin()->geoTravelType();
  }
  else if (pu != nullptr)
  {
    geoTvlType = pu->geoTravelType();
  }
  else if (LIKELY(fm != nullptr))
  {
    geoTvlType = fm->geoTravelType();
  }

  if (UNLIKELY(!getOrigDestLocFromGeoRuleItem(
          itemNoBtw, vendorCode, trx, checkOrigBtw, checkDestBtw, tsiBtw, locKey1Btw, locKey2Btw) ||
      !getOrigDestLocFromGeoRuleItem(
          itemNoAnd, vendorCode, trx, checkOrigAnd, checkDestAnd, tsiAnd, locKey1And, locKey2And)))
  {
    if (UNLIKELY(callerDiag && callerDiag->isActive()))
    {
      DiagCollector& diag = *callerDiag;
      diag << " FAILED GET GEO RULE ITEM" << std::endl;
    }
    return false;
  }

  bool directConflicted = false;

  if (tsiBtw != 0)
  {
    directConflicted =
        getTvlSegBtwAndGeoDirectConflicted(checkOrigBtw, checkDestBtw, checkOrigAnd, checkDestAnd);
  }
  else if (UNLIKELY(tsiAnd != 0))
  {
    directConflicted =
        getTvlSegBtwAndGeoDirectConflicted(checkOrigAnd, checkDestAnd, checkOrigBtw, checkDestBtw);
  }

  if (directConflicted)
  {
    if (UNLIKELY(callerDiag && callerDiag->isActive()))
    {
      DiagCollector& diag = *callerDiag;
      diag << " DIRECTION CONFLICTED BETWEEN BTWGEO AND ANDGEO" << std::endl;
    }
    return false;
  }

  if (UNLIKELY(!fm && !pu && !fp))
    return false;

  const std::vector<TravelSeg*>& allTvlSegs =
      (fm) ? fm->travelSeg() : ((pu) ? pu->travelSeg() : fp->itin()->travelSeg());

  if (tsiBtw != 0 || tsiAnd != 0)
  {
    RuleUtil::TravelSegWrapperVector applBtwTvlSegs;

    if (!validateGeoRuleItem(itemNoBtw,
                             vendorCode,
                             defaultScope,
                             false,
                             false,
                             false,
                             trx,
                             fp, // fare path
                             nullptr,
                             pu, // pricing unit
                             fm,
                             ticketingDate,
                             applBtwTvlSegs, // this will contain the results
                             checkOrigBtw,
                             checkDestBtw,
                             fltStopCheck,
                             tsiBtw,
                             locKey1Btw,
                             locKey2Btw,
                             callerDiag ? callerDiag->diagnosticType() : DiagnosticNone,
                             LOGIC_AND))
    {
      return false;
    }

    if (UNLIKELY(applBtwTvlSegs.size() == 0))
      return false;

    RuleUtil::TravelSegWrapperVector applAndTvlSegs;

    if (!validateGeoRuleItem(itemNoAnd,
                             vendorCode,
                             defaultScope,
                             false,
                             false,
                             false,
                             trx,
                             fp, // fare path
                             nullptr,
                             pu, // pricing unit
                             fm,
                             ticketingDate,
                             applAndTvlSegs, // this will contain the results
                             checkOrigAnd,
                             checkDestAnd,
                             fltStopCheck,
                             tsiAnd,
                             locKey1And,
                             locKey2And,
                             callerDiag ? callerDiag->diagnosticType() : DiagnosticNone,
                             LOGIC_AND))
    {
      return false;
    }

    if (UNLIKELY(applAndTvlSegs.size() == 0))
      return false;

    if (checkOrigBtw && checkDestBtw)
    {
      return (findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, tvlSegsRtn) ||
              findBtwTvlSegs(applAndTvlSegs, applBtwTvlSegs, allTvlSegs, tvlSegsRtn));
    }
    else if (checkOrigBtw)
    {
      return findBtwTvlSegs(applBtwTvlSegs, applAndTvlSegs, allTvlSegs, tvlSegsRtn);
    }
    else
      return findBtwTvlSegs(applAndTvlSegs, applBtwTvlSegs, allTvlSegs, tvlSegsRtn);
  }

  if (LIKELY(fm))
  {
    if (lookForSingleSegments)
    {
      for (TravelSeg* ts : allTvlSegs)
      {
        std::vector<TravelSeg*> oneSeg(1, ts);
        std::vector<TravelSeg*> tempRet;

        if (getTvlSegBtwAndGeoForFm(trx,
                                    locKey1Btw,
                                    locKey1And,
                                    locKey2Btw,
                                    locKey2And,
                                    vendorCode,
                                    oneSeg,
                                    fltStopCheck,
                                    checkOrigBtw,
                                    checkDestBtw,
                                    geoTvlType,
                                    tempRet,
                                    nullptr))
        {
          tvlSegsRtn.push_back(ts);
        }
      }
      return tvlSegsRtn.size();
    }

    return getTvlSegBtwAndGeoForFm(trx,
                                   locKey1Btw,
                                   locKey1And,
                                   locKey2Btw,
                                   locKey2And,
                                   vendorCode,
                                   allTvlSegs,
                                   fltStopCheck,
                                   checkOrigBtw,
                                   checkDestBtw,
                                   geoTvlType,
                                   tvlSegsRtn,
                                   tvlSegsRtn2ndDirection,
                                   isCat12Request);
  } // check on fare market travel segments

  const size_t origRtnSegSz = tvlSegsRtn.size();

  if (checkOrigBtw)
  {
    // check from geoBtw to geoAnd
    matchTvlSegsFromTo(
        trx, locKey1Btw, locKey1And, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey1Btw, locKey2And, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey2Btw, locKey1And, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey2Btw, locKey2And, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);
  }

  if (checkDestBtw)
  {
    // check from geoAnd to geoBtw
    matchTvlSegsFromTo(
        trx, locKey1And, locKey1Btw, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey1And, locKey2Btw, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey2And, locKey1Btw, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);

    matchTvlSegsFromTo(
        trx, locKey2And, locKey2Btw, vendorCode, allTvlSegs, fltStopCheck, tvlSegsRtn, geoTvlType);
  }

  return origRtnSegSz != tvlSegsRtn.size();
}

bool
RuleUtil::getTvlSegBtwAndGeoForFm(PricingTrx& trx,
                                  const LocKey& locKey1Btw,
                                  const LocKey& locKey1And,
                                  const LocKey& locKey2Btw,
                                  const LocKey& locKey2And,
                                  const VendorCode& vendorCode,
                                  const std::vector<TravelSeg*>& allTvlSegs,
                                  bool fltStopCheck,
                                  bool checkOrigBtw,
                                  bool checkDestBtw,
                                  const GeoTravelType& geoTvlType,
                                  std::vector<TravelSeg*>& tvlSegsRtn,
                                  std::vector<TravelSeg*>* tvlSegsRtn2ndDirection,
                                  bool isCat12Request)
{
  // we should only have one match
  std::set<TravelSeg*> filteredSegs, filteredSegs2ndDirection;
  bool origBtwMatch = false;
  bool destBtwMatch = false;

  auto getTvlSegMatcher = [&](auto& filteredSegsSet)
  {
    return [&](const auto& locKey1, const auto& locKey2)
    {
      return matchAllTvlSegsFromTo(
          trx, locKey1, locKey2, vendorCode, allTvlSegs, fltStopCheck, filteredSegsSet, geoTvlType);
    };
  };

  auto matchTvlSegOrigAnd = getTvlSegMatcher(filteredSegs);
  auto matchTvlSegOrigBtw =
      getTvlSegMatcher(tvlSegsRtn2ndDirection ? filteredSegs2ndDirection : filteredSegs);

  if (LIKELY(checkOrigBtw))
  {
    // APO:42730: the geolocs in T995 have an OR relationship. So get all
    // matching tvl segments using geoLocs in  btw and And tables. Do this
    // only for cat12 request. May also be needed for other categories
    if (isCat12Request)
    {
      bool retVal1 = matchTvlSegOrigAnd(locKey1Btw, locKey1And);
        bool retVal2 = matchTvlSegOrigAnd(locKey1Btw, locKey2And);
        bool retVal3 = matchTvlSegOrigAnd(locKey2Btw, locKey1And);
        bool retVal4 = matchTvlSegOrigAnd(locKey2Btw, locKey2And);
        if (retVal1 || retVal2 || retVal3 || retVal4)
           origBtwMatch = true;
    }
    else
    {
      // check from geoBtw to geoAnd
      if (matchTvlSegOrigAnd(locKey1Btw, locKey1And) || matchTvlSegOrigAnd(locKey1Btw, locKey2And) ||
        matchTvlSegOrigAnd(locKey2Btw, locKey1And) || matchTvlSegOrigAnd(locKey2Btw, locKey2And))
         // found match
         origBtwMatch = true;
    } //fallback
  }

  if (LIKELY(checkDestBtw))
  {
    if ( isCat12Request)
    {
       // check from geoAnd to geoBtw
      bool retVal1 = matchTvlSegOrigAnd(locKey1And, locKey1Btw);
        bool retVal2 = matchTvlSegOrigAnd(locKey1And, locKey2Btw );
        bool retVal3 = matchTvlSegOrigAnd(locKey2And, locKey1Btw );
        bool retVal4 = matchTvlSegOrigAnd(locKey2And, locKey2Btw );
        if (retVal1 || retVal2 || retVal3 || retVal4)
           destBtwMatch = true;
    }
    else
    {
      // check from geoAnd to geoBtw
      if (matchTvlSegOrigBtw(locKey1And, locKey1Btw) || matchTvlSegOrigBtw(locKey1And, locKey2Btw) ||
        matchTvlSegOrigBtw(locKey2And, locKey1Btw) || matchTvlSegOrigBtw(locKey2And, locKey2Btw))
        // found match
        destBtwMatch = true;
    }
  }

  if (UNLIKELY(origBtwMatch && destBtwMatch && tvlSegsRtn2ndDirection))
  {
    if (filteredSegs != filteredSegs2ndDirection)
    {
      *tvlSegsRtn2ndDirection = getFilteredSegsInOrder(allTvlSegs, filteredSegs2ndDirection);
    }
  }

  if (origBtwMatch || destBtwMatch)
  {
    tvlSegsRtn = getFilteredSegsInOrder(allTvlSegs, filteredSegs);
  }

  return origBtwMatch || destBtwMatch;
}

bool
RuleUtil::getOrigDestLocFromGeoRuleItem(const uint32_t itemNo,
                                        const VendorCode& vendorCode,
                                        PricingTrx& trx,
                                        bool& checkOrig,
                                        bool& checkDest,
                                        TSICode& tsi,
                                        LocKey& locKey1,
                                        LocKey& locKey2)
{
  // const TSELatencyData metrics(trx,"GET GEO RULE ITEM 6");
  // get item Geo RUle Item (table995) from the DB
  //
  const std::vector<GeoRuleItem*>& geoRuleItemList =
      trx.dataHandle().getGeoRuleItem(vendorCode, itemNo);

  if (UNLIKELY(geoRuleItemList.size() != 1))
  {
    LOG4CXX_ERROR(_dataErrLogger,
                  "Geo Rule Item vector did not contain 1 item"
                      << " ItemNo: " << itemNo << " VendorCode: " << vendorCode);
    return false;
  }

  const GeoRuleItem* const gri = geoRuleItemList.front();

  // populate tsi, geo1Return, and geo2Return
  tsi = gri->tsi();
  locKey1 = gri->loc1();
  locKey2 = gri->loc2();
  tsi = gri->tsi();

  return !tsi || LIKELY(getTSIOrigDestCheck(tsi, trx, checkOrig, checkDest));
}

bool
RuleUtil::matchTvlSegsFromTo(PricingTrx& trx,
                             const LocKey& locKeyFrom,
                             const LocKey& locKeyTo,
                             const VendorCode& vendorCode,
                             const std::vector<TravelSeg*>& allTvlSegs,
                             const bool& fltStopCheck,
                             std::vector<TravelSeg*>& tvlSegsRtn,
                             GeoTravelType geoTvlType)
{
  const LocTypeCode& locTypeFrom = locKeyFrom.locType();
  const LocTypeCode& locTypeTo = locKeyTo.locType();

  if (locTypeFrom == LOCTYPE_NONE || locTypeTo == LOCTYPE_NONE)
    return false;

  const LocCode& locCodeFrom = locKeyFrom.loc();
  const LocCode& locCodeTo = locKeyTo.loc();

  std::vector<TravelSeg*>::const_reverse_iterator tvlSegI = allTvlSegs.rbegin();
  std::vector<TravelSeg*>::const_reverse_iterator tvlSegEndI = allTvlSegs.rend();

  std::vector<TravelSeg*>::const_reverse_iterator fromTvlSegI = tvlSegEndI;
  std::vector<TravelSeg*>::const_reverse_iterator toTvlSegI = tvlSegEndI;

  std::vector<TravelSeg*>::const_reverse_iterator toTvlSegIByHiddenStop = tvlSegEndI;

  for (; tvlSegI != tvlSegEndI; ++tvlSegI)
  {
    const TravelSeg& tvlSeg = **tvlSegI;
    const std::vector<const Loc*>::const_reverse_iterator hiddenStopIterBegin =
        tvlSeg.hiddenStops().rbegin();
    const std::vector<const Loc*>::const_reverse_iterator hiddenStopIterEnd =
        tvlSeg.hiddenStops().rend();
    std::vector<const Loc*>::const_reverse_iterator hiddenStopIter = hiddenStopIterBegin;

    // there could be orig of multiple tvlSeg matches locCodeFrom,
    // to get the shortest list, we keep checking and replacing fromTvlSegI
    // with the last one
    if (LocUtil::isInLoc(*(tvlSeg.destination()),
                         locTypeTo,
                         locCodeTo,
                         vendorCode,
                         RESERVED,
                         LocUtil::FLIGHT_RULE,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      toTvlSegI = tvlSegI;
    }
    else if (fltStopCheck && !tvlSeg.hiddenStops().empty())
    {
      for (; hiddenStopIter != hiddenStopIterEnd; hiddenStopIter++)
      {
        if (LocUtil::isInLoc(**hiddenStopIter,
                             locTypeTo,
                             locCodeTo,
                             vendorCode,
                             RESERVED,
                             LocUtil::FLIGHT_RULE,
                             geoTvlType))
        {
          toTvlSegI = tvlSegI;
          toTvlSegIByHiddenStop = tvlSegI;
          break;
        }
      }
    }

    if (toTvlSegI == tvlSegEndI)
      continue;

    // looking for Origin match
    if (LocUtil::isInLoc(*((*tvlSegI)->origin()),
                         locTypeFrom,
                         locCodeFrom,
                         vendorCode,
                         RESERVED,
                         LocUtil::FLIGHT_RULE,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      fromTvlSegI = tvlSegI + 1;
      // insert everything [from, to)
      tvlSegsRtn.insert(tvlSegsRtn.end(), toTvlSegI, fromTvlSegI);

      return true;
    }
    else if (fltStopCheck && !tvlSeg.hiddenStops().empty())
    {
      if (toTvlSegIByHiddenStop == tvlSegI && toTvlSegI == tvlSegI)
      {
        // matched From point on a hiddenStop, when checking same
        // TravelSeg, To point need to be next hiddenStop
        hiddenStopIter++;
      }
      else
      {
        // possible that now hiddenStopIter == hiddenStopIterEnd, when ]
        // we search *hiddenStopIter to match From point and got nothing
        // so reset the hiddenStopIter
        hiddenStopIter = hiddenStopIterBegin;
      }

      for (; hiddenStopIter != hiddenStopIterEnd; ++hiddenStopIter)
      {
        if (LocUtil::isInLoc(**hiddenStopIter,
                             locTypeFrom,
                             locCodeFrom,
                             vendorCode,
                             RESERVED,
                             LocUtil::FLIGHT_RULE,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT()))
        {
          fromTvlSegI = tvlSegI + 1;
          // insert everything [from, to)
          tvlSegsRtn.insert(tvlSegsRtn.end(), toTvlSegI, fromTvlSegI);

          return true;
        }
      }
    }
  }
  return false;
}
bool
RuleUtil::checkTravelSegLoc(PricingTrx& trx,
                            const GeoRuleItem* geoRuleItem,
                            const Loc& loc,
                            const VendorCode& vendorCode,
                            GeoTravelType geoTvlType,
                            LocUtil::ApplicationType locAppl)
{
  // if loc1 is filled
  if (LIKELY(geoRuleItem->loc1().locType() != LOCTYPE_NONE))
  {
    if (LocUtil::isInLoc(loc,
                         geoRuleItem->loc1().locType(),
                         geoRuleItem->loc1().loc(),
                         vendorCode,
                         RESERVED,
                         locAppl,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      return true;
    }
  }

  // if loc2 is filled
  if (geoRuleItem->loc2().locType() != LOCTYPE_NONE)

  {
    if (LocUtil::isInLoc(loc,
                         geoRuleItem->loc2().locType(),
                         geoRuleItem->loc2().loc(),
                         vendorCode,
                         RESERVED,
                         locAppl,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      return true;
    }
  }
  return false;
}
bool
RuleUtil::checkTravelSeg(PricingTrx& trx,
                         const GeoRuleItem* geoRuleItem,
                         const TravelSeg* travelSeg,
                         bool& origCheck,
                         bool& destCheck,
                         bool& fltStopCheck,
                         const Indicator chkLogic,
                         const VendorCode& vendorCode,
                         GeoTravelType geoTvlType)
{
  bool inputOrigCh = origCheck;
  bool inputDestCh = destCheck;

  bool resultOrgin = false;
  bool resultDest = false;
  bool resultFlight = false;

  LocUtil::ApplicationType locAppl = LocUtil::FLIGHT_RULE;

  if (origCheck)
  {
    resultOrgin =
        checkTravelSegLoc(trx, geoRuleItem, *travelSeg->origin(), vendorCode, geoTvlType, locAppl);
  }
  if (destCheck)
  {
    resultDest = checkTravelSegLoc(
        trx, geoRuleItem, *travelSeg->destination(), vendorCode, geoTvlType, locAppl);
  }

  if (!origCheck)
    resultOrgin = true; // this will exclude the origin check from the result
  else
    origCheck = resultOrgin;

  if (!destCheck)
    resultDest = true; // this will exclude the dest check from the result
  else
    destCheck = resultDest;

  // There are cases that there is no hiddenStop, if either origCheck or
  // destCheck is true and resulted match, we will let it pass those cases.
  if (!fltStopCheck)
  {
    resultFlight = true; // this will exclude the flt check from the result
  }
  else if (travelSeg->hiddenStops().empty())
  {
    fltStopCheck = false; // no match
    if ((origCheck || destCheck) && (resultOrgin && resultDest))
    {
      resultFlight = true;
    }
  }
  else
  {
    bool checkLoc1 = (geoRuleItem->loc1().locType() != LOCTYPE_NONE);
    bool checkLoc2 = (geoRuleItem->loc2().locType() != LOCTYPE_NONE);

    if (LIKELY(checkLoc1 || checkLoc2))
    {
      std::vector<const Loc*>::const_iterator i = travelSeg->hiddenStops().begin();
      std::vector<const Loc*>::const_iterator j = travelSeg->hiddenStops().end();

      for (; i != j; ++i)
      {
        if (checkLoc1 && LocUtil::isInLoc(*((*i)),
                                          geoRuleItem->loc1().locType(),
                                          geoRuleItem->loc1().loc(),
                                          vendorCode,
                                          RESERVED,
                                          locAppl,
                                          geoTvlType,
                                          EMPTY_STRING(),
                                          trx.getRequest()->ticketingDT()))
        {
          resultFlight = true;
          break;
        }
        if (checkLoc2 && LocUtil::isInLoc(*((*i)),
                                          geoRuleItem->loc2().locType(),
                                          geoRuleItem->loc2().loc(),
                                          vendorCode,
                                          RESERVED,
                                          locAppl,
                                          geoTvlType,
                                          EMPTY_STRING(),
                                          trx.getRequest()->ticketingDT()))
        {
          resultFlight = true;
          break;
        }
      }
    }

    fltStopCheck = resultFlight;
  }

  if (chkLogic == LOGIC_AND)
  {
    if (inputOrigCh && inputDestCh)
    {
      if ((resultOrgin || resultDest) && resultFlight)
        return true;
      else
        return false;
    }
    else
    {
      if (resultOrgin && resultDest && resultFlight)
        return true;
      else
        return false;
    }
  }
  else
  {
    // LOGIC_OR
    return ((inputOrigCh && resultOrgin) || (inputDestCh && resultDest) ||
            (fltStopCheck && resultFlight));
  }
}

//----------------------------------------------------------------------------
// matchFareFootnote()
//----------------------------------------------------------------------------
bool
RuleUtil::matchFareFootnote(const Footnote& ft1FromRule,
                            const Footnote& ft2FromRule,
                            const Footnote& ft1FromFare,
                            const Footnote& ft2FromFare)
{
  if (!ft1FromRule.empty())
  {
    //----------------------------
    // fn1 does not match fare fn1
    //----------------------------
    if (ft1FromRule != ft1FromFare)
    {
      //----------------------------
      // fn1 does not match fare fn2
      //----------------------------
      if (ft1FromRule != ft2FromFare)
      {
        return false;
      }
      //----------------------------
      // fn2 does not match fare fn1 (DEFAULT_FOOTNOTE is "  ")
      //----------------------------
      else if (UNLIKELY(!ft2FromRule.empty() && ft2FromRule != ft1FromFare))
      {
        return false;
      }
    }

    //----------------------------
    // fn2 does not match fare fn2 (DEFAULT_FOOTNOTE is "  ")
    //----------------------------
    else if (UNLIKELY(!ft2FromRule.empty() && ft2FromRule != ft2FromFare))
    {
      return false;
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
//
// @function bool RuleUtil::getSurcharges
//
// Description:
//
//  get all types of surcharges for the current Journey (FarePath),
//  accumulate plusUp surcharges for the all Fare usages.
//
// @param  trx                - The Transaction object
// @param  farePath           - The Journey
//
// @return true if there is a plusUp Surcharge amount for the current FarePath.
//
// ----------------------------------------------------------------------------
bool
RuleUtil::getSurcharges(PricingTrx& trx, FarePath& fPath)
{
  MoneyAmount plusUpAmountFP = 0;
  MoneyAmount plusUpAmountFU = 0;
  bool isTktSurchargeFP = false;
  bool isTktSurchargeFU = false;
  bool isEMS = false;
  bool dummyFarePlusUp = false;

  SecSurchargeAppl::SecSurchRuleMap ssrMap;

  std::vector<PricingUnit*>::iterator puIt = fPath.pricingUnit().begin();
  std::vector<PricingUnit*>::iterator puItEnd = fPath.pricingUnit().end();
  std::vector<PricingUnit*>::iterator puItL = puItEnd - 1;

  const CarrierCode& validatingCxr =
      (trx.isValidatingCxrGsaApplicable() && !fPath.validatingCarriers().empty())
          ? fPath.validatingCarriers().front()
          : fPath.itin()->validatingCarrier();

  recreateRefundPlusUps(trx, fPath);

  for (; puIt != puItEnd; ++puIt)
  {
    PricingUnit& pricingUnit = **puIt;
    // go throuth the Fare Usages
    std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
    std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

    for (; fareUsageI != fareUsageEnd; ++fareUsageI)
    {
      // --------- calculate possible sector surcharges ---
      FareUsage& fu = *(*fareUsageI);

      // do not collect any sector surcharge for the dummy fares
      // when processing a ExchangePricingTrx but check if there
      // is surcharge override and process that
      if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX && fu.paxTypeFare()->isDummyFare()))
      {
        dummyFarePlusUp = true;
        ExchangePricingTrx& exchangeTrx = static_cast<ExchangePricingTrx&>(trx);
        exchangeSurchargeOverride(exchangeTrx, fu);
        getExcSTOSurchargeOverride(exchangeTrx, fPath, fu);
        getFUPlusUpOverride(exchangeTrx, fu, fPath);
      }
      else
      {
        SecSurchargeAppl secSurchAppl;
        secSurchAppl.process(trx, validatingCxr, fu, ssrMap);
      }

      isTktSurchargeFU = false;
      // accumulate non Tkt surcharges
      plusUpAmountFU = addNonTktSurcharges(fu, isTktSurchargeFU);

      plusUpAmountFP += plusUpAmountFU;
      if (isTktSurchargeFU)
      {
        isTktSurchargeFP = true;
        fu.perTktCharges();
      }

      if (!isEMS && fu.paxTypeFare()->mileageSurchargeAmt() > 0)
        fPath.plusUpFlag() = true;
    }
  }
  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX && dummyFarePlusUp))
  {
    ExchangePricingTrx& exchangeTrx = static_cast<ExchangePricingTrx&>(trx);
    getFPPlusUpOverride(exchangeTrx, fPath);
  }

  // Search Ticket/Direction surcharges

  MoneyAmount amount = 0;
  CarrierCode cxr;
  Indicator sType;
  Indicator tPortion;
  LocKey lc1;
  LocKey lc2;
  LocKey lc3;
  LocKey lc4;
  LocKey lc5;
  LocKey lc6;
  TSICode tsi;
  TSICode tsiBtw;
  TSICode tsiAnd;
  bool inBound = true;

  if (isTktSurchargeFP)
  {
    puIt = fPath.pricingUnit().begin();

    for (; puIt != puItEnd; ++puIt)
    {
      // go throuth the Fare Usages
      std::vector<FareUsage*>::iterator fareUsageI = (*puIt)->fareUsage().begin();
      std::vector<FareUsage*>::iterator fareUsageEnd = (*puIt)->fareUsage().end();
      std::vector<FareUsage*>::iterator fareUsageL = fareUsageEnd - 1;

      for (; fareUsageI != fareUsageEnd; ++fareUsageI)
      {
        if (!(*fareUsageI)->isPerTktCharges())
        {
          continue;
        }

        std::vector<SurchargeData*>::iterator sI = (*fareUsageI)->surchargeData().begin();
        std::vector<SurchargeData*>::iterator sEnd = (*fareUsageI)->surchargeData().end();
        std::vector<SurchargeData*>::iterator sLast = sEnd - 1;

        for (; sI != sEnd; ++sI)
        {
          if ((*sI)->processedTkt())
          {
            continue;
          }

          if ((*sI)->travelPortion() == RuleConst::PERDIRECTION ||
              (*sI)->travelPortion() == RuleConst::PERTICKET)
          {
            if (UNLIKELY((*sI)->processedTkt()))
            {
              continue;
            }
            tPortion = (*sI)->travelPortion();
            cxr = (*fareUsageI)->paxTypeFare()->carrier();
            sType = (*sI)->surchargeType();
            lc1 = (*sI)->locKey1();
            lc2 = (*sI)->locKey2();
            lc3 = (*sI)->locKeyBtw1();
            lc4 = (*sI)->locKeyBtw2();
            lc5 = (*sI)->locKeyAnd1();
            lc6 = (*sI)->locKeyAnd2();
            tsi = (*sI)->tsi();
            tsiBtw = (*sI)->tsiBtw();
            tsiAnd = (*sI)->tsiAnd();
            inBound = (*fareUsageI)->isInbound();
            amount = 0;

            if (sI == sLast)
            {
              amount = ((*sI)->amountNuc() * (*sI)->itinItemCount());
            }
            else
            {
              getGroupMaxAmtForFU(trx,
                                  sI,
                                  sEnd,
                                  amount,
                                  sType,
                                  lc1,
                                  lc2,
                                  lc3,
                                  lc4,
                                  lc5,
                                  lc6,
                                  tsi,
                                  tsiBtw,
                                  tsiAnd,
                                  tPortion);
            }

            FareUsage* fUsg =
                *fareUsageI; // save FareUsage pointer for the selected surcharge group

            if (fareUsageI != fareUsageL)
            { // try to find Max amount in current PricingUnit for other FU's

              FareUsage* fu1 = getGroupMaxAmtForPU(trx,
                                                   fareUsageI + 1,
                                                   fareUsageEnd,
                                                   amount,
                                                   cxr,
                                                   sType,
                                                   lc1,
                                                   lc2,
                                                   lc3,
                                                   lc4,
                                                   lc5,
                                                   lc6,
                                                   tsi,
                                                   tsiBtw,
                                                   tsiAnd,
                                                   inBound,
                                                   tPortion);
              if (fu1 != nullptr) // new Max from other FU's in current PU?
              {
                fUsg = fu1; // If so, update FU pointer for selected group
              }
            }
            if (puIt != puItL)
            { // try to find Max amount in FarePath for other PU's
              FareUsage* fu2 = getGroupMaxAmtForFP(trx,
                                                   puIt + 1,
                                                   puItEnd,
                                                   amount,
                                                   cxr,
                                                   sType,
                                                   lc1,
                                                   lc2,
                                                   lc3,
                                                   lc4,
                                                   lc5,
                                                   lc6,
                                                   tsi,
                                                   tsiBtw,
                                                   tsiAnd,
                                                   inBound,
                                                   tPortion);
              if (fu2 != nullptr) // new Max from other PU?
              {
                fUsg = fu2; // If so, update FU pointer for selected group
              }
            }
            //  indicate "selected" SurchargeData items with max group amount in FU
            //  per Carrier, s_type, Geo's, Direction.

            setupSelectedGroupForFu(
                *fUsg, cxr, sType, lc1, lc2, lc3, lc4, lc5, lc6, tsi, tsiBtw, tsiAnd, tPortion);

            plusUpAmountFP += amount; // add "ticket and per_direction" surcharges.
          }

        } // Surch
      } // fu
    } // pu
  }

  if (plusUpAmountFP != 0)
  {
    fPath.plusUpAmount() += plusUpAmountFP;
    fPath.plusUpFlag() = true;
    fPath.increaseTotalNUCAmount(plusUpAmountFP);
  }

  Diag512Collector* diag = nullptr;
  // bool dg = false; // never used

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic512))
  {
    // Display a surcharge rule data
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag512Collector*>(factory->create(trx));
    diag->enable(Diagnostic512);
    // dg = true;  //never used
    diag->diag512Collector(plusUpAmountFP, fPath, trx);
    diag->flushMsg();
  }

  return true;
}
MoneyAmount
RuleUtil::addNonTktSurcharges(FareUsage& fU, bool& isTktSurchargeFU)
{
  std::vector<SurchargeData*>::iterator sI = fU.surchargeData().begin();
  std::vector<SurchargeData*>::iterator sEnd = fU.surchargeData().end();

  MoneyAmount plusUpAmount = 0;

  // loop thrue all surcharge data
  for (; sI != sEnd; ++sI)
  {
    if ((*sI)->travelPortion() == RuleConst::PERDIRECTION ||
        (*sI)->travelPortion() == RuleConst::PERTICKET)
    {
      isTktSurchargeFU = true;
      (*sI)->processedTkt() = false; // clear indicator
      (*sI)->selectedTkt() = false; // clear indicator
    }
    else
    {
      (*sI)->selectedTkt() = true;
      plusUpAmount += ((*sI)->amountNuc() * (*sI)->itinItemCount());
    }
  }
  // add plusup data
  fU.accumulateSurchargeAmt(plusUpAmount);

  return plusUpAmount;
}

void
RuleUtil::exchangeSurchargeOverride(ExchangePricingTrx& exchangeTrx, FareUsage& fu)
{
  if (exchangeTrx.exchangeOverrides().surchargeOverride().empty())
    return;

  if (!fu.surchargeData().empty())
    return;

  std::vector<SurchargeOverride*>::iterator surI =
      exchangeTrx.exchangeOverrides().surchargeOverride().begin();
  std::vector<SurchargeOverride*>::iterator surE =
      exchangeTrx.exchangeOverrides().surchargeOverride().end();

  std::vector<TravelSeg*>::iterator tvlI = fu.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = fu.travelSeg().end();
  TravelSeg* tvlSeg = nullptr;
  SurchargeData* surchargeData = nullptr;
  AirSeg* surAirSeg = nullptr;
  AirSeg* airSeg = nullptr;

  for (; tvlI != tvlE; ++tvlI)
  {
    tvlSeg = *tvlI;

    surI = exchangeTrx.exchangeOverrides().surchargeOverride().begin();
    for (; surI != surE; ++surI)
    {
      SurchargeOverride& surchargeOverride = *(*surI);

      if (surchargeOverride.removed())
        continue;

      if (surchargeOverride.fromExchange())
      {
        surAirSeg = dynamic_cast<AirSeg*>(surchargeOverride.travelSeg());
        if (surAirSeg == nullptr)
          continue;
        airSeg = dynamic_cast<AirSeg*>(tvlSeg);
        if (airSeg == nullptr)
          continue;
        if (surAirSeg->marketingFlightNumber() == airSeg->marketingFlightNumber() &&
            surAirSeg->marketingCarrierCode() == airSeg->marketingCarrierCode() &&
            surAirSeg->origAirport() == airSeg->origAirport() &&
            surAirSeg->destAirport() == airSeg->destAirport() &&
            surAirSeg->departureDT() == airSeg->departureDT() &&
            surAirSeg->getBookingCode() == airSeg->getBookingCode())
        {
          surchargeOverride.travelSeg() = tvlSeg;
        }
      }
      if (surchargeOverride.travelSeg() != tvlSeg)
        continue;

      surchargeData = exchangeTrx.constructSurchargeData();

      if (surchargeData == nullptr)
        return;
      surchargeData->brdAirport() = tvlSeg->origAirport();
      surchargeData->offAirport() = tvlSeg->destAirport();
      surchargeData->travelPortion() = RuleConst::ONEWAY;
      surchargeData->itinItemCount() = 1;
      surchargeData->surchargeType() = surchargeOverride.type();
      surchargeData->amountSelected() = surchargeOverride.amount();
      surchargeData->currSelected() = surchargeOverride.currency();
      surchargeData->amountNuc() = surchargeOverride.amount();
      surchargeData->singleSector() = surchargeOverride.singleSector();
      surchargeData->fcBrdCity() = surchargeOverride.fcBrdCity();
      surchargeData->fcOffCity() = surchargeOverride.fcOffCity();
      surchargeData->fcFpLevel() = surchargeOverride.fcFpLevel();

      if (!surchargeOverride.fromExchange())
        surchargeData->isFromOverride() = true;

      fu.surchargeData().push_back(surchargeData);
    }
  }
}
void
RuleUtil::setupSelectedGroupForFu(FareUsage& fUsg,
                                  const CarrierCode& cc,
                                  const Indicator sType,
                                  const LocKey& lc1,
                                  const LocKey& lc2,
                                  const LocKey& lc3,
                                  const LocKey& lc4,
                                  const LocKey& lc5,
                                  const LocKey& lc6,
                                  const TSICode& tsi,
                                  const TSICode& tsiBtw,
                                  const TSICode& tsiAnd,
                                  const Indicator tPortion)
{
  MoneyAmount amt = 0;

  std::vector<SurchargeData*>::iterator sI = fUsg.surchargeData().begin();
  std::vector<SurchargeData*>::iterator sEnd = fUsg.surchargeData().end();
  for (; sI != sEnd; ++sI)
  {
    if (sType == (*sI)->surchargeType() && tsi == (*sI)->tsi() && tsiBtw == (*sI)->tsiBtw() &&
        tsiAnd == (*sI)->tsiAnd() && lc1.locType() == (*sI)->locKey1().locType() &&
        lc1.loc() == (*sI)->locKey1().loc() && lc2.locType() == (*sI)->locKey2().locType() &&
        lc2.loc() == (*sI)->locKey2().loc() && lc3.locType() == (*sI)->locKeyBtw1().locType() &&
        lc3.loc() == (*sI)->locKeyBtw1().loc() && lc4.locType() == (*sI)->locKeyBtw2().locType() &&
        lc4.loc() == (*sI)->locKeyBtw2().loc() && lc5.locType() == (*sI)->locKeyAnd1().locType() &&
        lc5.loc() == (*sI)->locKeyAnd1().loc() && lc6.locType() == (*sI)->locKeyAnd2().locType() &&
        lc6.loc() == (*sI)->locKeyAnd2().loc() && tPortion == (*sI)->travelPortion())
    {
      amt += ((*sI)->amountNuc() * (*sI)->itinItemCount());
      (*sI)->selectedTkt() = true;
    }
  }
  fUsg.accumulateSurchargeAmt(amt);
}

void
RuleUtil::getGroupMaxAmtForFU(const PricingTrx& trx,
                              std::vector<SurchargeData*>::iterator sI,
                              std::vector<SurchargeData*>::iterator sE,
                              MoneyAmount& amt,
                              const Indicator sType,
                              const LocKey& lc1,
                              const LocKey& lc2,
                              const LocKey& lc3,
                              const LocKey& lc4,
                              const LocKey& lc5,
                              const LocKey& lc6,
                              const TSICode& tsi,
                              const TSICode& tsiBtw,
                              const TSICode& tsiAnd,
                              const Indicator tPortion)
{
  for (; sI != sE; ++sI)
  {
    if ((*sI)->processedTkt())
    {
      continue;
    }

    if ((*sI)->travelPortion() == RuleConst::PERDIRECTION ||
        (*sI)->travelPortion() == RuleConst::PERTICKET)
    {
      if (sType == (*sI)->surchargeType() && tsi == (*sI)->tsi() && tsiBtw == (*sI)->tsiBtw() &&
          tsiAnd == (*sI)->tsiAnd() && lc1.locType() == (*sI)->locKey1().locType() &&
          lc1.loc() == (*sI)->locKey1().loc() && lc2.locType() == (*sI)->locKey2().locType() &&
          lc2.loc() == (*sI)->locKey2().loc() && lc3.locType() == (*sI)->locKeyBtw1().locType() &&
          lc3.loc() == (*sI)->locKeyBtw1().loc() &&
          lc4.locType() == (*sI)->locKeyBtw2().locType() &&
          lc4.loc() == (*sI)->locKeyBtw2().loc() &&
          lc5.locType() == (*sI)->locKeyAnd1().locType() &&
          lc5.loc() == (*sI)->locKeyAnd1().loc() &&
          lc6.locType() == (*sI)->locKeyAnd2().locType() &&
          lc6.loc() == (*sI)->locKeyAnd2().loc() && tPortion == (*sI)->travelPortion())
      {
        const MoneyAmount surAmt = (*sI)->amountNuc() * (*sI)->itinItemCount();

        if (fallback::perTicketSurchargeFix(&trx))
          amt += surAmt;
        else
        {
          if (tPortion != RuleConst::PERTICKET)
            amt += surAmt;
          else  if (surAmt > amt)
            amt = surAmt;
        }

        (*sI)->processedTkt() = true;
      }
    }
  }
}

/// search max surcharge Amount through each FareUsage in PU
/// for desired group per carrier, type, Geo's, direction (if sType = "PERDIRECTION")

// REMOVE PricingTrx arg with perTicketSurchargeFix fallback
FareUsage*
RuleUtil::getGroupMaxAmtForPU(const PricingTrx& trx,
                              std::vector<FareUsage*>::iterator fUI,
                              std::vector<FareUsage*>::iterator fUEnd,
                              MoneyAmount& amt,
                              const CarrierCode& cc,
                              const Indicator sType,
                              const LocKey& lc1,
                              const LocKey& lc2,
                              const LocKey& lc3,
                              const LocKey& lc4,
                              const LocKey& lc5,
                              const LocKey& lc6,
                              const TSICode& tsi,
                              const TSICode& tsiBtw,
                              const TSICode& tsiAnd,
                              const bool inBound,
                              const Indicator tPortion)
{
  FareUsage* psd = nullptr;
  MoneyAmount amount = 0;

  for (; fUI != fUEnd; ++fUI)
  {
    if (cc != (*fUI)->paxTypeFare()->carrier() || !(*fUI)->isPerTktCharges())
    {
      continue;
    }
    if (tPortion == RuleConst::PERDIRECTION &&
        ((inBound && !(*fUI)->isInbound()) || (!inBound && (*fUI)->isInbound())))
    {
      continue;
    }
    std::vector<SurchargeData*>::iterator sI = (*fUI)->surchargeData().begin();
    std::vector<SurchargeData*>::iterator sEnd = (*fUI)->surchargeData().end();

    for (; sI != sEnd; ++sI)
    {
      if ((*sI)->processedTkt())
      {
        continue;
      }

      if ((*sI)->travelPortion() == RuleConst::PERDIRECTION ||
          (*sI)->travelPortion() == RuleConst::PERTICKET)
      {
        getGroupMaxAmtForFU(
            trx, sI, sEnd, amount, sType, lc1, lc2, lc3, lc4, lc5, lc6, tsi, tsiBtw, tsiAnd, tPortion);

        if (amount > amt)
        {
          amt = amount; // update MAX amount for group
          psd = *fUI;
        }
        break; // search is done for the current FU, go to next FU.
      }
    }
  }

  return psd;
}

/// search max surcharge Amount through each FareUsage in PU
/// for desired group per carrier, type, Geo's, direction (if sType = "PERDIRECTION")

FareUsage*
RuleUtil::getGroupMaxAmtForFP(const PricingTrx& trx,
                              std::vector<PricingUnit*>::iterator puIt,
                              std::vector<PricingUnit*>::iterator puItEnd,
                              MoneyAmount& amt,
                              const CarrierCode& cc,
                              const Indicator sType,
                              const LocKey& lc1,
                              const LocKey& lc2,
                              const LocKey& lc3,
                              const LocKey& lc4,
                              const LocKey& lc5,
                              const LocKey& lc6,
                              const TSICode& tsi,
                              const TSICode& tsiBtw,
                              const TSICode& tsiAnd,
                              const bool inBound,
                              const Indicator tPortion)
{
  FareUsage* psd = nullptr;
  MoneyAmount amount = 0;

  for (; puIt != puItEnd; ++puIt)
  {
    std::vector<FareUsage*>::iterator fareUsageI = (*puIt)->fareUsage().begin();
    std::vector<FareUsage*>::iterator fareUsageEnd = (*puIt)->fareUsage().end();
    FareUsage* psdCurrent = getGroupMaxAmtForPU(trx,
                                                fareUsageI,
                                                fareUsageEnd,
                                                amount,
                                                cc,
                                                sType,
                                                lc1,
                                                lc2,
                                                lc3,
                                                lc4,
                                                lc5,
                                                lc6,
                                                tsi,
                                                tsiBtw,
                                                tsiAnd,
                                                inBound,
                                                tPortion);
    if (amount > amt)
    {
      amt = amount; // update MAX amount for group
      psd = psdCurrent; // update SurchargeData pointer if MAX amount is found for group
    }
  }

  return psd;
}

bool
RuleUtil::isDiscount(const uint16_t categoryNumber)
{
  if (categoryNumber == RuleConst::CHILDREN_DISCOUNT_RULE ||
      categoryNumber == RuleConst::TOUR_DISCOUNT_RULE ||
      categoryNumber == RuleConst::AGENTS_DISCOUNT_RULE ||
      categoryNumber == RuleConst::OTHER_DISCOUNT_RULE)
    return true;
  return false;
}

//
// get pointer to the first GeneralFareRuleInfo with matching location (ignore swap)
// if no overrides, uses data in PaxTypeFare
// if no matches, returns null
//
GeneralFareRuleInfo*
RuleUtil::getGeneralFareRuleInfo(PricingTrx& trx,
                                 const PaxTypeFare& paxTypeFare,
                                 const uint16_t categoryNumber,
                                 bool& isLocationSwapped,
                                 const TariffNumber* overrideTcrRuleTariff,
                                 const RuleNumber* overrideRuleNumber)
{
  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  // tariff is int; rule is string
  const TariffNumber tn =
      (overrideTcrRuleTariff != nullptr) ? *overrideTcrRuleTariff : paxTypeFare.tcrRuleTariff();
  const RuleNumber* rn =
      (overrideRuleNumber != nullptr) ? overrideRuleNumber : &(paxTypeFare.ruleNumber());

  const std::vector<GeneralFareRuleInfo*>& gfrList =
      trx.dataHandle().getGeneralFareRule(paxTypeFare.vendor(),
                                          carrier,
                                          tn,
                                          *rn,
                                          categoryNumber,
                                          paxTypeFare.fareMarket()->travelDate());

  if (fallback::fixed::fallbackDisableESVIS())
  {
  BindingResultCRIP result(
      checkBindings(trx, paxTypeFare, categoryNumber, gfrList, isLocationSwapped, FARERULE));
  if (UNLIKELY(result.first))
  {
    return static_cast<GeneralFareRuleInfo*>(result.second);
  }
  else
  {
    // scan the vector and find the first match
    // isLocationSwapped = false;
    std::vector<GeneralFareRuleInfo*>::const_iterator bIt = gfrList.begin();

    while (bIt != gfrList.end())
    {
      if (matchGeneralFareRule(trx, paxTypeFare, **bIt, isLocationSwapped))
      {
        return *bIt;
      }
      ++bIt;
    }
  }
  }

  else
  {
    // scan the vector and find the first match
    // isLocationSwapped = false;
    std::vector<GeneralFareRuleInfo*>::const_iterator bIt = gfrList.begin();

    while (bIt != gfrList.end())
    {
      if (matchGeneralFareRule(trx, paxTypeFare, **bIt, isLocationSwapped))
      {
        return *bIt;
      }
      ++bIt;
    }
  }

  return nullptr;
}

// same as above only return non-null by adding it to vector
bool
RuleUtil::getGeneralFareRuleInfo(PricingTrx& trx,
                                 const PaxTypeFare& paxTypeFare,
                                 const uint16_t categoryNumber,
                                 GeneralFareRuleInfoVec& gfrInfoVec,
                                 const TariffNumber* overrideTcrRuleTariff,
                                 const RuleNumber* overrideRuleNumber)
{
  bool isLocationSwapped = false;
  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  // tariff is int; rule is string
  const TariffNumber tn =
      (overrideTcrRuleTariff != nullptr) ? *overrideTcrRuleTariff : paxTypeFare.tcrRuleTariff();
  const RuleNumber* rn =
      (overrideRuleNumber != nullptr) ? overrideRuleNumber : &(paxTypeFare.ruleNumber());

  const std::vector<GeneralFareRuleInfo*>& gfrList =
      trx.dataHandle().getGeneralFareRule(paxTypeFare.vendor(),
                                          carrier,
                                          tn,
                                          *rn,
                                          categoryNumber,
                                          paxTypeFare.fareMarket()->travelDate(),
                                          paxTypeFare.fareMarket()->ruleApplicationDate());
  if (fallback::fixed::fallbackDisableESVIS())
  {
  BindingResultCRIP result(
      checkBindings(trx, paxTypeFare, categoryNumber, gfrList, isLocationSwapped, FARERULE));
  if (UNLIKELY(result.first))
  {
    if (result.second != nullptr)
    {
      gfrInfoVec.push_back(std::make_pair(static_cast<GeneralFareRuleInfo*>(result.second),
                                          GeoMatchResult(isLocationSwapped)));
    }
    return true;
  }
  else
  {
    // scan the vector and find the first match
    std::vector<GeneralFareRuleInfo*>::const_iterator bIt = gfrList.begin();
    bool foundRec2 = false;
    bool isHistorical = trx.dataHandle().isHistorical();
    const DateTime& tktDate = trx.ticketingDate();
    while (bIt != gfrList.end())
    {
      if (matchGeneralFareRule(trx, paxTypeFare, **bIt, isLocationSwapped))
      {
        if (LIKELY(!foundRec2))
        {
          // push back
          gfrInfoVec.push_back(std::make_pair(*bIt, GeoMatchResult(isLocationSwapped)));
          foundRec2 = true;
          if (LIKELY(!isHistorical || tktDate.historicalIncludesTime() ||
              !matchCreateOrExpireDate(tktDate, (*bIt)->createDate(), (*bIt)->expireDate())))
            break;
        }
        else
        {
          if (!matchCreateOrExpireDate(tktDate, (*bIt)->createDate(), (*bIt)->expireDate()))
            break;
          gfrInfoVec.push_back(std::make_pair(*bIt, GeoMatchResult(isLocationSwapped)));
        }
      }
      ++bIt;
    }
  }
  }

  else
  {
    // scan the vector and find the first match
    std::vector<GeneralFareRuleInfo*>::const_iterator bIt = gfrList.begin();
    bool foundRec2 = false;
    bool isHistorical = trx.dataHandle().isHistorical();
    const DateTime& tktDate = trx.ticketingDate();
    while (bIt != gfrList.end())
    {
      if (matchGeneralFareRule(trx, paxTypeFare, **bIt, isLocationSwapped))
      {
        if (LIKELY(!foundRec2))
        {
          // push back
          gfrInfoVec.push_back(std::make_pair(*bIt, GeoMatchResult(isLocationSwapped)));
          foundRec2 = true;
          if (LIKELY(!isHistorical || tktDate.historicalIncludesTime() ||
                     !matchCreateOrExpireDate(tktDate, (*bIt)->createDate(), (*bIt)->expireDate())))
            break;
        }
        else
        {
          if (!matchCreateOrExpireDate(tktDate, (*bIt)->createDate(), (*bIt)->expireDate()))
            break;
          gfrInfoVec.push_back(std::make_pair(*bIt, GeoMatchResult(isLocationSwapped)));
        }
      }
      ++bIt;
    }
  }

  if (gfrInfoVec.empty())
    return false;
  return true;
}

//
// get pointer to the first GeneralRuleApp with matching category
// if no matches, returns null
//
GeneralRuleApp*
RuleUtil::getGeneralRuleApp(PricingTrx& trx,
                            const PaxTypeFare& paxTypeFare,
                            const uint16_t categoryNumber)
{
  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;
  if (!fallback::fallbackAPO37838Record1EffDateCheck(&trx))
  {
    return trx.dataHandle().getGeneralRuleAppByTvlDate(paxTypeFare.vendor(),
                                                       carrier,
                                                       paxTypeFare.tcrRuleTariff(),
                                                       RuleConst::NULL_GENERAL_RULE,
                                                       categoryNumber,
                                                       paxTypeFare.fareMarket()->travelDate());
  }
  else
  {
    return trx.dataHandle().getGeneralRuleApp(paxTypeFare.vendor(),
                                              carrier,
                                              paxTypeFare.tcrRuleTariff(),
                                              RuleConst::NULL_GENERAL_RULE,
                                              categoryNumber);
  }
}

bool
RuleUtil::getGeneralRuleApp(PricingTrx& trx,
                            const PaxTypeFare& paxTypeFare,
                            const uint16_t categoryNumber,
                            std::vector<GeneralRuleApp*>& vecGenRuleApp)
{
  GeneralRuleApp* gra = getGeneralRuleApp(trx, paxTypeFare, categoryNumber);

  if (gra != nullptr)
    vecGenRuleApp.push_back(gra);
  return true;
}

namespace
{
class FbrR2Filter
{
  typedef std::function<bool(const FareByRuleCtrlInfo*)> InhibitF;
  IsNotEffectiveG<FareByRuleCtrlInfo> _dateFilter;
  InhibitF _inhibitFilter;

  FbrR2Filter(const DateTime& tvlDate, InhibitF inf, const DateTime& ticketDate)
    : _dateFilter(tvlDate, ticketDate), _inhibitFilter(inf)
  {
  }

public:
  bool operator()(const FareByRuleCtrlInfo* fbrCI) const
  {
    return _dateFilter(fbrCI) || _inhibitFilter(fbrCI);
  }

  static FbrR2Filter makeMe(const PricingTrx& trx, const DateTime& tvlDate)
  {
    if (trx.dataHandle().isFareDisplay())
      return FbrR2Filter(tvlDate, InhibitForFD<FareByRuleCtrlInfo>, trx.dataHandle().ticketDate());

    return FbrR2Filter(tvlDate, Inhibit<FareByRuleCtrlInfo>, trx.dataHandle().ticketDate());
  }
};

void
determineFareByRuleCtrlInfoRecord(const FbrR2Filter& filter,
                                  const std::vector<FareByRuleCtrlInfo*>& fbrList,
                                  FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                                  PricingTrx& trx,
                                  const FareMarket& fareMarket)
{
  for (FareByRuleCtrlInfo* fbrCI : fbrList)
  {
    if (!filter(fbrCI))
    {
      bool isLocationSwapped = false;
      if (RuleUtil::matchLocation(trx,
                                  fbrCI->loc1(),
                                  fbrCI->loc1zoneTblItemNo(),
                                  fbrCI->loc2(),
                                  fbrCI->loc2zoneTblItemNo(),
                                  fbrCI->vendorCode(),
                                  fareMarket,
                                  isLocationSwapped,
                                  fbrCI->carrierCode()))
      {
        fbrCtrlInfoVec.push_back(std::make_pair(fbrCI, GeoMatchResult(isLocationSwapped)));
        return;
      }
    }
  }
}

void
printNotMatchedFbrR2(const std::vector<FareByRuleCtrlInfo*>& fbrList,
                     const FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                     DiagManager& diagManager,
                     FareMarket& fareMarket)
{
  Diag225Collector* diag = dynamic_cast<Diag225Collector*>(&diagManager.collector());

  auto printIt = fbrList.begin();
  auto printEnd = fbrList.end();

  if (!fbrCtrlInfoVec.empty())
    printEnd = std::find(fbrList.begin(), fbrList.end(), fbrCtrlInfoVec.front().first);

  for (const FareByRuleCtrlInfo* fbr : makeIteratorRange(printIt, printEnd))
    diag->diag225Collector(fbr, Diag225Collector::R2_FAIL_GEO, fareMarket);
}
}

bool
RuleUtil::getFareByRuleCtrlInfo(PricingTrx& trx,
                                FareByRuleApp& fbrApp,
                                FareMarket& fareMarket,
                                FareByRuleCtrlInfoVec& fbrCtrlInfoVec,
                                DiagManager& diagManager)
{
  if (!trx.dataHandle().isHistorical())
  {
    const std::vector<FareByRuleCtrlInfo*>& fbrList = trx.dataHandle().getAllFareByRuleCtrl(
        fbrApp.vendor(), fbrApp.carrier(), fbrApp.ruleTariff(), fbrApp.ruleNo());

    determineFareByRuleCtrlInfoRecord(FbrR2Filter::makeMe(trx, fareMarket.travelDate()),
                                      fbrList,
                                      fbrCtrlInfoVec,
                                      trx,
                                      fareMarket);

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic225))
      printNotMatchedFbrR2(fbrList, fbrCtrlInfoVec, diagManager, fareMarket);

    Diag202Collector* dc = Diag202Collector::makeMe(trx, &Diag202Collector::FBR, &fareMarket);
    if (_UNLIKELY(dc))
    {
      dc->printR2sMatchDetails<FareByRuleCtrlInfo>(FB_FARE_RULE_RECORD_2,
                                                   trx,
                                                   fbrList,
                                                   fbrCtrlInfoVec,
                                                   fareMarket,
                                                   fbrApp.carrier(),
                                                   fareMarket.travelDate());
    }

    return !fbrCtrlInfoVec.empty();
  }

  bool isLocationSwapped = false;
  std::vector<FareByRuleCtrlInfo*> fbrList =
      trx.dataHandle().getFareByRuleCtrl(fbrApp.vendor(),
                                         fbrApp.carrier(),
                                         fbrApp.ruleTariff(),
                                         fbrApp.ruleNo(),
                                         fareMarket.travelDate());

  TvlDatesSet tvlDatesSet;

  tvlDatesSet.insert(fareMarket.travelDate());
  TvlDatesSet* tvlDates = &tvlDatesSet;

  std::vector<char> matchedDates(tvlDates->size(), 0); // should be faster than vector<bool>,
  // and more flexible than bitset which is fixed-size

  std::set<std::pair<const FareByRuleCtrlInfo*, bool>, CategoryRuleInfoComp<FareByRuleCtrlInfo>>
  uniqueRec2s;
  // to disallow duplicates

  // scan the vector and find the first match for non historical and more matches for historical
  std::vector<FareByRuleCtrlInfo*>::const_iterator cb = fbrList.begin();
  std::vector<FareByRuleCtrlInfo*>::const_iterator ce = fbrList.end();
  bool isHistorical = trx.dataHandle().isHistorical();
  const DateTime& tktDate = trx.ticketingDate();

  for (; cb != ce; ++cb)
  {
    if (matchLocation(trx,
                      (*cb)->loc1(),
                      (*cb)->loc1zoneTblItemNo(),
                      (*cb)->loc2(),
                      (*cb)->loc2zoneTblItemNo(),
                      (*cb)->vendorCode(),
                      fareMarket,
                      isLocationSwapped,
                      (*cb)->carrierCode()))
    {
      TvlDatesSet::iterator tvlDatesIt = tvlDates->begin();
      for (size_t i = 0; i < tvlDates->size(); ++i)
      {
        // check if rec matched to some date already
        if (((*cb)->effDate() <= (*tvlDatesIt)) && ((*cb)->discDate() >= (*tvlDatesIt)))
        {
          if (matchedDates[i] == 0)
          {
            uniqueRec2s.insert(std::make_pair(*cb, isLocationSwapped));
            matchedDates[i] = 1;
          }
          else // already matched travel date
          {
            if (UNLIKELY(matchCreateOrExpireDate(tktDate, (*cb)->createDate(), (*cb)->expireDate()) &&
                isHistorical && !tktDate.historicalIncludesTime()))
              uniqueRec2s.insert(
                  std::make_pair(*cb, isLocationSwapped)); // allow more matches for historical
          }
        }
        tvlDatesIt++;
      }
    }
    else
    {
      if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic225))
      {
        Diag225Collector* diag = dynamic_cast<Diag225Collector*>(&diagManager.collector());
        diag->diag225Collector(*cb, Diag225Collector::R2_FAIL_GEO, fareMarket);
      }
    }
  }

  fbrCtrlInfoVec.assign(uniqueRec2s.begin(), uniqueRec2s.end());

  return (!fbrCtrlInfoVec.empty());
}

CombinabilityRuleInfo*
RuleUtil::getCombinabilityRuleInfo(PricingTrx& trx, PaxTypeFare& ptFare, bool& isLocationSwapped)
{
  // retrieve a vector of Combination rule candidates

  PaxTypeFare* paxTypeFare(&ptFare);

  const DateTime& travelDate = paxTypeFare->fareMarket()->travelDate();

  TariffNumber tcrRuleTariff;
  CarrierCode carrier;
  RuleNumber ruleNumber;
  VendorCode vendor;

  if (UNLIKELY(isFromCat25BaseFare(trx, *paxTypeFare) && !paxTypeFare->isSpecifiedFare()))
  {
    paxTypeFare = paxTypeFare->baseFare(25);
    tcrRuleTariff = paxTypeFare->tcrRuleTariff();
    carrier = paxTypeFare->carrier();
    ruleNumber = paxTypeFare->ruleNumber();
    vendor = paxTypeFare->vendor();
  }
  else
  {
    carrier = paxTypeFare->carrier();
    if (paxTypeFare->fare()->isIndustry())
      carrier = INDUSTRY_CARRIER;

    tcrRuleTariff = paxTypeFare->tcrRuleTariff();
    ruleNumber = paxTypeFare->ruleNumber();
    vendor = paxTypeFare->vendor();
  }

  const std::vector<CombinabilityRuleInfo*>& combinationList =
      trx.dataHandle().getCombinabilityRule(vendor, carrier, tcrRuleTariff, ruleNumber, travelDate);

  // scan the vector and find the first match

  CombinabilityRuleInfo* returnValue = nullptr;

  std::vector<CombinabilityRuleInfo*>::const_iterator cb = combinationList.begin();
  std::vector<CombinabilityRuleInfo*>::const_iterator ce = combinationList.end();

  while (cb != ce)
  {
    if (matchOneWayRoundTrip((*cb)->owrt(), paxTypeFare->owrt()) &&
        matchFareRouteNumber((*cb)->routingAppl(), (*cb)->routing(), paxTypeFare->routingNumber()) &&
        matchFareType((*cb)->fareType(), paxTypeFare->fcaFareType()) &&
        matchSeasons((*cb)->seasonType(), paxTypeFare->fcaSeasonType()) &&
        matchDayOfWeek((*cb)->dowType(), paxTypeFare->fcaDowType()) &&
        matchFareFootnote((*cb)->footNote1(),
                          (*cb)->footNote2(),
                          paxTypeFare->footNote1(),
                          paxTypeFare->footNote2()) &&
        matchJointCarrier(trx, *paxTypeFare, (*cb)->jointCarrierTblItemNo()) &&
        matchLoc_R1_2_6(trx, (*cb)->locKey1(), (*cb)->locKey2(), *paxTypeFare, isLocationSwapped) &&
        //                            isFareDisplayTrx)                      &&
        matchFareClass((*cb)->fareClass().c_str(), paxTypeFare->fareClass().c_str()))

    {
      ptFare.r2Cat10LocSwapped() = isLocationSwapped;
      return *cb;
    }

    ++cb;
  }

  return returnValue;
}

CombinabilityRuleInfo*
RuleUtil::getCombinabilityRuleInfo(PricingTrx& trx, PaxTypeFare& ptFare)
{
  // retrieve a vector of Combination rule candidates

  const PaxTypeFare* paxTypeFare(&ptFare);
  bool isLocationSwapped(false);
  const DateTime& travelDate = paxTypeFare->fareMarket()->travelDate();

  TariffNumber tcrRuleTariff;
  CarrierCode carrier;
  RuleNumber ruleNumber;
  VendorCode vendor;

  if (isFromCat25BaseFare(trx, *paxTypeFare) && !paxTypeFare->isSpecifiedFare())
  {
    paxTypeFare = paxTypeFare->baseFare(25);
    tcrRuleTariff = paxTypeFare->tcrRuleTariff();
    carrier = paxTypeFare->carrier();
    ruleNumber = paxTypeFare->ruleNumber();
    vendor = paxTypeFare->vendor();
  }
  else
  {
    carrier = paxTypeFare->carrier();
    if (paxTypeFare->fare()->isIndustry())
      carrier = INDUSTRY_CARRIER;

    tcrRuleTariff = paxTypeFare->tcrRuleTariff();
    ruleNumber = paxTypeFare->ruleNumber();
    vendor = paxTypeFare->vendor();
  }

  const std::vector<CombinabilityRuleInfo*>& combinationList =
      trx.dataHandle().getCombinabilityRule(vendor, carrier, tcrRuleTariff, ruleNumber, travelDate);

  // scan the vector and find the first match
  CombinabilityRuleInfo* returnValue = nullptr;

  std::vector<CombinabilityRuleInfo*>::const_iterator cb = combinationList.begin();
  std::vector<CombinabilityRuleInfo*>::const_iterator ce = combinationList.end();

  while (cb != ce)
  {
    if (matchOneWayRoundTrip((*cb)->owrt(), paxTypeFare->owrt()) &&
        matchFareRouteNumber((*cb)->routingAppl(), (*cb)->routing(), paxTypeFare->routingNumber()) &&
        matchFareType((*cb)->fareType(), paxTypeFare->fcaFareType()) &&
        matchSeasons((*cb)->seasonType(), paxTypeFare->fcaSeasonType()) &&
        matchDayOfWeek((*cb)->dowType(), paxTypeFare->fcaDowType()) &&
        matchFareFootnote((*cb)->footNote1(),
                          (*cb)->footNote2(),
                          paxTypeFare->footNote1(),
                          paxTypeFare->footNote2()) &&
        matchJointCarrier(trx, *paxTypeFare, (*cb)->jointCarrierTblItemNo()) &&
        matchLoc_R1_2_6(trx, (*cb)->locKey1(), (*cb)->locKey2(), *paxTypeFare, isLocationSwapped) &&
        matchFareClass((*cb)->fareClass().c_str(), paxTypeFare->fareClass().c_str()))

    {
      return *cb;
    }

    ++cb;
  }
  return returnValue;
}
//-------------------------------------------------------------------------------------
bool
RuleUtil::isFromCat25BaseFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  FareDisplayTrx* fTrx;
  bool rc = false;

  if (UNLIKELY(FareDisplayUtil::getFareDisplayTrx(&trx, fTrx)))
  {
    if (paxTypeFare.isFareByRule())
    {
      // check if Cat10 is from base fare
      if (paxTypeFare.fareByRuleInfo().ovrdcat10() == 'B')
        rc = true;
    }
  }
  return rc;
}

bool
RuleUtil::matchGeneralFareRuleNew(PricingTrx& trx,
                                  const PaxTypeFare& paxTypeFare,
                                  const GeneralFareRuleInfo& gfri,
                                  bool& isLocationSwapped)
{
  DCFactory* factory = nullptr;
  Diag502Collector* diag502 = nullptr;

  if (UNLIKELY(Diag502Collector::isDiagNeeded(trx, paxTypeFare, gfri.categoryRuleItemInfoSet())))
  {
    factory = DCFactory::instance();
    diag502 = dynamic_cast<Diag502Collector*>(factory->create(trx));

    if (diag502)
    {
      diag502->enable(Diagnostic502);
      diag502->diag502Collector(paxTypeFare, gfri);
    }
  }

  if (fallback::fixed::fallbackDisableESVIS())
  {
    // Bound fare
    BindingResult bindingResult(checkBindings(trx, paxTypeFare, gfri, isLocationSwapped, FARERULE));
    if (UNLIKELY(bindingResult.first))
    {
      return bindingResult.second;
    }
  }

  if (UNLIKELY(gfri.inhibit() == RuleConst::INHIBIT_IGNORE))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                            -- INHIBIT:INGORE --\n";
    }
  }
  else if (!matchOneWayRoundTrip(gfri.owrt(), paxTypeFare.owrt()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- OW/RT ---\n";
    }
  }
  else if (!matchFareRouteNumber(gfri.routingAppl(), gfri.routing(), paxTypeFare.routingNumber()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- RT NUM---\n";
    }
  }
  else if (!matchFareType(gfri.fareType(), paxTypeFare.fcaFareType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- FARE TYPE---\n";
    }
  }
  else if (!matchSeasons(gfri.seasonType(), paxTypeFare.fcaSeasonType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- SEASON ---\n";
    }
  }
  else if (!matchDayOfWeek(gfri.dowType(), paxTypeFare.fcaDowType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- DOW  ---\n";
    }
  }
  else if (!matchFareFootnote(gfri.footNote1(),
                              gfri.footNote2(),
                              paxTypeFare.footNote1(),
                              paxTypeFare.footNote2()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- FOOTNOTE---\n";
    }
  }
  else if (!matchLoc_R1_2_6(trx, gfri.loc1(), gfri.loc2(), paxTypeFare, isLocationSwapped))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- LOC  ---\n";
    }
  }
  else if (!matchFareClass(gfri.fareClass().c_str(), paxTypeFare.fareClass().c_str()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                 --- FARE CLASS---\n";
    }
  }
  else
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                 *** MATCH ***\n";
      diag502->flushMsg();
    }
    return true;
  }

  if (UNLIKELY(diag502))
  {
    diag502->flushMsg();
  }
  return false;
}

bool
RuleUtil::matchGeneralFareRule(PricingTrx& trx,
                               const PaxTypeFare& paxTypeFare,
                               const GeneralFareRuleInfo& gfri,
                               bool& isLocationSwapped)
{
  DCFactory* factory = nullptr;
  Diag502Collector* diag502 = nullptr;

  if (UNLIKELY(Diag502Collector::isDiagNeeded(trx, paxTypeFare, gfri.categoryRuleItemInfoSet())))
  {
    factory = DCFactory::instance();
    diag502 = dynamic_cast<Diag502Collector*>(factory->create(trx));

    if (diag502)
    {
      diag502->enable(Diagnostic502);
      diag502->diag502Collector(paxTypeFare, gfri);
    }
  }

  if (fallback::fixed::fallbackDisableESVIS())
  {
  // Bound fare
  BindingResult bindingResult(checkBindings(trx, paxTypeFare, gfri, isLocationSwapped, FARERULE));
  if (UNLIKELY(bindingResult.first))
  {
    return bindingResult.second;
  }
  }

  if (UNLIKELY(gfri.inhibit() == RuleConst::INHIBIT_IGNORE))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                            -- INHIBIT:INGORE --\n";
    }
  }
  else if (!matchOneWayRoundTrip(gfri.owrt(), paxTypeFare.owrt()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- OW/RT ---\n";
    }
  }
  else if (!matchFareRouteNumber(gfri.routingAppl(), gfri.routing(), paxTypeFare.routingNumber()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- RT NUM---\n";
    }
  }
  else if (!matchFareType(gfri.fareType(), paxTypeFare.fcaFareType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- FARE TYPE---\n";
    }
  }
  else if (!matchSeasons(gfri.seasonType(), paxTypeFare.fcaSeasonType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- SEASON ---\n";
    }
  }
  else if (!matchDayOfWeek(gfri.dowType(), paxTypeFare.fcaDowType()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- DOW  ---\n";
    }
  }
  else if (!matchFareFootnote(gfri.footNote1(),
                              gfri.footNote2(),
                              paxTypeFare.footNote1(),
                              paxTypeFare.footNote2()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- FOOTNOTE---\n";
    }
  }

  else if (UNLIKELY(!matchJointCarrier(trx, paxTypeFare, gfri.jointCarrierTblItemNo())))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- JNT CXR---\n";
    }
  }
  else if (!matchLoc_R1_2_6(trx, gfri.loc1(), gfri.loc2(), paxTypeFare, isLocationSwapped))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- LOC  ---\n";
    }
  }
  else if (!matchFareClass(gfri.fareClass().c_str(), paxTypeFare.fareClass().c_str()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                 --- FARE CLASS---\n";
    }
  }
  else
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                 *** MATCH ***\n";
      diag502->flushMsg();
    }
    return true;
  }

  if (UNLIKELY(diag502))
  {
    diag502->flushMsg();
  }
  return false;
}

bool
RuleUtil::matchOneWayRoundTrip(const Indicator owrtFromRule, const Indicator owrtFromFare)
{
  switch (owrtFromRule)
  {
  // a blank will match any OW/RT value

  case ' ':

    return true;

  // ROUND_TRIP1 will match a fare value of ROUND_TRIP1 or ROUND_TRIP3

  case ONE_WAY_MAY_BE_DOUBLED:

    if (owrtFromFare != ONE_WAY_MAY_BE_DOUBLED && owrtFromFare != ONE_WAY_MAYNOT_BE_DOUBLED)
    {
      return false;
    }

    break;

  // otherwise exact match to fare

  default:

    if (owrtFromRule != owrtFromFare)
    {
      return false;
    }
  }

  return true;
}

bool
RuleUtil::matchFareRouteNumber(Indicator routingAppl,
                               const RoutingNumber& rnFromRule,
                               const RoutingNumber& rnFromFare)
{
  if (routingAppl == RuleConst::NCROUTING_MATCH_CHAR) // 'S'-exact match to the RTG#
  {
    if (rnFromRule != rnFromFare)
      return false;
  }
  else if (UNLIKELY(routingAppl == RuleConst::ANY_SPECIFIED_ROUTING_MATCH_CHAR)) // 'R'-any specified routing
  {
    if (rnFromFare == "0000")
      return false;
  }
  else if (routingAppl == RuleConst::MPM_ROUTING_MATCH_CHAR) // 'M'-any mileage routing
  {
    if (rnFromFare != "0000")
      return false;
  }
  return true;
}

bool
RuleUtil::matchLocation(PricingTrx& trx,
                        const LocKey& loc1FromRule,
                        const LocKey& loc2FromRule,
                        const PaxTypeFare& paxTypeFare,
                        bool& isLocationSwapped,
                        const Footnote& footnote,
                        const TariffNumber& tariff)
{
  isLocationSwapped = false;

  if (loc1FromRule.locType() == RuleConst::ANY_LOCATION_TYPE &&
      loc2FromRule.locType() == RuleConst::ANY_LOCATION_TYPE)
  {
    return true;
  }

  return matchLoc(trx, loc1FromRule, loc2FromRule, paxTypeFare, isLocationSwapped);
}

bool
RuleUtil::matchGeo(PricingTrx& trx,
                   const LocKey& loc1FromRule,
                   const LocKey& loc2FromRule,
                   const LocCode& origin,
                   const LocCode& destination,
                   const VendorCode& vendorCode,
                   GeoTravelType geoTvlType,
                   const bool isRecord1_2_6)
{
  bool ret = true;

  LocUtil::ApplicationType locAppl = (isRecord1_2_6) ? LocUtil::RECORD1_2_6 : LocUtil::OTHER;

  if (loc1FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    ret = LocUtil::isInLoc(origin,
                           loc1FromRule.locType(),
                           loc1FromRule.loc(),
                           vendorCode,
                           RESERVED,
                           geoTvlType,
                           locAppl,
                           trx.getRequest()->ticketingDT());
  }

  if (ret && loc2FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    if (loc2FromRule.loc()[0] == RuleConst::DOLLAR_SIGN)
    {
      ret = !(LocUtil::isInLoc(destination,
                               loc1FromRule.locType(),
                               loc1FromRule.loc(),
                               vendorCode,
                               RESERVED,
                               geoTvlType,
                               locAppl,
                               trx.getRequest()->ticketingDT()));
    }
    else
    {
      ret = LocUtil::isInLoc(destination,
                             loc2FromRule.locType(),
                             loc2FromRule.loc(),
                             vendorCode,
                             RESERVED,
                             geoTvlType,
                             locAppl,
                             trx.getRequest()->ticketingDT());
    }
  }

  return ret;
}

bool
RuleUtil::matchGeo(PricingTrx& trx,
                   const LocKey& loc1FromRule,
                   const LocKey& loc2FromRule,
                   const LocCode& origin,
                   const LocCode& destination,
                   const Loc& originLoc,
                   const Loc& destinationLoc,
                   const VendorCode& vendorCode,
                   GeoTravelType geoTvlType)
{
  bool ret = true;

  if (loc1FromRule.locType() == LOCTYPE_CITY || loc1FromRule.locType() == LOCTYPE_AIRPORT)
  {
    ret = (origin == loc1FromRule.loc());
  }
  else if (LIKELY(loc1FromRule.locType() != LOCTYPE_NONE))
  {
    ret = LocUtil::isInLoc(originLoc,
                           loc1FromRule.locType(),
                           loc1FromRule.loc(),
                           vendorCode,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT());
  }

  if (!ret)
    return false;

  if (loc2FromRule.locType() == LOCTYPE_CITY || loc2FromRule.locType() == LOCTYPE_AIRPORT)
  {
    ret = (destination == loc2FromRule.loc());
  }
  else if (loc2FromRule.locType() != LOCTYPE_NONE)
  {
    if (UNLIKELY(loc2FromRule.loc()[0] == RuleConst::DOLLAR_SIGN))
    {
      ret = !(LocUtil::isInLoc(destinationLoc,
                               loc1FromRule.locType(),
                               loc1FromRule.loc(),
                               vendorCode,
                               RESERVED,
                               LocUtil::OTHER,
                               geoTvlType,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT()));
    }
    else
    {
      ret = LocUtil::isInLoc(destinationLoc,
                             loc2FromRule.locType(),
                             loc2FromRule.loc(),
                             vendorCode,
                             RESERVED,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT());
    }
  }

  return ret;
}

bool
RuleUtil::matchLoc_R1_2_6(PricingTrx& trx,
                          const LocKey& loc1FromRule,
                          const LocKey& loc2FromRule,
                          const PaxTypeFare& paxTypeFare,
                          bool& isLocationSwapped)
//                          const bool&        isFareDisplayTrx)
{
  isLocationSwapped = false;
  if (loc1FromRule.locType() == RuleConst::ANY_LOCATION_TYPE &&
      loc2FromRule.locType() == RuleConst::ANY_LOCATION_TYPE)
  {
    return true;
  }
  return matchLoc(trx, loc1FromRule, loc2FromRule, paxTypeFare, isLocationSwapped);
}

bool
RuleUtil::matchLoc(PricingTrx& trx,
                   const LocKey& loc1FromRule,
                   const LocKey& loc2FromRule,
                   const PaxTypeFare& paxTypeFare,
                   bool& isLocationSwapped)
{
  GeoTravelType geoTvlType = GeoTravelType::International;
  if (LIKELY(paxTypeFare.fareMarket() != nullptr))
  {
    geoTvlType = paxTypeFare.fareMarket()->geoTravelType();
  }
  const LocCode& origMarket =
      (paxTypeFare.isReversed()) ? paxTypeFare.fare()->market2() : paxTypeFare.fare()->market1();
  const LocCode& destMarket =
      (paxTypeFare.isReversed()) ? paxTypeFare.fare()->market1() : paxTypeFare.fare()->market2();

  if (matchGeo(trx,
               loc1FromRule,
               loc2FromRule,
               origMarket,
               destMarket,
               *paxTypeFare.fareMarket()->origin(),
               *paxTypeFare.fareMarket()->destination(),
               paxTypeFare.vendor(),
               geoTvlType))
  {
    return true;
  }
  else if (matchGeo(trx,
                    loc1FromRule,
                    loc2FromRule,
                    destMarket,
                    origMarket,
                    *paxTypeFare.fareMarket()->destination(),
                    *paxTypeFare.fareMarket()->origin(),
                    paxTypeFare.vendor(),
                    geoTvlType))
  {
    isLocationSwapped = true;
    return true;
  }
  return false;
}

bool
RuleUtil::matchGeo(PricingTrx& trx,
                   const LocKey& loc1FromRule,
                   const LocKey& loc2FromRule,
                   const Loc& origin,
                   const Loc& destination,
                   const VendorCode& vendorCode,
                   GeoTravelType geoTvlType)
{
  bool ret = true;

  if (loc1FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    ret = LocUtil::isInLoc(origin,
                           loc1FromRule.locType(),
                           loc1FromRule.loc(),
                           vendorCode,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT());
  }

  if (ret && loc2FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    if (loc2FromRule.loc()[0] == RuleConst::DOLLAR_SIGN)
    {
      ret = !(LocUtil::isInLoc(destination,
                               loc1FromRule.locType(),
                               loc1FromRule.loc(),
                               vendorCode,
                               RESERVED,
                               LocUtil::OTHER,
                               geoTvlType,
                               EMPTY_STRING(),
                               trx.getRequest()->ticketingDT()));
    }
    else
    {
      ret = LocUtil::isInLoc(destination,
                             loc2FromRule.locType(),
                             loc2FromRule.loc(),
                             vendorCode,
                             RESERVED,
                             LocUtil::OTHER,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT());
    }
  }

  return ret;
}

bool
RuleUtil::matchLocation(PricingTrx& trx,
                        const LocKey& loc1FromRule,
                        const LocCode& loc1ZoneFromRule,
                        const LocKey& loc2FromRule,
                        const LocCode& loc2ZoneFromRule,
                        const VendorCode& vendorCode,
                        const FareMarket& fareMarket,
                        bool& isLocationSwapped,
                        const CarrierCode& carrierCode)
{
  isLocationSwapped = false;

  GeoTravelType geoTvlType = fareMarket.geoTravelType();

  if (loc1FromRule.locType() == RuleConst::ANY_LOCATION_TYPE &&
      loc1ZoneFromRule == RuleConst::NOT_APPLICABLE_ZONE &&
      loc2FromRule.locType() == RuleConst::ANY_LOCATION_TYPE &&
      loc2ZoneFromRule == RuleConst::NOT_APPLICABLE_ZONE)
  {
    return true;
  }
  // match the original direction
  //
  else if (matchGeo(trx,
                    loc1FromRule,
                    loc1ZoneFromRule,
                    loc2FromRule,
                    loc2ZoneFromRule,
                    vendorCode,
                    *(fareMarket.origin()),
                    *(fareMarket.destination()),
                    geoTvlType,
                    carrierCode))
  {
    return true;
  }
  else if (matchGeo(trx,
                    loc1FromRule,
                    loc1ZoneFromRule,
                    loc2FromRule,
                    loc2ZoneFromRule,
                    vendorCode,
                    *(fareMarket.destination()),
                    *(fareMarket.origin()),
                    geoTvlType,
                    carrierCode))
  {
    isLocationSwapped = true;
    return true;
  }

  return false;
}
bool
RuleUtil::matchGeo(PricingTrx& trx,
                   const LocKey& loc1FromRule,
                   const LocCode& loc1ZoneFromRule,
                   const LocKey& loc2FromRule,
                   const LocCode& loc2ZoneFromRule,
                   const VendorCode& vendorCode,
                   const Loc& origin,
                   const Loc& destination,
                   GeoTravelType geoTvlType,
                   const CarrierCode& carrierCode)
{
  bool ret = true;

  if (loc1FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    ret = LocUtil::isInLoc(origin,
                           loc1FromRule.locType(),
                           loc1FromRule.loc(),
                           vendorCode,
                           RESERVED,
                           LocUtil::OTHER,
                           geoTvlType,
                           carrierCode,
                           trx.getRequest()->ticketingDT());
  }
  else if (LIKELY(loc1ZoneFromRule != RuleConst::NOT_APPLICABLE_ZONE))
  {
    ret = LocUtil::isInLoc(origin,
                           LOCTYPE_ZONE,
                           loc1ZoneFromRule,
                           vendorCode,
                           USER_DEFINED,
                           LocUtil::OTHER,
                           geoTvlType,
                           carrierCode,
                           trx.getRequest()->ticketingDT());
  }

  if (ret)
  {
    if (loc2FromRule.locType() != RuleConst::ANY_LOCATION_TYPE)
    {
      if (UNLIKELY(loc2FromRule.loc()[0] == RuleConst::DOLLAR_SIGN))
      {
        ret = !(LocUtil::isInLoc(destination,
                                 loc1FromRule.locType(),
                                 loc1FromRule.loc(),
                                 vendorCode,
                                 RESERVED,
                                 LocUtil::OTHER,
                                 geoTvlType,
                                 carrierCode,
                                 trx.getRequest()->ticketingDT()));
      }
      else
      {
        ret = LocUtil::isInLoc(destination,
                               loc2FromRule.locType(),
                               loc2FromRule.loc(),
                               vendorCode,
                               RESERVED,
                               LocUtil::OTHER,
                               geoTvlType,
                               carrierCode,
                               trx.getRequest()->ticketingDT());
      }
    }
    else if (loc2ZoneFromRule != RuleConst::NOT_APPLICABLE_ZONE)
    {
      ret = LocUtil::isInLoc(destination,
                             LOCTYPE_ZONE,
                             loc2ZoneFromRule,
                             vendorCode,
                             USER_DEFINED,
                             LocUtil::OTHER,
                             geoTvlType,
                             carrierCode,
                             trx.getRequest()->ticketingDT());
    }
  }

  return ret;
}

void
RuleUtil::collectAddonFootnote(const Footnote& footnote,
                               const TariffNumber& tariff,
                               bool isVendorATPCO,
                               std::vector<Footnote>& footnotes,
                               std::vector<TariffNumber>& fareTariffs)
{
  if (!footnote.empty() && !(isVendorATPCO && (footnote == "T" || footnote == "F")))
  {
    footnotes.push_back(footnote);
    fareTariffs.push_back(tariff);
  }
}

void
RuleUtil::getFootnotes(const PaxTypeFare& paxTypeFare,
                       std::vector<Footnote>& footnotes,
                       std::vector<TariffNumber>& fareTariffs)
{
  if (!paxTypeFare.footNote1().empty())
  {
    footnotes.push_back(paxTypeFare.footNote1());
    fareTariffs.push_back(paxTypeFare.fareTariff());
  }

  if (!paxTypeFare.footNote2().empty())
  {
    footnotes.push_back(paxTypeFare.footNote2());
    fareTariffs.push_back(paxTypeFare.fareTariff());
  }

  if (paxTypeFare.isConstructed())
  {
    bool isVendorATPCO = (paxTypeFare.vendor() == ATPCO_VENDOR_CODE);
    const ConstructedFareInfo& cfci = *(paxTypeFare.fare()->constructedFareInfo());

    if (cfci.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
        cfci.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    {
      collectAddonFootnote(
          cfci.origAddonFootNote1(), cfci.origAddonTariff(), isVendorATPCO, footnotes, fareTariffs);

      collectAddonFootnote(
          cfci.origAddonFootNote2(), cfci.origAddonTariff(), isVendorATPCO, footnotes, fareTariffs);
    }

    if (cfci.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
        cfci.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    {
      collectAddonFootnote(
          cfci.destAddonFootNote1(), cfci.destAddonTariff(), isVendorATPCO, footnotes, fareTariffs);

      collectAddonFootnote(
          cfci.destAddonFootNote2(), cfci.destAddonTariff(), isVendorATPCO, footnotes, fareTariffs);
    }
  }
}

void
RuleUtil::getFirstValidTvlDT(DateTime& rtnDT,
                             const RuleUtil::TravelSegWrapperVector& applTravelSegment,
                             const bool orig,
                             NoPNRPricingTrx* noPNRTrx)
{
  if (UNLIKELY(applTravelSegment.empty()))
    return;

  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI = applTravelSegment.begin();

  const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();
  if (LIKELY(orig))

  {
    DateTime checkedDT = ts->departureDT();
    if (UNLIKELY(noPNRTrx != nullptr))
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (LIKELY(checkedDT.isValid()))
    {
      rtnDT = checkedDT;
      return;
    }
  }
  else
  {
    DateTime checkedDT = ts->arrivalDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }

  // First seg is Open Seg with no valid date, find the closest no open date
  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentEndI = applTravelSegment.end();

  applTravelSegmentI++;

  for (; applTravelSegmentI != applTravelSegmentEndI; applTravelSegmentI++)
  {
    ts = (*applTravelSegmentI)->travelSeg();

    DateTime checkedDT = ts->departureDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }
}

void
RuleUtil::getFirstValidTvlDT(DateTime& startDT,
                             DateTime& endDT,
                             const RuleUtil::TravelSegWrapperVector& applTravelSegment,
                             const bool orig,
                             NoPNRPricingTrx* noPNRTrx)
{
  if (applTravelSegment.empty())
    return;

  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI = applTravelSegment.begin();

  const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();

  if (orig)

  {
    DateTime checkedStartDT = ts->earliestDepartureDT();
    DateTime checkedEndDT = ts->latestDepartureDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }
  else
  {
    DateTime checkedStartDT = ts->earliestArrivalDT();
    DateTime checkedEndDT = ts->latestArrivalDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }

  // First seg is Open Seg with no valid date, find the closest no open date
  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentEndI = applTravelSegment.end();

  applTravelSegmentI++;

  for (; applTravelSegmentI != applTravelSegmentEndI; applTravelSegmentI++)
  {
    ts = (*applTravelSegmentI)->travelSeg();

    DateTime checkedStartDT = ts->earliestDepartureDT();
    DateTime checkedEndDT = ts->latestDepartureDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }
}

void
RuleUtil::getLastValidTvlDT(DateTime& rtnDT,
                            const RuleUtil::TravelSegWrapperVector& applTravelSegment,
                            const bool orig,
                            NoPNRPricingTrx* noPNRTrx)
{
  RuleUtil::TravelSegWrapperVectorCI applTravelSegmentI = applTravelSegment.end();
  applTravelSegmentI--;

  const TravelSeg* ts = (*applTravelSegmentI)->travelSeg();

  if (UNLIKELY(ts->openSegAfterDatedSeg() && noPNRTrx == nullptr))
  {
    rtnDT = DateTime::openDate();
    return;
  }

  if (orig)
  {
    DateTime checkedDT = ts->departureDT();
    if (UNLIKELY(noPNRTrx != nullptr))
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (LIKELY(checkedDT.isValid()))
    {
      rtnDT = checkedDT;
      return;
    }
  }
  else
  {
    DateTime checkedDT = ts->arrivalDT();
    if (UNLIKELY(noPNRTrx != nullptr))
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }

  // Last seg is Open Seg with no valid date, find the closest no open date
  while (applTravelSegmentI != applTravelSegment.begin())
  {
    applTravelSegmentI--;
    ts = (*applTravelSegmentI)->travelSeg();

    DateTime checkedDT = ts->arrivalDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }
}

void
RuleUtil::getFirstValidTvlDT(DateTime& rtnDT,
                             const std::vector<TravelSeg*>& travelSegs,
                             const bool orig,
                             NoPNRPricingTrx* noPNRTrx)
{
  if (UNLIKELY(travelSegs.empty()))
    return;

  std::vector<TravelSeg*>::const_iterator tvlSegI = travelSegs.begin();

  TravelSeg* ts = *tvlSegI;

  if (UNLIKELY(ts->openSegAfterDatedSeg() && noPNRTrx == nullptr))
  {
    rtnDT = DateTime::openDate();
    return;
  }

  if (LIKELY(orig))
  {
    DateTime checkedDT = ts->departureDT();
    if (UNLIKELY(noPNRTrx != nullptr))
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (LIKELY(checkedDT.isValid()))
    {
      rtnDT = checkedDT;
      return;
    }
  }
  else
  {
    DateTime checkedDT = ts->arrivalDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }

  // First seg is Open Seg with no valid date, find the closest no open date
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = travelSegs.end();

  tvlSegI++;

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    ts = *tvlSegI;

    DateTime checkedDT = ts->departureDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }
}

void
RuleUtil::getFirstValidTvlDT(DateTime& startDT,
                             DateTime& endDT,
                             const std::vector<TravelSeg*>& travelSegs,
                             const bool orig,
                             NoPNRPricingTrx* noPNRTrx)
{
  if (travelSegs.empty())
    return;

  std::vector<TravelSeg*>::const_iterator tvlSegI = travelSegs.begin();

  TravelSeg* ts = *tvlSegI;

  if (ts->openSegAfterDatedSeg() && noPNRTrx == nullptr)
  {
    startDT = DateTime::openDate();
    endDT = DateTime::openDate();
    return;
  }

  if (orig)
  {
    DateTime checkedStartDT = ts->earliestDepartureDT();
    DateTime checkedEndDT = ts->latestDepartureDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }
  else
  {
    DateTime checkedStartDT = ts->earliestArrivalDT();
    DateTime checkedEndDT = ts->latestArrivalDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }

  // First seg is Open Seg with no valid date, find the closest no open date
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = travelSegs.end();

  tvlSegI++;

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    ts = *tvlSegI;

    DateTime checkedStartDT = ts->earliestDepartureDT();
    DateTime checkedEndDT = ts->latestDepartureDT();
    if (noPNRTrx != nullptr)
    {
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedStartDT);
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedEndDT);
    }

    if (checkedStartDT.isValid() && checkedEndDT.isValid())
    {
      startDT = checkedStartDT;
      endDT = checkedEndDT;
      return;
    }
  }
}

void
RuleUtil::getLastValidTvlDT(DateTime& rtnDT,
                            const std::vector<TravelSeg*>& travelSegs,
                            const bool orig,
                            NoPNRPricingTrx* noPNRTrx)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = travelSegs.end();

  if (tvlSegI == travelSegs.begin()) // empty check
    return;

  tvlSegI--;

  const TravelSeg* ts = *tvlSegI;

  if (ts->openSegAfterDatedSeg() && noPNRTrx == nullptr)
  {
    rtnDT = DateTime::openDate();
    return;
  }

  if (orig)
  {
    DateTime checkedDT = ts->departureDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }
  else
  {
    DateTime checkedDT = ts->arrivalDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (LIKELY(checkedDT.isValid()))
    {
      rtnDT = checkedDT;
      return;
    }
  }

  // Last seg is Open Seg with no valid date, find the closest no open date
  while (tvlSegI != travelSegs.begin())
  {
    tvlSegI--;
    ts = *tvlSegI;

    DateTime checkedDT = ts->arrivalDT();
    if (noPNRTrx != nullptr)
      noPNRTrx->updateOpenDateIfNeccesary(ts, checkedDT);

    if (checkedDT.isValid())
    {
      rtnDT = checkedDT;
      return;
    }
  }
}

void
RuleUtil::diagGeoTblItem(const uint32_t itemNo,
                         const VendorCode& vendorCode,
                         PricingTrx& trx,
                         DiagCollector& diag)
{
  LocKey locKey1;
  LocKey locKey2;
  bool checkOrig = false;
  bool checkDest = false;
  TSICode tsi = 0;

  if (!getOrigDestLocFromGeoRuleItem(
          itemNo, vendorCode, trx, checkOrig, checkDest, tsi, locKey1, locKey2))
  {
    diag << itemNo << ":???" << std::endl;
  }

  if (locKey1.locType() != LOCTYPE_NONE)
  {
    if (locKey2.locType() == LOCTYPE_NONE)
    {
      if (tsi)
      {
        if (checkOrig && checkDest)
        {
          diag << "FROM/TO";
        }
        else
        {
          if (checkOrig)
            diag << "FROM";
          else if (checkDest)
            diag << "TO";
        }
      }
      diagLocKey(trx, locKey1, diag);
    }
    else
    {
      if (tsi)
      {
      }
      diagLocKey(trx, locKey1, diag);
      diag << "   ";
      diagLocKey(trx, locKey2, diag);
    }
  }

  if (tsi)
  {
    diag << " TSI " << tsi;

    const TSIInfo* tsiInfo = trx.dataHandle().getTSI(tsi);

    if (!tsiInfo)
    {
      diag << " INVALID TSI";
    }
    else
    {
      diag << tsiInfo->description();
    }
  }
}

void
RuleUtil::diagLocKey(PricingTrx& trx, const LocKey& locKey1, DiagCollector& diag)
{
  const Loc* loc1 = trx.dataHandle().getLoc(locKey1.loc(), trx.ticketingDate());
  if (loc1)
  {
    diag << loc1->description();
  }
  else
  {
    diag << locKey1.locType() << " " << locKey1.loc();
  }
}

bool
RuleUtil::isInDirection(PricingTrx& trx,
                        const TravelSeg& tvlSeg,
                        const std::vector<TravelSeg*>& tvlSegs,
                        const LocKey& locFrom,
                        const LocKey& locTo)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    if ((LOCTYPE_NONE == locFrom.locType()) || LocUtil::isInLoc(*((*tvlSegI)->origin()),
                                                                locFrom.locType(),
                                                                locFrom.loc(),
                                                                EMPTY_VENDOR,
                                                                RESERVED,
                                                                LocUtil::OTHER,
                                                                GeoTravelType::International,
                                                                EMPTY_STRING(),
                                                                trx.getRequest()->ticketingDT()))
    {
      // Found first travel segment of Fare Component matched direction
      break;
    }
    if (*tvlSegI == &tvlSeg)
    {
      // not in the direction
      return false;
    }
  }

  // check tvlSeg destination is before locTo
  // walk tvlSegI from where tvlSeg is stored to the end
  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    if (*tvlSegI == &tvlSeg)
      break;
  }

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    if ((LOCTYPE_NONE == locTo.locType()) || LocUtil::isInLoc(*((*tvlSegI)->destination()),
                                                              locTo.locType(),
                                                              locTo.loc(),
                                                              EMPTY_VENDOR,
                                                              RESERVED,
                                                              LocUtil::OTHER,
                                                              GeoTravelType::International,
                                                              EMPTY_STRING(),
                                                              trx.getRequest()->ticketingDT()))
    {
      return true;
    }
  }

  return false;
}

bool
RuleUtil::isWithinLoc(PricingTrx& trx, const TravelSeg& tvlSeg, const LocKey& loc1)
{
  return (LocUtil::isInLoc(*tvlSeg.origin(),
                           loc1.locType(),
                           loc1.loc(),
                           EMPTY_VENDOR,
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()) &&
          LocUtil::isInLoc(*tvlSeg.destination(),
                           loc1.locType(),
                           loc1.loc(),
                           EMPTY_VENDOR,
                           RESERVED,
                           LocUtil::OTHER,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           trx.getRequest()->ticketingDT()));
}

void
RuleUtil::displayCPWarning(std::ostringstream& output, const uint32_t failedStat, bool asCatNum)
{
  uint32_t currBit = 0x00000001;

  uint32_t category = 1;

  for (; category <= PaxTypeFare::PTFF_Max_Numbered; category++)
  {
    if (failedStat & currBit)
    {
      if (asCatNum)
      {
        output << "  " << category;
      }
    }
    currBit = currBit << 1;
  }
  // TODO for category higher than 25
  if (failedStat & PaxTypeFare::PTFF_Cat35)
  {
    if (asCatNum)
    {
      output << "  35";
    }
    else
    {
    }
  }
  if (failedStat & PaxTypeFare::PTFF_RBD)
  {
    if (asCatNum)
    {
      output << "  RBD";
    }
  }
}

bool
RuleUtil::matchFootNoteCtrlInfoNew(PricingTrx& trx,
                                   const FootNoteCtrlInfo& fnci,
                                   const PaxTypeFare& paxTypeFare,
                                   bool& isLocationSwapped,
                                   const Footnote& footnote,
                                   const TariffNumber& tariff)
{
  DCFactory* factory = nullptr;
  Diag502Collector* diag502 = nullptr;

  if (_UNLIKELY(Diag502Collector::isDiagNeeded(trx, paxTypeFare, fnci.categoryRuleItemInfoSet())))
  {
    factory = DCFactory::instance();
    diag502 = dynamic_cast<Diag502Collector*>(factory->create(trx));

    if (diag502)
    {
      diag502->enable(Diagnostic502);
      diag502->diag502Collector(paxTypeFare, fnci);
    }
  }

  if (fallback::fixed::fallbackDisableESVIS())
  {
  // Bound fare
  // joint cxr & match location can be removed from binding in this version
  // binding do not need isLocationSwapped when GFR handling added
  BindingResult bindingResult(checkBindings(trx, paxTypeFare, fnci, isLocationSwapped, FOOTNOTE));
  if (_UNLIKELY(bindingResult.first))
  {
    return bindingResult.second;
  }
  }

  if (_UNLIKELY(fnci.inhibit() == RuleConst::INHIBIT_IGNORE))
  {
    if (_UNLIKELY(diag502))
    {
      *diag502 << "                            -- INHIBIT:INGORE --\n";
    }
  }
  else if (!RuleUtil::matchOneWayRoundTrip(fnci.owrt(), paxTypeFare.owrt()))
  {
    if (_UNLIKELY(diag502))
    {
      *diag502 << "                                  --- OW/RT ---\n";
    }
  }
  else if (!RuleUtil::matchFareRouteNumber(
               fnci.routingAppl(), fnci.routing(), paxTypeFare.routingNumber()))
  {
    if (_UNLIKELY(diag502))
    {
      *diag502 << "                                  --- RT NUM---\n";
    }
  }
  else if (!RuleUtil::matchFareClass(fnci.fareClass().c_str(), paxTypeFare.fareClass().c_str()))
  {
    if (_UNLIKELY(diag502))
    {
      *diag502 << "                                 --- FARE CLASS---\n";
    }
  }
  else
  {
    if (_UNLIKELY(diag502))
    {
      *diag502 << "                                  *** MATCH ***\n";
      diag502->flushMsg();
    }
    return true;
  }

  if (_UNLIKELY(diag502))
  {
    diag502->flushMsg();
  }

  return false;
}

bool
RuleUtil::matchFootNoteCtrlInfo(PricingTrx& trx,
                                const FootNoteCtrlInfo& fnci,
                                const PaxTypeFare& paxTypeFare,
                                bool& isLocationSwapped,
                                const Footnote& footnote,
                                const TariffNumber& tariff)
{
  DCFactory* factory = nullptr;
  Diag502Collector* diag502 = nullptr;

  if (UNLIKELY(Diag502Collector::isDiagNeeded(trx, paxTypeFare, fnci.categoryRuleItemInfoSet())))
  {
    factory = DCFactory::instance();
    diag502 = dynamic_cast<Diag502Collector*>(factory->create(trx));

    if (diag502)
    {
      diag502->enable(Diagnostic502);
      diag502->diag502Collector(paxTypeFare, fnci);
    }
  }

  if (fallback::fixed::fallbackDisableESVIS())
  {
  // Bound fare
  BindingResult bindingResult(checkBindings(trx, paxTypeFare, fnci, isLocationSwapped, FOOTNOTE));
  if (UNLIKELY(bindingResult.first))
  {
    return bindingResult.second;
  }
  }

  if (UNLIKELY(fnci.inhibit() == RuleConst::INHIBIT_IGNORE))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                            -- INHIBIT:INGORE --\n";
    }
  }
  else if (!RuleUtil::matchOneWayRoundTrip(fnci.owrt(), paxTypeFare.owrt()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- OW/RT ---\n";
    }
  }
  else if (!RuleUtil::matchFareRouteNumber(fnci.routingAppl(), fnci.routing(), paxTypeFare.routingNumber()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- RT NUM---\n";
    }
  }

  else if (UNLIKELY(!RuleUtil::matchJointCarrier(trx, paxTypeFare, fnci.jointCarrierTblItemNo())))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- JNT CXR---\n";
    }
  }
  else if (!RuleUtil::matchLocation(
               trx, fnci.loc1(), fnci.loc2(), paxTypeFare, isLocationSwapped, footnote, tariff))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  --- LOC  ---\n";
    }
  }
  else if (!RuleUtil::matchFareClass(fnci.fareClass().c_str(), paxTypeFare.fareClass().c_str()))
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                 --- FARE CLASS---\n";
    }
  }
  else
  {
    if (UNLIKELY(diag502))
    {
      *diag502 << "                                  *** MATCH ***\n";
      diag502->flushMsg();
    }
    return true;
  }

  if (UNLIKELY(diag502))
  {
    diag502->flushMsg();
  }

  return false;
}

void
RuleUtil::processCurrencyAdjustment(PricingTrx& trx, FarePath& farePath)
{
  if (!isSalePointAppl(trx) || !isTktCarrierAppl(trx, farePath))
    return;

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    if (UNLIKELY(LocUtil::isNigeria(*(pu->travelSeg()[0]->origin()))))
    {
      pu->nigeriaCurrencyAdjustment() = true;
      validateCurrAdjPU(trx, farePath, *pu);
    }
  }
}

bool
RuleUtil::isSalePointAppl(PricingTrx& trx)
{
  const Loc* locSale = nullptr;

  // Check Point of Sale
  if (!trx.getRequest()->salePointOverride().empty()) // Point of Sale overriden
  {
    locSale = trx.dataHandle().getLoc(trx.getRequest()->salePointOverride(),
                                      trx.getRequest()->ticketingDT());
    if (locSale == nullptr)
    {
      return false;
    }
  }
  else
  {
    locSale = trx.getRequest()->ticketingAgent()->agentLocation();
  }

  return (!LocUtil::isDomesticUSCA(*locSale) && !LocUtil::isNigeria(*locSale));
}

void
RuleUtil::validateCurrAdjPU(PricingTrx& trx, FarePath& farePath, PricingUnit& pricingUnit)
{
  // Do check for each fareUsage
  for (FareUsage* fareUsage : pricingUnit.fareUsage())
  {
    if (isNigeriaIntl(*fareUsage))
    {
      const PaxTypeFare* aptf = nullptr;
      aptf = selectAdjustedFare(
          trx, farePath, pricingUnit, *fareUsage); // search adjusted fare in opposite direction

      if (aptf != nullptr)
      {
        MoneyAmount plusUpAm = (aptf->nucFareAmount() + aptf->mileageSurchargeAmt()) -
                               (fareUsage->paxTypeFare()->nucFareAmount() +
                                fareUsage->paxTypeFare()->mileageSurchargeAmt());

        if (plusUpAm > 0)
        {
          // update FarePath, fareUsage.
          fareUsage->adjustedPaxTypeFare() = aptf;
          //          farePath.plusUpAmount()   += plusUpAmountFP;
          farePath.plusUpFlag() = true;
          farePath.increaseTotalNUCAmount(plusUpAm);
        }
      }

      if (farePath.trailerCurrAdjMsg().empty())
      {
        farePath.trailerCurrAdjMsg() = "NIGERIA CURRENCY ADJUSTMENT APPLIED";
      }
    }
  }
}

bool
RuleUtil::isTktCarrierAppl(PricingTrx& trx, FarePath& farePath)
{
  CarrierCode tktCarrier;

  if (LIKELY(!trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() ||
      !trx.getRequest()->ticketingAgent()->tvlAgencyIATA().empty()))
  {
    if (UNLIKELY(!trx.getRequest()->validatingCarrier().empty()))
    {
      tktCarrier = trx.getRequest()->validatingCarrier(); // validatedCarrier was overrided
    }
    else
    {
      tktCarrier = farePath.itin()->ticketingCarrier();
    }
  }
  else if (!trx.billing()->partitionID().empty())
  {
    // Airline agent
    tktCarrier = trx.billing()->partitionID();
  }
  else // unknown
  {
    tktCarrier = farePath.itin()->ticketingCarrier();
  }

  const DateTime& travelDate = farePath.itin()->travelSeg().front()->departureDT();
  const CarrierPreference* cp = trx.dataHandle().getCarrierPreference(tktCarrier, travelDate);

  return (cp && cp->noApplynigeriaCuradj() != YES);
}

bool
RuleUtil::isNigeriaIntl(FareUsage& fu)
{
  FareMarket* fm = fu.paxTypeFare()->fareMarket();
  const Loc* locOrig = fm->origin();
  const Loc* locDest = fm->destination();

  return ((LocUtil::isNigeria(*locOrig) && !LocUtil::isNigeria(*locDest)) ||
          (!LocUtil::isNigeria(*locOrig) && LocUtil::isNigeria(*locDest)));
}

const PaxTypeFare*
RuleUtil::selectAdjustedFare(PricingTrx& trx,
                             FarePath& farePath,
                             PricingUnit& pricingUnit,
                             FareUsage& fu,
                             DateTime travelDate)
{
  const PaxTypeFare* ptf = nullptr;

  // parameter added for historical data retrieval
  if (travelDate == DateTime::emptyDate())
    travelDate = farePath.itin()->travelDate();

  PaxTypeFare& thruFare = *fu.paxTypeFare();
  ptf = MinFareLogic::selectQualifyFare(NCJ,
                                        trx,
                                        *farePath.itin(),
                                        thruFare,
                                        *farePath.paxType(),
                                        thruFare.cabin(),
                                        thruFare.isNormal(),
                                        fu.isInbound() ? MinFareFareSelection::OUTBOUND
                                                       : MinFareFareSelection::INBOUND,
                                        MinFareLogic::eligibleFare(pricingUnit),
                                        thruFare.fareMarket()->travelSeg(),
                                        travelDate,
                                        nullptr,
                                        nullptr,
                                        nullptr,
                                        &farePath,
                                        &pricingUnit);

  return ptf;
}

const RuleItemInfo*
RuleUtil::getRuleItemInfo(PricingTrx& trx,
                          const CategoryRuleInfo* const rule,
                          const CategoryRuleItemInfo* const item,
                          const DateTime& applDate)
{
  const RuleItemInfo* ruleItemInfo = trx.dataHandle().getRuleItemInfo(rule, item, applDate);

  if (ruleItemInfo)
  {
    return ruleItemInfo;
  }

  uint32_t categoryNum = item->itemcat();

  // DataHandle::getRuleItemInfo do nothing with Cat25 and Cat10
  // so we log error for everything else but them
  if ((categoryNum != RuleConst::FARE_BY_RULE) && (categoryNum != RuleConst::COMBINABILITY_RULE))
  {
    std::ostringstream errMsg;
    errMsg << "VENDOR:" << rule->vendorCode() << " CAT " << categoryNum << " RECORD3 ITEM "
           << item->itemNo() << " MISSING";
    LOG4CXX_ERROR(_dataErrLogger, errMsg.str());

    DiagManager diag(trx, Diagnostic500);
    errMsg << std::endl;
    diag << errMsg.str();
  }
  return ruleItemInfo;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int RuleUtil::useSameCarrier
//
// </PRE>
// ----------------------------------------------------------------------------
bool
RuleUtil::useSameCarrier(const std::vector<TravelSeg*>& tvlSegs)
{
  const CarrierCode* desiredCarrier = nullptr;

  for (TravelSeg* tvlSeg : tvlSegs)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
    if (airSeg)
    {
      if (desiredCarrier == nullptr)
        desiredCarrier = &(airSeg->carrier());
      else
      {
        if (airSeg->carrier() != *desiredCarrier)
          return false;
      }
    }
  }

  return true;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int RuleUtil::useCxrFltTblFlts
//     return  true  - all air segs use carrier defined by CarrierFltTbl(Tbl986)
//            false  - at least one air seg does not use carrier in Tbl986
//
// </PRE>
// ----------------------------------------------------------------------------
bool
RuleUtil::useCxrInCxrFltTbl(const std::vector<TravelSeg*>& tvlSegs,
                            const VendorCode& vendor,
                            const int carrierFltTblItemNo,
                            const DateTime& ticketingDate)
{
  DataHandle localDataHandle(ticketingDate);
  const CarrierFlight* table986 = localDataHandle.getCarrierFlight(vendor, carrierFltTblItemNo);

  if (!table986 || (table986->segCnt() == 0))
  {
    LOG4CXX_ERROR(_logger, "FAILED TO READ TBL986 of " << vendor << " #" << carrierFltTblItemNo);
    return false;
  }
  return std::none_of(tvlSegs.begin(),
                      tvlSegs.end(),
                      [table986](TravelSeg* tvlSeg)
                      {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);

    return (airSeg && !useCxrInCxrFltTbl(*airSeg, *table986));
  });
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int RuleUtil::useCxrFltTblFlts
//     return  true  - air seg use carrier defined by CarrierFltTbl(Tbl986)
//            false  - air seg does not use carrier in Tbl986
//
// </PRE>
// ----------------------------------------------------------------------------
bool
RuleUtil::useCxrInCxrFltTbl(const AirSeg& airSeg, const CarrierFlight& table986)
{
  for (CarrierFlightSeg* item : table986.segs())
  {
    if ((!item->marketingCarrier().empty()) && (airSeg.carrier() != item->marketingCarrier()))
    {
      continue;
    }

    if ((!item->operatingCarrier().empty()) &&
        (airSeg.operatingCarrierCode() != item->operatingCarrier()))
    {
      continue;
    }

    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int RuleUtil::useSameTariffRule
//     return  true  - air seg use carrier defined by CarrierFltTbl(Tbl986)
//            false  - air seg does not use carrier in Tbl986
//
// </PRE>
// ----------------------------------------------------------------------------
bool
RuleUtil::useSameTariffRule(const std::vector<const PaxTypeFare*>& paxTypeFares,
                            const bool needSameRule)
{
  if (paxTypeFares.size() < 2)
    return true;

  std::vector<const PaxTypeFare*>::const_iterator ptfIter = paxTypeFares.begin();

  const TariffNumber& ruleTariff = (*ptfIter)->tcrRuleTariff();
  const RuleNumber& ruleNumber = (*ptfIter)->ruleNumber();

  ptfIter++;
  std::vector<const PaxTypeFare*>::const_iterator ptfIterEnd = paxTypeFares.end();

  for (; ptfIter != ptfIterEnd; ptfIter++)
  {
    if (needSameRule && (ruleNumber != (*ptfIter)->ruleNumber()))
      return false;
    if (ruleTariff != (*ptfIter)->tcrRuleTariff())
      return false;
  }
  return true;
}

void
RuleUtil::getExcSTOSurchargeOverride(ExchangePricingTrx& excTrx, FarePath& farePath, FareUsage& fu)
{
  if (excTrx.exchangeOverrides().stopoverOverride().empty())
    return;

  const bool isLastFU = (fu.travelSeg().back() == farePath.itin()->travelSeg().back());
  // if (isLastFU), need to collect stopover surcharge override on journey

  if (!isLastFU && !fu.paxTypeFare()->isDummyFare())
    return;

  CurrencyCode& ovrdCurr = (farePath.itin()->calcCurrencyOverride().empty())
                               ? (farePath.itin()->calculationCurrency())
                               : (farePath.itin()->calcCurrencyOverride());
  Money soOvrdMoney(ovrdCurr);
  CurrencyNoDec soOvrdNoDec = soOvrdMoney.noDec(excTrx.ticketingDate());

  for (TravelSeg* tvlSeg : fu.travelSeg())
  {
    const TravelSeg* tvlSegCharged = tvlSeg;
    if (isLastFU && (tvlSegCharged == fu.travelSeg().back()))
      tvlSegCharged = nullptr;

    for (StopoverOverride* stopoverOverride : excTrx.exchangeOverrides().stopoverOverride())
    {
      if (stopoverOverride->travelSeg() == tvlSegCharged)
      {
        FareUsage::StopoverSurcharge* fuSOSurcharge = nullptr;
        excTrx.dataHandle().get(fuSOSurcharge);
        if (!fuSOSurcharge)
          return;

        fuSOSurcharge->amount() = stopoverOverride->amount();
        fuSOSurcharge->currencyCode() = ovrdCurr;
        fuSOSurcharge->noDecimals() = soOvrdNoDec;
        fuSOSurcharge->travelSeg() = tvlSegCharged;
        fuSOSurcharge->isSegmentSpecific() = (tvlSegCharged != nullptr);
        if (!(stopoverOverride->fromExchange()))
          fuSOSurcharge->isFromOverride() = true;
        fu.stopoverSurcharges().insert(std::make_pair(tvlSeg, fuSOSurcharge));

        // surcharge override currency is same as calculation currency
        // override, so no need to support convertion
        farePath.increaseTotalNUCAmount(fuSOSurcharge->amount());
      }
    }
  }
}

void
RuleUtil::getFPPlusUpOverride(ExchangePricingTrx& excTrx, FarePath& farePath)
{
  if (excTrx.exchangeOverrides().plusUpOverride().empty())
    return;

  // insert plusUp according to module type in FarePath

  MoneyAmount plusUpAmount = 0.0;

  for (PlusUpOverride* plusUpOverride : excTrx.exchangeOverrides().plusUpOverride())
  {
    if (plusUpOverride->travelSeg() == nullptr)
    {
      if (plusUpOverride->moduleName() == OSC)
      {
        FarePath::OscPlusUp* oscPlusUp =
            dynamic_cast<FarePath::OscPlusUp*>(plusUpOverride->plusUpItem());
        if (oscPlusUp != nullptr)
        {
          farePath.oscPlusUp().push_back(oscPlusUp);
          plusUpAmount += oscPlusUp->plusUpAmount;
        }
      }
      else if (plusUpOverride->moduleName() == RSC)
      {
        FarePath::RscPlusUp* rscPlusUp =
            dynamic_cast<FarePath::RscPlusUp*>(plusUpOverride->plusUpItem());
        if (rscPlusUp != nullptr)
        {
          farePath.rscPlusUp().push_back(rscPlusUp);
          plusUpAmount += rscPlusUp->plusUpAmount;
        }
      }
    }
  }
  farePath.increaseTotalNUCAmount(plusUpAmount);
}

void
RuleUtil::getFUPlusUpOverride(ExchangePricingTrx& excTrx, FareUsage& fu, FarePath& farePath)
{
  if (excTrx.exchangeOverrides().plusUpOverride().empty())
    return;

  // Then find matching module for the last travelSeg of the fareUsage
  // insert plusUp according to module type in FareUsage
  MoneyAmount plusUpAmount = 0.0;

  for (PlusUpOverride* plusUpOverride : excTrx.exchangeOverrides().plusUpOverride())
  {
    if ((plusUpOverride->moduleName() == RSC) || (plusUpOverride->moduleName() == OSC))
      continue; // RSC or OSC is Fare Path plus up

    if ((plusUpOverride->travelSeg() == fu.travelSeg().back()) ||
        ((plusUpOverride->travelSeg() == nullptr) &&
         (fu.travelSeg().back() == farePath.itin()->travelSeg().back())))
    {
      BhcPlusUpItem* bhcPlusUp = dynamic_cast<BhcPlusUpItem*>(plusUpOverride->plusUpItem());
      if (bhcPlusUp != nullptr)
      {
        fu.minFarePlusUp().addItem(plusUpOverride->moduleName(), bhcPlusUp);
        plusUpAmount += bhcPlusUp->plusUpAmount;
      }
    }
  }
  farePath.increaseTotalNUCAmount(plusUpAmount);
}

bool
RuleUtil::matchCreateOrExpireDate(const DateTime& tktDate,
                                  const DateTime& createDate,
                                  const DateTime& expireDate)
{
  return (tktDate.date() == createDate.date() || tktDate.date() == expireDate.date());
}

bool
RuleUtil::findBtwTvlSegs(const RuleUtil::TravelSegWrapperVector& applBtwTvlSegs,
                         const RuleUtil::TravelSegWrapperVector& applAndTvlSegs,
                         const std::vector<TravelSeg*>& allTvlSegs,
                         std::vector<TravelSeg*>& matchedTvlSegs)
{
  RuleUtil::TravelSegWrapperVector::const_reverse_iterator tswBtwI = applBtwTvlSegs.rbegin();
  const RuleUtil::TravelSegWrapper* tswBtwLast = applBtwTvlSegs.front();

  RuleUtil::TravelSegWrapperVector::const_iterator tswAndI = applAndTvlSegs.begin();
  const RuleUtil::TravelSegWrapper* tswAndLast = applAndTvlSegs.back();

  while ((*tswBtwI)->travelSeg()->departureDT() > (*tswAndI)->travelSeg()->departureDT())
  {
    if ((*tswBtwI) != tswBtwLast)
    {
      tswBtwI++;
      continue;
    }
    if ((*tswAndI) != tswAndLast)
    {
      tswAndI++;
      continue;
    }
    return false;
  }

  std::vector<TravelSeg*>::const_iterator tsI = allTvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tsIEnd = allTvlSegs.end();

  bool startOfMatch = false;
  for (; tsI != tsIEnd; tsI++)
  {
    if (!startOfMatch)
    {
      if ((*tsI) != (*tswBtwI)->travelSeg())
        continue;

      startOfMatch = true;
    }
    matchedTvlSegs.push_back(*tsI);
    if ((*tsI) == (*tswAndI)->travelSeg())
      break;
  }
  return startOfMatch;
}

DateTime
RuleUtil::addPeriodToDate(const DateTime& travelDate, const PeriodOfStay& stayPeriod)
{
  if (UNLIKELY(!stayPeriod.isValid()))
  {
    LOG4CXX_INFO(_logger, " StayPeriod not valid, Leaving RuleUtil::addPeriodToDate()");
    return DateTime(neg_infin);
  }
  if (stayPeriod.isDayOfWeek())
  {
    // stay must be: SUN - SAT and stay unit is:
    // 01 - 52
    return travelDate.getFutureDate(stayPeriod.dayOfWeek(), stayPeriod.unit());
  }

  // stay must be: 001 - 999 and stay unit is:
  // either N (minutes) ,H (hours) ,D (days)  or M (months)
  switch (stayPeriod.unit())
  {
  case PeriodOfStay::MINUTES:
    return travelDate + Minutes((int)stayPeriod);

  case PeriodOfStay::HOURS:
    return travelDate + Hours((int)stayPeriod);

  case PeriodOfStay::DAYS:
    return travelDate.addDays((int)stayPeriod);

  case PeriodOfStay::MONTHS:
    return travelDate.addMonths((int)stayPeriod);

  default:
    break;
  }
  return DateTime(neg_infin);
}

bool
RuleUtil::validateDutyFunctionCode(const Agent* agent, const AgencyDutyCode& dbDutyFunctionCode)
{
  if (UNLIKELY(agent->agentDuty().empty()))
    return false;
  const char& duty1 = dbDutyFunctionCode[0];
  const char& duty2 = agent->agentDuty().at(0);
  if (LIKELY(duty1 != duty2))
  {
    // wacky mapping between db and SABRE 0123456789ABCDE <=> 0123456789$@*-/
    if ((duty1 >= '0' && duty1 <= '9') ||
        (duty1 == RuleConst::DB_SABRE_A && duty2 != RuleConst::DB_SABRE_DOLLAR) ||
        (duty1 == RuleConst::DB_SABRE_B && duty2 != RuleConst::DB_SABRE_AMP) ||
        (duty1 == RuleConst::DB_SABRE_C && duty2 != RuleConst::DB_SABRE_STAR) ||
        (duty1 == RuleConst::DB_SABRE_D && duty2 != RuleConst::DB_SABRE_DASH) ||
        (duty1 == RuleConst::DB_SABRE_E && duty2 != RuleConst::DB_SABRE_SLASH))
      return false;
  }
  if (dbDutyFunctionCode.size() > 1)
  {
    const char& function = dbDutyFunctionCode[1];
    if (LIKELY(agent->agentFunctions().empty() || function != agent->agentFunctions().at(0)))
      return false;
  }
  return true;
}

void
RuleUtil::getCat27TourCode(const PaxTypeFare* paxTypeFare, std::string& tourCode)
{
  if (LIKELY(paxTypeFare))
  {
    const PaxTypeFare* paxTypeFareForCat27 = paxTypeFare;

    if (paxTypeFare->isFareByRule())
    {
      const FBRPaxTypeFareRuleData* fbrPaxTypeFare =
          paxTypeFare->getFbrRuleData(RuleConst::FARE_BY_RULE);
      if (LIKELY(fbrPaxTypeFare))
      {
        const FareByRuleItemInfo* fbrItemInfo =
            dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());
        if (fbrItemInfo && !fbrPaxTypeFare->isSpecifiedFare() && (fbrItemInfo->ovrdcat27() == 'B'))
          paxTypeFareForCat27 = fbrPaxTypeFare->baseFare();
      }
    }

    const PaxTypeFareRuleData* ptfRuleData =
        paxTypeFareForCat27->paxTypeFareRuleData(RuleConst::TOURS_RULE);
    if (ptfRuleData)
    {
      const Tours* tours = dynamic_cast<const Tours*>(ptfRuleData->ruleItemInfo());
      if (tours)
        tourCode = tours->tourNo();
    }
  }
}

void RuleUtil::getAllPTFs(std::vector<const PaxTypeFare*>& ptfv, const FarePath& farePath)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      ptfv.push_back(fu->paxTypeFare());
    }
  }
}

void
RuleUtil::recreateRefundPlusUps(PricingTrx& trx, FarePath& fPath)
{
  if (LIKELY(trx.excTrxType() != PricingTrx::AF_EXC_TRX))
    return;

  RefundPricingTrx& refundTrx = static_cast<RefundPricingTrx&>(trx);

  if (!refundTrx.arePenaltiesAndFCsEqualToSumFromFareCalc() ||
      refundTrx.trxPhase() == RexBaseTrx::PRICE_NEWITIN_PHASE)
    return;

  refundTrx.exchangeOverrides().fill(fPath, false); // copy data without surcharges (cat12)
}

bool
RuleUtil::checkDutyFunctionCode(const Agent* agent, const AgencyDutyCode& dbDutyFunctionCode)
{
  if (agent->agentDuty().empty())
    return false;
  // The T183 DUTYFUNCTIONCODE field first character must match to AGI/N0G.
  // The second, if present, must be equal to the first character of AGI/A90
  const char& duty1 = dbDutyFunctionCode[0];
  const char& duty2 = agent->agentDuty().at(0);
  if (duty1 != duty2)
  {
    // wacky mapping between db and SABRE 0123456789ABCDE <=> 0123456789$@*-/
    if (duty1 >= '0' && duty1 <= '9')
      return false;
    if (!((duty1 == RuleConst::DB_SABRE_A && duty2 == RuleConst::DB_SABRE_DOLLAR) ||
          (duty1 == RuleConst::DB_SABRE_B && duty2 == RuleConst::DB_SABRE_AMP) ||
          (duty1 == RuleConst::DB_SABRE_C && duty2 == RuleConst::DB_SABRE_STAR) ||
          (duty1 == RuleConst::DB_SABRE_D && duty2 == RuleConst::DB_SABRE_DASH) ||
          (duty1 == RuleConst::DB_SABRE_E && duty2 == RuleConst::DB_SABRE_SLASH)))
      return false;
  }
  if (dbDutyFunctionCode.size() > 1)
  {
    const char& function = dbDutyFunctionCode[1];
    if (agent->agentFunctions().empty() || function != agent->agentFunctions().at(0))
      return false;
  }
  return true;
}

bool
RuleUtil::determineRuleChecks(const uint16_t& categorySequence,
                              const FareByRuleItemInfo& fbrItemInfo,
                              bool& checkFare,
                              bool& checkBaseFare)
{
  Indicator i;

  switch (categorySequence)
  {
  case 1:
    i = fbrItemInfo.ovrdcat1();
    break;
  case 2:
    i = fbrItemInfo.ovrdcat2();
    break;
  case 3:
    i = fbrItemInfo.ovrdcat3();
    break;
  case 4:
    i = fbrItemInfo.ovrdcat4();
    break;
  case 5:
    i = fbrItemInfo.ovrdcat5();
    break;
  case 6:
    i = fbrItemInfo.ovrdcat6();
    break;
  case 7:
    i = fbrItemInfo.ovrdcat7();
    break;
  case 8:
    i = fbrItemInfo.ovrdcat8();
    break;
  case 9:
    i = fbrItemInfo.ovrdcat9();
    break;

  case 10:
    i = fbrItemInfo.ovrdcat10();
    break;
  case 11:
    i = fbrItemInfo.ovrdcat11();
    break;
  case 12:
    i = fbrItemInfo.ovrdcat12();
    break;
  case 13:
    i = fbrItemInfo.ovrdcat13();
    break;
  case 14:
    i = fbrItemInfo.ovrdcat14();
    break;
  case 15:
    i = fbrItemInfo.ovrdcat15();
    break;
  case 16:
    i = fbrItemInfo.ovrdcat16();
    break;
  case 17:
    i = fbrItemInfo.ovrdcat17();
    break;
  case 18:
    i = fbrItemInfo.ovrdcat18();
    break;
  case 19:
    i = fbrItemInfo.ovrdcat19();
    break;
  case 20:
    i = fbrItemInfo.ovrdcat20();
    break;

  case 21:
    i = fbrItemInfo.ovrdcat21();
    break;
  case 22:
    i = fbrItemInfo.ovrdcat22();
    break;
  case 23:
    i = fbrItemInfo.ovrdcat23();
    break;
  case 24:
    i = fbrItemInfo.ovrdcat24();
    break;
  case 26:
    i = fbrItemInfo.ovrdcat26();
    break;
  case 27:
    i = fbrItemInfo.ovrdcat27();
    break;
  case 28:
    i = fbrItemInfo.ovrdcat28();
    break;
  case 29:
    i = fbrItemInfo.ovrdcat29();
    break;
  case 30:
    i = fbrItemInfo.ovrdcat30();
    break;
  case 31:
    i = fbrItemInfo.ovrdcat31();
    break;
  case 32:
    i = fbrItemInfo.ovrdcat32();
    break;
  case 33:
    i = fbrItemInfo.ovrdcat33();
    break;
  case 34:
    i = fbrItemInfo.ovrdcat34();
    break;

  case 35:
    i = fbrItemInfo.ovrdcat35();
    break;
  case 36:
    i = fbrItemInfo.ovrdcat36();
    break;
  case 37:
    i = fbrItemInfo.ovrdcat37();
    break;
  case 38:
    i = fbrItemInfo.ovrdcat38();
    break;
  case 39:
    i = fbrItemInfo.ovrdcat39();
    break;
  case 40:
    i = fbrItemInfo.ovrdcat40();
    break;
  case 41:
    i = fbrItemInfo.ovrdcat41();
    break;
  case 42:
    i = fbrItemInfo.ovrdcat42();
    break;
  case 43:
    i = fbrItemInfo.ovrdcat43();
    break;
  case 44:
    i = fbrItemInfo.ovrdcat44();
    break;
  case 45:
    i = fbrItemInfo.ovrdcat45();
    break;
  case 46:
    i = fbrItemInfo.ovrdcat46();
    break;
  case 47:
    i = fbrItemInfo.ovrdcat47();
    break;
  case 48:
    i = fbrItemInfo.ovrdcat48();
    break;
  case 49:
    i = fbrItemInfo.ovrdcat49();
    break;
  case 50:
    i = fbrItemInfo.ovrdcat50();
    break;
  default:
    i = 'X';
    break;
  }

  if (i == 'X')
  {
    checkFare = true;

    // check basefare for only footnote
    checkBaseFare = false;
  }
  else if (i == 'B')
  {
    checkFare = false;
    checkBaseFare = true;
  }
  else if (LIKELY(i == ' '))
  {
    checkFare = true;
    checkBaseFare = true;
  }
  else
  {
    checkFare = true;

    // check basefare for only footnote
    checkBaseFare = false;
  }

  return true;
}

bool
RuleUtil::matchAllTvlSegsFromTo(PricingTrx& trx,
                                const LocKey& locKeyFrom,
                                const LocKey& locKeyTo,
                                const VendorCode& vendorCode,
                                const std::vector<TravelSeg*>& allTvlSegs,
                                const bool& fltStopCheck,
                                std::set<TravelSeg*>& tvlSegsRtn,
                                GeoTravelType geoTvlType)
{
  const LocTypeCode& locTypeFrom = locKeyFrom.locType();
  const LocTypeCode& locTypeTo = locKeyTo.locType();

  if (locTypeFrom == LOCTYPE_NONE || locTypeTo == LOCTYPE_NONE)
    return false;

  const LocCode& locCodeFrom = locKeyFrom.loc();
  const LocCode& locCodeTo = locKeyTo.loc();

  std::vector<TravelSeg*>::const_iterator tvlSegI = allTvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = allTvlSegs.end();

  std::vector<TravelSeg*>::const_iterator fromTvlSegI = tvlSegI;
  std::vector<TravelSeg*>::const_iterator toTvlSegI = tvlSegI;

  std::vector<TravelSeg*>::const_iterator fromTvlSegIByHiddenStop = tvlSegI;

  bool bothGeoSame = (locTypeFrom == locTypeTo && locCodeFrom == locCodeTo);
  bool matchOrigin = false;
  bool matchBothGeo = false;
  bool isHiddenPointFound = false;

  for (; tvlSegI != tvlSegEndI; ++tvlSegI)
  {
    isHiddenPointFound = false;
    matchBothGeo = false;
    const TravelSeg& tvlSeg = **tvlSegI;
    const std::vector<const Loc*>::const_iterator hiddenStopIterBegin =
        tvlSeg.hiddenStops().begin();
    const std::vector<const Loc*>::const_iterator hiddenStopIterEnd = tvlSeg.hiddenStops().end();
    std::vector<const Loc*>::const_iterator hiddenStopIter = hiddenStopIterBegin;

    // there could be orig of multiple tvlSeg matches locCodeFrom,
    // to collect the matched one, we mark it as match and only after that check for destination
    // looking for Origin match
    if (LocUtil::isInLoc(*((*tvlSegI)->origin()),
                         locTypeFrom,
                         locCodeFrom,
                         vendorCode,
                         RESERVED,
                         LocUtil::FLIGHT_RULE,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      // test origin for match GEO2
      if (!bothGeoSame && LocUtil::isInLoc(*((*tvlSegI)->origin()),
                                           locTypeTo,
                                           locCodeTo,
                                           vendorCode,
                                           RESERVED,
                                           LocUtil::FLIGHT_RULE,
                                           geoTvlType,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT()))
      {
        matchBothGeo = true;
      }
      else
      {
        matchOrigin = true;
      }
      fromTvlSegI = tvlSegI;
    }
    else if (fltStopCheck && !tvlSeg.hiddenStops().empty())
    {
      for (; hiddenStopIter != hiddenStopIterEnd; ++hiddenStopIter)
      {
        if (LocUtil::isInLoc(**hiddenStopIter,
                             locTypeFrom,
                             locCodeFrom,
                             vendorCode,
                             RESERVED,
                             LocUtil::FLIGHT_RULE,
                             geoTvlType,
                             EMPTY_STRING(),
                             trx.getRequest()->ticketingDT()))
        {
          fromTvlSegI = tvlSegI;
          fromTvlSegIByHiddenStop = tvlSegI;
          matchOrigin = true;
          isHiddenPointFound = true;
          break;
        }
      }
    }

    if (LocUtil::isInLoc(*(tvlSeg.destination()),
                         locTypeTo,
                         locCodeTo,
                         vendorCode,
                         RESERVED,
                         LocUtil::FLIGHT_RULE,
                         geoTvlType,
                         EMPTY_STRING(),
                         trx.getRequest()->ticketingDT()))
    {
      // Check the destination to match for GEO1, if so then bump to next travel segment
      if (!bothGeoSame && LocUtil::isInLoc(*(tvlSeg.destination()),
                                           locTypeFrom,
                                           locCodeFrom,
                                           vendorCode,
                                           RESERVED,
                                           LocUtil::FLIGHT_RULE,
                                           geoTvlType,
                                           EMPTY_STRING(),
                                           trx.getRequest()->ticketingDT()) &&
          matchBothGeo)
      {
        continue;
      }
      else
      {
        toTvlSegI = tvlSegI;
        if (matchOrigin || matchBothGeo)
        {
          tvlSegsRtn.insert(fromTvlSegI, ++toTvlSegI);
          matchOrigin = false;
        }
      }
    }
    else if (fltStopCheck && !tvlSeg.hiddenStops().empty())
    {
      if (fromTvlSegIByHiddenStop == tvlSegI && fromTvlSegI == tvlSegI &&
          hiddenStopIter != hiddenStopIterEnd)
      {
        // matched From point on a hiddenStop, when checking same
        // TravelSeg, To point need to be next hiddenStop
        if (isHiddenPointFound)
          hiddenStopIter++;
      }
      else
      {
        // possible that now hiddenStopIter == hiddenStopIterEnd, when ]
        // we search *hiddenStopIter to match From point and got nothing
        // so reset the hiddenStopIter
        hiddenStopIter = hiddenStopIterBegin;
      }
      for (; hiddenStopIter != hiddenStopIterEnd; hiddenStopIter++)
      {
        if (LocUtil::isInLoc(**hiddenStopIter,
                             locTypeTo,
                             locCodeTo,
                             vendorCode,
                             RESERVED,
                             LocUtil::FLIGHT_RULE,
                             geoTvlType))
        {
          toTvlSegI = tvlSegI;
          if (matchOrigin)
          {
            tvlSegsRtn.insert(fromTvlSegI, ++toTvlSegI);
            matchOrigin = false;
            break;
          }
        }
      }
    }
  }

  return !tvlSegsRtn.empty();
}

bool
RuleUtil::isVendorPCC(const VendorCode& vendor, const PricingTrx& trx)
{
  if ((vendor == ATPCO_VENDOR_CODE) || (vendor == SITA_VENDOR_CODE) ||
      (vendor == SMF_ABACUS_CARRIER_VENDOR_CODE) || (vendor == SMF_CARRIER_VENDOR_CODE) ||
      (vendor == SMFO_VENDOR_CODE))
  {
    return false;
  }
  else
  {
    return (trx.dataHandle().getVendorType(vendor) == RuleConst::SMF_VENDOR);
  }
}

bool
RuleUtil::compareRoutings(const RoutingNumber& routingFromRule,
                          const RoutingNumber& routingFromFare)
{
  if (routingFromRule.empty())
    return true;

  std::string fromRule = FareDisplayResponseUtil::routingNumberToString(routingFromRule);
  std::string fromFare = FareDisplayResponseUtil::routingNumberToString(routingFromFare);

  return (fromRule.compare(fromFare) == 0);
}

bool
RuleUtil::validateMatchExpression(const FareClassCodeC& fareClassMatchRule)
{
  std::string fareClassRule = fareClassMatchRule;

  if ((fareClassRule == "-") ||
      (fareClassRule == "?") ||
      (fareClassRule.find("-?") != std::string::npos) ||
      (fareClassRule.find("?-") != std::string::npos) ||
      (fareClassRule.find("--") != std::string::npos))
  {
    return false;
  }

  return true;
}

bool
RuleUtil::matchFareClassExpression(const char* fareClassRule, const char* fareBasis)
{
  while(*fareBasis)
  {
    // If direct match, or single wildcard.
    if (*fareBasis == *fareClassRule || *fareClassRule =='?')
    {
      ++fareClassRule;
      ++fareBasis;
      continue;
    }

    // "-" matches one or more characters.
    if (*fareClassRule == '-')
    {
      ++fareClassRule;
      while(*fareBasis)
        if (matchFareClassExpression(fareClassRule, ++fareBasis))
          return true;
    }
    return false;
  }

  return (*fareClassRule == 0);
}

std::string
RuleUtil::getFareBasis(PricingTrx& trx, const PaxTypeFare* ptf)
{
  std::string fareBasis;

  const PaxTypeFare* baseFare = nullptr;

  if (ptf->isDiscounted())
  {
    try
    {
      baseFare = ptf->baseFare(19);
    }
    catch (...) {}
  }

  if (!baseFare)
    baseFare = ptf;

  if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
  {
    FareDisplayTrx* fdTrx = static_cast<FareDisplayTrx*>(&trx);
    fareBasis = baseFare->createFareBasisCodeFD(*fdTrx);
  }
  else
    fareBasis = baseFare->createFareBasis(trx);

  std::string::size_type posBasis = fareBasis.find("/");
  if (posBasis != std::string::npos)
    fareBasis.erase(posBasis);

  return fareBasis;
}

void
RuleUtil::setAdjustedTicketDateTime(PricingTrx& trx, DateTime& adjustedTicketDate)
{
  if (LIKELY(!trx.dataHandle().isHistorical()))
  {
    short utcOffset = 0;
    utcOffset = getTimeDiffAgentPccHDQ(trx);
    if (utcOffset)
      adjustedTicketDate = adjustedTicketDate.addSeconds(utcOffset * 60);
  }
}

short
RuleUtil::getTimeDiffAgentPccHDQ(PricingTrx& trx)
{
  short utcOffset = 0;
  DateTime tktDT = trx.dataHandle().ticketDate();
  DateTime curDT = DateTime::localTime();

  if (LIKELY(trx.getRequest()->ticketingAgent()))
  {
    const Loc* hdqLoc = trx.dataHandle().getLoc(RuleConst::HDQ_CITY, tktDT);
    const Loc* salesLoc = trx.getRequest()->salePointOverride().empty()
                            ? trx.getRequest()->ticketingAgent()->agentLocation()
                            : trx.dataHandle().getLoc(trx.getRequest()->salePointOverride(), tktDT);
    if (LIKELY(hdqLoc && salesLoc))
    {
      if (LIKELY(LocUtil::getUtcOffsetDifference(*hdqLoc, *salesLoc, utcOffset, trx.dataHandle(), curDT, curDT)))
        return utcOffset;
    }
  }
  return 0;
}

MoneyAmount
RuleUtil::convertCurrency(PricingTrx& trx,
                          const FarePath& fp,
                          MoneyAmount sourceAmount,
                          const CurrencyCode& sourceCurrency,
                          const CurrencyCode& targetCurrency,
                          bool doNonIataRounding)
{
  Money sourceMoney(sourceAmount, sourceCurrency);
  Money targetMoney(targetCurrency);
  MoneyAmount targetAmount = 0;

  CurrencyConversionFacade ccFacade;
  ccFacade.setRoundFare(true);

  if (doNonIataRounding)
    targetMoney.setApplyNonIATARounding();

  if (ccFacade.convert(targetMoney, sourceMoney, trx, fp.itin()->useInternationalRounding()))
  {
    targetAmount = targetMoney.value();
  }
  return targetAmount;
}

void
RuleUtil::getPrimeRBDrec1(const PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes)
{
  const FareClassAppSegInfo* fcasi = paxTypeFare.fareClassAppSegInfo();
  if (fcasi == nullptr)
    return;
  for (uint16_t i = 0; i < 8; ++i)
  {
    if (fcasi->_bookingCode[i].empty())
      break;
    bkgCodes.push_back(fcasi->_bookingCode[i]);
  }
}

void
RuleUtil::getPrimeRBD(const PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes)
{
  getPrimeRBDrec1(paxTypeFare, bkgCodes);
  if (bkgCodes.empty())
  {
    const FBRPaxTypeFareRuleData* fbrPTFare = nullptr;
    if (paxTypeFare.isFareByRule())
      fbrPTFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTFare && !fbrPTFare->isSpecifiedFare())
      fbrPTFare->getBaseFarePrimeBookingCode(bkgCodes);
  }
  return;
}

bool
RuleUtil::matchPrimeRBD(const FareFocusBookingCodeInfo* ffbci,
                        const std::vector<BookingCode>& bkgCodes)
{
  for (BookingCode bc : bkgCodes)
  {
    for (BookingCode bcRule : ffbci->bookingCode())
    {
      if (bc == bcRule)
        return true;
    }
  }
  return false;
}

bool
RuleUtil::isInLoc(PricingTrx& trx,
                  const LocCode& validatingLoc,
                  LocTypeCode ruleLocType,
                  const LocCode& ruleLoc,
                  const PaxTypeFare& paxTypeFare)
{
  GeoTravelType geoType = GeoTravelType::International;
  if (paxTypeFare.fareMarket() != nullptr)
    geoType = paxTypeFare.fareMarket()->geoTravelType();

  if (ruleLocType == GROUP_LOCATION)
    return isInFFUserGroup(trx, validatingLoc, ruleLocType, ruleLoc, paxTypeFare, geoType);

  return LocUtil::isInLoc(validatingLoc,
                          ruleLocType,
                          ruleLoc,
                          ATPCO_VENDOR_CODE,
                          RESERVED,
                          geoType,
                          LocUtil::OTHER,
                          trx.ticketingDate());
}

bool
RuleUtil::isInFFUserGroup(PricingTrx& trx,
                          const LocCode& validatingLoc,
                          LocTypeCode ruleLocType,
                          const LocCode& ruleLoc,
                          const PaxTypeFare& paxTypeFare,
                          GeoTravelType geoType)
{
  const DateTime ticketDate = trx.dataHandle().ticketDate();
  const ZoneInfo* zoneInfo = trx.dataHandle().getZoneFareFocusGroup(SABRE_USER, ruleLoc, USER_DEFINED, ticketDate, true);
  if (zoneInfo == nullptr)
    return false;
  const Loc* loc = trx.dataHandle().getLoc(validatingLoc, ticketDate);
  if (LocUtil::matchZone(
          trx.dataHandle(), *loc, zoneInfo, LocUtil::OTHER, geoType, EMPTY_STRING()) > 0)
    return true;
  return false;
}

std::vector<TravelSeg*>
RuleUtil::getFilteredSegsInOrder(const std::vector<TravelSeg*>& allTvlSegs,
                                 const std::set<TravelSeg*>& filteredSegSet)
{
  std::vector<TravelSeg*> tvlSegsRtn;
  tvlSegsRtn.reserve(filteredSegSet.size());
  for (auto seg : allTvlSegs)
    if (filteredSegSet.count(seg))
      tvlSegsRtn.push_back(seg);
  return tvlSegsRtn;
}

bool
RuleUtil::validateTvlDateRange(const DateTime& refDT,
                               const FareFocusDaytimeApplDetailInfo* ffdtaInfo)
{
  const DateTime& earlistDT = ffdtaInfo->startDate();
  const DateTime& latestDT = ffdtaInfo->stopDate();

  if (earlistDT.isValid() && refDT < earlistDT)
    return false;

  if (latestDT.isValid() && refDT.date() > latestDT.date())
    return false;

  return true;
}

bool
RuleUtil::matchTravelRangeX5(PricingTrx& trx,
                             const PaxTypeFare& ptf,
                             DateTime adjustedTicketDate,
                             const FareFocusDaytimeApplInfo* ffdtInfo)
{
  const DateTime& travelDate = ptf.fareMarket()->travelDate();
  if (travelDate.isValid())
  {
    if (ffdtInfo == nullptr)
      return false;

    for (auto dtls: ffdtInfo->details())
    {
   //  Validate  Range Stop Date and Start Date
   //
      if (validateTvlDateRange(travelDate, dtls))
      {
        return true;
      }
    }
  }
  return false;
}

bool
RuleUtil::matchCat35Type(const Indicator displayTypeRule, const Indicator displayTypeFare)
{
  if (displayTypeRule == ' ')
    return true;
  if (displayTypeRule == 'L' || displayTypeRule == 'T' || displayTypeRule == 'Q')
  {
    if (displayTypeRule == displayTypeFare)
      return true;
    if (displayTypeRule == 'Q' && (displayTypeFare == 'L' || displayTypeFare == 'T'))
      return true;
  }

  return false;
}

bool
RuleUtil::matchBookingCode(PricingTrx& trx,
                           uint64_t bookingCdItemNo,
                           const PaxTypeFare& paxTypeFare,
                           DateTime adjustedTicketDate)
{
  if (bookingCdItemNo == 0)
    return true;

  const std::vector<class FareFocusBookingCodeInfo*>& ffbciV =
    trx.dataHandle().getFareFocusBookingCode(bookingCdItemNo, adjustedTicketDate);

  if (ffbciV.empty())
    return false;

  std::vector<FareFocusBookingCodeInfo*>::const_iterator ffbci = ffbciV.begin();
  if (*ffbci == nullptr)
    return false;

  std::vector<BookingCode> bkgCodes;
  getPrimeRBD(paxTypeFare, bkgCodes);
  if (bkgCodes.empty())
    return false;

  return matchPrimeRBD(*ffbci, bkgCodes);
}

bool
RuleUtil::isFrrNetCustomer(PricingTrx& trx)
{
  if (!trx.getRequest() ||
      !trx.getRequest()->ticketingAgent() ||
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    return false;

  const DateTime localTimeDT = DateTime::localTime();
  const std::vector<CustomerSecurityHandshakeInfo*>& customerSH1 =
    trx.dataHandle().getCustomerSecurityHandshake(
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC(), "RN", localTimeDT);
  if (!customerSH1.empty())
    return true;

  if (trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC() !=
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC())
  {
    const std::vector<CustomerSecurityHandshakeInfo*>& customerSH2 =
      trx.dataHandle().getCustomerSecurityHandshake(
        trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC(),"RN", localTimeDT);
    if (!customerSH2.empty())
      return true;
  }

  return false;
}

bool
RuleUtil::isCat35FareUsingFRCode(const PaxTypeFare& paxTypeFare, PseudoCityCode& sourcePCC)
{
  if (!paxTypeFare.hasCat35Filed())
    return false;

  const NegPaxTypeFareRuleData* negRD = paxTypeFare.getNegRuleData();
  if (negRD && !negRD->sourcePseudoCity().empty() && !negRD->fareRetailerCode().empty())
  {
    sourcePCC = negRD->sourcePseudoCity();
    return true;
  }

  return false;
}

bool
RuleUtil::isAslFareUsingFRCode(const PaxTypeFare& paxTypeFare)
{
  const AdjustedSellingCalcData* adjSellingCalcData = paxTypeFare.getAdjustedSellingCalcData();
  if (adjSellingCalcData != nullptr)
  {
    if ((adjSellingCalcData->getFareRetailerRuleInfo() != nullptr) &&
        !adjSellingCalcData->getSourcePcc().empty() &&
        !adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode().empty())
      return true;
  }

  return false;
}

bool
RuleUtil::isFareValidForRetailerCodeQualiferMatch(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  if (trx.getRequest()->prmValue() == false)
    return true;

  PseudoCityCode sourcePCC;
  if (isCat35FareUsingFRCode(paxTypeFare, sourcePCC))
    return true;

  return isAslFareUsingFRCode(paxTypeFare);
}

bool
RuleUtil::isPricingUnitValidForRetailerCode(PricingUnit& pu)
{
  for (const FareUsage* fu : pu.fareUsage())
  {
    PseudoCityCode ptfSourcePCC;
    if (RuleUtil::isCat35FareUsingFRCode(*fu->paxTypeFare(), ptfSourcePCC))
    {
      if (pu.frrCodeSourcePCC().empty())
        pu.frrCodeSourcePCC() = ptfSourcePCC; // first PTF with FR code
      else if (pu.frrCodeSourcePCC() != ptfSourcePCC)
      {
        pu.frrCodeSourcePCC().clear();
        return false;
      }
    }
  }

  return true;
}

bool
RuleUtil::isFarePathValidForRetailerCode(PricingTrx& trx, const FarePath& fPath)
{
  if (fallback::fallbackFRRProcessingRetailerCode(&trx))
    return true; // remove trx argument when removing the fallback

  if (fPath.pricingUnit().size() == 1)
    return true;

  std::set<PseudoCityCode> sourcePccs;
  for (const PricingUnit* pu : fPath.pricingUnit())
  {
    if (!pu->frrCodeSourcePCC().empty())
      sourcePccs.insert(pu->frrCodeSourcePCC());

    if (sourcePccs.size() == 2) // No different source pccs allowed
      return false;
  }

  return true;
}

} // tse
