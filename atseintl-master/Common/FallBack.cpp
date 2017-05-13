//----------------------------------------------------------------------------
//
//  Description: FallBack TSE Resources
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Common/Config/ConfigMan.h"
#include "Common/Config/DynamicConfigurableFallbackValue.h"
#include "Common/Config/FallbackValue.h"
#include "Common/Logger.h"
#include "Common/TseUtil.h"

namespace tse
{
namespace fallback
{
#ifdef CONFIG_HIERARCHY_REFACTOR
  const std::string FallbackValue::CONFIG_SECTION = "FALLBACK_SECTION";
#else
  const std::string FallbackValueBase::CONFIG_SECTION = "FALLBACK_SECTION";
#endif

}

// HOWTO:
//   1. Use FALLBACK_DEF to create a fallback in places where trx is available. Such fallbacks can
//   be changed on a transaction level
//
//   2. Use FIXEDFALLBACK_DEF to create fallbacks in other places. Such fallbacks need a server
//   restart to active.
//
//   3. Consult http://wiki.sabre.com/confluence/display/V2Shopping/Creating+fallbacks for detailed
//   information


FALLBACK_DEF(fallbackFixFRRHpuForNet,
             "FALLBACK_FIX_FRR_HPU_FOR_NET",
             false);
FALLBACK_DEF(fallbackFootNotePrevalidationForAltDates,
             "FALLBACK_FOOTNOTE_PREVALIDATION_FOR_ALTDATES",
             false);
FALLBACK_DEF(fallbackNonRefAmountOptimizationForAbacus,
             "FALLBACK_NON_REF_AMT_OPTIMIZATION_FOR_ABACUS",
             false);
FALLBACK_DEF(fallbackFixFNFailedFares, "FALLBACK_FIX_FN_FAILED_FARES", false);
FALLBACK_DEF(fallbackWpncCloning, "FALLBACK_WPNC_CLONING", false);
FALLBACK_DEF(fallbackFsc1155MoreThan147, "FALLBACK_FSC_1155_MORE_THAN_147", false);
FALLBACK_DEF(fallbackValidatingCxrActivation, "FALLBACK_VALIDATINGCXR_ACTIVATION", false);
FALLBACK_DEF(fallbackValidatingCxrGTC, "FALLBACK_VALIDATINGCXR_GTC", false);
FALLBACK_DEF(fallbackGSAChildItinFix2, "FALLBACK_GSA_CHILDITIN_FIX2", false);
FALLBACK_DEF(validateCarrierModuleMotherSolution, "VALIDATE_CARRIER_MODULE_MOTHER_SOLUTION", false);
FALLBACK_DEF(clearSurchargesForGSARevalidation, "CLEAR_SURCHARGES_FOR_GSA_REVALIDATION", false);
FALLBACK_DEF(fallbackValidatingCxrMultiSp, "FALLBACK_VALIDATINGCXR_MULTI_SP", false);
FALLBACK_DEF(fallbackMafTaxFix, "FALLBACK_MAF_TAX_FIX", false);
FALLBACK_DEF(fallbackMipHPUFix, "FALLBACK_MIP_HPU_FIX", false);
FALLBACK_DEF(fallbackCat31KeepWholeFareSetOnExcFM,
             "FALLBACK_CAT31_KEEP_WHOLE_FARE_SET_ON_EXC_FM",
             false);
FALLBACK_DEF(AF_CAT33_ResponseConverter2, "AF_CAT33_ResponseConverter2", false)
FALLBACK_DEF(endorsementExpansion, "ENDORSEMENT_EXPANSION", false);
FALLBACK_DEF(fallbackASLDisplayC57, "FALLBACK_ASL_DISPLAY_C57", false);
FALLBACK_DEF(fallbackBrandedFaresPricing, "BRANDED_FARES_PRICING", false);
FALLBACK_DEF(fallbackBrandedFaresTNShopping, "BRANDED_FARES_TN_SHOPPING", false);
FALLBACK_DEF(fallbackBrandedFaresFareDisplay, "BRANDED_FARES_FAREDISPLAY", false);
FALLBACK_DEF(fallbackCbsForNoFaresFiled, "FALLBACK_CBS_FOR_NO_FARES_FILED", false);
FALLBACK_DEF(fallbackFootNotePrevalidation, "FALLBACK_FOOTNOTE_PREVALIDATION", false);
FALLBACK_DEF(fallbackNewRBDforWPAE, "FALLBACK_NEW_RBDBYCABIN_FOR_WPAE", false);
FALLBACK_DEF(fallbackNewRBDforM70, "FALLBACK_NEW_RBDBYCABIN_FOR_M70", false);
FALLBACK_DEF(sswvt22412ObFeeTCurConv, "SSWVT22412_OB_FEE_CUR_CONV", true);
FALLBACK_DEF(fallbackBrandedServiceInterface, "FALLBACK_BRANDED_SERVICE_INTERFACE", false);
FIXEDFALLBACK_DEF(memoryManagerChanges, "MEMORY_MANAGER_CHANGES", false);
FALLBACK_DEF(fallbackNGSJCBPaxTuning, "FALLBACK_NGS_JCB_PAX_TUNING", false);
FALLBACK_DEF(fallbackUpdateAvailabilityMap, "FALLBACK_UPDATE_AVAILABILITY_MAP", false);
FALLBACK_DEF(gsaSurchargesFix, "GSA_SURCHARGES_FIX", false);
FALLBACK_DEF(fallbackWPRDASLFix, "FALLBACK_WPRD_ASL_FIX", false);
FALLBACK_DEF(gsaSurchargesFixMotherModule, "GSA_SURCHARGES_FIX_MOTHER_MODULE", false);
FALLBACK_DEF(specifiedCarrierForFamilyLogic, "SPECIFIED_CARRIER_FOR_FAMILY_LOGIC", false);
FALLBACK_DEF(fallbackSkipAcrossStopoverDomesticFM,
             "FALLBACK_SKIP_ACROSS_STOPOVER_DOMESTIC_FM",
             false);
FALLBACK_DEF(fallbackFixMslNetFareAmount, "FALLBACK_FIX_MSL_NETFARE_AMOUNT", false);
FALLBACK_DEF(fallbackFixMslNetFareAmountV2, "FALLBACK_FIX_MSL_NETFARE_AMOUNT_V2", false);
FIXEDFALLBACK_DEF(fallbackVisMarriedCabins, "FALLBACK_VIS_MARRIED_CABINS", false);
FIXEDFALLBACK_DEF(fallbackSolSwapperUseSopScore, "FALLBACK_SOL_SWAPPER_USE_SOP_SCORE", false);
FIXEDFALLBACK_DEF(fallbackFMDirectionSetting, "FALLBACK_FM_DIRECTION_SETTING", false);
FALLBACK_DEF(fallbackISAvailTuning, "FALLBACK_IS_AVAIL_TUNING", false);
FIXEDFALLBACK_DEF(fallbackSkipDummyFareValidation, "FALLBACK_SKIP_DUMMY_FARE_VALIDATION", false);
FALLBACK_DEF(fallbackFixPQNRedistributedWholesale, "FALLBACK_FIX_PQN_REDISTRIBUTED_WHOLESALE", false);
FALLBACK_DEF(fallbackValCxrR2Cat15, "FALLBACK_VALCXR_R2_CAT15", false);
FALLBACK_DEF(fallbackGoverningCrxForT171, "FALLBACK_GOVERNING_CRX_FOR_T171", false);
FIXEDFALLBACK_DEF(fallbackCheckBckCodeSegStatusFix,
                  "FALLBACK_CHECK_BCK_CODE_SEG_STATUS_FIX",
                  false);
FALLBACK_DEF(fallbackCATElementMark, "FALLBACK_CAT_ELEMENT_MARK", false);
FALLBACK_DEF(fallbackNewRBDforAB240, "FALLBACK_NEW_RBDBYCABIN_FOR_AB240", false);
// this will turn off cat31 nonref for NON Abacus only
FALLBACK_DEF(inactiveNonRefAmountForNonAbacusUsers, "FALLBACK_P78637_NON_REF_OTHERS", false);
FIXEDFALLBACK_DEF(APO29538_StopoverMinStay, "APO29538_STOPOVERMINSTAY", false);
FALLBACK_DEF(unifyMemoryAborter, "UNIFY_MEMORY_ABORTER", false)
FALLBACK_DEF(yqyrADTotalPricing, "FALLBACK_YQYR_AD_TOTAL_PRICING", false);
FIXEDFALLBACK_DEF(fallbackMipOptEOERules, "FALLBACK_MIP_OPT_EOE_RULES", false);
FIXEDFALLBACK_DEF(fallbackMileageSurchargeExceptionValidation,
                  "FALLBACK_MILAGE_SURCHARGE_EXCEPTION_VALIDATION",
                  false);
FIXEDFALLBACK_DEF(fallbackShoppingPQCabinClassValid,
                  "FALLBACK_SHOPPINGPQ_CABIN_CLASS_VALID",
                  false);
FIXEDFALLBACK_DEF(fallbackRemoveFailedSopsInvalidation,
                  "FALLBACK_REMOVE_FAILED_SOPS_INVALIDATION",
                  false);
FALLBACK_DEF(fallbackFMRbyte58, "FALLBACK_FMR_BYTE58", false);
FIXEDFALLBACK_DEF(wheelTimerTaskExecutor, "WHEEL_TIMER_TASK_EXECUTOR", false)
FALLBACK_DEF(fallbackDividePartySolutionOptimization,
             "FALLBACK_DIVIDEPARTY_SOLUTION_OPTIMIZATION",
             false);
FALLBACK_DEF(fallbackAtpcoTaxDefaultAgeForADTCNN, "FALLBACK_ATPCO_TAX_DEFAULT_AGE_ADT_CNN", false);
FALLBACK_DEF(ATPCO_TAX_AcceptOCTagPrePaid, "ATPCO_TAX_ACCEPTOCTAGPREPAID", false);
FALLBACK_DEF(ATPCO_TAX_TaxPreSortFix, "ATPCO_TAX_TAX_PRESORT_FIX", false);
FALLBACK_DEF(ATPCO_TAX_OcCurrencyConversionFix, "ATPCO_TAX_OC_CURRENCY_CONVERSION_FIX", false);
FIXEDFALLBACK_DEF(ATPCO_TAX_X1byCodeDAORefactor, "ATPCO_TAX_X1_BYCODEDAO_REFACTOR", false);
FALLBACK_DEF(ATPCO_TAX_useIATARulesRoundingFix, "ATPCO_TAX_USE_IATA_RULES_ROUNDING_FIX", false);
FALLBACK_DEF(fallbackChangeSpecialDomesticRoundingLogic, "FALLBACK_CHANGE_SPECIAL_DOMESTIC_ROUNDING_LOGIC", true);
FALLBACK_DEF(fallbackAtpcoTaxTotalRounding, "FALLBACK_ATPCO_TAX_TOTAL_ROUNDING", false);
FIXEDFALLBACK_DEF(freqFlyerNewFormat, "FF_NEW_FORMAT", false);
FIXEDFALLBACK_DEF(fallbackErrorsInTaxDAOFix, "FALLBACK_ERRORS_IN_TAX_DAO_FIX_FIXED", false);
FIXEDFALLBACK_DEF(fallbackTaxRulesRecordDAOLogging, "FALLBACK_TAXRULESRECORDDAO_LOGGING_FIXED", false);
FALLBACK_DEF(validatingCxrOnBadDataFix, "VALIDATING_CXR_ON_BAD_DATA_FIX", false);
FALLBACK_DEF(taxResponseDuplicatesFix, "TAX_RESPONSE_DUPLICATES_FIX", false);
FALLBACK_DEF(fallbackXmlBrandInfoFaredisplay, "XML_BRAND_INFO_FAREDISPLAY", false)
FALLBACK_DEF(fallbackExcludeCodeSharingFix, "FALLBACK_EXCLUDE_CODE_SHARE_SOP_FIX", false);
FALLBACK_DEF(fallbackForINFINITvlCommencementDate,
             "FALLBACK_FOR_INFINI_TVL_COMMENCEMENT_DATE",
             false);
FALLBACK_DEF(fallbackFlexFareCopyValidationStatusForCorpIdAccCode,
             "FALLBACK_FLEXFARE_COPY_VALIDATION_STATUS_FOR_CORPID_ACCCODE",
             false);
FALLBACK_DEF(fixAPO37202, "FALLBACK_FIX_APO_37202", false);
FALLBACK_DEF(fallbackSearchForBrandsPricing, "FALLBACK_SEARCH_FOR_BRANDS_PRICING", false);
FALLBACK_DEF(fallbackDebugOverrideBrandingServiceResponses,
             "FALLBACK_DEBUG_OVERRIDE_BRANDING_SERVICE_RESPONSES",
             false);
FALLBACK_DEF(fallbackFootNoteR2Optimization, "FOOTNOTE_R2_OPTIMIZATION", false);
FALLBACK_DEF(fallbackGfrR2Optimization, "GFR_R2_OPTIMIZATION", false);
FALLBACK_DEF(fallbackSkipVTAForGSA, "FALLBACK_SKIP_VTA_FOR_GSA", false);
FALLBACK_DEF(fallbackSettingVTAToTrueForGSA, "FALLBACK_SETTING_VTA_TO_TRUE_FOR_GSA", false);
FIXEDFALLBACK_DEF(fallbackAvailBreakInterlineJourney,
                  "FALLBACK_AVAIL_BREAK_INTERLINE_JOURNEY",
                  false);
FALLBACK_DEF(similarItinSelectThroughAvailability, "SIMILAR_ITIN_SELECT_THROUGH_AVAILABILITY", false)
FALLBACK_DEF(fallbackRoutingForChildren, "ROUTING_FOR_CHILDREN", false);
FALLBACK_DEF(cat7DiagChange, "CAT7_DIAG_CHANGE", false);
FALLBACK_DEF(fallbackPromotionalAvailability, "FALLBACK_PROMOTIONAL_AVAILABILITY", false);
FALLBACK_DEF(fallbackFareSelction2016, "FALLBACK_FARE_SELECTION_2016", false);
FALLBACK_DEF(fallbackFareSelctionActivateMIP, "FALLBACK_FARE_SELECTION_ACTIVATE_MIP", false);
FIXEDFALLBACK_DEF(tmpFixForRequestThrottling, "FALLBACK_FIX_BCH_TAX_REQUEST_THROTTLING", false);
FIXEDFALLBACK_DEF(atpcoBchParallelProcessing, "FALLBACK_ATPCO_BCH_PARALLEL_PROCESSING", false);
FALLBACK_DEF(fallbackAB240, "FALLBACK_AB240", false);
FALLBACK_DEF(obFeesWPAforAbacus, "OB_FEES_WPA_FOR_ABACUS", false);
FALLBACK_DEF(obFeesWPAforInfini, "OB_FEES_WPA_FOR_INFINI", false);
FALLBACK_DEF(purgeBookingCodeOfNonAlpha, "FALLBACK_PURGE_BKC", false);
FALLBACK_DEF(fallbackAYTax, "FALLBAC_AY_TAX", false);
FALLBACK_DEF(fallbackDivideParty, "DIVIDE_PARTY", false);
FIXEDFALLBACK_DEF(fallbackRefactoring00, "REFACTORING_00", false);
FIXEDFALLBACK_DEF(fallbackDisableESVIS, "FALLBACK_DISABLE_ESVIS", false);
FALLBACK_DEF(fallbackRoutingValidationStartOnSecondMap,
             "FALLBACK_ROUTING_VALIDATION_START_ON_SECOND_MAP",
             false);
FIXEDFALLBACK_DEF(AB240_DecoupleServiceFeesAndFreeBag,
                  "AB240_DECOUPLE_SERVICE_FEES_AND_FREE_BAG_SERVICE",
                  false);
FALLBACK_DEF(fallbackDiag198, "FALLBACK_DIAG_198", false);
FIXEDFALLBACK_DEF(throttleEarly, "THROTTLE_EARLY", false);
FIXEDFALLBACK_DEF(chargerDontSkipReqHeader, "CHARGER_DONT_SKIP_REQ_HEADER_FIXED", false);
FALLBACK_DEF(fallback_FLE_combinability106, "FALLBACK_FLE_COMB_106", false);
FALLBACK_DEF(fallback_FLE_combinabilityAll, "FALLBACK_FLE_COMB_ALL", false);
FALLBACK_DEF(validateSurchargesMotherSolution, "VALIDATE_SURCHARGES_MOTHER_SOLUTION", false);
FALLBACK_DEF(fallbackFVORuleDirectionalityTuning, "FALLBACK_FVO_RULE_DIRECTIONALITY_TUNING", false);
FALLBACK_DEF(obFeesWpaOption1, "OB_FEE_WPA_OPTION1", false);
FALLBACK_DEF(fallbackAMChargesTaxes, "FALLBACK_AM_CHARGES_TAXES", false);
FALLBACK_DEF(fallbackAPO37838Record1EffDateCheck, "FALLBACK_APO37838_REC1_EFFDATE", false);
FALLBACK_DEF(fallbackGSAMipDifferentialFareFix, "FALLBACK_GSA_MIP_DIFFERENTIALFARE_FIX", false);
FALLBACK_DEF(ab240FallbackNoC7bValidation, "AB240_FALLBACK_NO_C7B_VALIDATION", false);
FALLBACK_DEF(ab240FixSsdmps171, "AB240_FIX_SSDMPS_171", false);
FALLBACK_DEF(fallbackAncillaryPricingMetrics, "FALLBACK_ANCILLARY_PRICING_METRICS", false);
FALLBACK_DEF(fallbackSimpleTripCorrectionOWFare, "FALLBACK_SIMPLETRIP_CORRECTION_OW_FARE", false);
FIXEDFALLBACK_DEF(detectRoutingCycles, "DETECT_ROUTING_CYCLES", false);
FIXEDFALLBACK_DEF(routingCycleDetectorHardening, "ROUTING_CYCLE_DETECTOR_HARDENING", false);
FIXEDFALLBACK_DEF(fallbackSplitYQYR_TaxShop, "FALLBACK_SPLIT_YQYR_TAXSHOP", false);
FALLBACK_DEF(fallbackExpandPreferredCxrCutoffCoeff,
             "FALLBACK_EXPAND_PREFERRED_CXR_CUTOFF_COEFF",
             false);
FALLBACK_DEF(fallbackExpandCarrierContinueProcessing,
             "FALLBACK_EXPAND_CARRIER_CONTINUE_PROCESSING",
             false);
FALLBACK_DEF(fallbackExpandCarrierRemoveMissingCarriers,
             "FALLBACK_EXPAND_CARRIER_REMOVE_MISSING_CARRIERS",
             false);
FALLBACK_DEF(revalidateVcxForSimilarItins, "REVALIDATE_VCX_FOR_SIMILAR_ITINS", false);
FALLBACK_DEF(revalidateVcxForSimilarItins_YQYR, "REVALIDATE_VCX_FOR_SIMILAR_ITINS_YQYR", false);
FALLBACK_DEF(revalidateVcxForSimilarItins_CAT12, "REVALIDATE_VCX_FOR_SIMILAR_ITINS_CAT12", false);
FALLBACK_DEF(clearGsaClonedFarePathsFamilyLogic, "CLEAR_GSA_CLONED_FARE_PATHS_FAMILY_LOGIC", false);
FALLBACK_DEF(setPrimarySectorChildItins, "SET_PRIMARY_SECTOR_CHILD_ITIN", false);
FALLBACK_DEF(allValCxrMapYqyrFamilyLogic, "ALL_VAL_CXR_MAP_YQYR_FAMILY_LOGIC", false);
FALLBACK_DEF(revalidateCat12MotherModule, "REVALIDATE_CAT12_MOTHER_MODULE", false);
FIXEDFALLBACK_DEF(validateAllCat16Records, "VALIDATE_ALL_CAT16_RECORDS", false);
FALLBACK_DEF(tfpInDiag, "TFP_IN_DIAG", false);
FALLBACK_DEF(ssdsp1836surchWithGeo, "SSDSP1836_SURCH_WITH_GEO", false);
FALLBACK_DEF(serviceFeeTimeInResponse, "SERVICE_FEE_TIME_IN_RESPONSE", false)
FALLBACK_DEF(serviceFeeOpenSeg, "SERVICE_FEE_OPEN_SEG", false)
FIXEDFALLBACK_DEF(fallbackTaxShoppingAmountGrouping, "FALLBACK_TAXSHOPPING_AMOUNT_GROUPING", false);
FALLBACK_DEF(fallbackAPM837, "FALLBACK_APM837", false);
FALLBACK_DEF(fallbackBrandDirectionality, "FALLBACK_BRAND_DIRECTIONALITY", false);
FIXEDFALLBACK_DEF(apo43454d8tax, "APO43454_D8_TAX_FIXED", false);
FALLBACK_DEF(apo44968apo44798XRandTQtaxfix, "APO44968_APO44798_XRANDTQTAX_FIX", false);
FALLBACK_DEF(apo44470multiCityIsStop, "APO44470_MULTICITY_IS_STOP", false);
FALLBACK_DEF(apo36040Cat6TSI5Check, "FALLBACK_APO36040_TSI5_CAT6_CHECK", false);
FIXEDFALLBACK_DEF(noNraAttrInShoppingResponse, "NO_NRA_ATTR_IN_SHOPPING_RESPONSE", false);
FALLBACK_DEF(fallbackNewGroup99Validation, "NEW_GROUP99_VALIDATION", false);
FIXEDFALLBACK_DEF(fallbackTaxShoppingTransitTimes, "FALLBACK_TAXSHOPPING_TRANSIT_TIMES", false);
FALLBACK_DEF(neutralToActualCarrierMapping, "NEUTRAL_TO_ACTUAL_CARRIER", false);
FALLBACK_DEF(fallbackInterlineIgnorePartners, "FALLBACK_INTERLINE_IGNORE_PARTNERS", false);
FALLBACK_DEF(fallbackProcessTypeLAndT, "FALLBACK_PROCESS_TYPE_L_AND_T", false);
FALLBACK_DEF(fallbackCreateHIPDiagAlways, "CREATE_HIP_DIAG_ALWAYS", false)
FALLBACK_DEF(fallbackDisableStructuredRule, "DISABLE_STRUCTURED_RULE", false);
FALLBACK_DEF(fallbackCommissionManagement, "FALLBACK_COMMISSION_MANAGEMENT", false);
FALLBACK_DEF(setFurthestPointForChildItins, "SET_FURTHEST_POINT_FOR_CHILD_ITINS", false);
FALLBACK_DEF(fallbackFRRRedistFix, "FALLBACK_FRR_REDIST_FIX", false);
FIXEDFALLBACK_DEF(dontCopyCacheInitializer, "DONT_COPY_CACHE_INITIALIZER", false);
FALLBACK_DEF(callOnceGNRCheck, "CALL_ONCE_GNR_CHECK", false);
FIXEDFALLBACK_DEF(replaceFastMutexInBigIP, "REPLACE_FASTMUTEX_BIGIP", false);
FALLBACK_DEF(azPlusUp, "AZ_PLUS_UP", false);
FIXEDFALLBACK_DEF(fallbackTaxShoppingStopoverGrouping,
                  "FALLBACK_TAXSHOPPING_STOPOVER_GROUPING",
                  false);
FALLBACK_DEF(fallbackDebugOverrideDssv2AndAsv2Responses,
             "FALLBACK_DEBUG_OVERRIDE_DSSV2_AND_ASV2_RESPONSES",
             false);
FALLBACK_DEF(keepCat11ResultInIs, "KEEP_CAT11_RESULT_IN_IS", false);
FALLBACK_DEF(fallbackFixForFlexFaresMultiPax, "FALLBACK_FOR_FLEX_FARES_MULTI_PAX", false);
FALLBACK_DEF(fallbackFRRFixNonItBtDirectTicketing,
             "FALLBACK_FRR_FIX_NON_ITBT_DIRECT_TICKETING",
             false);
FALLBACK_DEF(fallbackFRRCat25AmtCheckFix, "FALLBACK_FRR_CAT25_AMT_CHECK_FIX", false);
FALLBACK_DEF(fallbackFRRCat25Responsive, "FALLBACK_FRR_CAT25_RESPONSIVE", false);
FALLBACK_DEF(taxRexPricingRefundableIndAllTypes, "TAX_REXPRICING_REFUNDABLE_IND_ALL_TYPES", false);
FALLBACK_DEF(taxRefundableIndUseCat33Logic, "TAX_REFUNDABLE_IND_USE_CAT33_LOGIC", false);
FIXEDFALLBACK_DEF(taxReissueEmptyCarierProcessing,
                  "TAX_REISSUE_EMPTY_CARRIER_PROCESSING_FIXED",
                  false);
FALLBACK_DEF(taxRexPricingRefundableInd, "TAX_REXPRICING_REFUNDABLE_IND", false);
FALLBACK_DEF(taxRexPricingTxType, "TAX_REXPRICING_TX_TYPE", false);
FALLBACK_DEF(cutFFSolutions, "CUT_FF_SOLUTIONS", false);
FALLBACK_DEF(taxFixActualPaxType, "TAX_FIX_ACTUAL_PAX_TYPE", false);
FALLBACK_DEF(taxDisableTransitViaLocLogic, "TAX_DISABLE_TRANSIT_VIA_LOC_LOGIC", false);
FALLBACK_DEF(taxUtRestoreTransitLogic, "TAX_UT_RESTORE_TRANSIT_LOGIC", false);
FALLBACK_DEF(taxWithinSpecGetLastLoc2, "TAX_UT_WITHIN_SPEC_GET_LAST_LOC2", false);
FALLBACK_DEF(taxRefactorDiags, "TAX_REFACTOR_DIAGS", false);
FALLBACK_DEF(overusedMutexInLimitationOnIndirectTravel, "OVERUSED_MUTEX_IN_LIM_ON_IN_TVL", false)
FIXEDFALLBACK_DEF(fallbackTaxShoppingFirstTvlDateGrouping,
                  "FALLBACK_TAXSHOPPING_FIRST_TVL_DATE_GROUPING",
                  false);
FALLBACK_DEF(fallbackForcePricingWayForSFR, "FALLBACK_FORCE_PRICING_WAY_FOR_SFR", true);
FALLBACK_DEF(fallbackISBrandsInCalintl, "FALLBACK_IS_BRANDS_IN_CALINTL", false);
FALLBACK_DEF(fallback_AB240_UpgradeCheckForUPGroupCode,
             "FALLBACK_AB240_UPGRADE_CHECK_FOR_UP_GROUP_CODE",
             false);
FALLBACK_DEF(fallbackStopOverLegByCityCode, "FALLBACK_STOP_OVER_LEG_BY_CITY_CODE", false);
FALLBACK_DEF(fallbackAddC52ForRecordsWithNoCharge,
             "FALLBACK_ADD_C52_FOR_RECORDS_WITH_NO_CHARGE",
             false);

FALLBACK_DEF(fallbackAddC52ForRecordsWithNoChargeAB240,
             "FALLBACK_ADD_C52_FOR_RECORDS_WITH_NO_CHARGE_AB240",
             false);
FALLBACK_DEF(reduceTemporaryVectors, "REDUCE_TEMPORARY_VECTORS", false);
FIXEDFALLBACK_DEF(reduceTemporaryVectorsFixed, "REDUCE_TEMPORARY_VECTORS_FIXED", false);
FIXEDFALLBACK_DEF(validateBrandForKeepFare, "VALIDATE_BRAND_FOR_KEEP_FARE", false);
FIXEDFALLBACK_DEF(cat33AmountSelection, "CAT33_AMOUNT_SELECTION", false);
FALLBACK_DEF(fallbackSOPUsagesResize, "FALLBACK_SOPUSAGES_RESIZE", false);
FIXEDFALLBACK_DEF(fallbackFmBrandStatus, "FALLBACK_FM_BRAND_STATUS", false);
FIXEDFALLBACK_DEF(taxShoppingPfcInfant, "FALLBACK_TAXSHOPPING_PFC_INFANT", false);
FIXEDFALLBACK_DEF(xmlEmptyStringComparison, "XML_EMPTY_STRING_COMPARISON", false);
FALLBACK_DEF(fallbackNonPreferredVC, "FALLBACK_NON_PREFERRED_VC", false);
FALLBACK_DEF(conversionDateSSDSP1154, "CONVERSION_DATE_SSDSP1154", false);
FALLBACK_DEF(fallbackPriceByCabinActivation, "FALLBACK_PRICE_BY_CABIN_ACTIVATION", false);
FALLBACK_DEF(fallbackFareDisplayByCabinActivation, "FALLBACK_FQ_BY_CABIN_ACTIVATION", false);
FALLBACK_DEF(bagInPqLowerBound, "BAG_IN_PQ_LOWER_BOUND", false);
FALLBACK_DEF(smpValidateFPPQItem, "SMP_VALIDATE_FPPQ_ITEM", false);
FALLBACK_DEF(fallbackPreferredVC, "FALLBACK_PREFERRED_VC", false);
FALLBACK_DEF(fallbackConstructedFareEffectiveDate, "FALLBACK_CONSTRUCTED_FARE_EFFECTIVE_DATE", false);
FALLBACK_DEF(fallbackMipBForTNShopping, "FALLBACK_MIP_B_FOR_TN_SHOPPING", false);
FIXEDFALLBACK_DEF(fallbackFilterCacheNotify, "FALLBACK_FILTER_CACHE_NOTIFY_FIXED", false);
FIXEDFALLBACK_DEF(fallbackRmHistCaches, "FALLBACK_RM_HIST_CACHES_FIXED", false);
FIXEDFALLBACK_DEF(fallbackRCMultiTransportFix, "FALLBACK_RC_MULTITRASPORT_FIX", false);
FIXEDFALLBACK_DEF(fallbackRCCommandInterface, "FALLBACK_RC_COMMAND_INTERFACE_FIXED", false);
FIXEDFALLBACK_DEF(fallbackRCChanges, "FALLBACK_RC_CHANGES_FIXED", false);
FIXEDFALLBACK_DEF(fallbackRCLoadOnUpdate, "FALLBACK_RC_LOAD_ON_UPDATE_FIXED", false);
FALLBACK_DEF(fallbackCommissionCapRemoval, "FALLBACK_COMMISSIONCAP_REMOVAL", false);
FALLBACK_DEF(fallbackVCxrTrailerMsgDisplay, "FALLBACK_VCXR_TRAILER_MSG_DISPLAY", false);
FALLBACK_DEF(fallbackDJCBannerFix, "DJC_BANNER_FIX", false);
FALLBACK_DEF(fallbackHIPMinimumFareCat25FareExempt, "HIP_MINIMUMFARE_CAT25FAREEXEMPT", false);
FALLBACK_DEF(fallbackGovCxrForSubIata21, "FALLBACK_GOVCXR_FOR_SUBIATA21", false);
FALLBACK_DEF(fallbackCat35If1Prevalidation, "FALLBACK_CAT35_IF1_PREVALIDATION", false);
FALLBACK_DEF(cat35_psgTypeMatching, "CAT35_PSGTYPEMATCHING", false);
FALLBACK_DEF(markupAnyFareOptimization, "MARKUPANYFAREOPTIMIZATION", false);
FALLBACK_DEF(Taxes_BulkCharger_PassengerWithAge, "TAXES_BULKCHARGER_PASSENGERWITHAGE", false);
FALLBACK_DEF(fae_checkFareCompInfo, "FAE_CHECKFARECOMPINFO", false);
FALLBACK_DEF(azPlusUpExc, "AZ_PLUS_UP_EXC", false);
FALLBACK_DEF(fixSSDSP1780, "FIX_SSDSP_1780", false);
FALLBACK_DEF(fallbackDiag867ItinNum, "FALLBACK_DIAG867_ITIN_NUM", false);
FALLBACK_DEF(fallbackDuplicateVCxrMultiSP, "FALLBACK_DUPLICATE_VCXR_MULTI_SP", false);
FALLBACK_DEF(fallbackLegacyTaxProcessorRefactoring,
             "FALLBACK_LEGACY_TAX_PROCESSOR_REFACTORING",
             false);
FALLBACK_DEF(fallbackOCFeesInSearchForBrandsPricing,
             "FALLBACK_OC_FEES_IN_SEARCH_FOR_BRANDS_PRICING",
             false);
FALLBACK_DEF(fixCoreDumpTaxItinerary, "FIX_CORE_DUMP_TAX_ITINERARY", false);
FALLBACK_DEF(fallbackSkipRecordsWithOverlappingOccurenceForAB240,
             "FALLBACK_SKIP_RECORDS_WITH_OVERLAPPING_OCCURENCE_FOR_AB240",
             false);
FALLBACK_DEF(fallbackEnableFamilyLogicInBfa, "FALLBACK_ENABLE_FAMILY_LOGIC_IN_BFA", false);
FALLBACK_DEF(fallbackEnableFamilyLogicAvailiablityValidationInBfa,
             "FALLBACK_ENABLE_FAMILY_LOGIC_AVAILIABILITY_VALIDATION_IN_BFA",
             false);
FALLBACK_DEF(fallbackEnableFamilyLogicAdditionalSolutionsInBfa,
             "FALLBACK_ENABLE_FAMILY_LOGIC_ADDITIONAL_SOLUTIONS_IN_BFA",
             false);
FALLBACK_DEF(fallbackEnableFamilyLogicOriginAvailabilityInBfa,
             "FALLBACK_ENABLE_FAMILY_LOGIC_ORIGINAL_AVAILABILITY_IN_BFA",
             false);
FALLBACK_DEF(fallbackAMCPhase2, "FALLBACK_AGENCY_COMMISSION_PHASE2", false);
FIXEDFALLBACK_DEF(firstTvlDateRages, "FALLBACK_FIRST_TVLDATES_RANGES", false);
FALLBACK_DEF(smpCat33DoubleConversionFix, "SMP_CAT33_DOUBLE_CONVERSION_FIX", false);
FALLBACK_DEF(fallbackHalfRTPricingForIbf, "FALLBACK_HALF_RT_PRICING_FOR_IBF", false);
FALLBACK_DEF(dynamicThroughFarePrecedence, "DYNAMIC_THROUGH_FARE_PRECEDENCE", false)
FALLBACK_DEF(fallbackFdoAndSdoBaggageDefinedByPiecesOnly,
             "FDO_AND_SDO_BAGGAGE_DEFINED_BY_PIECES_ONLY",
             false);
FALLBACK_DEF(fallbackDisplaySelectiveItinNum, "FALLBACK_DISPLAY_SELECTIVE_ITIN_NUM", false);
FIXEDFALLBACK_DEF(transitTimeByTaxes, "FALLBACK_TRANSIT_TIME_BY_TAXES", false);
FALLBACK_DEF(r2sDirectionalityOpt, "R2S_DIRECTIONALITY_OPT", false);
FALLBACK_DEF(fallbackDisplayPriceForBaggageAllowance,
             "FALLBACK_DISPLAY_PRICE_FOR_BAGGAGE_ALLOWANCE",
             false);
FALLBACK_DEF(fallbackRemoveAttrMNVInVCL, "FALLBACK_REMOVE_ATTR_MNV_IN_VCL", false);
FALLBACK_DEF(compDiffCmdPricing, "COMP_DIFF_CMD_PRICING", false);
FIXEDFALLBACK_DEF(repricingForTaxShopping, "FALLBACK_REPRICING_FOR_TAXSHOPPING", false);
FALLBACK_DEF(setHistoricalOTA, "SET_HISTORICAL_OTA", false);
FALLBACK_DEF(fallbackDisplaySelectedItinNumInDiag,
             "FALLBACK_DISPLAY_SELECTED_ITIN_NUM_IN_DIAG", false);

FALLBACK_DEF(fallbackAMCFareBasisFix, "FALLBACK_AGENCY_COMMISSION_FARE_BASIS_FIX", false);
FALLBACK_DEF(fallbackFlexFareGroupNewXCLogic, "FALLBACK_FFG_NEW_XC_LOGIC", false);
FALLBACK_DEF(extractReissueExchangeFormatter, "EXTRACT_REISSUE_EXCHANGE_FORMATTER", false);
FALLBACK_DEF(fallbackSSDMPS242, "FALLBACK_SSDMPS242", false);
FALLBACK_DEF(spanishResidentFares, "SPANISH_RESIDENT_FARES_MBR400", false);
FALLBACK_DEF(fallbackAb240SkipOccurencesAfterInfinity,
             "FALLBACK_AB240_SKIP_OCCURENCES_AFTER_INFINITY",
             false);
FALLBACK_DEF(fallbackValidatingCarrierInItinOrder,
             "FALLBACK_VALIDATING_CARRIER_IN_ITIN_ORDER",
             false);
FALLBACK_DEF(fallbackOperatingCarrierForAncillaries,
             "FALLBACK_OPERATING_CARRIER_FOR_ANCILLARIES",
             false);
FIXEDFALLBACK_DEF(enhancedRefundDiscountApplierRefactor,
                  "ENHANCED_REFUND_DISCOUNT_APPLIER_REFACTOR",
                  false);
FALLBACK_DEF(fallbackVCLForISShortcut, "FALLBACK_VCL_FOR_IS_SHORTCUT", false);
FALLBACK_DEF(fallbackICERAPO44318Fix, "FALLBACK_ICER_APO_44318_FIX", false);
FALLBACK_DEF(fallbackDisableIEMDForGroup99, "FALLBACK_DISABLE_IEMD_FOR_GROUP99", false);
FALLBACK_DEF(fallbackSumUpAvlForMotherItin, "FALLBACK_SUM_UP_AVL_FOR_MOTHER_ITIN", false);
FALLBACK_DEF(smpMissingDataSingleDeparture, "SMP_MISSING_DATA_SINGLE_DEPARTURE", false);
FALLBACK_DEF(fallbackFixDisplayValidatingCarrierDiag867,
             "FALLBACK_AGENCY_COMMISSION_DIAGNOSTIC_867_FIX",
             false);
FALLBACK_DEF(fallbackGBTaxEquipmentException, "FALLBACK_TAXSHOPPING_GB_EQUIPMENT_EXCEPTION", false);
FIXEDFALLBACK_DEF(fallbackGBTaxEquipmentException, "FALLBACK_GB_EQUIPMENT_TYPE_CHECK", false);
FALLBACK_DEF(fallbackPVCWithoutMultiSp, "FALLBACK_PVC_WITHOUT_MULTI_SP", false);
FALLBACK_DEF(SSDSP_1844_removeArunkSegForSFR, "SSDSP_1844_REMOVE_ARUNK_SEG_FOR_SFR", false);
FALLBACK_DEF(cat33DoubleConversionFix, "CAT33_DOUBLE_CONVERSION_FIX", false);
FIXEDFALLBACK_DEF(memoryOptimizationForTaxShopping,
                  "FALLBACK_MEMORY_OPTIMIZATION_TAXSHOPPING",
                  false);
FALLBACK_DEF(cat25baseFarePrevalidation, "CAT25_BASE_FARE_PREVALIDATION", false);
FALLBACK_DEF(fallbackAddSequenceNumberInOOS, "FALLBACK_ADD_SEQUENCE_NUMBER_IN_OOS", false);
FALLBACK_DEF(fallbackRBDByCabinOpt, "FALLBACK_RBBBYCABIN_OPT", false);
FALLBACK_DEF(fallbackFlexFareGroupNewXOLogic, "FALLBACK_FFG_NEW_XO_LOGIC", false);
FALLBACK_DEF(fallbackAlways_display_purchase_by_date_for_AB240,
             "FALLBACK_ALWAYS_DISPLAY_PURCHASE_BY_DATE_FOR_AB240",
             false);
FIXEDFALLBACK_DEF(fallbackAMC2SrcTgtPCCChange, "FALLBACK_AMC2_SRCTGT_PCC_CHANGE", false);
FALLBACK_DEF(fallbackAAExcludedBookingCode, "FALLBACK_AA_EXCLUDED_BOOKING_CODE", false);
FALLBACK_DEF(fallbackProcessEmptyTktEndorsements, "FALLBACK_PROCESS_EMPTY_TKT_ENDORSEMENTS", false);
FALLBACK_DEF(fallbackEndorsementsRefactoring, "FALLBACK_ENDORSEMENTS_REFACTORING", false);
FALLBACK_DEF(fallbackLatamJourneyActivation, "LATAM_JOURNEY_ACTIVATION", false);
FALLBACK_DEF(failPricingUnitsInIs, "FAIL_PRICING_UNITS_IN_IS", false);
FALLBACK_DEF(skippedBitValidationInIsOpt, "SKIPPED_BIT_VALIDATION_IN_IS_OPT", false);
FALLBACK_DEF(smpSkipDepartureMatchingForRecordValidation,
             "SMP_SKIP_DEPARTURE_MATCHING_FOR_RECORD_VALIDATION",
             false)
FALLBACK_DEF(cat9LeastRestrictive, "CAT9_LEAST_RESTRICTIVE", false)
FALLBACK_DEF(cat33DoubleConversionFixForAll, "CAT33_DOUBLE_CONVERSION_FIX_FOR_ALL", false);
FALLBACK_DEF(fallbackMipRexSingleItinException, "FALLBACK_MIP_REX_SINGLE_ITIN_EXCEPTION", false);
FALLBACK_DEF(fallbackAlternateVcxrInsteadOptionalVcxr,
             "FALLBACK_ALTERNATE_VCXR_INSTEAD_OPTIONAL_VCXR",
             false);
FALLBACK_DEF(fallbackSPInHierarchyOrder, "FALLBACK_SP_IN_HIERARCHY_ORDER", false);
FALLBACK_DEF(highestGovCxrCheckInForeignDomFareMkts,
             "FALLBACK_HIGHEST_GOV_CXR_CHK_IN_FOREIGN_DOM_FARE_MKTS",
             false);
FALLBACK_DEF(fallbackAMC2ShoppingChange, "FALLBACK_AMC2_SHOPPING_CHANGE", false);
FALLBACK_DEF(fallbackAMC2Cat35CommInfo, "FALLBACK_AMC2_CAT35_COMM_INFO", false);
FALLBACK_DEF(smpShoppingISCoreFix, "SMP_SHOPPING_IS_CORE_FIX", false)
FALLBACK_DEF(ngsMaximumPenaltyFarePathValidation,
             "NGS_MAXIMUM_PENALTY_FARE_PATH_VALIDATION",
             false);
FALLBACK_DEF(dateAdjustTaxFix, "DATE_ADJUST_TAX_FIX", false);
FALLBACK_DEF(virtualFOPMaxOBCalculation, "VIRTUAL_FOP_MAX_OB", false);
FALLBACK_DEF(fallbackJira1908NoMarkup, "FALLBACK_JIRA_1908_NO_MARKUP", false);
FALLBACK_DEF(ssdsp1720Cat31CurrencyFix, "SSDSP1720_CAT31_CURRENCY_FIX", false);
FALLBACK_DEF(exsCalendar, "EXS_CALENDAR", false);
FALLBACK_DEF(fallbackCorpIDFareBugFixing, "FALLBACK_CORP_ID_FARE_BUG_FIXING", false);
FALLBACK_DEF(fallbackSurroundOscWithOccForMonetaryDiscount,
             "FALLBACK_SURROUNDING_OSC_WITH_OCC_FOR_MONETARY_DISCOUNT",
             false);
FALLBACK_DEF(fallbackShoppingSPInHierarchyOrder, "FALLBACK_SHOPPING_SP_IN_HIERARCHY_ORDER", false);
FALLBACK_DEF(fallbackFFGAcctCodeFareFix, "FALLBACK_FFG_ACCT_CODE_FARE_FIX", false);
FALLBACK_DEF(fallbackNonBSPVcxrPhase1, "FALLBACK_NON_BSP_VCXR_PHASE1", false);
FALLBACK_DEF(fallbackAPO44549SkipMaxTimecheckAtPULevel,
             "FALLBACK_APO44549_CAT8SKIP_MAXTIME_CHECK_INPU",
             false);
FALLBACK_DEF(ssdsp1788SanitizerErrorFix, "SSDSP1788_SANITIZER_ERROR_FIX", false);
FALLBACK_DEF(fallbackAPO45157GatewayReqCheck, "FALLBACK_APO45157_CAT8GTWY_REQUIRED_CHECK", false);
FALLBACK_DEF(fallbackAMC2Diag867NewLogic, "FALLBACK_AMC2_DIAG867_NEW_LOGIC", false);
FALLBACK_DEF(fallbackFRRProcessingRetailerCode, "FALLBACK_FRR_PROCESSING_RETAILER_CODE", false);
FALLBACK_DEF(fallbackFixProcessingRetailerCodeXRS, "FALLBACK_FIX_PROCESSING_RETAILER_CODE_XRS", false);
FALLBACK_DEF(monetaryDiscountTaxesForAllSegments,
             "MONETARY_DISCOUNT_TAXES_FOR_ALL_SEGMENTS",
             false);
FALLBACK_DEF(tktDesignatorCoreFix, "TKT_DESIGNATOR_CORE_FIX", false);
FIXEDFALLBACK_DEF(fallbackFFGAcctCodeCorpIDFilterFix,
                  "FALLBACK_FFG_ACCT_CODE_CORPID_FILTER_FIX",
                  false);
FIXEDFALLBACK_DEF(fallbackFRRaddRetailerCodeInFRR, "FALLBACK_FRR_ADD_RETAILER_CODE_IN_FRR", false);
FALLBACK_DEF(fallbackAcctCodeCorpIDGroupFilter, "FALLBACK_ACCT_CODE_CORPID_GROUP_FILTER", false);
FALLBACK_DEF(automatedRefundCat33TaxDriverRefactoring,
             "AUTOMATED_REFUND_CAT_33_TAX_DRIVER_REFACTORING",
             false)
FALLBACK_DEF(rexFareTypeTbl, "REX_FARE_TYPE_TBL", false);
FALLBACK_DEF(fallbackNoItinParityForShopByLeg, "FALLBACK_NO_ITIN_PARITY_FOR_SHOP_BY_LEG", false);
FALLBACK_DEF(fallbackBSPMt3For1S, "FALLBACK_BSP_MT3_FOR_1S", false);
FALLBACK_DEF(fallbackAMCFixMissingMileageNetAmtCat35L,
             "FALLBACK_FIX_MISSING_MILEAGE_NETAMT_CAT35L",
             false);
FALLBACK_DEF(fallbackapo44172Cat8EmbeddedGtwyChk,
    "FALLBACK_APO44172_EMBEDDED_GTWY_CHK", false);
FALLBACK_DEF(cosExceptionFixDynamic, "COS_EXCEPTION_FIX_DYNAMIC", false);
FALLBACK_DEF(monetaryDiscountFlatTaxesApplication, "MONETARY_DISCOUNT_FLAT_TAXES_APPLICATION", false);
FALLBACK_DEF(logRequestWhenLowMemory, "LOG_REQUEST_WHEN_LOW_MEMORY", false);
FALLBACK_DEF(fallbackRBDByCabinPh2, "FALLBACK_RBDBYCABIN_PH2", false);
FALLBACK_DEF(fallbackFlexFareGroupNewJumpCabinLogic, "FALLBACK_FFG_NEW_JUMP_CABIN_LOGIC", false);
FIXEDFALLBACK_DEF(keepFareCombinationsFixed, "KEEP_FARE_COMBINATIONS_FIXED", false);
FALLBACK_DEF(skipMixedClassForExcItin, "SKIP_MIXED_CLASS_FOR_EXC_ITIN", false);
FALLBACK_DEF(cat9RecurringSegFcScope, "CAT9_RECURRING_SEG_FC_SCOPE", false);
FALLBACK_DEF(fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852,
             "FALLBACK_ADD_SEGMENTS_NUMBERS_TO_CHECKED_PORTION_SECTION_IN_DIAGNOSTIC_852",
             false);
FALLBACK_DEF(familyLogicSideTrip, "FAMILY_LOGIC_SIDE_TRIP", false);
FALLBACK_DEF(familyLogicRevalidateAllVc, "FAMILY_LOGIC_REVALIDATE_ALL_VC", false);
FALLBACK_DEF(segmentAttributesRefactor, "SEGMENT_ATTRIBUTES_REFACTOR", false);
FALLBACK_DEF(taxNationVectorRefactoring, "TAX_NATION_VECTOR_REFACTORING", false)
FALLBACK_DEF(taxProcessExemptionTable, "TAX_PROCESS_EXEMPTION_TABLE", false)
FALLBACK_DEF(Cat33GetTicketingDateRefactoring, "CAT_33_GET_TICKETING_DATE_REFACTORING", false)
FALLBACK_DEF(apo40993UsDotBaggageIfStopoverInUs, "APO40993_USDOT_BAGGAGE_IF_STOPOVER_IN_US", false)
FALLBACK_DEF(fallbackBfaLoopTimeout, "FALLBACK_BFA_LOOP_TIMEOUT", false);
FALLBACK_DEF(taxReissueCat33OnlyInd, "TAX_REISSUE_CAT33_ONLY_IND", false);
FALLBACK_DEF(taxShowSomeExemptions, "TAX_SHOW_SOME_EXEMPTIONS", false);
FALLBACK_DEF(taxRefundableIndUseD92Date, "TAX_REFUNDABLE_IND_USE_D92_DATE", false);
FALLBACK_DEF(exchangeRefactorRaiiDate, "EXCHANGE_REFACTOR_RAII_DATE", false);
FALLBACK_DEF(cat33DisablePssPath, "CAT33_DISABLE_PSS_PATH", false);
FALLBACK_DEF(ocFeesAmountRoundingRefactoring, "OC_FEES_AMOUNT_ROUNDING_REFACTORING", false);
FALLBACK_DEF(changeIndOpt, "CHANGE_IND_OPT", false);
FALLBACK_DEF(fallbackJumpCabinExistingLogic,"FALLBACK_JUMPCABIN_EXISTING_LOGIC", false);
FALLBACK_DEF(fallbackXMLVCCOrderChange, "FALLBACK_SSDSP1975_VCC_ORDER", false);
FALLBACK_DEF(fallbackFRRmatchTravelRangeX5, "FALLBACK_FRR_MATCH_TRAVEL_RANGE_X5", false);
FALLBACK_DEF(fallbackFixForRTPricingInSplit, "FALLBACK_FIX_FOR_SPLIT_BY_FARE_IN_RT_PRICING", false);
FALLBACK_DEF(fallbackValidatingCarrierInItinOrderMultiSp,
             "FALLBACK_VALIDATING_CARRIER_IN_ITIN_ORDER_MULTI_SP",
             false);
FALLBACK_DEF(skipBkcValidationForFlownFM, "SKIP_BKC_VALIDATION_FOR_FLOWN_FM", false);
FIXEDFALLBACK_DEF(dynamicCastsFBRPTFRuleData, "DYNAMIC_CASTS_FBRPTFRULEDATA", false);
FALLBACK_DEF(shallowScanShoppingHist, "SHALLOW_SCAN_SHOPPINGHIST", false);
FALLBACK_DEF(shallowScanHistorical, "SHALLOW_SCAN_HISTORICAL", false);
FALLBACK_DEF(cat33FixTaxesOnChangeFeeAF, "CAT_33_FIX_TAXES_ON_CHANGE_FEE_AF", false);
FIXEDFALLBACK_DEF(failIfIncorrectShoppingRequestType,
                  "FAIL_IF_INCORRECT_SHOPPING_REQUEST_TYPE",
                  true);
FALLBACK_DEF(fallbackapo37432Cat9BlankIORecurSeg, "FALLBACK_APO37432_BLANK_IO_RECUR_SEG_CAT9", false)
FALLBACK_DEF(SSDSP_1988, "SSDSP_1988", false);
FALLBACK_DEF(fallbackFixUnbrandedInfFares, "FALLBACK_FIX_UNBRANDED_INF_FARES", false);
FALLBACK_DEF(fallbackSoldoutFixForHalfRTPricing, "FALLBACK_SOLDOUT_FIX_FOR_HALF_RT_PRICING", false);
FIXEDFALLBACK_DEF(fallbackFFRaddFocusCodeInFFR, "FALLBACK_FFR_ADD_FOCUS_CODE_IN_FFR", false);
FALLBACK_DEF(exscChangeFinderRefactor, "EXSC_CHANGE_FINDER_REFACTOR", false);
FALLBACK_DEF(sfrPenaltyCurrency, "SFR_PENALTY_CURRENCY", false);
FALLBACK_DEF(excDiscDiag23XImprovements, "EXC_DISC_DIAG_23X_IMPROVEMENTS", false);
FALLBACK_DEF(simpleFareMarketPaths, "SIMPLE_FAREMARKET_PATHS", false);
FALLBACK_DEF(simpleFareMarketBuilding, "SIMPLE_FARE_MARKET_BUILDING", false);
FIXEDFALLBACK_DEF(excDiscountSegsInit, "EXC_DISCOUNT_SEGS_INIT", false);
FALLBACK_DEF(fixSSDTP103CurrencyDisplay, "FIX_SSDTP103_CURRENCY_DISPLAY", false);
FALLBACK_DEF(yqyrGetYQYRPreCalcLessLock, "SHORTCUT_YQYR_PRECALC_CRITICAL_SECTION", false)
FALLBACK_DEF(fallbackIbfParityThruMarketsFix, "FALLBACK_IBF_PARITY_THRU_MARKETS_FIX", false);
FALLBACK_DEF(noDelayedValidationForAwardRequest, "NO_DELAYED_VALIDATION_FOR_AWARD_REQUEST", false);
FALLBACK_DEF(yqyrRexFix, "YQYR_REX_FIX", false);
FALLBACK_DEF(cat9unusedCode, "CAT9_UNUSED_CODE", false);
FALLBACK_DEF(footNotePrevalidationForExc, "FOOTNOTE_PREVALIDATION_FOR_EXC", false)
FALLBACK_DEF(Cat33_Diag, "CAT33_DIAG", false)
FALLBACK_DEF(latencyDataInAncillaryPricingResponse, "LATENCY_DATA_IN_ANCILLARY_PRICING_RESPONSE", false)
FIXEDFALLBACK_DEF(fallback_record2_sharing_part2, "FALLBACK_RECORD2_SHARING_PART2", false);
FALLBACK_DEF(ssdsp1508fix, "SSDSP_1508_FIX", false)
FALLBACK_DEF(fallbackSSDSP2058FMSelection, "FIX_SSDSP2058_FARE_MARKET_SELECTION", false);
FALLBACK_DEF(reworkTrxAborter, "REWORK_TRX_ABORTER", false);
FALLBACK_DEF(ssdsp1511fix, "SSDSP_1511_FIX", false)
FALLBACK_DEF(xformBillingRefactoring, "XFORM_BILLING_REFACTORING", false);
FALLBACK_DEF(fallbackRTPricingContextFix, "FALLBACK_RTPRICING_FIX_FOR_CONTEXT", false);
FALLBACK_DEF(fixDiag817, "FIX_DIAG_817", false)
FALLBACK_DEF(removeDynamicCastForAddonConstruction,
             "REDUCE_DYNCAST_ADDONCONSTRUCTION_COMPONENT",
             false);
FALLBACK_DEF(fallbackNonBSPVcxrPhase1R9, "FALLBACK_NON_BSP_VCXR_PHASE1_R9", false);
FALLBACK_DEF(fallbackPrevFailedFareTuning, "FALLBACK_PREV_FAILED_FARE_TUNING", false);
FALLBACK_DEF(diag23XVarianceFix, "DIAG_23X_VARIANCE_FIX", false);
FALLBACK_DEF(fallbackFRRFixProcessingRetailerCodeR9,
             "FALLBACK_FRR_FIX_PROCESSING_RETAILER_CODE_R9", false);
FALLBACK_DEF(fallbackFixBrandsOriginBasedPricing, "FALLBACK_FIX_BRANDS_ORIGIN_BASED_PRICING", false);
FALLBACK_DEF(fallbackFixCarrierOriginBasedPricing, "FALLBACK_FIX_CARRIER_ORIGIN_BASED_PRICING", false);
FALLBACK_DEF(excDiscountAmountFix, "EXC_DISCOUNT_AMOUNT_FIX", false);
FALLBACK_DEF(fallbackSanitizerError1, "FALLBACK_SANITIZER_ERROR_1", false);
FALLBACK_DEF(fallbackFlexFareGroupXOLogicDeffects, "FALLBACK_FFG_NEW_XO_LOGIC_DEFFECTS", false);
FALLBACK_DEF(exscCat31MergeCalendarRange, "EXSC_CAT31_MERGE_CALENDAR_RANGE", false)
FIXEDFALLBACK_DEF(excDiscountAmountGroupInitFix, "EXC_DISCOUNT_AMOUNT_GROUP_INIT_FIX", false);
FALLBACK_DEF(fixSpanishLargeFamilyForSRFE, "FIX_SPANISH_LARGE_FAMILY_FOR_SRFE", false);
FIXEDFALLBACK_DEF(xrayTaxOTAModelMapTagsParsing, "FALLBACK_TAX_XRAY_TAGS_PARSING_FIXED", false);
FALLBACK_DEF(fallbackFFGAffectsMainFare, "FALLBACK_FFG_AFFECTS_MAIN_FARE", false);
FALLBACK_DEF(fallbackFRROrgChange, "FALLBACK_FRR_MIP_ORG_CHANGE", false);
FALLBACK_DEF(fallbackFRROBFeesFixAsl, "FALLBACK_FRR_OBFEES_FIX_ASL", false);
FALLBACK_DEF(cat9FixMaxTransfers, "CAT9_FIX_MAX_TRANSFERS", false);
FALLBACK_DEF(cat9FixMaxTransfers2, "CAT9_FIX_MAX_TRANSFERS2", false);
FALLBACK_DEF(exactMatchOfDiscountedFares, "EXACT_MATCH_OF_DISCOUNTED_FARES", false);
FALLBACK_DEF(fallbackSoldoutOriginRT, "FALLBACK_SOLDOUT_ORIGIN_RT", false);
FALLBACK_DEF(fallbackFixCrossBrandWithoutParity, "FALLBACK_FIX_CROSS_BRAND_WITHOUT_PARITY", false);
FALLBACK_DEF(sortMatchedExcFaresByDiff, "SORT_MATCHED_EXC_FARES_BY_DIFF", false);
FALLBACK_DEF(srfeSsdsp2040DifferentDiscountTypes, "SRFE_SSDSP2040_DIFFERENT_DISCOUNT_TYPES", false);
FALLBACK_DEF(carrierApplicationOpt, "CARRIER_APPLICATION_OPT", false);
FIXEDFALLBACK_DEF(saveAtpcoTaxDisplayData, "SAVE_ATPCO_TAX_DISPLAY_DATA", false);
FALLBACK_DEF(fallbackTagPY9matchITBTCCPayment, "FALLBACK_TAG_PY9_MATCH_ITBT_CC_PAYMENT", false);
FALLBACK_DEF(fallbackSkipDirectionalityCheckInFQ, "FALLBACK_SKIP_DIRECTIONALITY_CHECK_IN_FQ", false);
FALLBACK_DEF(fallbackProperDirectionalityValidationInFQ, "FALLBACK_PROPER_DIRECTIONALITY_VALIDATION_IN_FQ", false);
FALLBACK_DEF(fallbackRrmCmdPricFixErrorMsg, "FALLBACK_RRM_CMD_PRICING_FIX_ERROR_MSG", false);
FALLBACK_DEF(fallbackPrevFailedFarePairTuning, "FALLBACK_PREV_FAILED_FARE_PAIR_TUNING", false);
FALLBACK_DEF(fallbackFFGCabinLogicDeffect, "FALLBACK_FFG_CABIN_LOGIC_DEFFECT", false);
FALLBACK_DEF(rexCat5APO41906, "REX_CAT5_APO41906", false);
FALLBACK_DEF(fallbackTraditionalValidatingCxr, "VITA_TRADITIONAL_VALIDATING_CXR", false);
FALLBACK_DEF(fallbackIbfNumberOfSoldouts, "FALLBACK_IBF_NUMBER_OF_SOLDOUTS", false);
FIXEDFALLBACK_DEF(checkArunkForSfr, "CHECK_ARUNK_FOR_SFR", true);
FIXEDFALLBACK_DEF(fallbackXrayJsonContainerInTseServer, "XRAY_JSON_CONTAINER_IN_TSE_SERVER", false);
FIXEDFALLBACK_DEF(fixCat33Discount, "FIX_CAT33_DISCOUNT", false)
FALLBACK_DEF(perTicketSurchargeFix, "PER_TICKET_SURCHARGE_FIX", false);
FALLBACK_DEF(applyExcDiscToMatchedRefundFares, "APPLY_EXC_DISC_TO_MATCHED_REFUND_FARES", false);
FALLBACK_DEF(dffOaFareCreation, "DFF_OTHER_AIRLINES_FARE_CREATION", true);
FALLBACK_DEF(createSMFOFaresForALlUsers, "CREATE_SMFO_FARES_FOR_ALL_USERS", true);
FALLBACK_DEF(fallbackVITA4NonBSP, "FALLBACK_VITA_4_NON_BSP", false);
FIXEDFALLBACK_DEF(fallbackWpdfEnhancedRuleDisplay, "FALLBACK_WPDF_ENHANCED_RULE_DISPLAY", false);
FALLBACK_DEF(fallbackFixFQRedstColumnError, "FALLBACK_FIX_FQ_REDST_COLUMN_ERROR", false);
FALLBACK_DEF(fallbackTaxTrxTimeout, "FALLBACK_TAX_TRX_TIMEOUT", false);
FALLBACK_DEF(fallbackCheckLegFixFromEnd, "FALLBACK_CHECK_LEG_FIX_FROM_END", false);
FALLBACK_DEF(fallbackSci1126, "FALLBACK_SCI_1126", false);
FALLBACK_DEF(exscGetSpecificOnd, "EXSC_GET_SPECIFIC_OND", false)
FALLBACK_DEF(allow100PExcDiscounts, "ALLOW_100P_EXC_DISCOUNTS", false);
FALLBACK_DEF(srfeFareBasisSuffix, "SRFE_FARE_BASIS_SUFFIX", false);
FALLBACK_DEF(srfeFareBasisSuffixOldWay, "SRFE_FARE_BASIS_SUFFIX_OLD_WAY", false);
FALLBACK_DEF(fallbckFareMarketMergerRefactoring, "FALLBACK_FARE_MARKET_REFACTORING", false);
FALLBACK_DEF(fallbckFareMarketMergerRefactoring2, "FALLBACK_FARE_MARKET_REFACTORING2", false);
FALLBACK_DEF(fallbackOneSecondHurryOut, "FALLBACK_ONE_SECOND_HURRY_OUT", false);
FALLBACK_DEF(fallbackSortItinsByNumFlights, "FALLBACK_SORT_ITINS_BY_NUM_FLIGHTS", false);
FALLBACK_DEF(AF_CAT33_ATPCO, "AF_CAT33_ATPCO", false)
FALLBACK_DEF(AF_CAT33_TaxRequestBuilder, "AF_CAT33_TaxRequestBuilder", false)
FALLBACK_DEF(fallbackDisableCrossBrandInOneWay, "DISABLE_CROSS_BRAND_IN_ONE_WAY", false);
FALLBACK_DEF(exscDefaultWholePeriodRange, "EXSC_DEFAULT_WHOLE_PERIOD_RANGE", false)
FALLBACK_DEF(checkNumberOfSegments, "FALLBACK_CHECK_NUMBER_OF_SEGMENTS", false);
FALLBACK_DEF(exscSortOADResponse, "EXSC_SORT_OAD_RESPONSE", false);
FALLBACK_DEF(excDiscountsFixDivideByZero, "EXC_DISCOUNTS_FIX_DIVIDE_BY_ZERO", false);
FALLBACK_DEF(apo45023ApplyCat2DefaultsInOOJPU, "FALLBACK_APO45023_APPLYCAT2_DEFAULTS_OOJ_ITIN", false);
FALLBACK_DEF(treatTrsAsPlane, "FALLBACK_TREAT_TRS_AS_PLANE", false);
FALLBACK_DEF(fallbackAPO45852ExcInItin, "FALLBACK_APO_45852_EXCEPTION_IN_ITIN", false);
FALLBACK_DEF(fallbackSegFromGriRemoval, "FALLBACK_SEG_FROM_GRI_REMOVAL", false);
FALLBACK_DEF(getCat33ChangeOnFeeFromExcItin, "GET_CAT33_CHANGE_ON_FEE_FROM_EXC_ITIN", false);
FALLBACK_DEF(fallbackFFGMaxPenaltyNewLogic, "FALLBACK_FFG_MAX_PENALTY_NEW_LOGIC", false);
FALLBACK_DEF(fallbackFFGroupIdTypeChanges, "FALLBACK_FF_GROUP_ID_TYPE_CHANGES", false);
FIXEDFALLBACK_DEF(fallbackHandlingFee, "FALLBACK_HANDLING_FEE", false);
FALLBACK_DEF(excPrivatePublicOpt, "EXC_PRIVATE_PUBLIC_OPT", false);
FALLBACK_DEF(excPrivPubNoAtpFares, "EXC_PRIV_PUB_NO_ATP_FARES", false);
FALLBACK_DEF(dontFilterFailedTaxesFor817Diag, "FALLBACK_FILTER_FAILED_TAXES_817_DIAG", false);
FALLBACK_DEF(fallbackAB240SupportInvalidPaxType, "FALLBACK_AB240_SUPPORT_INVALID_PAX_TYPE", false);
FALLBACK_DEF(displayTaxInPaymentPrecision, "FALLBACK_DISPLAY_TAX_PAYMENT_PRECISION", false);
FALLBACK_DEF(taxFareWithoutBase, "FALLBACK_TAX_FARE_WITHOUT_BASE", false);
FALLBACK_DEF(fallbackSendDirectionToMM, "FALLBACK_SEND_DIRECTION_TO_MM", false);
FALLBACK_DEF(fallbackUseOnlyFirstProgram, "FALLBACK_USE_ONLY_FIRST_PROGRAM", false);
FALLBACK_DEF(fallbackUseFareUsageDirection, "FALLBACK_USE_FAREUSAGE_DIRECTION", false);
FALLBACK_DEF(fallbackPFCOvercollectionInRTItinMax4, "FALLBACK_APO41311_PFC_OVERCOLLECTION", false);
FALLBACK_DEF(throughFarePrecedenceStopoverFix, "THROUGH_FARE_PRECEDENCE_STOPOVER_FIX", false);
FALLBACK_DEF(fallbackMoveGetCabinToDSS, "FALLBACK_MOVE_GET_CABIN_TO_DSS", false);
FALLBACK_DEF(fallbackAgencyRetailerHandlingFees,"FALLBACK_AGENCY_RETAILER_HANDLING_FEES", false);
FALLBACK_DEF(roundTaxToZero, "FALLBACK_ROUND_TAX_TO_ZERO", false);
FALLBACK_DEF(fallbackPFFAffectsMainFare, "FALLBACK_PFF_AFFECTS_MAIN_FARE", false);
FALLBACK_DEF(exscDiag989FixOndDisplay, "EXSC_DIAG989_FIX_OND_DISPLAY", false)
FALLBACK_DEF(exscSetEmptyDateRangeAsWholePeriod, "EXSC_SET_EMPTY_DATE_RANGE_AS_WHOLE_PERIOD", false)
FALLBACK_DEF(cat31ChangeFinderOffByOne, "CAT31_CHANGE_FINDER_OFF_BY_ONE", false)
} // tse
