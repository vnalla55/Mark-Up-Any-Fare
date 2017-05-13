//----------------------------------------------------------------------------
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

#include "Pricing/CombinabilityScoreboard.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollectorGuard.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/Combinations.h"
#include "Pricing/PricingUtil.h"
#include "Rules/RuleUtil.h"

namespace tse
{
static Logger
logger("atseintl.Pricing.CombinabilityScoreboard");
bool CombinabilityScoreboard::_enableFailedFareUsageOptimization = true;

CombinabilityValidationResult
CombinabilityScoreboard::validate(PricingUnit& pu, FareUsage*& failedFareUsage, DiagCollector& diag)
{
  failedFareUsage = nullptr;

  if (UNLIKELY(pu.fareUsage().empty()))
  {
    LOG4CXX_WARN(logger, " *** No FareUsage *** ");
    displayDiag(diag, pu, FAILED_UNSPECIFIED, nullptr, nullptr);
    return CVR_UNSPECIFIED_FAILURE;
  }

  DiagCollectorGuard dcg1(diag, Diagnostic653);

  diag.printLine();
  diag << pu << std::endl;

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();

  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;
    if (UNLIKELY(fareUsage.paxTypeFare() == nullptr))
    {
      LOG4CXX_WARN(logger, " *** No Fare *** ");
      displayDiag(diag, pu, FAILED_UNSPECIFIED, &fareUsage, nullptr);
      return CVR_UNSPECIFIED_FAILURE;
    }

    if (UNLIKELY(_trx->getTrxType() == PricingTrx::MIP_TRX && fareUsage.paxTypeFare()->isDummyFare()))
      continue;

    CombinabilityValidationResult ret = checkRec2(pu, fareUsage, diag);

    if (ret != CVR_CONTINUE)
      return ret;
  }

  CombinabilityValidationResult ret = CVR_PASSED;

  processSamePointTable993(pu, diag);

  bool priorDiagState2 = diag.enable(Diagnostic606);
  diag << pu << std::endl;
  diag.restoreState(priorDiagState2);

  // Invalidate Record 2 Scoreboard
  // ------------------------------
  ret = invalidate(pu, failedFareUsage, diag);
  if (ret != CVR_PASSED)
  {
    if (UNLIKELY(!_enableFailedFareUsageOptimization))
    {
      failedFareUsage = nullptr;
    }

    LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return fail");
    return ret;
  }

  // Validate Record 2 Scoreboard
  // ----------------------------
  switch (pu.puType())
  {
  case PricingUnit::Type::ONEWAY:
  {
    ret = CVR_PASSED;
    break;
  }
  case PricingUnit::Type::OPENJAW:
  {
    ret = analyzeOpenJaw(pu, diag);
    break;
  }
  case PricingUnit::Type::ROUNDTRIP:
  {
    ret = analyzeRoundTrip(pu, diag);
    break;
  }
  case PricingUnit::Type::CIRCLETRIP:
  {
    ret = CVR_PASSED;
    break;
  }
  case PricingUnit::Type::UNKNOWN:
  {
    break;
  }
  default:
  {
    break;
  }
  }

  if (LIKELY(ret == CVR_PASSED))
  {
    displayDiag(diag, pu, PASSED_VALIDATION, pu.fareUsage()[0], nullptr);
  }

  return ret;
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkRec2(PricingUnit& pu, FareUsage& fareUsage, DiagCollector& diag)
{
  bool isLocationSwapped = false;
  CombinabilityRuleInfo* pCat10;

  if (fareUsage.rec2Cat10() == nullptr)
  {
    pCat10 =
        RuleUtil::getCombinabilityRuleInfo(*_trx, *(fareUsage.paxTypeFare()), isLocationSwapped);
    fareUsage.rec2Cat10() = pCat10;
  }
  else
  {
    pCat10 = fareUsage.rec2Cat10();
  }

  if (pCat10 != nullptr)
  {
    if (UNLIKELY(pCat10->applInd() == Combinations::NOT_APPLICABLE &&
        (!pu.noPUToEOE() || pu.puType() != PricingUnit::Type::ONEWAY)))
    {
      LOG4CXX_WARN(logger, " *** Rec 2 Cat 10 Not Applicable *** ");
      diag.enable(&pu, Diagnostic605);
      displayDiag(diag, pu, FAILED_REC_2_CAT_10_NOT_APPLICABLE, &fareUsage, nullptr);
      return CVR_UNSPECIFIED_FAILURE;
    }
  }
  else
  {
    LOG4CXX_WARN(logger,
                 fareUsage.paxTypeFare()->fare()->fareClass() << ": *** No Rec 2 Cat 10 ***");

    diag.enable(&pu, Diagnostic605);
    if (pu.fareUsage().size() == 1 || isMirrorImage(pu))
    {
      displayDiag(diag, pu, PASSED_SYSTEM_ASSUMPTION);
      return CVR_PASSED;
    }
    else
    {
      displayDiag(diag, pu, FAILED_NO_REC_2_CAT_10, &fareUsage, nullptr);
      return CVR_UNSPECIFIED_FAILURE;
    }
  }
  return CVR_CONTINUE;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::processSamePointTable993(PricingUnit& pu, DiagCollector& diag)
{
  if (pu.puType() != PricingUnit::Type::OPENJAW)
  {
    return;
  }

  const FareUsage& fuFirst = *pu.fareUsage().front();
  const FareUsage& fuLast = *pu.fareUsage().back();

  bool origOJCloses = false;
  bool destOJCloses = false;

  if (processSamePointTable993(fuFirst, fuLast, diag))
  {
    origOJCloses = true;
  }

  size_t numOfFU = pu.fareUsage().size();
  for (size_t fuCount = 1; fuCount < numOfFU; ++fuCount)
  {
    const FareUsage& curFU = *pu.fareUsage()[fuCount];
    const FareUsage& preFU = *pu.fareUsage()[fuCount - 1];
    if (processSamePointTable993(curFU, preFU, diag))
    {
      destOJCloses = true;
      break;
    }
  }

  if (!origOJCloses && !destOJCloses)
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag.flushMsg();
    }
    return;
  }

  diag.enable(&pu, Diagnostic605);
  if (pu.puSubType() == PricingUnit::DOUBLE_OPENJAW)
  {
    if (origOJCloses && destOJCloses)
    {
      if (pu.fareUsage().size() > 2)
      {
        pu.puType() = PricingUnit::Type::CIRCLETRIP;
        if (diag.isActive())
        {
          diag << " SAMEPOINT TABLE: CONVERT TO CT\n";
        }
      }
      else
      {
        pu.puType() = PricingUnit::Type::ROUNDTRIP;
        if (diag.isActive())
        {
          diag << " SAMEPOINT TABLE: CONVERT TO RT\n";
        }
      }
    }
    else if (origOJCloses)
    {
      pu.puSubType() = PricingUnit::DEST_OPENJAW;
      if (diag.isActive())
      {
        diag << " SAMEPOINT TABLE: CONVERT TO DEST-OJ\n";
      }
    }
    else if (destOJCloses)
    {
      pu.puSubType() = PricingUnit::ORIG_OPENJAW;
      if (diag.isActive())
      {
        diag << " SAMEPOINT TABLE: CONVERT TO ORIG-OJ\n";
      }
    }
  }
  else
  {
    if (origOJCloses || destOJCloses)
    {
      if (pu.fareUsage().size() > 2)
      {
        pu.puType() = PricingUnit::Type::CIRCLETRIP;
        if (diag.isActive())
        {
          diag << " SAMEPOINT TABLE: CONVERT TO CT\n";
        }
      }
      else
      {
        pu.puType() = PricingUnit::Type::ROUNDTRIP;
        if (diag.isActive())
        {
          diag << " SAMEPOINT TABLE: CONVERT TO RT\n";
        }
      }
    }
  }

  if (diag.isActive())
  {
    diag.flushMsg();
  }
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::processSamePointTable993(const FareUsage& curFareUsage,
                                                  const FareUsage& preFareUsage,
                                                  DiagCollector& diag)

{
  const LocCode ojPoint1 = curFareUsage.paxTypeFare()->fareMarket()->boardMultiCity();
  const LocCode ojPoint2 = preFareUsage.paxTypeFare()->fareMarket()->offMultiCity();

  if (ojPoint1 == ojPoint2)
  {
    return false;
  }

  diag.enable(Diagnostic653);
  if (UNLIKELY(diag.isActive()))
  {
    diag << "  VALIDATE POINT: " << ojPoint1 << " - " << ojPoint2 << std::endl;
  }

  const DateTime& travelDate1 = curFareUsage.paxTypeFare()->fareMarket()->travelDate();
  const DateTime& travelDate2 = preFareUsage.paxTypeFare()->fareMarket()->travelDate();
  if (processSamePointTable993(curFareUsage, ojPoint1, ojPoint2, travelDate1, diag) &&
      processSamePointTable993(preFareUsage, ojPoint1, ojPoint2, travelDate2, diag))
  {
    diag.enable(Diagnostic653);
    if (diag.isActive())
    {
      diag << "  SAME POINT: " << ojPoint1 << " - " << ojPoint2 << std::endl;
    }
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::processSamePointTable993(const FareUsage& fareUsage,
                                                  const LocCode& loc1,
                                                  const LocCode& loc2,
                                                  const DateTime& travelDate,
                                                  DiagCollector& diag)

{

  // SamePoint info does not vary between Rec2 to Rec2,
  // therefore, one Rec2 check is good enough

  const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();

  diag.enable(Diagnostic653);
  if (UNLIKELY(diag.isActive()))
  {
    diag << std::endl << " ------" << std::endl << " VENDOR " << pCat10->vendorCode()
         << " - SAME POINT TABLE ITEM NO " << pCat10->samepointstblItemNo() << " - FARE "
         << fareUsage.paxTypeFare()->fareClass() << std::endl;
  }

  if (pCat10->samepointstblItemNo() != 0)
  {
    return RuleUtil::validateSamePoint(*_trx,
                                       pCat10->vendorCode(),
                                       pCat10->samepointstblItemNo(),
                                       loc1,
                                       loc2,
                                       travelDate,
                                       &diag,
                                       Diagnostic653);
  }
  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function bool CombinabilityScoreboard::invalidate
//
// Description:  Preliminary check record 2 cat 10 scoreboard
//
// @param  pu - Pricing Unit
//
// @param  diag - Diagnostic object
//
// @return true if it pass
//
// </PRE>
// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidate(PricingUnit& pu,
                                    FareUsage*& failedFareUsage,
                                    DiagCollector& diag)
{
  failedFareUsage = nullptr;
  CombinabilityValidationResult retResult = CVR_PASSED;

  if (pu.fareUsage().size() >= 2)
  {
    LOG4CXX_INFO(logger, " in CombinabilityScoreboard::invalidate() >= 2");

    // ----- Process by PU type ---------
    // ----------------------------------
    switch (pu.puType())
    {
    case PricingUnit::Type::ROUNDTRIP:
    {
      retResult = invalidateRoundTrip(pu, diag);
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retResult = invalidateCircleTrip(pu, failedFareUsage, diag);
      break;
    }
    case PricingUnit::Type::OPENJAW:
    {
      retResult = invalidateOpenJaw(pu, failedFareUsage, diag);
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retResult = invalidateOneWay(pu, diag);
      break;
    }
    default:
    {
      displayDiag(diag, pu, FAILED_REC2_SCOREBOARD, pu.fareUsage()[0], nullptr);
      LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return false");
      retResult = CVR_UNSPECIFIED_FAILURE;
      break;
    }
    }
  }

  return retResult;
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidateRoundTrip(PricingUnit& pu, DiagCollector& diag)
{
  const bool mirrorImage = isMirrorImage(pu);

  bool chk102Carrier = false;
  bool chk102Rule = false;
  bool chk102Tariff = false;
  bool chk102Class = false;
  bool chk102Type = false;

  size_t sameCarrierCount = 0;
  size_t sameRuleCount = 0;
  size_t sameTariffCount = 0;
  size_t sameClassCount = 0;
  size_t sameTypeCount = 0;

  CombinabilityValidationResult retResult = CVR_PASSED;

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  const PaxTypeFare& firstFarePaxTypeFare = *((*fuIt)->paxTypeFare());

  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;

    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret =
        checkCt2IndicatorToInvalidate(pu, mirrorImage, *pCat10, diag);
    if (ret == CVR_RT_NOT_PERMITTED)
      return ret; // no need to continue
    else
    {
      if (retResult != CVR_PASSED || ret != CVR_PASSED)
      {
        retResult = CVR_CONTINUE;
      }
      checkCt2FareRuleData(
          *pCat10, chk102Carrier, chk102Rule, chk102Tariff, chk102Class, chk102Type);
    }

    if (fuIt != pu.fareUsage().begin())
    {
      comparePaxTypeFares(firstFarePaxTypeFare,
                          *((*fuIt)->paxTypeFare()),
                          sameCarrierCount,
                          sameRuleCount,
                          sameTariffCount,
                          sameClassCount,
                          sameTypeCount);
    }
  }

  if (retResult == CVR_PASSED)
  {
    return CVR_PASSED;
  }

  return invalidateRuleAndFareData(pu,
                                   diag,
                                   chk102Carrier,
                                   sameCarrierCount,
                                   chk102Rule,
                                   sameRuleCount,
                                   chk102Tariff,
                                   sameTariffCount,
                                   chk102Class,
                                   sameClassCount,
                                   chk102Type,
                                   sameTypeCount);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkCt2IndicatorToInvalidate(PricingUnit& pu,
                                                       bool mirrorImage,
                                                       const CombinabilityRuleInfo& pCat10,
                                                       DiagCollector& diag)
{
  bool indRtPermitted = true;
  bool indRtNotPermitted = false;

  if (mirrorImage)
  {
    indRtPermitted =
        ((pCat10.ct2Ind() == Combinations::ROUND_TRIP_PERMITTED_MIRROR_IMAGE_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_RESTRICTIONS_MIRROR_IMAGE_PERMITTED));

    indRtNotPermitted =
        ((pCat10.ct2Ind() == Combinations::ROUND_TRIP_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_RESTRICTIONS_MIRROR_IMAGE_NOT_PERMITTED));
  }
  else
  {
    indRtPermitted =
        ((pCat10.ct2Ind() == Combinations::ROUND_TRIP_PERMITTED_MIRROR_IMAGE_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED));

    indRtNotPermitted =
        ((pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_PERMITTED) ||
         (pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED));
  }

  CombinabilityValidationResult retResult = CVR_CONTINUE;
  if (indRtPermitted)
  {
    retResult = CVR_PASSED;
  }
  else if (indRtNotPermitted)
  {
    displayDiag(diag, pu, FAILED_ROUND_TRIP_NOT_PERMITTED);
    retResult = CVR_RT_NOT_PERMITTED;
  }

  return retResult;
}
// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::checkCt2FareRuleData(const CombinabilityRuleInfo& pCat10,
                                              bool& chk102Carrier,
                                              bool& chk102Rule,
                                              bool& chk102Tariff,
                                              bool& chk102Class,
                                              bool& chk102Type)
{

  chk102Carrier |= (pCat10.ct2SameCarrierInd() == Combinations::SAME_CARRIER);
  chk102Rule |= (pCat10.ct2SameRuleTariffInd() == Combinations::SAME_RULE);
  chk102Tariff |= (pCat10.ct2SameRuleTariffInd() == Combinations::SAME_TARIFF);
  chk102Class |= (pCat10.ct2SameFareInd() == Combinations::SAME_FARECLASS);
  chk102Type |= (pCat10.ct2SameFareInd() == Combinations::SAME_FARETYPE);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidateCircleTrip(PricingUnit& pu,
                                              FareUsage*& failedFareUsage,
                                              DiagCollector& diag)
{
  failedFareUsage = nullptr;

  bool chk103Carrier = false;
  bool chk103Rule = false;
  bool chk103Tariff = false;
  bool chk103Class = false;
  bool chk103Type = false;

  size_t sameCarrierCount = 0;
  size_t sameRuleCount = 0;
  size_t sameTariffCount = 0;
  size_t sameClassCount = 0;
  size_t sameTypeCount = 0;

  CombinabilityValidationResult retResult = CVR_PASSED;

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  const PaxTypeFare& firstFarePaxTypeFare = *((*fuIt)->paxTypeFare());

  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;

    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret = checkCt2PlusIndicatorToInvalidate(pu, *pCat10, diag);
    if (ret == CVR_CT_NOT_PERMITTED)
    {
      failedFareUsage = *fuIt;
      return ret; // no need to continue
    }
    else
    {
      if (retResult != CVR_PASSED || ret != CVR_PASSED)
      {
        retResult = CVR_CONTINUE;
      }
      checkCt2PlusFareRuleData(
          *pCat10, chk103Carrier, chk103Rule, chk103Tariff, chk103Class, chk103Type);
    }

    if (fuIt != pu.fareUsage().begin())
    {
      comparePaxTypeFares(firstFarePaxTypeFare,
                          *((*fuIt)->paxTypeFare()),
                          sameCarrierCount,
                          sameRuleCount,
                          sameTariffCount,
                          sameClassCount,
                          sameTypeCount);
    }
  }

  if (retResult == CVR_PASSED)
  {
    return CVR_PASSED;
  }

  return invalidateRuleAndFareData(pu,
                                   diag,
                                   chk103Carrier,
                                   sameCarrierCount,
                                   chk103Rule,
                                   sameRuleCount,
                                   chk103Tariff,
                                   sameTariffCount,
                                   chk103Class,
                                   sameClassCount,
                                   chk103Type,
                                   sameTypeCount);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkCt2PlusIndicatorToInvalidate(PricingUnit& pu,
                                                           const CombinabilityRuleInfo& pCat10,
                                                           DiagCollector& diag)
{

  bool indCtPermitted = (pCat10.ct2plusInd() == Combinations::PERMITTED);
  bool indCtNotPermitted = (pCat10.ct2plusInd() == Combinations::NOT_PERMITTED);

  if (indCtPermitted)
  {
    return CVR_PASSED;
  }
  else if (indCtNotPermitted)
  {
    displayDiag(diag, pu, FAILED_CIRCLE_TRIP_NOT_PERMITTED);
    return CVR_CT_NOT_PERMITTED;
  }

  return CVR_CONTINUE;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::checkCt2PlusFareRuleData(const CombinabilityRuleInfo& pCat10,
                                                  bool& chk103Carrier,
                                                  bool& chk103Rule,
                                                  bool& chk103Tariff,
                                                  bool& chk103Class,
                                                  bool& chk103Type)
{

  chk103Carrier |= (pCat10.ct2plusSameCarrierInd() == Combinations::SAME_CARRIER);
  chk103Rule |= (pCat10.ct2plusSameRuleTariffInd() == Combinations::SAME_RULE);
  chk103Tariff |= (pCat10.ct2plusSameRuleTariffInd() == Combinations::SAME_TARIFF);
  chk103Class |= (pCat10.ct2plusSameFareInd() == Combinations::SAME_FARECLASS);
  chk103Type |= (pCat10.ct2plusSameFareInd() == Combinations::SAME_FARETYPE);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidateOpenJaw(PricingUnit& pu,
                                           FareUsage*& failedFareUsage,
                                           DiagCollector& diag)
{
  failedFareUsage = nullptr;

  bool chk101Carrier = false;
  bool chk101Rule = false;
  bool chk101Tariff = false;
  bool chk101Class = false;
  bool chk101Type = false;

  size_t sameCarrierCount = 0;
  size_t sameRuleCount = 0;
  size_t sameTariffCount = 0;
  size_t sameClassCount = 0;
  size_t sameTypeCount = 0;

  CombinabilityValidationResult retResult = CVR_PASSED;

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  const PaxTypeFare& firstFarePaxTypeFare = *((*fuIt)->paxTypeFare());

  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;

    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret = checkOJIndicatorToInvalidate(pu, *pCat10, diag);

    if (ret == CVR_SOJ_NOT_PERMITTED || ret == CVR_DOJ_NOT_PERMITTED)
    {
      failedFareUsage = *fuIt;
      return ret; // no need to continue
    }
    else
    {
      if (retResult != CVR_PASSED || ret != CVR_PASSED)
      {
        retResult = CVR_CONTINUE;
      }
      checkOJFareRuleData(
          *pCat10, chk101Carrier, chk101Rule, chk101Tariff, chk101Class, chk101Type);
    }

    if (fuIt != pu.fareUsage().begin())
    {
      comparePaxTypeFares(firstFarePaxTypeFare,
                          *((*fuIt)->paxTypeFare()),
                          sameCarrierCount,
                          sameRuleCount,
                          sameTariffCount,
                          sameClassCount,
                          sameTypeCount);
    }
  }

  if (retResult == CVR_PASSED)
  {
    return CVR_PASSED;
  }

  return invalidateRuleAndFareData(pu,
                                   diag,
                                   chk101Carrier,
                                   sameCarrierCount,
                                   chk101Rule,
                                   sameRuleCount,
                                   chk101Tariff,
                                   sameTariffCount,
                                   chk101Class,
                                   sameClassCount,
                                   chk101Type,
                                   sameTypeCount);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkOJIndicatorToInvalidate(PricingUnit& pu,
                                                      const CombinabilityRuleInfo& pCat10,
                                                      DiagCollector& diag)
{

  bool indSojPermitted = (pCat10.sojInd() == Combinations::PERMITTED);
  bool indDojPermitted = (pCat10.dojInd() == Combinations::PERMITTED);
  bool indSojNotPermitted = (pCat10.sojInd() == Combinations::NOT_PERMITTED);
  bool indDojNotPermitted = (pCat10.dojInd() == Combinations::NOT_PERMITTED);

  if (pu.puSubType() == PricingUnit::DEST_OPENJAW || pu.puSubType() == PricingUnit::ORIG_OPENJAW)
  {
    if (indSojPermitted)
    {
      return CVR_PASSED;
    }
    else if (UNLIKELY(indSojNotPermitted))
    {
      displayDiag(diag, pu, FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED);
      LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return false");
      return CVR_SOJ_NOT_PERMITTED;
    }
  }
  else if (LIKELY(pu.puSubType() == PricingUnit::DOUBLE_OPENJAW))
  {
    if (indDojPermitted)
    {
      return CVR_PASSED;
    }
    else if (indDojNotPermitted)
    {
      displayDiag(diag, pu, FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED);
      LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return false");
      return CVR_DOJ_NOT_PERMITTED;
    }
  }

  return CVR_CONTINUE;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::checkOJFareRuleData(const CombinabilityRuleInfo& pCat10,
                                             bool& chk101Carrier,
                                             bool& chk101Rule,
                                             bool& chk101Tariff,
                                             bool& chk101Class,
                                             bool& chk101Type)
{
  chk101Carrier |= (pCat10.dojSameCarrierInd() == Combinations::SAME_CARRIER);
  chk101Rule |= (pCat10.dojSameRuleTariffInd() == Combinations::SAME_RULE);
  chk101Tariff |= (pCat10.dojSameRuleTariffInd() == Combinations::SAME_TARIFF);
  chk101Class |= (pCat10.dojSameFareInd() == Combinations::SAME_FARECLASS);
  chk101Type |= (pCat10.dojSameFareInd() == Combinations::SAME_FARETYPE);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidateOneWay(PricingUnit& pu, DiagCollector& diag)
{

  bool chk104Carrier = false;
  bool chk104Rule = false;
  bool chk104Tariff = false;
  bool chk104Class = false;
  bool chk104Type = false;

  size_t sameCarrierCount = 0;
  size_t sameRuleCount = 0;
  size_t sameTariffCount = 0;
  size_t sameClassCount = 0;
  size_t sameTypeCount = 0;

  CombinabilityValidationResult retResult = CVR_PASSED;

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  const PaxTypeFare& firstFarePaxTypeFare = *((*fuIt)->paxTypeFare());

  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;

    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret = checkOWIndicatorToInvalidate(pu, *pCat10, diag);
    if (ret == CVR_EOE_NOT_PERMITTED)
    {
      return ret; // no need to continue
    }
    else
    {
      if (retResult != CVR_PASSED || ret != CVR_PASSED)
      {
        retResult = CVR_CONTINUE;
      }
      checkOWFareRuleData(
          *pCat10, chk104Carrier, chk104Rule, chk104Tariff, chk104Class, chk104Type);
    }

    if (fuIt != pu.fareUsage().begin())
    {
      comparePaxTypeFares(firstFarePaxTypeFare,
                          *((*fuIt)->paxTypeFare()),
                          sameCarrierCount,
                          sameRuleCount,
                          sameTariffCount,
                          sameClassCount,
                          sameTypeCount);
    }
  }

  if (retResult == CVR_PASSED)
  {
    return CVR_PASSED;
  }

  return invalidateRuleAndFareData(pu,
                                   diag,
                                   chk104Carrier,
                                   sameCarrierCount,
                                   chk104Rule,
                                   sameRuleCount,
                                   chk104Tariff,
                                   sameTariffCount,
                                   chk104Class,
                                   sameClassCount,
                                   chk104Type,
                                   sameTypeCount);
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkOWIndicatorToInvalidate(PricingUnit& pu,
                                                      const CombinabilityRuleInfo& pCat10,
                                                      DiagCollector& diag)
{

  bool indEoeNotPermitted =
      (pCat10.eoeInd() == Combinations::NOT_PERMITTED ||
       pCat10.eoeInd() == Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED);

  if (indEoeNotPermitted)
  {
    displayDiag(diag, pu, FAILED_END_ON_END_NOT_PERMITTED);
    LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return false");
    return CVR_EOE_NOT_PERMITTED;
  }

  return CVR_CONTINUE;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::checkOWFareRuleData(const CombinabilityRuleInfo& pCat10,
                                             bool& chk104Carrier,
                                             bool& chk104Rule,
                                             bool& chk104Tariff,
                                             bool& chk104Class,
                                             bool& chk104Type)
{
  chk104Carrier |= (pCat10.eoeSameCarrierInd() == Combinations::SAME_CARRIER);
  chk104Rule |= (pCat10.eoeSameRuleTariffInd() == Combinations::SAME_RULE);
  chk104Tariff |= (pCat10.eoeSameRuleTariffInd() == Combinations::SAME_TARIFF);
  chk104Class |= (pCat10.eoeSameFareInd() == Combinations::SAME_FARECLASS);
  chk104Type |= (pCat10.eoeSameFareInd() == Combinations::SAME_FARETYPE);
}

namespace
{
inline bool
sameYYGovCxr(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2)
{
  return ptf1.carrier() == INDUSTRY_CARRIER &&
         ptf1.fareMarket()->governingCarrier() == ptf2.carrier();
}
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::comparePaxTypeFares(const PaxTypeFare& firstPtf,
                                             const PaxTypeFare& secondPtf,
                                             size_t& sameCarrierCount,
                                             size_t& sameRuleCount,
                                             size_t& sameTariffCount,
                                             size_t& sameClassCount,
                                             size_t& sameTypeCount) const
{
  if (firstPtf.carrier() == secondPtf.carrier() || sameYYGovCxr(firstPtf, secondPtf) ||
      sameYYGovCxr(secondPtf, firstPtf))
    ++sameCarrierCount;

  if (firstPtf.ruleNumber() == secondPtf.ruleNumber())
  {
    sameRuleCount++;
  }

  if (firstPtf.tcrRuleTariff() == secondPtf.tcrRuleTariff())
  {
    sameTariffCount++;
  }

  if (firstPtf.fareClass() == secondPtf.fareClass())
  {
    sameClassCount++;
  }

  if (firstPtf.fcaFareType() == secondPtf.fcaFareType())
  {
    sameTypeCount++;
  }
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::invalidateRuleAndFareData(PricingUnit& pu,
                                                   DiagCollector& diag,
                                                   bool checkCarrier,
                                                   size_t sameCarrierCount,
                                                   bool checkRule,
                                                   size_t sameRuleCount,
                                                   bool checkTariff,
                                                   size_t sameTariffCount,
                                                   bool checkClass,
                                                   size_t sameClassCount,
                                                   bool checkType,
                                                   size_t sameTypeCount) const
{
  DiagnosticID diagID;
  const PricingUnit::Type puType = pu.puType();
  const size_t numOfFUMinusOne = pu.fareUsage().size() - 1;

  if (checkCarrier && sameCarrierCount != numOfFUMinusOne)
  {
    diagID = getErrorID(FAILED_SAME_CARRIER_REQUIRED, puType);
  }
  else if (checkRule && sameRuleCount != numOfFUMinusOne)
  {
    diagID = getErrorID(FAILED_SAME_RULE_REQUIRED, puType);
  }
  else if (checkTariff && sameTariffCount != numOfFUMinusOne)
  {
    diagID = getErrorID(FAILED_SAME_TARIFF_REQUIRED, puType);
  }
  else if (checkClass && sameClassCount != numOfFUMinusOne)
  {
    diagID = getErrorID(FAILED_SAME_FARECLASS_REQUIRED, puType);
  }
  else if (checkType && sameTypeCount != numOfFUMinusOne)
  {
    diagID = getErrorID(FAILED_SAME_FARETYPE_REQUIRED, puType);
  }
  else
  {
    return CVR_PASSED;
  }

  displayDiag(diag, pu, diagID);
  LOG4CXX_INFO(logger, __LINE__ << ", invalidate() return false");
  return CVR_UNSPECIFIED_FAILURE;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::displayDiag(DiagCollector& diag,
                                     const PricingUnit& pu,
                                     DiagnosticID failureReason) const
{
  DiagCollectorGuard dcg1(diag, &pu, Diagnostic605);

  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, pu)))
  {
    // IF there is no fare usage info necessary for this diagnostic
    if (failureReason == PASSED_VALIDATION || failureReason == PASSED_SYSTEM_ASSUMPTION ||
        failureReason == FAILED_UNSPECIFIED ||
        failureReason == FAILED_REC_2_CAT_10_NOT_APPLICABLE ||
        failureReason == FAILED_NO_REC_2_CAT_10)
    {
      // Just display the diagnostic
      displayDiag(diag, pu, failureReason, pu.fareUsage()[0], nullptr);
    }
    // ELSE (we need to determine the true cause of the failure)
    else
    {
      size_t numOfFU = pu.fareUsage().size();

      if (numOfFU < 2)
      {
        LOG4CXX_FATAL(logger, " in CombinabilityScoreboard::invalidate() < 2");
      }
      else
      {
        for (size_t fuSourceCount = 0; fuSourceCount < numOfFU; ++fuSourceCount)
        {
          const FareUsage* sourceFareUsage = pu.fareUsage()[fuSourceCount];

          for (size_t fuTargetCount = 0; fuTargetCount < numOfFU; ++fuTargetCount)
          {
            if (fuTargetCount != fuSourceCount)
            {
              const FareUsage* targetFareUsage = pu.fareUsage()[fuTargetCount];

              // ----- Process by PU type ---------
              // ----------------------------------
              switch (pu.puType())
              {
              case PricingUnit::Type::ROUNDTRIP:
              {
                if (displayRoundTripDiag(diag, pu, failureReason, sourceFareUsage, targetFareUsage))
                {
                  return;
                }
                break;
              }
              case PricingUnit::Type::CIRCLETRIP:
              {
                if (displayCircleTripDiag(
                        diag, pu, failureReason, sourceFareUsage, targetFareUsage))
                {
                  return;
                }
                break;
              }
              case PricingUnit::Type::OPENJAW:
              {
                if (displayOpenJawDiag(
                        pu.puSubType(), diag, pu, failureReason, sourceFareUsage, targetFareUsage))
                {
                  return;
                }
                break;
              }
              case PricingUnit::Type::ONEWAY:
              {
                if (displayOneWayDiag(diag, pu, failureReason, sourceFareUsage, targetFareUsage))
                {
                  return;
                }
                break;
              }
              default:
              {
                LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
                break;
              }
              } // end switch
            }
          } // for fuTargetCount
        } // for fuSourceCount
      }
    }
  }
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::displayRoundTripDiag(DiagCollector& diag,
                                              const PricingUnit& pu,
                                              DiagnosticID failureReason,
                                              const FareUsage* sourceFareUsage,
                                              const FareUsage* targetFareUsage) const
{
  const CombinabilityRuleInfo* pCat10 = sourceFareUsage->rec2Cat10();

  bool chk102Carrier = (pCat10->ct2SameCarrierInd() == Combinations::SAME_CARRIER);
  bool chk102Rule = (pCat10->ct2SameRuleTariffInd() == Combinations::SAME_RULE);
  bool chk102Tariff = (pCat10->ct2SameRuleTariffInd() == Combinations::SAME_TARIFF);
  bool chk102Class = (pCat10->ct2SameFareInd() == Combinations::SAME_FARECLASS);
  bool chk102Type = (pCat10->ct2SameFareInd() == Combinations::SAME_FARETYPE);

  bool indRtPermitted = (pCat10->ct2Ind() == Combinations::PERMITTED);
  bool indRtNotPermitted = (pCat10->ct2Ind() == Combinations::NOT_PERMITTED);

  bool retHandled;
  if (indRtPermitted)
  {
    LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
    retHandled = true;
  }
  else if (indRtNotPermitted)
  {
    if (failureReason == FAILED_ROUND_TRIP_NOT_PERMITTED)
    {
      displayDiag(diag, pu, FAILED_ROUND_TRIP_NOT_PERMITTED, sourceFareUsage, targetFareUsage);
    }
    retHandled = true;
  }
  else
  {
    retHandled = displayDiagForRuleAndFareData(diag,
                                               pu,
                                               failureReason,
                                               PricingUnit::Type::ROUNDTRIP,
                                               sourceFareUsage,
                                               targetFareUsage,
                                               chk102Carrier,
                                               chk102Rule,
                                               chk102Tariff,
                                               chk102Class,
                                               chk102Type);
  }
  return retHandled;
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::displayCircleTripDiag(DiagCollector& diag,
                                               const PricingUnit& pu,
                                               DiagnosticID failureReason,
                                               const FareUsage* sourceFareUsage,
                                               const FareUsage* targetFareUsage) const
{
  const CombinabilityRuleInfo* pCat10 = sourceFareUsage->rec2Cat10();

  bool chk103Carrier = (pCat10->ct2plusSameCarrierInd() == Combinations::SAME_CARRIER);
  bool chk103Rule = (pCat10->ct2plusSameRuleTariffInd() == Combinations::SAME_RULE);
  bool chk103Tariff = (pCat10->ct2plusSameRuleTariffInd() == Combinations::SAME_TARIFF);
  bool chk103Class = (pCat10->ct2plusSameFareInd() == Combinations::SAME_FARECLASS);
  bool chk103Type = (pCat10->ct2plusSameFareInd() == Combinations::SAME_FARETYPE);

  bool indCtPermitted = (pCat10->ct2plusInd() == Combinations::PERMITTED);
  bool indCtNotPermitted = (pCat10->ct2plusInd() == Combinations::NOT_PERMITTED);

  bool retHandled;
  if (indCtPermitted)
  {
    LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
    retHandled = true;
  }
  else if (indCtNotPermitted)
  {
    displayFailureReason(diag,
                         pu,
                         FAILED_CIRCLE_TRIP_NOT_PERMITTED,
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  else
  {
    retHandled = displayDiagForRuleAndFareData(diag,
                                               pu,
                                               failureReason,
                                               PricingUnit::Type::CIRCLETRIP,
                                               sourceFareUsage,
                                               targetFareUsage,
                                               chk103Carrier,
                                               chk103Rule,
                                               chk103Tariff,
                                               chk103Class,
                                               chk103Type);
  }
  return retHandled;
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::displayOpenJawDiag(PricingUnit::PUSubType puSubType,
                                            DiagCollector& diag,
                                            const PricingUnit& pu,
                                            DiagnosticID failureReason,
                                            const FareUsage* sourceFareUsage,
                                            const FareUsage* targetFareUsage) const
{
  const CombinabilityRuleInfo* pCat10 = sourceFareUsage->rec2Cat10();

  bool chk101Carrier = (pCat10->dojSameCarrierInd() == Combinations::SAME_CARRIER);
  bool chk101Rule = (pCat10->dojSameRuleTariffInd() == Combinations::SAME_RULE);
  bool chk101Tariff = (pCat10->dojSameRuleTariffInd() == Combinations::SAME_TARIFF);
  bool chk101Class = (pCat10->dojSameFareInd() == Combinations::SAME_FARECLASS);
  bool chk101Type = (pCat10->dojSameFareInd() == Combinations::SAME_FARETYPE);

  bool indSojPermitted = (pCat10->sojInd() == Combinations::PERMITTED);
  bool indSojNotPermitted = (pCat10->sojInd() == Combinations::NOT_PERMITTED);
  bool indDojPermitted = (pCat10->dojInd() == Combinations::PERMITTED);
  bool indDojNotPermitted = (pCat10->dojInd() == Combinations::NOT_PERMITTED);

  if (puSubType == PricingUnit::DEST_OPENJAW || puSubType == PricingUnit::ORIG_OPENJAW)
  {
    if (indSojPermitted)
    {
      LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
      return true;
    }
    else if (indSojNotPermitted)
    {
      displayFailureReason(diag,
                           pu,
                           FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED,
                           failureReason,
                           sourceFareUsage,
                           targetFareUsage);
      return true;
    }
  }
  else if (puSubType == PricingUnit::DOUBLE_OPENJAW)
  {
    if (indDojPermitted)
    {
      LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
      return true;
    }
    else if (indDojNotPermitted)
    {
      displayFailureReason(diag,
                           pu,
                           FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED,
                           failureReason,
                           sourceFareUsage,
                           targetFareUsage);
      return true;
    }
  }

  return displayDiagForRuleAndFareData(diag,
                                       pu,
                                       failureReason,
                                       PricingUnit::Type::OPENJAW,
                                       sourceFareUsage,
                                       targetFareUsage,
                                       chk101Carrier,
                                       chk101Rule,
                                       chk101Tariff,
                                       chk101Class,
                                       chk101Type);
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::displayOneWayDiag(DiagCollector& diag,
                                           const PricingUnit& pu,
                                           DiagnosticID failureReason,
                                           const FareUsage* sourceFareUsage,
                                           const FareUsage* targetFareUsage) const
{
  const CombinabilityRuleInfo* pCat10 = sourceFareUsage->rec2Cat10();

  bool chk104Carrier = (pCat10->eoeSameCarrierInd() == Combinations::SAME_CARRIER);
  bool chk104Rule = (pCat10->eoeSameRuleTariffInd() == Combinations::SAME_RULE);
  bool chk104Tariff = (pCat10->eoeSameRuleTariffInd() == Combinations::SAME_TARIFF);
  bool chk104Class = (pCat10->eoeSameFareInd() == Combinations::SAME_FARECLASS);
  bool chk104Type = (pCat10->eoeSameFareInd() == Combinations::SAME_FARETYPE);

  bool indEoeNotPermitted =
      (pCat10->eoeInd() == Combinations::NOT_PERMITTED ||
       pCat10->eoeInd() == Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED);

  bool retHandled;
  if (indEoeNotPermitted)
  {
    displayFailureReason(
        diag, pu, FAILED_END_ON_END_NOT_PERMITTED, failureReason, sourceFareUsage, targetFareUsage);
    retHandled = true;
  }
  else
  {
    retHandled = displayDiagForRuleAndFareData(diag,
                                               pu,
                                               failureReason,
                                               PricingUnit::Type::ONEWAY,
                                               sourceFareUsage,
                                               targetFareUsage,
                                               chk104Carrier,
                                               chk104Rule,
                                               chk104Tariff,
                                               chk104Class,
                                               chk104Type);
  }
  return retHandled;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::displayFailureReason(DiagCollector& diag,
                                              const PricingUnit& pu,
                                              DiagnosticID expectedFailureReason,
                                              DiagnosticID failureReason,
                                              const FareUsage* sourceFareUsage,
                                              const FareUsage* targetFareUsage) const
{
  if (expectedFailureReason == failureReason)
  {
    displayDiag(diag, pu, failureReason, sourceFareUsage, targetFareUsage);
  }
  else
  {
    LOG4CXX_FATAL(logger, __LINE__ << ", invalid failure reason");
  }
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::displayDiagForRuleAndFareData(DiagCollector& diag,
                                                       const PricingUnit& pu,
                                                       DiagnosticID failureReason,
                                                       PricingUnit::Type puType,
                                                       const FareUsage* sourceFareUsage,
                                                       const FareUsage* targetFareUsage,
                                                       bool checkCarrier,
                                                       bool checkRule,
                                                       bool checkTariff,
                                                       bool checkClass,
                                                       bool checkType) const
{
  bool retHandled = false;

  const PaxTypeFare* sourcePaxTypeFare = sourceFareUsage->paxTypeFare();
  const PaxTypeFare* targetPaxTypeFare = targetFareUsage->paxTypeFare();

  if (checkCarrier && (sourcePaxTypeFare->carrier() != targetPaxTypeFare->carrier()))
  {
    displayFailureReason(diag,
                         pu,
                         getErrorID(FAILED_SAME_CARRIER_REQUIRED, puType),
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  else if (checkRule &&
           ((sourcePaxTypeFare->ruleNumber() != targetPaxTypeFare->ruleNumber()) ||
            (sourcePaxTypeFare->tcrRuleTariff() != targetPaxTypeFare->tcrRuleTariff())))
  {
    displayFailureReason(diag,
                         pu,
                         getErrorID(FAILED_SAME_RULE_REQUIRED, puType),
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  else if (checkTariff &&
           (sourcePaxTypeFare->tcrRuleTariff() != targetPaxTypeFare->tcrRuleTariff()))
  {
    displayFailureReason(diag,
                         pu,
                         getErrorID(FAILED_SAME_TARIFF_REQUIRED, puType),
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  else if (checkClass && (sourcePaxTypeFare->fareClass() != targetPaxTypeFare->fareClass()))
  {
    displayFailureReason(diag,
                         pu,
                         getErrorID(FAILED_SAME_FARECLASS_REQUIRED, puType),
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  else if (checkType && (sourcePaxTypeFare->fcaFareType() != targetPaxTypeFare->fcaFareType()))
  {
    displayFailureReason(diag,
                         pu,
                         getErrorID(FAILED_SAME_FARETYPE_REQUIRED, puType),
                         failureReason,
                         sourceFareUsage,
                         targetFareUsage);
    retHandled = true;
  }
  return retHandled;
}

// ----------------------------------------------------------------------------
// Y - Permitted -> validate in Combinations::processMajorSubCat()
// R - Permitted with Restrictions -> Process 104
// M - End on End Combinations are required -> Process 104

// SITA
// A - Permitted/Side Trip Not Permitted
// B - Not Permitted/Side Trip Not Permitted
// C - Permitted with Restrictions/Side Trip Not Permitted
// D - Required/Side Trip Not Permitted

// ATPCO
// N - Not Permitted
// M - End on End Combinations are required on 1FC
//------------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::analyzeOneWay(FarePath& farePath, DiagCollector& diag)
{
  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  CombinabilityRuleInfo* pCat10 = nullptr;
  bool isLocationSwapped = false;

  for (; puIt != puItEnd; ++puIt)
  {
    PricingUnit& pu = *(*puIt);

    std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
    std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
    for (; fuIt != fuItEnd; ++fuIt)
    {
      FareUsage& fareUsage = **fuIt;
      isLocationSwapped = false;
      pCat10 = nullptr;

      if (fareUsage.rec2Cat10() == nullptr)
      {
        pCat10 = RuleUtil::getCombinabilityRuleInfo(
            *_trx, *(fareUsage.paxTypeFare()), isLocationSwapped);
        fareUsage.rec2Cat10() = pCat10;
      }
      else
      {
        pCat10 = fareUsage.rec2Cat10();
      }

      if (pCat10)
      {
        CombinabilityValidationResult ret =
            checkOWIndicatorOnFarePath(pu, fareUsage, *pCat10, diag);
        if (ret != CVR_PASSED)
        {
          return ret; // no need to continue
        }
      }
      else
      {
        if (isMirrorImage(pu))
        {
          // MIRROR IMAGE RT: PASSED BY SYSTEM ASSUMPTION
          break;
        }
        else if (pu.puType() == PricingUnit::Type::ONEWAY && farePath.pricingUnit().size() == 1)
        {
          // only one OW PU and no Cat-10 restriction
          break;
        }
        else
        {
          if (diag.isActive())
            diag << " FAILED COMBINATION - NO RECORD 2 CAT 10" << std::endl;
          return CVR_NO_REC2CAT10;
        }
      }
    }
  }

  return CVR_PASSED;
}
// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkOWIndicatorOnFarePath(PricingUnit& pu,
                                                    FareUsage& fareUsage,
                                                    const CombinabilityRuleInfo& pCat10,
                                                    DiagCollector& diag)
{
  if (pu.puType() == PricingUnit::Type::ONEWAY)
  {
    if (UNLIKELY((pCat10.eoeInd() == Combinations::REQUIRED ||
         pCat10.eoeInd() == Combinations::EOE_REQUIRED_SIDE_TRIP_NOT_PERMITTED) &&
        pu.noPUToEOE()))
    {
      if (diag.isActive())
      {
        diag << " FAILED RECORD 2 SCOREBOARD CHECK - " << fareUsage.paxTypeFare()->fareClass()
             << " FARE" << std::endl << "        EOE REQUIRED" << std::endl;
        if (!checkSideTripPermitted(pCat10.eoeInd(), pu, fareUsage))
          diag << "        SIDETRIP IS NOT PERMITTED" << std::endl;
      }
      return CVR_EOE_REQUIRED;
    }
  }

  if ((pCat10.eoeInd() == Combinations::NOT_PERMITTED ||
       pCat10.eoeInd() == Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED) &&
      !pu.noPUToEOE())
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " FAILED RECORD 2 SCOREBOARD CHECK - " << fareUsage.paxTypeFare()->fareClass()
           << " FARE" << std::endl << "        EOE IS NOT PERMITTED" << std::endl;
    }
    return CVR_EOE_NOT_PERMITTED;
  }

  if (!checkSideTripPermitted(pCat10.eoeInd(), pu, fareUsage))
  {
    if (UNLIKELY(diag.isActive()))
    {
      diag << " FAILED RECORD 2 SCOREBOARD CHECK - " << fareUsage.paxTypeFare()->fareClass()
           << " FARE" << std::endl << "        SIDETRIP IS NOT PERMITTED" << std::endl;
    }
    return CVR_UNSPECIFIED_FAILURE;
  }
  return CVR_PASSED;
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::analyzeOpenJaw(PricingUnit& pu, DiagCollector& diag)
{

  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;
    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret = checkOJIndicatorToValidate(pu, fareUsage, *pCat10, diag);
    if (UNLIKELY(ret == CVR_UNSPECIFIED_FAILURE))
      return ret; // no need to continue
  }
  return CVR_PASSED;
}
// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkOJIndicatorToValidate(PricingUnit& pu,
                                                    FareUsage& fareUsage,
                                                    const CombinabilityRuleInfo& pCat10,
                                                    DiagCollector& diag)
{

  // N - Not Permitted -> support in invalidate()
  // Y - Permitted -> validate in Combinations::processMajorSubCat()
  // R - Permitted with Restrictions -> Process 101

  if (UNLIKELY(pu.puSubType() != PricingUnit::DEST_OPENJAW &&
      pCat10.sojorigIndestInd() == Combinations::DEST_OPEN_JAW_REQ))
  {
    displayDiag(diag, pu, FAILED_DESTINATION_OPEN_JAW_REQUIRED, &fareUsage, nullptr);
    LOG4CXX_INFO(logger, __LINE__ << ", analyzeOpenJaw() return fail");
    return CVR_UNSPECIFIED_FAILURE;
  }
  else if (UNLIKELY(pu.puSubType() != PricingUnit::ORIG_OPENJAW &&
           pCat10.sojorigIndestInd() == Combinations::ORIGIN_OPEN_JAW_REQ))
  {
    displayDiag(diag, pu, FAILED_ORIGIN_OPEN_JAW_REQUIRED, &fareUsage, nullptr);
    LOG4CXX_INFO(logger, __LINE__ << ", analyzeOpenJaw() return fail");
    return CVR_UNSPECIFIED_FAILURE;
  }

  // S - Permitted OJ comprises of no more than 2 international FCs
  //     and open segment is within one country
  // T - Permitted with Restrictions for OJ comprises of no more than
  //     2 international FCs and open segment is within one country
  //     -> Process 101

  if (pCat10.sojInd() == Combinations::PERMITTED_S ||
      pCat10.sojInd() == Combinations::RESTRICTIONS_T)
  {
    if (!pu.sameNationOJ())
    {
      displayDiag(diag, pu, FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED, &fareUsage, nullptr);
      LOG4CXX_INFO(logger, __LINE__ << ", analyzeOpenJaw() return fail");
      return CVR_UNSPECIFIED_FAILURE;
    }
  }

  // SITA SOJ
  // U - Permitted/Different Country
  // V - Restricted/Different Country
  // W - Permitted/Origin OJ must be in the same Country
  // X - Restricted/Origin OJ must be in the same Country

  // SITA DOJ
  // U - Permitted/Different Country
  // V - Restricted/Different Country
  // W - Permitted/Origin OJ must be in the same Country
  // X - Restricted/Origin OJ must be in the same Country

  if (pCat10.sojInd() == Combinations::OPEN_JAW_PERMITTED_DIFFERENT_COUNTRY ||
      pCat10.sojInd() == Combinations::OPEN_JAW_RESTRICTED_DIFFERENT_COUNTRY ||
      pCat10.dojInd() == Combinations::OPEN_JAW_PERMITTED_DIFFERENT_COUNTRY ||
      pCat10.dojInd() == Combinations::OPEN_JAW_RESTRICTED_DIFFERENT_COUNTRY)
  {
    if (pu.sameNationOJ())
    {
      displayDiag(diag, pu, FAILED_OJ_DIFF_COUNTRY_REQUIRED, &fareUsage, nullptr);
      LOG4CXX_INFO(logger, __LINE__ << ", analyzeOpenJaw() return fail");
      return CVR_UNSPECIFIED_FAILURE;
    }
  }
  else if (pCat10.sojInd() == Combinations::OPEN_JAW_PERMITTED_ORIGIN_SAME_COUNTRY ||
           pCat10.sojInd() == Combinations::OPEN_JAW_RESTRICTED_ORIGIN_SAME_COUNTRY ||
           pCat10.dojInd() == Combinations::OPEN_JAW_PERMITTED_ORIGIN_SAME_COUNTRY ||
           pCat10.dojInd() == Combinations::OPEN_JAW_RESTRICTED_ORIGIN_SAME_COUNTRY)
  {
    if (UNLIKELY(pu.puSubType() == PricingUnit::ORIG_OPENJAW && !pu.sameNationOJ()))
    {
      displayDiag(diag, pu, FAILED_OJ_SAME_COUNTRY_REQUIRED, &fareUsage, nullptr);
      LOG4CXX_INFO(logger, __LINE__ << ", analyzeOpenJaw() return fail");
      return CVR_UNSPECIFIED_FAILURE;
    }
  }

  return CVR_PASSED;
}

// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::analyzeRoundTrip(PricingUnit& pu, DiagCollector& diag)
{
  bool mirrorImage = isMirrorImage(pu);
  std::vector<FareUsage*>::iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::iterator fuItEnd = pu.fareUsage().end();
  for (; fuIt != fuItEnd; ++fuIt)
  {
    FareUsage& fareUsage = **fuIt;

    if (UNLIKELY(_trx->getTrxType() == PricingTrx::MIP_TRX && fareUsage.paxTypeFare()->isDummyFare()))
    {
      continue;
    }

    const CombinabilityRuleInfo* pCat10 = fareUsage.rec2Cat10();
    CombinabilityValidationResult ret =
        checkRTIndicatorToValidate(pu, fareUsage, mirrorImage, *pCat10, diag);
    if (UNLIKELY(ret == CVR_UNSPECIFIED_FAILURE))
      return ret; // no need to continue
  }

  return CVR_PASSED;
}
// ----------------------------------------------------------------------------
CombinabilityValidationResult
CombinabilityScoreboard::checkRTIndicatorToValidate(const PricingUnit& pu,
                                                    FareUsage& fareUsage,
                                                    bool mirrorImage,
                                                    const CombinabilityRuleInfo& pCat10,
                                                    DiagCollector& diag)
{
  // N - Not Permitted -> support in invalidate()
  // Y - Permitted -> validate in Combinations::processMajorSubCat()
  // R - Permitted with Restrictions -> Process 102

  // V - RT are Permitted but Mirror Image are Not Permitted
  // X - RT are Not Permitted and Mirror Image are Not Permitted
  // W - RT are Permitted with Restrictions but Mirror Image are Not Permitted

  if (mirrorImage)
  {
    if (UNLIKELY(pCat10.ct2Ind() == Combinations::ROUND_TRIP_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED ||
        pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED ||
        pCat10.ct2Ind() == Combinations::ROUND_TRIP_RESTRICTIONS_MIRROR_IMAGE_NOT_PERMITTED))
    {
      displayDiag(diag, pu, FAILED_MIRROR_IMAGE_NOT_PERMITTED, &fareUsage, nullptr);
      LOG4CXX_INFO(logger, __LINE__ << ", analyzeRoundTrip() return false");
      return CVR_UNSPECIFIED_FAILURE;
    }
  }

  if (UNLIKELY(pCat10.ct2Ind() == Combinations::ROUND_TRIP_NOT_PERMITTED_MIRROR_IMAGE_NOT_PERMITTED))
  {
    displayDiag(diag, pu, FAILED_ROUND_TRIP_NOT_PERMITTED, &fareUsage, nullptr);
    LOG4CXX_INFO(logger, __LINE__ << ", analyzeRoundTrip() return false");
    return CVR_UNSPECIFIED_FAILURE;
  }

  return CVR_PASSED;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void CombinabilityScoreboard::displayDiag
//
// Description: Display Diagnostic request
//
// </PRE>
// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::outputDiag(DiagCollector& diag,
                                    const char* prefix,
                                    const char* suffix,
                                    const FareUsage* sourceFareUsage,
                                    const FareUsage* targetFareUsage) const
{
  diag << prefix;

  if (sourceFareUsage)
  {
    diag << sourceFareUsage->paxTypeFare()->fareClass() << "-"
         << sourceFareUsage->paxTypeFare()->ruleNumber();

    if (targetFareUsage && sourceFareUsage != targetFareUsage)
    {
      diag << " - " << targetFareUsage->paxTypeFare()->fareClass();
    }
  }
  if (suffix)
  {
    diag << suffix;
  }
  diag << std::endl;
}

// ----------------------------------------------------------------------------
void
CombinabilityScoreboard::displayDiag(DiagCollector& diag,
                                     const PricingUnit& pu,
                                     DiagnosticID errNum,
                                     const FareUsage* sourceFareUsage,
                                     const FareUsage* targetFareUsage) const
{
  DiagCollectorGuard dcg1(diag, &pu, Diagnostic605);

  if (UNLIKELY(diag.isActive() && !DiagnosticUtil::filter(*_trx, pu)))
  {
    switch (errNum)
    {
    case INVALID_DIAGNOSTIC_TOO_FEW_FARES:
      diag << " INVALID DIAGNOSTIC - TOO FEW FARES" << std::endl;
      break;
    case PASSED_VALIDATION:
      diag << " PASSED RECORD 2 SCOREBOARD CHECK" << std::endl;
      break;
    case FAILED_UNSPECIFIED:
      diag << " FAILED COMBINATION" << std::endl;
      break;
    case FAILED_REC_2_CAT_10_NOT_APPLICABLE:
      diag << " FAILED COMBINATION - RECORD 2 CAT 10 NOT APPLICABLE" << std::endl;
      break;
    case FAILED_NO_REC_2_CAT_10:
      diag << " FAILED COMBINATION - NO RECORD 2 CAT 10" << std::endl;
      break;
    case PASSED_SYSTEM_ASSUMPTION:
      diag << " PASSED COMBINATION - SYSTEM ASSUMPTION" << std::endl;
      break;

    case FAILED_REC2_SCOREBOARD:
      outputDiag(diag, " FAILED RECORD 2 SCOREBOARD CHECK - ", " FARE", sourceFareUsage);
      break;

    case FAILED_ROUND_TRIP_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - ROUND TRIP NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_CARRIER_REQUIRED_FOR_RT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME CARRIER REQ FOR RT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_RULE_REQUIRED_FOR_RT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME RULE REQ FOR RT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_TARIFF_REQUIRED_FOR_RT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME TARIFF REQ FOR RT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARECLASS_REQUIRED_FOR_RT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARECLASS REQ FOR RT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARETYPE_REQUIRED_FOR_RT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARETYPE REQ FOR RT", sourceFareUsage, targetFareUsage);
      break;

    case FAILED_CIRCLE_TRIP_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - CIRCLE TRIP NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_CARRIER_REQUIRED_FOR_CT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME CARRIER REQ FOR CT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_RULE_REQUIRED_FOR_CT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME RULE REQ FOR CT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_TARIFF_REQUIRED_FOR_CT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME TARIFF REQ FOR CT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARECLASS_REQUIRED_FOR_CT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARECLASS REQ FOR CT", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARETYPE_REQUIRED_FOR_CT:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARETYPE REQ FOR CT", sourceFareUsage, targetFareUsage);
      break;

    case FAILED_SINGLE_OPEN_JAW_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - SINGLE OJ NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_DOUBLE_OPEN_JAW_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - DOUBLE OJ NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW:
      outputDiag(
          diag, " FAILED FARE ", " - SAME CARRIER REQ FOR OJ", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW:
      outputDiag(
          diag, " FAILED FARE ", " - SAME RULE REQ FOR OJ", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW:
      outputDiag(
          diag, " FAILED FARE ", " - SAME TARIFF REQ FOR OJ", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARECLASS REQ FOR OJ", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW:
      outputDiag(
          diag, " FAILED FARE ", " - SAME FARETYPE REQ FOR OJ", sourceFareUsage, targetFareUsage);
      break;

    case FAILED_END_ON_END_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - END ON END NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_END_ON_END_SAME_CARRIER_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - SAME CARRIER REQ FOR END ON END",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_END_ON_END_SAME_RULE_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - SAME RULE REQ FOR END ON END",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_END_ON_END_SAME_TARIFF_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - SAME TARIFF REQ FOR END ON END",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_END_ON_END_SAME_FARECLASS_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - SAME FARECLASS REQ FOR END ON END",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_END_ON_END_SAME_FARETYPE_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - SAME FARETYPE REQ FOR END ON END",
                 sourceFareUsage,
                 targetFareUsage);
      break;

    case FAILED_DESTINATION_OPEN_JAW_REQUIRED:
      outputDiag(
          diag, " FAILED FARE ", " - DEST OPEN JAW REQUIRED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_ORIGIN_OPEN_JAW_REQUIRED:
      outputDiag(
          diag, " FAILED FARE ", " - ORIGIN OPEN JAW REQUIRED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_2_MAX_INTL_FARES_SAME_COUNTRY_REQUIRED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - 2 MAX INTL FARES SAME COUNTRY REQUIRED",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_OJ_DIFF_COUNTRY_REQUIRED:
      outputDiag(
          diag, " FAILED FARE ", " - OJ DIFF COUNTRY REQUIRED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_OJ_SAME_COUNTRY_REQUIRED:
      outputDiag(
          diag, " FAILED FARE ", " - OJ SAME COUNTRY REQUIRED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_MIRROR_IMAGE_NOT_PERMITTED:
      outputDiag(diag,
                 " FAILED FARE ",
                 " - MIRROR IMAGE IS NOT PERMITTED",
                 sourceFareUsage,
                 targetFareUsage);
      break;
    case FAILED_SIDE_TRIP_NOT_PERMITTED:
      outputDiag(
          diag, " FAILED FARE ", " - SIDE TRIP IS NOT PERMITTED", sourceFareUsage, targetFareUsage);
      break;
    case FAILED_REQUIRED_EOE_WITH_OTHER_PU:
      outputDiag(
          diag, " FAILED FARE ", " - REQUIRED EOE WITH OTHER PU", sourceFareUsage, targetFareUsage);
      break;
    }
  }
}

// ----------------------------------------------------------------------------
bool
CombinabilityScoreboard::isMirrorImage(PricingUnit& pu)
{
  bool mirrorImage = false;
  if (pu.puType() == PricingUnit::Type::ROUNDTRIP)
  {
    const Fare& fare1 = *pu.fareUsage()[0]->paxTypeFare()->fare();
    const Fare& fare2 = *pu.fareUsage()[1]->paxTypeFare()->fare();

    mirrorImage = (fare1.fareClass() == fare2.fareClass() && fare1.carrier() == fare2.carrier() &&
                   fare1.ruleNumber() == fare2.ruleNumber() &&
                   fare1.tcrRuleTariff() == fare2.tcrRuleTariff() &&
                   fare1.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
                   fare2.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
  }
  return mirrorImage;
}

// ----------------------------------------------------------------------------
CombinabilityScoreboard::DiagnosticID
CombinabilityScoreboard::getErrorID(DiagnosticIDGroup idGroup, PricingUnit::Type puType) const
{
  DiagnosticID retID = FAILED_UNSPECIFIED;
  switch (idGroup)
  {
  case FAILED_SAME_CARRIER_REQUIRED:
  {
    switch (puType)
    {
    case PricingUnit::Type::OPENJAW:
    {
      retID = FAILED_SAME_CARRIER_REQUIRED_FOR_OPEN_JAW;
      break;
    }
    case PricingUnit::Type::ROUNDTRIP:
    {
      retID = FAILED_SAME_CARRIER_REQUIRED_FOR_RT;
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retID = FAILED_SAME_CARRIER_REQUIRED_FOR_CT;
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retID = FAILED_END_ON_END_SAME_CARRIER_REQUIRED;
      break;
    }
    default:
    {
      break;
    }
    }
    break;
  }
  case FAILED_SAME_RULE_REQUIRED:
  {
    switch (puType)
    {
    case PricingUnit::Type::OPENJAW:
    {
      retID = FAILED_SAME_RULE_REQUIRED_FOR_OPEN_JAW;
      break;
    }
    case PricingUnit::Type::ROUNDTRIP:
    {
      return FAILED_SAME_RULE_REQUIRED_FOR_RT;
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retID = FAILED_SAME_RULE_REQUIRED_FOR_CT;
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retID = FAILED_END_ON_END_SAME_RULE_REQUIRED;
      break;
    }
    default:
    {
      break;
    }
    }
    break;
  }
  case FAILED_SAME_TARIFF_REQUIRED:
  {
    switch (puType)
    {
    case PricingUnit::Type::OPENJAW:
    {
      retID = FAILED_SAME_TARIFF_REQUIRED_FOR_OPEN_JAW;
      break;
    }
    case PricingUnit::Type::ROUNDTRIP:
    {
      retID = FAILED_SAME_TARIFF_REQUIRED_FOR_RT;
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retID = FAILED_SAME_TARIFF_REQUIRED_FOR_CT;
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retID = FAILED_END_ON_END_SAME_TARIFF_REQUIRED;
      break;
    }
    default:
    {
      break;
    }
    }
    break;
  }
  case FAILED_SAME_FARECLASS_REQUIRED:
  {
    switch (puType)
    {
    case PricingUnit::Type::OPENJAW:
    {
      retID = FAILED_SAME_FARECLASS_REQUIRED_FOR_OPEN_JAW;
      break;
    }
    case PricingUnit::Type::ROUNDTRIP:
    {
      retID = FAILED_SAME_FARECLASS_REQUIRED_FOR_RT;
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retID = FAILED_SAME_FARECLASS_REQUIRED_FOR_CT;
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retID = FAILED_END_ON_END_SAME_FARECLASS_REQUIRED;
      break;
    }
    default:
    {
      break;
    }
    }
    break;
  }
  case FAILED_SAME_FARETYPE_REQUIRED:
  {
    switch (puType)
    {
    case PricingUnit::Type::OPENJAW:
    {
      retID = FAILED_SAME_FARETYPE_REQUIRED_FOR_OPEN_JAW;
      break;
    }
    case PricingUnit::Type::ROUNDTRIP:
    {
      retID = FAILED_SAME_FARETYPE_REQUIRED_FOR_RT;
      break;
    }
    case PricingUnit::Type::CIRCLETRIP:
    {
      retID = FAILED_SAME_FARETYPE_REQUIRED_FOR_CT;
      break;
    }
    case PricingUnit::Type::ONEWAY:
    {
      retID = FAILED_END_ON_END_SAME_FARETYPE_REQUIRED;
      break;
    }
    default:
    {
      break;
    }
    }
    break;
  }
  }
  return retID;
}

bool
CombinabilityScoreboard::checkSideTripPermitted(Indicator eoeInd, PricingUnit& pu, FareUsage& fu)
    const
{
  if ((eoeInd == Combinations::EOE_PERMITTED_SIDE_TRIP_NOT_PERMITTED || //'A'
       eoeInd == Combinations::EOE_NOT_PERMITTED_SIDE_TRIP_NOT_PERMITTED || //'B'
       eoeInd == Combinations::EOE_RESTRICTIONS_SIDE_TRIP_NOT_PERMITTED || //'C'
       eoeInd == Combinations::EOE_REQUIRED_SIDE_TRIP_NOT_PERMITTED) //'D'
      &&
      (pu.isSideTripPU() || fu.hasSideTrip()))

    return false;
  else
    return true;
}
}
