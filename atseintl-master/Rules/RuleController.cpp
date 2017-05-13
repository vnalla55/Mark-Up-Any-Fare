//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  File:        RuleController.cpp
//  Created:     Apr 8, 2003
//  Authors:     Devapriya SenGupta, Vladimir Koliasnikov
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
//-------------------------------------------------------------------

#include "Rules/RuleController.h"

#include "Common/ConfigList.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Rec2Selector.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TicketInfo.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/SalesRestriction.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag202Collector.h"
#include "Diagnostic/Diag225Collector.h"
#include "Diagnostic/Diag500Collector.h"
#include "Diagnostic/Diag550Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/Config.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleProcessingData.h"
#include "Rules/RuleUtil.h"
#include "Rules/Stopovers.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/Transfers1.h"
#include "Rules/TransfersInfoWrapper.h"
#include "Util/BranchPrediction.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <boost/date_time/gregorian/greg_day.hpp>
#include <boost/tokenizer.hpp>
#include <stdlib.h>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackSkipDummyFareValidation);
FALLBACK_DECL(cat25baseFarePrevalidation);
FALLBACK_DECL(fallbackFootNotePrevalidationForAltDates);
FALLBACK_DECL(fallbackValCxrR2Cat15);
FALLBACK_DECL(fallbackFlexFareCopyValidationStatusForCorpIdAccCode);
FALLBACK_DECL(fallbackAPO37838Record1EffDateCheck);
FIXEDFALLBACK_DECL(fallbackDisableESVIS);
FALLBACK_DECL(fallbackFootNoteR2Optimization);
FALLBACK_DECL(fallbackGfrR2Optimization);

namespace
{
ConfigurableValue<ConfigVector<uint16_t>>
noReuseCategories("REUSE_RULES_RESULT", "NO_REUSE_CATEGORIES");

// below two adaptors should be removed in further steps, they exist to provide fallback
template <typename R2Type>
std::vector<R2Type*>
locToDbVecAdaptor(const std::vector<std::pair<const R2Type*, GeoMatchResult>>& pairVec)
{
  std::vector<R2Type*> adapted;
  adapted.reserve(pairVec.size());

  for (const std::pair<const R2Type*, GeoMatchResult>& r2p : pairVec)
    adapted.emplace_back(const_cast<R2Type*>(r2p.first));

  return adapted;
}

template <typename R2Type>
const std::vector<std::pair<const R2Type*, GeoMatchResult>>
dbToLocVecAdaptor(const std::vector<R2Type*>& flatVec)
{
  std::vector<std::pair<const R2Type*, GeoMatchResult>> adapted;
  adapted.reserve(flatVec.size());

  for (const R2Type* r2 : flatVec)
    adapted.emplace_back(r2, GeoMatchResult(false));
  //.second is irrelevant, just for type matching

  return adapted;
}
}

bool
RuleController::FallBackOn::
operator()(PricingTrx& trx,
           const FootNoteCtrlInfoPair& fnci,
           const PaxTypeFare& paxTypeFare,
           bool& isLocationSwapped,
           const Footnote& footnote,
           const TariffNumber& tariff)
{
  return RuleUtil::matchFootNoteCtrlInfo(
      trx, *fnci.first, paxTypeFare, isLocationSwapped, footnote, tariff);
}

bool
RuleController::FallBackOff::
operator()(PricingTrx& trx,
           const FootNoteCtrlInfoPair& fnci,
           const PaxTypeFare& paxTypeFare,
           bool& isLocationSwapped,
           const Footnote& footnote,
           const TariffNumber& tariff)
{
  isLocationSwapped = fnci.second;
  return RuleUtil::matchFootNoteCtrlInfoNew(
      trx, *fnci.first, paxTypeFare, isLocationSwapped, footnote, tariff);
}

bool
RuleController::FallBackOnGfr::
operator()(PricingTrx& trx,
           const PaxTypeFare& paxTypeFare,
           const GeneralFareRuleInfoPair& gfri,
           bool& isLocationSwapped)
{
  return RuleUtil::matchGeneralFareRule(trx, paxTypeFare, *gfri.first, isLocationSwapped);
}

bool
RuleController::FallBackOffGfr::
operator()(PricingTrx& trx,
           const PaxTypeFare& paxTypeFare,
           const GeneralFareRuleInfoPair& gfri,
           bool& isLocationSwapped)
{
  return RuleUtil::matchGeneralFareRuleNew(trx, paxTypeFare, *gfri.first, isLocationSwapped);
}

static Logger
logger("atseintl.Rules.RuleController");

void
RuleControllerParam::reset()
{
  _genTariffNumber = 0;
  _genRuleNumber = RuleConst::NULL_GENERAL_RULE;
  _checkFareRuleAltGenRule = true;
  _generalRuleEnabled = true;
  _readRecord0 = true;
  _processGenRuleForFMS = false;
}

// -----------------------------------------------------
// End of functors
// -----------------------------------------------------

namespace
{
// Bound Fare
BindingResult
hasCat(const PricingTrx& trx,
       const PaxTypeFare& paxTypeFare,
       uint16_t catNumber,
       MATCHTYPE matchType)
{
  BindingResult result(paxTypeFare.fare()->_fareInfo->hasCat(trx, catNumber, matchType));
  return result;
}
bool
needGeneralRuleValidation(const PricingTrx& trx, int catNumber, const PaxTypeFare& paxTypeFare)
{
  return paxTypeFare.fare()->_fareInfo->needGeneralRuleValidation(trx, catNumber);
}
}

bool RuleController::_skipReuseCat[MAX_REUSE_CAT + 1];

RuleController::RuleController()
{
  setPhase(NormalValidation);

  setupFallbacks();
}

RuleController::RuleController(CategoryPhase categoryPhase) : _categoryRuleItemSet(categoryPhase)
{
  setPhase(categoryPhase);
  categoryRuleItemSet().setCategoryPhase(categoryPhase);

  setupFallbacks();
}

RuleController::RuleController(CategoryPhase categoryPhase, const std::vector<uint16_t>& categories)
  : _categoryRuleItemSet(categoryPhase)
{
  _categoryPhase = categoryPhase;
  _categorySequence.insert(_categorySequence.end(), categories.begin(), categories.end());
  categoryRuleItemSet().setCategoryPhase(categoryPhase);

  setupFallbacks();
}

void
RuleController::setupFallbacks()
{
#ifdef CONFIG_HIERARCHY_REFACTOR
  try
  {
    _fallbackCat25baseFarePrevalidation = fallback::cat25baseFarePrevalidation(nullptr);
  }
  catch (...)
  {
    // Should happen only in tests.
  }
#else
  _fallbackCat25baseFarePrevalidation = fallback::cat25baseFarePrevalidation(nullptr);
#endif
}

void
RuleController::setCategoryPhase(CategoryPhase categoryPhase)
{
  setPhase(categoryPhase);
  categoryRuleItemSet().setCategoryPhase(categoryPhase);
}

namespace
{
RuleController::CategoryPhaseMap* categoryPhases = nullptr;

ConfigurableCategories
configFCORuleValidation("FCO_RULE_VALIDATION"),
    configFCORuleValidationMIPALT("FCO_RULE_VALIDATION_MIP_ALT"),
    configPreValidation("PRE_VALIDATION"), configNormalValidation("NORMAL_VALIDATION"),
    configPURuleValidation("PU_RULE_VALIDATION"), configPURuleValidationIS("PU_RULE_VALIDATION_IS"),
    configPURuleValidationISALT("PU_RULE_VALIDATION_IS_ALT"),
    configFPRuleValidation("FP_RULE_VALIDATION"),
    configFPRuleValidationISALT("FP_RULE_VALIDATION_IS_ALT"),
    configShoppingComponentValidation("SHOPPING_COMPONENT_VALIDATION"),
    configShoppingAcrossStopOverComponentValidation(
        "SHOPPING_ACROSS_STOPOVER_COMPONENT_VALIDATION"),
    configShoppingComponentWithFlightsValidation("SHOPPING_COMPONENT_WITH_FLIGHTS_VALIDATION"),
    configShoppingASOComponentWithFlightsValidation(
        "SHOPPING_ASO_COMPONENT_WITH_FLIGHTS_VALIDATION"),
    configShoppingComponentValidateQualifiedCat4("SHOPPING_VALIDATE_IF_CAT4"),
    configShoppingItinBasedValidation("SHOPPING_ITIN_BASED_VALIDATION"),
    configShoppingAltDateItinBasedValidation("SHOPPING_ITIN_BASED_VALIDATION_ALT"),
    configShoppingItinFlightIndependentValidation("SHOPPING_ITIN_FLIGHT_INDEPENDENT_VALIDATION"),
    configPreCombinabilityValidation("PRE_COMBINABILITY_VALIDATION"),
    configFareDisplayValidation("FARE_DISPLAY_VALIDATION"),
    configRuleDisplayValidation("RULE_DISPLAY_VALIDATION"),
    configFareGroupSRValidation("FAREGROUP_SR_VALIDATION"),
    configVolunExcPrevalidation("EXC_ITIN_RULE_PREVALIDATION"),
    configRevalidatePassResultReusedFU("REVALIDATE_PASS_RESULT_REUSED_FU"),
    configFPRuleFamilyLogicChildrenValidation("FP_FAMILY_LOGIC_CHILDREN");

void
addPhase(CategoryPhase phase, ConfigurableCategories& config)
{
  (*categoryPhases)[phase] = config.read();
}

void
addPhaseStatic(CategoryPhase phase, const std::vector<uint16_t>& categories)
{
  std::vector<uint16_t>& sequence = (*categoryPhases)[phase];

  sequence.insert(sequence.end(), categories.begin(), categories.end());
}

RuleController::CategoryPhaseMap*
buildCategoryPhaseMap()
{
  if (categoryPhases == nullptr)
  {
    categoryPhases = new RuleController::CategoryPhaseMap;
    addPhase(FCORuleValidation, configFCORuleValidation);
    addPhase(FCORuleValidationMIPALT, configFCORuleValidationMIPALT);
    addPhase(PreValidation, configPreValidation);
    addPhase(NormalValidation, configNormalValidation);
    addPhase(PURuleValidation, configPURuleValidation);
    addPhase(PURuleValidationIS, configPURuleValidationIS);
    addPhase(PURuleValidationISALT, configPURuleValidationISALT);
    addPhase(FPRuleValidation, configFPRuleValidation);
    addPhase(FPRuleValidationISALT, configFPRuleValidationISALT);
    addPhase(ShoppingComponentValidation, configShoppingComponentValidation);
    addPhase(ShoppingAcrossStopOverComponentValidation,
             configShoppingAcrossStopOverComponentValidation);
    addPhase(ShoppingComponentWithFlightsValidation, configShoppingComponentWithFlightsValidation);
    addPhase(ShoppingASOComponentWithFlightsValidation,
             configShoppingASOComponentWithFlightsValidation);
    addPhase(ShoppingComponentValidateQualifiedCat4, configShoppingComponentValidateQualifiedCat4);
    addPhase(ShoppingItinBasedValidation, configShoppingItinBasedValidation);
    addPhase(ShoppingAltDateItinBasedValidation, configShoppingAltDateItinBasedValidation);
    addPhase(ShoppingItinFlightIndependentValidation,
             configShoppingItinFlightIndependentValidation);
    addPhase(PreCombinabilityValidation, configPreCombinabilityValidation);
    addPhase(FareDisplayValidation, configFareDisplayValidation);
    addPhase(RuleDisplayValidation, configRuleDisplayValidation);
    addPhase(FareGroupSRValidation, configFareGroupSRValidation);
    addPhase(VolunExcPrevalidation, configVolunExcPrevalidation);
    addPhase(RevalidatePassResultReusedFU, configRevalidatePassResultReusedFU);
    addPhase(FPRuleFamilyLogicChildrenValidation, configFPRuleFamilyLogicChildrenValidation);
    addPhaseStatic(FootNotePrevalidation,
                   {RuleConst::SEASONAL_RULE,
                    RuleConst::BLACKOUTS_RULE,
                    RuleConst::TRAVEL_RESTRICTIONS_RULE,
                    RuleConst::SALE_RESTRICTIONS_RULE});
    addPhaseStatic(FootNotePrevalidationALT, {RuleConst::SALE_RESTRICTIONS_RULE});
  }
  return categoryPhases;
}
}
bool
RuleController::doesPhaseContainCategory(const CategoryPhase phase, const uint16_t categoryNumber)
    const
{
  const CategoryPhaseMap& phaseMap = getCategoryPhaseMap();
  CategoryPhaseMap::const_iterator it = phaseMap.find(phase);
  if (it == phaseMap.end())
    return false;
  return it->second.end() != std::find(it->second.begin(), it->second.end(), categoryNumber);
}

const RuleController::CategoryPhaseMap&
RuleController::getCategoryPhaseMap() const
{
  static CategoryPhaseMap* phaseMap = buildCategoryPhaseMap();
  return *phaseMap;
}

bool
RuleController::setPhase(CategoryPhase categoryPhase)
{
  _categoryPhase = categoryPhase;

  const CategoryPhaseMap& phaseMap = getCategoryPhaseMap();
  CategoryPhaseMap::const_iterator itor = phaseMap.find(categoryPhase);
  if (UNLIKELY(itor == phaseMap.end()))
  {
    itor = phaseMap.find(NormalValidation);
  }

  if (LIKELY(itor != phaseMap.end()))
  {
    _categorySequence = itor->second;
    return true;
  }
  else
  {
    return false;
  }
}

bool
RuleController::skipCategoryProcessing(uint16_t category,
                                       const PaxTypeFare& paxTypeFare,
                                       const PricingTrx& trx) const
{
  bool skip = false;
  if (category == RuleConst::FARE_BY_RULE)
  {
    if (!paxTypeFare.isFareByRule())
    {
      skip = true;
    }
  }
  else if (category == RuleConst::NEGOTIATED_RULE)
  {
    if (!paxTypeFare.isNegotiated())
    {
      skip = true;
    }
  }
  else if (RuleUtil::isDiscount(category))
  {
    if (!paxTypeFare.isDiscounted())
    {
      skip = true;
    }
  }
  else if (category == RuleConst::ELIGIBILITY_RULE || category == RuleConst::ACCOMPANIED_PSG_RULE)
  {
    if (paxTypeFare.isDiscounted())
    {
      skip = true;
    }
  }
  else if (UNLIKELY(category == RuleConst::VOLUNTARY_EXCHANGE_RULE &&
                    trx.excTrxType() != PricingTrx::AR_EXC_TRX &&
                    !smp::isPenaltyCalculationRequired(trx)))
    skip = true;

  else if (UNLIKELY(category == RuleConst::VOLUNTARY_REFUNDS_RULE &&
                    trx.excTrxType() != PricingTrx::AF_EXC_TRX &&
                    !smp::isPenaltyCalculationRequired(trx)))
    skip = true;

  return skip;
}

Record3ReturnTypes
RuleController::processCategoryNormalValidation(PricingTrx& trx,
                                                const uint16_t category,
                                                PaxTypeFare& paxTypeFare,
                                                bool fbrCalcFare,
                                                const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const
{
  Record3ReturnTypes result = PASS;

  // Tkt Endorsement should always go thru
  if (paxTypeFare.isCategoryProcessed(category) && category != RuleConst::TICKET_ENDORSMENT_RULE)
  {
    if (trx.isValidatingCxrGsaApplicable())
      paxTypeFare.consolidValidatingCxrList();

    if (!paxTypeFare.isCategoryValid(category))
    {
      if (!(category == RuleConst::SALE_RESTRICTIONS_RULE && paxTypeFare.cat15SecurityFail()))
        return FAIL;
    }
    result = SKIP;
  }
  if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE)
  {
    if (!paxTypeFare.isCategorySoftPassed(category))
      result = SKIP; // Skip hard_PASS for CAT25/CAT35
  }
  // discounted fare can share validation result with base fare
  // and each other during pre-val and FC val phase
  // Tkt Endorsement should always go thru
  if (paxTypeFare.isDiscounted() && category != RuleConst::TICKET_ENDORSMENT_RULE &&
      category < RuleConst::CHILDREN_DISCOUNT_RULE &&
      paxTypeFare.adoptBaseFareRuleStat(trx.dataHandle(), category))
  {
    if (!trx.getRequest()->isSFR())
      result = SKIP;
  }
  // skip Cat 6/Cat 7 validation at FC Phase for Cat 25 OW fare
  // that was created from base RT fare
  if (fbrCalcFare &&
      (category == RuleConst::MINIMUM_STAY_RULE || category == RuleConst::MAXIMUM_STAY_RULE) &&
      (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
       paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
  {
    if (fbrPaxTypeFare->baseFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      result = SKIP;
    }
  }
  return result;
}

bool
RuleController::skipCategoryNormalValidation(PricingTrx& trx, const uint16_t category,
                                             PaxTypeFare& paxTypeFare,
                                             bool fbrCalcFare,
                                             const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const
{
  bool skip = false;

  // Tkt Endorsement should always go thru
  if (paxTypeFare.isCategoryProcessed(category) && category != RuleConst::TICKET_ENDORSMENT_RULE)

  {
    skip = true;
    if (trx.isValidatingCxrGsaApplicable())
      paxTypeFare.consolidValidatingCxrList();
  }
  if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE)
  {
    if (!paxTypeFare.isCategorySoftPassed(category))
      skip = true; // Skip hard_PASS for CAT25/CAT35
  }
  // discounted fare can share validation result with base fare
  // and each other during pre-val and FC val phase
  // Tkt Endorsement should always go thru
  if (paxTypeFare.isDiscounted() && category != RuleConst::TICKET_ENDORSMENT_RULE &&
      category < RuleConst::CHILDREN_DISCOUNT_RULE &&
      paxTypeFare.adoptBaseFareRuleStat(trx.dataHandle(), category))
  {
    if (!trx.getRequest()->isSFR())
      skip = true;
  }
  // skip Cat 6/Cat 7 validation at FC Phase for Cat 25 OW fare
  // that was created from base RT fare
  if (fbrCalcFare &&
      (category == RuleConst::MINIMUM_STAY_RULE || category == RuleConst::MAXIMUM_STAY_RULE) &&
      (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED ||
       paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
  {
    if (fbrPaxTypeFare->baseFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      skip = true;
    }
  }

  return skip;
}


bool
RuleController::skipCategoryFCORuleValidation(PricingTrx& trx,
                                              uint16_t category,
                                              PaxTypeFare& paxTypeFare) const
{
  bool skip = false;

  if (paxTypeFare.isCategoryProcessed(category))
  {
    skip = true;
    if (trx.isValidatingCxrGsaApplicable())
      paxTypeFare.consolidValidatingCxrList();
  }
  if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE)
  {
    if (!paxTypeFare.isCategorySoftPassed(category))
      skip = true; // Skip hard_PASS for CAT25/CAT35
  }
  // discounted fare can share validation result with base fare
  // and each other during pre-val and FC val phase
  if (paxTypeFare.isDiscounted() && category < RuleConst::CHILDREN_DISCOUNT_RULE &&
      paxTypeFare.adoptBaseFareRuleStat(trx.dataHandle(), category))
  {
    if (!trx.getRequest()->isSFR())
      skip = true;
  }

  return skip;
}

bool
RuleController::skipRuleDisplayValidation(uint16_t category, const PaxTypeFare& paxTypeFare) const
{
  bool skip = false;

  if (FareDisplayUtil::isCat15TuningEnabled())
  {
    if (paxTypeFare.isCategoryProcessed(category))
      skip = true;
  }
  else
  {
    if (category != RuleConst::SALE_RESTRICTIONS_RULE && paxTypeFare.isCategoryProcessed(category))
      skip = true;
  }
  if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE)
  {
    if (!paxTypeFare.isCategorySoftPassed(category))
      skip = true; // Skip hard_PASS for CAT25 and CAT35
  }

  return skip;
}

bool
RuleController::revalidateDiscountQualifiers(PricingTrx& trx,
                                             RuleControllerDataAccess& da,
                                             uint16_t category,
                                             PaxTypeFare& paxTypeFare,
                                             PaxTypeFareRuleData* ruleData)
{
  if ((reValidDiscQualifiers(paxTypeFare, category, da) == FAIL) ||
      (da.currentFU() == nullptr ||
       CategoryRuleItem::isDirectionPass(trx,
                                         *da.currentFU(),
                                         *ruleData->categoryRuleInfo(),
                                         ruleData->categoryRuleItemInfo(),
                                         ruleData->isLocationSwapped()) == FAIL))
  {
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic555 &&
                 !trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN)))
    {
      DCFactory* factory = DCFactory::instance();
      DiagCollector* diagPtr = factory->create(trx);
      DiagCollector& diag = *diagPtr;

      diag.enable(Diagnostic555);
      if (diag.isActive())
      {
        diag << "       CAT " << category << " FAILED RULE QUALIFIER OR DIRECTIONALITY"
             << std::endl;
        diag.flushMsg();
      }
    }
    paxTypeFare.setCategoryValid(category, false);
    return false;
  }

  return true;
}

bool
RuleController::skipCategoryPreValidation(PricingTrx& trx,
                                          uint16_t category,
                                          PaxTypeFare& paxTypeFare) const
{
  // discounted fare can share validation result with base fare
  // and each other during pre-val and FC val phase
  return (UNLIKELY(paxTypeFare.isDiscounted() && category < RuleConst::CHILDREN_DISCOUNT_RULE &&
                   paxTypeFare.adoptBaseFareRuleStat(trx.dataHandle(), category) &&
                   (!trx.getRequest()->isSFR())));
}

bool
RuleController::skipCalculatedFBR(const PaxTypeFare& paxTypeFare,
                                  bool fbrCalcFare,
                                  uint16_t category) const
{
  return (fbrCalcFare) &&
         (_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
          _categoryPhase == FPRuleValidation || _categoryPhase == PURuleValidationISALT ||
          _categoryPhase == FPRuleValidationISALT) &&
         paxTypeFare.isCategoryProcessed(category) && !paxTypeFare.isCategorySoftPassed(category);
}

bool
RuleController::skipCat15ForSellingNetFare(Indicator fcaDisplayCatType) const
{
  return (fcaDisplayCatType == RuleConst::SELLING_FARE ||
          fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE ||
          fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE_UPD);
}

Record3ReturnTypes
RuleController::processCategoryPhase(PricingTrx& trx,
                                     RuleControllerDataAccess& da,
                                     PaxTypeFare& paxTypeFare,
                                     uint16_t category,
                                     const FBRPaxTypeFareRuleData* fbrPaxTypeFare,
                                     bool fbrCalcFare)
{
  Record3ReturnTypes ret = PASS;

  switch (_categoryPhase)
  {
  case ShoppingComponentValidateQualifiedCat4:
    if (!isCatQualifiedFltApp(paxTypeFare, category))
      ret = SKIP;
    break;

  case NormalValidation:
  case FBRBaseFarePrevalidation:
    if (skipCategoryNormalValidation(trx, category, paxTypeFare, fbrCalcFare, fbrPaxTypeFare))
      ret = SKIP;
    break;

  case FCORuleValidationMIPALT:
  case FCORuleValidation:
    if (skipCategoryFCORuleValidation(trx, category, paxTypeFare))
      ret = SKIP;
    break;

  case FareDisplayValidation:
  case RuleDisplayValidation:
    if (skipRuleDisplayValidation(category, paxTypeFare))
      ret = SKIP;
    break;

  case ShoppingComponentValidation:
  case ShoppingAcrossStopOverComponentValidation:
    if (paxTypeFare.isCategoryProcessed(category))
      ret = SKIP;
    break;

  case PURuleValidation:
  case PURuleValidationIS:
  case FPRuleValidation:
  case PURuleValidationISALT:
  case FPRuleValidationISALT:
  case ShoppingItinFlightIndependentValidation:
  case ShoppingItinBasedValidation:
  case ShoppingAltDateItinBasedValidation:
    ret = processCategoryPurFprShopping(trx, da, paxTypeFare, category, fbrCalcFare);
    break;

  case PreValidation:
    if (UNLIKELY(skipCategoryPreValidation(trx, category, paxTypeFare)))
      ret = SKIP;
    break;

  default:
    break;
  }

  return ret;
}

Record3ReturnTypes
RuleController::processCategoryPurFprShopping(PricingTrx& trx,
                                              RuleControllerDataAccess& da,
                                              PaxTypeFare& paxTypeFare,
                                              uint16_t category,
                                              bool fbrCalcFare)
{
  Record3ReturnTypes ret = PASS;
  FareUsage* fu = da.currentFU();

  // Do not revalidate Cat1 for the selected ticketed fare
  // for the WPNETT entry (Jal/Axess).
  //
  if (UNLIKELY(paxTypeFare.ticketedFareForAxess() && category == RuleConst::ELIGIBILITY_RULE))
    ret = SKIP; // Skip Cat1 for Jal/axess selected fare.
  else if (paxTypeFare.isDiscounted())
  {
    if (RuleUtil::isDiscount(category))
    {
      // only re-validate the Qualifier, Out/Inbound Indicator
      // and Directionality
      PaxTypeFareRuleData* ruleData =
          paxTypeFare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);

      if (!ruleData || !ruleData->isSoftPassDiscount())
        ret = SKIP; // PASS discount categories
      else if (ruleData->categoryRuleInfo() &&
               (ruleData->categoryRuleInfo()->categoryNumber() != category))
        ret = SKIP; // discount from different category, skip
      else if (!revalidateDiscountQualifiers(trx, da, category, paxTypeFare, ruleData))
        ret = FAIL;
      else
        ret = SKIP;
    }
  }

  if (ret == PASS)
  {
    if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE)
    {
      if (!paxTypeFare.isCategorySoftPassed(category))
        ret = SKIP; // Skip hard_PASS for CAT25 and CAT35
    }
    else if (fu != nullptr && fu->ignorePTFCmdPrcFailedFlag())
    {
    }
    else if (!fbrCalcFare && paxTypeFare.isCategoryProcessed(category) &&
             !paxTypeFare.isCategorySoftPassed(category))
    {
      if (category != RuleConst::MINIMUM_STAY_RULE && category != RuleConst::MAXIMUM_STAY_RULE)
      {
        if (paxTypeFare.isFareByRule())
        {
          if (category != RuleConst::STOPOVER_RULE && category != RuleConst::TRANSFER_RULE &&
              category != RuleConst::SURCHARGE_RULE)
          {
            ret = SKIP;
          }
        }
        else
        {
          ret = SKIP;
        }
      }
    }
    else if (category == RuleConst::SALE_RESTRICTIONS_RULE && !paxTypeFare.isFareByRule() &&
             paxTypeFare.isDiscounted() && !paxTypeFare.isNegotiated() &&
             paxTypeFare.isCategoryProcessed(category) &&
             paxTypeFare.isCategorySoftPassed(category))
    {
      ret = SOFTPASS;
    }
  }

  return ret;
}

bool
RuleController::processCategorySequenceCommon(PricingTrx& trx,
                                              RuleControllerDataAccess& da,
                                              std::vector<uint16_t>& categorySequence)
{
  // Shortcuts
  PaxTypeFare& paxTypeFare = da.paxTypeFare();
  getFootnotes(paxTypeFare, da.footNoteTbl());

  // Check for a Diagnostic request
  const FBRPaxTypeFareRuleData* fbrPaxTypeFare = nullptr;
  const FareByRuleItemInfo* fbrItemInfo = nullptr;
  bool fbrCalcFare = false;
  bool revalidC15ForDisc = false;

  if (paxTypeFare.isFareByRule())
  {
    fbrPaxTypeFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (LIKELY(fbrPaxTypeFare != nullptr))
    {
      fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());
      fbrCalcFare = (fbrItemInfo && !fbrPaxTypeFare->isSpecifiedFare());
    }
  }
  const bool isFareDisplayTrx = (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX);

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
               trx.excTrxType() == PricingTrx::AF_EXC_TRX))
    skipRexSecurityCheck(static_cast<RexBaseTrx&>(trx), paxTypeFare, categorySequence);

  if (!fallback::fixed::fallbackSkipDummyFareValidation())
  {
    if (UNLIKELY(paxTypeFare.isDummyFare()))
    {
      return true;
    }
  }

  for (const uint16_t category : categorySequence)
  {
    setProcessedCategory(category);
    if (skipCategoryProcessing(category, paxTypeFare, trx))
      continue;
    da.currentCatNum() = category;
    revalidC15ForDisc = false;

    switch (processCategoryPhase(trx, da, paxTypeFare, category, fbrPaxTypeFare, fbrCalcFare))
    {
    case SKIP:
      continue;
    case FAIL:
      return false;
    case SOFTPASS:
      revalidC15ForDisc = true;
      break;
    default:
      break;
    }

    if (category == RuleConst::TICKET_ENDORSMENT_RULE)
    {
      paxTypeFare.setCategoryValid(RuleConst::TICKET_ENDORSMENT_RULE, true);
    }

    Record3ReturnTypes retResultOfRule = SKIP;
    bool checkFare = true;

    if (UNLIKELY(_categoryPhase == FareGroupSRValidation))
    {
      retResultOfRule = processFareGroupRules(trx, da, category);
      continue;
    }
    else if (fbrCalcFare && category != RuleConst::FARE_BY_RULE)
    {
      retResultOfRule = validateBaseFare(category, fbrItemInfo, checkFare, &paxTypeFare, da);

      if (UNLIKELY(isFareDisplayTrx))
      {
        saveRuleInfo(trx, category, paxTypeFare, *fbrItemInfo);
        setFBRBaseFareRuleInfo(trx, category, paxTypeFare, fbrItemInfo);
      }
    }

    // Non-Cat 25, Cat 25 specified, Cat 25 calculated with override tag 'X' or ' ' fare,
    // the checkFare value is always true.
    if (checkFare)
    {
      if (retResultOfRule != FAIL)
      {
        da.retrieveBaseFare(false);

        if (skipCalculatedFBR(paxTypeFare, fbrCalcFare, category))
        {
          // Do not revalidate the Calculated FBR fare when job was done.
          // Note: Cat8/9/12 are 'softpassed' always.
          //       Min/Max stay rule should be revalidated always (a temporary problem)
          if (category != RuleConst::MINIMUM_STAY_RULE && category != RuleConst::MAXIMUM_STAY_RULE)
          {
            continue;
          }
        }

        bool skipCat15Security = ifSkipCat15Security(paxTypeFare, category, isFareDisplayTrx);

        if (UNLIKELY(paxTypeFare.selectedFareNetRemit() &&
                     (category == RuleConst::SALE_RESTRICTIONS_RULE ||
                      category == RuleConst::TRAVEL_RESTRICTIONS_RULE)))
        {
          // Validate only Footnotes for the selected ticketing (cat35 Net Remit) fares
          // in Cat14 and in Cat15.
          checkFare = false;
        }

        if (UNLIKELY(isFareDisplayTrx))
        {
          try
          {
            // NegFareCtrl has already verified rules and saved them
            retResultOfRule = processRules(trx, da, category, checkFare, skipCat15Security);
          }
          catch (boost::gregorian::bad_day_of_month& ex)
          {
            LOG4CXX_INFO(logger,
                         "EXCEPTION FROM BOOST: " << ex.what()
                                                  << " FARE:" << paxTypeFare.fareClass()
                                                  << " CATEGORY: " << category);

            retResultOfRule = FAIL;
          }
        }
        else
        {
          if (UNLIKELY(revalidC15ForDisc))
          {
            // Need to revalidate Cat15 for the pure Discounted fare
            // in FarePass scope for Pricing/Shopping
            // when the base fare has SOFTPASS.
            retResultOfRule = revalidateC15BaseFareForDisc(category, checkFare, &paxTypeFare, da);
          }
          else
            retResultOfRule = processRules(trx, da, category, checkFare, skipCat15Security);
        }
      }
    } // checkFare
    // Only Cat 25 fare with override tag 'B' can have checkFare value changed to be false.
    else
    {
      // Fail Cat 25 private fare if it does not have Cat 35 and Cat 15 override tag is 'B'
      // but base fare has no security in Cat 15 and passes Cat 15 validation
      updateResultOfRuleAndFareDisplayInfo(
          retResultOfRule, category, paxTypeFare, fbrPaxTypeFare, trx);
    }

    if (isBasicValidationPhase() || _categoryPhase == FBRBaseFarePrevalidation)
    {
      paxTypeFare.setCategoryProcessed(category);
    }

    if (trx.getTrxType() == PricingTrx::MIP_TRX &&
        (_categoryPhase == FCORuleValidation || (_categoryPhase == PreValidation)))
    {
      PaxTypeFare* baseFare = (paxTypeFare.isFareByRule() && !paxTypeFare.isSpecifiedFare())
                                  ? paxTypeFare.baseFare()
                                  : nullptr;
      bool baseFareQual = (baseFare != nullptr && isCatQualifiedFltApp(*baseFare, category));

      if (baseFareQual)
      {
        baseFare->setCategoryProcessed(category, false);
        paxTypeFare.setCategoryProcessed(category, false);
      }
      else if (isCatQualifiedFltApp(paxTypeFare, category))
        paxTypeFare.setCategoryProcessed(category, false);
    }

    if (_categoryPhase != FBRBaseFarePrevalidation)
      updateResultOfRule(retResultOfRule, category, paxTypeFare, revalidC15ForDisc, fbrPaxTypeFare);

    if (!passCategory(category, retResultOfRule, paxTypeFare))
    {
      if (isBasicValidationPhase() || _categoryPhase == ShoppingComponentWithFlightsValidation ||
          _categoryPhase == ShoppingASOComponentWithFlightsValidation)
      {
        if (UNLIKELY(skipCmdPricingOrErd(trx, paxTypeFare, category)))
          continue;

        paxTypeFare.setCategoryValid(category, false);
        updateBaseFareRuleStatDiscounted(paxTypeFare, trx, category);
        break;
      }
      else if ((_categoryPhase == DynamicValidation) ||
               (_categoryPhase == PreCombinabilityValidation))
      {
        return false;
      }
      else if (_categoryPhase == FBRBaseFarePrevalidation)
      {
        if (UNLIKELY(skipCmdPricingOrErd (trx, paxTypeFare, category)))
          continue;

        paxTypeFare.setCategoryValid(category, false);
        return false;
      }
      else
      {
        if (UNLIKELY(checkIfPassForCmdPricing(trx, category, paxTypeFare, da)))
          continue;

        return false;
      }
    }
    else if (LIKELY(!trx.noPNRPricing()))
    {
      updateBaseFareRuleStatDiscounted(paxTypeFare, trx, category);
    }
  }
  return checkMissedFootnote(paxTypeFare, trx, da);
}

bool
RuleController::isBasicValidationPhase() const
{
  return _categoryPhase == NormalValidation || _categoryPhase == FareDisplayValidation ||
         _categoryPhase == RuleDisplayValidation || _categoryPhase == FCORuleValidation ||
         _categoryPhase == FCORuleValidationMIPALT || _categoryPhase == PreValidation ||
         _categoryPhase == ShoppingComponentValidation ||
         _categoryPhase == ShoppingAcrossStopOverComponentValidation ||
         _categoryPhase == ShoppingComponentValidateQualifiedCat4;
}

bool
RuleController::passForErd(const PricingTrx& trx, const PaxTypeFare& paxTypeFare, uint16_t category)
    const
{
  bool ret = false;

  if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
  {
    const FareDisplayTrx* fdTrx = static_cast<const FareDisplayTrx*>(&trx);
    if (fdTrx->isERD() && fdTrx->getOptions() && fdTrx->getRequest() &&
        fdTrx->getOptions()->isErdAfterCmdPricing() &&
        RuleController::mayBeSkippedByCmdPricing(trx, paxTypeFare, category) &&
        fdTrx->getRequest()->fareBasisCode() ==
            paxTypeFare.createFareBasis(const_cast<PricingTrx&>(trx)))
    {
      ret = true;
    }
  }
  return ret;
}

bool
RuleController::passForCmdPricing(const PricingTrx& trx,
                                  const PaxTypeFare& paxTypeFare,
                                  uint16_t category) const
{
  return category != RuleConst::CHILDREN_DISCOUNT_RULE &&
         category != RuleConst::TOUR_DISCOUNT_RULE && category != RuleConst::AGENTS_DISCOUNT_RULE &&
         category != RuleConst::OTHER_DISCOUNT_RULE &&
         RuleController::mayBeSkippedByCmdPricing(trx, paxTypeFare, category) &&
         paxTypeFare.isCmdPricing() && paxTypeFare.validForCmdPricing(trx.fxCnException());
}

bool
RuleController::skipCmdPricingOrErd(const PricingTrx& trx,
                                    PaxTypeFare& paxTypeFare,
                                    uint16_t category) const
{
  bool ret = false;
  if (UNLIKELY(RuleController::mayBeSkippedByCmdPricing(trx, paxTypeFare, category) &&
               paxTypeFare.isCmdPricing() && paxTypeFare.validForCmdPricing(trx.fxCnException()) &&
               paxTypeFare.setCmdPrcFailedFlag(category)))
  {
    ret = true;
  }
  else if (UNLIKELY(passForErd(trx, paxTypeFare, category)))
  {
    ret = true;
  }

  return ret;
}

bool
RuleController::isCat35Fare(const PaxTypeFare& paxTypeFare) const
{
  bool cat35Fare = false;
  if (paxTypeFare.isNegotiated() ||
      (_categoryPhase == PreValidation &&
       (paxTypeFare.fcaDisplayCatType() == RuleConst::SELLING_FARE ||
        paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE ||
        paxTypeFare.fcaDisplayCatType() == RuleConst::NET_SUBMIT_FARE_UPD)))
  {
    cat35Fare = true;
  }

  return cat35Fare;
}

void
RuleController::updateResultOfRule(Record3ReturnTypes& retResultOfRule,
                                   uint16_t category,
                                   const PaxTypeFare& paxTypeFare,
                                   bool revalidC15ForDisc,
                                   const FBRPaxTypeFareRuleData* fbrPaxTypeFare) const
{
  if (category == RuleConst::SALE_RESTRICTIONS_RULE &&
      (retResultOfRule == PASS || retResultOfRule == SKIP) &&
      paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && !isCat35Fare(paxTypeFare) &&
      !revalidC15ForDisc && fbrPaxTypeFare == nullptr && // fbr is NULL
      !paxTypeFare.fare()->cat15HasSecurity() &&
      !paxTypeFare.selectedFareNetRemit())
  {
    retResultOfRule = FAIL;
  }
}
void
RuleController::updateResultOfRuleAndFareDisplayInfo(Record3ReturnTypes& retResultOfRule,
                                                     uint16_t category,
                                                     PaxTypeFare& paxTypeFare,
                                                     const FBRPaxTypeFareRuleData* fbrPaxTypeFare,
                                                     PricingTrx& trx) const
{
  if (category == RuleConst::SALE_RESTRICTIONS_RULE && retResultOfRule == PASS &&
      paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF && !isCat35Fare(paxTypeFare) &&
      fbrPaxTypeFare != nullptr && !fbrPaxTypeFare->baseFare()->fare()->cat15HasSecurity())
  {
    retResultOfRule = FAIL;
  }
  else
  {
    updateFareDisplayInfo(category, trx, paxTypeFare);
  }
}

bool
RuleController::checkMissedFootnote(PaxTypeFare& paxTypeFare,
                                    PricingTrx& trx,
                                    RuleControllerDataAccess& da)
{
  if (missedFootnote(paxTypeFare, da.footNoteTbl(), trx))
  {
    // Can't return false for Fare Display otherwise next
    //  paxTypeFares will not be processed
    if ((_categoryPhase != FareDisplayValidation) && (_categoryPhase != RuleDisplayValidation))
    {
      // pass command pricing on missing footnote, warn as Combinability
      if (paxTypeFare.validForCmdPricing(trx.fxCnException()))
        paxTypeFare.cpFailedStatus().set(PaxTypeFare::PTFF_MISSED_FTNOTE, true);
      else
      {
        if (!TrxUtil::optimusNetRemitEnabled(trx) ||
            typeid(*(da.farePath())) != typeid(NetRemitFarePath))
          return false;
      }
    }
  }

  return true;
}

void
RuleController::updateBaseFareRuleStatDiscounted(PaxTypeFare& paxTypeFare,
                                                 const PricingTrx& trx,
                                                 uint16_t category) const
{
  if (trx.getRequest() && trx.getRequest()->isSFR())
    return;

  if (category == RuleConst::FARE_BY_RULE || category == RuleConst::NEGOTIATED_RULE ||
      (category == RuleConst::SURCHARGE_RULE && TrxUtil::isFVOSurchargesEnabled()))
    return;

  if (paxTypeFare.isDiscounted() && !trx.isAltDates())
  {
    if (_categoryPhase == NormalValidation || _categoryPhase == ShoppingComponentValidation ||
        _categoryPhase == ShoppingAcrossStopOverComponentValidation ||
        _categoryPhase == PreValidation)
    { // for sharing result

      //  IN case the rule has an "if CAT 1(ELIGIBILITY)" we can not assume that the base fare will
      // pass the validation
      //  and we will NOT allow for sharing result seting the basefare as pass.
      bool hasQual = false;
      PaxTypeFareRuleData* gfrRuleData = nullptr;

      const PaxTypeFareRuleData* ptfRd = paxTypeFare.paxTypeFareRuleData(category);

      if (ptfRd)
      {
        const CategoryRuleInfo* cri = ptfRd->categoryRuleInfo();

        if (cri && isThereQualifier(*cri, RuleConst::ELIGIBILITY_RULE) == true)
        {
          hasQual = true;
        }
      }
      if (!hasQual && (paxTypeFare.alreadyChkedGfrRule(category, trx.dataHandle(), gfrRuleData)))
      {
        if (gfrRuleData)
        {
          const CategoryRuleInfo* gfrRuleInfo = gfrRuleData->categoryRuleInfo();
          if (UNLIKELY(gfrRuleInfo &&
                       isThereQualifier(*gfrRuleInfo, RuleConst::ELIGIBILITY_RULE) == true))
          {
            hasQual = true;
          }
        }
      }
      if (!hasQual)
      {
        paxTypeFare.updateBaseFareRuleStat(trx.dataHandle(), category);
      }
    }
  }
}

bool
RuleController::checkIfPassForCmdPricing(PricingTrx& trx,
                                         uint16_t category,
                                         const PaxTypeFare& paxTypeFare,
                                         RuleControllerDataAccess& da) const
{
  DiagCollector* diagPtr = nullptr;
  bool ret = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic555 &&
               !trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN)))
  {
    // Add output to Diagnostic555 about which category failed
    // the rule validation
    DCFactory* factory = DCFactory::instance();

    diagPtr = factory->create(trx);
    DiagCollector& diag = *diagPtr;

    diag.enable(Diagnostic555);

    if (diag.isActive())
    {
      diag << "       CAT " << category << " FAILED RULE VALIDATION" << std::endl;
      diag.flushMsg();
    }
  }

  if (UNLIKELY(passForCmdPricing(trx, paxTypeFare, category)))
  {
    // although we can access paxTypeFare during PU validation,
    // because there might be multithread sharing same
    // paxTypeFare, we are not allowed to change anything
    // of it, instead we set the flag at pricingUnit
    PricingUnit* pu = da.currentPU();

    if (pu && pu->setCmdPrcFailedFlag(category))
    {
      if (UNLIKELY(diagPtr))
      {
        DiagCollector& diag = *diagPtr;

        diag << "      LET PASS FOR COMMAND PRICING" << std::endl;
        diag.flushMsg();
      }
    }

    ret = true;
  }

  return ret;
}

bool
RuleController::ifSkipCat15Security(const PaxTypeFare& paxTypeFare, uint16_t category, bool isFD)
    const
{
  bool skipCat15Security = false;
  Indicator fcaDisplayCatType = paxTypeFare.fcaDisplayCatType();

  if (UNLIKELY(isFD)) // FD
  {
    if (skipCat15ForSellingNetFare(fcaDisplayCatType))
    {
      skipCat15Security = true;
    }
  }
  else if ((paxTypeFare.isNegotiated() &&
            category == RuleConst::SALE_RESTRICTIONS_RULE) || // not FD
           skipCat15ForSellingNetFare(fcaDisplayCatType))
  {
    skipCat15Security = true;
  }

  return skipCat15Security;
}

//----------------------------------------------------------------------------
// passCategory()
//----------------------------------------------------------------------------
bool
RuleController::passCategory(uint16_t categoryNumber,
                             Record3ReturnTypes retResultOfRule,
                             PaxTypeFare& paxTypeFare)
{
  switch (retResultOfRule)
  {
  case PASS:
    return true;

  case SOFTPASS:
    return true;

  case FAIL:
    return false;

  case NOTPROCESSED: // Shopping Qualifycat4 not yet process
    return true;

  case SKIP:
    if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
    {
      if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF) // isPrivate()?
      {
        if (paxTypeFare.hasCat35Filed())
        {
          if (paxTypeFare.cat35FailIf1OfThen15())
          {
            return false;
          }
          return true;
        }

        Indicator fcaDisplayCatType = paxTypeFare.fcaDisplayCatType();
        if (fcaDisplayCatType == RuleConst::SELLING_FARE ||
            fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE ||
            fcaDisplayCatType == RuleConst::NET_SUBMIT_FARE_UPD)
        //&&
        //!paxTypeFare.isFareByRule())
        {
          return true; // security will be done in Cat35 process.
        }

        if (UNLIKELY(!paxTypeFare.isFareByRule()))
        {
          return false;
        }

        const PaxTypeFareRuleData* paxTypeFareRuleData =
            paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE);

        if (UNLIKELY(!paxTypeFareRuleData))
        {
          return false;
        }

        const std::vector<CategoryRuleItemInfo>* criiVec =
            paxTypeFareRuleData->categoryRuleItemInfoVec();

        if (criiVec == nullptr || criiVec->empty())

        {
          return false;
        }

        std::vector<CategoryRuleItemInfo>::const_iterator i = criiVec->begin();
        std::vector<CategoryRuleItemInfo>::const_iterator j = criiVec->end();
        for (; i != j; ++i)
        {
          if (UNLIKELY(i->itemcat() == RuleConst::SALE_RESTRICTIONS_RULE))
            return true; // IF CAT 15 of THEN CAT 25 will be validated
          // thru Cat 25 qualifiers validation
        }
        return false;
      }
    }
    return true;

  default:
    return false;
  }
}

//----------------------------------------------------------------------------
// needFootnoteRule()
//----------------------------------------------------------------------------
bool
RuleController::needFootnoteRule(uint16_t categoryNumber) const
{
  const std::set<uint16_t>& categoryNotFootnote = RuleConst::CATEGORY_NOT_FOOTNOTE;
  return categoryNotFootnote.find(categoryNumber) == categoryNotFootnote.end();
}

bool
RuleController::needFootnoteRule(const PricingTrx& trx,
                                 uint16_t categoryNumber,
                                 const RuleControllerDataAccess& da,
                                 Record3ReturnTypes& retResultOfRule) const
{
  if (fallback::fallbackFootNotePrevalidationForAltDates(&trx))
    return needFootnoteRule(categoryNumber, da, retResultOfRule);

  if (_fmPhase)
  {
    // This is list of footnote categories that need to be validated here, dispite of footnote
    // prevalidation.
    // Also, for AltDates, only non date related categories can have footnote rules prevalidated.
    if ((categoryNumber == RuleConst::SEASONAL_RULE &&
         UNLIKELY(da.isSitaVendor() || trx.isAltDates())) ||
        categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE ||
        (categoryNumber == RuleConst::BLACKOUTS_RULE && UNLIKELY(trx.isAltDates())) ||
        categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::MISC_FARE_TAG)
      return true;
  }
  else
  {
    if (categoryNumber == RuleConst::SEASONAL_RULE ||
        categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE ||
        categoryNumber == RuleConst::BLACKOUTS_RULE ||
        categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
      return true;
  }

  if (da.paxTypeFare().fare()->footnoteRec2Status(categoryNumber))
    retResultOfRule = PASS;

  return false;
}

bool
RuleController::needFootnoteRule(uint16_t categoryNumber,
                                 const RuleControllerDataAccess& da,
                                 Record3ReturnTypes& retResultOfRule) const
{
  if (_fmPhase)
  {
    if ((da.isSitaVendor() && categoryNumber == RuleConst::SEASONAL_RULE) ||
        categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE ||
        categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::MISC_FARE_TAG)
      return true;
  }
  else
  {
    if (categoryNumber == RuleConst::SEASONAL_RULE ||
        categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE ||
        categoryNumber == RuleConst::BLACKOUTS_RULE ||
        categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
      return true;
  }

  if (da.paxTypeFare().fare()->footnoteRec2Status(categoryNumber))
    retResultOfRule = PASS;

  return false;
}

//----------------------------------------------------------------------------
// needFareRule()
//----------------------------------------------------------------------------
bool
RuleController::needFareRule(const uint16_t categoryNumber,
                             Record3ReturnTypes retRule,
                             RuleControllerParam& rcParam)
{
  if (!rcParam._checkFareRuleAltGenRule)
  {
    if (categoryNumber == RuleConst::BLACKOUTS_RULE ||
        categoryNumber == RuleConst::SURCHARGE_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
    {
      return true; // need to continue
    }
    return false;
  }

  switch (retRule)
  {
  case PASS:
    if (categoryNumber == RuleConst::BLACKOUTS_RULE ||
        categoryNumber == RuleConst::SURCHARGE_RULE ||
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
    {
      return true; // need to continue
    }
    return false;

  case FAIL:
    return false;

  case SKIP:
    return true;

  default:
    return false;
  }
}

//----------------------------------------------------------------------------
// needGeneralRule()
//----------------------------------------------------------------------------
bool
RuleController::needGeneralRule(PricingTrx& trx,
                                const uint16_t categoryNumber,
                                Record3ReturnTypes retRule,
                                RuleControllerParam& rcParam)
{
  if (!rcParam._checkFareRuleAltGenRule)
  {
    return false;
  }

  switch (retRule)
  {
  case PASS:
  {
    if (categoryNumber == RuleConst::STOPOVER_RULE)
      return false;

    // some GRule category need to be processed even if FareRule is passed.
    const std::set<uint16_t>& categoryGenRule = RuleConst::CATEGORY_GRULE_PROCESS;
    return categoryGenRule.find(categoryNumber) != categoryGenRule.end();
  }

  case SOFTPASS:
    return categoryNumber == RuleConst::TRANSFER_RULE;

  case FAIL:
    return false;

  case SKIP:
    return true;

  default:
    return false;
  }
}

//
// get from PaxTypeFareRuleData or db
// if from db, store in PaxTypeFareRuleData
//
const GeneralFareRuleInfo*
RuleController::findFareRule(PricingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             GeneralFareRuleInfoVec& gfrInfoVec,
                             bool& isLocationSwapped,
                             const uint16_t categoryNumber)
{
  const GeneralFareRuleInfo* gfrRuleInfo = nullptr;

  if (fallback::fixed::fallbackDisableESVIS())
  {
  BindingResult bindingResult(hasCat(trx, paxTypeFare, categoryNumber, FARERULE));
  if (UNLIKELY(bindingResult.first && !bindingResult.second))
  {
    return nullptr;
  }
  // already found?
  }
  PaxTypeFareRuleData* paxTypeFareRuleData = nullptr;

  if (paxTypeFare.alreadyChkedFareRule(categoryNumber, trx.dataHandle(), paxTypeFareRuleData))
  {
    if (paxTypeFareRuleData != nullptr)
    {
      gfrRuleInfo =
          dynamic_cast<const GeneralFareRuleInfo*>(paxTypeFareRuleData->categoryRuleInfo());
      if (LIKELY(gfrRuleInfo != nullptr))
      {
        isLocationSwapped = paxTypeFareRuleData->isLocationSwapped();
        gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
      }
    }
  }
  // if not, get from db
  else
  {
    RuleUtil::getGeneralFareRuleInfo(trx, paxTypeFare, categoryNumber, gfrInfoVec);
    bool isLocationSwapped = false;
    if (gfrInfoVec.size() > 0)
    {
      gfrRuleInfo = gfrInfoVec.front().first;
      isLocationSwapped = gfrInfoVec.front().second;
    }

    // save it for next time
    paxTypeFare.setCatRuleInfo(
        gfrRuleInfo, categoryNumber, trx.dataHandle(), isLocationSwapped, true);
  }

  return gfrRuleInfo;
} // findFareRule

//
// get from FareMarket saved list or db
// and store in PaxTypeFareRuleData and FareMarket (if got from db)
//
template <typename FallBackSwitch>
const GeneralFareRuleInfo*
RuleController::findGnFareRuleOnFareMarket(
    PricingTrx& trx,
    PaxTypeFare& paxTypeFare,
    const TariffNumber& ruleTariff,
    const RuleNumber& ruleNumber,
    const uint16_t categoryNumber,
    GeneralFareRuleInfoVec& gfrInfoVec,
    bool& isLocationSwapped,
    const bool isFareRule,
    const FareMarket::FareMarketSavedGfrResult::Result*& savedRet,
    FareMarket::FareMarketSavedGfrResult::Results*& savedResults,
    Itin* itin,
    bool skipCat15Security)
{
  const GeneralFareRuleInfo* gfrRuleInfo = nullptr;

  // try to reuse general rule in same faremarket,
  FareMarket& fareMarket = *paxTypeFare.fareMarket();

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  GeneralFareRuleInfoVec* savedGfrList = nullptr;
  std::vector<FareMarket::FareMarketSavedGfrResult::Results>* savedRetVec = nullptr;
  const FareMarket::FareMarketSavedGfrResult::Result* preSavedRet = nullptr;
  FareMarket::FareMarketSavedGfrResult::Results* preSavedResults = nullptr;

  const VendorCode& vendor = paxTypeFare.vendor();

  const DateTime& travelDate = trx.adjustedTravelDate(itin->travelSeg().front()->departureDT());

  const DateTime& returnDate = (trx.isAltDates() && (trx.getTrxType() == PricingTrx::IS_TRX ||
                                                     trx.getTrxType() == PricingTrx::FF_TRX) &&
                                itin->datePair() != nullptr)
                                   ? itin->datePair()->second
                                   : DateTime::emptyDate();

  const DateTime& ticketDate =
      trx.matchFareRetrievalDate(paxTypeFare) ? trx.ticketingDate() : paxTypeFare.retrievalDate();

  savedGfrList = fareMarket.getGfrList(vendor,
                                       carrier,
                                       ruleTariff,
                                       ruleNumber,
                                       categoryNumber,
                                       travelDate.date(),
                                       returnDate.date(),
                                       ticketDate,
                                       savedRetVec);

  if (!savedGfrList && (_categoryPhase != DynamicValidation))
  {
    trx.dataHandle().get(savedGfrList);
    if (fallback::fallbackGfrR2Optimization(&trx))
    {
      // did not see from saved list, get from database/cache

    const DateTime& ruleApplicationDate =
        (isFareRule && (categoryNumber == RuleConst::VOLUNTARY_EXCHANGE_RULE ||
                        categoryNumber == RuleConst::VOLUNTARY_REFUNDS_RULE))
            ? paxTypeFare.fareMarket()->ruleApplicationDate()
            : DateTime::emptyDate();

    const std::vector<GeneralFareRuleInfo*>& gfrListFromDB = trx.dataHandle().getGeneralFareRule(
        vendor, carrier, ruleTariff, ruleNumber, categoryNumber, travelDate, ruleApplicationDate);

    *savedGfrList = dbToLocVecAdaptor(gfrListFromDB);
    }
    else
    {
      *savedGfrList = Rec2Selector::getGfr(
          trx, paxTypeFare, ruleTariff, ruleNumber, categoryNumber, travelDate);
    }

    // save the list into faremarket
    savedRetVec = fareMarket.saveGfrUMList(trx.dataHandle(),
                                           paxTypeFare.vendor(),
                                           carrier,
                                           ruleTariff,
                                           ruleNumber,
                                           categoryNumber,
                                           travelDate.date(),
                                           returnDate.date(),
                                           ticketDate,
                                           savedGfrList);
  }

  if (!savedGfrList || !savedRetVec)
  {
    return nullptr;
  }

  bool isCat25PrivateBasePublic = false;
  if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE && !skipCat15Security &&
      paxTypeFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF && paxTypeFare.cat25Fare() &&
      paxTypeFare.cat25Fare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    isCat25PrivateBasePublic = true;

  // find the match
  uint16_t index = 0;
  auto gfrIt = savedGfrList->begin();
  auto gfrEndIt = savedGfrList->end();
  bool findR2 = false;
  bool stopFORLoop = false;
  bool isHistorical = trx.dataHandle().isHistorical();
  DateTime& tktDate = trx.ticketingDate();

  FallBackSwitch fallBackSwitch;

  for (; gfrIt != gfrEndIt; gfrIt++, index++)
  {
    isLocationSwapped = false;
    if (fallBackSwitch(trx, paxTypeFare, *gfrIt, isLocationSwapped))
    {
      gfrRuleInfo = gfrIt->first;
      if (LIKELY(!findR2))
      {
        findR2 = true;
        gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
        if (LIKELY(!isHistorical ||
                   !RuleUtil::matchCreateOrExpireDate(
                       tktDate, gfrRuleInfo->createDate(), gfrRuleInfo->expireDate())))
        {
          stopFORLoop = true;
        }
      }
      else
      {
        if (!RuleUtil::matchCreateOrExpireDate(
                tktDate, gfrRuleInfo->createDate(), gfrRuleInfo->expireDate()))
        {
          stopFORLoop = true;
        }
        gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
      }

      bool chkPsgEligibility = (categoryNumber == RuleConst::ELIGIBILITY_RULE) ||
                               isThereQualifier(*gfrRuleInfo, RuleConst::ELIGIBILITY_RULE);

      bool doNotReuseResult = false;

      doNotReuseResult = !_reuseResult;

      if (fallback::fallbackValCxrR2Cat15(&trx))
      {
        _loopValidatingCxrs = _categoryPhase != DynamicValidation &&
                              !_highLevelLoopValidatingCxrs && trx.isValidatingCxrGsaApplicable() &&
                              !paxTypeFare.fareMarket()->validatingCarriers().empty() &&
                              existCat15QualifyCat15ValidatingCxrRest(trx, *gfrRuleInfo);

        if (RuleController::doNotReuseGrFareRuleResult(
                trx, categoryNumber, chkPsgEligibility, skipCat15Security) ||
            isCat25PrivateBasePublic)
        {
          doNotReuseResult = true;
        }
        else if (_loopValidatingCxrs || _highLevelLoopValidatingCxrs)
          doNotReuseResult = true;
      }
      else
      {
        if (!doNotReuseResult && (RuleController::doNotReuseGrFareRuleResult(
                                      trx, categoryNumber, chkPsgEligibility, skipCat15Security) ||
                                  isCat25PrivateBasePublic || _highLevelLoopValidatingCxrs))
        {
          doNotReuseResult = true;
        }
        if (!doNotReuseResult)
        {
          _loopValidatingCxrs = _categoryPhase != DynamicValidation &&
                                !_highLevelLoopValidatingCxrs &&
                                trx.isValidatingCxrGsaApplicable() &&
                                !paxTypeFare.fareMarket()->validatingCarriers().empty() &&
                                existCat15QualifyCat15ValidatingCxrRest(trx, *gfrRuleInfo);
          if (_loopValidatingCxrs)
            doNotReuseResult = true;
        }
      }

      if (doNotReuseResult || trx.getRequest()->isSFR())
      {
        // do not reuse the result
        savedRet = nullptr;
        savedResults = nullptr;
        if (UNLIKELY(trx.dataHandle().isHistorical()))
        {
          if (!trx.isValidatingCxrGsaApplicable() || !stopFORLoop)
          {
            continue;
          }
        }

        break;
      }

      savedResults = &((*savedRetVec)[index]);

      {
        FMScopedLock guard(savedResults->resultMapMutex());

        if (savedResults->resultMap().size() == 0)
        {
          // flags for reusability are not set yet, we will set them
          savedResults->setDirectional(isDirectional((const CategoryRuleInfo&)*gfrRuleInfo));
          if (chkPsgEligibility || categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
          {
            savedResults->setChkPsgType(true);
            savedResults->setChkPsgAge(true);
          }
        }

      } // end guard scope

      savedRet = fareMarket.savedGfrResult(categoryNumber, paxTypeFare, *savedResults);

      // save ret and results for historical, fare rule and pricing
      if (LIKELY(findR2))
      {
        preSavedRet = savedRet;
        preSavedResults = savedResults;
        findR2 = false;
      }
      if (LIKELY(stopFORLoop))
      {
        break;
      }
    }
  }

  Diag202Collector* dc =
      Diag202Collector::makeMe(trx, &Diag202Collector::GFR, paxTypeFare.fareMarket(), &paxTypeFare);
  if (UNLIKELY(dc))
  {
    dc->printR2sMatchDetails<GeneralFareRuleInfo>(FB_GENERAL_RULE_RECORD_2,
                                                  trx,
                                                  locToDbVecAdaptor(*savedGfrList),
                                                  gfrInfoVec,
                                                  *paxTypeFare.fareMarket(),
                                                  carrier,
                                                  travelDate,
                                                  &paxTypeFare);
  }

  //  restore save ret and results
  if (UNLIKELY(trx.dataHandle().isHistorical() && gfrInfoVec.size() > 0))
  {
    savedRet = preSavedRet;
    savedResults = preSavedResults;
  }
  if (!gfrInfoVec.empty())
  {
    gfrRuleInfo = gfrInfoVec.front().first;
    isLocationSwapped = gfrInfoVec.front().second;
    if (savedRet && (savedRet->_ret != NOTPROCESSED))
    {
      if (_categoryPhase == DynamicValidation &&
          ((categoryNumber == RuleConst::STOPOVER_RULE && savedRet->_ret != FAIL) ||
           categoryNumber == RuleConst::TRANSFER_RULE)) // Do not reuse result for Cat8/9 for
      // DynamicValidation (unless it is fail for
      // Cat8)
      {
        savedRet = nullptr;
        savedResults = nullptr;
        return gfrRuleInfo;
      }

      if ((savedRet->_ret == SOFTPASS) &&
          (_categoryPhase != DynamicValidation)) // Do not update PaxTypeFare in DynamicValidation,
      // multi-thread issue
      {
        paxTypeFare.setCategorySoftPassed(categoryNumber);
      }

      if (!fallback::fallbackFlexFareCopyValidationStatusForCorpIdAccCode(&trx))
      {
        // NGSII-741: Restrict copying all categories except Acc Code/Corp Id in Validation Status
        copyFlexFaresValidationStatus(trx, paxTypeFare, *savedRet->_ptfResultFrom, categoryNumber);
      }

      if (categoryNumber == RuleConst::ADVANCE_RESERVATION_RULE &&
          _categoryPhase != DynamicValidation)
      {
        paxTypeFare.reProcessCat05NoMatch() = savedRet->_reProcessCat05NoMatch;
      }

      if (UNLIKELY(TrxUtil::isFVOSurchargesEnabled() &&
                   categoryNumber == RuleConst::SURCHARGE_RULE))
      {
        // do not reuse FVO surcharges for now
        //          if(_categoryPhase == DynamicValidation)
        //          {
        savedRet = nullptr;
        savedResults = nullptr;
        return gfrRuleInfo;
        //          }

        //          if(!savedRet->_ptfResultFrom->needRecalculateCat12())
        //          {
        //              paxTypeFare.needRecalculateCat12() = false;
        //
        //              if(!savedRet->_ptfResultFrom->surchargeData().empty())
        //                  paxTypeFare.surchargeData() = savedRet->_ptfResultFrom->surchargeData();
        //          }
      }

      if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
        diagReuse(trx,
                  gfrRuleInfo,
                  paxTypeFare,
                  categoryNumber,
                  vendor,
                  carrier,
                  ruleTariff,
                  &ruleNumber,
                  nullptr,
                  isFareRule ? 'R' : 'G',
                  savedRet->_ptfResultFrom->fareClass(),
                  savedRet->_ret);
    }
  }

  return gfrRuleInfo;
}

bool
RuleController::doNotReuseGrFareRuleResult(PricingTrx& trx,
                                           const uint16_t categoryNumber,
                                           const bool chkPsgEligibility,
                                           const bool skipCat15Security)
{
  return (chkPsgEligibility && trx.getOptions()->isWeb()) ||
         (categoryNumber == 15 && skipCat15Security) ||
         categoryNumber == RuleConst::TICKET_ENDORSMENT_RULE;
}

template <typename FallBackSwitch>
const FootNoteCtrlInfo*
RuleController::findFnCtrlInfoOnFareMarket(
    PricingTrx& trx,
    PaxTypeFare& paxTypeFare,
    FootNoteCtrlInfoVec& fnCtrlInfoVec,
    bool& isLocationSwapped,
    const TariffNumber& ruleTariff,
    const Footnote& footnote,
    const uint16_t& fnIndex,
    const uint16_t categoryNumber,
    const FareMarket::FareMarketSavedFnResult::Result*& savedRet,
    FareMarket::FareMarketSavedFnResult::Results*& savedResults,
    Itin* itin,
    bool skipCat15Security)
{
  // try to reuse footnote rule in same faremarket,
  FareMarket& fareMarket = *paxTypeFare.fareMarket();

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  FootNoteCtrlInfoVec* savedFnList = nullptr;
  std::vector<FareMarket::FareMarketSavedFnResult::Results>* savedRetVec = nullptr;

  const VendorCode& vendor = paxTypeFare.vendor();
  const DateTime& travelDate = trx.adjustedTravelDate(itin->travelSeg().front()->departureDT());
  const DateTime& returnDate = (trx.isAltDates() && (trx.getTrxType() == PricingTrx::IS_TRX ||
                                                     trx.getTrxType() == PricingTrx::FF_TRX) &&
                                itin->datePair() != nullptr)
                                   ? itin->datePair()->second
                                   : DateTime::emptyDate();

  const DateTime& ticketDate =
      trx.matchFareRetrievalDate(paxTypeFare) ? trx.ticketingDate() : paxTypeFare.retrievalDate();

  savedFnList =
      const_cast<FootNoteCtrlInfoVec*>(paxTypeFare.fareMarket()->getFnCtrlList(vendor,
                                                                               carrier,
                                                                               ruleTariff,
                                                                               footnote,
                                                                               categoryNumber,
                                                                               travelDate.date(),
                                                                               returnDate.date(),
                                                                               ticketDate,
                                                                               savedRetVec));

  if (!savedFnList && (_categoryPhase != DynamicValidation))
  {
    trx.dataHandle().get(savedFnList);

    if (fallback::fallbackFootNoteR2Optimization(&trx))
    {
      // did not see from saved list, get from database/cache
      std::vector<FootNoteCtrlInfo*> fnListFromDB = trx.dataHandle().getFootNoteCtrl(
          paxTypeFare.vendor(), carrier, ruleTariff, footnote, categoryNumber, travelDate);

      if (_LIKELY(savedFnList))
      {
        *savedFnList = dbToLocVecAdaptor(fnListFromDB);
      }
    }
    else
    {
      *savedFnList = Rec2Selector::getFootNoteCtrl(
          trx, paxTypeFare, ruleTariff, footnote, categoryNumber, travelDate);
    }
    // save the list into faremarket
    savedRetVec = fareMarket.saveFnCtlUMLst(trx.dataHandle(),
                                            paxTypeFare.vendor(),
                                            carrier,
                                            ruleTariff,
                                            footnote,
                                            categoryNumber,
                                            travelDate.date(),
                                            returnDate.date(),
                                            ticketDate,
                                            savedFnList);
  }

  if (savedFnList && !savedFnList->empty())
  {
    paxTypeFare.fare()->setFoundFootnote(fnIndex);
  }

  if (!savedFnList || !savedRetVec)
  {
    return nullptr;
  }

  bool isCat25PrivateBasePublic = false;
  if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE && !skipCat15Security &&
      paxTypeFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF && paxTypeFare.cat25Fare() &&
      paxTypeFare.cat25Fare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    isCat25PrivateBasePublic = true;

  // find the match
  const FootNoteCtrlInfo* fnCtrlInfo = nullptr;

  uint16_t index = 0;
  auto fnIt = savedFnList->begin();
  auto fnEndIt = savedFnList->end();

  bool stopFORLoop = false;
  bool foundRec2 = false;
  bool isHistorical = trx.dataHandle().isHistorical();
  DateTime& tktDate = trx.ticketingDate();

  FallBackSwitch fallBackSwitch;

  for (; fnIt != fnEndIt; fnIt++, index++)
  {
    if (fallBackSwitch(trx, *fnIt, paxTypeFare, isLocationSwapped, footnote, ruleTariff))
    {
      fnCtrlInfo = fnIt->first;
      bool chkPsgEligibility = (categoryNumber == RuleConst::ELIGIBILITY_RULE) ||
                               isThereQualifier(*fnCtrlInfo, RuleConst::ELIGIBILITY_RULE);

      bool doNotReuseResult = false;
      doNotReuseResult = !_reuseResult;

      _loopValidatingCxrs = _categoryPhase != DynamicValidation && !_highLevelLoopValidatingCxrs &&
                            trx.isValidatingCxrGsaApplicable() &&
                            !paxTypeFare.fareMarket()->validatingCarriers().empty() &&
                            existCat15QualifyCat15ValidatingCxrRest(trx, *fnCtrlInfo);

      if (UNLIKELY(chkPsgEligibility && trx.getOptions()->isWeb()))
      {
        doNotReuseResult = true;
      }

      else if (UNLIKELY(_loopValidatingCxrs || _highLevelLoopValidatingCxrs))
        doNotReuseResult = true;

      // To prevent Cat 25 fare which is L/C/T and has Cat 15 override tag B to
      // reuse the result of its base fare that failed Cat 15 security.
      else if (categoryNumber == 15 && skipCat15Security)
      {
        doNotReuseResult = true;
      }
      // To prevent Cat 25 private fare with base public fare to
      // reuse the result of its base fare Cat 15 security check.
      else if (isCat25PrivateBasePublic)
      {
        doNotReuseResult = true;
      }

      if (LIKELY(!foundRec2))
      {
        fnCtrlInfoVec.emplace_back(fnCtrlInfo, GeoMatchResult(isLocationSwapped));
        foundRec2 = true;
        if (LIKELY(!isHistorical ||
                   !RuleUtil::matchCreateOrExpireDate(
                       tktDate, fnCtrlInfo->createDate(), fnCtrlInfo->expireDate())))
          stopFORLoop = true;
      }
      else
      {
        if (!RuleUtil::matchCreateOrExpireDate(
                tktDate, fnCtrlInfo->createDate(), fnCtrlInfo->expireDate()))
          stopFORLoop = true;
        fnCtrlInfoVec.emplace_back(fnCtrlInfo, GeoMatchResult(isLocationSwapped));
      }

      if (trx.getRequest() && trx.getRequest()->isSFR())
      {
        doNotReuseResult = true;
      }

      if (doNotReuseResult)
      {
        // do not reuse the result
        savedRet = nullptr;
        savedResults = nullptr;
        break;
      }

      savedResults = &((*savedRetVec)[index]);

      {
        FMScopedLock guard(savedResults->resultMapMutex());
        if (savedResults->resultMap().size() == 0)
        {
          savedResults->setDirectional(isDirectional((const CategoryRuleInfo&)*fnCtrlInfo));
          if (UNLIKELY(chkPsgEligibility || categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE))
          {
            savedResults->setChkPsgType(true);
            savedResults->setChkPsgAge(true);
          }
        }

      } // end guard scope

      savedRet = fareMarket.savedFnResult(categoryNumber, paxTypeFare, *savedResults);

      break;
    }
    if (UNLIKELY(stopFORLoop))
      break;
  }

  Diag202Collector* dc =
      Diag202Collector::makeMe(trx, &Diag202Collector::FNT, paxTypeFare.fareMarket(), &paxTypeFare);
  if (UNLIKELY(dc))
  {
    dc->printR2sMatchDetails<FootNoteCtrlInfo>(FB_FOOTNOTE_RECORD_2,
                                               trx,
                                               locToDbVecAdaptor(*savedFnList),
                                               fnCtrlInfoVec,
                                               *paxTypeFare.fareMarket(),
                                               carrier,
                                               travelDate,
                                               &paxTypeFare);
  }

  if (LIKELY(trx.isFootNotePrevalidationEnabled()))
  {
    if (savedRet && categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE &&
        savedRet->_cat14NVAData)
    {
      // do not reuse
      savedRet = nullptr;
      savedResults = nullptr;
    }
  }
  else
  {
    if (savedRet && categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE &&
        savedRet->_ptfResultFrom->getNVAData())
    {
      // do not reuse
      savedRet = nullptr;
      savedResults = nullptr;
    }
  }

  if (UNLIKELY(fnCtrlInfoVec.size() > 1))
  {
    savedRet = nullptr;
    savedResults = nullptr;
  }

  if (!fnCtrlInfoVec.empty())
  {
    fnCtrlInfo = fnCtrlInfoVec.front().first;
    isLocationSwapped = fnCtrlInfoVec.front().second;
  }

  if (savedRet && (savedRet->_ret != NOTPROCESSED))
  {
    if (UNLIKELY(
            ((_categoryPhase == DynamicValidation) &&
             ((categoryNumber == RuleConst::STOPOVER_RULE) ||
              (categoryNumber == RuleConst::TRANSFER_RULE)) &&
             (savedRet->_ret != FAIL)) // Do not reuse result for Cat8/9 for DynamicValidation
                                       // unless it
            // is fail
            ||
            (TrxUtil::isFVOSurchargesEnabled() && categoryNumber == RuleConst::SURCHARGE_RULE)))
    {
      savedRet = nullptr;
      savedResults = nullptr;
      return fnCtrlInfo;
    }

    // do not reuse FVO surcharges for now
    //      if (categoryNumber == RuleConst::SURCHARGE_RULE)
    //          paxTypeFare.surchargeData() = savedRet->_ptfResultFrom->surchargeData();

    if (UNLIKELY(trx.diagnostic().diagnosticType() != DiagnosticNone))
    {
      if (LIKELY(trx.isFootNotePrevalidationEnabled()))
        diagReuse(trx,
                  fnCtrlInfo,
                  paxTypeFare,
                  categoryNumber,
                  vendor,
                  carrier,
                  ruleTariff,
                  nullptr,
                  &footnote,
                  'F',
                  savedRet->_fare->fareClass(),
                  savedRet->_ret);
      else
        diagReuse(trx,
                  fnCtrlInfo,
                  paxTypeFare,
                  categoryNumber,
                  vendor,
                  carrier,
                  ruleTariff,
                  nullptr,
                  &footnote,
                  'F',
                  savedRet->_ptfResultFrom->fareClass(),
                  savedRet->_ret);
    }
  }

  return fnCtrlInfo;
}

//----------------------------------------------------------------------------
// processFootnoteRule()
//----------------------------------------------------------------------------
template <typename FallBackSwitch>
Record3ReturnTypes
RuleController::processFootnoteRule(PricingTrx& trx,
                                    RuleControllerDataAccess& da,
                                    uint16_t categoryNumber,
                                    RuleControllerParam& rcParam,
                                    RuleProcessingData& rpData,
                                    bool skipCat15Security)
{
  // Shortcut
  PaxTypeFare& paxTypeFare = da.paxTypeFare();
  Itin* itin = da.itin();
  const bool isFareDisplayTrx = (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX);

  if (UNLIKELY(isFareDisplayTrx))
  {
    updateRuleType(paxTypeFare, RuleConst::FOOT_NOTE);
  }

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  Record3ReturnTypes finalRet = SKIP;

  std::vector<FootnoteTable>::iterator fnTblIter = da.footNoteTbl().begin();
  std::vector<FootnoteTable>::iterator fnTblIterEnd = da.footNoteTbl().end();
  uint16_t fnIndex = 0;

  // all the hassle just to avoid fallback check inside loop,
  // can be removed with fallback and when historical handling added
  FallBackSwitch fallBackSwitch;

  for (; fnTblIter != fnTblIterEnd; fnTblIter++, fnIndex++)
  {
    Record3ReturnTypes ret = SKIP;
    FootNoteCtrlInfoVec fnCtrlInfoVec;
    const FootNoteCtrlInfo* fnCtrlInfo = nullptr;
    bool foundMatchedFnCtrlInfo = false;
    bool isLocationSwapped = false;
    const FareMarket::FareMarketSavedFnResult::Result* savedRet = nullptr;
    FareMarket::FareMarketSavedFnResult::Results* savedResults = nullptr;

    // for validation on FareMarket, we can reuse validation result
    // of same record2 on same FareMarket
    if (checkIfShouldFindFnCtrlInfo(categoryNumber, isFareDisplayTrx))
    {
      fnCtrlInfo = findFnCtrlInfoOnFareMarket<FallBackSwitch>(trx,
                                                              paxTypeFare,
                                                              fnCtrlInfoVec,
                                                              isLocationSwapped,
                                                              fnTblIter->tariffNumber(),
                                                              fnTblIter->footNote(),
                                                              fnIndex,
                                                              categoryNumber,
                                                              savedRet,
                                                              savedResults,
                                                              itin,
                                                              skipCat15Security);
      if (fnCtrlInfo != nullptr)
      {
        foundMatchedFnCtrlInfo = true;
      }
    }
    else
    {
      if (checkIfShouldFindFnCtrlInfoForDynamicValidation(
              paxTypeFare, categoryNumber, isFareDisplayTrx))
      {
        // for DynamicValidation, only reuse result but not to save new result
        //     do not reuse result for the selected Cat35 Net Remit fare.
        fnCtrlInfo = findFnCtrlInfoOnFareMarket<FallBackSwitch>(trx,
                                                                paxTypeFare,
                                                                fnCtrlInfoVec,
                                                                isLocationSwapped,
                                                                fnTblIter->tariffNumber(),
                                                                fnTblIter->footNote(),
                                                                fnIndex,
                                                                categoryNumber,
                                                                savedRet,
                                                                savedResults,
                                                                itin,
                                                                skipCat15Security);
      }

      if (fnCtrlInfo != nullptr)
      {
        foundMatchedFnCtrlInfo = true;
      }
      else
      {
        // Do the regular FootNote matching and processing
        const DateTime& travelDate =
            trx.adjustedTravelDate(itin->travelSeg().front()->departureDT());

        FootNoteCtrlInfoVec ftnList;

        if (fallback::fallbackFootNoteR2Optimization(&trx))
        {
        // retrieve a vector of FootnoteRule records
          std::vector<FootNoteCtrlInfo*> fnListFromDB =
              trx.dataHandle().getFootNoteCtrl(paxTypeFare.vendor(),
                                               carrier,
                                               fnTblIter->tariffNumber(),
                                               fnTblIter->footNote(),
                                               categoryNumber,
                                               travelDate);
          ftnList = dbToLocVecAdaptor(fnListFromDB);
        }
        else
        {
          ftnList = Rec2Selector::getFootNoteCtrl(trx,
                                                  paxTypeFare,
                                                  fnTblIter->tariffNumber(),
                                                  fnTblIter->footNote(),
                                                  categoryNumber,
                                                  travelDate);
        }

        if (!ftnList.empty())
        {
          paxTypeFare.fare()->setFoundFootnote(fnIndex);
        }

        // scan the vector and find the first match
        auto bIt = ftnList.begin();
        auto bItEnd = ftnList.end();
        //  NEW
        bool isHistorical = trx.dataHandle().isHistorical();
        bool foundRec2 = false;
        DateTime& tktDate = trx.ticketingDate();
        for (; bIt != bItEnd; bIt++)
        {
          fnCtrlInfo = bIt->first;

          if (fallBackSwitch(trx,
                             *bIt,
                             paxTypeFare,
                             isLocationSwapped,
                             fnTblIter->footNote(),
                             fnTblIter->tariffNumber()))
          {
            foundMatchedFnCtrlInfo = true;
            if (LIKELY(!foundRec2))
            {
              fnCtrlInfoVec.emplace_back(fnCtrlInfo, GeoMatchResult(isLocationSwapped));
              foundRec2 = true;
              if (LIKELY(!isHistorical ||
                         !RuleUtil::matchCreateOrExpireDate(
                             tktDate, fnCtrlInfo->createDate(), fnCtrlInfo->expireDate())))
                break;
            }
            else
            {
              if (!RuleUtil::matchCreateOrExpireDate(
                      tktDate, fnCtrlInfo->createDate(), fnCtrlInfo->expireDate()))
                break;
              fnCtrlInfoVec.emplace_back(fnCtrlInfo, GeoMatchResult(isLocationSwapped));
            }
          }
        }

        if (!fnCtrlInfoVec.empty())
        {
          fnCtrlInfo = fnCtrlInfoVec.front().first;
          isLocationSwapped = fnCtrlInfoVec.front().second;
        }

        Diag202Collector* dc = Diag202Collector::makeMe(
            trx, &Diag202Collector::FNT, paxTypeFare.fareMarket(), &paxTypeFare);
        if (UNLIKELY(dc))
        {
          dc->printR2sMatchDetails<FootNoteCtrlInfo>(FB_FOOTNOTE_RECORD_2,
                                                     trx,
                                                     locToDbVecAdaptor(ftnList),
                                                     fnCtrlInfoVec,
                                                     *paxTypeFare.fareMarket(),
                                                     carrier,
                                                     travelDate,
                                                     &paxTypeFare);
        }
      }
    }
    if (foundMatchedFnCtrlInfo)
    {
      if (savedRet && (savedRet->_ret != NOTPROCESSED))
      {
        ret = savedRet->_ret;

        if (UNLIKELY(trx.noPNRPricing()))
        {
          const WarningMap& fromWarningMap = LIKELY(trx.isFootNotePrevalidationEnabled())
                                                 ? savedRet->_fare->warningMap()
                                                 : savedRet->_ptfResultFrom->warningMap();
          const WarningMap& toWarningMap = paxTypeFare.warningMap();

          reuseWarningMap(fromWarningMap, toWarningMap, categoryNumber);
        }

        if (fnCtrlInfo->applInd() == RuleConst::STRING_DOES_NOT_APPLY)
        {
          // stop processing for this category
          rcParam._checkFareRuleAltGenRule = false;
        }
        if ((ret != FAIL) && (_categoryPhase != DynamicValidation)) // Do not update PaxTypeFare in
        // DynamicValidation,
        // multi-thread issue
        {
          if (ret == SOFTPASS)
          {
            paxTypeFare.setCategorySoftPassed(categoryNumber);
          }

          if (LIKELY(trx.isFootNotePrevalidationEnabled()))
            copySavedValidationResult(paxTypeFare, categoryNumber, *savedRet);
          else
            copySavedValidationResult_deprecated(paxTypeFare, categoryNumber, *savedRet);
        }

        if (LIKELY(trx.isFootNotePrevalidationEnabled()))
          storeSalesRestrictionValuesFn(trx, paxTypeFare, categoryNumber, savedRet);
        else
          storeSalesRestrictionValuesFn_deprecated(trx, paxTypeFare, categoryNumber, savedRet);
      }
      else
      {
        const bool currentFOPFlag = categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE &&
                                    !paxTypeFare.fare()->forbiddenFop().isNull();

        savedRetNotProcessedFn(ret,
                               isLocationSwapped,
                               paxTypeFare,
                               trx,
                               categoryNumber,
                               fnCtrlInfo,
                               fnCtrlInfoVec,
                               da,
                               rpData,
                               rcParam,
                               skipCat15Security);

        // Save FootNote result into the faremarket for possible reuse
        if (LIKELY(trx.isFootNotePrevalidationEnabled()))
        {
          saveFnIntoFareMarket(paxTypeFare,
                               trx,
                               savedResults,
                               categoryNumber,
                               carrier,
                               ret,
                               *fnTblIter,
                               fnCtrlInfo,
                               currentFOPFlag);
        }
        else
        {
          saveFnIntoFareMarket_deprecated(paxTypeFare,
                                          trx,
                                          savedResults,
                                          categoryNumber,
                                          carrier,
                                          ret,
                                          *fnTblIter,
                                          fnCtrlInfo,
                                          currentFOPFlag);
        }
      }

      if (ret == FAIL)
      {
        if ((_categoryPhase == DynamicValidation) &&
            (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE))
        {
          if (paxTypeFare.cat15SoftPass())
            continue;
          else
            return FAIL;
        }

        return FAIL;
      }
      if (finalRet == SOFTPASS)
      {
        continue;
      }
      if ((ret == SOFTPASS) || (finalRet != PASS))
      {
        finalRet = ret;
      }
    } // foundMatchedFnCtrlInfo
  }

  return finalRet;
}

bool
RuleController::checkIfShouldFindFnCtrlInfo(uint16_t categoryNumber, bool isFareDisplayTrx) const
{
  return !skipReuse(categoryNumber) &&
         (_categoryPhase == FBRBaseFarePrevalidation ||
          _categoryPhase == FootNotePrevalidation || _categoryPhase == FootNotePrevalidationALT ||
          _categoryPhase == PreValidation || _categoryPhase == NormalValidation ||
          _categoryPhase == ShoppingComponentValidation ||
          _categoryPhase == ShoppingAcrossStopOverComponentValidation) &&
         categoryNumber < RuleConst::MISC_FARE_TAG &&
         categoryNumber != RuleConst::TICKET_ENDORSMENT_RULE && !isFareDisplayTrx && _reuseResult;
}

bool
RuleController::checkIfShouldFindFnCtrlInfoForDynamicValidation(const PaxTypeFare& paxTypeFare,
                                                                uint16_t categoryNumber,
                                                                bool isFareDisplayTrx) const
{
  return !skipReuse(categoryNumber) && _categoryPhase == DynamicValidation && _fmPhase &&
         !paxTypeFare.selectedFareNetRemit() && categoryNumber < RuleConst::MISC_FARE_TAG &&
         categoryNumber != RuleConst::TICKET_ENDORSMENT_RULE && !isFareDisplayTrx && _reuseResult;
}

void
RuleController::storeSalesRestrictionValuesFn_deprecated(
    PricingTrx& trx,
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedFnResult::Result* savedRet)
{
  // Set Cat15 security result
  if ((_categoryPhase != DynamicValidation) &&
      (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE))
  {
    paxTypeFare.setCat15SoftPass(savedRet->_cat15SoftPass);

    if (savedRet->_cat15SecurityFail)
    {
      paxTypeFare.setCat15SecurityFail();
    }
    // Set Cat15 latestTktDT result
    const DateTime& ltd = savedRet->_latestTktDT;
    paxTypeFare.fare()->latestTktDT().setWithEarlier(ltd);
    paxTypeFare.fare()->latestTktDTFootN().setWithEarlier(ltd);
    paxTypeFare.fare()->setElectronicTktable(!savedRet->_ptfResultFrom->isElectronicTktable());

    if (savedRet->_ptfResultFrom && savedRet->_ptfResultFrom->isCat15HasT996FT())
      paxTypeFare.setCat15HasT996FT(true);

    if (savedRet->_cat15FOPExist && paxTypeFare.fare()->forbiddenFop().isNull())
      paxTypeFare.fare()->mutableForbiddenFop() = savedRet->_ptfResultFrom->fare()->forbiddenFop();

    // Set Cat15 (nation in FR for CWT)
    if (savedRet->_cat15NationFRForCWT)
    {
      paxTypeFare.fare()->setNationFRInCat15Fn();
      paxTypeFare.fare()->setNationFRInCat15();
    }
    // Set Cat15 (Security is found in C15)
    if (savedRet->_cat15HasSecurity)
    {
      paxTypeFare.fare()->setSecurityInCat15Fn();
      paxTypeFare.fare()->setCat15HasSecurity();
    }
    // Set Cat15 (Etkt warning is found in C15)
    if (savedRet->_cat15HasEtktWarning)
    {
      paxTypeFare.fare()->setWarningEtktInCat15Fn();
      paxTypeFare.fare()->setWarningEtktInCat15();
    }
  }
}

void
RuleController::storeSalesRestrictionValuesFn(
    PricingTrx& trx,
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedFnResult::Result* savedRet)
{
  // Set Cat15 security result
  if ((_categoryPhase != DynamicValidation) &&
      (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE))
  {
    paxTypeFare.setCat15SoftPass(savedRet->_cat15SoftPass);

    if (UNLIKELY(savedRet->_cat15SecurityFail))
      paxTypeFare.setCat15SecurityFail();

    // Set Cat15 latestTktDT result
    const DateTime& ltd = savedRet->_fare->latestTktDTFootN();
    paxTypeFare.fare()->latestTktDT().setWithEarlier(ltd);
    paxTypeFare.fare()->latestTktDTFootN().setWithEarlier(ltd);
    paxTypeFare.fare()->setElectronicTktable(!savedRet->_fare->isElectronicTktable());

    if (savedRet->_cat15HasT996FT)
      paxTypeFare.setCat15HasT996FT(true);

    if (UNLIKELY(savedRet->_cat15FOPExist && paxTypeFare.fare()->forbiddenFop().isNull()))
      paxTypeFare.fare()->mutableForbiddenFop() = savedRet->_fare->forbiddenFop();

    // Set Cat15 (nation in FR for CWT)
    if (UNLIKELY(savedRet->_fare->isNationFRInCat15Fn()))
    {
      paxTypeFare.fare()->setNationFRInCat15Fn();
      paxTypeFare.fare()->setNationFRInCat15();
    }
    // Set Cat15 (Security is found in C15)
    if (UNLIKELY(savedRet->_fare->isSecurityInCat15Fn()))
    {
      paxTypeFare.fare()->setSecurityInCat15Fn();
      paxTypeFare.fare()->setCat15HasSecurity();
    }
    // Set Cat15 (Etkt warning is found in C15)
    if (UNLIKELY(savedRet->_fare->isWarningEtktInCat15Fn()))
    {
      paxTypeFare.fare()->setWarningEtktInCat15Fn();
      paxTypeFare.fare()->setWarningEtktInCat15();
    }
  }
}

void
RuleController::saveFnIntoFareMarket_deprecated(
    PaxTypeFare& paxTypeFare,
    PricingTrx& trx,
    FareMarket::FareMarketSavedFnResult::Results* savedResults,
    uint16_t categoryNumber,
    CarrierCode& carrier,
    Record3ReturnTypes ret,
    FootnoteTable& fnTblIter,
    const FootNoteCtrlInfo* fnCtrlInfo,
    bool currentFOPFlag)
{
  if (savedResults && (_categoryPhase != DynamicValidation))
  {
    const bool currentPenaltyRIndFlag = paxTypeFare.penaltyRestInd() == YES;
    FareMarket::FareMarketSavedFnResult::Result* savingRet = nullptr;
    trx.dataHandle().get(savingRet);
    if (savingRet)
    {
      savingRet->_ret = ret;
      savingRet->_ptfResultFrom = &paxTypeFare;

      if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
      {
        savingRet->_needCat13AccFareBreak = paxTypeFare.needAccSameFareBreak();
      }
      else if (categoryNumber == RuleConst::PENALTIES_RULE)
      {
        savingRet->_penaltyRestInd = false;

        if (!currentPenaltyRIndFlag)
          savingRet->_penaltyRestInd = paxTypeFare.penaltyRestInd() == YES;
      }
      else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
      {
        // Save Cat15 security result
        savingRet->_cat15SoftPass = paxTypeFare.cat15SoftPass();
        savingRet->_cat15SecurityFail = paxTypeFare.cat15SecurityFail();

        // Save Cat15 latestTktDT
        savingRet->_latestTktDT = paxTypeFare.fare()->latestTktDTFootN();

        // Save Cat15 (nation in FR for CWT)
        savingRet->_cat15NationFRForCWT = paxTypeFare.fare()->isNationFRInCat15Fn();
        // Save Cat15 (security)
        savingRet->_cat15HasSecurity = paxTypeFare.fare()->isSecurityInCat15Fn();
        // Save Cat15 (Etkt warning )
        savingRet->_cat15HasEtktWarning = paxTypeFare.fare()->isWarningEtktInCat15Fn();

        // Format of Purchase message
        if (!currentFOPFlag && !paxTypeFare.fare()->forbiddenFop().isNull())
          savingRet->_cat15FOPExist = true;
      }

      paxTypeFare.fareMarket()->saveFnResult(
          trx.dataHandle(), categoryNumber, paxTypeFare, savingRet, *savedResults);

      // diagnostic
      DiagManager diag(trx, Diagnostic511);

      if (UNLIKELY(diag.isActive() && trx.diagnostic().shouldDisplay(paxTypeFare, categoryNumber)))
      {
        diag << "FM: " << paxTypeFare.fareMarket()->origin()->loc()
             << paxTypeFare.fareMarket()->destination()->loc() << std::setw(10)
             << paxTypeFare.fareClass();
        diag << std::setw(5) << paxTypeFare.vendor() << " " << carrier << std::setw(5)
             << paxTypeFare.fareTariff() << std::setw(7) << fnTblIter.footNote()
             << " SEQ:" << std::setw(9) << fnCtrlInfo->sequenceNumber() << " F" << categoryNumber
             << '\n' << "     SAVE RESULT-" << savingRet->_ret << '\n';
      }
    }
  } // savedResults != 0, need to save result
}

void
RuleController::saveFnIntoFareMarket(PaxTypeFare& paxTypeFare,
                                     PricingTrx& trx,
                                     FareMarket::FareMarketSavedFnResult::Results* savedResults,
                                     uint16_t categoryNumber,
                                     CarrierCode& carrier,
                                     Record3ReturnTypes ret,
                                     FootnoteTable& fnTblIter,
                                     const FootNoteCtrlInfo* fnCtrlInfo,
                                     bool currentFOPFlag)
{
  if (savedResults && (_categoryPhase != DynamicValidation))
  {
    FareMarket::FareMarketSavedFnResult::Result* savingRet = nullptr;
    trx.dataHandle().get(savingRet);
    if (LIKELY(savingRet))
    {
      savingRet->_ret = ret;
      savingRet->_fare = paxTypeFare.fare();

      if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
      {
        // Save Cat15 security result
        savingRet->_cat15SoftPass = paxTypeFare.cat15SoftPass();
        savingRet->_cat15SecurityFail = paxTypeFare.cat15SecurityFail();

        // Format of Purchase message
        if (UNLIKELY(!currentFOPFlag && !paxTypeFare.fare()->forbiddenFop().isNull()))
          savingRet->_cat15FOPExist = true;

        savingRet->_cat15HasT996FT = paxTypeFare.isCat15HasT996FT();
      }
      else if (categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE)
      {
        savingRet->_cat14NVAData = paxTypeFare.getNVAData();
      }

      paxTypeFare.fareMarket()->saveFnResult(
          trx.dataHandle(), categoryNumber, paxTypeFare, savingRet, *savedResults);

      // diagnostic
      DiagManager diag(trx, Diagnostic511);

      if (UNLIKELY(diag.isActive() && trx.diagnostic().shouldDisplay(paxTypeFare, categoryNumber)))
      {
        diag << "FM: " << paxTypeFare.fareMarket()->origin()->loc()
             << paxTypeFare.fareMarket()->destination()->loc() << std::setw(10)
             << paxTypeFare.fareClass();
        diag << std::setw(5) << paxTypeFare.vendor() << " " << carrier << std::setw(5)
             << paxTypeFare.fareTariff() << std::setw(7) << fnTblIter.footNote()
             << " SEQ:" << std::setw(9) << fnCtrlInfo->sequenceNumber() << " F" << categoryNumber
             << '\n' << "     SAVE RESULT-" << savingRet->_ret << '\n';
      }
    }
  } // savedResults != 0, need to save result
}

void
RuleController::savedRetNotProcessedFn(Record3ReturnTypes& ret,
                                       bool& isLocationSwapped,
                                       PaxTypeFare& paxTypeFare,
                                       PricingTrx& trx,
                                       uint16_t categoryNumber,
                                       const FootNoteCtrlInfo*& fnCtrlInfo,
                                       FootNoteCtrlInfoVec& fnCtrlInfoVec,
                                       RuleControllerDataAccess& da,
                                       RuleProcessingData& rpData,
                                       RuleControllerParam& rcParam,
                                       bool skipCat15Security)
{
  // Check a FootNote data to contimue processing
  // where:  applInd()         == : STRING_DOES_NOT_APPLY... ->stop Footnote processing
  DCFactory* factory = nullptr;
  Diag500Collector* diag500 = nullptr;
  bool isCatNeededInDiag500 = false;

  if (fnCtrlInfo->applInd() != RuleConst::STRING_DOES_NOT_APPLY)
  {
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500 &&
                 diag500Parameters(trx, factory, diag500)))
    {
      isCatNeededInDiag500 = isCatNeededInDiag(fnCtrlInfo->categoryRuleItemInfoSet());

      if (isCatNeededInDiag500)
      {
        diag500->printRulePhase(_categoryPhase);
        if (_categoryPhase == DynamicValidation)
        {
          *diag500 << (_fmPhase ? "FARE MARKET SCOPE\n" : "PRICING UNIT SCOPE\n");
          if (paxTypeFare.selectedFareNetRemit())
            *diag500 << " SELECTED TKT CAT35 FARE FOR NET REMIT\n";
        }
        diag500->diag500Collector(paxTypeFare, fnCtrlInfo);
        diag500->flushMsg();
      }
    }

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic550))
    {
      DCFactory* dcFactory = DCFactory::instance();

      Diag550Collector* diag = dynamic_cast<Diag550Collector*>(dcFactory->create(trx));
      diag->enable(Diagnostic550);
      diag->diag550Collector(paxTypeFare, fnCtrlInfo);

      diag->flushMsg();
    }
    // inhibit check, INHIBIT_IGNORE has been processed at matching
    const Indicator inhibit = fnCtrlInfo->inhibit();

    if (UNLIKELY(inhibit == RuleConst::INHIBIT_FAIL))
    {
      ret = FAIL;

      if (UNLIKELY(isCatNeededInDiag500))
      {
        *diag500 << "FAIL BY INHIBIT\n";
      }
    }
    else if (UNLIKELY(inhibit == RuleConst::INHIBIT_ASSUMPTION))
    {
      ret = SKIP;

      if (UNLIKELY(isCatNeededInDiag500))
      {
        *diag500 << "SKIP BY INHIBIT\n";
      }
    }
    else
    {
      bool isHistorical = trx.dataHandle().isHistorical();
      FootNoteCtrlInfoVec::iterator iter = fnCtrlInfoVec.begin();
      FootNoteCtrlInfoVec::iterator iterEnd = fnCtrlInfoVec.end();
      for (; iter != iterEnd; ++iter)
      {
        fnCtrlInfo = (*iter).first;
        isLocationSwapped = (*iter).second;
        ret = callCategoryRuleItemSet(_categoryRuleItemSet,
                                      *fnCtrlInfo,
                                      fnCtrlInfo->categoryRuleItemInfoSet(),
                                      da,
                                      rpData,
                                      isLocationSwapped,
                                      false,
                                      skipCat15Security);
        if (LIKELY(!isHistorical || ret != FAIL))
          break;
      }
    }

    setFBDisplayData(trx, categoryNumber, nullptr, fnCtrlInfo, nullptr, paxTypeFare);
  }
  else
  {
    // stop processing for this category
    rcParam._checkFareRuleAltGenRule = false;
  }

  if (UNLIKELY(isCatNeededInDiag500))
  {
    diag500->diag500Collector(ret);
    diag500->flushMsg();
  }

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic550))
  {
    DCFactory* dcFactory = DCFactory::instance();

    Diag550Collector* diag = dynamic_cast<Diag550Collector*>(dcFactory->create(trx));
    diag->enable(Diagnostic550);
    diag->diag550Collector(ret);
    diag->flushMsg();
  }
}

bool
RuleController::setFBDisplayData(PricingTrx& trx,
                                 uint16_t categoryNumber,
                                 const FareRuleRecord2Info* fRR2,
                                 const FootNoteRecord2Info* fNR2,
                                 const GeneralRuleRecord2Info* gRR2,
                                 PaxTypeFare& paxTypeFare)
{
  FareDisplayTrx* fareDisplayTrx(nullptr);
  if (LIKELY(!FareDisplayUtil::getFareDisplayTrx(&trx, fareDisplayTrx)))
    return false;

  if (!fareDisplayTrx->isRD())
    return false;

  FareDisplayInfo* fareDisplayInfo(nullptr);

  fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  // Copy only FareBasisCopy which is requested for short RD request.
  // For long RDs, we may not have basefare information for created fares
  // and therefore cannot perform this tuning (SPR 94802)
  if (!fareDisplayTrx->isShortRD() ||
      (fareDisplayInfo && fareDisplayTrx->getRequest() &&
       (fareDisplayTrx->getRequest()->fareBasisCode() == paxTypeFare.fareClass() ||
        fareDisplayTrx->getOptions()->fareClass() == paxTypeFare.fareWithoutBase()->fareClass())))
  {
    fareDisplayInfo->setFBDisplayData(
        categoryNumber, fRR2, fNR2, gRR2, paxTypeFare.fareClass(), trx.dataHandle());
    return true;
  }
  return false;
}

bool
RuleController::fareRuleTakePrecedenceOverGenRule(PricingTrx& trx, uint16_t categoryNumber)
{
  return (categoryNumber == RuleConst::TRANSFER_RULE) ||
         (categoryNumber == RuleConst::STOPOVER_RULE);
}

void
RuleController::reuseWarningMap(const WarningMap& srcWarningMap,
                                const WarningMap& trgWarningMap,
                                uint16_t categoryNumber) const
{
  WarningMap::WarningID warn1 = WarningMap::map_size, warn2 = WarningMap::map_size;
  if (categoryNumber == RuleConst::DAY_TIME_RULE)
  {
    warn1 = WarningMap::cat2_warning_1;
    warn2 = WarningMap::cat2_warning_2;
  }
  else if (categoryNumber == RuleConst::FLIGHT_APPLICATION_RULE)
    warn1 = WarningMap::cat4_warning;
  else if (categoryNumber == RuleConst::ADVANCE_RESERVATION_RULE)
  {
    warn1 = WarningMap::cat5_warning_1;
    warn2 = WarningMap::cat5_warning_2;
  }
  else if (categoryNumber == RuleConst::MINIMUM_STAY_RULE)
    warn1 = WarningMap::cat6_warning;
  else if (categoryNumber == RuleConst::MAXIMUM_STAY_RULE)
    warn1 = WarningMap::cat7_warning;
  else if (categoryNumber == RuleConst::BLACKOUTS_RULE)
    warn1 = WarningMap::cat11_warning;
  else if (categoryNumber == RuleConst::SURCHARGE_RULE)
    warn1 = WarningMap::cat12_warning;
  else if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
    warn1 = WarningMap::cat13_warning;
  else if (categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE)
    warn1 = WarningMap::cat14_warning;
  else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    warn1 = WarningMap::cat15_warning_1;
    warn2 = WarningMap::cat15_warning_2;
  }
  else if (categoryNumber >= RuleConst::CHILDREN_DISCOUNT_RULE &&
           categoryNumber <= RuleConst::OTHER_DISCOUNT_RULE)
    warn1 = WarningMap::cat19_22_warning;

  if (warn1 != WarningMap::map_size && srcWarningMap.isSet(warn1))
    trgWarningMap.set(warn1, true);

  if (warn2 != WarningMap::map_size && srcWarningMap.isSet(warn2))
    trgWarningMap.set(warn2, true);
}

Record3ReturnTypes
RuleController::processFareRuleCommon(PricingTrx& trx,
                                      RuleControllerDataAccess& da,
                                      const uint16_t categoryNumber,
                                      RuleControllerParam& rcParam,
                                      RuleProcessingData& rpData,
                                      bool skipCat15Security)
{
  // Shortcut
  PaxTypeFare& paxTypeFare = da.paxTypeFare();
  Itin* itin = da.itin();
  const bool isFareDisplayTrx = (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX);
  //  NEW
  GeneralFareRuleInfoVec gfrInfoVec;

  if (UNLIKELY(isFareDisplayTrx))
  {
    updateRuleType(paxTypeFare, RuleConst::FARE_RULE);
  }

  rcParam._checkFareRuleAltGenRule = true;
  rcParam._generalRuleEnabled = true; // prepare for GeneralRule
  rcParam._readRecord0 = true;

  Record3ReturnTypes ret = SKIP;
  bool isLocationSwapped = false;

  // validate CAT 25 qualifiers and Record 2 Cat 25 Directionality,
  // Inbound/Outbound for FBR only

  if (categoryNumber == RuleConst::FARE_BY_RULE)
  {
    rcParam._checkFareRuleAltGenRule = false;
    const FBRPaxTypeFareRuleData* fbrPTFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (UNLIKELY(fbrPTFare == nullptr))
    {
      return FAIL;
    }
    const FareByRuleCtrlInfo* fbrCtrlInfo =
        dynamic_cast<const FareByRuleCtrlInfo*>(fbrPTFare->categoryRuleInfo());

    DCFactory* factory = nullptr;
    Diag500Collector* diag = nullptr;
    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500 &&
                 diag500Parameters(trx, factory, diag)))
    {
      if (isCatNeededInDiag(fbrCtrlInfo->categoryRuleItemInfoSet()))
      {
        diag->printRulePhase(_categoryPhase);
        if (_categoryPhase == DynamicValidation)
        {
          *diag << (_fmPhase ? "FARE MARKET SCOPE\n" : "PRICING UNIT SCOPE\n");
        }
        diag->diag500Collector(paxTypeFare, fbrCtrlInfo);
        diag->flushMsg();
      }
    }

    ret = callCategoryRuleItemSet(_categoryRuleItemSet,
                                  (*fbrCtrlInfo),
                                  fbrCtrlInfo->categoryRuleItemInfoSet(),
                                  da,
                                  rpData,
                                  isLocationSwapped,
                                  false,
                                  skipCat15Security);

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500 &&
                 diag500Parameters(trx, factory, diag)))
    {
      if (isCatNeededInDiag(fbrCtrlInfo->categoryRuleItemInfoSet()))
      {
        diag->diag500Collector(ret);
        diag->flushMsg();
      }
    }

    return ret;
  }

  const GeneralFareRuleInfo* gfrRuleInfo = nullptr;

  // retrieve FareRule record2, try reuse result for PreValidation
  // or NormalValidation on FareMarket

  const FareMarket::FareMarketSavedGfrResult::Result* savedRet = nullptr;
  FareMarket::FareMarketSavedGfrResult::Results* savedResults = nullptr;

  if ((categoryNumber > RuleConst::OTHER_DISCOUNT_RULE) ||
      (categoryNumber == RuleConst::TICKET_ENDORSMENT_RULE) || skipReuse(categoryNumber))
  {
    // no fare market result reuse for these categories
    gfrRuleInfo = findFareRule(trx, paxTypeFare, gfrInfoVec, isLocationSwapped, categoryNumber);
  }
  else if ((_categoryPhase == FBRBaseFarePrevalidation) ||
           (_categoryPhase == PreValidation) || (_categoryPhase == ShoppingComponentValidation) ||
           (_categoryPhase == ShoppingAcrossStopOverComponentValidation) ||
           (_categoryPhase == NormalValidation) || (_categoryPhase == FCORuleValidation))
  {
    gfrRuleInfo = fallback::fallbackGfrR2Optimization(&trx)
                      ? findGnFareRuleOnFareMarket<FallBackOnGfr>(trx,
                                                                  paxTypeFare,
                                                                  paxTypeFare.tcrRuleTariff(),
                                                                  paxTypeFare.ruleNumber(),
                                                                  categoryNumber,
                                                                  gfrInfoVec,
                                                                  isLocationSwapped,
                                                                  true, // isFareRule
                                                                  savedRet,
                                                                  savedResults,
                                                                  itin,
                                                                  skipCat15Security)
                      : findGnFareRuleOnFareMarket<FallBackOffGfr>(trx,
                                                                   paxTypeFare,
                                                                   paxTypeFare.tcrRuleTariff(),
                                                                   paxTypeFare.ruleNumber(),
                                                                   categoryNumber,
                                                                   gfrInfoVec,
                                                                   isLocationSwapped,
                                                                   true, // isFareRule
                                                                   savedRet,
                                                                   savedResults,
                                                                   itin,
                                                                   skipCat15Security);

    paxTypeFare.setCatRuleInfo(
        gfrRuleInfo, categoryNumber, trx.dataHandle(), isLocationSwapped, true);

    if (!gfrRuleInfo)
    {
      return ret;
    }

    // Tkt Endorsement should always go thru
    if (savedRet && (savedRet->_ret != NOTPROCESSED))
    {
      ret = savedRet->_ret;

      if (UNLIKELY(trx.noPNRPricing()))
      {
        const WarningMap& fromWarningMap = savedRet->_ptfResultFrom->warningMap();
        const WarningMap& toWarningMap = paxTypeFare.warningMap();

        reuseWarningMap(fromWarningMap, toWarningMap, categoryNumber);
      }

      storeSalesRestrictionValuesFr(paxTypeFare, categoryNumber, savedRet, isFareDisplayTrx);
      storeEligibilityValues(paxTypeFare, categoryNumber, savedRet);

      if (ret == FAIL)
      {
        // LOG4CXX_ERROR(logger, "Reusing fail val cat:"<<categoryNumber
        //      <<" from #("<<((long) savedRet->_ptfResultFrom ) <<") to
        // ("<<((long)&paxTypeFare)<<")");
        // it is done deal now, no need to save further information
        return ret;
      }

      storeResultsFr(trx, paxTypeFare, categoryNumber, savedRet, rcParam);
      return ret;
    }
  }
  else
  {
    if ((_categoryPhase == DynamicValidation) && _fmPhase)
    {
      // for DynamicValidation, only reuse result but not to save new result
      gfrRuleInfo = fallback::fallbackGfrR2Optimization(&trx)
                        ? findGnFareRuleOnFareMarket<FallBackOnGfr>(trx,
                                                                    paxTypeFare,
                                                                    paxTypeFare.tcrRuleTariff(),
                                                                    paxTypeFare.ruleNumber(),
                                                                    categoryNumber,
                                                                    gfrInfoVec,
                                                                    isLocationSwapped,
                                                                    true, // isFareRule
                                                                    savedRet,
                                                                    savedResults,
                                                                    itin,
                                                                    skipCat15Security)
                        : findGnFareRuleOnFareMarket<FallBackOffGfr>(trx,
                                                                     paxTypeFare,
                                                                     paxTypeFare.tcrRuleTariff(),
                                                                     paxTypeFare.ruleNumber(),
                                                                     categoryNumber,
                                                                     gfrInfoVec,
                                                                     isLocationSwapped,
                                                                     true, // isFareRule
                                                                     savedRet,
                                                                     savedResults,
                                                                     itin,
                                                                     skipCat15Security);

      if (gfrRuleInfo && savedRet && (savedRet->_ret != NOTPROCESSED))
      {
        if ((categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE) && (savedRet->_ret == FAIL))
        {
          // Dynamic rule validation for MinFares. Only check Hard fail
          return savedRet->_cat15SoftPass ? PASS : FAIL;
        }

        rcParam._checkFareRuleAltGenRule = savedRet->_checkFareRuleAltGenRule;
        rcParam._generalRuleEnabled = savedRet->_generalRuleEnabled;
        rcParam._readRecord0 = savedRet->_readRecord0;
        rcParam._genTariffNumber = savedRet->_genTariffNumber;
        rcParam._genRuleNumber = savedRet->_genRuleNumber;
        ret = savedRet->_ret;
        return ret;
      }
    }

    if (gfrRuleInfo == nullptr)
    {
      gfrRuleInfo = findFareRule(trx, paxTypeFare, gfrInfoVec, isLocationSwapped, categoryNumber);
    }
  }

  processGeneralFareRuleInfoVecFr(ret,
                                  gfrInfoVec,
                                  paxTypeFare,
                                  categoryNumber,
                                  da,
                                  trx,
                                  rpData,
                                  rcParam,
                                  gfrRuleInfo,
                                  savedResults,
                                  isLocationSwapped,
                                  skipCat15Security);

  if ((_categoryPhase == DynamicValidation) &&
      (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE) && (ret == FAIL) && _fmPhase)
  {
    return paxTypeFare.cat15SoftPass() ? PASS : FAIL;
  }

  return ret;
}

void
RuleController::storeSalesRestrictionValuesFr(
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedGfrResult::Result* savedRet,
    bool isFareDisplayTrx)
{
  // Set Cat15 security result
  if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    paxTypeFare.setCat15SoftPass(savedRet->_cat15SoftPass);

    if (savedRet->_cat15SecurityFail)
    {
      paxTypeFare.setCat15SecurityFail();
    }

    if (UNLIKELY(savedRet->_webFare))
    {
      paxTypeFare.setWebFare();
    }

    // Set Cat15 latestTktDT result
    if (UNLIKELY(isFareDisplayTrx))
      setTktDates(paxTypeFare, savedRet->_earliestTktDT, savedRet->_latestTktDT);

    const DateTime& ltd = savedRet->_latestTktDT;
    paxTypeFare.fare()->latestTktDTFareR().setWithEarlier(ltd);
    paxTypeFare.fare()->latestTktDT().setWithEarlier(ltd);

    // Set Cat15 (nation in FR for CWT)
    if (UNLIKELY(savedRet->_cat15NationFRForCWT))
    {
      paxTypeFare.fare()->setNationFRInCat15Fr();
      paxTypeFare.fare()->setNationFRInCat15();
    }
    // Set Cat15 (security)
    if (savedRet->_cat15HasSecurity)
    {
      paxTypeFare.fare()->setSecurityInCat15Fr();
      paxTypeFare.fare()->setCat15HasSecurity();
    }
    // Set Cat15 (Etkt warning is found in C15)
    if (UNLIKELY(savedRet->_cat15HasEtktWarning))
    {
      paxTypeFare.fare()->setWarningEtktInCat15Fr();
      paxTypeFare.fare()->setWarningEtktInCat15();
    }

    if (savedRet->_cat15FOPExist && paxTypeFare.fare()->forbiddenFop().isNull())
      paxTypeFare.fare()->mutableForbiddenFop() = savedRet->_ptfResultFrom->fare()->forbiddenFop();
  }
}

void
RuleController::storeNotFailedValuesFr(PaxTypeFare& paxTypeFare,
                                       uint16_t categoryNumber,
                                       FareMarket::FareMarketSavedGfrResult::Result* savingRet,
                                       RuleControllerParam& rcParam,
                                       bool currentMatchCorpIDFlag)
{
  const bool currentWebFareFlag = paxTypeFare.isWebFare();
  const bool currentPenaltyRIndFlag = paxTypeFare.penaltyRestInd() == YES;

  savingRet->_checkFareRuleAltGenRule = rcParam._checkFareRuleAltGenRule;
  savingRet->_generalRuleEnabled = rcParam._generalRuleEnabled;
  savingRet->_readRecord0 = rcParam._readRecord0;
  savingRet->_genTariffNumber = rcParam._genTariffNumber;
  savingRet->_genRuleNumber = rcParam._genRuleNumber;
  if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
  {
    savingRet->_needCat13AccFareBreak = paxTypeFare.needAccSameFareBreak();
  }
  else if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
  {
    savingRet->_isWebFare = false;
    savingRet->_matchedCorpId = false;

    if (LIKELY(!currentWebFareFlag))
      savingRet->_isWebFare = paxTypeFare.isWebFare();
    if (!currentMatchCorpIDFlag)
      savingRet->_matchedCorpId = paxTypeFare.matchedCorpID();
  }
  else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    savingRet->_webFare = false;

    if (LIKELY(!currentWebFareFlag))
      savingRet->_webFare = paxTypeFare.isWebFare();
  }
  else if (UNLIKELY(categoryNumber == RuleConst::PENALTIES_RULE))
  {
    savingRet->_penaltyRestInd = false;

    if (!currentPenaltyRIndFlag)
      savingRet->_penaltyRestInd = paxTypeFare.penaltyRestInd() == YES;
  }
}

void
RuleController::storeResultsFr(PricingTrx& trx,
                               PaxTypeFare& paxTypeFare,
                               uint16_t categoryNumber,
                               const FareMarket::FareMarketSavedGfrResult::Result* savedRet,
                               RuleControllerParam& rcParam)
{
  rcParam._checkFareRuleAltGenRule = savedRet->_checkFareRuleAltGenRule;
  rcParam._generalRuleEnabled = savedRet->_generalRuleEnabled;
  rcParam._readRecord0 = savedRet->_readRecord0;
  rcParam._genTariffNumber = savedRet->_genTariffNumber;
  rcParam._genRuleNumber = savedRet->_genRuleNumber;
  if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
  {
    if (savedRet->_needCat13AccFareBreak)
    {
      paxTypeFare.setAccSameFareBreak(true);

      PaxTypeFareRuleData* ptfRuleData =
          paxTypeFare.paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      const PaxTypeFareRuleData* ptfRuleDataFrom =
          savedRet->_ptfResultFrom->paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      if (ptfRuleData && ptfRuleDataFrom)
      {
        ptfRuleData->ruleItemInfo() = ptfRuleDataFrom->ruleItemInfo();
      }
    }
  }
  else if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
  {
    if (UNLIKELY(savedRet->_isWebFare))
      paxTypeFare.setWebFare();

    if (savedRet->_matchedCorpId)
    {
      paxTypeFare.setMatchedCorpID();
      paxTypeFare.matchedAccCode() = savedRet->_ptfResultFrom->matchedAccCode();
    }
  }
  else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    if (savedRet->_ptfResultFrom && savedRet->_ptfResultFrom->isCat15HasT996FR())
      paxTypeFare.setCat15HasT996FR(true);

    if (UNLIKELY(savedRet->_webFare))
      paxTypeFare.setWebFare();

    if (UNLIKELY(savedRet->_ptfResultFrom->matchedCat15QualifyingCorpID()))
    {
      paxTypeFare.setMatchedCorpID();
      paxTypeFare.matchedAccCode() = savedRet->_ptfResultFrom->matchedAccCode();
    }
  }
  else if (UNLIKELY(categoryNumber == RuleConst::PENALTIES_RULE))
  {
    if (savedRet->_penaltyRestInd)
    {
      paxTypeFare.penaltyRestInd() = YES;
    }
  }

  if (categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE &&
      savedRet->_ptfResultFrom->getNVAData())
  {
    paxTypeFare.useNVAData(savedRet->_ptfResultFrom->getNVAData());
  }
}

void
RuleController::processGeneralFareRuleInfoVecFr(
    Record3ReturnTypes& ret,
    GeneralFareRuleInfoVec& gfrInfoVec,
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    RuleControllerDataAccess& da,
    PricingTrx& trx,
    RuleProcessingData& rpData,
    RuleControllerParam& rcParam,
    const GeneralFareRuleInfo* gfrRuleInfo,
    FareMarket::FareMarketSavedGfrResult::Results* savedResults,
    bool isLocationSwapped,
    bool skipCat15Security)
{
  if (gfrInfoVec.size() > 0)
  {
    const bool currentWebFareFlag = paxTypeFare.isWebFare();
    const bool currentMatchCorpIDFlag = paxTypeFare.matchedCorpID();
    const bool currentFOPFlag = categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE &&
                                !paxTypeFare.fare()->forbiddenFop().isNull();
    ret = processFareRuleInfo(trx,
                              da,
                              gfrInfoVec,
                              categoryNumber,
                              gfrRuleInfo,
                              isLocationSwapped,
                              &rcParam,
                              rpData,
                              true,
                              skipCat15Security);

    if (fareRuleTakePrecedenceOverGenRule(trx, categoryNumber))
      rcParam._generalRuleEnabled = false;

    if (savedResults && (_categoryPhase != DynamicValidation))
    {
      FareMarket::FareMarketSavedGfrResult::Result* savingRet = nullptr;
      trx.dataHandle().get(savingRet);
      if (LIKELY(savingRet))
      {
        savingRet->_ret = ret;
        savingRet->_ptfResultFrom = &paxTypeFare;

        CarrierCode carrier = paxTypeFare.carrier();
        if (paxTypeFare.fare()->isIndustry())
          carrier = INDUSTRY_CARRIER;

        if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
        {
          savingRet->_foundCat1R3NoADT = paxTypeFare.foundCat1R3NoADT();
          savingRet->_foundCat1R3ChkAge = paxTypeFare.foundCat1R3ChkAge();
        }

        if (ret != FAIL)
        {
          storeNotFailedValuesFr(
              paxTypeFare, categoryNumber, savingRet, rcParam, currentMatchCorpIDFlag);
        }

        // Save Cat15 security result
        if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
        {
          savingRet->_cat15SoftPass = paxTypeFare.cat15SoftPass();
          savingRet->_cat15SecurityFail = paxTypeFare.cat15SecurityFail();
          // Save Cat15 latestTktDT
          if (UNLIKELY(trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
            saveTktDates(paxTypeFare, savingRet);
          else
          {
            savingRet->_latestTktDT = paxTypeFare.fare()->latestTktDTFareR();
          }

          // Save Cat15 (nation in FR for CWT)
          savingRet->_cat15NationFRForCWT = paxTypeFare.fare()->isNationFRInCat15Fr();
          // Save Cat15 (security)
          savingRet->_cat15HasSecurity = paxTypeFare.fare()->isSecurityInCat15Fr();
          // Save Cat15 (Etkt warning )
          savingRet->_cat15HasEtktWarning = paxTypeFare.fare()->isWarningEtktInCat15Fr();
          // Format of Purchase message
          if (!currentFOPFlag && !paxTypeFare.fare()->forbiddenFop().isNull())
            savingRet->_cat15FOPExist = true;
          savingRet->_webFare = false;

          if (LIKELY(!currentWebFareFlag))
            savingRet->_webFare = paxTypeFare.isWebFare();
        }

        // Save Cat05 indicator used by WPA NOMATCH process
        if (categoryNumber == RuleConst::ADVANCE_RESERVATION_RULE)
        {
          savingRet->_reProcessCat05NoMatch = paxTypeFare.reProcessCat05NoMatch();
        }

        paxTypeFare.fareMarket()->saveGfrResult(
            trx.dataHandle(), categoryNumber, paxTypeFare, savingRet, *savedResults);

        DiagManager diag(trx, Diagnostic511);

        if (UNLIKELY(diag.isActive() &&
                     trx.diagnostic().shouldDisplay(paxTypeFare, categoryNumber)))
        {
          diag << "FM: " << paxTypeFare.fareMarket()->origin()->loc()
               << paxTypeFare.fareMarket()->destination()->loc() << std::setw(10)
               << paxTypeFare.fareClass();
          diag << std::setw(5) << paxTypeFare.vendor() << " " << carrier << std::setw(5)
               << paxTypeFare.tcrRuleTariff() << std::setw(7) << paxTypeFare.ruleNumber()
               << " SEQ:" << std::setw(9) << gfrRuleInfo->sequenceNumber() << " R" << categoryNumber
               << '\n' << "     SAVE RESULT-" << ret << '\n';
        }

        LOG4CXX_INFO(logger,
                     "FM: " << paxTypeFare.fareMarket()->origin()->loc()
                            << paxTypeFare.fareMarket()->destination()->loc() << " "
                            << paxTypeFare.fareClass() << " R" << categoryNumber << " "
                            << paxTypeFare.vendor() << " " << carrier << " "
                            << paxTypeFare.tcrRuleTariff() << " " << paxTypeFare.ruleNumber()
                            << " SEQ:" << gfrRuleInfo->sequenceNumber() << " SAVE RESULT-" << ret
                            << '\n');
      }
    } // savedResults != 0, need to save result
    setFBDisplayData(trx, categoryNumber, gfrRuleInfo, nullptr, nullptr, paxTypeFare);
  }
}

//----------------------------------------------------------------------------
// Set optional RuleControllerParam, add diagnostic to Diag500, Diag550, and
// callCategoryRuleItemSet( _categoryRuleItemSet, ... )
Record3ReturnTypes
RuleController::processGnFareRule(PricingTrx& trx,
                                  RuleControllerDataAccess& da,
                                  PaxTypeFare& paxTypeFare,
                                  const GeneralFareRuleInfo& gfrRuleInfo,
                                  uint16_t categoryNumber,
                                  RuleControllerParam* rcPara,
                                  RuleProcessingData& rpData,
                                  const bool isLocationSwapped,
                                  const bool isFareRule,
                                  const bool skipCat15Security)
{
  DCFactory* factory = nullptr;
  Diag500Collector* diag500 = nullptr;
  bool isCatNeededInDiag500 = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500 &&
               diag500Parameters(trx, factory, diag500)))
  {
    isCatNeededInDiag500 = isCatNeededInDiag(gfrRuleInfo.categoryRuleItemInfoSet());
  }

  // inhibit check, INHIBIT_IGNORE has been processed at matching
  const Indicator inhibit = gfrRuleInfo.inhibit();

  if (inhibit == RuleConst::INHIBIT_FAIL)
  {
    if (isCatNeededInDiag500)
    {
      *diag500 << "FAIL BY INHIBIT\n";
      diag500->flushMsg();
    }
    return FAIL;
  }
  else if (UNLIKELY(inhibit == RuleConst::INHIBIT_ASSUMPTION))
  {
    if (isCatNeededInDiag500)
    {
      *diag500 << "SKIP BY INHIBIT\n";
      diag500->flushMsg();
    }
    return SKIP;
  }

  Record3ReturnTypes ret = SKIP;

  // Prepare data to read the GeneralRule
  // where:  applInd()         =='X' : STRING_DOES_NOT_APPLY...
  //               ->stop FareRule/GeneralRule processing
  //         generalRuleAppl() =='N' : RECORD_0_NOT_APPLY......
  //               ->stop GeneralRule (Record0)

  if (gfrRuleInfo.applInd() != RuleConst::STRING_DOES_NOT_APPLY)
  {
    if (rcPara != nullptr)
    {
      RuleControllerParam& rcParam = *rcPara;

      rcParam._checkFareRuleAltGenRule = true;
      rcParam._genTariffNumber = gfrRuleInfo.generalRuleTariff();
      rcParam._genRuleNumber = gfrRuleInfo.generalRule();

      if (rcParam._genRuleNumber == RuleConst::NULL_GENERAL_RULE)
      {
        // Combination of GR and FR
        rcParam._generalRuleEnabled =
            (gfrRuleInfo.generalRuleAppl() != RuleConst::RECORD_0_NOT_APPLY);
      }
      else
      {
        rcParam._readRecord0 = false;

        if (UNLIKELY(gfrRuleInfo.generalRuleAppl() == RuleConst::RECORD_0_NOT_APPLY &&
                     needGeneralRule(trx, categoryNumber, PASS, rcParam)))
        {
          rcParam._generalRuleEnabled = false;
        }
      }
    }

    if (UNLIKELY(isCatNeededInDiag500))
    {
      diag500->printRulePhase(_categoryPhase);
      if (_categoryPhase == DynamicValidation)
      {
        *diag500 << (_fmPhase ? "FARE MARKET SCOPE\n" : "PRICING UNIT SCOPE\n");
        if (paxTypeFare.selectedFareNetRemit())
          *diag500 << " SELECTED TKT CAT35 FARE FOR NET REMIT\n";
      }
      diag500->diag500Collector(paxTypeFare, &gfrRuleInfo, (rcPara != nullptr));
      diag500->flushMsg();
    }

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic550))
    {
      DCFactory* dcFactory = DCFactory::instance();

      Diag550Collector* diag = dynamic_cast<Diag550Collector*>(dcFactory->create(trx));
      diag->enable(Diagnostic550);
      diag->diag550Collector(paxTypeFare, &gfrRuleInfo, (rcPara != nullptr));
      diag->flushMsg();
    }

    if ((rcPara == nullptr) || !rcPara->_processGenRuleForFMS)
    {
      ret = callCategoryRuleItemSet(_categoryRuleItemSet,
                                    gfrRuleInfo,
                                    gfrRuleInfo.categoryRuleItemInfoSet(),
                                    da,
                                    rpData,
                                    isLocationSwapped,
                                    isFareRule,
                                    skipCat15Security);
    }

    // setFBDisplayData(trx, categoryNumber, 0, 0, &gfrRuleInfo, paxTypeFare);
  }
  else
  {
    if (rcPara)
    {
      // stop processing for this category
      rcPara->_checkFareRuleAltGenRule = false;
    }
  }

  if (UNLIKELY(isCatNeededInDiag500))
  {
    diag500->diag500Collector(ret);
    diag500->flushMsg();
  }

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic550))
  {
    DCFactory* dcFactory = DCFactory::instance();

    Diag550Collector* diag = dynamic_cast<Diag550Collector*>(dcFactory->create(trx));
    diag->enable(Diagnostic550);
    diag->diag550Collector(ret);
    diag->flushMsg();
  }
  return ret;
}

//----------------------------------------------------------------------------
// applySystemDefaultAssumption()
//----------------------------------------------------------------------------
Record3ReturnTypes
RuleController::applySystemDefaultAssumption(PricingTrx& trx,
                                             RuleControllerDataAccess& da,
                                             const uint16_t category,
                                             bool& displayDiag)
{
  displayDiag = false;
  if (category != RuleConst::STOPOVER_RULE && category != RuleConst::TRANSFER_RULE)
    return SKIP;

  displayDiag = true;
  const Itin* itin = da.itin();
  const PaxTypeFare& paxTypeFare = da.paxTypeFare();
  const FareMarket* fm = paxTypeFare.fareMarket();
  if (LIKELY(fm && itin))
  {
    if (category == RuleConst::STOPOVER_RULE)
    {
      return Stopovers::applySystemDefaultAssumption(trx, *itin, paxTypeFare, *fm);
    }
    else if (LIKELY(category == RuleConst::TRANSFER_RULE))
    {
      return Transfers1::applySystemDefaultAssumption(*itin, *fm);
    }
  }
  return SKIP;
}

//----------------------------------------------------------------------------
// processGeneralRule()
//----------------------------------------------------------------------------
template <typename FallBackSwitch>
Record3ReturnTypes
RuleController::processGeneralRule(PricingTrx& trx,
                                   RuleControllerDataAccess& da,
                                   uint16_t categoryNumber,
                                   RuleControllerParam& rcParam,
                                   RuleProcessingData& rpData,
                                   bool skipCat15Security)
{
  // Shortcut
  PaxTypeFare& paxTypeFare = da.paxTypeFare();
  Itin* itin = da.itin();
  const bool isFareDisplayTrx = (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX);
  // NEW
  GeneralFareRuleInfoVec gfrInfoVec;

  if (UNLIKELY(isFareDisplayTrx))

  {
    updateRuleType(paxTypeFare, RuleConst::GENERAL_RULE);
  }

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  Record3ReturnTypes ret = SKIP;

  //----------------------------------------------------------------------

  // Process the record 2 general rule or alternate general rule
  //----------------------------------------------------------------------

  if (rcParam._checkFareRuleAltGenRule)
  {
    bool validRec0 = true;
    const GeneralFareRuleInfo* gfrRuleInfo = nullptr;
    bool isLocationSwapped = false;

    // see if we already tried to the general rule, we can reuse the retrieving
    // result and validation result
    PaxTypeFareRuleData* gfrRuleData = nullptr;

    if (!(trx.dataHandle().isHistorical() && categoryNumber == RuleConst::SURCHARGE_RULE) &&
        paxTypeFare.alreadyChkedGfrRule(categoryNumber, trx.dataHandle(), gfrRuleData))
    {
      if (gfrRuleData != nullptr)
      {
        gfrRuleInfo = dynamic_cast<const GeneralFareRuleInfo*>(gfrRuleData->categoryRuleInfo());
      }
      if (gfrRuleInfo == nullptr)
      {
        return SKIP;
      }

      isLocationSwapped = gfrRuleData->isLocationSwapped();
      rcParam._genTariffNumber = gfrRuleInfo->tariffNumber();
      rcParam._genRuleNumber = gfrRuleInfo->ruleNumber();
      gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
    }
    else
    {
      // search the rule
      // if readGeneralRule is true, get the record 0 general rule info
      // otherwise rely on the the record 2 alternate general rule info
      //

      if (rcParam._readRecord0 && rcParam._generalRuleEnabled)
      {
        if (!fallback::fallbackAPO37838Record1EffDateCheck(&trx))
        {
          DateTime tvlDate;
          if (trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX)
            tvlDate = trx.adjustedTravelDate(itin->travelSeg().front()->departureDT());
          else
          {
            tvlDate = paxTypeFare.fareMarket()->travelDate();
          }
          validRec0 =
              trx.dataHandle().getGeneralRuleAppTariffRuleByTvlDate(paxTypeFare.vendor(),
                                                                    carrier,
                                                                    paxTypeFare.tcrRuleTariff(),
                                                                    RuleConst::NULL_GENERAL_RULE,
                                                                    categoryNumber,
                                                                    rcParam._genRuleNumber,
                                                                    rcParam._genTariffNumber,
                                                                    tvlDate);
        }
        else
        {
          validRec0 = trx.dataHandle().getGeneralRuleAppTariffRule(paxTypeFare.vendor(),
                                                                   carrier,
                                                                   paxTypeFare.tcrRuleTariff(),
                                                                   RuleConst::NULL_GENERAL_RULE,
                                                                   categoryNumber,
                                                                   rcParam._genRuleNumber,
                                                                   rcParam._genTariffNumber);
        }
      }
    }

    if (validRec0)
    {
      // for validation on FareMarket, we can reuse validation result on same
      // record2 on same FareMarket

      const FareMarket::FareMarketSavedGfrResult::Result* savedRet = nullptr;
      FareMarket::FareMarketSavedGfrResult::Results* savedResults = nullptr;

      if (checkIfReuse(categoryNumber))
      {
        gfrRuleInfo = findGnFareRuleOnFareMarket<FallBackSwitch>(trx,
                                                                 paxTypeFare,
                                                                 rcParam._genTariffNumber,
                                                                 rcParam._genRuleNumber,
                                                                 categoryNumber,
                                                                 gfrInfoVec,
                                                                 isLocationSwapped,
                                                                 false, // isFareRule
                                                                 savedRet,
                                                                 savedResults,
                                                                 itin,
                                                                 skipCat15Security);

        if (savedRet && (savedRet->_ret != NOTPROCESSED))
        {
          if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE &&
              RuleController::dontReuseSaleRestrictionRule(*savedRet, paxTypeFare))
          {
            // do not reuse GR
            savedRet = nullptr;
            savedResults = nullptr;
          }
        }

        paxTypeFare.setCatRuleInfo(
            gfrRuleInfo, categoryNumber, trx.dataHandle(), isLocationSwapped, false);

        if (savedRet && (savedRet->_ret != NOTPROCESSED))
        {
          ret = savedRet->_ret;

          if (UNLIKELY(trx.noPNRPricing()))
          {
            const WarningMap& fromWarningMap = savedRet->_ptfResultFrom->warningMap();
            const WarningMap& toWarningMap = paxTypeFare.warningMap();

            reuseWarningMap(fromWarningMap, toWarningMap, categoryNumber);
          }

          storeSalesRestrictionValuesGr(
              trx, paxTypeFare, categoryNumber, savedRet); // Set Cat15 security result

          storeEligibilityValues(paxTypeFare, categoryNumber, savedRet);

          if (ret != FAIL)
          {
            storeNotFailedValuesGr(paxTypeFare, categoryNumber, savedRet);
          }
          setFBDisplayData(trx, categoryNumber, nullptr, nullptr, gfrRuleInfo, paxTypeFare);
          return ret;
        }
      }
      else
      {
        if (!skipReuse(categoryNumber) && (_categoryPhase == DynamicValidation) && _fmPhase &&
            (categoryNumber < RuleConst::MISC_FARE_TAG) &&
            (_reuseResult)) // try reuse but not save result
        {
          gfrRuleInfo = findGnFareRuleOnFareMarket<FallBackSwitch>(trx,
                                                                   paxTypeFare,
                                                                   rcParam._genTariffNumber,
                                                                   rcParam._genRuleNumber,
                                                                   categoryNumber,
                                                                   gfrInfoVec,
                                                                   isLocationSwapped,
                                                                   false, // isFareRule
                                                                   savedRet,
                                                                   savedResults,
                                                                   itin,
                                                                   skipCat15Security);

          if (gfrRuleInfo && savedRet && (savedRet->_ret != NOTPROCESSED))
          {
            if ((categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE) && (savedRet->_ret == FAIL))
            {
              // Dynamic rule validation for MinFares. Only check Hard Fail bytes
              return savedRet->_cat15SoftPass ? PASS : FAIL;
            }

            ret = savedRet->_ret;
            return ret;
          }
        }

        // not PreValidation or NormalValidation phase
        // do the regular general rule processing

        if (gfrRuleInfo == nullptr)
        {
          // not saved in PaxTypeFare rule data yet
          GeneralFareRuleInfoVec gfrList;

          if (fallback::fallbackGfrR2Optimization(&trx))
          {
          const DateTime& travelDate =
              trx.adjustedTravelDate(itin->travelSeg().front()->departureDT());
          const DateTime& ruleApplicationDate =
              (rcParam._checkFareRuleAltGenRule &&
               (categoryNumber == RuleConst::VOLUNTARY_EXCHANGE_RULE ||
                categoryNumber == RuleConst::VOLUNTARY_REFUNDS_RULE))
                  ? paxTypeFare.fareMarket()->ruleApplicationDate()
                  : DateTime::emptyDate();

          const std::vector<GeneralFareRuleInfo*>& gfrListFromDB =
              trx.dataHandle().getGeneralFareRule(paxTypeFare.vendor(),
                                                  carrier,
                                                  rcParam._genTariffNumber,
                                                  rcParam._genRuleNumber,
                                                  categoryNumber,
                                                  travelDate,
                                                  ruleApplicationDate);

          gfrList = dbToLocVecAdaptor(gfrListFromDB);
          }
          else
          {
            gfrList = Rec2Selector::getGfr(
                trx,
                paxTypeFare,
                rcParam._genTariffNumber,
                rcParam._genRuleNumber,
                categoryNumber,
                trx.adjustedTravelDate(itin->travelSeg().front()->departureDT()));
          }

          // scan the vector and find the first match
          auto bIt = gfrList.begin();
          bool isHistorical = trx.dataHandle().isHistorical();
          bool foundRec2 = false;
          DateTime& tktDate = trx.ticketingDate();
          FallBackSwitch fallBackSwitch;
          while (bIt != gfrList.end())
          {
            if (fallBackSwitch(trx, paxTypeFare, *bIt, isLocationSwapped))
            {
              gfrRuleInfo = bIt->first;
              if (LIKELY(!foundRec2))
              {
                gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
                foundRec2 = true;
                if (LIKELY(!isHistorical ||
                           !RuleUtil::matchCreateOrExpireDate(
                               tktDate, gfrRuleInfo->createDate(), gfrRuleInfo->expireDate())))
                  break;
              }
              else
              {
                if (!RuleUtil::matchCreateOrExpireDate(
                        tktDate, gfrRuleInfo->createDate(), gfrRuleInfo->expireDate()))
                  break;
                gfrInfoVec.emplace_back(gfrRuleInfo, GeoMatchResult(isLocationSwapped));
              }
            }
            ++bIt;
          }
          paxTypeFare.setCatRuleInfo(
              gfrRuleInfo, categoryNumber, trx.dataHandle(), isLocationSwapped, false);

          Diag202Collector* dc = Diag202Collector::makeMe(
              trx, &Diag202Collector::GFR, paxTypeFare.fareMarket(), &paxTypeFare);
          if (UNLIKELY(dc))
          {
            dc->printR2sMatchDetails<GeneralFareRuleInfo>(
                FB_GENERAL_RULE_RECORD_2,
                trx,
                locToDbVecAdaptor(gfrList),
                gfrInfoVec,
                *paxTypeFare.fareMarket(),
                carrier,
                trx.adjustedTravelDate(itin->travelSeg().front()->departureDT()),
                &paxTypeFare);
          }
        }
      } // end of retrieving general rule

      processGeneralFareRuleInfoVecGr(ret,
                                      gfrInfoVec,
                                      paxTypeFare,
                                      categoryNumber,
                                      da,
                                      trx,
                                      rpData,
                                      gfrRuleInfo,
                                      savedResults,
                                      carrier,
                                      isLocationSwapped,
                                      skipCat15Security);
    } // if validRec0
  }

  if ((_categoryPhase == DynamicValidation) &&
      (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE) && (ret == FAIL))
  {
    // Dynamic rule validation for MinFares. Only check Hard Fail bytes 73-84
    return paxTypeFare.cat15SoftPass() ? PASS : FAIL;
  }

  return ret;
}

bool
RuleController::checkIfReuse(uint16_t categoryNumber) const
{
  return !skipReuse(categoryNumber) &&
         (_categoryPhase == FBRBaseFarePrevalidation ||
          _categoryPhase == PreValidation || _categoryPhase == ShoppingComponentValidation ||
          _categoryPhase == ShoppingAcrossStopOverComponentValidation ||
          _categoryPhase == NormalValidation) &&
         categoryNumber < RuleConst::MISC_FARE_TAG && _reuseResult;
  // no need to worry TICKET_ENDORSMENT_RULE here, it does not have
  // general rule
}

void
RuleController::storeSalesRestrictionValuesGr(
    PricingTrx& trx,
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedGfrResult::Result* savedRet)
{
  if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    paxTypeFare.setCat15SoftPass(savedRet->_cat15SoftPass);

    if (savedRet->_cat15SecurityFail)
    {
      paxTypeFare.setCat15SecurityFail();
    }
    // Set Cat15 latestTktDT result
    const DateTime& ltd = savedRet->_latestTktDT;
    paxTypeFare.fare()->latestTktDT().setWithEarlier(ltd);
    paxTypeFare.fare()->latestTktDTGenR().setWithEarlier(ltd);

    // Set Cat15 (nation in FR for CWT)
    if (UNLIKELY(savedRet->_cat15NationFRForCWT))
    {
      paxTypeFare.fare()->setNationFRInCat15Gr();
      paxTypeFare.fare()->setNationFRInCat15();
    }
    // Set Cat15 (security)
    if (savedRet->_cat15HasSecurity)
    {
      paxTypeFare.fare()->setSecurityInCat15Gr();
      paxTypeFare.fare()->setCat15HasSecurity();
    }
    // Set Cat15 (Etkt warning is found in C15)
    if (UNLIKELY(savedRet->_cat15HasEtktWarning))
    {
      paxTypeFare.fare()->setWarningEtktInCat15Gr();
      paxTypeFare.fare()->setWarningEtktInCat15();
    }
    if (savedRet->_ptfResultFrom && savedRet->_ptfResultFrom->isCat15HasT996GR())
      paxTypeFare.setCat15HasT996GR(true);

    if (UNLIKELY(savedRet->_cat15FOPExist && paxTypeFare.fare()->forbiddenFop().isNull()))
      paxTypeFare.fare()->mutableForbiddenFop() = savedRet->_ptfResultFrom->fare()->forbiddenFop();
  }
}

void
RuleController::storeEligibilityValues(PaxTypeFare& paxTypeFare,
                                       uint16_t categoryNumber,
                                       const FareMarket::FareMarketSavedGfrResult::Result* savedRet)
{
  if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
  {
    if (savedRet->_foundCat1R3NoADT)
    {
      paxTypeFare.setFoundCat1R3NoADT();
    }
    if (savedRet->_foundCat1R3ChkAge)
    {
      paxTypeFare.setFoundCat1R3ChkAge();
    }
  }
}

void
RuleController::storeNotFailedValuesGr(PaxTypeFare& paxTypeFare,
                                       uint16_t categoryNumber,
                                       const FareMarket::FareMarketSavedGfrResult::Result* savedRet)
{
  if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
  {
    if (savedRet->_needCat13AccFareBreak)
    {
      paxTypeFare.setAccSameFareBreak(true);

      PaxTypeFareRuleData* ptfRuleData =
          paxTypeFare.paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      const PaxTypeFareRuleData* ptfRuleDataFrom =
          savedRet->_ptfResultFrom->paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      if (ptfRuleData && ptfRuleDataFrom)
      {
        ptfRuleData->ruleItemInfo() = ptfRuleDataFrom->ruleItemInfo();
      }
    }
  }
  else if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
  {
    if (UNLIKELY(savedRet->_isWebFare))
    {
      paxTypeFare.setWebFare();
    }
    if (UNLIKELY(savedRet->_matchedCorpId))
    {
      paxTypeFare.setMatchedCorpID();
      paxTypeFare.matchedAccCode() = savedRet->_ptfResultFrom->matchedAccCode();
    }
  }
  else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
  {
    if (UNLIKELY(savedRet->_webFare))
    {
      paxTypeFare.setWebFare();
    }

    if (UNLIKELY(savedRet->_ptfResultFrom->matchedCat15QualifyingCorpID()))
    {
      paxTypeFare.setMatchedCorpID();
      paxTypeFare.matchedAccCode() = savedRet->_ptfResultFrom->matchedAccCode();
    }
  }
  else if (UNLIKELY(categoryNumber == RuleConst::PENALTIES_RULE))
  {
    if (savedRet->_penaltyRestInd)
      paxTypeFare.penaltyRestInd() = YES;
  }
}

void
RuleController::processGeneralFareRuleInfoVecGr(
    Record3ReturnTypes& ret,
    GeneralFareRuleInfoVec& gfrInfoVec,
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    RuleControllerDataAccess& da,
    PricingTrx& trx,
    RuleProcessingData& rpData,
    const GeneralFareRuleInfo* gfrRuleInfo,
    FareMarket::FareMarketSavedGfrResult::Results* savedResults,
    CarrierCode& carrier,
    bool isLocationSwapped,
    bool skipCat15Security)
{
  if (gfrInfoVec.size() > 0)
  {
    const bool currentWebFareFlag = paxTypeFare.isWebFare();
    const bool currentMatchCorpIDFlag = paxTypeFare.matchedCorpID();
    const bool currentPenaltyRIndFlag = paxTypeFare.penaltyRestInd() == YES;
    const bool currentFOPFlag = categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE &&
                                !paxTypeFare.fare()->forbiddenFop().isNull();

    da.paxTypeFare().fare()->setCat15GeneralRuleProcess();
    ret = processFareRuleInfo(trx,
                              da,
                              gfrInfoVec,
                              categoryNumber,
                              gfrRuleInfo,
                              isLocationSwapped,
                              nullptr,
                              rpData,
                              false,
                              skipCat15Security);

    da.paxTypeFare().fare()->setCat15GeneralRuleProcess(false);

    if (savedResults && (_categoryPhase != DynamicValidation))
    {
      FareMarket::FareMarketSavedGfrResult::Result* savingRet = nullptr;
      trx.dataHandle().get(savingRet);
      if (LIKELY(savingRet))
      {
        savingRet->_ret = ret;
        savingRet->_ptfResultFrom = &paxTypeFare;

        if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
        {
          savingRet->_foundCat1R3NoADT = paxTypeFare.foundCat1R3NoADT();
          savingRet->_foundCat1R3ChkAge = paxTypeFare.foundCat1R3ChkAge();
        }

        if (ret != FAIL)
        {
          if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
          {
            savingRet->_needCat13AccFareBreak = paxTypeFare.needAccSameFareBreak();
          }

          else if (categoryNumber == RuleConst::ELIGIBILITY_RULE)
          {
            savingRet->_isWebFare = false;
            savingRet->_matchedCorpId = false;

            if (!currentWebFareFlag)
              savingRet->_isWebFare = paxTypeFare.isWebFare();
            if (!currentMatchCorpIDFlag)
              savingRet->_matchedCorpId = paxTypeFare.matchedCorpID();
          }
          else if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
          {
            savingRet->_webFare = false;

            if (LIKELY(!currentWebFareFlag))
              savingRet->_webFare = paxTypeFare.isWebFare();
          }
          else if (UNLIKELY(categoryNumber == RuleConst::PENALTIES_RULE))
          {
            savingRet->_penaltyRestInd = false;

            if (!currentPenaltyRIndFlag)
              savingRet->_penaltyRestInd = paxTypeFare.penaltyRestInd() == YES;
          }
        }

        // Save Cat15 security result
        if (categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
        {
          savingRet->_cat15SoftPass = paxTypeFare.cat15SoftPass();
          savingRet->_cat15SecurityFail = paxTypeFare.cat15SecurityFail();
          // Save Cat15 latestTktDT
          savingRet->_latestTktDT = paxTypeFare.fare()->latestTktDTGenR();
          // Save Cat15 (nation in FR for CWT)
          savingRet->_cat15NationFRForCWT = paxTypeFare.fare()->isNationFRInCat15Gr();
          // Save Cat15 (security)
          savingRet->_cat15HasSecurity = paxTypeFare.fare()->isSecurityInCat15Gr();
          // Save Cat15 (Etkt warning )
          savingRet->_cat15HasEtktWarning = paxTypeFare.fare()->isWarningEtktInCat15Gr();
          // Format of Purchase message

          if (UNLIKELY(!currentFOPFlag && !paxTypeFare.fare()->forbiddenFop().isNull()))
            savingRet->_cat15FOPExist = true;
        }

        paxTypeFare.fareMarket()->saveGfrResult(
            trx.dataHandle(), categoryNumber, paxTypeFare, savingRet, *savedResults);

        // diagnostic
        DiagManager diag(trx, Diagnostic511);

        if (UNLIKELY(diag.isActive() &&
                     trx.diagnostic().shouldDisplay(paxTypeFare, categoryNumber)))
        {
          CarrierCode ptCarrier = paxTypeFare.carrier();
          if (paxTypeFare.fare()->isIndustry())
            ptCarrier = INDUSTRY_CARRIER;

          diag << "FM: " << paxTypeFare.fareMarket()->origin()->loc()
               << paxTypeFare.fareMarket()->destination()->loc() << std::setw(10)
               << paxTypeFare.fareClass();
          diag << std::setw(5) << paxTypeFare.vendor() << " " << ptCarrier << std::setw(5)
               << gfrRuleInfo->tariffNumber() << std::setw(7) << gfrRuleInfo->ruleNumber()
               << " SEQ:" << std::setw(9) << gfrRuleInfo->sequenceNumber() << " G" << categoryNumber
               << '\n' << "     SAVE RESULT-" << ret << '\n';
        }
        LOG4CXX_INFO(logger,
                     "FM: " << paxTypeFare.fareMarket()->origin()->loc()
                            << paxTypeFare.fareMarket()->destination()->loc() << " "
                            << paxTypeFare.fareClass() << " G" << categoryNumber << " "
                            << paxTypeFare.vendor() << " " << carrier << " "
                            << paxTypeFare.tcrRuleTariff() << " " << paxTypeFare.ruleNumber()
                            << " SEQ:" << gfrRuleInfo->sequenceNumber() << " SAVE RESULT-" << ret
                            << '\n');
      }
    } // savedResults != 0, need to save result

    setFBDisplayData(trx, categoryNumber, nullptr, nullptr, gfrRuleInfo, paxTypeFare);
  } // if gfrRuleInfo != 0, end of validation
}

Record3ReturnTypes
RuleController::processRules(PricingTrx& trx,
                             RuleControllerDataAccess& da,
                             const uint16_t categoryNumber,
                             const bool& checkOtherRules,
                             const bool skipCat15Security,
                             const bool processSystemAssumption)
{ // lint !e578
  Record3ReturnTypes retRule = SKIP;
  Record3ReturnTypes retResultOfRule = SKIP;

  RuleControllerParam rcParam;

  rcParam._checkFareRuleAltGenRule =
      LIKELY(trx.isFootNotePrevalidationAllowed())
          ? da.paxTypeFare().fare()->checkFareRuleAltGenRule(categoryNumber)
          : true;

  RuleProcessingData rpData;

  StopoversInfoWrapper soInfoWrapper(da.currentFU());
  TransfersInfoWrapper trInfoWrapper(da.currentFU());
  rpData.soInfoWrapper(&soInfoWrapper);
  rpData.trInfoWrapper(&trInfoWrapper);

  if (UNLIKELY(PricingTrx::ESV_TRX == trx.getTrxType())) // ESV processing
  {
    bool checkFareRule = true;
    // check footnote only for cat 15 and 14
    if ((categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE ||
         categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE) &&
                (LIKELY(trx.isFootNotePrevalidationAllowed() &&
                        da.paxTypeFare().areFootnotesPrevalidated()))
            ? needFootnoteRule(trx, categoryNumber, da, retResultOfRule)
            : needFootnoteRule(categoryNumber))
    {
      retResultOfRule = fallback::fallbackFootNoteR2Optimization(&trx)
                            ? processFootnoteRule<FallBackOn>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
                            : processFootnoteRule<FallBackOff>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security);

      soInfoWrapper.clearResults();
      trInfoWrapper.clearResults();

      // for cat 14 footnote overrides FareRule
      if (categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE)
      {
        checkFareRule = false;
      }
    }

    if (checkOtherRules && checkFareRule)
    {
      if (needFareRule(categoryNumber, retResultOfRule, rcParam))
      {
        soInfoWrapper.isFareRule() = true;
        trInfoWrapper.isFareRule() = true;
        retRule =
            processFareRuleCommon(trx, da, categoryNumber, rcParam, rpData, skipCat15Security);

        if (retRule != SKIP)
        {
          retResultOfRule = retRule;
        }
      }

      retResultOfRule = doCategoryPostProcessing(trx, da, categoryNumber, rpData, retResultOfRule);
    }
  }
  else // not ESV processing
  {
    if ((LIKELY(trx.isFootNotePrevalidationAllowed() &&
                da.paxTypeFare().areFootnotesPrevalidated()))
            ? needFootnoteRule(trx, categoryNumber, da, retResultOfRule)
            : needFootnoteRule(categoryNumber))
    {
      da.setRuleType(FootNote);
      retResultOfRule = fallback::fallbackFootNoteR2Optimization(&trx)
                            ? processFootnoteRule<FallBackOn>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
                            : processFootnoteRule<FallBackOff>(
                                  trx, da, categoryNumber, rcParam, rpData, skipCat15Security);
      soInfoWrapper.clearResults();
      trInfoWrapper.clearResults();
    }

    if (checkOtherRules)
    {
      da.setRuleType(FareRule);
      if (needFareRule(categoryNumber, retResultOfRule, rcParam))
      {
        soInfoWrapper.isFareRule() = true;
        trInfoWrapper.isFareRule() = true;
        retRule =
            processFareRuleCommon(trx, da, categoryNumber, rcParam, rpData, skipCat15Security);

        if (retRule != SKIP)
        {
          retResultOfRule = retRule;
        }
      }
      da.setRuleType(GeneralRule);
      // bindings don't contain general rules
      if (needGeneralRuleValidation(trx, categoryNumber, da.paxTypeFare()) &&
          needGeneralRule(trx, categoryNumber, retResultOfRule, rcParam))
      {
        soInfoWrapper.isFareRule() = false;
        trInfoWrapper.isFareRule() = false;
        retRule = fallback::fallbackGfrR2Optimization(&trx)
                      ? processGeneralRule<FallBackOnGfr>(
                            trx, da, categoryNumber, rcParam, rpData, skipCat15Security)
                      : processGeneralRule<FallBackOffGfr>(
                            trx, da, categoryNumber, rcParam, rpData, skipCat15Security);
        if (retRule != SKIP)
        {
          retResultOfRule = retRule;
        }
      }

      retResultOfRule = doCategoryPostProcessing(trx, da, categoryNumber, rpData, retResultOfRule);
    }
  }

  if ((categoryNumber == RuleConst::SURCHARGE_RULE) &&
      (_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
       _categoryPhase == PURuleValidationISALT || _categoryPhase == FPRuleValidation ||
       _categoryPhase == FPRuleValidationISALT))
  {
    applySurchargeGenRuleForFMS(trx, da, categoryNumber, rcParam, skipCat15Security);
  }

  if (processSystemAssumption && (retResultOfRule == SKIP))
  {
    // Apply the system default assumption for the category.
    bool displayDiag = false;
    retResultOfRule = applySystemDefaultAssumption(trx, da, categoryNumber, displayDiag);

    if (displayDiag)
    {
      DCFactory* factory = nullptr;
      DiagCollector* diagPtr = nullptr;
      if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic500))
      {
        factory = DCFactory::instance();
        diagPtr = factory->create(trx);

        diagPtr->enable(Diagnostic500);

        DiagCollector& dc = *diagPtr;
        Diag500Collector* dc500 = dynamic_cast<Diag500Collector*>(diagPtr);

        PaxTypeFare& ptf = da.paxTypeFare();

        if (dc500)
        {
          dc500->printRulePhase(_categoryPhase);

          dc.setf(std::ios::left, std::ios::adjustfield);
          if (ptf.isValid())
          {
            dc << std::setw(3) << ptf.paxType()->paxType();
            dc << ":";
          }
          else
          {
            if (ptf.isFareClassAppMissing())
            {
              dc << "-R1:";
            }
            else if (ptf.isFareClassAppSegMissing())
            {
              dc << "-1B:";
            }
            else if (ptf.isRoutingProcessed() && !ptf.isRoutingValid())
            {
              dc << "-RT:";
            }
            else if (ptf.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL))
            {
              dc << "-BK:";
            }
            else if (!ptf.areAllCategoryValid())
            {
              if (ptf.paxType())
              {
                dc << std::setw(3) << ptf.paxType()->paxType();
              }
              else
              {
                dc << std::setw(3) << "   ";
              }
              dc << "*";
            }
            else
            {
              dc << "-XX:";
            }
          }

          dc << std::setw(3) << ptf.fareMarket()->origin()->loc();
          dc << std::setw(4) << ptf.fareMarket()->destination()->loc();

          FareClassCode fareClass = ptf.fareClass();
          if (fareClass.size() > 7)
            fareClass = fareClass.substr(0, 7) + "*";
          dc << std::setw(8) << fareClass << " ";

          std::string fareBasis = ptf.createFareBasis(trx);
          if (fareBasis.size() > 7)
            fareBasis = fareBasis.substr(0, 7) + "*";
          dc << std::setw(8) << fareBasis << " ";

          if (ptf.directionality() == FROM)
            dc << std::setw(2) << "O";
          else if (ptf.directionality() == TO)
            dc << std::setw(2) << "I";
          else
            dc << std::setw(2) << "-";

          dc << "R2:";
          dc << std::setw(5) << ptf.vendor();
          dc << std::setw(3) << ptf.fare()->carrier();
          dc << std::setw(5) << ptf.fareTariff();

          dc << std::setw(7) << ptf.ruleNumber();
          dc << " D";
          dc << std::setw(2) << categoryNumber;
          dc << '\n';
        }

        dc << std::endl << "SYSTEM DEFAULT APPLIED FOR CAT " << std::setw(2) << std::left
           << categoryNumber;

        if (retResultOfRule == FAIL)
        {
          dc << std::setw(24) << std::right << "---FAIL--- ";
        }
        else if (retResultOfRule == PASS)
        {
          dc << std::setw(24) << std::right << "---PASS--- ";
        }
        else
        {
          dc << std::setw(24) << std::right << "---SKIP--- ";
        }

        dc << std::endl << "---------------------------------------------------------" << std::endl;

        diagPtr->flushMsg();
      }
    }
  }
  return retResultOfRule;
}

Record3ReturnTypes
RuleController::processFareGroupRules(PricingTrx& trx,
                                      RuleControllerDataAccess& da,
                                      const uint16_t categoryNumber)

{ // lint !e578
  RuleControllerParam rcParam;
  RuleProcessingData rpData;

  StopoversInfoWrapper soInfoWrapper(da.currentFU());
  TransfersInfoWrapper trInfoWrapper(da.currentFU());
  rpData.soInfoWrapper(&soInfoWrapper);
  rpData.trInfoWrapper(&trInfoWrapper);

  return processFareRuleCommon(trx, da, categoryNumber, rcParam, rpData, true);
}

bool
RuleController::diag500Parameters(PricingTrx& trx, DCFactory*& factory, Diag500Collector*& diag)
{
  if (!trx.diagnostic().isActive())
    return false;
  factory = DCFactory::instance();
  diag = dynamic_cast<Diag500Collector*>(factory->create(trx));

  diag->enable(Diagnostic500);
  diag->givenValidationPhase() = Diag500Collector::ANY;

  bool rv = true;
  std::map<std::string, std::string>::iterator end = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::iterator begin =
      trx.diagnostic().diagParamMap().find(Diag500Collector::SHORT_OUTPUT);

  if (begin != end)
  {
    if (!((*begin).second).empty())
    {
      std::string categPh = (*begin).second;
      if (categPh == Diag500Collector::PREVALIDATION)
      {
        rv = (_categoryPhase == PreValidation);
        diag->givenValidationPhase() = Diag500Collector::PRV;
      }
      else if (categPh == Diag500Collector::NORMAL_OR_FARECOMP_VALIDATION)
      {
        rv = (_categoryPhase == ShoppingComponentValidation ||
              _categoryPhase == ShoppingAcrossStopOverComponentValidation ||
              _categoryPhase == ShoppingComponentWithFlightsValidation ||
              _categoryPhase == ShoppingASOComponentWithFlightsValidation);
        diag->givenValidationPhase() = Diag500Collector::NFV;
      }
      else if (categPh == Diag500Collector::REVALIDATION)
      {
        // rv =  (_categoryPhase == ReValidation);
        rv = (_categoryPhase == PURuleValidation || _categoryPhase == PURuleValidationIS ||
              _categoryPhase == PURuleValidationISALT || _categoryPhase == FPRuleValidation ||
              _categoryPhase == FPRuleValidationISALT);
        diag->givenValidationPhase() = Diag500Collector::REV;
      }
      else if (categPh == Diag500Collector::DYNAMIC_VALIDATION)
      {
        rv = (_categoryPhase == DynamicValidation);
        diag->givenValidationPhase() = Diag500Collector::DYV;
      }
      else
      {
        diag->givenValidationPhase() = Diag500Collector::ANY;
        rv = false;
      }
    }
  }
  switch (_categoryPhase)
  {
  case PreValidation:
  case ShoppingComponentValidation:
  case ShoppingAcrossStopOverComponentValidation:
  case ShoppingComponentWithFlightsValidation:
  case ShoppingASOComponentWithFlightsValidation:
    diag->validationPhase() = Diag500Collector::NFV;
    break;

  // case ReValidation:
  case FPRuleValidation:
  case FPRuleValidationISALT:
    diag->validationPhase() = Diag500Collector::REV;
    break;
  case DynamicValidation:
    diag->validationPhase() = Diag500Collector::DYV;
    break;
  default:
    break;
  }

  _diagCategoryNumber = 0;
  diag->categoryNumber() = 0;

  begin = trx.diagnostic().diagParamMap().find(Diag500Collector::SPECIFIC_CATEGORY);

  if (begin != end)
  {
    if (!((*begin).second).empty())
    {
      std::string catNum = (*begin).second;
      uint16_t categoryNumber = atoi(catNum.c_str());

      if (categoryNumber != 0)
      {
        diag->categoryNumber() = categoryNumber;
        _diagCategoryNumber = categoryNumber;
      }
    }
  }
  return rv;
}

bool
RuleController::isCatNeededInDiag(const std::vector<CategoryRuleItemInfoSet*>& ruleSet) const
{
  if (_diagCategoryNumber == 0)
    return true;

  for (const auto& setPtr : ruleSet)
  {
    for (const auto& item : *setPtr)
    {
      if (item.itemcat() == _diagCategoryNumber)
      {
        return true;
      }
    }
  }
  return false;
}

bool
RuleController::isCatQualifiedFltApp(const PaxTypeFare& paxTypeFare, const uint16_t category) const
{
  if (paxTypeFare.qualifyFltAppRuleDataMap().find(category) !=
      paxTypeFare.qualifyFltAppRuleDataMap().end())
  {
    return true;
  }
  return false;
}

bool
RuleController::isDirectional(const CategoryRuleInfo& catRuleInfo)
{
  // we should have a flag in categoryRuleInfo and save it in cache after
  // the first check, to avoid duplicated process
  for (const auto& setPtr : catRuleInfo.categoryRuleItemInfoSet())
  {
    for (const auto& info : *setPtr)
    {
      if (info.inOutInd() != RuleConst::ALWAYS_APPLIES)
      {
        return true;
      }
    }
  }
  return false;
}

bool
RuleController::isThereQualifier(const CategoryRuleInfo& catRuleInfo,
                                 const uint16_t& qualifierCatNum) const
{
  // we should have a flag in categoryRuleInfo and save it in cache after
  // the first check, to avoid duplicated process
  if (UNLIKELY(catRuleInfo.categoryNumber() == qualifierCatNum))
    return false;

  for (const auto& setPtr : catRuleInfo.categoryRuleItemInfoSet())
  {
    for (const auto& item : *setPtr)
    {
      if (item.itemcat() == qualifierCatNum)
      {
        return true;
      }
    }
  }
  return false;
}

// ----------------------------------------
// updateFareDisplayInfo
// This method updates rules, travel info of base PaxTypeFare to another
// This situation arries when we process one paxTypeFare but the rule
// is applied to file some other fare. ( cat25 override )
//-----------------------------------------
bool
RuleController::updateFareDisplayInfo(uint16_t cat, PricingTrx& trx, const PaxTypeFare& paxTypeFare)
    const
{
  if (LIKELY(trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX))
    return false;

  // Get destination paxTypeFare's fareDisplayInfo
  FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());
  if (!fareDisplayInfo)
    return false;

  // Get base PaxTypeFare
  const PaxTypeFare* basePaxTypeFare = paxTypeFare.fareWithoutBase();
  if (!basePaxTypeFare)
    return false;

  // Get source paxTypeFare's fareDisplayInfo
  FareDisplayInfo* baseFareDisplayInfo =
      const_cast<FareDisplayInfo*>(basePaxTypeFare->fareDisplayInfo());
  if (!baseFareDisplayInfo)
    return false;

  // ------------------

  // Copy Travel Info
  // ------------------

  if (cat == 14)
  {
    std::vector<TravelInfo*>& srcTravelInfo = baseFareDisplayInfo->travelInfo();
    std::vector<TravelInfo*>& destTravelInfo = fareDisplayInfo->travelInfo();

    destTravelInfo.insert(destTravelInfo.end(), srcTravelInfo.begin(), srcTravelInfo.end());
  }

  // ------------------
  // Copy Ticket Info
  // ------------------
  if (cat == 15)
  {
    std::vector<TicketInfo*>& srcTicketInfo = baseFareDisplayInfo->ticketInfo();
    std::vector<TicketInfo*>& destTicketInfo = fareDisplayInfo->ticketInfo();

    destTicketInfo.insert(destTicketInfo.end(), srcTicketInfo.begin(), srcTicketInfo.end());
  }

  // ------------------
  // Set FB Display
  // ------------------

  FBDisplay::setRuleRecordData(cat, trx, *basePaxTypeFare, paxTypeFare);

  return true;
}

//--------------------------------------------------------
// saveTktDates
// This method saves Ticket dates from Cat15 in FareMarket
// for further reuse
//---------------------------------------------------------

void
RuleController::saveTktDates(PaxTypeFare& paxTypeFare,
                             FareMarket::FareMarketSavedGfrResult::Result* savingRet)
{
  const FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

  if (!fdInfo)
  {
    LOG4CXX_ERROR(logger, "Unable to get FareDisplayInfo object");
    LOG4CXX_INFO(logger, " Leaving FDSalesRestrictionRule::validate() - FAIL");

    return;
  }

  for (const auto* const ticketInfo : fdInfo->ticketInfo())
  {
    if (fdInfo->ruleType() == ticketInfo->ruleType())
    {
      if (ticketInfo->earliestTktDate().isValid())
        savingRet->_earliestTktDT = ticketInfo->earliestTktDate();
      if (ticketInfo->latestTktDate().isValid())
        savingRet->_latestTktDT = ticketInfo->latestTktDate();
    }
  }
}

//--------------------------------------------------------
// setTktDates
// This method saves Ticket dates from Cat15 in FareMarket
// for further reuse
//---------------------------------------------------------

void
RuleController::setTktDates(PaxTypeFare& paxTypeFare,
                            const DateTime& earliestTktDate,
                            const DateTime& latestTktDate)
{
  FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

  if (!fdInfo)
  {
    LOG4CXX_ERROR(logger, "Unable to get FareDisplayInfo object");
    return;
  }
  if (earliestTktDate.isValid() || latestTktDate.isValid())
  {
    fdInfo->addTicketInfo(earliestTktDate, latestTktDate);
  }
}

// ----------------------------------------
// updateFareDisplayInfo
// This method updates rules, travel info of base PaxTypeFare to another
// This situation arries when we process one paxTypeFare but the rule
// is applied to file some other fare. ( cat25 override )
//-----------------------------------------
bool
RuleController::saveRuleInfo(PricingTrx& trx,
                             uint16_t cat,
                             const PaxTypeFare& paxTypeFare,
                             const FareByRuleItemInfo& fbrItemInfo)
{
  if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
    return false;

  // Get destination paxTypeFare's fareDisplayInfo
  FareDisplayInfo* fareDisplayInfo = const_cast<FareDisplayInfo*>(paxTypeFare.fareDisplayInfo());
  if (!fareDisplayInfo)
    return false;

  // Get base PaxTypeFare
  const PaxTypeFare* basePaxTypeFare = paxTypeFare.fareWithoutBase();
  if (!basePaxTypeFare)
    return false;

  // Get source paxTypeFare's fareDisplayInfo
  const FareDisplayInfo* baseFareDisplayInfo = basePaxTypeFare->fareDisplayInfo();
  if (!baseFareDisplayInfo)
    return false;

  // ------------------
  // Copy Seasonal Application
  // (only if category override isn't 'X')
  // ------------------
  if (cat == RuleConst::SEASONAL_RULE &&
      (fbrItemInfo.ovrdcat3() != 'X' || (baseFareDisplayInfo->ruleType() == RuleConst::FOOT_NOTE)))
  {
    std::vector<SeasonsInfo*>::const_iterator seasonsInfoI = baseFareDisplayInfo->seasons().begin();
    std::vector<SeasonsInfo*>::const_iterator seasonsInfoEnd = baseFareDisplayInfo->seasons().end();
    for (; seasonsInfoI != seasonsInfoEnd; seasonsInfoI++)
      fareDisplayInfo->seasons().push_back(*seasonsInfoI);
  }

  // ------------------
  // Copy Travel Purchase
  // (only if category override isn't 'X')
  // ------------------
  if (cat == 5 && fbrItemInfo.ovrdcat5() != 'X')
  {
    // Save Advance Purchase data in current FareDisplayInfo
    fareDisplayInfo->lastResPeriod() = baseFareDisplayInfo->lastResPeriod();
    fareDisplayInfo->lastResUnit() = baseFareDisplayInfo->lastResUnit();
    fareDisplayInfo->ticketed() = baseFareDisplayInfo->ticketed();
    fareDisplayInfo->advTktPeriod() = baseFareDisplayInfo->advTktPeriod();
    fareDisplayInfo->advTktUnit() = baseFareDisplayInfo->advTktUnit();
  }

  // Save Minstay data in current FareDisplayInfo
  // only if category override is not 'X'
  if (cat == 6 && fbrItemInfo.ovrdcat6() != 'X')
  {
    fareDisplayInfo->minStay() = baseFareDisplayInfo->minStay();
  }

  // Save Maxstay data in current FareDisplayInfo
  // (only if category override isn't 'X')
  if (cat == 7 && fbrItemInfo.ovrdcat7() != 'X')
  {
    fareDisplayInfo->maxStay() = baseFareDisplayInfo->maxStay();
    fareDisplayInfo->maxStayUnit() = baseFareDisplayInfo->maxStayUnit();
  }

  // ----------------------------------------------------------------------------
  // Copy surcharge data only if basefare surcharges are needed
  // (when ovrdcat12 is not  'X' )
  // ----------------------------------------------------------------------------
  if (cat == 12 &&
      (fbrItemInfo.ovrdcat12() != 'X' || (baseFareDisplayInfo->ruleType() == RuleConst::FOOT_NOTE)))
  {
    // Save the surcharge data in the current FareDisplayInfo
    fareDisplayInfo->inboundSurchargeData().insert(
        fareDisplayInfo->inboundSurchargeData().end(),
        baseFareDisplayInfo->inboundSurchargeData().begin(),
        baseFareDisplayInfo->inboundSurchargeData().end());

    fareDisplayInfo->outboundSurchargeData().insert(
        fareDisplayInfo->outboundSurchargeData().end(),
        baseFareDisplayInfo->outboundSurchargeData().begin(),
        baseFareDisplayInfo->outboundSurchargeData().end());
  }
  // -----------------------------------------------
  // Copy Travel Info when basefare travel is needed
  // -----------------------------------------------
  if (cat == 14 &&
      (fbrItemInfo.ovrdcat14() != 'X' || (baseFareDisplayInfo->ruleType() == RuleConst::FOOT_NOTE)))
  {
    std::vector<TravelInfo*>::const_iterator travelInfoI =
        baseFareDisplayInfo->travelInfo().begin();
    std::vector<TravelInfo*>::const_iterator travelInfoEnd =
        baseFareDisplayInfo->travelInfo().end();
    for (; travelInfoI != travelInfoEnd; travelInfoI++)
      fareDisplayInfo->travelInfo().push_back(*travelInfoI);
  }

  // -----------------------------------------------
  // Copy Ticket Info when basefare travel is needed
  // -----------------------------------------------
  if (cat == 15 &&
      (fbrItemInfo.ovrdcat15() != 'X' || (baseFareDisplayInfo->ruleType() == RuleConst::FOOT_NOTE)))
  {
    std::vector<TicketInfo*>::const_iterator ticketInfoI =
        baseFareDisplayInfo->ticketInfo().begin();
    std::vector<TicketInfo*>::const_iterator ticketInfoEnd =
        baseFareDisplayInfo->ticketInfo().end();
    for (; ticketInfoI != ticketInfoEnd; ticketInfoI++)
      fareDisplayInfo->ticketInfo().push_back(*ticketInfoI);
  }
  return true;
}

void
RuleController::getFootnotes(PaxTypeFare& paxTypeFare, std::vector<FootnoteTable>& footNoteTbl)
{
  std::vector<Footnote> footNotes;
  std::vector<TariffNumber> tariffNumbers;

  RuleUtil::getFootnotes(paxTypeFare, footNotes, tariffNumbers);

  uint16_t footNoteTblSz = footNotes.size();

  // should be of same size, just ensure no vector overflow
  if (UNLIKELY(footNoteTblSz > tariffNumbers.size()))
    footNoteTblSz = tariffNumbers.size();

  footNoteTbl.resize(footNoteTblSz);

  std::vector<FootnoteTable>::iterator fnTblIter = footNoteTbl.begin();
  std::vector<FootnoteTable>::iterator fnTblIterEnd = footNoteTbl.end();
  std::vector<Footnote>::const_iterator footNoteIter = footNotes.begin();
  std::vector<TariffNumber>::const_iterator tariffNumIter = tariffNumbers.begin();

  for (; fnTblIter != fnTblIterEnd; fnTblIter++, footNoteIter++, tariffNumIter++)
  {
    fnTblIter->footNote() = *footNoteIter;
    fnTblIter->tariffNumber() = *tariffNumIter;
  }

  paxTypeFare.fare()->resetFnMissingStat(footNoteTblSz);
}

bool
RuleController::missedFootnote(PaxTypeFare& paxTypeFare,
                               const std::vector<FootnoteTable>& footNoteTbl,
                               PricingTrx& trx)
{
  if (_categoryPhase == PreValidation ||
      ((trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::AF_EXC_TRX) &&
       (static_cast<const RexBaseTrx&>(trx)).isAnalyzingExcItin()))
    return false;

  if (noFailOnFNMissing(paxTypeFare))
    return false;

  if (paxTypeFare.isDiscounted())
  {
    // We could have shared discounted rule status with base fare,
    // thus skipped validation now it is time to share the footnote
    // status too
    PaxTypeFare* baseFare = nullptr;

    try
    {
      baseFare = paxTypeFare.baseFare(19); // lint !e578
      // exception would be thrown if baseFare == 0
    }
    catch (...)
    {
    }

    if (LIKELY(baseFare))
    {
      uint8_t missStat =
          paxTypeFare.fare()->getMissFootnoteStat() & baseFare->fare()->getMissFootnoteStat();

      if (!missStat)
      {
        paxTypeFare.fare()->setNotMissingAnyFootnote();
        baseFare->fare()->setNotMissingAnyFootnote();
      }
    }
  }

  if ((_categoryPhase != FPRuleValidation) && (_categoryPhase != FPRuleValidationISALT) &&
      (_categoryPhase != FareDisplayValidation) && (_categoryPhase != RuleDisplayValidation))
    return false;

  uint16_t missingFnIndex = 0;

  if (!paxTypeFare.fare()->missedFootnote(missingFnIndex))
  {
    return false;
  }

  // Fail by footnote missing will be in diag555
  const Footnote& footNote = footNoteTbl[missingFnIndex].footNote();
  const TariffNumber& tariffNumber = footNoteTbl[missingFnIndex].tariffNumber();

  DiagManager diag(trx, Diagnostic555);

  if (UNLIKELY(diag.isActive() &&
               !trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, Diagnostic::MAX_PEN)))
  {
    diag << "     " << paxTypeFare.fareClass() << " FOOTNOTE " << footNote << " TARIFF "
         << tariffNumber;
    diag << " MISSING - FAILED RULE\n";
  }

  LOG4CXX_ERROR(logger,
                "Missing footnoot " << footNote << " tariff " << tariffNumber << " of "
                                    << paxTypeFare.vendor() << " " << paxTypeFare.carrier()
                                    << " - Fail the fare " << paxTypeFare.fareClass());

  paxTypeFare.fare()->setMissingFootnote(true);

  return true;
}

bool
RuleController::noFailOnFNMissing(const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isFareByRule() && !paxTypeFare.isSpecifiedFare())
  {
    return true;
  }

  if (paxTypeFare.isDiscounted())
  {
    PaxTypeFare* baseFare = nullptr;

    try
    {
      baseFare = paxTypeFare.baseFare(19);
      // exception would be thrown if baseFare == 0
    }
    catch (...)
    {
    }

    if (LIKELY(baseFare))
    {
      return noFailOnFNMissing(*baseFare);
    }
  }
  return false;
}

void
RuleController::updateRuleType(PaxTypeFare& paxTypeFare, const Indicator& ruleType)
{
  FareDisplayInfo* fdInfo = paxTypeFare.fareDisplayInfo();

  if (!fdInfo)
  {
    LOG4CXX_ERROR(logger, "Unable to get FareDisplayInfo object");
    return;
  }

  fdInfo->ruleType() = ruleType;
}
// ----------------------------------------
// setFBRBaseFareRuleInfo - FB and Rule Display
// When Cat25 override Tag is blank; base fare rule must be saved in FBDisplay object
// for future use during RD and FB displays.
//-----------------------------------------
void
RuleController::setFBRBaseFareRuleInfo(PricingTrx& trx,
                                       uint16_t category,
                                       const PaxTypeFare& paxTypeFare,
                                       const FareByRuleItemInfo* fbrItemInfo)
{
  bool useFBRFareRule = false;
  bool useBaseFareRule = false;

  RuleUtil::determineRuleChecks(category, *fbrItemInfo, useFBRFareRule, useBaseFareRule);

  if (useBaseFareRule && useFBRFareRule)
  {
    // Get base PaxTypeFare
    const PaxTypeFare* basePaxTypeFare = paxTypeFare.fareWithoutBase();

    if (!basePaxTypeFare)
      return;

    FBDisplay::setRuleRecordData(category, trx, *basePaxTypeFare, paxTypeFare, useBaseFareRule);
  }
  return;
}

bool
RuleController::skipReuse(const uint16_t categoryNum)
{
  static bool skipReuseInitialized = collectSkipReuse();
  SUPPRESS_UNUSED_WARNING(skipReuseInitialized);

  LOG4CXX_DEBUG(logger, "skipReuseInitialized:" << skipReuseInitialized);

  if (categoryNum <= MAX_REUSE_CAT)
    return (_skipReuseCat[categoryNum]);
  else
    return true;
}

bool
RuleController::collectSkipReuse()
{
  for (uint16_t i = 0; i <= MAX_REUSE_CAT; i++)
  {
    _skipReuseCat[i] = false;
  }
  ConfigVector<uint16_t> skippedCat = noReuseCategories.getValue();
  if (noReuseCategories.isDefault() || skippedCat.begin() == skippedCat.end() ||
      noReuseCategories.getRawValue() == "N")
    return true;
  for (const auto sc : skippedCat)
  {
    if (sc <= MAX_REUSE_CAT)
      _skipReuseCat[sc] = true;
  }
  return true;
}

Record3ReturnTypes
RuleController::processFareRuleInfo(PricingTrx& trx,
                                    RuleControllerDataAccess& da,
                                    GeneralFareRuleInfoVec& gfrInfoVec,
                                    const uint16_t categoryNumber,
                                    const GeneralFareRuleInfo* gfrInfo,
                                    bool& isLocationSwapped,
                                    RuleControllerParam* rcParam,
                                    RuleProcessingData& rpData,
                                    const bool isFareRule,
                                    bool skipCat15Security)

{
  Record3ReturnTypes ret = SKIP;
  if (UNLIKELY(gfrInfoVec.size() == 0))
    return ret;

  PaxTypeFare& paxTypeFare = da.paxTypeFare();
  bool historicalCat12 = false;
  if (UNLIKELY((categoryNumber == RuleConst::SURCHARGE_RULE) && trx.dataHandle().isHistorical() &&
               (gfrInfoVec.size() > 1)))
  {
    historicalCat12 = true;
    da.cloneFU(gfrInfoVec.size());
  }

  bool processNextR2 = false;
  PaxTypeFareRuleData* ptfRuleData = paxTypeFare.paxTypeFareRuleData(categoryNumber);

  if (UNLIKELY(trx.dataHandle().isHistorical() && gfrInfoVec.size() > 1 && ptfRuleData &&
               (RuleUtil::isDiscount(categoryNumber) ||
                categoryNumber == RuleConst::NEGOTIATED_RULE ||
                categoryNumber == RuleConst::FARE_BY_RULE)))
  {
    processNextR2 = true;
  }

  unsigned int ruleIndex = 0;
  // looping through General Fare Rule vector
  GeneralFareRuleInfoVec::iterator iter = gfrInfoVec.begin();
  GeneralFareRuleInfoVec::iterator iterEnd = gfrInfoVec.end();
  bool isHistorical = trx.dataHandle().isHistorical();
  for (; iter != iterEnd; ++iter)
  {
    gfrInfo = (*iter).first;
    if (UNLIKELY(gfrInfo == nullptr))
      continue;
    isLocationSwapped = (*iter).second;

    if (!fallback::fallbackValCxrR2Cat15(&trx))
      _loopValidatingCxrs = _categoryPhase != DynamicValidation && !_highLevelLoopValidatingCxrs &&
                            trx.isValidatingCxrGsaApplicable() &&
                            !paxTypeFare.fareMarket()->validatingCarriers().empty() &&
                            existCat15QualifyCat15ValidatingCxrRest(trx, *gfrInfo);

    if (UNLIKELY(processNextR2))
    {
      // Match sequence number
      if (ptfRuleData->categoryRuleInfo()->sequenceNumber() != gfrInfo->sequenceNumber() ||
          ptfRuleData->categoryRuleInfo()->createDate() != gfrInfo->createDate())
        continue;
    }

    ret = processGnFareRule(trx,
                            da,
                            paxTypeFare,
                            *gfrInfo,
                            categoryNumber,
                            rcParam,
                            rpData,
                            isLocationSwapped,
                            isFareRule,
                            skipCat15Security);

    if (UNLIKELY(historicalCat12))
    {
      if (da.processRuleResult(ret, ruleIndex))
      {
        if (ruleIndex != 0 && ruleIndex < gfrInfoVec.size())
        {
          gfrInfo = gfrInfoVec[ruleIndex].first;
          isLocationSwapped = gfrInfoVec[ruleIndex].second;
          paxTypeFare.setCatRuleInfo(
              gfrInfo, categoryNumber, trx.dataHandle(), isLocationSwapped, isFareRule);
        }
        break;
      }
      ruleIndex++;
    }
    else if (LIKELY(!isHistorical || ret != FAIL))
      break;
  }
  return ret;
}

void
RuleController::diagReuse(PricingTrx& trx,
                          const CategoryRuleInfo* catRuleInfo,
                          const PaxTypeFare& paxTypeFare,
                          const uint16_t categoryNumber,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber* ruleNumber,
                          const Footnote* footNote,
                          const Indicator ruleType,
                          const FareClassCode& fcReusedFrom,
                          const Record3ReturnTypes& reusedRet)
{
  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  DCFactory* factory = nullptr;

  if (catRuleInfo->applInd() != RuleConst::STRING_DOES_NOT_APPLY)
  {
    Diag500Collector* diag500 = nullptr;
    bool isCatNeededInDiag500 = false;

    if (diagType == Diagnostic500 && diag500Parameters(trx, factory, diag500))
    {
      isCatNeededInDiag500 = isCatNeededInDiag(catRuleInfo->categoryRuleItemInfoSet());
      if (isCatNeededInDiag500)
      {
        diag500->printRulePhase(_categoryPhase);
        if (_categoryPhase == DynamicValidation)
        {
          *diag500 << (_fmPhase ? "FARE MARKET SCOPE\n" : "PRICING UNIT SCOPE\n");
          if (paxTypeFare.selectedFareNetRemit())
            *diag500 << " SELECTED TKT CAT35 FARE FOR NET REMIT\n";
        }
        if (ruleNumber != nullptr)
        {
          const GeneralFareRuleInfo* gfrRuleInfo =
              dynamic_cast<const GeneralFareRuleInfo*>(catRuleInfo);
          if (gfrRuleInfo)
            diag500->diag500Collector(paxTypeFare, gfrRuleInfo, ruleType == 'R');
        }
        else
        {
          const FootNoteCtrlInfo* fnCtrlInfo = dynamic_cast<const FootNoteCtrlInfo*>(catRuleInfo);
          if (fnCtrlInfo)
            diag500->diag500Collector(paxTypeFare, fnCtrlInfo);
        }

        *diag500 << " REUSE RESULT OF " << fcReusedFrom << " ON FAREMARKET\n";
        diag500->diag500Collector(reusedRet);

        if (ruleNumber != nullptr)
        {
          const RexPricingTrx* rexTrx = static_cast<const RexPricingTrx*>(&trx);
          if ((rexTrx != nullptr) && (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE))
          {
            diag500->displayRetrievalDate(paxTypeFare);
            *diag500 << "\n";
          }
        }

        diag500->flushMsg();
      }
    }
  }

  bool shouldDisplay = false;

  if (diagType == Diagnostic511)
  {
    shouldDisplay = trx.diagnostic().shouldDisplay(paxTypeFare, categoryNumber);
  }
  else if ((diagType > Diagnostic300) && (diagType < Diagnostic325))
  {
    uint16_t catNum = static_cast<uint16_t>(diagType) - 300;
    shouldDisplay = (catNum == categoryNumber) && trx.diagnostic().shouldDisplay(paxTypeFare);

    if (shouldDisplay && diagType == Diagnostic315 &&
        categoryNumber == RuleConst::SALE_RESTRICTIONS_RULE)
    {
      if (!(DiagnosticUtil::isvalidForCarrierDiagReq(trx, paxTypeFare) ||
            (paxTypeFare.fare()->cat15HasSecurity() && reusedRet != FAIL)))
        shouldDisplay = false;
    }
  }

  if (shouldDisplay)
  {
    if (!factory)
      factory = DCFactory::instance();
    DiagCollector* diag = factory->create(trx);
    if (diag && diag->parseFareMarket(trx, *(paxTypeFare.fareMarket())))
    {
      diag->enable(diagType);

      *diag << "FM: " << paxTypeFare.fareMarket()->origin()->loc()
            << paxTypeFare.fareMarket()->destination()->loc() << std::setw(10)
            << paxTypeFare.fareClass();
      *diag << std::setw(5) << vendor << " " << carrier << std::setw(5) << ruleTariff;
      if (ruleNumber != nullptr)
        *diag << std::setw(7) << (*ruleNumber);
      else if (footNote != nullptr)
        *diag << std::setw(7) << (*footNote);
      *diag << " SEQ:" << std::setw(9) << catRuleInfo->sequenceNumber();
      *diag << " " << ruleType << categoryNumber << "\n     REUSE RESULT OF " << fcReusedFrom
            << " -" << reusedRet << '\n';

      diag->flushMsg();
    }
  }
}

struct Rule1or15 : public std::unary_function<uint16_t, bool>
{
  bool operator()(uint16_t rule)
  {
    return (rule == RuleConst::ELIGIBILITY_RULE || rule == RuleConst::SALE_RESTRICTIONS_RULE);
  }
};

void
RuleController::skipRexSecurityCheck(const RexBaseTrx& trx,
                                     PaxTypeFare& ptf,
                                     std::vector<uint16_t>& categorySequence)
{
  if (trx.isAnalyzingExcItin() &&
      (trx.skipSecurityForExcItin() ||
       (ptf.fareMarket()->fareCompInfo() && ptf.fareMarket()->fareCompInfo()->hasVCTR())))
  {
    ptf.setCategoryProcessed(RuleConst::ELIGIBILITY_RULE, true);
    ptf.setCategoryProcessed(RuleConst::SALE_RESTRICTIONS_RULE, true);

    ptf.setCategoryValid(RuleConst::ELIGIBILITY_RULE, true);
    ptf.setCategoryValid(RuleConst::SALE_RESTRICTIONS_RULE, true);

    categorySequence.erase(remove_if(categorySequence.begin(), categorySequence.end(), Rule1or15()),
                           categorySequence.end());
  }
}
void
RuleController::removeCat(const uint16_t category)
{
  _categorySequence.erase(std::remove(_categorySequence.begin(), _categorySequence.end(), category),
                          _categorySequence.end());
}

bool
RuleController::mayBeSkippedByCmdPricing(const PricingTrx& trx,
                                         const PaxTypeFare& paxTypeFare,
                                         uint16_t category)
{
  return category != RuleConst::TRAVEL_RESTRICTIONS_RULE && category != RuleConst::MISC_FARE_TAG &&
         category != RuleConst::FARE_BY_RULE && category != RuleConst::NEGOTIATED_RULE &&
         (category != RuleConst::SALE_RESTRICTIONS_RULE ||
          (category == RuleConst::SALE_RESTRICTIONS_RULE && paxTypeFare.cat15SoftPass() &&
           (trx.excTrxType() != PricingTrx::AR_EXC_TRX ||
            static_cast<const RexBaseTrx&>(trx).trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)));
}
bool
RuleController::dontReuseSaleRestrictionRule(
    const FareMarket::FareMarketSavedGfrResult::Result& savedRet, const PaxTypeFare& paxTypeFare)
{
  return (savedRet._ret == FAIL &&
          savedRet._ptfResultFrom->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ||
         (savedRet._ptfResultFrom->tcrTariffCat() != RuleConst::PRIVATE_TARIFF &&
          paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF) ||
         (savedRet._ptfResultFrom->tcrTariffCat() == RuleConst::PRIVATE_TARIFF &&
          paxTypeFare.tcrTariffCat() != RuleConst::PRIVATE_TARIFF);
}

void
RuleController::copySavedValidationResult_deprecated(
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedFnResult::Result& savedRet)
{
  if (categoryNumber == RuleConst::ACCOMPANIED_PSG_RULE)
  {
    if (savedRet._needCat13AccFareBreak)
    {
      paxTypeFare.setAccSameFareBreak(true);

      PaxTypeFareRuleData* ptfRuleData =
          paxTypeFare.paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      const PaxTypeFareRuleData* ptfRuleDataFrom =
          savedRet._ptfResultFrom->paxTypeFareRuleData(RuleConst::ACCOMPANIED_PSG_RULE);
      if (ptfRuleData && ptfRuleDataFrom)
      {
        ptfRuleData->ruleItemInfo() = ptfRuleDataFrom->ruleItemInfo();
      }
    }
  }
  else if (categoryNumber == RuleConst::PENALTIES_RULE)
  {
    if (savedRet._penaltyRestInd)
      paxTypeFare.penaltyRestInd() = YES;
  }
  else if (categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE &&
           savedRet._ptfResultFrom->getNVAData())
  {
    paxTypeFare.useNVAData(savedRet._ptfResultFrom->getNVAData());
  }
}

void
RuleController::copySavedValidationResult(
    PaxTypeFare& paxTypeFare,
    uint16_t categoryNumber,
    const FareMarket::FareMarketSavedFnResult::Result& savedRet)
{
  if (UNLIKELY(categoryNumber == RuleConst::TRAVEL_RESTRICTIONS_RULE && savedRet._cat14NVAData))
  {
    paxTypeFare.useNVAData(savedRet._cat14NVAData);
  }
}

bool
RuleController::shouldApplyGeneralRuleSurcharges(PricingTrx& trx, const PaxTypeFare& ptf) const
{
  PaxTypeFareRuleData* ptFrData = ptf.paxTypeFareRuleData(RuleConst::SURCHARGE_RULE);

  if (nullptr != ptFrData && nullptr != ptFrData->categoryRuleInfo())
  {
    const GeneralFareRuleInfo* i =
        dynamic_cast<const GeneralFareRuleInfo*>(ptFrData->categoryRuleInfo());

    //   "2"  =  Do not apply either carrier filed general rule surcharges or surcharges from the
    // sector-surcharge table
    //   "N"  =  Do not apply carrier filed general rule surcharges
    if (UNLIKELY('2' == i->generalRuleAppl()))
    {
      return false;
    }
    if ('N' == i->generalRuleAppl())
    {
      return false;
    }
  }

  if (LIKELY(trx.dataHandle().getVendorType(ptf.vendor()) != RuleConst::SMF_VENDOR))
    return false;

  return true;
}

bool
RuleController::fareApplicableToGeneralRuleSurcharges(PricingTrx& trx,
                                                      const PaxTypeFare& ptf,
                                                      const PaxTypeFare* candidate) const
{
  if (candidate == nullptr)
    return false;

  //  Try to match any paxTapeFare to FMS fare.
  if (trx.dataHandle().getVendorType(candidate->vendor()) != RuleConst::PUBLIC_VENDOR)
    return false; // skip fares with non public vendor

  if (candidate->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
    return false; // skip private fares

  if ((candidate->carrier() != ptf.carrier()) ||
      (candidate->globalDirection() != ptf.globalDirection()) || (candidate->inhibit() != 'N'))
    return false;

  if (candidate->isDiscounted() || candidate->isFareByRule() || candidate->isNegotiated())
    return false;

  return true;
}

void
RuleController::copyFlexFaresValidationStatus(PricingTrx& trx,
                                              PaxTypeFare& target,
                                              const PaxTypeFare& source,
                                              const uint16_t& category)
{
  if (LIKELY((!target.hasFlexFareValidationStatus()) || (!source.hasFlexFareValidationStatus())))
    return;

  flexFares::ValidationStatusPtr targetStatus = target.getMutableFlexFaresValidationStatus();
  const flexFares::ValidationStatusPtr sourceStatus = source.getFlexFaresValidationStatus();
  switch (category)
  {
    // NGSII-741: Restrict copying all categories except Acc Code/Corp Id.
    if (fallback::fallbackFlexFareCopyValidationStatusForCorpIdAccCode(&trx))
    {
    case RuleConst::PENALTIES_RULE:
      targetStatus->updateAttribute<flexFares::NO_PENALTIES>(sourceStatus);
      break;

    case RuleConst::ADVANCE_RESERVATION_RULE:
      targetStatus->updateAttribute<flexFares::NO_ADVANCE_PURCHASE>(sourceStatus);
      break;

    case RuleConst::MINIMUM_STAY_RULE:
    case RuleConst::MAXIMUM_STAY_RULE:
      targetStatus->updateAttribute<flexFares::NO_MIN_MAX_STAY>(sourceStatus);
      break;
    }

  case RuleConst::ELIGIBILITY_RULE:
    if (!fallback::fallbackFlexFareCopyValidationStatusForCorpIdAccCode(&trx) && !targetStatus)
      break;

    targetStatus->updateAttribute<flexFares::CORP_IDS>(sourceStatus);
    targetStatus->updateAttribute<flexFares::ACC_CODES>(sourceStatus);
    break;
  default:
    // Categories not supported, do nothing
    break;
  }
}

bool
RuleController::existCat15QualifyCat15ValidatingCxrRest(PricingTrx& trx,
                                                        const CategoryRuleInfo& catRuleInfo) const
{
  for (const auto& setPtr : catRuleInfo.categoryRuleItemInfoSet())
  {
    for (const auto& info : *setPtr)
    {
      if (info.itemcat() == 15)
      {
        if (UNLIKELY(trx.getTrxType() == PricingTrx::FF_TRX))
          return true;

        // dive into record3
        const RuleItemInfo* ruleItemInfo = RuleUtil::getRuleItemInfo(trx, &catRuleInfo, &info);
        const SalesRestriction* salesRest = dynamic_cast<const SalesRestriction*>(ruleItemInfo);
        if (salesRest && salesRest->validationInd() != RuleConst::BLANK)
          return true;
      }
    }
  }
  return false;
}

} // namespace tse
