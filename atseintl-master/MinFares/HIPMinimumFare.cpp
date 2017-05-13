//----------------------------------------------------------------------------
//
//  File:           HIPMinimumFare.cpp
//  Created:        3/4/2004
//  Authors:
//
//  Description:    A class to represent data and methods for HIP
//
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "MinFares/HIPMinimumFare.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/NegFareRest.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "MinFares/BHCMinimumFare.h"
#include "MinFares/MinFareFareSelection.h"
#include "MinFares/MinFareLogic.h"
#include "MinFares/MinFareNormalFareSelection.h"
#include "MinFares/MinFareSpecialFareSelection.h"
#include "Rules/RuleConst.h"
#include "Util/BranchPrediction.h"

#include <iomanip>
#include <iostream>

namespace tse
{
FALLBACK_DECL(fallbackHIPMinimumFareCat25FareExempt);
FALLBACK_DECL(fallbackCreateHIPDiagAlways)

static Logger
logger("atseintl.MinFares.HIPMinimumFare");

namespace
{
inline DiagCollector*
getDiag718(PricingTrx& trx)
{
  return (trx.diagnostic().diagnosticType() == Diagnostic718 ||
          fallback::fallbackCreateHIPDiagAlways(&trx))
             ? DCFactory::instance()->create(trx)
             : nullptr;
}
}

HIPMinimumFare::HIPMinimumFare(PricingTrx& trx, HipProcess hipProcess)
  : MinimumFare(), _trx(trx), _diag(getDiag718(_trx))
{
  _hipProcess = hipProcess;

  if (UNLIKELY(_diag))
    _diag->enable(Diagnostic718);
}

HIPMinimumFare::~HIPMinimumFare()
{
  if (UNLIKELY(_diag))
    _diag->flushMsg();
}

MoneyAmount
HIPMinimumFare::process(const FarePath& farePath,
                        std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                        std::multimap<uint16_t, const MinFareAppl*>& applMap,
                        std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
{
  MoneyAmount totalPlusup = 0;

  const Itin* itin = farePath.itin();

  if ((itin != nullptr) && (itin->geoTravelType() == GeoTravelType::International))
  {
    _passLit = true;
    _throw = true;

    if (_diag)
    {
      *_diag << "       HIGHER INTERMEDIATE POINT CHECK - " << farePath.paxType()->paxType()
             << (_hipProcess == MinimumFare::NORMAL_HIP_PROCESSING ? " - NL\n" : "\n");
      *_diag << "CITY      GOV    CLASS                       DIR FARE  GLOB EXCL\n"
             << "PAIR      CXR    CUR          AMOUNT RTG TAG I/O TYPE  IND  IND \n";
    }

    const std::vector<PricingUnit*>& pricingUnits = farePath.pricingUnit();
    std::vector<PricingUnit*>::const_iterator puI;
    for (puI = pricingUnits.begin(); puI != pricingUnits.end() && _passLit; puI++)
    {
      if ((*puI)->isSideTripPU())
        continue;
      totalPlusup += process(**puI, farePath, ruleLevelMap, applMap, defaultLogicMap);
    }
  }

  return totalPlusup; // Include HIP and BHC
}

MoneyAmount
HIPMinimumFare::process(const PricingUnit& pu,
                        const FarePath& farePath,
                        std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                        std::multimap<uint16_t, const MinFareAppl*>& applMap,
                        std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
{
  MoneyAmount totalPlusup = 0;

  if (pu.geoTravelType() == GeoTravelType::International)
  {
    const std::vector<FareUsage*>& fuVec = pu.fareUsage();
    std::vector<FareUsage*>::const_iterator fareUsageI;
    MoneyAmount fmPlusUp = 0;

    for (fareUsageI = fuVec.begin(); fareUsageI != fuVec.end() && _passLit; fareUsageI++)
    {
      fmPlusUp =
          process(**fareUsageI, pu, farePath, ruleLevelMap, applMap, defaultLogicMap) + _bhcPlusUp;

      // Save the HIP and BHC plus up in fare usage. BHC needed for tax !!!
      (*fareUsageI)->accumulateMinFarePlusUpAmt(fmPlusUp);

      totalPlusup += fmPlusUp;
    }
  }

  // Check side trip PUs
  std::vector<PricingUnit*>::const_iterator stPuI = pu.sideTripPUs().begin();
  for (; stPuI != pu.sideTripPUs().end(); ++stPuI)
  {
    totalPlusup += process(**stPuI, farePath, ruleLevelMap, applMap, defaultLogicMap);
  }

  return totalPlusup; // Include HIP and BHC
}

MoneyAmount
HIPMinimumFare::process(FareUsage& fu, const PricingUnit& pu, const FarePath& farePath)
{
  std::multimap<uint16_t, const MinFareRuleLevelExcl*> ruleLevelMap;
  std::multimap<uint16_t, const MinFareAppl*> applMap;
  std::map<uint16_t, const MinFareDefaultLogic*> defaultLogicMap;

  if (LIKELY(!_throw))
  {
    if (UNLIKELY(_diag))
    {
      *_diag << "       HIGHER INTERMEDIATE POINT CHECK - " << farePath.paxType()->paxType()
             << (_hipProcess == MinimumFare::NORMAL_HIP_PROCESSING ? " - NL\n   " : "\n")
             << "        CALLED DIRECT FOR A FARE COMPONENT\n";
      *_diag << "CITY      GOV    CLASS                       DIR FARE  GLOB EXCL\n"
             << "PAIR      CXR    CUR          AMOUNT RTG TAG I/O TYPE  IND  IND \n";
    }
  }

  return process(fu, pu, farePath, ruleLevelMap, applMap, defaultLogicMap); // Include ONLY HIP
}

MoneyAmount
HIPMinimumFare::process(FareUsage& fu,
                        const PricingUnit& pu,
                        const FarePath& farePath,
                        std::multimap<uint16_t, const MinFareRuleLevelExcl*>& ruleLevelMap,
                        std::multimap<uint16_t, const MinFareAppl*>& applMap,
                        std::map<uint16_t, const MinFareDefaultLogic*>& defaultLogicMap)
{
  if (UNLIKELY(fu.exemptMinFare()))
    return 0.0;

  if (UNLIKELY(fu.paxTypeFare()->isDummyFare()))
    return 0;

  _bhcPlusUp = 0; // Reset BHC plus up

  const PaxTypeFare& paxTypeFare = *fu.paxTypeFare(); // lint !e530

  // Print thru fare info
  if (UNLIKELY(fu.adjustedPaxTypeFare()))
  {
    MoneyAmount ngAdjustment =
        fu.adjustedPaxTypeFare()->nucFareAmount() - paxTypeFare.nucFareAmount();

    if (IS_DEBUG_ENABLED(logger))
    {
      if (UNLIKELY(_diag))
      {
        printFareInfo(paxTypeFare, *_diag, HIP, "N ");
        printFareInfo(*fu.adjustedPaxTypeFare(), *_diag, HIP, "R ");
      }
    }

    if (UNLIKELY(_diag))
      printFareInfo(paxTypeFare, *_diag, HIP, "* ", false, ngAdjustment);
  }
  else
  {
    if (UNLIKELY(_diag))
      printFareInfo(paxTypeFare, *_diag, HIP, "* ");
  }

  if (paxTypeFare.fareMarket()->geoTravelType() != GeoTravelType::International)
  {
    if (UNLIKELY(_diag))
      *_diag << "  NO HIP - EXEMPTED BY NON-INTERNATIONAL TRAVEL \n";
    return 0;
  }

  if (fu.paxTypeFare()->isNegotiated() && !fu.paxTypeFare()->isFareByRule())
  {
    if (UNLIKELY(_diag))
      *_diag << "  NO HIP - EXEMPTED BY PUBLIC CAT-35 \n";

    return 0;
  }

  const FareMarket& fareMarket = *(paxTypeFare.fareMarket());
  const std::vector<TravelSeg*>& travelSegVec = fareMarket.travelSeg();
  MoneyAmount totalHipPlusup = 0;

  // Note: if HIP is called again for the same FU but for Normal-HIP,
  // we want to reprocess it.
  if (_hipProcess != NORMAL_HIP_PROCESSING && fu.isHipProcessed())
  {
    if (UNLIKELY(_diag))
    {
      *_diag << "  HIP HAS BEEN PROCESSED \n";
      printPlusUpInfo(paxTypeFare, fu.minFarePlusUp(), *_diag);
    }

    MoneyAmount hipAmount = fu.minFarePlusUp().getSum(HIP);
    if (LIKELY(_throw)) // Called for a FarePath
    {
      _bhcPlusUp = fu.minFarePlusUp().getSum(BHC);

      if (hipAmount > 0)
      {
        LimitationOnIndirectTravel lit(_trx, *farePath.itin());
        if (!lit.validateFareComponentAfterHip(fu)) // Failed Limitations
          _passLit = false;
      }
    }

    return hipAmount;
  }

  if (travelSegVec.size() <= 1)
  {
    return totalHipPlusup;
  }

  if (paxTypeFare.isFareByRule() && cat25FareExempt(paxTypeFare))
  {
    if (UNLIKELY(_diag))
      *_diag << "  CAT-25 FARE EXEMPT \n";

    return totalHipPlusup;
  }

  if (paxTypeFare.isPsrHipExempt())
  {
    if (UNLIKELY(_diag))
      *_diag << "  PERMISSIBLE SPECIFIED ROUTING EXEMPT \n";

    return totalHipPlusup;
  }

  const Itin& itin = *farePath.itin();

  _travelDate = itin.travelDate();

  // here rex adjustment
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    adjustRexPricingDates(_trx, fu.paxTypeFare()->retrievalDate());

  else if (UNLIKELY(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
    adjustPortExcDates(_trx);

  //--- Check Rule Level Exclusion Table
  _matchedRuleItem =
      MinFareLogic::getRuleLevelExcl(_trx, itin, paxTypeFare, HIP, ruleLevelMap, _travelDate);

  if (_matchedRuleItem && (_matchedRuleItem->hipMinFareAppl() == YES))
  {
    if (UNLIKELY(_diag))
    {
      printExceptionInfo(paxTypeFare, *_diag);
      *_diag << "EXEMPT BY RULE LEVEL EXCL TABLE - " << _matchedRuleItem->seqNo() << '\n';
    }

    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      restorePricingDates(_trx, _trx.ticketingDate());

    return totalHipPlusup;
  }

  //--- Check Application Table
  _matchedApplItem = MinFareLogic::getApplication(
      _trx, farePath, itin, paxTypeFare, HIP, applMap, defaultLogicMap, _travelDate);
  if (isExemptedByApplication(paxTypeFare))
  {
    if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
      restorePricingDates(_trx, _trx.ticketingDate());

    return 0;
  }

  _matchedDefaultItem = nullptr;
  if (_matchedApplItem->applyDefaultLogic() == YES)
  {
    _matchedDefaultItem =
        MinFareLogic::getDefaultLogic(HIP, _trx, itin, paxTypeFare, defaultLogicMap);
    if (UNLIKELY(_matchedDefaultItem == nullptr))
    {
      if (UNLIKELY(_diag))
      {
        printExceptionInfo(paxTypeFare, *_diag);
        *_diag << "FAILED MATCHING DEFAULT LOGIC TABLE - RETURN" << '\n';
      }

      if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        restorePricingDates(_trx, _trx.ticketingDate());

      return 0; // temp until data completed
      //    if (_throw)
      //      throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA);
      //    else
      //      return -1;
    }
  }
  if (UNLIKELY(skipProcessForSpecialOnly()))
  {
    return 0;
  }

  MinFareFareSelection::FareDirectionChoice outbInb = paxTypeFare.directionality() == FROM
                                                          ? MinFareFareSelection::OUTBOUND
                                                          : MinFareFareSelection::INBOUND;

  // Check PU trip type
  MinFareFareSelection::EligibleFare eligibleFare = (pu.puType() == PricingUnit::Type::ONEWAY)
                                                        ? MinFareFareSelection::ONE_WAY
                                                        : MinFareFareSelection::HALF_ROUND_TRIP;

  MinFarePlusUpItem curPlusUp;

  const PaxType& requestedPaxType = *farePath.paxType();
  const PaxTypeCode actualPaxType = checkDoubleDiscount(paxTypeFare);

  bool selectNormalFare = paxTypeFare.isNormal();
  if (UNLIKELY(_hipProcess == MinimumFare::NORMAL_HIP_PROCESSING))
  {
    selectNormalFare = true;
  }

  bool reverseFcOrigin = (fu.isInbound() || (paxTypeFare.directionality() == TO));

  processIntermediate(HIP,
                      eligibleFare,
                      _trx,
                      itin,
                      paxTypeFare,
                      requestedPaxType,
                      _diag,
                      selectNormalFare,
                      outbInb,
                      _normalExemptSet,
                      curPlusUp,
                      &farePath,
                      &pu,
                      &fu,
                      actualPaxType,
                      reverseFcOrigin);

  if ((curPlusUp.plusUpAmount > 0) && (_hipProcess != MinimumFare::NORMAL_HIP_PROCESSING))
  {
    MinFarePlusUpItem* finalPlusUp = nullptr;
    _trx.dataHandle().get(finalPlusUp);
    if (!finalPlusUp)
    {
      LOG4CXX_ERROR(logger, "DataHandle failed to allocate memory (finalPlusUp)");

      if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
        restorePricingDates(_trx, _trx.ticketingDate());

      return 0.0;
    }

    // lint --e{413}
    finalPlusUp->baseAmount = curPlusUp.baseAmount;
    finalPlusUp->plusUpAmount = curPlusUp.plusUpAmount;
    finalPlusUp->boardPoint = curPlusUp.boardPoint;
    finalPlusUp->offPoint = curPlusUp.offPoint;
    finalPlusUp->constructPoint = curPlusUp.constructPoint;

    fu.minFarePlusUp().addItem(HIP, finalPlusUp);
  }

  totalHipPlusup = curPlusUp.plusUpAmount;

  if (LIKELY(_hipProcess != MinimumFare::NORMAL_HIP_PROCESSING))
  {
    if (UNLIKELY(_diag))
      printPlusUpInfo(paxTypeFare, fu.minFarePlusUp(), *_diag);
  }
  else
  {
    MinFarePlusUp minFarePlusUp;
    minFarePlusUp.addItem(HIP, &curPlusUp);
    if (UNLIKELY(_diag))
      printPlusUpInfo(paxTypeFare, minFarePlusUp, *_diag);
  }

  // Skip BHC check when processing NL HIP for CTM
  if (UNLIKELY(_hipProcess == MinimumFare::NORMAL_HIP_PROCESSING))
  {
    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      restorePricingDates(_trx, _trx.ticketingDate());

    return totalHipPlusup;
  }

  fu.setHipProcessed();

  if (_throw) // Called for fare path
  {
    if (curPlusUp.plusUpAmount > 0)
    {
      // Check if Limitation allows HIP

      LimitationOnIndirectTravel lit(_trx, *farePath.itin());
      if (!lit.validateFareComponentAfterHip(fu)) // Failed Limitations
        _passLit = false;
    }

    if (pu.puType() == PricingUnit::Type::ONEWAY && // Only do BHC for OW PU and oneway thru fare.
        !(paxTypeFare.fcaOwrt() == ROUND_TRIP_MAYNOT_BE_HALVED) &&
        !(paxTypeFare.isFareByRule()))
    {
      BHCMinimumFare _bhc(_normalExemptSet);
      _bhc._matchedRuleItem = _matchedRuleItem;
      _bhc._matchedApplItem = _matchedApplItem;
      _bhc._matchedDefaultItem = _matchedDefaultItem;
      _bhcPlusUp = _bhc.process(_trx, farePath, pu, fu, _diag);
    }
  }
  if (UNLIKELY(_trx.excTrxType() == PricingTrx::AR_EXC_TRX))
    restorePricingDates(_trx, _trx.ticketingDate());

  return totalHipPlusup; // Include only HIP
}

PaxTypeCode
HIPMinimumFare::checkDoubleDiscount(const PaxTypeFare& paxTypeFare)
{
  PaxTypeCode actualPaxTypeCode = "";
  if (!(paxTypeFare.isFareByRule() && paxTypeFare.isDiscounted()))
    return actualPaxTypeCode;

  //--- Get record 8 primary passenger type
  const FBRPaxTypeFareRuleData* fbrPTFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

  if (fbrPTFare->fbrApp()->segCnt() != 0)
    actualPaxTypeCode = fbrPTFare->fbrApp()->primePaxType();

  return actualPaxTypeCode;
}

bool
HIPMinimumFare::cat25FareExempt(const PaxTypeFare& paxTypeFare)
{
  const FBRPaxTypeFareRuleData* fbrPaxTypeFareRuleData =
      paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

  if (!fbrPaxTypeFareRuleData)
    return false;

  if (UNLIKELY(fbrPaxTypeFareRuleData->isSpecifiedFare()))
    return true;

  // Cat 25 fare with F/IND H or L and was created from the specified amount
  if (fbrPaxTypeFareRuleData->isMinMaxFare())
    return true;

  const FareByRuleItemInfo* fbrItemInfo =
      dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFareRuleData->ruleItemInfo());

  if (fbrItemInfo == nullptr)
    return false;

  if (fbrItemInfo->ovrdcat17() == 'X')
    return true;

  const PaxTypeFare* fbrBaseFare = fbrPaxTypeFareRuleData->baseFare();
  if (fbrBaseFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) // value 1 is Private fare
    return true;

  if (paxTypeFare.isNegotiated() &&
      (paxTypeFare.fcaDisplayCatType() != RuleConst::SELLING_FARE)) // 'L'
  {
    if(!fallback::fallbackHIPMinimumFareCat25FareExempt(&_trx))
      return true;
    const PaxTypeFareRuleData* negFareRuleData =
    paxTypeFare.paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE);
    if (negFareRuleData != nullptr)
    {
      const NegFareRest* negFareRest =
      dynamic_cast<const NegFareRest*>(negFareRuleData->ruleItemInfo());
      if (negFareRest && negFareRest->negFareCalcTblItemNo() > 0) // cat35 table 979
        return true;
    }
  }
  return false;
}

bool
HIPMinimumFare::compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                   const PaxTypeFare& intermediatePaxTypeFare,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  MoneyAmount intermediateAmount = getIntermediateFareAmount(
      paxTypeFare, intermediatePaxTypeFare.nucFareAmount(), useInternationalRounding);
  MoneyAmount thruFareAmount = paxTypeFare.nucFareAmount();

  if (intermediateAmount <= thruFareAmount)
  {
    return false;
  }

  // Calculate mileage surchage for mileage fare based on thru fare:
  if (!paxTypeFare.isRouting() && paxTypeFare.mileageSurchargePctg() > 0)
  {
    intermediateAmount +=
        intermediateAmount * (paxTypeFare.mileageSurchargePctg() / RuleConst::HUNDRED_PERCENTS);
    CurrencyUtil::truncateNUCAmount(intermediateAmount);

    thruFareAmount +=
        thruFareAmount * (paxTypeFare.mileageSurchargePctg() / RuleConst::HUNDRED_PERCENTS);
    CurrencyUtil::truncateNUCAmount(thruFareAmount);
  }

  if (intermediateAmount <= curPlusUp.baseAmount) // It may less than previous intermediate fare
    return true;

  //--- update higher intermediate fare amount
  const FareMarket* fareMarket = intermediatePaxTypeFare.fareMarket();
  curPlusUp.plusUpAmount = intermediateAmount - thruFareAmount;
  curPlusUp.baseAmount = intermediateAmount;
  curPlusUp.boardPoint = outbound ? fareMarket->boardMultiCity() : fareMarket->offMultiCity();
  curPlusUp.offPoint = outbound ? fareMarket->offMultiCity() : fareMarket->boardMultiCity();
  curPlusUp.constructPoint = "";

  return true;
}

bool
HIPMinimumFare::compareAndSaveFare(const PaxTypeFare& paxTypeFare,
                                   const PtfPair& ptfPair,
                                   MinFarePlusUpItem& curPlusUp,
                                   bool useInternationalRounding,
                                   bool outbound)
{
  MoneyAmount unPubAmount = ptfPair.first->nucFareAmount() + ptfPair.second->nucFareAmount();

  const LocCode& boardPoint = ptfPair.first->fareMarket()->boardMultiCity();
  const LocCode& constPoint = ptfPair.first->fareMarket()->offMultiCity();
  const LocCode& offPoint = ptfPair.second->fareMarket()->offMultiCity();

  MoneyAmount intermediateAmount =
      getIntermediateFareAmount(paxTypeFare, unPubAmount, useInternationalRounding);
  MoneyAmount thruFareAmount = paxTypeFare.nucFareAmount();

  if (intermediateAmount <= thruFareAmount)
  {
    return false;
  }

  // Calculate mileage surchage for mileage fare based on thru fare:
  if (!paxTypeFare.isRouting() && paxTypeFare.mileageSurchargePctg() > 0)
  {
    intermediateAmount +=
        intermediateAmount * (paxTypeFare.mileageSurchargePctg() / RuleConst::HUNDRED_PERCENTS);
    CurrencyUtil::truncateNUCAmount(intermediateAmount);

    thruFareAmount +=
        thruFareAmount * (paxTypeFare.mileageSurchargePctg() / RuleConst::HUNDRED_PERCENTS);
    CurrencyUtil::truncateNUCAmount(thruFareAmount);
  }

  if (intermediateAmount <= curPlusUp.baseAmount) // It may less than previous intermediate fare
    return true;

  curPlusUp.plusUpAmount = intermediateAmount - thruFareAmount;
  curPlusUp.baseAmount = intermediateAmount;
  curPlusUp.boardPoint = outbound ? boardPoint : offPoint;
  curPlusUp.offPoint = outbound ? offPoint : boardPoint;
  curPlusUp.constructPoint = constPoint;

  curPlusUp.currency = ptfPair.first->currency();

  return true;
}

MoneyAmount
HIPMinimumFare::getIntermediateFareAmount(const PaxTypeFare& paxTypeFare,
                                          MoneyAmount originalIntermediateAmt,
                                          bool useInternationalRounding)
{
  if (!(paxTypeFare.isFareByRule()))
    return originalIntermediateAmt;

  const PaxTypeFareRuleData* paxTypeFareRuleData =
      paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);

  if (paxTypeFareRuleData == nullptr)
    return originalIntermediateAmt;

  const FareByRuleItemInfo* fbrItemInfo =
      dynamic_cast<const FareByRuleItemInfo*>(paxTypeFareRuleData->ruleItemInfo());

  if (fbrItemInfo == nullptr)
    return originalIntermediateAmt;

  MoneyAmount fbrAmt = originalIntermediateAmt;

  Indicator i = fbrItemInfo->fareInd();
  if (i == RuleConst::CALCULATED || i == RuleConst::CREATE_RT_FROM_OW ||
      i == RuleConst::SELECT_HIGHEST || i == RuleConst::SELECT_LOWEST)
    fbrAmt = originalIntermediateAmt * (fbrItemInfo->percent() / RuleConst::HUNDRED_PERCENTS);
  else if (i == RuleConst::ADD_SPECIFIED_TO_CALCULATED)
    fbrAmt = (originalIntermediateAmt * (fbrItemInfo->percent() / RuleConst::HUNDRED_PERCENTS)) +
             convertCat25Currency(paxTypeFare, *fbrItemInfo, useInternationalRounding);
  else if (i == RuleConst::SUBTRACT_SPECIFIED_FROM_CALCULATED)
    fbrAmt = (originalIntermediateAmt * (fbrItemInfo->percent() / RuleConst::HUNDRED_PERCENTS)) -
             convertCat25Currency(paxTypeFare, *fbrItemInfo, useInternationalRounding);

  //--- Apply discount from cat 19-22 if any
  if (!(paxTypeFare.isDiscounted()))
  {
    if (UNLIKELY(_diag))
      *_diag << "  FARE BY RULE CALCULATION AMOUNT  " << fbrAmt << '\n';

    return fbrAmt;
  }

  //  const DiscountedPaxTypeFare* discPaxTypeFare = dynamic_cast<const DiscountedPaxTypeFare*>
  //                                                                   (&paxTypeFare);
  //  if (discPaxTypeFare == 0 || &discPaxTypeFare->discountInfo() == 0)
  //    return fbrAmt;

  MoneyAmount finalAmt = fbrAmt;

  try
  {
    const DiscountInfo& discountInfo(paxTypeFare.discountInfo());
    MoneyAmount percentAmt = fbrAmt * (discountInfo.discPercent() / RuleConst::HUNDRED_PERCENTS);

    Indicator ind = discountInfo.farecalcInd();
    if (ind == RuleConst::CALCULATED)
      finalAmt = percentAmt;
    else if (ind == RuleConst::ADD_SPECIFIED_TO_CALCULATED)
      finalAmt = percentAmt + convertCat19Currency(paxTypeFare, discountInfo);
    else if (ind == RuleConst::SUBTRACT_SPECIFIED_FROM_CALCULATED)
      finalAmt = percentAmt - convertCat19Currency(paxTypeFare, discountInfo);

    if (UNLIKELY(_diag))
      *_diag << "  FARE BY RULE AND CAT19-22 CALCULATION AMOUNT  " << finalAmt << '\n';
  } // endtry - discountInfo access
  catch (...) {} // if discount fail, default already in finalAmt; NOP
  return finalAmt;
}

MoneyAmount
HIPMinimumFare::convertCat25Currency(const PaxTypeFare& paxTypeFare,
                                     const FareByRuleItemInfo& fbrItemInfo,
                                     bool useInternationalRounding)
{
  MoneyAmount specifiedFareAmt = 0;

  if ((fbrItemInfo.specifiedCur1().empty()) ||
      (fbrItemInfo.specifiedCur1()[0] == RuleConst::NO_CURRENCY))
    return specifiedFareAmt;

  const CurrencyCode* specifiedCur = nullptr;

  if ((fbrItemInfo.specifiedFareAmt1() == 0) && (fbrItemInfo.specifiedFareAmt2() == 0))
    return specifiedFareAmt;

  const CurrencyCode& paxTypeFareCurrency = paxTypeFare.currency();

  if (fbrItemInfo.specifiedCur1() == paxTypeFareCurrency)
  {
    specifiedFareAmt = fbrItemInfo.specifiedFareAmt1();
    specifiedCur = &(fbrItemInfo.specifiedCur1());
  }
  else if (fbrItemInfo.specifiedCur2() == paxTypeFareCurrency)
  {
    specifiedFareAmt = fbrItemInfo.specifiedFareAmt2();
    specifiedCur = &(fbrItemInfo.specifiedCur2());
  }
  else
    return specifiedFareAmt;

  CurrencyConversionFacade ccFacade;

  if (paxTypeFare.isInternational())
  {
    Money fareCurrency(specifiedFareAmt, *specifiedCur);
    Money nuc(NUC);

    if (ccFacade.convert(nuc, fareCurrency, _trx))
      specifiedFareAmt = nuc.value();
  }
  else
  {
    Money specified(specifiedFareAmt, *specifiedCur);
    Money fareCurrency(paxTypeFareCurrency);

    if (ccFacade.convert(fareCurrency, specified, _trx, useInternationalRounding))
      specifiedFareAmt = fareCurrency.value();
  }

  return specifiedFareAmt;
}

MoneyAmount
HIPMinimumFare::convertCat19Currency(const PaxTypeFare& paxTypeFare,
                                     const DiscountInfo& discountInfo)
{
  MoneyAmount matchFareAmt = 0;

  const CurrencyCode& fareCur = paxTypeFare.currency();

  const CurrencyCode& cur1 = discountInfo.cur1();
  const CurrencyCode& cur2 = discountInfo.cur2();

  const MoneyAmount& fareAmt1 = discountInfo.fareAmt1();
  const MoneyAmount& fareAmt2 = discountInfo.fareAmt2();

  // First check if this is an 'empty calculation'
  if ((fareAmt1 == 0) && (discountInfo.discPercent() == 0) &&
      (cur1.empty() || (cur1[0] == RuleConst::NO_CURRENCY)))
    return matchFareAmt;

  // Next see if we can match on currency;
  if (fareCur == cur1)
    matchFareAmt = fareAmt1;
  else if (fareCur == cur2)
    matchFareAmt = fareAmt2;
  else
  {
    MoneyAmount amt1 = -1;
    MoneyAmount amt2 = -1;
    Money nuc(NUC);
    CurrencyConversionFacade ccFacade;

    if (!cur1.empty())
    {
      if (fareAmt1 == 0)
        amt1 = 0; // We dont need to convert 0
      else
      {
        Money fareCurrency(fareAmt1, cur1);

        if (ccFacade.convert(nuc, fareCurrency, _trx, paxTypeFare.isInternational()))
        {
          amt1 = nuc.value();
        }
      }
    }

    if (!cur2.empty())
    {
      if (fareAmt2 == 0)
        amt2 = 0;
      else
      {
        Money fareCurrency(fareAmt2, cur2);

        if (ccFacade.convert(nuc, fareCurrency, _trx, paxTypeFare.isInternational()))
        {
          amt2 = nuc.value();
        }
      }
    }

    // Use the lowest one
    if (amt1 < 0 && amt2 < 0)
      return matchFareAmt; // Failure didnt find any amt

    if (amt1 < 0)
      matchFareAmt = amt2;
    else if (amt2 < 0)
      matchFareAmt = amt1;
    else if (amt1 < amt2)
      matchFareAmt = amt1;
    else if (amt2 < amt1)
      matchFareAmt = amt2;
    else
      matchFareAmt = amt1; // They must be equal
  }
  return matchFareAmt;
}

void
HIPMinimumFare::printPlusUpInfo(const PaxTypeFare& paxTypeFare,
                                const MinFarePlusUp& minFarePlusUp,
                                DiagCollector& diag)
{
  if (LIKELY(!diag.isActive()))
    return;

  MinFarePlusUp::const_iterator iter;
  MinFarePlusUp::const_iterator lowerPos = minFarePlusUp.lower_bound(HIP);
  MinFarePlusUp::const_iterator upperPos = minFarePlusUp.upper_bound(HIP);
  bool hasHip = false;
  for (iter = lowerPos; iter != upperPos; iter++)
  {
    hasHip = true;

    diag << "  BASE FARE NUC ";
    diag.setf(std::ios::right, std::ios::adjustfield);
    diag.setf(std::ios::fixed, std::ios::floatfield);
    diag.precision(2);
    diag << std::setw(8) << iter->second->baseAmount << " ";

    // Display EMS
    if (paxTypeFare.mileageSurchargePctg() > 0)
      diag << " " << paxTypeFare.mileageSurchargePctg() << "M";

    // Display HIP citypair
    diag << " " << iter->second->boardPoint << iter->second->offPoint << "\n";
  }

  if (!hasHip)
    diag << "  NO HIP\n";

  return;
}

bool
HIPMinimumFare::isNormalExempted(const std::vector<TravelSeg*>& tvl) const
{
  uint32_t hash = TravelSegUtil::hashCode(tvl);
  return (_normalExemptSet.find(hash) != _normalExemptSet.end());
}

bool
HIPMinimumFare::isExemptedByApplication(const PaxTypeFare& paxTypeFare)
{
  if (UNLIKELY(_matchedApplItem == nullptr))
  {
    if (LIKELY(!_diag))
      return true;

    printExceptionInfo(paxTypeFare, *_diag);
    *_diag << "FAILED MATCHING IN APPLICATION TABLE - RETURN" << '\n';

    return true;
  }

  if (_matchedApplItem->hipCheckAppl() == NO)
  {
    if (LIKELY(!_diag))
      return true;

    printExceptionInfo(paxTypeFare, *_diag);
    *_diag << "EXEMPT BY APPLICATION TABLE - " << _matchedApplItem->seqNo() << '\n';
    return true;
  }

  return false;
}
bool
HIPMinimumFare::skipProcessForSpecialOnly()
{
  if (UNLIKELY((_hipProcess == MinimumFare::NORMAL_HIP_PROCESSING) &&
      ((_matchedApplItem->applyDefaultLogic() == YES &&
        _matchedDefaultItem->spclHipSpclOnlyInd() == YES) ||
       (_matchedApplItem->applyDefaultLogic() == NO &&
        _matchedApplItem->spclHipSpclOnlyInd() == YES))))
  {
    return true;
  }
  return false;
}
}
