#include "Rules/FareMarketRuleController.h"
#include "Common/FallbackUtil.h"
#include "Common/MetricsUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/FareMarketDataAccess.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleUtil.h"
#include "Rules/Stopovers.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Util/BranchPrediction.h"

using namespace std;
namespace tse
{
FALLBACK_DECL(fallbackGfrR2Optimization);
//----------------------------------------------------------------------------
// validate() what the outside world calls...
//----------------------------------------------------------------------------
bool
FareMarketRuleController::validate(PricingTrx& trx, FareMarket& fareMarket, Itin& itin)
{

#ifdef DEBUG_PERF
  TSELatencyData metrics(trx, "RULE VALIDATION PAX FARE TYPE");
#endif

  MetricsUtil::MetricsFactor factor = MetricsUtil::RC_FC_NORMAL_VALIDATION;

  switch (_categoryPhase)
  {
  case PreValidation:
    factor = MetricsUtil::RC_FC_PRE_VALIDATION;
    break;
  case NormalValidation:
    factor = MetricsUtil::RC_FC_NORMAL_VALIDATION;
    break;
  case PURuleValidation:
  case PURuleValidationIS:
  case FPRuleValidation:
  case PURuleValidationISALT:
  case FPRuleValidationISALT:
    factor = MetricsUtil::RC_FC_RE_VALIDATION;
    break;
  //        case ReValidation:
  //          factor = MetricsUtil::RC_FC_RE_VALIDATION;
  //          break;
  case ShoppingComponentValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_VALIDATION;
    break;
  case ShoppingAcrossStopOverComponentValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ASO_VALIDATION;
    break;
  case ShoppingComponentWithFlightsValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_WITH_FLIGHTS_VALIDATION;
    break;
  case ShoppingASOComponentWithFlightsValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION;
    break;
  case DynamicValidation:
    factor = MetricsUtil::RC_FC_DYNAMIC_VALIDATION;
    break;
  case ShoppingComponentValidateQualifiedCat4:
    factor = MetricsUtil::RC_FC_SHOPPING_VALIDATE_IF_CAT4;
    break;
  case ShoppingItinBasedValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION;
    break;
  case ShoppingAltDateItinBasedValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION_ALT;
    break;
  case FareDisplayValidation:
    factor = MetricsUtil::RC_FC_FARE_DISPLAY_VALIDATION;
    break;
  case RuleDisplayValidation:
    factor = MetricsUtil::RC_FC_RULE_DISPLAY_VALIDATION;
    break;
  default:
    break;
  }

#ifdef DEBUG_PERF
  TSELatencyData phaseMetrics(trx, factor);
#endif

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  bool diagFilterNeeded = (trx.diagnostic().isActive() &&
                           ((diagType == Diagnostic500) ||
                            ((diagType >= Diagnostic301) && (diagType <= Diagnostic350))));

  DCFactory* factory = nullptr;
  DiagCollector* dcPtr = nullptr;

  if (UNLIKELY(diagFilterNeeded))
  {
    factory = DCFactory::instance();
    dcPtr = factory->create(trx);
  }

  const std::vector<PaxTypeFare*>& paxTypeFares = fareMarket.allPaxTypeFare();
  std::vector<PaxTypeFare*>::const_iterator iter = paxTypeFares.begin();
  std::vector<PaxTypeFare*>::const_iterator iterEnd = paxTypeFares.end();

  for (; iter != iterEnd; iter++)
  {
    PaxTypeFare* paxTypeFare = *iter;

    if (UNLIKELY(paxTypeFare == nullptr))
      continue;

    if (UNLIKELY((_categoryPhase == NormalValidation || _categoryPhase == FBRBaseFarePrevalidation) && !trx.matchFareRetrievalDate(*paxTypeFare)))
      continue;

    //        if(fareMarket.direction() != FMDirection::UNKNOWN &&
    if (fareMarket.direction() == FMDirection::OUTBOUND &&
        factor == MetricsUtil::RC_FC_NORMAL_VALIDATION)
    {
      //            if((fareMarket.direction() == FMDirection::OUTBOUND &&
      //                paxTypeFare->directionality() == TO) ||
      //               (fareMarket.direction() == FMDirection::INBOUND &&
      //                paxTypeFare->directionality() == FROM))
      if (paxTypeFare->directionality() == TO)
      {
        paxTypeFare->puRuleValNeeded() = true;
        continue;
      }
    }

    if (UNLIKELY(diagFilterNeeded))
    {
      if (!trx.diagnostic().shouldDisplay(*paxTypeFare))
      {
        dcPtr->rootDiag()->deActivate();
      }
    }

    if (UNLIKELY(paxTypeFare->isValid() && !validate(trx, itin, *paxTypeFare)))
    {
      if (UNLIKELY(diagFilterNeeded))
      {
        dcPtr->rootDiag()->activate();
      }
      return false;
    }

    if (UNLIKELY(diagFilterNeeded))
    {
      dcPtr->rootDiag()->activate();
    }
  }

  if (UNLIKELY(diagFilterNeeded))
  {
    dcPtr->rootDiag()->activate();
  }
  return true;
}

//----------------------------------------------------------------------------
// validate(): what the outside world calls...
//----------------------------------------------------------------------------

bool
FareMarketRuleController::validate(PricingTrx& trx, Itin& itin, PaxTypeFare& paxTypeFare)
{
#ifdef DEBUG_PERF
  TSELatencyData metrics(trx, "RULE VALIDATION PAX FARE TYPE");
#endif

  if (_categorySequence.empty())
  {
    return true;
  }

#ifdef DEBUG_PERF
  MetricsUtil::MetricsFactor factor = MetricsUtil::RC_FC_NORMAL_VALIDATION;

  switch (_categoryPhase)
  {
  case PreValidation:
    factor = MetricsUtil::RC_FC_PRE_VALIDATION;
    break;
  case NormalValidation:
    factor = MetricsUtil::RC_FC_NORMAL_VALIDATION;
    break;
  case ShoppingComponentValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_VALIDATION;
    break;
  case ShoppingAcrossStopOverComponentValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ASO_VALIDATION;
    break;
  case ShoppingComponentWithFlightsValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_WITH_FLIGHTS_VALIDATION;
    break;
  case ShoppingASOComponentWithFlightsValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ASO_WITH_FLIGHTS_VALIDATION;
    break;
  case DynamicValidation:
    factor = MetricsUtil::RC_FC_DYNAMIC_VALIDATION;
    break;
  case ShoppingComponentValidateQualifiedCat4:
    factor = MetricsUtil::RC_FC_SHOPPING_VALIDATE_IF_CAT4;
    break;
  case ShoppingItinBasedValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION;
    break;
  case ShoppingAltDateItinBasedValidation:
    factor = MetricsUtil::RC_FC_SHOPPING_ITIN_BASED_VALIDATION_ALT;
    break;
  case FareDisplayValidation:
    factor = MetricsUtil::RC_FC_FARE_DISPLAY_VALIDATION;
    break;
  case RuleDisplayValidation:
    factor = MetricsUtil::RC_FC_RULE_DISPLAY_VALIDATION;
    break;
  default:
    break;
  }

  TSELatencyData phaseMetrics(trx, factor);
#endif

  bool ret = true;

  // TODO  ... move this filter somewhere common for all stage ... saranya

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  bool diagFilterNeeded =
      trx.diagnostic().isActive() && ((diagType == Diagnostic500) || (diagType == Diagnostic550) ||
                                      ((diagType >= Diagnostic301) && (diagType <= Diagnostic350)));

  bool diagNeededForPaxTypeFare = diagFilterNeeded && trx.diagnostic().shouldDisplay(paxTypeFare);

  DCFactory* factory = nullptr;
  DiagCollector* dcPtr = nullptr;

  if (UNLIKELY(diagFilterNeeded && !diagNeededForPaxTypeFare))
  {
    factory = DCFactory::instance();
    dcPtr = factory->create(trx);

    dcPtr->rootDiag()->deActivate();
  }

  // go throuth the Categories to validate rules

  if (!processCategorySequence(trx, itin, paxTypeFare))
  {
    ret = false;
  }

  if (UNLIKELY(diagFilterNeeded && !diagNeededForPaxTypeFare))
  {
    dcPtr->rootDiag()->activate();
  }

  return ret;
}

bool
FareMarketRuleController::processCategorySequence(PricingTrx& trx,
                                                  Itin& itin,
                                                  PaxTypeFare& paxTypeFare)
{ // lint !e578
  FareMarketDataAccess da(trx, &itin, paxTypeFare);

  bool rtn = processCategorySequenceCommon(trx, da, _categorySequence);

  if (!rtn) return false;

  if (UNLIKELY(paxTypeFare.isValid() && !da.mixedRtnResultCategroies().empty()))
  {
    _highLevelLoopValidatingCxrs = true;
    rtn = reValidateMixedRtnPTF(da, paxTypeFare, paxTypeFare.validatingCarriers());
    _highLevelLoopValidatingCxrs = false;
  }
  return rtn;
}

Record3ReturnTypes
FareMarketRuleController::validateBaseFare(uint16_t category,
                                           const FareByRuleItemInfo* fbrItemInfo,
                                           bool& checkFare,
                                           PaxTypeFare* fbrFare,
                                           RuleControllerDataAccess& da)
{
  bool checkFBRBaseFare = false;

  RuleUtil::determineRuleChecks(category, *fbrItemInfo, checkFare, checkFBRBaseFare);

  // Notify Cat 15 rule validation to skip Cat 15 security check on base fare
  // when Cat 25 has Cat 35 rule.
  bool skipCat15Security = false;
  Indicator fcaDisplayCatType = fbrFare->fcaDisplayCatType();

  da.retrieveBaseFare(true);
  PaxTypeFare& currentBaseFare(da.paxTypeFare());
  const VendorCode& vendor(currentBaseFare.vendor());

  if (fbrFare->isNegotiated() ||
      (_categoryPhase == PreValidation && (fcaDisplayCatType == RuleConst::SELLING_FARE ||
                                           fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE ||
                                           fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE_UPD)))
  {
    if (RuleUtil::isVendorPCC(fbrFare->vendor(), da.trx()) &&
        (vendor.equalToConst("ATP") || vendor.equalToConst("SITA") || vendor.equalToConst("SMFA") || vendor.equalToConst("SMFC")) &&
        !(currentBaseFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
          currentBaseFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
          currentBaseFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD))
    {
      // For Cat 25/35 DFF and base fare non-Cat 35 ATP/SITA/SMFA/SMFC,
      // Do not skip Cat 15 security validation of this base fare
      skipCat15Security = false;
    }
    else
    {
      skipCat15Security = true;
    }
  } // end of Cat 25/35

  bool isCat25PrivateBasePublic = false;
  if (category == RuleConst::SALE_RESTRICTIONS_RULE && !skipCat15Security && checkFBRBaseFare &&
      currentBaseFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
      fbrFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    isCat25PrivateBasePublic = true;
  // Do not reuse Cat 15 result of base fare if Cat 25 fare is private fare and base fare is public
  // fare.

  if (checkFBRBaseFare && (_categoryPhase != ShoppingComponentWithFlightsValidation) &&
      !isCat25PrivateBasePublic)
  {
    // Check it only when Footnotes/FareRule/ GeneralRule for the BaseFare should be processed
    if (currentBaseFare.isCategoryProcessed(category))
    {
      if (currentBaseFare.isCategoryValid(category)) // pass or softpass => valid
      {
        if (!_fallbackCat25baseFarePrevalidation)
          fbrFare->setCmdPrcFailedFlag(category, currentBaseFare.isFailedCmdPrc(category));

        return PASS;
      }
      else
      {
        bool doNotReuseResult = false;

        // Do not reuse Cat 15 result of base fare for Cat 25 type L
        if (category == 15 && skipCat15Security)
          doNotReuseResult = true;

        if (!doNotReuseResult)
        {
          copyFlexFaresValidationStatus(da.trx(), *fbrFare, currentBaseFare, category);
          return FAIL;
        }
      }
    }
  }
  // Need to process this category for the BaseFare when only a Footnote is requested.
  // We always need to check the FootNote rule for baseFares

  // Create Footnotes Vector for the base fare
  da.footNoteTbl().clear();
  getFootnotes(currentBaseFare, da.footNoteTbl());
  da.paxTypeFare().cat25Fare() = fbrFare;
  currentBaseFare.cat25Fare() = fbrFare;

  bool savedBaseCat15SoftPass = currentBaseFare.cat15SoftPass();
  if (category == RuleConst::SALE_RESTRICTIONS_RULE)
    currentBaseFare.setCat15SoftPass(false);

  Record3ReturnTypes retResultOfRule;
  retResultOfRule =
      processRules(da.trx(), da, category, checkFBRBaseFare, skipCat15Security, checkFBRBaseFare);

  currentBaseFare.cat25Fare() = nullptr;
  da.paxTypeFare().cat25Fare() = nullptr;

  if (category == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    if (retResultOfRule == FAIL)
      fbrFare->setCat15SoftPass(currentBaseFare.cat15SoftPass());
    currentBaseFare.setCat15SoftPass(savedBaseCat15SoftPass);
  }

  bool isAltDateTrx = (da.trx().isAltDates() && da.trx().getTrxType() == PricingTrx::IS_TRX);

  if (checkFBRBaseFare && !isAltDateTrx &&
      !isCat25PrivateBasePublic) // Do this only when Footnotes/FareRule/ GeneralRule were processed
  {
    bool skipBaseFareModification = false;
    skipBaseFareModification = da.trx().isValidatingCxrGsaApplicable() &&
                              (RuleConst::SALE_RESTRICTIONS_RULE == category) &&
                              (da.trx().getTrxType() == PricingTrx::MIP_TRX);

    // using currentBaseFare is better than keep using da.paxTypeFare()
    if (!skipBaseFareModification &&
        (!TrxUtil::isFVOSurchargesEnabled() || RuleConst::SURCHARGE_RULE != category))
      currentBaseFare.setCategoryProcessed(category);

    if (!passCategory(category, retResultOfRule, currentBaseFare))
    {
      if (LIKELY((_categoryPhase == PreValidation) || (_categoryPhase == NormalValidation) ||
          (_categoryPhase == FBRBaseFarePrevalidation) ||
          (_categoryPhase == FareDisplayValidation) || (_categoryPhase == RuleDisplayValidation) ||
          (_categoryPhase == FCORuleValidation) || (_categoryPhase == FCORuleValidationMIPALT) ||
          (_categoryPhase == ShoppingComponentValidation) ||
          (_categoryPhase == ShoppingAcrossStopOverComponentValidation) ||
          (_categoryPhase == ShoppingComponentValidateQualifiedCat4) ||
          (_categoryPhase == ShoppingComponentWithFlightsValidation) ||
          (_categoryPhase == ShoppingASOComponentWithFlightsValidation)))
      {
        if (UNLIKELY(currentBaseFare.isCmdPricing() &&
            currentBaseFare.validForCmdPricing(da.trx().fxCnException())))
          currentBaseFare.setCmdPrcFailedFlag(category);
        else
          currentBaseFare.setCategoryValid(category, false);
      }
    }
  }
  // When checkFBRBaseFare is false, it means that check basefare
  // for only footnote, we can not override footnote at command pricing

  // Restore Footnotes vector for the original FBR fare
  da.footNoteTbl().clear();
  getFootnotes(*fbrFare, da.footNoteTbl());

  if (retResultOfRule == FAIL)
    copyFlexFaresValidationStatus(da.trx(), *fbrFare, currentBaseFare, category);

  return retResultOfRule;
}

Record3ReturnTypes
FareMarketRuleController::doCategoryPostProcessing(PricingTrx& trx,
                                                   RuleControllerDataAccess& da,
                                                   const uint16_t category,
                                                   RuleProcessingData& rpData,
                                                   const Record3ReturnTypes preResult)
{
  Record3ReturnTypes result = preResult;

  if (category == RuleConst::STOPOVER_RULE)
  {
    FareMarketDataAccess& fmda = dynamic_cast<FareMarketDataAccess&>(da);

    if ((result == PASS || result == SOFTPASS) &&
        (rpData.soInfoWrapper()->stopoversRuleExistsInSet()))
    {
      result = rpData.soInfoWrapper()->processResults(trx, fmda.paxTypeFare());
    }
    if (result == SOFTPASS)
    {
      fmda.paxTypeFare().setCategorySoftPassed(category);
    }
  }

  if (category == RuleConst::TRANSFER_RULE)
  {
    FareMarketDataAccess& fmda = dynamic_cast<FareMarketDataAccess&>(da);

    if ((result == PASS || result == SOFTPASS) &&
        (rpData.trInfoWrapper()->transfersRuleExistsInSet()))
    {
      result = rpData.trInfoWrapper()->processResults(trx, fmda.paxTypeFare());
    }
    if (result == SOFTPASS)
    {
      fmda.paxTypeFare().setCategorySoftPassed(category);
    }
  }

  return result;
}

Record3ReturnTypes
FareMarketRuleController::callCategoryRuleItemSet(
    CategoryRuleItemSet& catRuleIS,
    const CategoryRuleInfo& ruleInfo,
    const std::vector<CategoryRuleItemInfoSet*>& crInfo,
    RuleControllerDataAccess& da,
    RuleProcessingData& rpData,
    bool isLocationSwapped,
    bool isFareRule,
    bool skipCat15Security)
{
  if (UNLIKELY(_categoryPhase == FareGroupSRValidation))
  {
    return catRuleIS.processFareGroup(da.trx(), ruleInfo, da.paxTypeFare(), crInfo);
  }

  else
  {
    if (UNLIKELY(_categoryPhase == VolunExcPrevalidation))
    {
      if (ruleInfo.categoryNumber() == RuleConst::VOLUNTARY_EXCHANGE_RULE)
      {
        da.paxTypeFare().setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE, true);

        return SOFTPASS; // only get Rule now, validation after reprice
      }
      else if (ruleInfo.categoryNumber() == RuleConst::VOLUNTARY_REFUNDS_RULE)
      {
        da.paxTypeFare().setCategoryValid(RuleConst::VOLUNTARY_REFUNDS_RULE, true);

        return SOFTPASS; // only get Rule now, validation after reprice
      }

      else if (ruleInfo.categoryNumber() != RuleConst::ADVANCE_RESERVATION_RULE)
        return PASS; // only get Rule, no validation needed
    }

    if (LIKELY(da.itin()))
    {
      if (!_loopValidatingCxrs)
      {
        PaxTypeFare& ptf = da.paxTypeFare();
        if (ptf.validatingCarriers().size() == 1)
          da.setValidatingCxr(ptf.validatingCarriers().front());

        return catRuleIS.process(da.trx(),
                               *da.itin(),
                               ruleInfo,
                               ptf,
                               crInfo,
                               rpData,
                               isLocationSwapped,
                               isFareRule,
                               skipCat15Security,
                               da);
      }
      else
      {
        PaxTypeFare& ptf = da.paxTypeFare();
        const uint16_t mainCatNum = da.currentCatNum();

        if (ptf.validatingCarriers().empty())
          ptf.validatingCarriers() = ptf.fareMarket()->validatingCarriers();

        if (ptf.cat25Fare() && ptf.cat25Fare()->validatingCarriers().empty())
        {
          ptf.cat25Fare()->validatingCarriers() = ptf.cat25Fare()->fareMarket()->validatingCarriers();
        }

        // If ptf is a Cat 25 base fare, get validatingCarriers() from Cat 25 fare
        std::vector<CarrierCode>& validatingCarriers =
            ptf.cat25Fare() ? ptf.cat25Fare()->validatingCarriers() : ptf.validatingCarriers();

        Record3ReturnTypes combinedResult = SKIP;
        bool everSkip = false;
        bool mixedRtn = false;
        bool firstPass = true;
        bool storedResult_Cat15HasSecurity = false;

        DiagManager diag500(da.trx(), Diagnostic500);
        std::vector<CarrierCode> failedCxrs;

        for (CarrierCode& cxr : validatingCarriers)
        {
          if (UNLIKELY(diag500.isActive()))
          {
            diag500 << "      VALIDATING CXR - " << cxr << " :\n";
            diag500.collector().flushMsg();
          }

          da.setValidatingCxr(cxr);
          Record3ReturnTypes result = catRuleIS.process(da.trx(),
                               *da.itin(),
                               ruleInfo,
                               ptf,
                               crInfo,
                               rpData,
                               isLocationSwapped,
                               isFareRule,
                               skipCat15Security,
                               da);
          if (result == SOFTPASS)
          {
            ptf.setCategorySoftPassed(mainCatNum);
            combinedResult = SOFTPASS;
          }
          else if (result == FAIL)
          {
            failedCxrs.push_back(cxr);
          }
          else if (result == SKIP)
          {
            everSkip = true;
          }
          else if (LIKELY(combinedResult != SOFTPASS && result != SKIP))
            combinedResult = result;

          if (mainCatNum == 15)
          {
            if (result == PASS && ptf.tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
            {
              if (firstPass)
              {
                storedResult_Cat15HasSecurity = da.getTmpResultCat15HasSecurity();
                firstPass = false;
              }
              else
              {
                if (UNLIKELY(storedResult_Cat15HasSecurity != da.getTmpResultCat15HasSecurity()) )
                  mixedRtn = true;
              }
            }
          }
        }

        if (ptf.cat25Fare()) //ptf is a Cat 25 base fare
        {
          // If ptf is a Cat 25 base fare, update result in Cat 25 fare
          ptf.cat25Fare()->setValidatingCxrInvalid(failedCxrs, mainCatNum);
          if (ptf.cat25Fare()->validatingCarriers().empty())
            combinedResult = FAIL;
        }
        else
        {
          ptf.setValidatingCxrInvalid(failedCxrs, mainCatNum);
          if (ptf.validatingCarriers().empty())
            combinedResult = FAIL;
        }

        if (UNLIKELY(everSkip && combinedResult == PASS))
          mixedRtn = true;

        if (UNLIKELY(mixedRtn))
        {
          if (UNLIKELY(diag500.isActive()))
          {
            diag500 << "    MIXED RESULT FROM CURRENT RECORD2 BY VALIDATING CARRIER\n";
            diag500 << "    FORCE REVALIDATE PER VALIDATING CARRIER FOR MAIN CAT\n";
            diag500.collector().flushMsg();
          }
          da.setRtnMixedResult(mainCatNum);
          combinedResult = SOFTPASS;
        }
        return combinedResult;
      }
    }
  }
  return SKIP;
}

void
FareMarketRuleController::applySurchargeGenRuleForFMS(PricingTrx& trx,
                                                      RuleControllerDataAccess& da,
                                                      uint16_t categoryNumber,
                                                      RuleControllerParam& rcParam,
                                                      bool skipCat15Security)
{
  if (da.getRuleType() != GeneralRule || !TrxUtil::isFVOSurchargesEnabled())
    return;

  PaxTypeFare& ptFMS = da.paxTypeFare(); // current PaxTypeFare

  if (!shouldApplyGeneralRuleSurcharges(trx, ptFMS))
    return;

  for (PaxTypeFare* candidate : ptFMS.fareMarket()->allPaxTypeFare())
  {
    if (!fareApplicableToGeneralRuleSurcharges(trx, ptFMS, candidate))
      continue;

    if ((candidate->directionality() != BOTH) &&
        ((ptFMS.directionality() == TO && candidate->directionality() == FROM) ||
         (ptFMS.directionality() == FROM && candidate->directionality() == TO)))
      continue; // skip fare with wrong directionality

    FareMarketDataAccess fmda(da.trx(), da.itin(), *candidate);
    fmda.retrieveBaseFare(false);

    // apply the GeneralRule processing of the Public vendor to the FMS fare.
    // SurchargesRule processing will add the GenRule surcharge.
    rcParam._processGenRuleForFMS = true; // To skip FareRule processing

    RuleProcessingData rpData;

    StopoversInfoWrapper soInfoWrapper;
    TransfersInfoWrapper trInfoWrapper;
    rpData.soInfoWrapper(&soInfoWrapper);
    rpData.trInfoWrapper(&trInfoWrapper);

    processFareRuleCommon(trx, fmda, categoryNumber, rcParam, rpData, skipCat15Security);

    Record3ReturnTypes retResultOfGRule =
        fallback::fallbackGfrR2Optimization(&trx)
            ? processGeneralRule<FallBackOnGfr>(
                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
            : processGeneralRule<FallBackOffGfr>(
                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security);

    rcParam._processGenRuleForFMS = false;
    if (retResultOfGRule == PASS)
      break; // job is done...
  }
}

Record3ReturnTypes
FareMarketRuleController::revalidateC15BaseFareForDisc(uint16_t category,
                                                       bool& checkFare,
                                                       PaxTypeFare* ptf,
                                                       RuleControllerDataAccess& da)
{
  // revalidation will be done only in FP scope (Pricing/Shopping).
  return PASS;
}

bool
FareMarketRuleController::reValidateMixedRtnPTF(RuleControllerDataAccess& da,
                                                PaxTypeFare& paxTypeFare,
                                                std::vector<CarrierCode>& validatingCarriers)
{
  bool rtn = false;

  for (CarrierCode& cxr : validatingCarriers)
  {
    da.validatingCxr() = cxr;
    for (uint16_t& catNum : da.mixedRtnResultCategroies())
    {
      paxTypeFare.setCategoryProcessed(catNum, false);
      paxTypeFare.setCategorySoftPassed(catNum, false);
      if (catNum == 15)
        paxTypeFare.fare()->setCat15HasSecurity(false);
    }
    bool rtnCxr = processCategorySequenceCommon(da.trx(), da, da.mixedRtnResultCategroies() );
    if (!rtnCxr)
    {
      const uint16_t catNum = paxTypeFare.getFailedCatNum(da.mixedRtnResultCategroies() );
      if (catNum != 0)
        paxTypeFare.setValidatingCxrInvalid(cxr, catNum);
    }
    else
      rtn = true;
  }
  return rtn;
}

} // namespace tse
