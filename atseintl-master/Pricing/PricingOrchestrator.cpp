/*----------------------------------------------------------------------------
 *  File:    PricingOrchestrator.cpp
 *  Created: Dec 19, 2003
 *  Authors: Dave Hobt, Steve Suggs, Mark Kasprowicz, Mohammad Hossan
 *
 *  Description:
 *
 *  Change History:
 *    2/10/2004 - SMS - adding hooks to call the various pricing functions.
 *
 *  Copyright Sabre 2003
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/PricingOrchestrator.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "BookingCode/MixedClassController.h"
#include "BrandedFares/BrandedFaresComparator.h"
#include "BrandedFares/BrandedFaresSelector.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/AirlineShoppingUtils.h"
#include "Common/AltPricingUtil.h"
#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/IntlJourneyUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Memory/OutOfMemoryException.h"
#include "Common/MetricsUtil.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RemoveFakeTravelSeg.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/Thread/ThreadPoolFactory.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TNBrands/BrandingOptionSpacesDeduplicator.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "Common/YQYR/YQYRCalculator.h"
#include "DataModel/Agent.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/Itin.h"
#include "DataModel/MetricsTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PrecalcBaggageData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag527Collector.h"
#include "Diagnostic/Diag606Collector.h"
#include "Diagnostic/Diag611Collector.h"
#include "Diagnostic/Diag690Collector.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag982Collector.h"
#include "Diagnostic/Diag990Collector.h"
#include "Diagnostic/Diag993Collector.h"
#include "Diagnostic/Diag994Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/AllowanceFinder.h"
#include "FreeBagService/AllowanceSoftPassProcessor.h"
#include "FreeBagService/BaggageItinAnalyzer.h"
#include "FreeBagService/BaggageLowerBoundCalculator.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/ChargeSoftPassProcessor.h"
#include "Pricing/AltDateItinThreadTask.h"
#include "Pricing/CombinabilityScoreboard.h"
#include "Pricing/Combinations.h"
#include "Pricing/EstimatedSeatValue.h"
#include "Pricing/FareMarketMerger.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/FarePathCollector.h"
#include "Pricing/FarePathFactory.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FarePathValidator.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/ItinThreadTask.h"
#include "Pricing/MaximumPenaltyValidator.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/MIPFamilyLogicUtils.h"
#include "Pricing/PaxFarePathFactory.h"
#include "Pricing/PQDiversifier.h"
#include "Pricing/PricingUnitFactory.h"
#include "Pricing/PricingUnitFactoryBucket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PUPath.h"
#include "Pricing/SavedSurcharges.h"
#include "Pricing/Shopping/PQ/SoloOrchestrator.h"
#include "Pricing/ShoppingPQ.h"
#include "Pricing/SimilarItin/Context.h"
#include "Pricing/SimilarItin/DiagnosticWrapper.h"
#include "Pricing/SimilarItin/FarePathValidator.h"
#include "Pricing/SimilarItin/PriceWithSavedFPPQItems.h"
#include "Pricing/SimilarItin/Pricing.h"
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"
#include "Pricing/StructuredFareRulesUtils.h"
#include "Pricing/SurchargesPreValidator.h"
#include "Pricing/SurchargesThreadTask.h"
#include "Rules/AccompaniedTravel.h"
#include "Rules/Config.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"
#include "Server/TseServer.h"
#include "Util/BranchPrediction.h"
#include "Util/FlatMap.h"

#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackRoutingForChildren);
FALLBACK_DECL(fallback_FLE_combinabilityAll);
FALLBACK_DECL(fallback_FLE_combinability106);
FALLBACK_DECL(fallbackGSAChildSCI740);
FALLBACK_DECL(bagInPqLowerBound);
FALLBACK_DECL(fallbackFlexFareGroupNewXCLogic);
FALLBACK_DECL(fallbackSumUpAvlForMotherItin);
FALLBACK_DECL(fallbackFlexFareGroupNewXOLogic);
FALLBACK_DECL(fallbackCorpIDFareBugFixing);
FALLBACK_DECL(fallbackBfaLoopTimeout);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackSoldoutOriginRT)
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(applyExcDiscToMatchedRefundFares);
FALLBACK_DECL(allow100PExcDiscounts);

namespace
{
ConfigurableValue<uint16_t>
shortCktTimeOut("PRICING_SVC", "SHORT_CKT_TIMEOUT", 60);
ConfigurableValue<uint16_t>
minShortCktTimeOutShopping("PRICING_SVC", "MIN_SHORT_CKT_TIMEOUT_SHOPPING", 3);
ConfigurableValue<uint16_t>
percentageShortCktTimeOutShopping("PRICING_SVC", "PERCENTAGE_SHORT_CKT_TIMEOUT_SHOPPING", 25);
ConfigurableValue<uint16_t>
applyDomShoppingCktTimeoutValues("PRICING_SVC", "APPLY_DOM_SHOPPING_CKT_TIMEOUT_VALUES");
ConfigurableValue<uint16_t>
percentageShortCktTimeoutDomShopping("PRICING_SVC", "PERCENTAGE_SHORT_CKT_TIMEOUT_DOM_SHOPPING", 6);
ConfigurableValue<uint16_t>
minShortCktTimeoutDomShopping("PRICING_SVC", "MIN_SHORT_CKT_TIMEOUT_DOM_SHOPPING", 0);
ConfigurableValue<uint16_t>
percentageShortCktTimeOutShoppingMultiPax("PRICING_SVC",
                                          "PERCENTAGE_SHORT_CKT_TIMEOUT_SHOPPING_MULTIPAX");
ConfigurableValue<bool>
delayPUValidationMIPCfg("SHOPPING_OPT", "DELAY_PU_VALIDATION_MIP", false);
ConfigurableValue<uint16_t>
nextGFPAttemptsNumber("SHOPPING_OPT", "NEXT_GFP_FOR_SIMILAR_ITIN_ATTEMPTS_NUMBER", 1);
ConfigurableValue<uint16_t>
puScopeValMaxNum("PRICING_SVC", "PU_SCOPE_VALIDATION_MAXNUM", 500);
ConfigurableValue<uint16_t>
maxSPCTFareCompCount("PRICING_SVC", "MAX_SP_CT_FARE_COMP_COUNT", 2);
ConfigurableValue<uint16_t>
maxSPCTFareCompCount_1B1F("PRICING_SVC", "MAX_SP_CT_FARE_COMP_COUNT_ABACUS_INFINI", 2);
ConfigurableValue<uint32_t>
maxSPCTPUCount("PRICING_SVC", "MAX_SP_CT_PU_COUNT", 2000);
ConfigurableValue<int32_t>
maxSearchNextLevelFarePath("SHOPPING_OPT", "MAX_SEARCH_NEXTLEVEL_FAREPATH", -1);
ConfigurableValue<int32_t>
maxSearchNextLevelFarePathAlt("SHOPPING_OPT", "MAX_SEARCH_NEXTLEVEL_FAREPATH_ALT", 100000);
ConfigurableValue<int32_t>
maxSearchNextLevelFarePathComplex("SHOPPING_OPT", "MAX_SEARCH_NEXTLEVEL_FAREPATH_COMPLEX", 3000);
ConfigurableValue<bool>
enableFailedFareUsageOptimizationCfg("PRICING_SVC", "SCOREBOARD_FAILED_FARE_USAGE_TUNING", true);
ConfigurableValue<bool>
enablePULevelFailedFareUsageOptimizationCfg("PRICING_SVC",
                                            "PU_FAILED_FARE_USAGE_PAIR_TUNING",
                                            true);
ConfigurableValue<bool>
enableFPLevelFailedFareUsageOptimizationCfg("PRICING_SVC",
                                            "FP_FAILED_FARE_USAGE_PAIR_TUNING",
                                            true);
ConfigurableValue<uint32_t>
additionalFarePathCount("PRICING_SVC", "ADDITIONAL_FARE_PATH_COUNT");
ConfigurableValue<uint16_t>
altDateItinPriceJumpFactorCfg("PRICING_SVC", "ALT_DATE_ITIN_PRICE_JUMP_FACTOR", 7);
ConfigurableValue<uint16_t>
altDateCutOffNucThresholdCfg("PRICING_SVC", "ALT_DATE_CUTOFF_NUC_THRESHOLD", 100);
ConfigurableValue<int32_t>
hundredsOptionsReqAdjustPercent("PRICING_SVC", "HUNDREDS_OPTIONS_REQUEST_ADJUST", 100);
ConfigurableValue<int32_t>
hundredsOptionsResAdjustPercent("PRICING_SVC", "HUNDREDS_OPTIONS_RESPONSE_ADJUST", 100);
ConfigurableValue<int32_t>
numTaxCallForMipAltDate("SHOPPING_OPT", "NUM_TAXCALL_FOR_MIP_ALTDATE", 2);
ConfigurableValue<int32_t>
maxDirectFlightOnlySolution("SHOPPING_OPT", "MAX_DIRECT_FLIGHT_ONLY_SOLUTION", 1);
ConfigurableValue<bool>
taxPerFareBreakCfg("SHOPPING_OPT", "TAX_PER_FARE_BREAK", false);
ConfigurableValue<double>
shoppingAltDateCutoff("SHOPPING_OPT", "SHOPPING_ALTDATE_CUTOFF_AMOUNT_FACTOR", 120);
ConfigurableValue<double>
shoppingAltDateCutoffSnowman("SHOPPING_OPT", "SHOPPING_ALTDATE_CUTOFF_SNOWMAN_AMOUNT_FACTOR", 250);
ConfigurableValue<double>
altDateOptionRemoveFactorCfg("SHOPPING_OPT", "ALT_DATE_OPTION_REMOVE_FACTOR", 1.5);
ConfigurableValue<uint16_t>
percentageShortCktOnPushbackTimeout("PRICING_SVC", "PERCENTAGE_SHORT_CKT_ON_PUSHBACK_TIMEOUT", 95);
ConfigurableValue<uint16_t>
shutdownFPFSSecondsBeforeTimeout("PRICING_SVC", "SHUTDOWN_FPFS_SECONDS_BEFORE_TIMEOUT", 1);
ConfigurableValue<bool>
showCrossBrandOptionFirst("S8_BRAND_SVC", "SHOW_CROSS_BRAND_OPTION_FIRST", true);
}

Logger
PricingOrchestrator::_logger("atseintl.PricingOrchestrator");

PricingOrchestrator::PricingOrchestrator(TseServer& srv)
  : _config(srv.config()), _factoriesConfig(_config), _bitmapOpOrderer(srv.config()), _server(srv)
{
  EstimatedSeatValue::classInit();
  _shortCktTimeOut = shortCktTimeOut.getValue();
  _minShortCktTimeOutShopping = minShortCktTimeOutShopping.getValue();
  _percentageShortCktTimeOutShopping = percentageShortCktTimeOutShopping.getValue();
  // 0 - don't apply | 1 - apply to VIS | 2 - apply to all domestic
  _applyDomShoppingCktTimeoutValues = applyDomShoppingCktTimeoutValues.getValue();
  _percentageShortCktTimeoutDomShopping = percentageShortCktTimeoutDomShopping.getValue();
  _minShortCktTimeoutDomShopping = minShortCktTimeoutDomShopping.getValue();
  _percentageShortCktTimeOutShoppingMultiPax = percentageShortCktTimeOutShoppingMultiPax.getValue();
  _delayPUValidationMIP = delayPUValidationMIPCfg.getValue();

  _nextGFPAttemptsNumber = nextGFPAttemptsNumber.getValue();

  _puScopeValMaxNum = puScopeValMaxNum.getValue();
  _maxSPCTFareCompCount = maxSPCTFareCompCount.getValue();
  _maxSPCTFareCompCount_1B1F = maxSPCTFareCompCount_1B1F.getValue();
  _maxSPCTPUCount = maxSPCTPUCount.getValue();
  _maxSearchNextLevelFarePath = maxSearchNextLevelFarePath.getValue();
  _maxSearchNextLevelFarePathAlt = maxSearchNextLevelFarePathAlt.getValue();
  _maxSearchNextLevelFarePathComplex = maxSearchNextLevelFarePathComplex.getValue(); // IS/MIP
  CombinabilityScoreboard::enableFailedFareUsageOptimization(
      enableFailedFareUsageOptimizationCfg.getValue());
  Combinations::enablePULevelFailedFareUsageOptimization(
      enablePULevelFailedFareUsageOptimizationCfg.getValue());
  Combinations::enableFPLevelFailedFareUsageOptimization(
      enableFPLevelFailedFareUsageOptimizationCfg.getValue());

  _additionalFarePathCount = additionalFarePathCount.getValue();

  if (_additionalFarePathCount > 25)
  {
    _additionalFarePathCount = 25;
  }

  // Stop altDate-MIP if price jumps, e.g. is 10 time of the cheapest Itin
  //
  _altDateItinPriceJumpFactor = altDateItinPriceJumpFactorCfg.getValue();
  // AltDate-MIP Cutoff threshold, e.g. NUC 100. Do not cutoff even if it reaches
  // 10 times of the cheapest one when the amount is already very low.
  //
  _altDateCutOffNucThreshold = altDateCutOffNucThresholdCfg.getValue();
  _hundredsOptionsReqAdjustPercent = hundredsOptionsReqAdjustPercent.getValue(); // IS
  _hundredsOptionsResAdjustPercent = hundredsOptionsResAdjustPercent.getValue(); // IS
  _numTaxCallForMipAltDate = numTaxCallForMipAltDate.getValue();
  _maxDirectFlightOnlySolution = maxDirectFlightOnlySolution.getValue(); // IS

  _taxPerFareBreak = taxPerFareBreakCfg.getValue();

  _altDateCutOffAmount = shoppingAltDateCutoff.getValue() / 100.0; // IS

  _altDateCutOffSnowmanAmount = shoppingAltDateCutoffSnowman.getValue() / 100.0; // IS
  _altDateOptionRemoveFactor = altDateOptionRemoveFactorCfg.getValue(); // IS
  _pqDiversifier.init(); // IS
}

/*-----------------------------------------------------------------------------
 * process function
 *
 * main Pricing Orchestrator process transaction method
 *
 * @param trx - reference to a valid transaction object
 * @return    - returns true on success, false on error
 *----------------------------------------------------------------------------*/
bool
PricingOrchestrator::process(MetricsTrx& trx)
{
  std::ostringstream& oss = trx.response();
  MetricsUtil::header(oss, "PO Metrics");
  MetricsUtil::lineItemHeader(oss);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_COMB_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_LIMITATION);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_MINFARE_PROCESS);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_RULE_VALIDATE);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_BKGCODE_REVALIDATE);
  MetricsUtil::lineItem(oss, MetricsUtil::PO_BKGCODE_MIXED_CLASS);
  return true;
}

void
PricingOrchestrator::detailedFailureMsg(PricingTrx& trx, Itin& itin)
{
  if (trx.getRequest()->isMultiTicketRequest() && !trx.multiTicketMap().empty() &&
      itin.getMultiTktItinOrderNum() == 0 &&
      itin.errResponseCode() != ErrorResponseException::NO_ERROR)
  {
    throw ErrorResponseException(itin.errResponseCode(), itin.errResponseMsg().c_str());
  }

  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);
  VALIDATE_OR_THROW(!itin.salesNationRestr(), PRICING_REST_BY_GOV, trx.status());
  VALIDATE_OR_THROW(!trx.getOptions()->isPublishedFares(),
                    NO_PUBLIC_FARES_VALID_FOR_PASSENGER,
                    "NO PUBLIC FARES VALID FOR PASSENGER TYPE/"
                    "CLASS OF SERVICE");
  VALIDATE_OR_THROW(!trx.getOptions()->isPrivateFares(),
                    NO_PRIVATE_FARES_VALID_FOR_PASSENGER,
                    "NO PRIVATE FARES VALID FOR PASSENGER TYPE/"
                    "CLASS OF SERVICE");
  const bool isWpa = (altPricingTrx && (altPricingTrx->altTrxType() == AltPricingTrx::WP_NOMATCH ||
                                        altPricingTrx->altTrxType() == AltPricingTrx::WPA));
  VALIDATE_OR_THROW(!trx.getOptions()->isXoFares() || isWpa,
                    NO_RULES_FOR_PSGR_TYPE_OR_CLASS,
                    "NO RULES VALID FOR PASSENGER TYPE/"
                    "CLASS OF SERVICE");
  VALIDATE_OR_THROW(
      trx.getOptions()->isXoFares(), NO_FARE_FOR_CLASS_USED, "NO FARE FOR CLASS USED");
  VALIDATE_OR_THROW(!trx.getOptions()->forceCorpFares(),
                    NO_CORPORATE_NEG_FARES_EXISTS,
                    "REPRICE - NO CORPORATE NEGOTIATED FARES EXIST");
}

void
PricingOrchestrator::detailedFailureMsgAfterPricing(PricingTrx& trx, Itin& itin)
{
  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  if (trx.getOptions()->isIataFares())
  {
    std::string errMsg;
    checkWPCYYentry(trx, errMsg, itin);
    VALIDATE_OR_THROW(errMsg.empty(), NO_APPLICABLE_YY_FARES, errMsg);
  }
  else if (trx.getRequest()->isGoverningCarrierOverrideEntry() && trx.itin().size() == 1)
  {
    const std::string errMsg = getWPCxxErrMsg(trx, itin);
    VALIDATE_OR_THROW(errMsg.empty(), REQ_CARRIER_HAS_NO_FARES, errMsg);
  }

  bool throwCmdPricingMsg = noPNRPricingTrx == nullptr && trx.getOptions()->fbcSelected();
  VALIDATE_OR_THROW(!throwCmdPricingMsg, FARE_BASIS_NOT_AVAIL, "FORMAT FARE BASIS NOT AVAILABLE");
  VALIDATE_OR_THROW(!trx.getOptions()->forceCorpFares(),
                    NO_CORPORATE_NEG_FARES_EXISTS,
                    "REPRICE - NO CORPORATE NEGOTIATED FARES EXIST");
}

bool
PricingOrchestrator::itinHasAtLeastOneEmptyFMForOvrCxr(PricingTrx& trx, const Itin& itin)
{
  const CarrierCode ovrCxr = trx.getRequest()->governingCarrierOverrides().begin()->second;
  return itin.hasAtLeastOneEmptyFMForGovCxr(ovrCxr);
}

bool
PricingOrchestrator::priceItinTask(PricingTrx& trx,
                                   Itin& itin,
                                   PUPathMatrix& puMatrix,
                                   std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  TSELatencyData metrics(trx, "PO PRICE ITIN");
  AltPricingTrx* altPricingTrx = dynamic_cast<AltPricingTrx*>(&trx);
  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  bool ret = false;
  bool isDiagActive = trx.isDiagActive();
  bool isBranded =
      (trx.getRequest()->brandedFareEntry() || trx.getRequest()->isBrandedFaresRequest() ||
       trx.isBrandsForTnShopping() || trx.activationFlags().isSearchForBrandsPricing());

  if (!fallback::fallbackFlexFareGroupNewXOLogic(&trx))
    checkFlexFareGroupXOFareStatus(trx, puMatrix.getFlexFaresGroupId());

  if (!puMatrix.puPathMatrix().empty() || (noPNRPricingTrx && isDiagActive))
  {
    if (!isBranded && !trx.isFlexFare() && !itin.farePath().empty() &&
        !(trx.isLowestFareOverride() && altPricingTrx &&
          altPricingTrx->altTrxType() == AltPricingTrx::WPA &&
          trx.getRequest()->isLowFareRequested()) &&
        !noPNRPricingTrx)
    {
      itin.farePath().clear();
    }

    if ((altPricingTrx && (altPricingTrx->altTrxType() == AltPricingTrx::WP_NOMATCH ||
                           altPricingTrx->altTrxType() == AltPricingTrx::WPA)) ||
        trx.isRfbListOfSolutionEnabled())
    {
      ret = priceItin(*altPricingTrx, puMatrix, puFactoryBucketVect);
    }
    else
    {
      ret = priceItin(trx, puMatrix, puFactoryBucketVect);
    }
  }
  else
  {
    diag.enable(&puMatrix, Diagnostic600, Diagnostic603, Diagnostic601, Diagnostic609);
    if (diag.isActive())
    {
      diag.printHeader();
      diag << "\n\nNO RESULT - FAREMARKET MISSING?\n\n";
      diag.printLine();
      diag.flushMsg();

      LOG4CXX_ERROR(_logger, "Pricing a" << (isBranded ? " brand" : "n itin") << " failed");
    }

    if (!isBranded && !trx.isFlexFare())
      detailedFailureMsg(trx, itin);
  }

  if (!ret && !isBranded && !trx.isFlexFare())
    detailedFailureMsgAfterPricing(trx, itin);

  return ret;
}
void
PricingOrchestrator::checkFlexFareGroupXOFareStatus(PricingTrx& trx,
                                                    const flexFares::GroupId groupId)
{
  if (trx.isFlexFare())
  {
    if (trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupId))
      trx.getOptions()->xoFares() =
          trx.getRequest()->getMutableFlexFaresGroupsData().getFlexFareXOFares(groupId);
    else
      trx.getOptions()->xoFares() = trx.getOptions()->mainGroupXOFares();
  }
}

bool
PricingOrchestrator::priceADItinTask(PricingTrx& trx,
                                     PUPathMatrixVec& puMatrixVec,
                                     std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  TSELatencyData metrics(trx, "PO PRICE AD ITIN");
  return priceItin(trx, puMatrixVec, puFactoryBucketVect);
}

namespace
{
struct NotPricedSpecifiedLeg
{
  AirlineShoppingUtils::LegId _id;

  NotPricedSpecifiedLeg(AirlineShoppingUtils::LegId id) : _id(id) {}

  bool operator()(Itin* itin)
  {
    if (itin->legID().front().first == _id && itin->farePath().empty())
      return true;

    return false;
  }
};

void
restoreAggregatedAvailability(PricingTrx& trx)
{
  std::stringstream diagStr;
  Diag982Collector* diag = nullptr;
  bool diag982 = (trx.diagnostic().diagnosticType() == Diagnostic982) &&
                 (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL");

  if (UNLIKELY(diag982))
  {
    DCFactory* factory = DCFactory::instance();
    diag = dynamic_cast<Diag982Collector*>(factory->create(trx));
    diag->enable(Diagnostic982);
    diagStr << "\n*** RESTORING AVL OF MOTHERS ***\n";
  }

  for (Itin* itin : trx.itin())
  {
    if (UNLIKELY(diag982))
      diagStr << "\n*** PROCESSING AVAILIBILITY FOR ITIN " << itin->itinNum() << " ***\n\n";

    std::map<ClassOfService*, size_t>& aggregatedMap = itin->getFamilyAggregatedAvlMap();
    if (aggregatedMap.empty())
    {
      if (UNLIKELY(diag982))
        diagStr << "NOTHING TO DO\n";
    }
    else
    {
      if (UNLIKELY(diag982))
      {
        diagStr << "AVAILABILITY BEFORE RESTORE:\n";
        std::vector<std::string> motherInfo = diag->getMotherAvailabilityAsString(trx, *itin);
        for (std::string line : motherInfo)
          diagStr << line << "\n";
      }

      for (auto& mapItem : itin->getFamilyAggregatedAvlMap())
        mapItem.first->numSeats() = mapItem.second;

      if (UNLIKELY(diag982))
      {
        diagStr << "\nAVAILABILITY AFTER RESTORE:\n";
        std::vector<std::string> motherInfo = diag->getMotherAvailabilityAsString(trx, *itin);
        for (std::string line : motherInfo)
          diagStr << line << "\n";
      }
    }
  }

  if (UNLIKELY(diag982))
  {
    diagStr << "\n*** END OF AVL RESTORING ***\n";
    *diag << diagStr.str() << "\n";
    diag->flushMsg();
  }
}
}

/*-----------------------------------------------------------------------------
 * process function
 *
 * main Pricing Orchestrator process transaction method
 *
 * @param trx - reference to a valid transaction object
 * @return    - returns true on success, false on error
 *----------------------------------------------------------------------------*/
bool
PricingOrchestrator::process(PricingTrx& trx)
{
  TSELatencyData metrics(trx, "PO PROCESS");
  LOG4CXX_INFO(_logger, "PO_process. Num of Itin passed with Trx=" << trx.itin().size());
  checkDelayExpansion(trx);

  if (!fallback::fallbackSumUpAvlForMotherItin(&trx) && trx.isBRAll())
    restoreAggregatedAvailability(
        trx); // revert to origin availability in family logic mother itins

  if ((trx.getRequest()->owPricingRTTaxProcess() ||
       AirlineShoppingUtils::enableItinLevelThreading(trx)))
    cloneItinObjects(trx);

  if (trx.isFlexFare())
  {
    if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx))
      trx.getOptions()->initialForceCorpFares() = trx.getOptions()->forceCorpFares();
    else
      _isForcedCorpFares = trx.getOptions()->forceCorpFares();
  }

  if (trx.hasNoEmptyRfbOrAdditionalAttrOnEachSeg() &&
      trx.billing()->requestPath() != SWS_PO_ATSE_PATH)
    trx.setRfblistOfSolution(true);

  getValidBrands(trx);

  if (trx.isPbbRequest() == PBB_RQ_PROCESS_BRANDS)
    BrandingUtil::processSoftPassFares(trx);

  //---------------------------------------------------
  // Instantiate all the buckets of PricingUnitFactory
  // One bucket per PaxType
  //
  std::vector<PricingUnitFactoryBucket*> puFactoryBucketVect;

  if (!createPricingUnitFactoryBucket(trx, puFactoryBucketVect))
  {
    return false;
  }

  if (TrxUtil::isBaggageInPQEnabled(trx))
    prepareBaggageInPq(trx);

  //---------------------------------------------------
  // Create PUPath Matrix, one per (brand,itin)
  //
  const uint16_t brandedTableSize = getBrandSize(trx);

  PUPathMatrixVecTable puPathMatrixVectTable(brandedTableSize);
  std::vector<PUPathMatrixVecTable> tnShopPuPathMatrixVectTables(trx.itin().size());

  if (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
  {
    for (size_t itinIdx = 0; itinIdx < trx.itin().size(); ++itinIdx)
    {
      Itin* itin = trx.itin()[itinIdx];
      const size_t spacesCount = itin->getItinBranding().getBrandingOptionSpacesCount();

      PUPathMatrixVecTable& perItinTable = tnShopPuPathMatrixVectTables[itinIdx];
      perItinTable.resize(spacesCount);

      for (uint16_t poptSpaceIndex = 0; poptSpaceIndex < spacesCount; ++poptSpaceIndex)
      {
        buildPUPathMatrixForItin(
            trx, *itin, puFactoryBucketVect, perItinTable[poptSpaceIndex], poptSpaceIndex);
      }
    }
  }
  else
  {
    for (uint16_t brandIndex = 0; brandIndex < brandedTableSize; brandIndex++)
    {
      if (!trx.isAltDates() && !ShoppingUtil::isBrandValid(trx, brandIndex))
        continue;

      checkFlexFaresForceCorpFares(trx, brandIndex);
      buildPUPathMatrix(trx, puFactoryBucketVect, puPathMatrixVectTable[brandIndex], brandIndex);
    }
  }

  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();

  if (displayDiagnostics(trx, diagType, puFactoryBucketVect))
  {
    // Just want to see FareMarketPath and PUPath Matrix. No need to continue with pricing
    return true;
  }

  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *factory->create(trx);
  diag.enable(Diagnostic603, Diagnostic601, Diagnostic605, Diagnostic606, Diagnostic661);
  diag.printHeader();

  //------------- Init PricingUnit Factory -------------------------
  //
  if (!trx.delayXpn())
  {
    initPricingUnitFactory(trx, puFactoryBucketVect, diag);
    diag.flushMsg();
  }

  //------------------ Price Itins -----------------
  //
  bool itinPassed = false;

  if (trx.isAltDates())
  {
    itinPassed = priceCheapestItin(trx, puPathMatrixVectTable, puFactoryBucketVect);
  }
  else if (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
  {
    itinPassed = priceAllTNBrandedItin(trx, tnShopPuPathMatrixVectTables, puFactoryBucketVect);
  }
  else
  {
    itinPassed = priceAllItin(trx, puPathMatrixVectTable, puFactoryBucketVect);
  }

  displayDiagnostics606(diagType, factory, diag);
  RexExchangeTrx* rexExTrx = dynamic_cast<RexExchangeTrx*>(&trx);

  if (!(rexExTrx && !RexBaseTrx::isRexTrxAndNewItin(trx)))
  {
    promoteChildItins(trx);
  }

  if (!TrxUtil::taxEnhancementActivated(DateTime::fromMilitaryTime(1200)))
  {
    if (trx.getRequest()->originBasedRTPricing())
    {
      RemoveFakeTravelSeg::process(trx);
    }
  }
  else
  {
    if (trx.getRequest()->originBasedRTPricing())
    {
      setupCabinForDummyFareUsage(trx);
      resetSurchargeForDummyFareUsage(trx.itin());
    }
  }

  processOwPricingRTTaxProcess(trx);

  if (trx.isFlexFare())
  {
    if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx))
      trx.getOptions()->forceCorpFares() = trx.getOptions()->initialForceCorpFares();
    else
      trx.getOptions()->forceCorpFares() = _isForcedCorpFares;
  }

  return itinPassed;
}

static PrecalcBaggage::BagTravelData
prepareBaggageTravelData(const BaggageItinAnalyzer& analyzer, BaggageTravel& bt, PaxType& paxType)
{
  PricingTrx& trx = *bt._trx;
  PrecalcBaggage::BagTravelData btData;

  if (bt.paxType() == &paxType)
    btData.bagTravel = &bt;
  else
  {
    btData.bagTravel = &trx.dataHandle().safe_create<BaggageTravel>(bt);
    btData.bagTravel->setPaxType(paxType);
  }

  AllowanceSoftPassProcessor allowanceProc(btData);
  ChargeSoftPassProcessor chargeProc(btData);
  AllowanceFinder finder(
      allowanceProc, chargeProc, *btData.bagTravel, analyzer.furthestCheckedPoint());
  finder.findAllowanceAndCharges();

  return btData;
}

static PrecalcBaggage::ItinData*
prepareBaggageItinData(PricingTrx& trx, Itin& itin)
{
  BaggageItinAnalyzer analyzer(trx, itin);
  analyzer.analyzeTravels();

  PrecalcBaggage::ItinData* const pbd = trx.dataHandle().create<PrecalcBaggage::ItinData>();
  pbd->paxData.reserve(trx.paxType().size());

  for (PaxType* paxType : trx.paxType())
  {
    PrecalcBaggage::PaxData& paxData = pbd->paxData[paxType];
    paxData.bagTravelData.reserve(analyzer.baggageTravels().size());

    for (BaggageTravel* const bt : analyzer.baggageTravels())
      paxData.bagTravelData.push_back(prepareBaggageTravelData(analyzer, *bt, *paxType));
  }

  return pbd;
}

void
PricingOrchestrator::prepareBaggageInPq(PricingTrx& trx)
{
  TSELatencyData metrics(trx, "BAG PREVALIDATION");

  DiagManager diag852PR(trx, DiagnosticTypes::Diagnostic852, "PR");

  diag852PR << DiagCollector::Header();

  for (Itin* itin : trx.itin())
  {
    itin->setPrecalcBagData(prepareBaggageItinData(trx, *itin));
    diag852PR << *itin;
  }
}

void
PricingOrchestrator::calculateBaggageLowerBounds(PricingTrx& trx, const Itin& itin)
{
  TSELatencyData metrics(trx, "BAG LOWER BOUND");

  for (FareMarket* fm : itin.fareMarket())
  {
    uint16_t paxTypeNo = 0;

    for (PaxTypeBucket& ptc : fm->paxTypeCortege())
    {
      BaggageLowerBoundCalculator bagCalc(trx, itin, *ptc.requestedPaxType(), *fm);
      bool anyModified = false;

      for (PaxTypeFare* ptf : ptc.paxTypeFare())
      {
        if (!ptf->isValid())
          continue;
        bagCalc.calcLowerBound(*ptf);
        anyModified |= (ptf->baggageLowerBound() > EPSILON);
      }

      if (anyModified)
      {
        // TODO: I think it should be done in FareMarketMerger
        PaxTypeFare::FareComparator cmp(ptc, paxTypeNo, trx.getOptions()->isZeroFareLogic());
        std::sort(ptc.paxTypeFare().begin(), ptc.paxTypeFare().end(), cmp);
      }

      ++paxTypeNo;
    }
  }
}

uint16_t
PricingOrchestrator::getBrandSize(PricingTrx& trx)
{
  if (trx.getRequest()->brandedFareEntry())
    return trx.getRequest()->getBrandedFareSize();

  if (trx.getRequest()->isBrandedFaresRequest())
    return trx.validBrands().size();

  if (trx.isFlexFare())
    return trx.getRequest()->getFlexFaresGroupsData().getSize();

  return 1;
}

void
PricingOrchestrator::checkDelayExpansion(PricingTrx& trx)
{
  if (trx.getTrxType() != PricingTrx::MIP_TRX)
    return;

  if (trx.getRequest()->owPricingRTTaxProcess())
    return;

  trx.delayXpn() = true;
}

void
PricingOrchestrator::setupCabinForDummyFareUsage(PricingTrx& trx) const
{
  for (Itin* itin : trx.itin())
  {
    for (FarePath* farePath : itin->farePath())
    {
      for (PricingUnit* pricingUnit : farePath->pricingUnit())
      {
        std::vector<FareUsage*>::iterator const fakeFareUsageIt =
            RemoveFakeTravelSeg::findFakeFU(pricingUnit->fareUsage());
        std::vector<FareUsage*>::iterator const nonFakeFareUsageIt =
            RemoveFakeTravelSeg::findFirstNonFakeFU(pricingUnit->fareUsage());

        if (fakeFareUsageIt == pricingUnit->fareUsage().end() ||
            nonFakeFareUsageIt == pricingUnit->fareUsage().end())
        {
          continue;
        }

        FareUsage* fakeFareUsage = *fakeFareUsageIt;
        FareUsage* nonFakeFareUsage = *nonFakeFareUsageIt;
        PaxTypeFare::SegmentStatus& fakeFareUsageSegStat =
            fakeFareUsage->paxTypeFare()->segmentStatus()[0];
        PaxTypeFare::SegmentStatus& nonFakeFareUsageSegStat =
            nonFakeFareUsage->paxTypeFare()->segmentStatus()[0];
        fakeFareUsageSegStat._bkgCodeReBook = nonFakeFareUsageSegStat._bkgCodeReBook;
        fakeFareUsageSegStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
        break; // there is only single dummy FU per FP
      }
    }
  }
}

void
PricingOrchestrator::resetSurchargeForDummyFareUsage(std::vector<Itin*>& itins) const
{
  for (Itin* itin : itins)
  {
    for (FarePath* farePath : itin->farePath())
    {
      for (PricingUnit* pricingUnit : farePath->pricingUnit())
      {
        std::vector<FareUsage*>::iterator fu =
            RemoveFakeTravelSeg::findFakeFU(pricingUnit->fareUsage());

        if (fu == pricingUnit->fareUsage().end())
          continue;

        farePath->decreaseTotalNUCAmount((*fu)->surchargeAmt());
        (*fu)->surchargeAmt() = 0.0;
        break; // there is only single dummy FU per FP
      }
    }
  }
}

void
PricingOrchestrator::processOwPricingRTTaxProcess(PricingTrx& trx)
{
  if (trx.getRequest()->owPricingRTTaxProcess())
  {
    if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_OUTBOUND)
    {
      if (find_if(trx.itin().begin(),
                  trx.itin().end(),
                  NotPricedSpecifiedLeg(AirlineShoppingUtils::SECOND_LEG)) != trx.itin().end())
      {
        LOG4CXX_WARN(_logger, "Inbound fake leg not priced - switching to one way processing")
        trx.getRequest()->processingDirection() = ProcessingDirection::ONEWAY;
      }
    }
    else if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_INBOUND)
    {
      if (find_if(trx.itin().begin(),
                  trx.itin().end(),
                  NotPricedSpecifiedLeg(AirlineShoppingUtils::FIRST_LEG)) != trx.itin().end())
      {
        LOG4CXX_WARN(_logger, "Outbound fake leg not priced - switching to one way processing")
        trx.getRequest()->processingDirection() = ProcessingDirection::ONEWAY;
      }
    }

    if (trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_OUTBOUND ||
        trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP_INBOUND ||
        trx.getRequest()->processingDirection() == ProcessingDirection::ROUNDTRIP)
    {
      AirlineShoppingUtils::AseItinCombiner aseCombiner(trx);
      aseCombiner.combineOneWayItineraries();
    }

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic994))
    {
      DCFactory* factory = DCFactory::instance();
      Diag994Collector* dc = dynamic_cast<Diag994Collector*>(factory->create(trx));
      dc->enable(Diagnostic994);
      // At this point if processingDirection is ONEWAY it means that it was switched
      dc->displayItins(trx, trx.getRequest()->processingDirection() == ProcessingDirection::ONEWAY);
      dc->flushMsg();
    }
  }
}

void
PricingOrchestrator::promoteChildItins(PricingTrx& trx)
{
  RexExchangeTrx* rexExTrx = dynamic_cast<RexExchangeTrx*>(&trx);
  // isExcWFRNewItin = true - exchange shopping for cheapest option and new itins process
  const bool isExcWFRNewItin =
      rexExTrx && rexExTrx->billing()->actionCode() == "WFR" && RexBaseTrx::isRexTrxAndNewItin(trx);
  //'promote' child itins to be proper itins from now on,
  // duplicating their pricing information from their parent
  std::vector<Itin*> estimatedItins;
  const size_t toReserve = std::accumulate(trx.itin().cbegin(),
                                           trx.itin().cend(),
                                           0,
                                           [](size_t ret, const Itin* const itin)
                                           { return ret + itin->getSimilarItins().size(); });

  estimatedItins.reserve(trx.itin().size() * toReserve);
  for (size_t motherItinIndex = 0; motherItinIndex < trx.itin().size(); ++motherItinIndex)
  {
    Itin& motherItin = *trx.itin()[motherItinIndex];
    if (isExcWFRNewItin)
      rexExTrx->addMotherItinIndex(&motherItin, static_cast<uint16_t>(motherItinIndex));

    for (const SimilarItinData& similarItinData : motherItin.getSimilarItins())
    {
      Itin& similarItin = *similarItinData.itin;
      if (!similarItin.farePath().empty())
      {
        estimatedItins.push_back(&similarItin);

        if (isExcWFRNewItin)
          rexExTrx->addMotherItinIndex(&similarItin, static_cast<uint16_t>(motherItinIndex));
      }
    }

    motherItin.clearSimilarItins();
  }

  std::copy(estimatedItins.begin(), estimatedItins.end(), std::back_inserter(trx.itin()));
}

bool
PricingOrchestrator::priceAllItin(PricingTrx& trx,
                                  PUPathMatrixVecTable& puPathMatrixVectTable,
                                  std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  const bool isMIP = (trx.getTrxType() == PricingTrx::MIP_TRX);
  // if true drop all results on timeout, otherwise abort on timeout keeping partial results
  const bool isDropResultsOnTimeout = trx.getRequest()->isDropResultsOnTimeout();

  if (isMIP && trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry();
    else
      trx.setAbortOnHurry();
  }

  std::vector<ItinThreadTask> tasks;
  BaseExchangeTrx* excTrx = nullptr;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    excTrx = static_cast<BaseExchangeTrx*>(&trx);
  }

  ItinVector::iterator itinIter = trx.itin().begin();
  ItinVector::iterator itinIterEnd = trx.itin().end();
  const uint16_t brandedTableSize = getBrandSize(trx);

  TseThreadingConst::TaskId taskId;
  if (trx.getRequest()->owPricingRTTaxProcess() ||
      AirlineShoppingUtils::enableItinLevelThreading(trx))
    taskId = TseThreadingConst::PRICING_ITIN_ASE_TASK;
  else
    taskId = TseThreadingConst::PRICING_ITIN_TASK;

  try
  {
    tasks.reserve(trx.itin().size() * brandedTableSize);
    for (uint16_t brandIndex = 0; brandIndex < brandedTableSize; brandIndex++)
    {
      if (!ShoppingUtil::isBrandValid(trx, brandIndex))
        continue;

      checkFlexFaresForceCorpFares(trx, brandIndex);

      itinIter = trx.itin().begin();
      LOG4CXX_INFO(_logger, "Price all itins  brand-" << brandIndex);

      TseRunnableExecutor taskExecutor(taskId);

      PUPathMatrixMap& puPathMatrixMap = puPathMatrixVectTable[brandIndex];
      PUPathMatrixVec& puPathMatrixVect = (*puPathMatrixMap.begin()).second;

      for (uint16_t itinIdx = 0; itinIter != itinIterEnd; ++itinIter, ++itinIdx)
      {
        if (!isDropResultsOnTimeout)
        {
          try { checkTrxAborted(trx); }
          catch (...) { break; }
        }

        if (excTrx)
          excTrx->setItinIndex(itinIdx);

        Itin& itin = *(*itinIter);

        PUPathMatrix* puMatrix = puPathMatrixVect[itinIdx];
        if (puMatrix)
        {
          // TODO Will this be used for cheapest in BRAll?
          BrandCode* brandCode = nullptr;
          if (!trx.validBrands().empty() && !trx.isBrandsForTnShopping() &&
              !trx.activationFlags().isSearchForBrandsPricing())
            brandCode = &(trx.validBrands()[brandIndex]);

          if (brandCode)
            itin.fmpMatrix() = itin.brandFmpMatrices()[*brandCode];
          else if (trx.isFlexFare())
            itin.fmpMatrix() = itin.groupsFmpMatrices()[puMatrix->getFlexFaresGroupId()];

          tasks.emplace_back(*this, trx, itin, *puMatrix, puFactoryBucketVect);

          taskExecutor.execute(tasks.back());
        }
      } // for each itin

      taskExecutor.wait();
    } // for each brand
  }
  catch (const Memory::OutOfMemoryException&)
  {
    // we are running out of memory, stop processing this queue with what we priced so far
  }

  if (trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry(false);
    else
      trx.setAbortOnHurry(false);
  }

  if (excTrx)
    excTrx->setItinIndex(0);

  return returnOrThrow(trx, tasks);
}

namespace
{
void
removePriceOptionFromItin(uint16_t optionIndex, Itin& itin)
{
  auto compareIndexes = [=](const FarePath* farePath)
  { return farePath->brandIndex() == optionIndex; };
  itin.farePath().erase(
      std::remove_if(itin.farePath().begin(), itin.farePath().end(), compareIndexes),
      itin.farePath().end());
}

void
removeLastPriceOptionIfNeeded(const PricingTrx& trx, Itin& itin)
{
  if (trx.getNumberOfBrands() == static_cast<size_t>(TnShoppingBrandsLimit::UNLIMITED_BRANDS))
    return;

  auto priceOptionsIndexes = itin.getPricedBrandCombinationIndexes();

  if (priceOptionsIndexes.size() > trx.getNumberOfBrands())
    removePriceOptionFromItin(
        *std::max_element(priceOptionsIndexes.begin(), priceOptionsIndexes.end()), itin);
}

void
removeDuplicatesFromTNBrandedItin(PricingTrx& trx,
                                  Itin& itin,
                                  Diag892Collector* diag892,
                                  skipper::BrandingOptionSpacesDeduplicator& deduplicator)
{
  skipper::BrandingOptionSpacesDeduplicator::KeyBrandsMap keyBrandMap;
  deduplicator.calculateDeduplicationMap(itin, keyBrandMap);

  if (diag892 != nullptr)
    diag892->printDeduplicationInfo(&itin, keyBrandMap);

  auto indexesOfPriceOptionsToRemove =
      deduplicator.getBrandOptionIndexesToDeduplicate(itin, keyBrandMap);
  for (auto index : indexesOfPriceOptionsToRemove)
    removePriceOptionFromItin(index, itin);

  removeLastPriceOptionIfNeeded(trx, itin);
}

} // namespace

bool
PricingOrchestrator::priceAllTNBrandedItin(
    PricingTrx& trx,
    std::vector<PUPathMatrixVecTable>& tnShopPuPathMatrixVectTables,
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  const bool isMIP = (trx.getTrxType() == PricingTrx::MIP_TRX);
  // if true drop all results on timeout, otherwise abort on timeout keeping partial results
  const bool isDropResultsOnTimeout = trx.getRequest()->isDropResultsOnTimeout();

  if (isMIP && trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry();
    else
      trx.setAbortOnHurry();
  }

  BaseExchangeTrx* excTrx = nullptr;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    excTrx = static_cast<BaseExchangeTrx*>(&trx);
  }

  TseRunnableExecutor itinExecutor(TseThreadingConst::PRICING_ITIN_TASK);

  const size_t toReserve = std::accumulate(tnShopPuPathMatrixVectTables.cbegin(),
                                           tnShopPuPathMatrixVectTables.cend(),
                                           0,
                                           [](size_t ret, const PUPathMatrixVecTable& elem)
                                           { return ret + elem.size(); });
  std::vector<ItinThreadTask> tasks;
  try
  {
    tasks.reserve(trx.itin().size() * toReserve);
    if (!fallback::fallbackBfaLoopTimeout(&trx))
    {
      const size_t maxSpaces = std::accumulate(
          trx.itin().cbegin(), trx.itin().cend(), 0,
          [](size_t ret, const Itin* itin)
          { return std::max(ret, itin->getItinBranding().getBrandingOptionSpacesCount()); });

      for (size_t poptSpaceIndex = 0; poptSpaceIndex < maxSpaces; ++poptSpaceIndex)
      {
        for (uint16_t itinIdx = 0; itinIdx < trx.itin().size(); ++itinIdx)
        {
          if (!isDropResultsOnTimeout)
          {
            try { checkTrxAborted(trx); }
            catch (...) { break; }
          }

          Itin& itin = *trx.itin()[itinIdx];
          skipper::ItinBranding& itinBranding = itin.getItinBranding();

          if (poptSpaceIndex >= itinBranding.getBrandingOptionSpacesCount())
            continue;

          if (excTrx)
            excTrx->setItinIndex(itinIdx);

          PUPathMatrixVecTable& perItinTable = tnShopPuPathMatrixVectTables[itinIdx];
          PUPathMatrixMap& puPathMatrixMap = perItinTable[poptSpaceIndex];
          PUPathMatrixVec& puPathMatrixVect = (*puPathMatrixMap.begin()).second;

          PUPathMatrix* puMatrix = puPathMatrixVect[0]; // In this processing size always 1
          if (puMatrix)
          {
            itin.fmpMatrix() = itinBranding.getFmpMatrix(poptSpaceIndex);

            tasks.emplace_back(*this, trx, itin, *puMatrix, puFactoryBucketVect);
            // TODO Do we need to synchronize pushback of new fare paths to itin???

            itinExecutor.execute(tasks.back());

            // TODO SearchForBrandsPricingPrototype
            if (trx.activationFlags().isSearchForBrandsPricing())
              itinExecutor.wait();
          }
        }
      }
    }
    else
    {
      ItinVector::iterator itinIter = trx.itin().begin();
      ItinVector::iterator itinIterEnd = trx.itin().end();

      for (uint16_t itinIdx = 0; itinIter != itinIterEnd; ++itinIter, ++itinIdx)
      {
        if (excTrx)
          excTrx->setItinIndex(itinIdx);

        Itin& itin = **itinIter;
        skipper::ItinBranding& itinBranding = itin.getItinBranding();

        PUPathMatrixVecTable& perItinTable = tnShopPuPathMatrixVectTables[itinIdx];
        for (size_t poptSpaceIndex = 0; poptSpaceIndex < perItinTable.size(); ++poptSpaceIndex)
        {
          PUPathMatrixMap& puPathMatrixMap = perItinTable[poptSpaceIndex];
          PUPathMatrixVec& puPathMatrixVect = (*puPathMatrixMap.begin()).second;

          PUPathMatrix* puMatrix = puPathMatrixVect[0]; // In this processing size always 1
          if (puMatrix)
          {
            itin.fmpMatrix() = itinBranding.getFmpMatrix(poptSpaceIndex);

            tasks.emplace_back(*this, trx, itin, *puMatrix, puFactoryBucketVect);
            // TODO Do we need to synchronize pushback of new fare paths to itin???

            itinExecutor.execute(tasks.back());

            // TODO SearchForBrandsPricingPrototype
            if (trx.activationFlags().isSearchForBrandsPricing())
              itinExecutor.wait();
          }
        }
      } // for each itin
    }

    itinExecutor.wait();
  }
  catch (const Memory::OutOfMemoryException&)
  {
    // we are running out of memory, stop processing this queue with what we priced so far
  }

  if (trx.aborter())
  {
    if (fallback::reworkTrxAborter(&trx))
      trx.aborter()->setAbortOnHurry(false);
    else
      trx.setAbortOnHurry(false);
  }

  if (excTrx)
    excTrx->setItinIndex(0);

  const bool result = returnOrThrow(trx, tasks);

  if (result)
  {
    // Workaround for FarePath swapping in taxes; see TaxItinerary::accumulator().
    // The cloned FarePaths don't have their brandIndexes set. We update them here.
    // If we don't do this we would not be able to serialize the result properly.
    for (Itin* itin : trx.itin())
    {
      for (FarePath* farePath : itin->farePath())
      {
        for (FarePath* clonedFp : farePath->gsaClonedFarePaths())
        {
          TSE_ASSERT(clonedFp != nullptr);
          clonedFp->brandIndex() = farePath->brandIndex();
        }
      }
    }
  }

  DCFactory* factory = DCFactory::instance();
  Diag892Collector* diag892 = dynamic_cast<Diag892Collector*>(factory->create(trx));
  if (diag892 != nullptr)
    diag892->enable(Diagnostic892);

  if (result)
  {
    if (diag892 != nullptr)
      diag892->printDeduplicationHeader();

    skipper::BrandingOptionSpacesDeduplicator deduplicator(trx);
    for (Itin* itin : trx.itin())
    {
      removeDuplicatesFromTNBrandedItin(trx, *itin, diag892, deduplicator);
      // similar itins (we need general diag collector for that)
      DiagCollector& diag = *(factory->create(trx));
      similaritin::Context context(trx, *itin, diag);
      for (const SimilarItinData& similarItinData : context.motherItin.getSimilarItins())
        removeDuplicatesFromTNBrandedItin(trx, *(similarItinData.itin), diag892, deduplicator);
    }
  }

  if (diag892 != nullptr)
  {
    diag892->printFooter();
    diag892->flushMsg();
  }

  return result;
}

namespace
{
class PurgeAltDateInfo
    : public std::unary_function<std::pair<const DatePair, PricingTrx::AltDateInfo*>&, void>
{
public:
  void operator()(std::pair<const DatePair, PricingTrx::AltDateInfo*>& in) const
  {
    PricingTrx::AltDateInfo* info = in.second;
    info->taxItin.clear();
    info->taxCxr = "";
    info->totalPlusTax = 0.0;
    info->differentCxrCnt = 0;
    info->sameCxrCnt = 0;
    info->numOfSolutionNeeded = 1;
    info->calendarGroupNumber = 0;
    info->goodItinForDatePairFound = false;
    info->pendingSolution = false;
  }
};

struct RestoreSolutionCounter
    : public std::unary_function<std::pair<const DatePair, PricingTrx::AltDateInfo*>&, void>
{
  void operator()(std::pair<const DatePair, PricingTrx::AltDateInfo*>& in) const
  {
    PricingTrx::AltDateInfo* info = in.second;

    if (info->pendingSolution)
    {
      info->pendingSolution = false;
      info->numOfSolutionNeeded = 1;
    }
  }
};
}

bool
PricingOrchestrator::priceCheapestItin(PricingTrx& trx,
                                       PUPathMatrixVecTable& puPathMatrixVectTable,
                                       std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  const uint16_t brandedTableSize = getBrandSize(trx);
  bool ret = false;
  bool continueWithPQ = true;

  if (trx.getRequest()->owPricingRTTaxProcess() ||
      AirlineShoppingUtils::enableItinLevelThreading(trx))
  {
    for (uint16_t brandIndex = 0; brandIndex < brandedTableSize && continueWithPQ; brandIndex++)
    {
      std::for_each(trx.altDatePairs().begin(), trx.altDatePairs().end(), PurgeAltDateInfo());

      PUPathMatrixMap& puPathMatrixMap = puPathMatrixVectTable[brandIndex];
      TseRunnableExecutor itinASEExecutor(TseThreadingConst::PRICING_ITIN_ASE_TASK);

      checkFlexFaresForceCorpFares(trx, brandIndex);

      std::vector<AltDateItinThreadTask> tasks;
      try
      {
        tasks.reserve(puPathMatrixMap.size());
        for (auto& pUPathMatrix : puPathMatrixMap)
        {
          tasks.emplace_back(*this, trx, pUPathMatrix.second, puFactoryBucketVect);
          itinASEExecutor.execute(tasks.back());
        }

        itinASEExecutor.wait();
      }
      catch (Memory::OutOfMemoryException& ere)
      {
        continueWithPQ = false;
      }

      ret = std::any_of(tasks.cbegin(),
                        tasks.cend(),
                        [](const AltDateItinThreadTask& task)
                        { return task.itinPassed(); });
    }

    // --- process cat12 - surcharges ---
    validateSurchargeForAltDate(trx);
  }
  else
  {
    for (uint16_t brandIndex = 0; brandIndex < brandedTableSize && continueWithPQ; brandIndex++)
    {
      PUPathMatrixMap& puPathMatrixMap = puPathMatrixVectTable[brandIndex];

      checkFlexFaresForceCorpFares(trx, brandIndex);

      if (puPathMatrixMap.size() != 1)
        LOG4CXX_ERROR(_logger, "Only one date pair allowed");

      bool itinBrandRet = false;
      std::for_each(trx.altDatePairs().begin(), trx.altDatePairs().end(), PurgeAltDateInfo());
      try
      {
        itinBrandRet = priceItin(trx, (puPathMatrixMap.begin())->second, puFactoryBucketVect);
      }
      catch (Memory::OutOfMemoryException& ere)
      {
        continueWithPQ = false;
      }

      LOG4CXX_INFO(_logger,
                   "Price cheapest itin brand- " << brandIndex << " status-" << itinBrandRet);
      ret |= itinBrandRet;
    }
  }

  return returnOrThrow(trx, ret);
}

bool
PricingOrchestrator::returnOrThrow(PricingTrx& trx, TaskVector& tasks)
{
  bool itinPassed = false;
  bool allItinsFailedForOverrideCxr = trx.getRequest()->isGoverningCarrierOverrideEntry();
  bool isMultiTicket = trx.getRequest()->multiTicketActive() && !trx.multiTicketMap().empty();
  bool subItin1Passed = false;
  bool subItin2Passed = false;

  for (const auto& task : tasks)
  {
    const Itin& interestedItin = task.getItin();
    if (task.itinPassed())
    {
      if (!isMultiTicket)
      {
        itinPassed = true;
        break;
      }
      else
      {
        if (interestedItin.getMultiTktItinOrderNum() == 1)
          subItin1Passed = true;
        else if (interestedItin.getMultiTktItinOrderNum() == 2)
          subItin2Passed = true;
        if (interestedItin.getMultiTktItinOrderNum() == 0 || (subItin1Passed && subItin2Passed))
        {
          itinPassed = true; // if 'original itin' or 'both sub itins of multi ticket' passed
          break;
        }
      }
    }

    if (allItinsFailedForOverrideCxr && !itinHasAtLeastOneEmptyFMForOvrCxr(trx, interestedItin))
    {
      allItinsFailedForOverrideCxr = false;
    }
  }

  if (isMultiTicket)
  {
    MultiTicketUtil::isMultiTicketSolutionFound(trx); // Cleanup sub-itins if MT solution not found
  }

  if (!itinPassed && trx.getRequest()->isBrandedFaresRequest())
  {
    BrandCodeSet brands(trx.validBrands().begin(), trx.validBrands().end());
    const bool isAnyBrandReal = ShoppingUtil::isAnyBrandReal(brands);
    if (trx.getRequest()->isParityBrandsPath() && isAnyBrandReal)
    {
      // if parity brands enabled (regular IBF turned on) then we need to continue this transaction
      // and display soldout statuses in the response
      LOG4CXX_INFO(_logger, "Pricing Itin (all) failed - IBF/ContextShopping exception");
      itinPassed = true;
    }

    if (!fallback::fallbackSoldoutOriginRT(&trx))
    {
      if (trx.getRequest()->originBasedRTPricing() && isAnyBrandReal)
      {
        // if origin based RT pricing (regular IBF turned on) then we need to continue this transaction
        // and display soldout statuses in the response
        LOG4CXX_INFO(_logger, "Pricing Itin (all) failed - IBF/ContextShopping origin based RT exception");
        itinPassed = true;
      }
    }
    // without regular IBF we stop the transaction and display a simple error message
  }

  if (!itinPassed)
  {
    LOG4CXX_ERROR(_logger, "Pricing Itin (all) failed");
    DiagnosticTypes dType = trx.diagnostic().diagnosticType();

    if (dType == Diagnostic600 || dType == Diagnostic603 || dType == Diagnostic601 ||
        dType == Diagnostic609)
    {
      return true; // return true so that the TO does not report "Unknown Exception"
    }

    if (allItinsFailedForOverrideCxr)
      throw ErrorResponseException(ErrorResponseException::REQ_CARRIER_HAS_NO_FARES);

    if (!tasks.empty() && tasks.front().error().code() != ErrorResponseException::NO_ERROR)
    {
      throw tasks.front().error();
    }

    if (trx.getOptions()->forceCorpFares())
      throw ErrorResponseException(ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS);

    if (!fallback::fallbackFRRProcessingRetailerCode(&trx) && (trx.getRequest()->prmValue()) )
      throw ErrorResponseException(ErrorResponseException::NO_RETAILER_RULE_QUALIFIER_FARES_EXISTS);

    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  return itinPassed;
}

bool
PricingOrchestrator::returnOrThrow(PricingTrx& trx, bool itinPassed)
{
  if (!itinPassed)
  {
    LOG4CXX_ERROR(_logger, "Pricing Itin (all) failed");
    DiagnosticTypes dType = trx.diagnostic().diagnosticType();

    if (dType == Diagnostic600 || dType == Diagnostic603 || dType == Diagnostic601 ||
        dType == Diagnostic609)
    {
      return true; // return true so that the TO does not report "Unknown Exception"
    }

    if (trx.getOptions()->forceCorpFares())
      throw ErrorResponseException(ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS);

    if (!fallback::fallbackFRRProcessingRetailerCode(&trx) && (trx.getRequest()->prmValue()) )
      throw ErrorResponseException(ErrorResponseException::NO_RETAILER_RULE_QUALIFIER_FARES_EXISTS);

    throw ErrorResponseException(ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS);
  }

  return itinPassed;
}

void
PricingOrchestrator::buildFareMarketPathMatrix(PricingTrx& trx,
                                               Itin* itin,
                                               std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                               const BrandCode& brandCode)
{
  TSELatencyData metrics(trx, "PO FMKT PATH MATRIX");
  FmpMatrixPtr fmpMatrix(new FareMarketPathMatrix(trx, *itin, mergedFareMarketVect, brandCode));
  fmpMatrix->buildAllFareMarketPath();
  itin->fmpMatrix() = fmpMatrix;
}

bool
PricingOrchestrator::buildPUPathMatrix(PricingTrx& trx,
                                       std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
                                       PUPathMatrixMap& puPathMatrixMap,
                                       uint16_t brandIndex)
{
  BaseExchangeTrx* excTrx = nullptr;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    excTrx = static_cast<BaseExchangeTrx*>(&trx);
  }

  Diag993Collector* dc = nullptr;

  if (trx.diagnostic().diagnosticType() == Diagnostic993)
  {
    DCFactory* factory = DCFactory::instance();
    dc = dynamic_cast<Diag993Collector*>(factory->create(trx));
  }

  DatePair datePair(DateTime::emptyDate(), DateTime::emptyDate());
  DatePair outbDatePair(DateTime::emptyDate(), DateTime::emptyDate());
  DatePair inbDatePair(DateTime::emptyDate(), DateTime::emptyDate());
  bool outbAdded = false;
  bool inbAdded = false;
  std::map<DatePair, int> datePairASE;
  ItinVector::iterator itinIter = trx.itin().begin();
  flexFares::GroupId groupId = 0;

  if (trx.isFlexFare())
  {
    flexFares::GroupsData::const_iterator itr = trx.getRequest()->getFlexFaresGroupsData().begin();

    // FlexFares' GroupsData is a derived class of std::map which key is the group Id,
    // and brandIndex starts from 0 to the size of GroupsData, so in order to find the
    // corresponding groupId based on the brandIndex, it's necessary for the itr to move
    // brandIndex times so that the itr will return the right group Id
    std::advance(itr, brandIndex);

    groupId = itr->first;
  }

  for (uint16_t itinIndex = 0; itinIter != trx.itin().end(); ++itinIter, ++itinIndex)
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
      excTrx->setItinIndex(itinIndex);

    Itin* itin = *itinIter;
    BrandCode* brandCode = trx.validBrands().empty() ? nullptr : &(trx.validBrands()[brandIndex]);

    if (brandCode && (itin->brandCodes().find(*brandCode) == itin->brandCodes().end()))
    {
      // Not a valid brand for this itin
      puPathMatrixMap[datePair].push_back(nullptr);
      continue;
    }

    if (trx.isAltDates() && !ShoppingUtil::isBrandValid(trx, brandIndex, *itin->datePair()))
      continue;

    // calculating surcharges(cat-12) and yqyr for first N fares in each fare market
    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.isAltDates())
      precalculateTaxes(trx, *itin);

    if (TrxUtil::isBaggageInPQEnabled(trx) && !fallback::bagInPqLowerBound(&trx))
      calculateBaggageLowerBounds(trx, *itin);

    //----- Merged FareMarket of multiple Gov Cxr into one  -----
    std::vector<MergedFareMarket*> mergedFareMarketVect;
    {
      TSELatencyData metrics(trx, "PO FMKT MERGER");
      if (dc)
        dc->enable(Diagnostic993);

      FareMarketMerger fmMerger(trx,
                                *(*itinIter),
                                _factoriesConfig.searchAlwaysLowToHigh(),
                                nullptr,
                                brandIndex,
                                brandCode,
                                groupId,
                                dc);
      if (dc)
      {
        dc->printNewItin(*itin);
        dc->setFlexFaresGroupId(groupId);
        // since in Brands for TN Shopping this path is only executed for "Single brand" path
        // (cheapest option only decorated with brands at the end of processing)
        // we don't need to display brand index
        if (!trx.isBrandsForTnShopping() && !trx.activationFlags().isSearchForBrandsPricing())
          dc->writeBrandInfo(itinIndex, brandIndex);
      }

      fmMerger.buildAllMergedFareMarket(mergedFareMarketVect);
      LOG4CXX_INFO(_logger,
                   "Itin-" << itinIndex << " Brand-" << brandIndex
                           << " mergedFareMarketVect size = " << mergedFareMarketVect.size());
    }

    if (brandCode)
    {
      buildFareMarketPathMatrix(trx, *itinIter, mergedFareMarketVect, *brandCode);
    }
    else
    {
      buildFareMarketPathMatrix(trx, *itinIter, mergedFareMarketVect);
    }

    YQYRLBFactory::init(trx, **itinIter);

    if (RexBaseTrx::isRexTrxAndNewItin(trx))
      static_cast<RexBaseTrx&>(trx).getRec2Cat10WithVariousRetrieveDates(
          mergedFareMarketVect, &PricingOrchestrator::getRec2Cat10);
    else
      getRec2Cat10(trx, mergedFareMarketVect);

    getDatePair(
        trx, *itinIter, datePair, outbDatePair, inbDatePair, datePairASE, outbAdded, inbAdded);
    PUPathMatrix* puMatrix = nullptr;
    {
      TSELatencyData metrics(trx, "PO PU PATH MATRIX");
      trx.dataHandle().get(puMatrix);
      puMatrix->trx() = &trx;
      puMatrix->itin() = *itinIter;
      puMatrix->avoidStaticObjectPool() = _factoriesConfig.avoidStaticObjectPool();
      puMatrix->config() = &_config;
      puMatrix->brandIndex() = brandIndex;
      puMatrix->brandCode() = brandCode;
      if (trx.isFlexFare())
      {
        puMatrix->setFlexFaresGroupId(groupId);
      }
      puMatrix->buildAllPUPath(*(*itinIter)->fmpMatrix(), puFactoryBucketVect);
      puPathMatrixMap[datePair].push_back(puMatrix);

      if (brandCode)
        puMatrix->itin()->brandFmpMatrices()[*brandCode] = puMatrix->itin()->fmpMatrix();

      if (trx.isFlexFare())
        puMatrix->itin()->groupsFmpMatrices()[groupId] = puMatrix->itin()->fmpMatrix();
    }
  } // for all itins

  if (trx.delayXpn())
    startPUFTimers(puFactoryBucketVect);

  if (dc)
  {
    dc->flushMsg();
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    excTrx->setItinIndex(0);
  }

  return true;
}

bool
PricingOrchestrator::buildPUPathMatrixForItin(
    PricingTrx& trx,
    Itin& itin,
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
    PUPathMatrixMap& puPathMatrixMap,
    uint16_t poptSpaceIndex)
{
  Diag993Collector* dc = nullptr;

  if (trx.diagnostic().diagnosticType() == Diagnostic993)
  {
    DCFactory* factory = DCFactory::instance();
    dc = dynamic_cast<Diag993Collector*>(factory->create(trx));
    if (dc)
      dc->enable(Diagnostic993);
  }

  const DatePair datePair(DateTime::emptyDate(), DateTime::emptyDate());

  //----- Merged FareMarket of multiple Gov Cxr into one  -----
  std::vector<MergedFareMarket*> mergedFareMarketVect;
  {
    TSELatencyData metrics(trx, "PO FMKT MERGER");

    FareMarketMerger fmMerger(trx,
                              itin,
                              _factoriesConfig.searchAlwaysLowToHigh(),
                              nullptr,
                              poptSpaceIndex,
                              nullptr,
                              0,
                              dc);
    fmMerger.buildAllMergedFareMarket(mergedFareMarketVect);
    LOG4CXX_INFO(_logger,
                 "Itin-" << itin.itinNum() << " Brand BrandingOptionSpace-" << poptSpaceIndex
                         << " mergedFareMarketVect size = " << mergedFareMarketVect.size());
    if (dc)
    {
      dc->printNewItin(itin);
      const skipper::BrandingOptionSpace& brandOptionSpace =
          itin.getItinBranding().getBrandingOptionSpace(poptSpaceIndex);

      std::string brandsFromSpace =
          itin.getItinBranding().brandsFromSpaceToString(brandOptionSpace);
      dc->writeBrandCombinationInfo(brandsFromSpace);
      dc->printMergedFareMarketVect(
          mergedFareMarketVect, 0, brandOptionSpace, itin.getItinBranding());
    }
  }

  buildFareMarketPathMatrix(trx, &itin, mergedFareMarketVect);

  YQYRLBFactory::init(trx, itin);

  getRec2Cat10(trx, mergedFareMarketVect);

  PUPathMatrix* puMatrix = nullptr;
  {
    TSELatencyData metrics(trx, "PO PU PATH MATRIX");
    trx.dataHandle().get(puMatrix);
    puMatrix->trx() = &trx;
    puMatrix->itin() = &itin;
    puMatrix->avoidStaticObjectPool() = _factoriesConfig.avoidStaticObjectPool();
    puMatrix->config() = &_config;
    puMatrix->brandIndex() = poptSpaceIndex;
    puMatrix->buildAllPUPath(*itin.fmpMatrix(), puFactoryBucketVect);
    puPathMatrixMap[datePair].push_back(puMatrix);

    // itin == *puMatrix->itin(), see above
    itin.getItinBranding().setFmpMatrix(itin.fmpMatrix(), poptSpaceIndex);
  }

  if (trx.delayXpn())
    startPUFTimers(puFactoryBucketVect);

  if (dc)
  {
    dc->flushMsg();
  }

  return true;
}

void
PricingOrchestrator::startPUFTimers(std::vector<PricingUnitFactoryBucket*>& pufbv)
{
  for (PricingUnitFactoryBucket* pufb : pufbv)
  {
    std::map<PU*, PricingUnitFactory*>::iterator i = pufb->puFactoryBucket().begin();
    std::map<PU*, PricingUnitFactory*>::iterator iEnd = pufb->puFactoryBucket().end();

    for (; i != iEnd; ++i)
      i->second->startTiming();
  }
}

void
PricingOrchestrator::displayDiagnostics606(DiagnosticTypes& diagType,
                                           DCFactory* factory,
                                           DiagCollector& diag)
{
  if (diagType == Diagnostic606)
  {
    boost::lock_guard<boost::mutex> g(Diag606Collector::fcSBdisplayMutex());

    if (!Diag606Collector::fcSBdisplayDone())
    {
      diag.enable(Diagnostic606);
      diag << "            FARE CLASS NOT FOUND" << std::endl;
      diag.printLine();
    }

    Diag606Collector::fcSBdisplayDone() = false;
  }

  diag.flushMsg();
}

bool
PricingOrchestrator::displayDiagnostics(PricingTrx& trx,
                                        DiagnosticTypes& diagType,
                                        std::vector<PricingUnitFactoryBucket*>& pufbv)
{
  if (diagType == Diagnostic600)
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic600);

    if (diag.isActive())
    {
      diag << "\n TOTAL UNIQUE PU COUNT: " << pufbv.front()->puFactoryBucket().size() << "\n";
      diag.flushMsg();
    }

    if (trx.excTrxType() != PricingTrx::AR_EXC_TRX ||
        (static_cast<RexPricingTrx*>(&trx))->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
      return true;
  }
  else if (diagType == Diagnostic527)
  {
    DCFactory* factory = DCFactory::instance();
    Diag527Collector* dc = dynamic_cast<Diag527Collector*>(factory->create(trx));

    if (dc)
    {
      dc->enable(diagType);
      dc->displayHeader();
      dc->flushMsg();
    }
  }

  LOG4CXX_INFO(_logger, "Num of PaxType, puFactoryBucketVect size=" << pufbv.size());
  return false;
}

void
PricingOrchestrator::displayPriceItinDiags(
    PricingTrx& trx, DiagCollector& diag, bool first, PUPathMatrix* puMatrix, bool ret)
{
  if (first)
  {
    diag.enable(puMatrix,
                Diagnostic609,
                Diagnostic690,
                Diagnostic610,
                Diagnostic614,
                Diagnostic620,
                Diagnostic625,
                Diagnostic631,
                Diagnostic632,
                Diagnostic633,
                Diagnostic634,
                Diagnostic636,
                Diagnostic637,
                Diagnostic638,
                Diagnostic639,
                Diagnostic640,
                Diagnostic653,
                Diagnostic654,
                Diagnostic660,
                Diagnostic682,
                Diagnostic699,
                Diagnostic990);
    diag.printHeader();
    diag.flushMsg();
  }
  else
  {
    diag.enable(puMatrix,
                Diagnostic609,
                Diagnostic690,
                Diagnostic610,
                Diagnostic614,
                Diagnostic620,
                Diagnostic631,
                Diagnostic632,
                Diagnostic633,
                Diagnostic634,
                Diagnostic636,
                Diagnostic637,
                Diagnostic638,
                Diagnostic639,
                Diagnostic640,
                Diagnostic653,
                Diagnostic654,
                Diagnostic660,
                Diagnostic682,
                Diagnostic990);

    if (!ret)
      diag << "               NO COMBINABLE VALID FARE FOUND " << std::endl;

    diag.printLine();
    diag.enable(Diagnostic661);
    diag.printLine();
    diag.flushMsg();
  }
}

void
PricingOrchestrator::getFMCOSBasedOnAvailBreak(PricingTrx& trx, Itin* itin)
{
  if (itin)
  {
    for (FareMarket* fareMarket : itin->fareMarket())
    {
      fareMarket->classOfServiceVec().clear();
      ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, itin, fareMarket, false);
    }
    for (FareMarket* fareMarket : itin->flowFareMarket())
    {
      fareMarket->classOfServiceVec().clear();
      ShoppingUtil::getFMCOSBasedOnAvailBreak(trx, itin, fareMarket, false);
    }
  }
}

void
PricingOrchestrator::getDatePair(PricingTrx& trx,
                                 Itin* itin,
                                 DatePair& datePair,
                                 DatePair& outbDatePair,
                                 DatePair& inbDatePair,
                                 std::map<DatePair, int>& datePairASE,
                                 bool& outbAdded,
                                 bool& inbAdded)
{
  if ((trx.getRequest()->owPricingRTTaxProcess() && trx.isAltDates()) ||
      AirlineShoppingUtils::enableItinLevelThreading(trx))
  {
    datePair = *itin->datePair();
  }

  // split cheapest AD if the same date on outbound and inbound
  if (trx.isAltDates() && trx.getRequest()->owPricingRTTaxProcess())
  {
    if (datePairASE.count(datePair) > 0 && datePairASE[datePair] != itin->legID().front().first)
      datePair = DatePair(DateTime::emptyDate(), DateTime::emptyDate());
    else if (datePairASE.count(datePair) == 0)
      datePairASE[datePair] = itin->legID().front().first;
  }
}

bool
PricingOrchestrator::priceItin(PricingTrx& trx,
                               PUPathMatrix& puMatrix,
                               std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  std::vector<PUPathMatrix*> tmpVect;
  tmpVect.push_back(&puMatrix);
  return priceItin(trx, tmpVect, puFactoryBucketVect);
}

bool
PricingOrchestrator::initGroupFarePathFactory(PricingTrx& trx,
                                              GroupFarePathFactory& groupFarePathFactory,
                                              std::vector<PUPathMatrix*>& puMatrixVect,
                                              std::vector<PaxFarePathFactory*>& pfpfBucket,
                                              std::vector<PricingUnitFactoryBucket*>& pufbv,
                                              DiagCollector& diag)
{
  //----- Create PaxFarePathFactory, one for each PaxType ----------
  if (!createPaxFarePathFactory(trx, puMatrixVect, pufbv, pfpfBucket, PaxFarePathFactoryCreator()))
  {
    return false;
  }

  //--------- Init FarePathFactory -- Multi-Threaded ------------
  //
  if (!initPaxFarePathFactory(trx, pfpfBucket, diag))
    return false;

  //----------------- Change PU ValidataionScope ----------------
  // After init of FarePath PQ is done, we want to validate PU in
  // PricingUnitFactory and use multi-threaded power
  //

  if (!trx.delayXpn())
    completePUFactoryInit(pufbv);

  //
  //----------------- Init GroupFarePathFactory ----------------
  groupFarePathFactory.setPaxFarePathFactoryBucket(pfpfBucket);
  AccompaniedTravel* accompaniedTravel = nullptr;
  trx.dataHandle().get(accompaniedTravel);
  groupFarePathFactory.accompaniedTravel() = accompaniedTravel;
  groupFarePathFactory.multiPaxShortCktTimeOut() = _factoriesConfig.multiPaxShortCktTimeOut();
  groupFarePathFactory.numTaxCallForMipAltDate() = _numTaxCallForMipAltDate;
  return true;
}

void
PricingOrchestrator::displayBrandCombinationInfo(PricingTrx& trx,
                                                 DiagCollector& diag,
                                                 const Itin& itin)
{
  if (diag.isActive() && diag.diagnosticType() == Diagnostic699)
  {
    if (trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
    {
      const skipper::ItinBranding& itinBranding = itin.getItinBranding();
      const size_t brandingSpaceIndex = itinBranding.getBrandingSpaceIndex(itin.fmpMatrix());
      const skipper::BrandingOptionSpace& brandingSpace =
          itinBranding.getBrandingOptionSpace(brandingSpaceIndex);
      std::string brandsFromSpace = itinBranding.brandsFromSpaceToString(brandingSpace);
      diag << " BRAND COMBINATION: " << brandsFromSpace << std::endl;
      diag.flushMsg();
    }
  }
}

bool
PricingOrchestrator::priceItin(PricingTrx& trx,
                               std::vector<PUPathMatrix*>& puMatrixVect,
                               std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  displayPriceItinDiags(trx, diag, true, puMatrixVect.front());
  displayBrandCombinationInfo(trx, diag, *puMatrixVect.front()->itin());

  //----------------- Create GroupFarePathFactory/start Timer ----------------
  //
  GroupFarePathFactory groupFarePathFactory(trx);
  groupFarePathFactory.startTimer();

  throughAvailabilitySetup(trx, puMatrixVect);

  //----- Create PaxFarePathFactory, one for each PaxType ----------
  //
  std::vector<PaxFarePathFactory*> paxFarePathFactoryBucket;
  if (!initGroupFarePathFactory(trx,
                                groupFarePathFactory,
                                puMatrixVect,
                                paxFarePathFactoryBucket,
                                puFactoryBucketVect,
                                diag))
  {
    groupFarePathFactory.clearFactories();
    return false;
  }

  bool ret = initFpFactory(groupFarePathFactory);

  //
  //----------------- get GroupFarePath ----------------
  //
  if (ret)
    ret = getGroupFarePath(trx, groupFarePathFactory, diag);

  if (!ret)
  {
    similaritin::Context context(trx, *(puMatrixVect.front()->itin()), diag);
    priceSimilarItinsWithSavedFPPQItems(context, groupFarePathFactory);
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.isAltDates() &&
      (!(trx.getRequest()->owPricingRTTaxProcess() ||
         AirlineShoppingUtils::enableItinLevelThreading(trx))))
  {
    // --- process cat12 - surcharges ---
    validateSurchargeForAltDate(trx);
  }

  // WP-PARTIAL-MATCH
  // For WP no match (ret == false), we should do this extra step similar
  // to WPA to catch the partial match.
  if (!ret && !trx.getRequest()->turnOffNoMatch() && trx.altTrxType() == PricingTrx::WP &&
      trx.fareCalcConfig()->wpNoMatchPermitted() == YES && puMatrixVect.size() == 1)
  {
    AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(&trx);

    if (altTrx)
    {
      FarePathCollector fpCollector;
      ret = fpCollector.process(paxFarePathFactoryBucket, *altTrx, *puMatrixVect.front(), diag);
    }
  }

  displayPriceItinDiags(trx, diag, false, puMatrixVect.front(), ret);
  groupFarePathFactory.clearFactories();
  return ret;
}

void
PricingOrchestrator::priceSimilarItinsWithSavedFPPQItems(similaritin::Context& context,
                                                         GroupFarePathFactory& gfpf)
{
  PricingTrx& trx = context.trx;

  if (context.motherItin.getSimilarItins().empty())
    return;

  similaritin::FarePathValidator validator(trx, context.motherItin);

  DiagManager manager(trx, Diagnostic991);
  if (UNLIKELY(manager.isActive()))
  {
    similaritin::DiagnosticWrapper diagWrapper(trx, manager.collector());
    similaritin::PriceWithSavedFPPQItems<similaritin::DiagnosticWrapper>(
        context, diagWrapper, validator).priceAll(gfpf);
  }
  else
  {
    similaritin::NoDiagnostic noDiagnostic;
    similaritin::PriceWithSavedFPPQItems<similaritin::NoDiagnostic>(
        context, noDiagnostic, validator).priceAll(gfpf);
  }
}

namespace
{
inline uint32_t
getSolutionCount(PricingTrx& trx)
{
  if (trx.isAltDates() && (!trx.altDatePairs().empty()))
  {
    if (trx.getRequest()->owPricingRTTaxProcess() ||
        AirlineShoppingUtils::enableItinLevelThreading(trx))
      return 1; // MT activation change

    return static_cast<uint32_t>(trx.altDatePairs().size());
  }
  else if (RexPricingTrx::isRexTrxAndNewItin(trx))
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX &&
        (trx.billing()->actionCode() == "WFR" || trx.billing()->actionCode() == "WFR.C"))
      return 1;
    return 2;
  }
  return 1;
}
}

bool
PricingOrchestrator::getGroupFarePath(PricingTrx& trx,
                                      GroupFarePathFactory& groupFarePathFactory,
                                      DiagCollector& diag)
{
  bool ret = false;
  uint32_t solutionCount = getSolutionCount(trx);

  for (uint16_t i = 0; i < solutionCount; ++i)
  {
    GroupFarePath* groupFPath = groupFarePathFactory.getGroupFarePath(diag);
    if (groupFPath == nullptr || groupFPath->groupFPPQItem().empty())
      return ret;

    ret = true;

    for (FPPQItem* fppqItem : groupFPath->groupFPPQItem())
    {
      FarePath& farePath = *fppqItem->farePath();
      MaximumPenaltyValidator(trx).completeResponse(farePath);
      if (trx.getRequest()->isSFR())
        structuredFareRulesUtils::finalizeDataCollection(farePath);
    }

    FarePath& farePath = *groupFPath->groupFPPQItem().front()->farePath();
    Itin* itin = farePath.itin();

    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.isAltDates() &&
        trx.getOptions()->inhibitSplitPNR())
    {
      PricingTrx::AltDateInfo& adInfo = *trx.altDatePairs()[*itin->datePair()];
      adInfo.pendingSolution = false;
      std::for_each(trx.altDatePairs().begin(), trx.altDatePairs().end(), RestoreSolutionCounter());
    }

    const bool cutOff = ShoppingAltDateUtil::cutOffAltDate(
        trx, &farePath, getAltDateItinPriceJumpFactor(), getAltDateCutOffNucThreshold());

    std::vector<FPPQItem*> groupFPPQItem;

    if (!itin->getSimilarItins().empty())
    {
      for (const FPPQItem* item : groupFPath->groupFPPQItem())
      {
        groupFPPQItem.push_back(item->createDuplicate(trx));
      }
    }

    if (!(trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
          static_cast<const RexPricingTrx&>(trx).trxPhase() ==
              RexPricingTrx::REPRICE_EXCITIN_PHASE &&
          !static_cast<const RexPricingTrx&>(trx).getRexOptions().isNetSellingIndicator()))
      ret = processFinalGroupFarePath(trx, *groupFPath, groupFarePathFactory, diag);

    if (itin->errResponseCode() != ErrorResponseException::INVALID_NUMBER_FOR_ALT_DATES)
    {
      AirlineShoppingUtils::setSolutionBrandInfo(trx, groupFarePathFactory, *groupFPath);
      copyFarePathToItin(trx, *groupFPath, *itin);
    }
    else
    {
      itin->errResponseCode() = ErrorResponseException::NO_ERROR;
      ++solutionCount;
    }

    if (trx.getRequest()->ticketingAgent()->axessUser() && trx.getRequest()->isWpNettRequested())
    {
      for (FPPQItem* fppqItem : groupFPath->groupFPPQItem())
      {
        if (fppqItem->farePath()->axessFarePath() != nullptr)
        {
          Itin* axessItin = fppqItem->farePath()->axessFarePath()->itin();
          copyNetRemitFarePathToNetRemitItin(*groupFPath, *axessItin);
        }
      }
    }

    if (!trx.getRequest()->isSFR())
    {
      for (FarePath* farePath : itin->farePath())
        ShoppingUtil::updateFinalBookingBasedOnAvailBreaks(trx, *farePath, *itin);
    }

    similaritin::Context context(trx, *itin, diag);
    priceSimilarItins(context, groupFarePathFactory, groupFPPQItem);

    if (cutOff)
    {
      // too expensive, no need to look for other dates
      LOG4CXX_INFO(_logger, "Too expensive")
      break;
    }

    if (trx.isAltDates() && (i == solutionCount - 1) && !trx.altDatePairs().empty())
    {
      if (trx.getRequest()->owPricingRTTaxProcess() ||
          AirlineShoppingUtils::enableItinLevelThreading(trx))
      {
        AltDateCellCompare altDateCellCompare(*(itin->datePair()));
        const PricingTrx::AltDatePairs::iterator itEnd = trx.altDatePairs().end();
        PricingTrx::AltDatePairs::iterator it =
            std::find_if(trx.altDatePairs().begin(), itEnd, altDateCellCompare);

        if (it != itEnd)
          solutionCount += static_cast<uint32_t>(it->second->numOfSolutionNeeded);
      }
      else
      {
        const PricingTrx::AltDatePairs::const_iterator itEnd = trx.altDatePairs().end();
        PricingTrx::AltDatePairs::const_iterator it = trx.altDatePairs().begin();

        for (; it != itEnd; ++it)
        {
          solutionCount += static_cast<uint32_t>(it->second->numOfSolutionNeeded);
        }
      }
    }
  }

  if (trx.getTnShoppingBrandingMode() == TnShoppingBrandingMode::SINGLE_BRAND)
    lazyBrandAllPaxFares(trx, diag);

  return ret;
}

void
PricingOrchestrator::lazyBrandAllPaxFaresInItin(Itin& itin, std::set<PaxTypeFare*>& faresToBrand)
{
  for (FarePath* farePath : itin.farePath())
  {
    // Gather all PaxFares from this fare path
    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        faresToBrand.insert(fareUsage->paxTypeFare());
      }
    }
    // The same for the possible cloned fare paths
    for (FarePath* clonedFp : farePath->gsaClonedFarePaths())
    {
      for (PricingUnit* pricingUnit : clonedFp->pricingUnit())
      {
        for (FareUsage* fareUsage : pricingUnit->fareUsage())
        {
          faresToBrand.insert(fareUsage->paxTypeFare());
        }
      }
    }
  }
}

void
PricingOrchestrator::lazyBrandAllPaxFares(PricingTrx& trx, DiagCollector& diag)
{
  std::set<PaxTypeFare*> faresToBrand;
  for (Itin* itin : trx.itin())
  {
    TSE_ASSERT(itin != nullptr);
    lazyBrandAllPaxFaresInItin(*itin, faresToBrand);

    // Cover itins priced by family logic
    similaritin::Context context(trx, *itin, diag);
    Itin& motherItin = context.motherItin;
    for (const SimilarItinData& similarItinData : motherItin.getSimilarItins())
    {
      TSE_ASSERT(similarItinData.itin);
      lazyBrandAllPaxFaresInItin(*similarItinData.itin, faresToBrand);
    }
  }

  BrandedFareValidatorFactory brandSelectorFactory(trx);
  BrandedFaresSelector brandedFaresSelector(trx, brandSelectorFactory);
  brandedFaresSelector.brandFares(faresToBrand);
}

bool
PricingOrchestrator::hasHardPassFareOnEachLeg(const PricingTrx& trx, const FPPQItem& fppQItem) const
{
  std::map<size_t, bool> areLegsHardPassed;

  const skipper::ItinBranding& itinBranding = fppQItem.puPath()->itin()->getItinBranding();
  const size_t brandingSpaceIndex =
      itinBranding.getBrandingSpaceIndex(fppQItem.puPath()->itin()->fmpMatrix());
  const skipper::BrandingOptionSpace& brandingSpace =
      itinBranding.getBrandingOptionSpace(brandingSpaceIndex);

  for (PricingUnit* pricingUnit : fppQItem.farePath()->pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      TSE_ASSERT(fareUsage->paxTypeFare()->fareMarket() != nullptr);

      std::pair<size_t, size_t> startEndSegments =
          itinBranding.getFareMarketStartEndSegments(*(fareUsage->paxTypeFare()->fareMarket()));

      for (size_t segment = startEndSegments.first; segment <= startEndSegments.second; ++segment)
      {
        size_t currentLeg = itinBranding.getTravelSegmentLegId(segment);
        // if not set it will be set to false, map [] operator
        if (areLegsHardPassed[currentLeg])
          continue;

        skipper::CarrierBrandPairs::const_iterator it = brandingSpace[segment].begin();
        for (; it != brandingSpace[segment].end(); ++it)
        {
          if (fareUsage->paxTypeFare()->isValidForBrand(trx, &(it->second), true))
          {
            areLegsHardPassed[currentLeg] = true;
            // no point checking other carrier/brand pairs on this segment
            break;
          }
        }
      }
    }
  }
  return std::all_of(areLegsHardPassed.begin(),
                     areLegsHardPassed.end(),
                     [](typename std::map<size_t, bool>::const_reference p)
                     { return p.second; });
}

bool
PricingOrchestrator::processFinalGroupFarePath(PricingTrx& trx,
                                               GroupFarePath& groupFPath,
                                               GroupFarePathFactory& groupFarePathFactory,
                                               DiagCollector& diag)
{
  std::vector<FPPQItem*>::iterator it = groupFPath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::iterator itEnd = groupFPath.groupFPPQItem().end();
  uint32_t posPax = 0;
  bool ret = true;

  for (; it != itEnd; ++it, ++posPax)
  {
    FPPQItem& fppqItem = **it;

    if (trx.isBRAll() && !trx.isSoftPassDisabled())
    {
      if (!hasHardPassFareOnEachLeg(trx, fppqItem))
      {
        continue;
      }
    }

    ret &= PricingUtil::finalPricingProcess(trx, *fppqItem.farePath());

    if (ret && trx.getRequest()->ticketingAgent()->axessUser() &&
        trx.getRequest()->isWpNettRequested())
    {
      if (fppqItem.farePath()->selectedNetRemitFareCombo())
      {
        preparePricingProcessForAxess(fppqItem, groupFarePathFactory, posPax, diag);
        FPPQItem* fppqItemAxess = nullptr;
        const std::vector<PaxFarePathFactoryBase*>& gfpf =
            groupFarePathFactory.getPaxFarePathFactoryBucket();
        PaxFarePathFactoryBase* pfpf = gfpf[posPax];
        fppqItemAxess = pfpf->getFPPQItem(0, diag);

        if (fppqItemAxess != nullptr)
        {
          bool rc = PricingUtil::finalFPathCreationForAxess(
              trx, *(fppqItem.farePath()), *(fppqItemAxess->farePath()));

          if (!rc)
          {
            LOG4CXX_INFO(_logger,
                         "FINAL AXESS FAREPATH WAS CREATED, PAXTYPE: "
                             << fppqItem.farePath()->paxType()->paxType());
          }
        }
        else
        {
          if (fppqItem.farePath()->axessFarePath() == nullptr &&
              fppqItem.farePath()->collectedNegFareData() != nullptr)
          {
            fppqItem.farePath()->collectedNegFareData()->trailerMsg() =
                "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
          }
        }
      }
      else
      {
        fppqItem.farePath()->collectedNegFareData()->trailerMsg() =
            "INVALID NET REMIT FARE - UNABLE TO AUTO TICKET";
      }
    }
  }

  return ret;
}

void
PricingOrchestrator::copyFarePathToItin(PricingTrx& trx, GroupFarePath& groupFPath, Itin& itin)
{
  std::vector<FPPQItem*>::iterator it = groupFPath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::iterator itEnd = groupFPath.groupFPPQItem().end();

  for (; it != itEnd; ++it)
  {
    itin.farePath().push_back((*it)->farePath());
    LOG4CXX_INFO(_logger,
                 "ITIN#(" << ((long)&itin) << ") FINAL FAREPATH PUSHED TO ITIN, PAXTYPE: "
                          << ((*it)->farePath())->paxType()->paxType()
                          << " NUM SOLUTION: " << itin.farePath().size());
  }
}

void
PricingOrchestrator::copyNetRemitFarePathToNetRemitItin(GroupFarePath& groupFPath, Itin& itin)
{
  std::vector<FPPQItem*>::iterator it = groupFPath.groupFPPQItem().begin();
  std::vector<FPPQItem*>::iterator itEnd = groupFPath.groupFPPQItem().end();

  for (; it != itEnd; ++it)
  {
    FarePath* axessFP = (*it)->farePath()->axessFarePath();

    if (!axessFP)
      continue;

    itin.farePath().push_back(axessFP);
  }
}

bool
PricingOrchestrator::priceItin(AltPricingTrx& trx,
                               PUPathMatrix& puMatrix,
                               std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  NoPNRPricingTrx* noPNRPricingTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  diag.enable(&puMatrix,
              Diagnostic609,
              Diagnostic690,
              Diagnostic610,
              Diagnostic611,
              Diagnostic614,
              Diagnostic620,
              Diagnostic625,
              Diagnostic631,
              Diagnostic632,
              Diagnostic633,
              Diagnostic634,
              Diagnostic636,
              Diagnostic637,
              Diagnostic638,
              Diagnostic639,
              Diagnostic640,
              Diagnostic653,
              Diagnostic654,
              Diagnostic660,
              Diagnostic682);

  if (!noPNRPricingTrx)
  {
    diag.printHeader();
    diag.flushMsg();
  }

  //----- Create PaxFarePathFactory, one for each PaxType ----------
  //
  std::vector<PaxFarePathFactory*> paxFarePathFactoryBucket;

  if (!createPaxFarePathFactory(trx,
                                puMatrix,
                                puFactoryBucketVect,
                                paxFarePathFactoryBucket,
                                PaxFarePathFactoryCreator()))
  {
    return false;
  }

  bool allowDuplicateTotals = false;
  uint32_t farePathCount = 0;
  uint32_t fareOptionMaxDefault = AltPricingUtil::getConfigFareOptionMaxDefault();
  uint32_t fareOptionMaxLimit = AltPricingUtil::getConfigFareOptionMaxLimit();

  if (noPNRPricingTrx)
  {
    const NoPNROptions* noPNROptions = noPNRPricingTrx->noPNROptions();
    allowDuplicateTotals = noPNROptions->wqDuplicateAmounts() == YES ? true : false;
    NoPNRPricingTrx::Solutions& solutions = noPNRPricingTrx->solutions();
    std::vector<PaxFarePathFactory*>::iterator iter = paxFarePathFactoryBucket.begin();

    while (iter != paxFarePathFactoryBucket.end())
    {
      PaxFarePathFactory* pfpf = *iter;
      PaxType* paxType = pfpf->paxType();
      pfpf->allowDuplicateTotals() = allowDuplicateTotals;
      int limit = solutions.limit(paxType);

      if (limit == 0 || !solutions.process(paxType))
        iter = paxFarePathFactoryBucket.erase(iter);
      else
      {
        pfpf->reqValidFPCount() = static_cast<uint32_t>(limit);
        ++iter;
      }
    }
  }
  else
  {
    if (trx.getRequest()->ticketingAgent()->sabre1SUser() && trx.getRequest()->isWpa50())
    {
      farePathCount = static_cast<uint32_t>(AltPricingTrx::WPA_50_OPTIONS);
    }

    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(trx);

    if (fcConfig)
    {
      allowDuplicateTotals = fcConfig->wpaShowDupAmounts() == YES ? true : false;

      if (!farePathCount)
      {
        farePathCount = static_cast<uint32_t>(fcConfig->wpaFareOptionMaxNo());
      }

      LOG4CXX_DEBUG(_logger,
                    "\n********** WPA ALTERNATIVE PRICING **********"
                        << "\nFARE CALC CONFIG FOR PCC: "
                        << trx.getRequest()->ticketingAgent()->tvlAgencyPCC()
                        << "\nFARE CALC CONFIG FARE OPTION MAX: " << farePathCount
                        << "\nFARE CALC CONFIG ALLOW DUP TOTALS: "
                        << (allowDuplicateTotals ? "YES" : "NO"));
    }
    else
    {
      LOG4CXX_WARN(_logger,
                   "Error getting Fare Calc Config for PCC: "
                       << trx.getRequest()->ticketingAgent()->tvlAgencyPCC()
                       << " Using default values.");
    }

    if (farePathCount <= 0)
    {
      farePathCount = fareOptionMaxDefault;
    }
    else if (farePathCount > fareOptionMaxLimit)
    {
      farePathCount = fareOptionMaxLimit;
    }

    if (trx.altTrxType() == AltPricingTrx::WP || trx.getRequest()->turnOffNoMatch())
    {
      farePathCount = 1;
      allowDuplicateTotals = true; // Avoid the extra work handling the dups.
    }

    if (trx.isRfbListOfSolutionEnabled())
    {
      farePathCount = AltPricingUtil::getConfigFareOptionMaxDefault();
      allowDuplicateTotals = true;
    }

    LOG4CXX_DEBUG(_logger,
                  "\n********** WPA ALTERNATIVE PRICING **********"
                      << "\nCONFIG FARE OPTION MAX DEFAULT: " << fareOptionMaxDefault
                      << "\nCONFIG FARE OPTION MAX LIMIT: " << fareOptionMaxLimit
                      << "\nUSING ALLOW DUP TOTALS: " << (allowDuplicateTotals ? "YES" : "NO")
                      << "\nUSING FP COUNT: " << farePathCount);

    if (!allowDuplicateTotals && trx.altTrxType() != AltPricingTrx::WP)
    {
      farePathCount += _additionalFarePathCount;
    }

    std::vector<PaxFarePathFactory*>::iterator iter = paxFarePathFactoryBucket.begin();
    std::vector<PaxFarePathFactory*>::iterator iterEnd = paxFarePathFactoryBucket.end();

    for (; iter != iterEnd; ++iter)
    {
      PaxFarePathFactory* pfpf = *iter;
      pfpf->allowDuplicateTotals() = allowDuplicateTotals;
      pfpf->reqValidFPCount() = farePathCount;
    }
  }

  //
  //--------- Init FarePathFactory -- Multi-Threaded ------------
  //
  bool initSuccess = initPaxFarePathFactory(trx, paxFarePathFactoryBucket, diag);

  if (!initSuccess)
    return false;

  //----------------- Change PU ValidataionScope ----------------
  // After init of FarePath PQ is done, we want to validate PU in
  // PricingUnitFactory and use multi-threaded power
  //
  completePUFactoryInit(puFactoryBucketVect);
  FarePathCollector fpCollector;
  bool ret = fpCollector.process(paxFarePathFactoryBucket, trx, puMatrix, diag);
  diag.enable(&puMatrix,
              Diagnostic609,
              Diagnostic690,
              Diagnostic610,
              Diagnostic611,
              Diagnostic614,
              Diagnostic620,
              Diagnostic631,
              Diagnostic632,
              Diagnostic633,
              Diagnostic634,
              Diagnostic636,
              Diagnostic637,
              Diagnostic638,
              Diagnostic639,
              Diagnostic640,
              Diagnostic653,
              Diagnostic654,
              Diagnostic660,
              Diagnostic682);

  if (!noPNRPricingTrx)
  {
    if (!ret)
      diag << "               NO COMBINABLE VALID FARE FOUND " << std::endl;
  }

  diag.printLine();
  diag.enable(Diagnostic661);
  diag.printLine();
  diag.flushMsg();
  return ret;
}

bool
PricingOrchestrator::createPricingUnitFactoryBucket(
    PricingTrx& trx, std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)

{
  if (trx.paxType().empty())
  {
    LOG4CXX_ERROR(_logger, "In PricingTrx, paxType vector is empty");
    return false;
  }

  CombinabilityScoreboard* comboScoreboard = nullptr;
  trx.dataHandle().get(comboScoreboard);
  // lint --e{413}
  comboScoreboard->trx() = &trx;
  Combinations* combinations = nullptr;
  trx.dataHandle().get(combinations);
  // lint -e{413}
  combinations->trx() = &trx;
  combinations->comboScoreboard() = comboScoreboard;
  //---------------------------------------------------
  // One bucket per PaxType
  // Instantiate all the buckets of PricingUnitFactory
  //
  std::vector<PaxType*>::iterator paxIter = trx.paxType().begin();

  for (; paxIter != trx.paxType().end(); ++paxIter)
  {
    // lint --e{413}
    PricingUnitFactory::JourneyPULowerBound* journeyPULowerBound = nullptr;
    trx.dataHandle().get(journeyPULowerBound);
    PricingUnitFactoryBucket* puFactoryBucket = nullptr;
    trx.dataHandle().get(puFactoryBucket);
    puFactoryBucket->paxType() = *paxIter;
    puFactoryBucket->combinations() = combinations;
    puFactoryBucket->journeyPULowerBound() = journeyPULowerBound;
    puFactoryBucketVect.push_back(puFactoryBucket);
  }

  return true;
}

bool
PricingOrchestrator::initPricingUnitFactory(
    PricingTrx& trx,
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
    DiagCollector& diag,
    bool onlyOwFares,
    bool onlyNonStops)
{
  DataHandle dataHandle(trx.ticketingDate());
  TseRunnableExecutor taskExecutor(TseThreadingConst::PRICINGPQ_TASK);
  std::vector<PUFInitThreadInput*> thrInputVect;
  LOG4CXX_INFO(
      _logger,
      "Number of UNIQUE PU Template = " << puFactoryBucketVect.front()->puFactoryBucket().size());
  uint16_t thrCount = 0;
  std::vector<PricingUnitFactoryBucket*>::iterator buktIt = puFactoryBucketVect.begin();
  const std::vector<PricingUnitFactoryBucket*>::iterator buktItEnd = puFactoryBucketVect.end();

  for (; buktIt != buktItEnd; ++buktIt)
  {
    std::map<PU*, PricingUnitFactory*>::iterator factIt = (*buktIt)->puFactoryBucket().begin();
    const std::map<PU*, PricingUnitFactory*>::iterator factItEnd =
        (*buktIt)->puFactoryBucket().end();

    for (; factIt != factItEnd; ++factIt)
    {
      if (factIt->first->isCompleteJourney())
      {
        // to make sure that complete journey PUFactory are started first
        //
        ++thrCount;
        initPricingUnitFactory(
            trx, factIt->second, dataHandle, taskExecutor, thrInputVect, onlyOwFares, onlyNonStops);
      }
    } // for each PU-Factory

    factIt = (*buktIt)->puFactoryBucket().begin();

    for (; factIt != factItEnd; ++factIt)
    {
      if (!factIt->first->isCompleteJourney())
      {
        // now start restof the PUFactory
        //
        ++thrCount;
        initPricingUnitFactory(
            trx, factIt->second, dataHandle, taskExecutor, thrInputVect, onlyOwFares, onlyNonStops);
      }
    }
  } // one Bucket per Pax

  //------------ JOIN rest of the PricingUnitFactory Init  Thread -----------
  //
  taskExecutor.wait();
  //------------ Collect Diagnostics  -----------
  //
  std::vector<PUFInitThreadInput*>::iterator pufIt = thrInputVect.begin();
  std::vector<PUFInitThreadInput*>::iterator pufItEnd = thrInputVect.end();

  for (; pufIt != pufItEnd; ++pufIt)
  {
    if (UNLIKELY((*pufIt)->errResponseCode != ErrorResponseException::NO_ERROR))
    {
      throw ErrorResponseException((*pufIt)->errResponseCode, (*pufIt)->errResponseMsg.c_str());
    }

    diag << (DiagCollector&)*((*pufIt)->diag);
  }

  return true;
}

void
PricingOrchestrator::initPricingUnitFactory(PricingTrx& trx,
                                            PricingUnitFactory* puf,
                                            DataHandle& dataHandle,
                                            TseRunnableExecutor& taskExecutor,
                                            std::vector<PUFInitThreadInput*>& thrInputVect,
                                            bool onlyOwFares,
                                            bool onlyNonStops)
{
  PUFInitThreadInput* thrInput = nullptr;
  dataHandle.get(thrInput);
  thrInputVect.push_back(thrInput);
  DCFactory* factory = DCFactory::instance();
  DiagCollector* dc = factory->create(trx);
  // lint --e{413}
  thrInput->trx(&trx);
  thrInput->diag = dc;
  thrInput->puFactory = puf;

  if (!trx.delayXpn())
    thrInput->puFactory->startTiming();

  thrInput->puFactory->shortCktTimeOut() = getShortCktTimeOut(trx, _shortCktTimeOut);
  thrInput->puFactory->puScopeValidationEnabled() = _factoriesConfig.puScopeValidationEnabled();
  thrInput->puFactory->delayPUValidationMIP() = _delayPUValidationMIP;
  thrInput->puFactory->puScopeValMaxNum() = _puScopeValMaxNum;
  thrInput->puFactory->searchAlwaysLowToHigh() = _factoriesConfig.searchAlwaysLowToHigh();
  thrInput->puFactory->enableCxrFareSearchTuning() = _factoriesConfig.enableCxrFareSearchTuning();
  thrInput->puFactory->maxNbrCombMsgThreshold() = _factoriesConfig.maxNbrCombMsgThreshold();
  thrInput->puFactory->taxPerFareBreak() = _taxPerFareBreak;

  if (LIKELY(_maxSPCTFareCompCount > 2))
  {
    thrInput->puFactory->maxSPCTFareCompCount() = _maxSPCTFareCompCount;
  }
  const Agent* agent = trx.getRequest()->ticketingAgent();
  if (agent && (agent->abacusUser() || agent->infiniUser()))
  {
    if (LIKELY(_maxSPCTFareCompCount_1B1F > 2))
      thrInput->puFactory->maxSPCTFareCompCount() = _maxSPCTFareCompCount_1B1F;
  }

  thrInput->puFactory->maxSPCTPUCount() = _maxSPCTPUCount;
  thrInput->puFactory->setOnlyOwFares(onlyOwFares);
  thrInput->puFactory->setOnlyNonStops(onlyNonStops);
  taskExecutor.execute(*thrInput);
}

void
PricingOrchestrator::PUFInitThreadInput::performTask()
{
  try
  {
    puFactory->initPricingUnitPQ(*diag);
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << ex.message() << " - initPricingUnitPQ failed");
    errResponseCode = ex.code();
    errResponseMsg = ex.message();
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << e.what() << " - initPricingUnitPQ failed");
    errResponseCode = ErrorResponseException::SYSTEM_EXCEPTION;
    errResponseMsg = e.what();
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "UNKNOWN EXCEPTION - initPricingUnitPQ failed");
    errResponseCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    errResponseMsg = string("UNKNOWN EXCEPTION");
  }
}

bool
PricingOrchestrator::createPaxFarePathFactory(
    PricingTrx& trx,
    PUPathMatrix& puMatrix,
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
    std::vector<PaxFarePathFactory*>& pfpFactoryBucket,
    const PaxFarePathFactoryCreator& creator)
{
  std::vector<PUPathMatrix*> puMatrixVect;
  puMatrixVect.push_back(&puMatrix);
  return createPaxFarePathFactory(
      trx, puMatrixVect, puFactoryBucketVect, pfpFactoryBucket, creator);
}

bool
PricingOrchestrator::createPaxFarePathFactory(
    PricingTrx& trx,
    std::vector<PUPathMatrix*>& puMatrixVect,
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect,
    std::vector<PaxFarePathFactory*>& pfpFactoryBucket,
    const PaxFarePathFactoryCreator& creator)
{
  if (trx.paxType().empty())
  {
    LOG4CXX_ERROR(_logger, "In PricingTrx, paxType vector is empty");
    return false;
  }

  //---------------------------------------------------
  // One PaxFarePathFactory per PaxType
  // Instantiate all the PaxFarePathFactory
  //
  uint32_t pos = 0;
  std::vector<PaxType*>::iterator paxIter = trx.paxType().begin();
  int32_t maxSearchFP = -1;

  if (trx.getTrxType() == PricingTrx::IS_TRX)
  {
    if (trx.isAltDates())
    {
      maxSearchFP = _maxSearchNextLevelFarePathAlt;
    }
    else
    {
      ShoppingTrx& shpTrx = static_cast<ShoppingTrx&>(trx);

      if (shpTrx.legs().size() > 2)
      {
        maxSearchFP = _maxSearchNextLevelFarePathComplex;
      }
      else
      {
        maxSearchFP = _maxSearchNextLevelFarePath;
      }
    }
  }

  time_t shortCktKeepValidFPsTime(0), shortCktShutdownFPFsTime(0);
  getPricingShortCktTimes(trx, shortCktKeepValidFPsTime, shortCktShutdownFPFsTime);

  for (; paxIter != trx.paxType().end(); ++paxIter, ++pos)
  {
    PaxFarePathFactory* paxFarePathFactory = creator.create(_factoriesConfig, trx);

    paxFarePathFactory->trx() = &trx;
    paxFarePathFactory->paxType() = *paxIter;
    paxFarePathFactory->puPathMatrixVect() = puMatrixVect;
    paxFarePathFactory->puFactoryBucket() = puFactoryBucketVect[pos];
    paxFarePathFactory->shortCktTimeOut() = getShortCktTimeOut(trx, _shortCktTimeOut);
    paxFarePathFactory->maxSearchNextLevelFarePath() = maxSearchFP;
    paxFarePathFactory->shortCktKeepValidFPsTime() = shortCktKeepValidFPsTime;
    paxFarePathFactory->shortCktShutdownFPFsTime() = shortCktShutdownFPFsTime;
    paxFarePathFactory->pricingOrchestrator() = this;
    pfpFactoryBucket.push_back(paxFarePathFactory);
  }

  pfpFactoryBucket[0]->primaryPaxType() = true; // first one is always primary-PaxType
  // LOG4CXX_ERROR(_logger,"pfpFactoryBucket size = " << pfpFactoryBucket.size())
  return true;
}

bool
PricingOrchestrator::initPaxFarePathFactory(
    PricingTrx& trx,
    std::vector<PaxFarePathFactory*>& paxFarePathFactoryBucket,
    DiagCollector& diag,
    bool eoeCombinabilityEnabled)
{
  LOG4CXX_INFO(_logger, "BEGIN: initFarePathFactory");
  std::vector<PaxFPFInitThreadInput> thrInputVect;
  TseRunnableExecutor initFarePathExecutor(TseThreadingConst::INITFAREPATH_TASK);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  LOG4CXX_INFO(_logger, "Number of PaxFarePathFactory = " << paxFarePathFactoryBucket.size());
  uint32_t numRemaining = static_cast<uint32_t>(paxFarePathFactoryBucket.size());
  std::vector<PaxFarePathFactory*>::iterator pfpfIt = paxFarePathFactoryBucket.begin();
  thrInputVect.resize(numRemaining);
  std::vector<PaxFPFInitThreadInput>::iterator it = thrInputVect.begin();

  for (; pfpfIt != paxFarePathFactoryBucket.end(); ++pfpfIt, numRemaining--, ++it)
  {
    PaxFPFInitThreadInput& thrInput = (*it);
    DCFactory* factory = DCFactory::instance();
    DiagCollector* dc = factory->create(trx);
    // lint --e{413}
    thrInput.diag = dc;
    thrInput.paxFPFactory = *pfpfIt;
    thrInput.paxFPFactory->setEoeCombinabilityEnabled(eoeCombinabilityEnabled);
    thrInput.trx(&trx);

    if (numRemaining > 1)
    {
      initFarePathExecutor.execute(thrInput);
    }
    else
    {
      synchronousExecutor.execute(thrInput);
    }
  }

  initFarePathExecutor.wait();
  //------------ Collect Diagnostics  -----------
  it = thrInputVect.begin();

  for (; it != thrInputVect.end(); ++it)
  {
    if ((*it).errResponseCode != ErrorResponseException::NO_ERROR)
    {
      throw ErrorResponseException((*it).errResponseCode, (*it).errResponseMsg.c_str());
    }

    diag << (DiagCollector&)*((*it).diag);
  }

  LOG4CXX_INFO(_logger, "END: initFarePathFactory");
  return true;
}

void
PricingOrchestrator::PaxFPFInitThreadInput::performTask()
{
  try
  {
    paxFPFactory->initPaxFarePathFactory(*diag);
  }
  catch (ErrorResponseException& ex)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << ex.message() << " - initFarePathPQ failed");
    errResponseCode = ex.code();
    errResponseMsg = ex.message();
  }
  catch (std::exception& e)
  {
    LOG4CXX_ERROR(_logger, "Exception:" << e.what() << " - initFarePathPQ failed");
    errResponseCode = ErrorResponseException::SYSTEM_EXCEPTION;
    errResponseMsg = e.what();
  }
  catch (...)
  {
    LOG4CXX_ERROR(_logger, "UNKNOWN EXCEPTION - initFarePathPQ failed");
    errResponseCode = ErrorResponseException::UNKNOWN_EXCEPTION;
    errResponseMsg = std::string("UNKNOWN EXCEPTION");
  }
}

//------------------------------------------------------------------------------
//
//   @method yyFareValid
//
//   Description: This function will be called to determine the error message
//                for WPQY26'C-YY type of entry. It will determine if the
//                Y26 is a valid YY fare in the fareMarket.
//
//   @param  FareMarket    - fmkt
//
//------------------------------------------------------------------------------

bool
PricingOrchestrator::yyFareValid(FareMarket& fmkt)
{
  PaxTypeFarePtrVecI paxTFare = fmkt.allPaxTypeFare().begin();
  PaxTypeFarePtrVecI paxTFareEnd = fmkt.allPaxTypeFare().end();

  for (; paxTFare != paxTFareEnd; paxTFare++)
  {
    PaxTypeFare& pTfare = **paxTFare;

    if (pTfare.carrier() == INDUSTRY_CARRIER)
    {
      if (pTfare.fareClass() == fmkt.fareBasisCode())
        return true;
    }
  }

  return false;
}

void
PricingOrchestrator::checkWPCYYentry(PricingTrx& trx, std::string& markets, const Itin& itinWPC)
{
  std::vector<FareMarket*>::const_iterator fmIt = itinWPC.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmItEnd = itinWPC.fareMarket().end();
  // if WPQY26'C-YY entry then also check if Y26 is a valid YY fare or not
  bool validYYFare = false;
  bool wpqEntry = false;

  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fmkt = *(*fmIt);

    if (fmkt.breakIndicator())
    {
      continue; // MKT NOT FOR COMB
    }

    if (!(fmkt.fareBasisCode().empty()))
    {
      wpqEntry = true;
    }

    if (wpqEntry && !validYYFare)
    {
      if (yyFareValid(fmkt))
      {
        validYYFare = true;
      }
    }

    if (fmkt.failCode() != ErrorResponseException::NO_ERROR)
    {
      markets = markets + fmkt.boardMultiCity();
      markets = markets + "-";
      markets = markets + fmkt.offMultiCity();
      markets = markets + " ";
    }
  }

  if (wpqEntry && !validYYFare)
  {
    markets.clear();
    markets = "FORMAT - YY FARE MUST BE USED WITH C-YY ENTRY ";
  }
  else
  {
    if (!markets.empty())
      markets = "THERE ARE NO APPLICABLE YY FARES " + markets;
    else
      markets.clear();
  }
}

std::string
PricingOrchestrator::getWPCxxErrMsg(PricingTrx& trx, const Itin& itin)
{
  std::string errMsg;
  std::vector<FareMarket*>::const_iterator fmIt = itin.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmItEnd = itin.fareMarket().end();

  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket& fm = *(*fmIt);

    if (fm.allPaxTypeFare().empty())
    {
      PricingRequest* request = trx.getRequest();

      if (request == nullptr)
        break;

      CarrierCode govCxrOrig =
          request->governingCarrierOverride(fm.travelSeg().front()->segmentOrder());
      CarrierCode govCxrDest =
          request->governingCarrierOverride(fm.travelSeg().back()->segmentOrder());

      if ((fm.governingCarrier() == govCxrOrig) && (fm.governingCarrier() == govCxrDest))
      {
        errMsg += fm.boardMultiCity();
        errMsg += "-";
        errMsg += fm.offMultiCity();
        errMsg += " ";
      }
    }
  }

  if (!errMsg.empty())
  {
    errMsg = "REQUESTED CARRIER HAS NO APPLICABLE FARES " + errMsg;
  }

  return errMsg;
}

void
PricingOrchestrator::getPricingShortCktTimes(PricingTrx& trx,
                                             time_t& shortCktOnPushBackTime,
                                             time_t& shutDownFPFsTime) const
{
  if (trx.getTrxType() == PricingTrx::PRICING_TRX || (PricingTrx::MIP_TRX == trx.getTrxType()))
  {
    const TrxAborter* aborter = trx.aborter();

    if (aborter && aborter->timeout() > 0)
    {
      shortCktOnPushBackTime = time_t(
          aborter->getTimeOutAt() -
          static_cast<int32_t>(aborter->timeout() *
                               (1 - (percentageShortCktOnPushbackTimeout.getValue() / 100.0))));
      shutDownFPFsTime =
          time_t(aborter->getTimeOutAt() - shutdownFPFSSecondsBeforeTimeout.getValue());
    }
  }
}

uint16_t
PricingOrchestrator::getShortCktTimeOut(PricingTrx& trx, uint16_t defaultShortCktTimeOut)
{
  if (LIKELY(ShoppingUtil::isShoppingTrx(trx)))
  {
    // Shopping trx gets timeout from Intellisell/XML-Request <D70>.
    const TrxAborter* aborter = trx.aborter();

    if (LIKELY(aborter && aborter->timeout() > 0))
    {
      if ((trx.paxType().size() > 1) && (_percentageShortCktTimeOutShoppingMultiPax > 0))
      {
        const uint16_t perTimeOutMultiPax = static_cast<uint16_t>(
            (aborter->timeout() * _percentageShortCktTimeOutShoppingMultiPax) / 100);
        return std::max(perTimeOutMultiPax, _minShortCktTimeOutShopping);
      }

      if ((trx.isVisMip() && _applyDomShoppingCktTimeoutValues > 0) ||
          (trx.isMipDomestic() && 2 == _applyDomShoppingCktTimeoutValues))
      {
        uint16_t domShortCktTO =
            static_cast<uint16_t>(aborter->timeout() * _percentageShortCktTimeoutDomShopping / 100);
        return std::max(domShortCktTO, _minShortCktTimeoutDomShopping);
      }
      else
      {
        const uint16_t perTimeOut =
            static_cast<uint16_t>((aborter->timeout() * _percentageShortCktTimeOutShopping) / 100);
        return std::max(perTimeOut, _minShortCktTimeOutShopping);
      }
    }
  }

  return defaultShortCktTimeOut;
}

bool
PricingOrchestrator::process(TktFeesPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(TktFeesPricingTrx)");

  AltPricingTrxData* altTrx = dynamic_cast<AltPricingTrxData*>(&trx);
  if (altTrx && altTrx->isPriceSelectionEntry())
  {
    AccompaniedTravel at;
    at.validateAccTvl(trx, altTrx->accompRestrictionVec());
  }
  return true;
}

bool
PricingOrchestrator::process(AltPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(AltPricingTrx)");

  if (trx.isPriceSelectionEntry())
  {
    AccompaniedTravel at;
    at.validateAccTvl(trx, trx.accompRestrictionVec());
    // Note the rc from validateAccTvl is whether is passed restrictions,
    // not if there was a fatal error.  So ignore it, the xform will read
    // the results from the trx
    return true;
  }
  else
  {
    return process((PricingTrx&)trx);
  }
}

bool
PricingOrchestrator::process(StructuredRuleTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(StructuredRuleTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

namespace
{
struct cmpNoPNROptionsSeg
    : public std::binary_function<const NoPNROptionsSeg* const, const NoPNROptionsSeg* const, bool>
{
  bool operator()(const NoPNROptionsSeg* const left, const NoPNROptionsSeg* const right) const
  {
    return (left->seqNo() > right->seqNo());
  }
};
}

bool
PricingOrchestrator::process(NoPNRPricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(NoPNRPricingTrx)");
  bool result = true;
  const Diagnostic& diagnostic = trx.diagnostic();
  const DiagnosticTypes diagType = diagnostic.diagnosticType();
  bool diagActive = (diagType >= PRICING_DIAG_RANGE_BEGIN && diagType <= PRICING_DIAG_RANGE_END);
  bool lowestFare = trx.lowestFare();
  ErrorResponseException::ErrorResponseCode errCode =
      ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS;
  NoPNRPricingTrx::Solutions& solutions = trx.solutions();

  if (trx.isNoMatch())
  {
    const NoPNROptions* noPNROptions = trx.noPNROptions();
    const std::vector<NoPNROptionsSeg*>& segs = noPNROptions->segs();
    std::priority_queue<NoPNROptionsSeg*, std::deque<NoPNROptionsSeg*>, cmpNoPNROptionsSeg> groups(
        segs.begin(), segs.end());
    NoPNRPricingTrx::FareTypes::FTGroup fareTypeGroup = NoPNRPricingTrx::FareTypes::FTG_NONE;

    if (diagActive)
      fareTypeGroup = trx.mapFTtoFTG(diagnostic.diagParamMapItem(Diagnostic::FARE_TYPE_DESIGNATOR));

    while (!groups.empty())
    {
      NoPNROptionsSeg* seg = groups.top();
      groups.pop();
      NoPNRPricingTrx::FareTypes::FTGroup group =
          (NoPNRPricingTrx::FareTypes::FTGroup)seg->fareTypeGroup();
      const string& groupName = NoPNRPricingTrx::FareTypes::FTGC[group];
      LOG4CXX_DEBUG(_logger, " - processing group: " << groupName);
      trx.processedFTGroup() = group;
      solutions.limit(!lowestFare ? seg->noDisplayOptions() : 1);
      bool all = solutions.all();

      if (diagActive)
      {
        if (fareTypeGroup != NoPNRPricingTrx::FareTypes::FTG_NONE && fareTypeGroup != group)
          continue;

        DiagManager dc(trx, diagType);
        dc << "DIAGNOSTIC FOR - " << groupName;

        if (!all)
          dc << "\n";
      }

      if (!all)
      {
        try
        {
          result = process((AltPricingTrx&)trx);
        }
        catch (ErrorResponseException& exc)
        {
          const ErrorResponseException::ErrorResponseCode& code = exc.code();
          bool noFaresCommon = (code == ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS ||
                                code == ErrorResponseException::NO_FARE_FOR_CLASS_USED ||
                                code == ErrorResponseException::NO_FARE_FOR_CLASS ||
                                code == ErrorResponseException::NO_FARES_RBD_CARRIER);

          if (noFaresCommon || code == ErrorResponseException::NO_CORPORATE_NEG_FARES_EXISTS ||
              code == ErrorResponseException::NO_PUBLIC_FARES_VALID_FOR_PASSENGER ||
              code == ErrorResponseException::NO_PRIVATE_FARES_VALID_FOR_PASSENGER)
          {
            if (!noFaresCommon)
              errCode = code;

            AltPricingUtil::prepareToReprocess(trx);
            result = true;
          }
          else
            throw;
        }
      }

      if (diagActive)
      {
        DiagManager dc(trx, diagType);

        if (!result)
        {
          dc << " - PROCESSING ERROR\n";
          break;
        }
        else if (all)
          dc << " - NOT PROCESSED\n";
      }
      else
      {
        if (!result || solutions.all())
          break;
      }
    }
  }
  else
  {
    solutions.limit(!lowestFare ? solutions.max() : 1);

    try
    {
      result = process((AltPricingTrx&)trx);
    }
    catch (ErrorResponseException& exc)
    {
      if (!trx.isFullFBCItin())
        throw;
    }
  }

  if (solutions.none() && !diagActive)
  {
    if (trx.getOptions()->fbcSelected())
      filterPricingErrors(trx);

    throw ErrorResponseException(errCode);
  }

  return result;
}

namespace
{
MoneyAmount
getExcDiscount(MoneyAmount amount,
               const CurrencyCode& currency,
               const FareMarket& fareMarket,
               RexBaseTrx& trx)
{
  const Percent* percent = trx.getExcDiscountPercentage(fareMarket);
  if (percent && (!fallback::allow100PExcDiscounts(&trx) ? (*percent >= 0.0 && *percent <= 100.0)
                                                         : (*percent > 0.0 && *percent < 100.0)))
  {
    return amount * (*percent) / 100.0;
  }
  else if (const DiscountAmount* discountAmount = trx.getExcDiscountAmount(fareMarket))
  {
    MoneyAmount markupAmt =
        CurrencyUtil::convertMoneyAmount(std::abs(discountAmount->amount),
                                         discountAmount->currencyCode,
                                         currency,
                                         trx,
                                         CurrencyConversionRequest::NO_ROUNDING);
    return std::copysign(markupAmt, discountAmount->amount);
  }
  else
  {
    return 0;
  }
}

void
applyExcDiscounts(RexBaseTrx& trx)
{
  const ExcItin& itin = *trx.exchangeItin().front();
  for (FarePath* farePath : itin.farePath())
  {
    for (PricingUnit* pricingUnit : farePath->pricingUnit())
    {
      for (FareUsage* fareUsage : pricingUnit->fareUsage())
      {
        const MoneyAmount discount = getExcDiscount(fareUsage->paxTypeFare()->totalFareAmount(),
                                                    itin.calculationCurrency(),
                                                    *fareUsage->paxTypeFare()->fareMarket(),
                                                    trx);

        if (std::abs(discount) > EPSILON)
        {
          fareUsage->setDiscAmount(discount);

          // Update the pricing unit and fare path total amount to include the discount
          pricingUnit->setTotalPuNucAmount(pricingUnit->getTotalPuNucAmount() - discount);
          farePath->accumulatePriceDeviationAmount(-discount);
        }
      }

      if (std::abs(pricingUnit->getTotalPuNucAmount()) <= EPSILON)
      {
        pricingUnit->setTotalPuNucAmount(0);
      }
    }

    if (std::abs(farePath->getTotalNUCAmount()) <= EPSILON)
    {
      farePath->setTotalNUCAmount(0);
    }
  }
}
}

bool
PricingOrchestrator::process(RefundPricingTrx& trx)
{
  const TSELatencyData metrics(trx, "PO PROCESS EXC ITIN");
  LOG4CXX_DEBUG(_logger, " - Entered process(RefundPricingTrx)");
  bool result = process(static_cast<PricingTrx&>(trx));

  if (TrxUtil::isAutomatedRefundCat33Enabled(trx) &&
      !fallback::applyExcDiscToMatchedRefundFares(&trx) &&
      trx.trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE)
  {
    applyExcDiscounts(trx);
  }

  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE &&
      trx.diagnostic().diagnosticType() == Diagnostic690)
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(trx));
    diag.enable(Diagnostic690);

    if (diag.isActive())
    {
      Diag690Collector& diag690 = static_cast<Diag690Collector&>(diag);
      diag690.printRefundFarePath();
      diag690.flushMsg();
    }
  }

  return result;
}

bool
PricingOrchestrator::process(RexPricingTrx& trx)
{
  const TSELatencyData metrics(trx,
                               trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE
                                   ? "PO PROCESS EXC ITIN"
                                   : "PO PROCESS NEW ITIN");
  LOG4CXX_DEBUG(_logger, " - Entered process(RexPricingTrx)");
  //@ if trx is in re-construct ExcItin phase
  const bool oldStat = trx.isAnalyzingExcItin();
  trx.setAnalyzingExcItin(trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE);
  bool result = process((PricingTrx&)trx);

  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    trx.getRequest()->lowFareRequested() = 'Y';
    std::vector<FarePath*>& vFp = trx.itin().front()->farePath();

    // if vFp has got 2 items then vFp[0] is as Rebooked and vFp[1] is as Booked
    // if vFp has got 1 item then vFp[0] is as Booked or as Rebooked
    if (vFp.size() > 0)
    {
      if (vFp[0]->rebookClassesExists())
        trx.lowestRebookedFarePath() = vFp[0];
      else
        trx.lowestBookedFarePath() = vFp[0];
    }

    if (vFp.size() > 1)
      trx.lowestBookedFarePath() = vFp[1];

    if (trx.isRebookedSolutionValid() && !trx.isBookedSolutionValid() &&
        trx.reissuePricingErrorCode() == ErrorResponseException::NO_ERROR)
    {
      trx.reissuePricingErrorCode() = ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS;
      trx.reissuePricingErrorMsg() = "NO COMBINABLE FARES FOR CLASS USED";
    }

    if (trx.isNeedDetermineBaseFareForExcFarePath())
    {
      ExcItin* excItin = trx.exchangeItin().front();
      CurrencyCode calcCurrencyOverride = excItin->calcCurrencyOverride();
      excItin->calcCurrencyOverride() = EMPTY_STRING(); // Prepare for Roll back
      PricingUtil::determineBaseFare(excItin->farePath().front(), trx, excItin);
      excItin->calcCurrencyOverride() = calcCurrencyOverride;
    }

    trx.calculateNonrefundableAmountForValidFarePaths();

    if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic690))
    {
      DCFactory* factory = DCFactory::instance();
      DiagCollector& diag = *(factory->create(trx));
      diag.enable(Diagnostic690);

      if (diag.isActive())
      {
        Diag690Collector& diag690 = static_cast<Diag690Collector&>(diag);
        diag690.printSummaryForLowestFarePaths();
        diag690.flushMsg();
      }
    }

    trx.logLowestResultInfo();
  }

  trx.setAnalyzingExcItin(oldStat);
  return result;
}

bool
PricingOrchestrator::process(RexShoppingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(RexShoppingTrx)");
  return process(static_cast<RexPricingTrx&>(trx));
}

bool
PricingOrchestrator::process(ExchangePricingTrx& trx)
{
  LOG4CXX_DEBUG(_logger, " - Entered process(ExchangePricingTrx)");
  return process(static_cast<PricingTrx&>(trx));
}

bool
PricingOrchestrator::process(RexExchangeTrx& trx)
{
  const TSELatencyData metrics(trx,
                               trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE
                                   ? "PO PROCESS EXC ITIN"
                                   : "PO PROCESS NEW ITIN");
  LOG4CXX_DEBUG(_logger, " - Entered process(RexExchangeTrx)");
  //@ if trx is in re-construct ExcItin phase
  const bool oldStat = trx.isAnalyzingExcItin();
  const bool oldStatOfCatchAllBucket = trx.getRequest()->isCatchAllBucketRequest();
  trx.setAnalyzingExcItin(trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE);

  if (trx.trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    trx.getRequest()->setCatchAllBucketRequest(true);
  }

  bool result = process((PricingTrx&)trx);

  if (trx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    trx.getRequest()->lowFareRequested() = 'Y';
    ItinVector::iterator itinIter = trx.itin().begin();

    for (; itinIter != trx.itin().end(); ++itinIter)
    {
      std::vector<FarePath*>& vFp = (*itinIter)->farePath();

      // add farePath to first - lowestRebookedFarePath
      if (vFp.size() > 0)
      {
        if (vFp[0]->rebookClassesExists())
          trx.lowestRebookedFarePath() = vFp[0];
        else
          trx.lowestBookedFarePath() = vFp[0];
      }

      if (trx.isNeedDetermineBaseFareForExcFarePath())
      {
        ExcItin* excItin = trx.exchangeItin().front();
        CurrencyCode calcCurrencyOverride = excItin->calcCurrencyOverride();
        excItin->calcCurrencyOverride() = EMPTY_STRING(); // Prepare for Roll back
        PricingUtil::determineBaseFare(excItin->farePath().front(), trx, excItin);
        excItin->calcCurrencyOverride() = calcCurrencyOverride;
      }

      trx.calculateNonrefundableAmountForValidFarePaths();

      if (trx.diagnostic().diagnosticType() == Diagnostic690)
      {
        DCFactory* factory = DCFactory::instance();
        DiagCollector& diag = *(factory->create(trx));
        diag.enable(Diagnostic690);

        if (diag.isActive())
        {
          Diag690Collector& diag690 = static_cast<Diag690Collector&>(diag);
          diag690.printSummaryForLowestFarePaths();
          diag690.flushMsg();
        }
      }

      trx.logLowestResultInfo();
    }
  }
  else
  {
    trx.validBrands().erase(
        std::find(trx.validBrands().begin(), trx.validBrands().end(), NO_BRAND));
  }

  trx.getRequest()->setCatchAllBucketRequest(oldStatOfCatchAllBucket);

  trx.setAnalyzingExcItin(oldStat);
  return result;
}

void
PricingOrchestrator::completePUFactoryInit(
    std::vector<PricingUnitFactoryBucket*>& puFactoryBucketVect)
{
  std::vector<PricingUnitFactoryBucket*>::iterator buktIt = puFactoryBucketVect.begin();
  std::vector<PricingUnitFactoryBucket*>::iterator buktItEnd = puFactoryBucketVect.end();

  for (; buktIt != buktItEnd; ++buktIt)
  {
    std::map<PU*, PricingUnitFactory*>::iterator factIt = (*buktIt)->puFactoryBucket().begin();
    std::map<PU*, PricingUnitFactory*>::iterator factItEnd = (*buktIt)->puFactoryBucket().end();

    for (; factIt != factItEnd; ++factIt)
    {
      PricingUnitFactory& puf = *(factIt->second);
      puf.initStage() = false;
    }
  }
}

void
PricingOrchestrator::getRec2Cat10(PricingTrx& trx,
                                  std::vector<MergedFareMarket*>& mergedFareMarketVect)
{
  // TSELatencyData metrics( trx, "PO GET REC2 CAT10 DATA");
  DataHandle dataHandle(trx.ticketingDate());
  TseRunnableExecutor pricingPQExecutor(TseThreadingConst::PRICINGPQ_TASK);
  TseRunnableExecutor synchronousExecutor(TseThreadingConst::SYNCHRONOUS_TASK);
  uint32_t remainingMFM = mergedFareMarketVect.size();
  std::vector<MergedFareMarket*>::iterator fmIt = mergedFareMarketVect.begin();
  const std::vector<MergedFareMarket*>::iterator fmItEnd = mergedFareMarketVect.end();

  for (; fmIt != fmItEnd; ++fmIt)
  {
    MergedFareMarket* mfm = *fmIt;
    std::vector<FareMarket*>::const_iterator i = mfm->mergedFareMarket().begin();

    for (; i != mfm->mergedFareMarket().end(); ++i)
    {
      if (LIKELY(trx.matchFareRetrievalDate(**i)))
        break;
    }

    if (UNLIKELY(i == mfm->mergedFareMarket().end()))
      continue; // No fare market has fares for this retrieve date.

    if (!mfm->collectRec2Cat10())
    {
      // LOG4CXX_ERROR(_logger, "FM NOT IN USE: "<<mfm->boardMultiCity()
      //                                   <<"--" <<mfm->offMultiCity());
      continue;
    }

    PricingOrchestrator::GetRec2Cat10Task* getRec2Cat10Task = nullptr;
    dataHandle.get(getRec2Cat10Task);
    getRec2Cat10Task->trx(&trx);
    getRec2Cat10Task->_mfm = mfm;
    TseRunnableExecutor& taskExecutor =
        (remainingMFM > 1) ? pricingPQExecutor : synchronousExecutor;

    taskExecutor.execute(*getRec2Cat10Task);
  }

  // wait for the threads to finish
  pricingPQExecutor.wait();
}

void
PricingOrchestrator::GetRec2Cat10Task::performTask()
{
  bool isLocationSwapped = false;
  std::vector<FareMarket*>::iterator it = _mfm->mergedFareMarket().begin();
  const std::vector<FareMarket*>::iterator itEnd = _mfm->mergedFareMarket().end();

  for (; it != itEnd; ++it)
  {
    FareMarket& fm = *(*it);
    std::vector<PaxTypeBucket>::iterator cortIt = fm.paxTypeCortege().begin();
    const std::vector<PaxTypeBucket>::iterator cortItEnd = fm.paxTypeCortege().end();

    for (; cortIt != cortItEnd; ++cortIt)
    {
      PaxTypeBucket& cortege = *cortIt;
      std::vector<PaxTypeFare*>::iterator ptfIt = cortege.paxTypeFare().begin();
      const std::vector<PaxTypeFare*>::iterator ptfItEnd = cortege.paxTypeFare().end();

      for (; ptfIt != ptfItEnd; ++ptfIt)
      {
        PaxTypeFare& paxTypeFare = *(*ptfIt);

        if (UNLIKELY(!trx()->matchFareRetrievalDate(paxTypeFare)))
          continue;

        if (paxTypeFare.rec2Cat10() == nullptr)
        {
          paxTypeFare.rec2Cat10() =
              RuleUtil::getCombinabilityRuleInfo(*trx(), paxTypeFare, isLocationSwapped);
          // if ( paxTypeFare.rec2Cat10() != 0 )
          //{
          //  LOG4CXX_ERROR(_logger, "Rec2Cat10 FOUND FC:" << paxTypeFare.fareClass()
          //                             << " FB: " << paxTypeFare.createFareBasis(0))
          //}
        }

        // If this fare is a FBR fare then get Rec2 for the BaseFare also
        if (paxTypeFare.isFareByRule())
        {
          const FBRPaxTypeFareRuleData* fbrPtfRd =
              paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

          if (LIKELY(fbrPtfRd))
          {
            PaxTypeFare* bptf = fbrPtfRd->baseFare();

            if (bptf && !bptf->rec2Cat10())
                bptf->rec2Cat10() =
                    RuleUtil::getCombinabilityRuleInfo(*trx(), *bptf, isLocationSwapped);
          }
        }
      }
    }
  }
}

uint32_t
PricingOrchestrator::getActiveThreads()
{
  if (!ThreadPoolFactory::isMetricsEnabled())
    return 0;

  static constexpr auto pools = {TseThreadingConst::PRICINGPQ_TASK,
                                 TseThreadingConst::INITFAREPATH_TASK,
                                 TseThreadingConst::SHOPPINGPQ_TASK,
                                 TseThreadingConst::PRICING_ITIN_TASK,
                                 TseThreadingConst::PRICING_ITIN_ASE_TASK,
                                 TseThreadingConst::NETFAREMARKETS_TASK,
                                 TseThreadingConst::SURCHARGES_TASK};

  return std::accumulate(pools.begin(),
                         pools.end(),
                         size_t(0),
                         [](size_t t, TseThreadingConst::TaskId taskId)
                         { return t + ThreadPoolFactory::getNumberActiveThreads(taskId); });
}

void
PricingOrchestrator::preparePricingProcessForAxess(FPPQItem& fppqItem,
                                                   GroupFarePathFactory& groupFarePathFactory,
                                                   const uint32_t posPax,
                                                   DiagCollector& diag)

{
  FarePath& farePath = *fppqItem.farePath();

  if (!farePath.selectedNetRemitFareCombo())
  {
    return;
  }

  // 1.PaxTypeFares in MergedFareMarket from the original
  //   Cat35 paxTypeFares (FP->PU->FU->FareMarket form TRX) the AXESS repricing for each
  //   requested PaxType.

  uint32_t posPU = 0;
  PUPath* puPath = fppqItem.puPath();
  std::vector<PU*>::iterator it = puPath->allPU().begin();
  std::vector<PU*>::iterator itEnd = puPath->allPU().end();

  for (; it != itEnd; ++it, ++posPU)
  {
    const std::vector<PricingUnit*>& pricingUnitOrig = farePath.pricingUnit();
    PricingUnit* pUnit = pricingUnitOrig[posPU]; // Get the original PU
    uint32_t posFU = 0;
    PU* pu = *it;
    std::vector<MergedFareMarket*>::iterator mfmIt = pu->fareMarket().begin();
    std::vector<MergedFareMarket*>::iterator mfmItEnd = pu->fareMarket().end();

    for (; mfmIt != mfmItEnd; ++mfmIt, ++posFU)
    {
      std::vector<FareUsage*>& fuOrig = pUnit->fareUsage(); // get original FU's
      FareUsage* fu = fuOrig[posFU];
      MergedFareMarket* mfMarket = *mfmIt;
      mfMarket->cxrFarePreferred() = false;
      FareMarket* fm = mfMarket->mergedFareMarket().front(); // 1st FMarket

      if (!fm->fareBasisCode().empty())
      {
        fm->fareBasisCode().clear();
      }

      PaxTypeBucket* paxTypeCortege = fm->paxTypeCortege(farePath.paxType());
      paxTypeCortege->paxTypeFare().clear();
      paxTypeCortege->paxTypeFare() = fu->selectedPTFs();
      std::vector<PaxTypeFare*>::iterator i = fu->selectedPTFs().begin();
      std::vector<PaxTypeFare*>::iterator iE = fu->selectedPTFs().end();

      for (; i != iE; ++i)
      {
        if (!(*i)->fareMarket()->fareBasisCode().empty())
        {
          (*i)->fareMarket()->fareBasisCode().clear();
        }
      }

      if (mfMarket->mergedFareMarket().size() > 1)
      {
        // clear all other FareMarket's
        std::vector<FareMarket*>::iterator iFM = mfMarket->mergedFareMarket().begin();
        std::vector<FareMarket*>::iterator iFMEnd = mfMarket->mergedFareMarket().end();
        mfMarket->mergedFareMarket().erase(iFM + 1, iFMEnd);
      }
    }
  }

  // clear FarePathFactory
  FarePathFactory* fpf = fppqItem.farePathFactory();
  fpf->pricingAxess() = true;
  // clear & init PricingUnitFactory
  std::vector<PricingUnitFactory*>::iterator itPUF = fpf->allPUF().begin();
  std::vector<PricingUnitFactory*>::iterator itPUFEnd = fpf->allPUF().end();

  for (; itPUF != itPUFEnd; ++itPUF)
  {
    (*itPUF)->clear();
    (*itPUF)->initPricingUnitPQ(diag);
  }

  // clear and init PaxFarePathFactory
  const std::vector<PaxFarePathFactoryBase*>& pfpfVec =
      groupFarePathFactory.getPaxFarePathFactoryBucket();
  PaxFarePathFactoryBase* pfpf = pfpfVec[posPax];
  pfpf->pricingAxess() = true;
  pfpf->clear(); // No fpf in the PQ is supposed to be empty
  pfpf->farePathFactoryBucket().clear();
  fpf->clear(); // this needs to be after pfpf->clear()
  pfpf->farePathFactoryBucket().push_back(fpf);
  pfpf->initPaxFarePathFactory(diag);
  return;
}

/*-------------------------------------------------------------------------
 * This method is to process cat 12 surcharge for altdate itin
 *------------------------------------------------------------------------*/
void
PricingOrchestrator::validateSurchargeForAltDate(PricingTrx& trx)
{
  std::vector<uint16_t> categories;
  categories.push_back(RuleConst::SURCHARGE_RULE); // category 12
  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(FPRuleValidation,
                                                                        categories);

  for (Itin* itin : trx.itin())
  {
    if (itin->farePath().empty())
      continue;

    for (FarePath* farePath : itin->farePath())
    {
      validateSurchargeForFarePath(trx, *itin, *farePath, ruleController);
    }
  }
}

void
PricingOrchestrator::validateSurchargeForFarePath(PricingTrx& trx,
                                                  Itin& itin,
                                                  FarePath& farePath,
                                                  PricingUnitRuleController& ruleController)
{
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    farePath.decreaseTotalNUCAmount(pu->taxAmount());
    pu->setTotalPuNucAmount(pu->getTotalPuNucAmount() - pu->taxAmount());
    ruleController.validate(trx, farePath, *pu);
    pu->taxAmount() = 0; // clean up tax amount for rollback currency
  }

  RuleUtil::getSurcharges(trx, farePath);
  PricingUtil::finalPricingProcess(trx, farePath);
}

void
PricingOrchestrator::filterPricingErrors(NoPNRPricingTrx& trx)
{
  std::vector<FareMarket*>::const_iterator fareMktIter = trx.fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fareMktEnd = trx.fareMarket().end();
  std::vector<PaxTypeFare*>::const_iterator pt;
  std::vector<PaxTypeFare*>::const_iterator ptE;
  bool filterFbcFound = false;

  for (; fareMktIter != fareMktEnd; fareMktIter++)
  {
    const FareMarket& fm = **fareMktIter;

    if (fm.breakIndicator() || fm.fareBasisCode().empty())
      continue;

    filterFbcFound = false;

    for (pt = fm.allPaxTypeFare().begin(), ptE = fm.allPaxTypeFare().end(); pt != ptE; pt++)
    {
      const PaxTypeFare& ptf = **pt;

      if (ptf.isFilterPricing() && ptf.validForFilterPricing())
      {
        filterFbcFound = true;
        break;
      }
    }

    if (!filterFbcFound)
      throw ErrorResponseException(ErrorResponseException::FARE_BASIS_NOT_AVAIL,
                                   "FORMAT FARE BASIS NOT AVAILABLE");
  }

  throw ErrorResponseException(ErrorResponseException::FARE_BASIS_NOT_AVAIL, "FARE BASIS FAILS");
}

namespace
{
struct PTFPtrCompare : public std::binary_function<PaxTypeFare*, PaxTypeFare*, bool>
{
  bool operator()(PaxTypeFare* ptfl, PaxTypeFare* ptfr) { return (ptfl == ptfr); }
};

struct TvlSegPtrUpdate
    : public std::binary_function<TravelSeg*, std::map<TravelSeg*, TravelSeg*>*, TravelSeg*>
{
  TravelSeg* operator()(TravelSeg* tvl, std::map<TravelSeg*, TravelSeg*>* segmentMap)
  {
    std::map<TravelSeg*, TravelSeg*>::iterator mIt = segmentMap->find(tvl);

    if (mIt == segmentMap->end())
    {
      TSE_ASSERT(false); // return NULL;//It should not happen!!
    }

    return mIt->second;
  }
};

struct UniqueTvlSegCreator : public std::binary_function<TravelSeg*, DataHandle*, TravelSeg*>
{
  TravelSeg* operator()(TravelSeg* segment, DataHandle* memory) { return segment->clone(*memory); }
};

struct UniquePTFSeeker : public std::binary_function<PaxTypeFare*, std::vector<PaxTypeFare*>*, bool>
{
  bool operator()(PaxTypeFare* ptf, std::vector<PaxTypeFare*>* collection)
  {
    return (find_if(collection->begin(),
                    collection->end(),
                    boost::bind(PTFPtrCompare(), _1, ptf)) != collection->end());
  }
};

struct UpdateFMTvlSegs
    : public std::binary_function<FareMarket*, std::map<TravelSeg*, TravelSeg*>*, void>
{
  void operator()(FareMarket* fm, std::map<TravelSeg*, TravelSeg*>* segs)
  {
    // travelSeg
    {
      std::vector<TravelSeg*> newSegments;
      transform(fm->travelSeg().begin(),
                fm->travelSeg().end(),
                std::back_inserter(newSegments),
                boost::bind(TvlSegPtrUpdate(), _1, segs));
      fm->travelSeg().swap(newSegments);
    }
    // sideTrip ???
    {
      for (std::vector<TravelSeg*>& segmentsVec : fm->sideTripTravelSeg())
      {
        std::vector<TravelSeg*> newSegments;
        transform(segmentsVec.begin(),
                  segmentsVec.end(),
                  std::back_inserter(newSegments),
                  boost::bind(TvlSegPtrUpdate(), _1, segs));
        segmentsVec.swap(newSegments);
      }
    }
    // Stopvers TvlSeg
    {
      std::set<TravelSeg*> newSet;
      TvlSegPtrUpdate updater;
      for (TravelSeg* segment : fm->stopOverTravelSeg())
        newSet.insert(newSet.end(), updater(segment, segs));

      fm->stopOverTravelSeg().swap(newSet);
    }
    // primarySector
    {
      if (fm->primarySector() != nullptr)
        fm->primarySector() = (TvlSegPtrUpdate().operator()(fm->primarySector(), segs));
    }
  }
};

struct UniqueFMCreator : public std::binary_function<FareMarket*, DataHandle*, FareMarket*>
{
  FareMarket* operator()(FareMarket* fm, DataHandle* memory)
  {
    FareMarket* cloneFM = nullptr;
    memory->get(cloneFM);
    fm->clone(*cloneFM);
    // Clone allPTF and update corteges accordingly
    cloneFM->allPaxTypeFare().clear();
    for (PaxTypeBucket& cortege : cloneFM->paxTypeCortege())
      cortege.paxTypeFare().clear();

    UniquePTFSeeker ptfSeek;
    for (PaxTypeFare* ptf : fm->allPaxTypeFare())
    {
      if (!ptf->isValidNoBookingCode())
        continue;

      PaxTypeFare* clonePTF = ptf->clone(*memory);
      clonePTF->fareMarket() = cloneFM;
      cloneFM->allPaxTypeFare().push_back(clonePTF);
      for (PaxTypeBucket& cortege : fm->paxTypeCortege())
      {
        if (ptfSeek(ptf, &cortege.paxTypeFare()))
          cloneFM->paxTypeCortege(cortege.requestedPaxType())->paxTypeFare().push_back(clonePTF);
      }
    }
    return cloneFM;
  }
};

struct UniqueItinCreator : public std::binary_function<Itin*, DataHandle*, void>
{
  void operator()(Itin* original, DataHandle* memory)
  {
    // Clone Travel Segments
    std::vector<TravelSeg*> segmentsClone;
    std::transform(original->travelSeg().begin(),
                   original->travelSeg().end(),
                   std::back_inserter(segmentsClone),
                   boost::bind(UniqueTvlSegCreator(), _1, memory));
    original->travelSeg().swap(segmentsClone);
    // Clone FareMarkte's
    std::vector<FareMarket*>& fareMarkets = original->fareMarket();
    std::vector<FareMarket*> clonedFareMarkets;
    std::transform(fareMarkets.begin(),
                   fareMarkets.end(),
                   std::back_inserter(clonedFareMarkets),
                   boost::bind(UniqueFMCreator(), _1, memory));
    original->fareMarket().swap(clonedFareMarkets);
    // update tvlSeg's in FM accordingly
    TSE_ASSERT(segmentsClone.size() == original->travelSeg().size());
    std::map<TravelSeg*, TravelSeg*> segmentMap;
    uint32_t idx = 0;
    for (TravelSeg* segment : original->travelSeg())
      segmentMap[(segmentsClone[idx++])] = segment;

    std::for_each(original->fareMarket().begin(),
                  original->fareMarket().end(),
                  boost::bind(UpdateFMTvlSegs(), _1, &segmentMap));
  }
};
}

void
PricingOrchestrator::cloneItinObjects(PricingTrx& trx)
{
  if (trx.itin().size() < 2)
    return;

  TSELatencyData metrics(trx, "PO ITIN CLONNING");
  std::for_each(trx.itin().begin(),
                trx.itin().end(),
                boost::bind(UniqueItinCreator(), _1, &trx.dataHandle()));
}

bool
PricingOrchestrator::initFpFactory(GroupFarePathFactory& factory) const
{
  return factory.initGroupFarePathPQ();
}

FareMarket*
PricingOrchestrator::cloneFareMarket(PricingTrx& trx, const FareMarket* fm)
{
  FareMarket* cloneFM = nullptr;
  trx.dataHandle().get(cloneFM);
  fm->clone(*cloneFM);

  for (PaxTypeBucket& cortege : cloneFM->paxTypeCortege())
  {
    const PaxTypeBucket& clonedCortege = *fm->paxTypeCortege(cortege.requestedPaxType());
    cortege.mutableCxrPrecalculatedTaxes() = clonedCortege.cxrPrecalculatedTaxes();
  }

  cloneFM->allPaxTypeFare().clear();
  cloneFM->allPaxTypeFare().reserve(fm->allPaxTypeFare().size());

  FlatMap<PaxTypeFare*, PaxTypeFare*> old2new;

  for (PaxTypeFare* ptf : fm->allPaxTypeFare())
  {
    PaxTypeFare* clonePTF = ptf->clone(trx.dataHandle());
    old2new.unsafe_emplace(ptf, clonePTF);

    clonePTF->fareMarket() = cloneFM;
    cloneFM->allPaxTypeFare().push_back(clonePTF);
  }

  old2new.order();

  for (PaxTypeBucket& cortege : cloneFM->paxTypeCortege())
  {
    const bool hasPrecalculatedTaxes = !cortege.cxrPrecalculatedTaxes().empty();

    const PaxTypeBucket& clonedCortege = *fm->paxTypeCortege(cortege.requestedPaxType());

    for (PaxTypeFare*& ptf : cortege.paxTypeFare())
    {
      PaxTypeFare* const origPtf = ptf;

      PaxTypeFare*& clonePTF = old2new[ptf];
      if (!clonePTF)
      {
        // It seems to be a PTF in cortege that is not in allPaxTypeFare vector.
        clonePTF = ptf->clone(trx.dataHandle());
      }
      ptf = clonePTF;

      if (LIKELY(hasPrecalculatedTaxes))
        cortege.mutableCxrPrecalculatedTaxes().copyAmountsIfPresent(
            clonedCortege.cxrPrecalculatedTaxes(), *origPtf, *ptf);
    }
  }

  return cloneFM;
}

namespace
{
bool
doPrecalculatedTaxesDiffer(const FareMarket& fm,
                           const PricingOrchestrator::CxrPrecalculatedTaxesArray& taxes)
{
  return !std::equal(fm.paxTypeCortege().begin(),
                     fm.paxTypeCortege().end(),
                     taxes.begin(),
                     [](const PaxTypeBucket& cortege, const CxrPrecalculatedTaxes& previousTaxes)
                         -> bool
                     { return (previousTaxes == cortege.cxrPrecalculatedTaxes()); });
}

bool
hasPrecalculatedTaxes(const FareMarket& fm)
{
  return std::any_of(fm.paxTypeCortege().begin(),
                     fm.paxTypeCortege().end(),
                     [](const PaxTypeBucket& cortege)
                     { return (cortege.cxrPrecalculatedTaxes().isProcessed()); });
}
}

void
PricingOrchestrator::cloneFmForEstimatedTaxes(PricingTrx& trx,
                                              Itin* itin,
                                              FareMarketTaxesCopyMap& previousAmounts)
{
  if (previousAmounts.empty())
    return;

  std::vector<FareMarket*> fmsToAdd;

  std::vector<FareMarket*>::iterator i = itin->fareMarket().begin();

  while (i != itin->fareMarket().end())
  {
    FareMarket* fm = *i;
    const auto iFm = previousAmounts.find(fm);

    if (iFm == previousAmounts.end() || !doPrecalculatedTaxesDiffer(*fm, iFm->second))
    {
      ++i;
      continue;
    }

    FareMarket* cloneFM = cloneFareMarket(trx, fm);
    {
      CxrPrecalculatedTaxesArray& taxes = iFm->second;
      size_t i = 0;
      for (PaxTypeBucket& cortege : fm->paxTypeCortege())
        cortege.mutableCxrPrecalculatedTaxes() = std::move(taxes[i++]);
    }

    fmsToAdd.push_back(cloneFM);
    i = itin->fareMarket().erase(i);
  }

  itin->fareMarket().insert(itin->fareMarket().end(), fmsToAdd.begin(), fmsToAdd.end());
  trx.fareMarket().insert(trx.fareMarket().end(), fmsToAdd.begin(), fmsToAdd.end());
}

void
PricingOrchestrator::precalculateTaxes(PricingTrx& trx, Itin& itin)
{
  DiagCollector* diag = DCFactory::instance()->create(trx);

  std::vector<SurchargesThreadTask> tasks;
  tasks.reserve(itin.fareMarket().size());
  SurchargesPreValidator validator(
      trx, _factoriesConfig, itin, diag, trx.getOptions()->getNumFaresForCat12Estimation());
  TseRunnableExecutor surchargesExecutor(TseThreadingConst::SURCHARGES_TASK);

  FareMarketTaxesCopyMap previousAmounts;

  for (FareMarket* fm : itin.fareMarket())
  {
    if (hasPrecalculatedTaxes(*fm))
    {
      PricingOrchestrator::CxrPrecalculatedTaxesArray& taxes = previousAmounts[fm];
      size_t i = 0;
      for (PaxTypeBucket& cortege : fm->paxTypeCortege())
      {
        taxes[i] = std::move(cortege.mutableCxrPrecalculatedTaxes());
        cortege.mutableCxrPrecalculatedTaxes().clear();
      }
    }

    tasks.emplace_back(trx, validator, *fm);
    surchargesExecutor.execute(tasks.back());
  }

  surchargesExecutor.wait();
  cloneFmForEstimatedTaxes(trx, &itin, previousAmounts);
}

void
PricingOrchestrator::getValidBrands(PricingTrx& trx)
{
  std::set<BrandCode> validBrandsSet;

  const bool addCrossBrand =
    showCrossBrandOptionFirst.getValue() && BrandingUtil::isCrossBrandOptionNeeded(trx);

  for (Itin* itin : trx.itin())
  {
    validBrandsSet.insert(itin->brandCodes().begin(), itin->brandCodes().end());

    if (trx.getRequest()->isCatchAllBucketRequest())
      itin->brandCodes().insert(NO_BRAND);

    if (addCrossBrand)
      itin->brandCodes().insert(ANY_BRAND_LEG_PARITY);
  }

  trx.validBrands().insert(trx.validBrands().end(), validBrandsSet.begin(), validBrandsSet.end());

  // We need to return brands in the order they were received from Merchandising
  // Since serializer iterates through trx.validBrands() we need to make sure it is sorted correctly
  BrandedFaresComparator brandComparator(trx.brandProgramVec(), _logger);
  std::sort(trx.validBrands().begin(), trx.validBrands().end(), brandComparator);

  if (trx.getRequest()->isCatchAllBucketRequest())
    trx.validBrands().push_back(NO_BRAND);

  if (addCrossBrand)
    trx.validBrands().insert(trx.validBrands().begin(), ANY_BRAND_LEG_PARITY);
}

void
PricingOrchestrator::checkFlexFaresForceCorpFares(PricingTrx& trx, const uint32_t& brandIndex)
{
  // In flex fare, if a particular group has account codes or corpIds, then it's necessary
  // to set PricingOptions::forceCorpFares() to true. When this group is done processing
  // and if the next group doesn't have corp id/account codes, then it's necessary to
  // set PricingOptions::forceCorpFares() back to its original value
  //
  // When PO is done, it's necessary to set PricingOptions::forceCorpFares() back to its
  // original value as well
  if (trx.isFlexFare())
  {
    flexFares::GroupsData::const_iterator itr = trx.getRequest()->getFlexFaresGroupsData().begin();
    std::advance(itr, brandIndex);
    flexFares::GroupId groupId = itr->first;

    // If this flex fare group has corp Ids/account codes
    if (trx.getRequest()->getFlexFaresGroupsData().getCorpIds(groupId).size() > 0 ||
        trx.getRequest()->getFlexFaresGroupsData().getAccCodes(groupId).size() > 0)
    {
      if (!fallback::fallbackCorpIDFareBugFixing(&trx))
      {
        if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx))
        {
          if (trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupId))
            trx.getOptions()->forceCorpFares() =
                trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(groupId);
          else
            trx.getOptions()->forceCorpFares() = trx.getOptions()->initialForceCorpFares();
        }
        else
          trx.getOptions()->forceCorpFares() = true;
      }
      else
      {
        if (trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupId) &&
            !fallback::fallbackFlexFareGroupNewXCLogic(&trx))
          trx.getOptions()->forceCorpFares() =
              trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(groupId);
        else
          trx.getOptions()->forceCorpFares() = true;
      }
    }
    else
    {
      if (!fallback::fallbackFlexFareGroupNewXCLogic(&trx))
      {
        if (trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareGroup(groupId))
          trx.getOptions()->forceCorpFares() =
              trx.getRequest()->getMutableFlexFaresGroupsData().isFlexFareXCIndicatorON(groupId);
        else
          trx.getOptions()->forceCorpFares() = trx.getOptions()->initialForceCorpFares();
      }
      else
        trx.getOptions()->forceCorpFares() = _isForcedCorpFares;
    }
  }
}

void
PricingOrchestrator::throughAvailabilitySetup(PricingTrx& trx,
                                              std::vector<PUPathMatrix*>& puPathMatrixVect)
{
  if (trx.getTrxType() != PricingTrx::MIP_TRX || !trx.getOptions()->applyJourneyLogic())
    return;

  if (puPathMatrixVect.empty())
    return;

  RexPricingTrx* rexPricingTrx = dynamic_cast<RexPricingTrx*>(&trx);
  if (rexPricingTrx && (rexPricingTrx->trxPhase() == RexPricingTrx::REPRICE_EXCITIN_PHASE))
    return;

  for (PUPathMatrix* puPathMatrix : puPathMatrixVect)
    getFMCOSBasedOnAvailBreak(trx, puPathMatrix->itin());
}

void
PricingOrchestrator::priceSimilarItins(similaritin::Context& context,
                                       GroupFarePathFactory& groupFarePathFactory,
                                       const std::vector<FPPQItem*>& groupFPath)
{
  PricingTrx& trx = context.trx;
  DiagManager manager(trx);
  if (UNLIKELY(manager.activate(Diagnostic990).activate(Diagnostic991).isActive()))
  {
    similaritin::DiagnosticWrapper diagnostic(trx, manager.collector());
    similaritin::Pricing<similaritin::DiagnosticWrapper>(context, diagnostic)
        .price(groupFarePathFactory, groupFPath);
    return;
  }
  similaritin::NoDiagnostic noDiagnostic;
  similaritin::Pricing<similaritin::NoDiagnostic>(context, noDiagnostic)
      .price(groupFarePathFactory, groupFPath);
}

} // tse
