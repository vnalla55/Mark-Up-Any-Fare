///----------------------------------------------------------------------------
//
//  File:           TrxUtil.cpp
//  Created:        4/7/2004
//  Authors:
//
//  Description:    Common functions required for ATSE shopping/pricing.
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

#include "Common/TrxUtil.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Config/DynamicConfigurableDate.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/DynamicConfigurableNumber.h"
#include "Common/Config/DynamicConfigurableString.h"
#include "Common/ConfigList.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/CsoPricingTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Trx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Cat05OverrideCarrier.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/InterlineCarrierInfo.h"
#include "DBAccess/IntralineCarrierInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxText.h"
#include "Util/Algorithm/Container.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FIXEDFALLBACK_DECL(repricingForTaxShopping);
FALLBACK_DECL(ssdsp1788SanitizerErrorFix)
FALLBACK_DECL(logRequestWhenLowMemory);
FALLBACK_DECL(unifyMemoryAborter)

namespace
{
Logger
logger("atseintl.Common.TrxUtil");
Logger
loggerLowMemory("atseintl.LowMemoryTransactions");
ConfigurableValue<bool>
minfareValDiscountRule("PRICING_SVC", "MINFARE_VALIDATE_DISCOUNT_RULE", false);
ConfigurableValue<bool>
rexExcCat31YYEnabled("FARESC_SVC", "REX_EXC_CAT31_YY_ENABLED", false);
ConfigurableValue<bool>
rexExcCat33YYEnabled("FARESC_SVC", "REX_EXC_CAT33_YY_ENABLED", false);
ConfigurableValue<bool>
reuseFarePuResultPricing("RULES_OPTIONS", "REUSE_FARE_PU_RESULT_PRICING", false);
ConfigurableValue<bool>
skipCat33AirlineSecurityCheck("REX_FARE_SELECTOR_SVC", "SKIP_CAT_33_AIRLINE_SECURITY_CHECK", false);
ConfigurableValue<bool>
skipCat33SubscriberSecurityCheck("REX_FARE_SELECTOR_SVC",
                                 "SKIP_CAT_33_SUBSCRIBER_SECURITY_CHECK",
                                 false);
ConfigurableValue<bool>
skipCat31AirlineSecurityCheck("REX_FARE_SELECTOR_SVC", "SKIP_CAT_31_AIRLINE_SECURITY_CHECK", false);
ConfigurableValue<bool>
skipCat31SubscriberSecurityCheck("REX_FARE_SELECTOR_SVC",
                                 "SKIP_CAT_31_SUBSCRIBER_SECURITY_CHECK",
                                 false);
ConfigurableValue<bool>
adjustRexTravelDate("REX_FARE_SELECTOR_SVC", "ADJUST_REX_TRAVEL_DATE", false);
ConfigurableValue<ConfigList<CarrierCode, VectorPolicy<CarrierCode>>>
mileageCarriers("PRICING_SVC", "JOURNEY_MILEAGE_CARRIERS");
ConfigurableValue<bool>
fvoSurchargesEnabled("RULES_OPTIONS", "FVO_SURCHARGES_ENABLED", false);
ConfigurableValue<bool>
prevalidateRecord18PrimeBkgCode("SHOPPING_OPT", "PREVALIDATE_RECORD1B_PRIME_BKG_CODE", false);
ConfigurableValue<bool>
useBookingCodeExceptionIndex("FARESV_SVC", "USE_BOOKINGCODEEXCEPTION_INDEX", false);
ConfigurableValue<ConfigList<CarrierCode, VectorPolicy<CarrierCode>>>
interlineAvailabilityCarriersCfg("SHOPPING_OPT", "INTERLINE_AVAILABILITY_CARRIERS");
ConfigurableValue<ConfigList<std::string, VectorPolicy<std::string>>>
metricsInResponse("TSE_SERVER", "METRICS_IN_RESPONSE");
ConfigurableValue<ConfigSet<std::string>>
isJcbCarrierCfg("SHOPPING_SVC", "IS_JCB_CARRIER");
ConfigurableValue<std::string>
enableTaxEnhancementDate("SHOPPING_OPT", "ENABLE_TAX_ENHANCEMENT_DATE");
ConfigurableValue<int32_t>
tcsTimeout("TSE_SERVER", "TCS_TIMEOUT");
ConfigurableValue<int32_t>
trxTimeoutCfg("TSE_SERVER", "TRX_TIMEOUT");
ConfigurableValue<uint32_t>
pricingOptionMaxLimit("TICKETING_FEE_OPT_MAX", "PRICING_OPTION_MAX_LIMIT", 50);
}

FALLBACK_DECL(yqyrADTotalPricing);
FALLBACK_DECL(fallbackGSAMipDifferentialFareFix);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackInterlineIgnorePartners);
FALLBACK_DECL(azPlusUp);

CurrencyCode TrxUtil::errorCCode = EMPTY_STRING();
DateTime TrxUtil::today = DateTime::localTime();
bool TrxUtil::abacusEnabled = false;

std::map<CarrierCode, std::vector<CarrierCode>> TrxUtil::interlineAvailabilityCxrs;
std::atomic<bool> TrxUtil::interlineAvailabilityCxrsConfigured;

std::map<std::string, std::vector<CarrierCode>> TrxUtil::intralineAvailabilityCxrs;
std::atomic<bool> TrxUtil::intralineAvailabilityCxrsConfigured;

boost::mutex TrxUtil::_mutex;

DateTime TrxUtil::_taxEnhancementActivationDate = DateTime(2025, 04, 01);
volatile bool TrxUtil::_taxEnhancementActivationDateConfigured = false;

std::vector<std::string> TrxUtil::_recordingMetricsDescription;
volatile bool TrxUtil::_recordingMetricsConfigured = false;

namespace
{
DateTime
defaultDate(2050, 1, 1);
std::string
emptyString("");

ConfigurableValue<bool>
FVOSurchargesEnabledConf("RULES_OPTIONS", "FVO_SURCHARGES_ENABLED", false);

const std::string TO_SVC = "TO_SVC", PRICING_SVC = "PRICING_SVC", FARESC_SVC = "FARESC_SVC",
                  REX_FARE_SELECTOR_SVC = "REX_FARE_SELECTOR_SVC",
                  SERVICE_FEES_SVC = "SERVICE_FEES_SVC", SHOPPING_OPT = "SHOPPING_OPT",
                  FREE_BAGGAGE = "FREE_BAGGAGE", TSE_SERVER = "TSE_SERVER",
                  RULECATEGORY = "RULECATEGORY", TAX_SVC = "TAX_SVC",
                  FAREDISPLAY_SVC = "FAREDISPLAY_SVC", RULES_OPTIONS = "RULES_OPTIONS",
                  S8_BRAND_SVC = "S8_BRAND_SVC", FARE_CALC_SVC = "FARE_CALC_SVC",
                  SHOPPING_SVC = "SHOPPING_SVC", COMMON_FUNCTIONAL = "COMMON_FUNCTIONAL",
                  EMD_ACTIVATION = "EMD_ACTIVATION";

DynamicConfigurableEffectiveDate
iata302BaggageActivationDate(FREE_BAGGAGE, "IATA_302_BAGGAGE_ACTIVATION_DATE", defaultDate),
    iata302BaggageActivationCarrierDate(FREE_BAGGAGE,
                                        "IATA_302_BAGGAGE_ACTIVATION_CARRIER_DATE",
                                        defaultDate),
    baggage302DotActivationDate(FREE_BAGGAGE, "BAGGAGE_302_DOT_ACTIVATION_DATE", defaultDate),
    baggage302DotActivationCarrierDate(FREE_BAGGAGE,
                                       "BAGGAGE_302_DOT_ACTIVATION_CARRIER_DATE",
                                       defaultDate),
    baggage302DotActivationAxDate(FREE_BAGGAGE, "BAGGAGE_302_DOT_ACTIVATION_AX_DATE", defaultDate),
    baggage302DotActivationAbDate(FREE_BAGGAGE, "BAGGAGE_302_DOT_ACTIVATION_AB_DATE", defaultDate),
    baggage302DotActivationInDate(FREE_BAGGAGE, "BAGGAGE_302_DOT_ACTIVATION_IN_DATE", defaultDate),
    baggage302NewUSDotMethodActivationDate(FREE_BAGGAGE,
                                           "BAGGAGE_302_NEW_USDOT_METHOD_ACTIVATION_DATE",
                                           defaultDate),
    baggageCTAMandateActivationDate(FREE_BAGGAGE,
                                    "BAGGAGE_CTA_MANDATE_ACTIVATION_DATE",
                                    defaultDate),
    iataReso302MandateActivationDate(FREE_BAGGAGE, "IATA_RESO_302_MANDATE_DATE", defaultDate),
    fqS8BrandedServiceActivationDate(S8_BRAND_SVC, "FQ_ACTIVATION_DATE", defaultDate),
    pricingS8BrandedServiceActivationDate(S8_BRAND_SVC, "PRICING_ACTIVATION_DATE", defaultDate),
    pricingInfiniCat05ValidationSkipDate(PRICING_SVC,
                                         "INFINI_CAT05_OVERRIDE_BKG_ACTIVATION_DATE",
                                         defaultDate),
    fullMapRoutingActivationDate(PRICING_SVC, "FULL_MAP_ROUTING_ACTIVATION_DATE", defaultDate),
    iataFareSelectionActivationDate(PRICING_SVC,
                                    "IATA_FARE_SELECTION_ACTIVATION_DATE",
                                    defaultDate),
    atpcoRbdByCabinAnswerTableActivationDate(PRICING_SVC,
                                             "ATPCO_RBDBYCABIN_ANSWER_TABLE_ACTIVATION_DATE",
                                             defaultDate),
    taxesNewHeaderActivationASDate(TAX_SVC,
                                   "ENHANCE_WP_PQ_TAXES_HEADER_AS_PROJECT_ACTIV_DATE",
                                   defaultDate),
    taxExemptionForChildActivationDate(TAX_SVC,
                                       "TAX_EXEMPTION_FOR_CHILD_ACTIVATION_DATE",
                                       defaultDate),
    specialOpenJawActivationDate(PRICING_SVC, "SPECIAL_OPEN_JAW_ACTIVATION_DATE", defaultDate),
    icerActivationDate(COMMON_FUNCTIONAL, "ICER_ACTIVATION_DATE", defaultDate),
    ffTierActivationDate(PRICING_SVC, "FF_TIER_ACTIVATION_DATE", defaultDate),
    ffTierOBActivationDate(PRICING_SVC, "FF_TIER_OB_ACTIVATION_DATE", defaultDate),
    emdValidationForM70(EMD_ACTIVATION, "EMD_VALIDATION_FOR_M70", defaultDate),
    emdValidationOnReservationPathForAB240(EMD_ACTIVATION,
                                           "EMD_VALIDATION_ON_RESERVATION_PATH_FOR_AB240",
                                           defaultDate),
    emdValidationFlightRelatedOnCheckingPathForAB240(
        EMD_ACTIVATION, "EMD_VALIDATION_FLIGHT_RELATED_ON_CHECKIN_PATH_FOR_AB240", defaultDate),
    emdValidationFlightRelatedServiceAndPrepaidBaggage(
        EMD_ACTIVATION, "EMD_VALIDATION_FLIGHT_RELATED_SERVICE_AND_PREPAID_BAGGAGE", defaultDate),
    emdValidationInterlineChargeChecks(EMD_ACTIVATION,
                                       "EMD_VALIDATION_INERLINE_CHARGE_CHECKS",
                                       defaultDate),
    emdValidationRelaxed(EMD_ACTIVATION, "EMD_VALIDATION_ENABLE_RELAXED", defaultDate);

DynamicConfigurableFlagOn
usAirAncillaryEnabled(SERVICE_FEES_SVC, "USAIR_ANCILLARY_SEAT_ACTIVATION", false),
    prepaidBaggageEnabled(SERVICE_FEES_SVC, "PREPAID_BAGGAGE_ENABLED", false),
    eMDSPhase2Enabled(SERVICE_FEES_SVC, "ANCILLARY_EMDS_PHASE_2_ENABLED", false),
    optimusNetRemitActivated(PRICING_SVC, "ACTIVATE_OPTIMUS_NET_REMIT", false),
    cmdPricingTuningActivated(PRICING_SVC, "CMD_PRICING_TUNING", false),
    multiTicketPricingEnabled(PRICING_SVC, "ENABLE_MULTI_TKT_PRICING", false),
    supplementChargeEnabled(FAREDISPLAY_SVC, "SUPPLEMENTAL_FEE_ACTIVATION", false),
    atpcoTTProjectHighRTEnabled(PRICING_SVC, "ATPCO_TT_PROJECT_HIGH_RT_ENABLED", false),
    dnataEnabled(PRICING_SVC, "DNATA_ENABLED", false),
    markupSecFilterEnhancementEnabled(PRICING_SVC, "MARKUP_SEC_FILTER_ENHANCEMENT_ENABLED", false),
    baggageDotPhase2ADisplayTnEnabled(FREE_BAGGAGE, "BAGGAGE_DOT_PHASE2A_DISPLAY_TN_ENABLED", true),
    baggageDotPhase2ADisplayCarriersEnabled(FREE_BAGGAGE,
                                            "BAGGAGE_DOT_PHASE2A_DISPLAY_CARRIERS_ENABLED",
                                            true),
    baggageDotPhase2ADisplayAxEnabled(FREE_BAGGAGE,
                                      "BAGGAGE_DOT_PHASE2A_DISPLAY_AXESS_ENABLED",
                                      true),
    baggageDotPhase2ADisplayAbEnabled(FREE_BAGGAGE,
                                      "BAGGAGE_DOT_PHASE2A_DISPLAY_ABACUS_ENABLED",
                                      true),
    baggageDotPhase2ADisplayInEnabled(FREE_BAGGAGE,
                                      "BAGGAGE_DOT_PHASE2A_DISPLAY_INFINI_ENABLED",
                                      true),
    baggageCarryOnActivationAxEnabled(FREE_BAGGAGE, "BAGGAGE_CARRY_ON_ACTIVATION_AX_ENABLED", true),
    baggageCarryOnActivationAbEnabled(FREE_BAGGAGE, "BAGGAGE_CARRY_ON_ACTIVATION_AB_ENABLED", true),
    baggageCarryOnActivationInEnabled(FREE_BAGGAGE, "BAGGAGE_CARRY_ON_ACTIVATION_IN_ENABLED", true),
    baggageCarryOnActivationTnEnabled(FREE_BAGGAGE, "BAGGAGE_CARRY_ON_ACTIVATION_TN_ENABLED", true),
    baggageCarryOnActivationAAEnabled(FREE_BAGGAGE, "BAGGAGE_CARRY_ON_ACTIVATION_AA_ENABLED", true),
    baggageCarryOnActivationAirlineEnabled(FREE_BAGGAGE,
                                           "BAGGAGE_CARRY_ON_ACTIVATION_AIRLINE_ENABLED",
                                           true),
    baggageExchangeActivationAbEnabled(FREE_BAGGAGE,
                                       "BAGGAGE_EXCHANGE_ACTIVATION_AB_ENABLED",
                                       true),
    baggageExchangeActivationAxEnabled(FREE_BAGGAGE,
                                       "BAGGAGE_EXCHANGE_ACTIVATION_AX_ENABLED",
                                       true),
    baggageExchangeActivationInEnabled(FREE_BAGGAGE,
                                       "BAGGAGE_EXCHANGE_ACTIVATION_IN_ENABLED",
                                       true),
    baggageExchangeActivationTnEnabled(FREE_BAGGAGE,
                                       "BAGGAGE_EXCHANGE_ACTIVATION_TN_ENABLED",
                                       true),
    baggageExchangeActivationAirlineEnabled(FREE_BAGGAGE,
                                            "BAGGAGE_EXCHANGE_ACTIVATION_AIRLINE_ENABLED",
                                            true),
    baggageActivationXmlResponseEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_ACTIVATION_XML_RESPONSE_ENABLED",
                                        true),
    baggageEmbargoesActivationAxEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_EMBARGOES_ACTIVATION_AX_ENABLED",
                                        true),
    baggageEmbargoesActivationAbEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_EMBARGOES_ACTIVATION_AB_ENABLED",
                                        true),
    baggageEmbargoesActivationInEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_EMBARGOES_ACTIVATION_IN_ENABLED",
                                        true),
    baggageEmbargoesActivationTnEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_EMBARGOES_ACTIVATION_TN_ENABLED",
                                        true),
    baggageEmbargoesActivationAAEnabled(FREE_BAGGAGE,
                                        "BAGGAGE_EMBARGOES_ACTIVATION_AA_ENABLED",
                                        true),
    baggageEmbargoesActivationAirlineEnabled(FREE_BAGGAGE,
                                             "BAGGAGE_EMBARGOES_ACTIVATION_AIRLINE_ENABLED",
                                             true),
    baggageGlobalDisclosureActivationAxEnabled(FREE_BAGGAGE,
                                               "BAGGAGE_GLOBAL_DISCLOSURE_AX_ENABLED",
                                               true),
    baggageGlobalDisclosureActivationAbEnabled(FREE_BAGGAGE,
                                               "BAGGAGE_GLOBAL_DISCLOSURE_AB_ENABLED",
                                               true),
    baggageGlobalDisclosureActivationInEnabled(FREE_BAGGAGE,
                                               "BAGGAGE_GLOBAL_DISCLOSURE_IN_ENABLED",
                                               true),
    baggageGlobalDisclosureActivationTnEnabled(FREE_BAGGAGE,
                                               "BAGGAGE_GLOBAL_DISCLOSURE_TN_ENABLED",
                                               true),
    baggageGlobalDisclosureActivationAAEnabled(FREE_BAGGAGE,
                                               "BAGGAGE_GLOBAL_DISCLOSURE_ACTIVATION_AA_ENABLED",
                                               true),
    baggageGlobalDisclosureActivationAirlineEnabled(FREE_BAGGAGE,
                                                    "BAGGAGE_GLOBAL_DISCLOSURE_AIRLINE_ENABLED",
                                                    true),
    baggageBTAActivationAxEnabled(FREE_BAGGAGE,
                                  "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_AX_ENABLED",
                                  false),
    baggageBTAActivationAbEnabled(FREE_BAGGAGE,
                                  "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_AB_ENABLED",
                                  false),
    baggageBTAActivationInEnabled(FREE_BAGGAGE,
                                  "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_IN_ENABLED",
                                  false),
    baggageBTAActivationTnEnabled(FREE_BAGGAGE,
                                  "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_TN_ENABLED",
                                  false),
    baggageBTAActivationAAEnabled(FREE_BAGGAGE,
                                  "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_AA_ENABLED",
                                  false),
    baggageBTAActivationAirlineEnabled(FREE_BAGGAGE,
                                       "BAGGAGE_TRAVEL_APPLICATION_ACTIVATION_AIRLINE_ENABLED",
                                       false),
    fareSelectionForSubIata21Only(SHOPPING_OPT, "FARE_SELECTION_FOR_SUB_IATA_21_ONLY", false),
    fareSelectionForNGSThruFMOnly(SHOPPING_OPT, "FARE_SELECTION_FOR_NGS_THRU_FM_ONLY", false),
    rbdByCabinValueInShopResponse(SHOPPING_OPT,
                                  "RBDBYCABIN_ANSWER_TABLE_IN_SHOPPING_RESPONSE",
                                  false),
    rbdByCabinValueInPriceResponse(PRICING_SVC,
                                   "RBDBYCABIN_ANSWER_TABLE_IN_PRICING_RESPONSE",
                                   false),
    project331PlusDaysEnabled(PRICING_SVC, "PROJECT_331_PLUS_DAYS_ENABLED", false),
    hpsTaxDomGroupingEnabled(TAX_SVC, "HPS_DOM_GROUPING_ENABLED", false),
    hpsTaxIntlGroupingEnabled(TAX_SVC, "HPS_INTL_GROUPING_ENABLED", false),
    hpsTaxUseFlightRanges(TAX_SVC, "HPS_USE_FLIGHT_RANGES", false),
    atpcoTaxesOnOcEnabled(TAX_SVC, "ATPCO_TAXES_ON_OC_ENABLED", false),
    atpcoTaxesOnBaggageEnabled(TAX_SVC, "ATPCO_TAXES_ON_BAGGAGE_ENABLED", false),
    atpcoTaxesOnItinEnabled(TAX_SVC, "ATPCO_TAXES_ON_ITIN_ENABLED", false),
    atpcoTaxesOnItinServiceFlag(TAX_SVC, "ATPCO_TAXES_ON_ITIN_SERVICE_FLAG", false),
    atpcoTaxesOnChangeFeeServiceFlag(TAX_SVC, "ATPCO_TAXES_ON_CHANGEFEE_SERVICE_FLAG", false),
    atpcoBackingOutTaxesEnabled(TAX_SVC, "ATPCO_BACKING_OUT_TAXES_ENABLED", false),
    atpcoTaxesOnChangeFeeEnabled(TAX_SVC, "ATPCO_TAXES_ONCHANGEFEE_ENABLED", false),
    atpcoTaxesDisplayEnabled(TAX_SVC, "ATPCO_TAXES_DISPLAY_ENABLED", false),
    atpcoTaxesDefaultRoundingEnabled(TAX_SVC, "ATPCO_TAXES_DEFAULT_ROUNDING_ENABLED", false),
    totalPriceEnabled(PRICING_SVC, "TOTAL_PRICE_ENABLED", false),
    totalPriceAllowed(PRICING_SVC, "TOTAL_PRICE_ALLOWED", true),
    dynamicConfigOverrideEnabled(TSE_SERVER, "DYNAMIC_CONFIG_OVERRIDE_ENABLED", false),
    dynamicConfigOverridePermament(TSE_SERVER, "DYNAMIC_CONFIG_OVERRIDE_PERMAMENT", false),
    jumpCabinLogicDisableCompletely(TSE_SERVER, "JUMP_CABIN_LOGIC_DISABLE_COMPLETELY", true),
    fqFareRetailerEnabled(TSE_SERVER, "FQ_FARE_RETAILER_ENABLED", true),
    splitTaxesByFareComponentEnabled(TAX_SVC, "SPLIT_TAXES_BY_FARE_COMPONENT_ENABLED", false),
    baggageChargesInMip(FREE_BAGGAGE, "BAGGAGE_CHARGES_IN_MIP", false),
    baggageInPQEnabled(FREE_BAGGAGE, "BAGGAGE_IN_PQ", false),
    automatedRefundCat33Enabled(TAX_SVC, "AUTOMATED_REFUND_CAT33_ENABLED", false),
    abacusEndorsementHierarchyAtpcoFares(TSE_SERVER,
                                         "ABACUS_ENDORSEMENT_HIERARCHY_ATPCO_FARES",
                                         true),
    infiniEndorsementHierarchyAtpcoFares(TSE_SERVER,
                                         "INFINI_ENDORSEMENT_HIERARCHY_ATPCO_FARES",
                                         true),
    dateAdjustmentIndicatorActive(TSE_SERVER, "DATE_ADJUSTMENT_INDICATOR_ACTIVE", true),
    controlConnectIndicatorActive(TSE_SERVER, "CONTROL_CONNECT_INDICATOR_ACTIVE", true);

DynamicConfigurableNumber
memCheckIntervalConfig(PRICING_SVC, "MEM_CHECK_INTERVAL", 0),
    permCheckIntervalConfig(REX_FARE_SELECTOR_SVC, "PERM_CHECK_INTERVAL", 100),
    referenceRSSConfig(PRICING_SVC, "REFERENCE_RSS", 0),
    referenceVMConfig(PRICING_SVC, "REFERENCE_VM", 0),
    maxRSSGrowthConfig(PRICING_SVC, "MAX_RSS_GROWTH", 0),
    maxVMGrowthConfig(PRICING_SVC, "MAX_VM_GROWTH", 0),
    abortCheckIntervalConfig(SHOPPING_SVC, "ABORT_CHECK_INTERVAL", 0),
    maxCat31SolutionCountConfig(REX_FARE_SELECTOR_SVC, "MAX_CAT31_SOLUTION_COUNT", 0),
    mipoTuningItinNumberConfig(RULES_OPTIONS, "MIPO_TUNING_ITIN_NUMBER_COUNT", 8),
    FVOSurchargeOnNumFm(RULES_OPTIONS, "FVO_SURCHARGE_ON_NUM_FM", 2),
    FVOSurchargeOnNumPtf(RULES_OPTIONS, "FVO_SURCHARGE_ON_NUM_PTF", 10000),
    yqyrPrecalcLimitConfig(PRICING_SVC, "YQYR_PRECALC_LIMIT", 100000000),
    maxNumberOfSolutions(SHOPPING_SVC, "MAX_NUMBER_OF_SOLUTIONS_ALT_DATE", 9),
    rec2Cat35SegCountLimitConfig(FARESC_SVC, "REC2_CAT35_SEGCOUNT_LIMIT", 0),
    ASBaseTaxEquivTotalLengthConfig(FARE_CALC_SVC, "AS_AMOUNT_LENGTH", 11);

DynamicConfigurableNumber
minAvailableMemoryTrx(PRICING_SVC, "MIN_AVAILABLE_MEMORY_TRX", 0);
DynamicConfigurableNumber
maxRSSPercentageMemoryTrx(PRICING_SVC, "MAX_RSS_PERCENTAGE_TRX", 100);
DynamicConfigurableNumber
memCheckTrxIntervalConfig(PRICING_SVC, "MEM_CHECK_TRX_INTERVAL", 0);

DynamicConfigurableNumber
memShoppingCheckInterval(SHOPPING_SVC, "SHOPPING_FVO_MEM_CHECK_INTERVAL_ALT_DATE", 100);
DynamicConfigurableNumber
perMemGrowthShoppingCheckInterval(SHOPPING_SVC,
                                  "SHOPPING_FVO_MEM_PERCENTAGE_GROWTH_CHECK_INTERVAL_ALT_DATE",
                                  100);

DynamicConfigurableString
supplementChargeCarrierList(FAREDISPLAY_SVC, "SUPPLEMENT_CHARGE_CARRIERS", emptyString),
    supplementHdrMsgFareDisplay(FAREDISPLAY_SVC, "SUPPLEMENT_CHARGE_HDR_MSG_FD", emptyString),
    subCodeDefinitions(SERVICE_FEES_SVC, "SUB_CODE_DEFINITIONS", emptyString),
    gtcCarrierList(PRICING_SVC, "GTC_CARRIERS", emptyString),
    amChargesTaxesList(SERVICE_FEES_SVC, "AM_CHARGES_TAXES", emptyString);

DynamicConfigurableEffectiveDate
diffCntyNMLOpenJawActivationDate(PRICING_SVC, "DIFFCNTY_NML_OPENJAW_ACTIVATION_DATE", defaultDate);

DynamicConfigurableString
dynamicConfigOverrideBSResponse("DEBUG_INFO", "OVERRIDE_BRANDING_SERVICE_RESPONSE", "N");

DynamicConfigurableNumber
maxPenaltyFailedFaresThreshold(PRICING_SVC, "MAX_PENALTY_FAILED_FARES_THRESHOLD", 0);

} // namespace

std::string
TrxUtil::getPCC(const PricingTrx& trx)
{
  const Billing* billing = trx.billing();
  if (LIKELY(billing != nullptr))
    return billing->aaaCity();

  return getPCCFromReq(trx);
}

std::string
TrxUtil::getPCCFromReq(const PricingTrx& trx)
{
  // If we didnt have a Billing record, try to get
  // the PCC from the Agent record
  const PricingRequest* pr = trx.getRequest();
  if (pr == nullptr)
    return EMPTY_STRING();

  const Agent* agt = pr->ticketingAgent();
  if (agt == nullptr)
    return EMPTY_STRING();

  // Use the PCC if set, otherwise the city
  if (!agt->tvlAgencyPCC().empty())
    return agt->tvlAgencyPCC();
  else
    return agt->agentCity();
}

const std::string&
TrxUtil::getHostCarrier(const PricingTrx& trx)
{
  const Billing* billing = trx.billing();
  if (billing != 0)
    return billing->partitionID();

  const PricingRequest* pr = trx.getRequest();
  if (pr == nullptr)
    return EMPTY_STRING();

  const Agent* agt = pr->ticketingAgent();
  return (agt ? agt->vendorCrsCode() : EMPTY_STRING());
}

const std::string&
TrxUtil::parentServiceName(const PricingTrx& trx)
{
  const Billing* billing = trx.billing();
  return billing ? billing->parentServiceName() : EMPTY_STRING();
}

const std::string&
TrxUtil::getHomeAgencyIATA(PricingTrx& trx)
{
  const PricingRequest* pr = trx.getRequest();
  if (pr == nullptr)
    return EMPTY_STRING();

  const Agent* agt = pr->ticketingAgent();
  return (agt ? agt->homeAgencyIATA() : EMPTY_STRING());
}

//----------------------------------------------------------------------------
// getFareMarket() by matching governing carrier and all travel segments to
// handle side trips
//----------------------------------------------------------------------------
FareMarket*
TrxUtil::getFareMarket(const PricingTrx& trx,
                       const CarrierCode carrier,
                       const std::vector<TravelSeg*>& travelSegs,
                       const DateTime& date,
                       const Itin* itin)
{
  const size_t numTravelSegs = travelSegs.size();
  bool isRexNewItinTrx = false;

  const Itin* currentItin = nullptr;
  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
               static_cast<const RexPricingTrx&>(trx).trxPhase() ==
                   RexPricingTrx::PRICE_NEWITIN_PHASE))
  {
    isRexNewItinTrx = true;
    currentItin = static_cast<const RexPricingTrx&>(trx).curNewItin();

    if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() == "WFR")
    {
      currentItin = itin;
    }
  }

  const std::vector<FareMarket*>& fareMarket =
      (isRexNewItinTrx && currentItin) ? currentItin->fareMarket() : trx.fareMarket();

  std::vector<FareMarket*>::const_iterator fareMarketI;
  for (fareMarketI = fareMarket.begin(); fareMarketI != fareMarket.end(); fareMarketI++)
  {
    if (UNLIKELY(isRexNewItinTrx && date != (*fareMarketI)->retrievalDate()))
      continue;

    if (carrier != (*fareMarketI)->governingCarrier())
      continue;

    const std::vector<TravelSeg*>& fmTravelSegs = (*fareMarketI)->travelSeg();
    if (fmTravelSegs.size() != numTravelSegs)
      continue;

    bool sameSeg = true;
    std::vector<TravelSeg*>::const_iterator fmTravelSegIter = fmTravelSegs.begin();
    std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
    for (; sameSeg && fmTravelSegIter != fmTravelSegs.end() && travelSegIter != travelSegs.end();
         fmTravelSegIter++, travelSegIter++)
    {
      if (*fmTravelSegIter != *travelSegIter)
        sameSeg = false;

      if ((sameSeg == false) && (itin != nullptr) && (trx.getTrxType() == PricingTrx::MIP_TRX))
      {
        int segOrderItin = itin->segmentOrder(*travelSegIter);
        int segOrderFM = itin->segmentOrder(*fmTravelSegIter);
        if (segOrderItin == segOrderFM)
          sameSeg = true;
      }
    }

    if (sameSeg && fmTravelSegIter == fmTravelSegs.end() && travelSegIter == travelSegs.end())
      return *fareMarketI;
  }

  return nullptr;
}

//----------------------------------------------------------------------------
// getFareMarket() by only matching all travel segments to handle side trips
// and multiple governing carriers
//----------------------------------------------------------------------------
void
TrxUtil::getFareMarket(const PricingTrx& trx,
                       const std::vector<TravelSeg*>& travelSegs,
                       const DateTime& date,
                       std::vector<FareMarket*>& retFareMarket,
                       const Itin* itin)
{
  const size_t numTravelSegs = travelSegs.size();

  std::vector<FareMarket*>::const_iterator fareMarketI;

  bool isRexNewItinTrx = false;
  bool isExchangeShoppingNewItinTrx = false;

  if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
               static_cast<const RexPricingTrx&>(trx).trxPhase() ==
                   RexPricingTrx::PRICE_NEWITIN_PHASE))
  {
    isRexNewItinTrx = true;
    if (itin && trx.getTrxType() == PricingTrx::MIP_TRX && trx.billing()->actionCode() == "WFR")
    {
      isExchangeShoppingNewItinTrx = true;
    }
  }

  const std::vector<FareMarket*>& fareMarket =
      (isRexNewItinTrx
           ? (isExchangeShoppingNewItinTrx
                  ? itin->fareMarket()
                  : static_cast<const RexPricingTrx&>(trx).newItin().front()->fareMarket())
           : trx.fareMarket());

  for (fareMarketI = fareMarket.begin(); fareMarketI != fareMarket.end(); fareMarketI++)
  {
    if (UNLIKELY(isRexNewItinTrx && date != (*fareMarketI)->retrievalDate()))
      continue;

    const std::vector<TravelSeg*>& fmTravelSegs = (*fareMarketI)->travelSeg();
    if (fmTravelSegs.size() != numTravelSegs)
      continue;

    bool sameSeg = true;
    std::vector<TravelSeg*>::const_iterator fmTravelSegIter = fmTravelSegs.begin();
    std::vector<TravelSeg*>::const_iterator travelSegIter = travelSegs.begin();
    for (; sameSeg && fmTravelSegIter != fmTravelSegs.end() && travelSegIter != travelSegs.end();
         fmTravelSegIter++, travelSegIter++)
    {
      if (*fmTravelSegIter != *travelSegIter)
        sameSeg = false;
    }

    if (sameSeg && fmTravelSegIter == fmTravelSegs.end() && travelSegIter == travelSegs.end())
      retFareMarket.push_back(*fareMarketI);
  }
}

//----------------------------------------------------------------------------
// buildFareMarket()
//----------------------------------------------------------------------------
bool
TrxUtil::buildFareMarket(PricingTrx& trx, const std::vector<TravelSeg*>& travelSeg)
{
  //
  // temp vectors to use as scratch board
  std::vector<const Loc*> locationList;
  std::vector<TravelSeg*> localTravelSeg;

  // pull out stuff from trx and build locationList
  for (size_t i = 0; i < travelSeg.size(); i++)
  {
    if (i == 0) // the first tvlSeg we need both orig and dest
    {
      locationList.push_back(travelSeg[i]->origin());
      locationList.push_back(travelSeg[i]->destination());
    }
    else
    {
      locationList.push_back(travelSeg[i]->destination());
    }
  }

  // fareMarket building algorithm
  for (size_t i = 0; i < locationList.size(); i++)
  {
    for (size_t i1 = i + 1; i1 < locationList.size(); i1++)
    {
      // We cannot have same orig - dest fare market
      // in case of a side trip but we yet need to keep
      // track of the travel segments

      if (locationList[i]->loc() == locationList[i1]->loc())
      {
        localTravelSeg.push_back(travelSeg[i1 - 1]);
      }
      else
      {
        FareMarket* fareMarket;
        trx.dataHandle().get(fareMarket); // lint !e530

        fareMarket->origin() = locationList[i];
        fareMarket->destination() = locationList[i1];
        fareMarket->travelDate() = travelSeg[0]->departureDT();

        // to insert the last tvl segment that builds up this fare market
        localTravelSeg.push_back(travelSeg[i1 - 1]);
        fareMarket->travelSeg().resize(localTravelSeg.size());
        copy(localTravelSeg.begin(), localTravelSeg.end(), fareMarket->travelSeg().begin());
        trx.fareMarket().push_back(fareMarket);
      }
    }

    localTravelSeg.clear();
    localTravelSeg.resize(0);
  }

  locationList.clear();
  locationList.resize(0);

  return true;
}

const Loc*
TrxUtil::getOverridenLoc(const PricingTrx& trx, GetOverridenLocCode getOverridenLocCode)
{
  const PricingRequest* request = trx.getRequest();
  if (UNLIKELY(request == nullptr))
  {
    LOG4CXX_ERROR(logger, "TrxUtil - request is NULL");
    return nullptr;
  }

  if ((request->*getOverridenLocCode)().empty())
  {
    const Agent* agent = request->ticketingAgent();
    if (UNLIKELY(agent == nullptr))
    {
      LOG4CXX_ERROR(logger, "TrxUtil - ticketingAgent is NULL");
      return nullptr;
    }
    return agent->agentLocation();
  }
  else
  {
    return trx.dataHandle().getLoc((request->*getOverridenLocCode)(), request->ticketingDT());
  }
}

//----------------------------------------------------------------------------
// ticketingLoc()
//----------------------------------------------------------------------------
const Loc*
TrxUtil::ticketingLoc(PricingTrx& trx)
{
  return getOverridenLoc(trx, &PricingRequest::ticketPointOverride);
}

//----------------------------------------------------------------------------
// saleLoc()
//----------------------------------------------------------------------------
const Loc*
TrxUtil::saleLoc(const PricingTrx& trx)
{
  return getOverridenLoc(trx, &PricingRequest::salePointOverride);
}

bool
TrxUtil::isRepricingForTaxShoppingDisabled(const PricingTrx* trx)
{
  const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(trx);
  if (taxTrx)
  {
    if (fallback::fixed::repricingForTaxShopping())
      return taxTrx->isShoppingPath();
    else
      return taxTrx->isRepricingForTaxShoppingDisabled();
  }

  return false;
}

//----------------------------------------------------------------------------
// reprice()
//----------------------------------------------------------------------------
RepricingTrx*
TrxUtil::reprice(PricingTrx& trx,
                 const std::vector<TravelSeg*>& travelSegs,
                 FMDirection fmDirectionOverride,
                 bool skipRuleValidation,
                 const CarrierCode* carrierOverride,
                 const GlobalDirection* globalDirectionOverride,
                 const PaxTypeCode& extraPaxType,
                 bool retrieveFbrFares,
                 bool retrieveNegFares,
                 Indicator wpncsFlagIndicator,
                 const char optionsFareFamilyType,
                 bool useCurrentDate,
                 bool privateFareCheck,
                 bool overrideTicketingAgent,
                 FareMarket::RetrievalInfo* retrievalInfo)
{
  if (TrxUtil::isRepricingForTaxShoppingDisabled(&trx))
  {
    LOG4CXX_WARN(logger, "TrxUtil::reprice - skip repricing for Shopping Tax requests");
    return nullptr;
  }

  const bool isRexPricingTrx = (trx.excTrxType() == PricingTrx::AR_EXC_TRX);
  RepricingTrx* newTrx = nullptr;
  RepriceCache::Result cacheResult;

  // check if RepricingTrx already exist
  {
    RepriceCache::Key cacheKey;
    cacheKey.tvlSeg = travelSegs;
    if (isRexPricingTrx)
      cacheKey.ticketingDate = trx.dataHandle().ticketDate();
    else
      cacheKey.ticketingDate = DateTime::emptyDate();
    if (carrierOverride)
      cacheKey.carrierOverride = *carrierOverride;
    cacheKey.extraPaxType = extraPaxType;
    cacheKey.fmDirectionOverride = fmDirectionOverride;
    if (globalDirectionOverride)
      cacheKey.globalDirectionOverride = *globalDirectionOverride;
    cacheKey.wpncsFlagIndicator = wpncsFlagIndicator;
    cacheKey.optionsFareFamilyType = optionsFareFamilyType;
    cacheKey.skipRuleValidation = skipRuleValidation;
    cacheKey.retrieveFbrFares = retrieveFbrFares;
    cacheKey.retrieveNegFares = retrieveNegFares;
    cacheKey.privateFareCheck = privateFareCheck;

    cacheResult = trx.repriceCache().get(cacheKey);

    if (cacheResult.hit())
      return cacheResult.result();
  }

  std::string toSvcName = "TO_SVC";

  Service* to = Global::service(toSvcName);

  if (to == nullptr)
  {
    LOG4CXX_ERROR(logger, "TrxUtil - unable to retrieve TO pointer");
    return nullptr;
  }

  trx.dataHandle().get(newTrx);

  if (!fallback::unifyMemoryAborter(&trx) && trx.aborter())
    trx.aborter()->addChildTrx(newTrx);

  newTrx->redirectedDiagnostic() = &trx.diagnostic();

  DateTime ticketDate = useCurrentDate ? DateTime::localTime() : trx.dataHandle().ticketDate();

  newTrx->dataHandle().setTicketDate(ticketDate);

  newTrx->setOriginTrxType(trx.getTrxType());

  if (isRexPricingTrx)
  {
    PricingRequest* request = nullptr;
    trx.dataHandle().get(request);
    newTrx->setRequest(request);

    newTrx->getRequest()->assign(*trx.getRequest());
    newTrx->getRequest()->ticketingDT() = ticketDate;

    newTrx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
  }
  else
  {
    if (wpncsFlagIndicator != 'I') // force or reset wpncs flag
    {
      PricingRequest* request = nullptr;
      trx.dataHandle().get(request);
      newTrx->setRequest(request);

      newTrx->getRequest()->assign(*trx.getRequest());
    }
    else
      newTrx->setRequest(trx.getRequest());
  }

  if (overrideTicketingAgent)
  {
    // overrideTicketingAgent should be true only when actual type of request is not PricingRequest
    // for example, AncRequest overrides ticketingAgent and copy done in assign() is not enough
    newTrx->getRequest()->ticketingAgent() = trx.getRequest()->ticketingAgent();
  }

  if (wpncsFlagIndicator != 'I') // force or reset wpncs flag
  {
    newTrx->getRequest()->lowFareNoAvailability() = wpncsFlagIndicator;
    newTrx->getRequest()->lowFareRequested() = 'N';
  }

  if (optionsFareFamilyType != 0 || privateFareCheck)
  {
    PricingOptions* options = nullptr;
    trx.dataHandle().get(options);
    options->assign(*trx.getOptions());

    if (optionsFareFamilyType != 0)
      options->fareFamilyType() = optionsFareFamilyType;

    if (privateFareCheck)
      options->privateFares() = '0';

    newTrx->setOptions(options);
  }
  else
  {
    newTrx->setOptions(trx.getOptions());
  }

  newTrx->billing() = trx.billing();
  newTrx->setTravelDate(trx.travelDate());
  newTrx->bookingDate() = trx.bookingDate();
  newTrx->ticketingDate() = trx.ticketingDate();
  newTrx->transactionStartTime() = trx.transactionStartTime();
  newTrx->setRetrieveFbrFares(retrieveFbrFares);
  newTrx->retrieveNegFares() = retrieveNegFares;
  newTrx->setDynamicCfgOverriden(trx.isDynamicCfgOverriden());
  newTrx->dynamicCfg() = trx.dynamicCfg();
#ifdef CONFIG_HIERARCHY_REFACTOR
  newTrx->mutableConfigBundle() = trx.configBundle();
#endif
  newTrx->availabilityMap() = trx.availabilityMap();
  if (trx.getTrxType() == PricingTrx::MIP_TRX &&
      !fallback::fallbackGSAMipDifferentialFareFix(&trx) && trx.isValidatingCxrGsaApplicable())
  {
    newTrx->setValidatingCxrGsaApplicable(true);
    newTrx->setParentTrx(&trx);
  }

  std::for_each(trx.paxType().begin(), trx.paxType().end(), RepricingTrx::AddReqPaxType(*newTrx));

  newTrx->posPaxType().insert(
      newTrx->posPaxType().begin(), trx.posPaxType().begin(), trx.posPaxType().end());

  if (!extraPaxType.empty())
  {
    PaxType* paxType = nullptr;
    newTrx->dataHandle().get(paxType);
    if (paxType != nullptr)
    {
      paxType->paxType() = extraPaxType;
      paxType->number() = trx.paxType().front()->number();

      PaxTypeUtil::initialize(*newTrx,
                              *paxType,
                              paxType->paxType(),
                              paxType->number(),
                              paxType->age(),
                              paxType->stateCode(),
                              uint16_t(newTrx->paxType().size() + 1));

      newTrx->paxType().push_back(paxType);
    }
  }

  Itin* itin = nullptr;
  trx.dataHandle().get(itin);

  std::vector<Itin*>& itins = newTrx->itin();
  itins.push_back(itin);

  std::vector<TravelSeg*>& itinTvlSegs = itin->travelSeg();
  itinTvlSegs.insert(itinTvlSegs.begin(), travelSegs.begin(), travelSegs.end());

  itin->setTravelDate(TseUtil::getTravelDate(itin->travelSeg()));

  std::vector<TravelSeg*>& tvlSegs = newTrx->travelSeg();
  tvlSegs.insert(tvlSegs.begin(), travelSegs.begin(), travelSegs.end());

  if (carrierOverride != nullptr)
    newTrx->carrierOverride() = *carrierOverride;

  if (globalDirectionOverride != nullptr)
    newTrx->globalDirectionOverride() = *globalDirectionOverride;

  if (fmDirectionOverride != FMDirection::UNKNOWN)
    newTrx->setFMDirectionOverride(fmDirectionOverride);

  newTrx->setSkipRuleValidation(skipRuleValidation);

  to->process(*newTrx);

  std::for_each(
      newTrx->fareMarket().begin(), newTrx->fareMarket().end(), RepricingTrx::ComparePaxTypeCode());

  if (isRexPricingTrx)
  {
    if (!retrievalInfo)
    {
      LOG4CXX_ERROR(logger,
                    "Are you mad ?! No RetrievalInfo for RexPricingTrx during RepricingTrx !");
    }

    else
      ExchangeUtil::setRetrievalInfo(*newTrx, retrievalInfo);
  }

  // Save it for reuse
  cacheResult.fill(newTrx);

  return newTrx;
}

//--------------------------------------------------------------------------
// getAlternateCurrency()
//--------------------------------------------------------------------------
const CurrencyCode&
TrxUtil::getAlternateCurrency(const PricingTrx& trx)
{
  const PricingOptions* options = trx.getOptions();
  if (options != nullptr)
  {
    return (options->alternateCurrency());
  }
  return (errorCCode);
}

//--------------------------------------------------------------------------
// getTicketingDT()
//--------------------------------------------------------------------------
const DateTime&
TrxUtil::getTicketingDT(const PricingTrx& trx)
{
  const PricingRequest* request = trx.getRequest();
  if (LIKELY(request != nullptr))
  {
    return (request->ticketingDT());
  }
  else
  {
    return (today);
  }
}

//--------------------------------------------------------------------------
// getTicketingDTCat5()
//--------------------------------------------------------------------------
const DateTime&
TrxUtil::getTicketingDTCat5(const PricingTrx& trx)
{
  if (trx.getParentTrx())
  {
    const MultiExchangeTrx* meTrx = dynamic_cast<const MultiExchangeTrx*>(trx.getParentTrx());
    if (meTrx)
    {
      if (meTrx->newPricingTrx() == &trx)
        return (trx.getRequest()->ticketingDT()); // D93
      else if (meTrx->excPricingTrx1() == &trx)
        return (meTrx->cat5TktDT_Ex1());
      else if (meTrx->excPricingTrx2() == &trx)
        return (meTrx->cat5TktDT_Ex2());
    }
  }

  if (UNLIKELY(trx.excTrxType() == PricingTrx::PORT_EXC_TRX))
  {
    const BaseExchangeTrx& beTrx = static_cast<const BaseExchangeTrx&>(trx);
    if (!beTrx.currentTicketingDT().isEmptyDate())
      return (beTrx.currentTicketingDT());
  }

  else if (UNLIKELY(trx.excTrxType() == PricingTrx::AR_EXC_TRX))
  {
    const RexPricingTrx& rexTrx = static_cast<const RexPricingTrx&>(trx);

    if (rexTrx.isAnalyzingExcItin())
      return rexTrx.originalTktIssueDT();
    else
      return rexTrx.currentTicketingDT();
  }

  return (getTicketingDT(trx));
}

namespace
{
bool
areNotEqual(const double* a, const double* b)
{
  return fabs(*a - *b) > EPSILON;
}
} // namespace

const Percent*
TrxUtil::getDiscountPercentage(const FareMarket& fareMarket,
                               const Discounts& discounts,
                               const bool isMip)
{
  const Percent* discount = nullptr;

  if (discounts.isDPorPPentry())
    for (const TravelSeg* travelSeg : fareMarket.travelSeg())
    {
      if (!travelSeg->isAir())
        continue;

      if (discount == nullptr)
        discount = discounts.getPercentage(travelSeg->segmentOrder(), isMip);
      else if (areNotEqual(discount, discounts.getPercentage(travelSeg->segmentOrder(), isMip)))
      {
        discount = nullptr;
        break;
      }
    }

  return discount;
}

const DiscountAmount*
TrxUtil::getDiscountAmount(const FareMarket& fareMarket, const Discounts& discounts)
{
  const DiscountAmount* discount = nullptr;

  if (discounts.isDAorPAentry())
    for (const TravelSeg* travelSeg : fareMarket.travelSeg())
    {
      if (!travelSeg->isAir())
        continue;

      if (discount == nullptr)
        discount = discounts.getAmount(travelSeg->segmentOrder());

      if (discount != nullptr)
        if ((fareMarket.travelSeg().front()->segmentOrder() < discount->startSegmentOrder) ||
            (fareMarket.travelSeg().back()->segmentOrder() > discount->endSegmentOrder))
        {
          discount = nullptr;
          break;
        }
    }

  return discount;
}

const Percent*
TrxUtil::getDiscountPercentage(const PricingUnit& pricingUnit,
                               const Discounts& discounts,
                               const bool isMip)
{
  const Percent* discount = nullptr;

  if (discounts.isDPorPPentry())
  {
    const Percent* lastDiscount = nullptr;
    for (const FareUsage* fareUsage : pricingUnit.fareUsage())
    {
      const FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();
      discount = getDiscountPercentage(*fareMarket, discounts, isMip);
      if (discount == nullptr)
        return discount;

      if (lastDiscount == nullptr)
        lastDiscount = discount;
      else if (areNotEqual(lastDiscount, discount))
      {
        discount = nullptr;
        break;
      }
    }
  }

  return discount;
}

const DiscountAmount*
TrxUtil::getDiscountAmount(const PricingUnit& pricingUnit, const Discounts& discounts)
{
  const DiscountAmount* discount = nullptr;

  if (discounts.isDAorPAentry())
  {
    const DiscountAmount* lastDiscount = nullptr;
    for (const FareUsage* fareUsage : pricingUnit.fareUsage())
    {
      const FareMarket* fareMarket = fareUsage->paxTypeFare()->fareMarket();
      discount = getDiscountAmount(*fareMarket, discounts);
      if (discount == nullptr)
        return discount;

      if (lastDiscount == nullptr)
        lastDiscount = discount;
      else if (lastDiscount != discount)
      {
        discount = nullptr;
        break;
      }
    }
  }

  return discount;
}

bool
TrxUtil::validateDiscountOld(const PricingTrx& trx, const FareMarket& fareMarket)
{
  Percent dp = 0;
  const DiscountAmount* da = nullptr;

  return TrxUtil::getDiscountOld(trx, fareMarket, dp, da);
}

bool
TrxUtil::validateDiscountOld(const PricingTrx& trx, const PricingUnit& pu)
{
  Percent dp = 0;
  const DiscountAmount* da = nullptr;

  return TrxUtil::getDiscountOld(trx, pu, dp, da);
}

bool
TrxUtil::validateDiscountNew(const PricingTrx& trx, const FareMarket& fareMarket)
{
  return validateDiscount(fareMarket, trx.getRequest()->discountsNew(), trx.isMip());
}

bool
TrxUtil::validateDiscountNew(const PricingTrx& trx, const PricingUnit& pricingUnit)
{
  return validateDiscount(pricingUnit, trx.getRequest()->discountsNew(), trx.isMip());
}

bool
TrxUtil::validateDiscount(const FareMarket& fareMarket,
                          const Discounts& discounts,
                          const bool isMip)
{
  if (LIKELY(!discounts.isDPorPPentry() && !discounts.isDAorPAentry()))
    return true;

  if (getDiscountPercentage(fareMarket, discounts, isMip) ||
      getDiscountAmount(fareMarket, discounts))
    return true;
  else
    return false;
}

bool
TrxUtil::validateDiscount(const PricingUnit& pricingUnit,
                          const Discounts& discounts,
                          const bool isMip)
{
  if (LIKELY(!discounts.isDPorPPentry() && !discounts.isDAorPAentry()))
    return true;

  if (getDiscountPercentage(pricingUnit, discounts, isMip) ||
      getDiscountAmount(pricingUnit, discounts))
    return true;
  else
    return false;
}

bool
TrxUtil::getDiscountOld(const PricingTrx& trx,
                        const FareMarket& fareMarket,
                        Percent& discPercent,
                        const DiscountAmount*& discAmount)
{
  discPercent = 0;
  discAmount = nullptr;

  const PricingRequest& request = *trx.getRequest();

  if (LIKELY(!request.isDPEntry() && !request.isDAEntry()))
    return true;

  Percent dp = -1;
  const DiscountAmount* da = nullptr;

  for (const TravelSeg* travelSeg : fareMarket.travelSeg())
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (airSeg == nullptr)
      continue;

    if (request.isDPEntry())
    {
      // Conflict Discount Percentages, invalidate this fare market
      if (dp == -1)
        dp = request.discountPercentage(airSeg->segmentOrder());
      else
      {
        if (dp != request.discountPercentage(airSeg->segmentOrder()))
          return false;
      }
    }

    if (request.isDAEntry() && da == nullptr)
    {
      // Conflict Discount Amounts, invalidate this fare market.
      if ((da = request.discountAmount(airSeg->segmentOrder())) != nullptr)
      {
        if ((fareMarket.travelSeg().front()->segmentOrder() < da->startSegmentOrder) ||
            (fareMarket.travelSeg().back()->segmentOrder() > da->endSegmentOrder))
          return false;
      }
    }

    // Co-exists Discount Amount and Discount Percentages, invalidate this fare market.
    if (dp > 0 && da != nullptr)
      return false;
  }

  if (dp > 0)
    discPercent = dp;

  discAmount = da;

  return true;
}

bool
TrxUtil::getDiscountOld(const PricingTrx& trx,
                        const PricingUnit& pu,
                        Percent& discPercent,
                        const DiscountAmount*& discAmount)
{
  discPercent = 0;
  discAmount = nullptr;

  const PricingRequest& request = *trx.getRequest();

  if (LIKELY(!request.isDPEntry() && !request.isDAEntry()))
    return true;

  Percent lastDp = -1;
  const DiscountAmount* lastDa = nullptr;

  for (const FareUsage* fareUsage : pu.fareUsage())
  {
    const FareMarket& fm = *(fareUsage->paxTypeFare()->fareMarket());

    Percent dp = 0;
    const DiscountAmount* da = nullptr;
    if (!getDiscountOld(trx, fm, dp, da)) // Must have conflict DA or DP
      return false;

    if (lastDp < 0) // First fare Usage
    {
      lastDp = dp;
      lastDa = da;
    }
    else // Check for conflict DA or DP across Fare Usage
    {
      if (lastDp != dp || lastDa != da)
        return false;
    }
  }

  if (lastDp > 0)
    discPercent = lastDp;

  discAmount = lastDa;

  return true;
}

bool
TrxUtil::determineIfNotCurrentRequest(const PricingTrx& trx)
{
  DateTime localTime = DateTime::localTime();
  DateTime pccCalcTime;
  short utcOffset = 0;
  const Loc* saleLoc;
  // since local time is of HDQ -- we should send HDQ location as reference
  if (trx.getRequest()->PricingRequest::salePointOverride().size())
    saleLoc =
        trx.dataHandle().getLoc(trx.getRequest()->PricingRequest::salePointOverride(), localTime);
  else
    saleLoc = trx.getRequest()->ticketingAgent()->agentLocation();
  const Loc* hdqLoc = trx.dataHandle().getLoc("HDQ", localTime);

  if (saleLoc && hdqLoc)
  {
    if (!LocUtil::getUtcOffsetDifference(
            *saleLoc, *hdqLoc, utcOffset, trx.dataHandle(), localTime, localTime))
      utcOffset = 0;
  }

  if (utcOffset)
    pccCalcTime = localTime.addSeconds(utcOffset * 60);
  else
    pccCalcTime = localTime;

  DateTime pcctime = trx.ticketingDate();

  if (pcctime.year() == pccCalcTime.year() && pcctime.month() == pccCalcTime.month() &&
      pcctime.day() == pccCalcTime.day())
    return false;

  return true;
}

bool
TrxUtil::reuseFarePUResult()
{
  return reuseFarePuResultPricing.getValue();
}

bool
TrxUtil::isAdjustRexTravelDateEnabled()
{
  return adjustRexTravelDate.getValue();
}

bool
TrxUtil::isExchangeOrTicketing(PricingTrx& trx)
{
  if (UNLIKELY(trx.getRequest()->isTicketEntry()))
    return true;

  if (LIKELY(trx.isNotExchangeTrx()))
    return false;

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX || trx.excTrxType() == PricingTrx::NEW_WITHIN_ME ||
      trx.excTrxType() == PricingTrx::EXC1_WITHIN_ME ||
      trx.excTrxType() == PricingTrx::EXC2_WITHIN_ME || trx.excTrxType() == PricingTrx::ME_DIAG_TRX)
    return true;

  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    if ((static_cast<ExchangePricingTrx&>(trx)).reqType() != AGENT_PRICING_MASK)
      return true;
  }
  return false;
}

bool
TrxUtil::journeyMileageApply(const CarrierCode& carrier)
{
  return alg::contains(mileageCarriers.getValue(), carrier);
}

bool
TrxUtil::interlineAvailabilityApply(const CarrierCode& carrier)
{
  return alg::contains(interlineAvailabilityCarriersCfg.getValue(), carrier);
}

bool
TrxUtil::interlineAvailabilityApply(const TravelSeg* segment)
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(segment);
  return (airSeg && interlineAvailabilityApply(airSeg->marketingCarrierCode()));
}

namespace
{
bool
isCxrInVec(const CarrierCode& cxr, const std::vector<CarrierCode>& cxrVec)
{
  return std::find(cxrVec.begin(), cxrVec.end(), cxr) != cxrVec.end();
}
}

bool
TrxUtil::interlineAvailabilityApply(const PricingTrx& trx,
                                    const CarrierCode& currCarrier,
                                    const CarrierCode& nextCarrier)
{
  if (UNLIKELY(!(PricingTrx::MIP_TRX == trx.getTrxType() ||
                 PricingTrx::IS_TRX == trx.getTrxType() || PricingTrx::FF_TRX == trx.getTrxType())))
    return false;

  const std::vector<InterlineCarrierInfo*>& iciVec = trx.dataHandle().getInterlineCarrier();
  for (const InterlineCarrierInfo* ici : iciVec)
  {
    if (ici->carrier() == currCarrier)
    {
      if (!fallback::fallbackInterlineIgnorePartners(&trx))
        return true;

      if (currCarrier == nextCarrier)
        return true;

      if (isCxrInVec(ANY_CARRIER, ici->partners()) || isCxrInVec(nextCarrier, ici->partners()))
        return true;
    }
  }
  return false;
}

bool
TrxUtil::interlineAvailabilityApply(const PricingTrx& trx,
                                    const TravelSeg* currSegment,
                                    const TravelSeg* nextSegment)
{
  if (UNLIKELY(!(PricingTrx::MIP_TRX == trx.getTrxType() ||
                 PricingTrx::IS_TRX == trx.getTrxType() || PricingTrx::FF_TRX == trx.getTrxType())))
    return false;

  if (UNLIKELY(!currSegment))
    return false;

  const AirSeg* currAirSeg = currSegment->toAirSeg();
  const AirSeg* nextAirSeg = (nextSegment) ? nextSegment->toAirSeg() : nullptr;
  if (!currAirSeg)
    return false;

  const CarrierCode currCC = currAirSeg->marketingCarrierCode();
  const CarrierCode nextCC = (nextAirSeg) ? nextAirSeg->marketingCarrierCode() : "**";

  return interlineAvailabilityApply(trx, currCC, nextCC);
}

bool
TrxUtil::intralineAvailabilityApply(const PricingTrx& trx,
                                    const CarrierCode& current,
                                    const CarrierCode& next)
{
  if (!((PricingTrx::MIP_TRX == trx.getTrxType() || PricingTrx::IS_TRX == trx.getTrxType() ||
         PricingTrx::FF_TRX == trx.getTrxType()) &&
        trx.billing()->actionCode() != "WPNI.C" && trx.billing()->actionCode() != "WFR.C"))
    return false;

  const std::vector<IntralineCarrierInfo*>& iciVec = trx.dataHandle().getIntralineCarrier();
  for (const IntralineCarrierInfo* ici : iciVec)
  {
    if (isCxrInVec(current, ici->partners()) && isCxrInVec(next, ici->partners()))
      return true;
  }
  return false;
}

bool
TrxUtil::isIntralineAvailabilityCxr(const PricingTrx& trx, const CarrierCode& cxr)
{
  if (!((PricingTrx::MIP_TRX == trx.getTrxType() || PricingTrx::IS_TRX == trx.getTrxType() ||
         PricingTrx::FF_TRX == trx.getTrxType()) &&
        trx.billing()->actionCode() != "WPNI.C" && trx.billing()->actionCode() != "WFR.C"))
    return false;

  const std::vector<IntralineCarrierInfo*>& iciVec = trx.dataHandle().getIntralineCarrier();
  for (const IntralineCarrierInfo* ici : iciVec)
  {
    if (isCxrInVec(cxr, ici->partners()))
      return true;
  }
  return false;
}

void
TrxUtil::getIntralineAvailabilityCxrsNPartners(const PricingTrx& trx, IntraMap& intraCxrs)
{
  configureIntralineAvailabilityCxrsPartners(trx, intraCxrs);
}

void
TrxUtil::getInterlineAvailabilityCxrsNPartners(const PricingTrx& trx, InterMap& interlineCxrs)
{
  configureInterlineAvailabilityCxrsPartners(trx, interlineCxrs);
}

void
TrxUtil::configureInterlineAvailabilityCxrsPartners(const PricingTrx& trx, InterMap& interCxrs)
{
  boost::lock_guard<boost::mutex> guard(_mutex);

  const std::vector<InterlineCarrierInfo*>& iciVec = trx.dataHandle().getInterlineCarrier();

  if (!iciVec.empty())
  {
    interCxrs.clear();

    for (const InterlineCarrierInfo* ici : iciVec)
    {
      interCxrs.insert(
          std::pair<CarrierCode, std::vector<CarrierCode>>(ici->carrier(), ici->partners()));
    }
  }
}

void
TrxUtil::configureIntralineAvailabilityCxrsPartners(const PricingTrx& trx, IntraMap& intraCxrs)
{
  boost::lock_guard<boost::mutex> guard(_mutex);

  std::map<std::string, std::vector<CarrierCode>> intralineCxrs;

  const std::vector<IntralineCarrierInfo*>& iciVec = trx.dataHandle().getIntralineCarrier();

  if (!iciVec.empty())
  {
    intraCxrs.clear();
    for (const IntralineCarrierInfo* ici : iciVec)
    {
      intraCxrs.insert(
          std::pair<std::string, std::vector<CarrierCode>>(ici->name(), ici->partners()));
    }
  }
}

// E3-11 Cat 35 L Type
bool
TrxUtil::cat35LtypeEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest() ? trx.getRequest()->ticketingAgent() : nullptr;
  if (LIKELY(agent))
  {
    if (agent->abacusUser() || agent->infiniUser())
      return true;

    return const_cast<PricingTrx&>(trx).isGNRAllowed();
  }
  return false;
}

// E3-11 Cat 35 With TFD
bool
TrxUtil::tfdNetRemitFareCombEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest() ? trx.getRequest()->ticketingAgent() : nullptr;
  if (LIKELY(agent))
  {
    if (agent->abacusUser() || agent->infiniUser() || agent->axessUser())
      return true;

    return const_cast<PricingTrx&>(trx).isGNRAllowed();
  }
  return false;
}

void
TrxUtil::configureTaxEnhancementActivationDate(void)
{
  boost::lock_guard<boost::mutex> guard(_mutex);

  if (_taxEnhancementActivationDateConfigured)
    return;
  std::string cfgDate = enableTaxEnhancementDate.getValue();
  if (!cfgDate.empty())
  {
    if (!TrxUtil::convertYYYYMMDDDate(cfgDate, _taxEnhancementActivationDate))
    {
      LOG4CXX_ERROR(logger, "Unable to Set Tax Enhancement Activation Date " << cfgDate);
    }
  }
  _taxEnhancementActivationDateConfigured = true;
}

bool
TrxUtil::taxEnhancementActivated(const DateTime& dateChk)
{
  if (!_taxEnhancementActivationDateConfigured)
    configureTaxEnhancementActivationDate();

  return (dateChk >= TrxUtil::_taxEnhancementActivationDate);
}

void
TrxUtil::createTrxAborter(Trx& trx)
{
  if (trx.aborter() == nullptr)
  {
    int trxTimeout = 0;

    TicketingCxrTrx* ticketingCxrTrx = dynamic_cast<TicketingCxrTrx*>(&trx);
    if (ticketingCxrTrx)
    {
      trxTimeout = tcsTimeout.getValue();
    }
    else
    {
      trxTimeout = trxTimeoutCfg.getValue();
    }
    if (trxTimeout > 0)
    {
      TrxAborter* aborter = &trx.dataHandle().safe_create<TrxAborter>(&trx);
      aborter->setTimeout(trxTimeout);
      if (fallback::unifyMemoryAborter(&trx))
      {
        aborter->setErrorCode(ErrorResponseException::WORLDFARE_AT_PEAK_USE);
        aborter->setErrorMsg("WORLDFARE AT PEAK USE - RETRY 8 SEC");
      }
      else
      {
        TrxAborter::Error error{
            ErrorResponseException::WORLDFARE_AT_PEAK_USE, "WORLDFARE AT PEAK USE - RETRY 8 SEC"};
        aborter->setError(TrxAborter::ErrorKind::TIMEOUT, error);
        aborter->setError(TrxAborter::ErrorKind::MEMORY, error);
      }

      trx.aborter() = aborter;
    }
  }
}

bool
TrxUtil::isDisableYYForExcItin(const PricingTrx& trx)
{
  return ((trx.excTrxType() == PricingTrx::AR_EXC_TRX && !rexExcCat31YYEnabled.getValue()) ||
          (trx.excTrxType() == PricingTrx::AF_EXC_TRX && !rexExcCat33YYEnabled.getValue())) &&
         static_cast<const RexBaseTrx&>(trx).isAnalyzingExcItin();
}

bool
TrxUtil::swsPoAtsePath(const PricingTrx& trx)
{
  if (trx.billing() != nullptr && trx.billing()->requestPath() == SWS_PO_ATSE_PATH)
    return true;
  return false;
}

bool
TrxUtil::pssPoAtsePath(const PricingTrx& trx)
{
  if (trx.billing() != nullptr && trx.billing()->requestPath() == PSS_PO_ATSE_PATH)
    return true;
  return false;
}

bool
TrxUtil::libPoAtsePath(const PricingTrx& trx)
{
  if (trx.billing() == nullptr)
    return false;

  return trx.billing()->requestPath() == LIBERTY_PO_ATSE_PATH ||
         trx.billing()->requestPath() == LIBERTY_ATSE_PATH;
}

bool
TrxUtil::isIataFareSelectionActivated(const PricingTrx& trx)
{
  return iataFareSelectionActivationDate.isValid(&trx);
}

bool
TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(const PricingTrx& trx)
{
  return atpcoRbdByCabinAnswerTableActivationDate.isValid(&trx, trx.dataHandle().ticketDate());
}

bool
TrxUtil::isIataFareSelectionApplicable(PricingTrx* trx)
{
  const bool isAncillaryTrx = (dynamic_cast<AncillaryPricingTrx*>(trx) != nullptr);
  const bool isTktFeesTrx = (dynamic_cast<TktFeesPricingTrx*>(trx) != nullptr);

  if (isAncillaryTrx || isTktFeesTrx)
    return false;

  if (trx->getRequest() && trx->getRequest()->isGoverningCarrierOverrideEntry())
    return false;

  const PricingTrx::TrxType trxType = trx->getTrxType();
  return PricingTrx::PRICING_TRX == trxType || PricingTrx::IS_TRX == trxType ||
         PricingTrx::MIP_TRX == trxType || PricingTrx::REPRICING_TRX == trxType;
}

DynamicConfigurableEffectiveDate&
TrxUtil::getIata302BaggageActivationDate(const PricingTrx& trx)
{
  return trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty()
             ? iata302BaggageActivationCarrierDate
             : iata302BaggageActivationDate;
}

bool
TrxUtil::isIata302BaggageActivated(const PricingTrx& trx)
{
  if (trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return getIata302BaggageActivationDate(trx).isValid(&trx);
}

bool
TrxUtil::isIata302BaggageActivated(const PricingTrx& trx, const DateTime& trxDate)
{
  if (trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return getIata302BaggageActivationDate(trx).isValid(&trx, trxDate);
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggageActivationXmlResponseEnabled(const PricingTrx& trx)
{
  return baggageActivationXmlResponseEnabled;
}

bool
TrxUtil::isFareSelectionForSubIata21Only(const PricingTrx& trx)
{
  return fareSelectionForSubIata21Only.isValid(&trx);
}

bool
TrxUtil::isRbdByCabinValueInShopResponse(const PricingTrx& trx)
{
  return rbdByCabinValueInShopResponse.isValid(&trx);
}

bool
TrxUtil::isRbdByCabinValueInPriceResponse(const PricingTrx& trx)
{
  return rbdByCabinValueInPriceResponse.isValid(&trx);
}

bool
TrxUtil::isFareSelectionForNGSThruFMOnly(const PricingTrx& trx)
{
  return fareSelectionForNGSThruFMOnly.isValid(&trx);
}

bool
TrxUtil::isBaggageActivationXmlResponse(const PricingTrx& trx)
{
  return isBaggageActivationXmlResponseEnabled(trx).isValid(&trx);
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggage302CarryOnActivatedEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->abacusUser())
  {
    return baggageCarryOnActivationAbEnabled; // ABACUS
  }
  else if (agent->axessUser())
  {
    return baggageCarryOnActivationAxEnabled; // AXESS
  }
  else if (agent->infiniUser())
  {
    return baggageCarryOnActivationInEnabled; // INFINI
  }
  else if (!agent->tvlAgencyPCC().empty())
  {
    return baggageCarryOnActivationTnEnabled; // TN
  }
  else
  {
    if (trx.billing()->partitionID() == SPECIAL_CARRIER_AA)
    {
      return baggageCarryOnActivationAAEnabled; // AA
    }
    else
    {
      return baggageCarryOnActivationAirlineEnabled; // Other Airline
    }
  }
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggage302ExchangeActivatedEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->abacusUser())
  {
    return baggageExchangeActivationAbEnabled; // ABACUS
  }
  else if (agent->axessUser())
  {
    return baggageExchangeActivationAxEnabled; // AXESS
  }
  else if (agent->infiniUser())
  {
    return baggageExchangeActivationInEnabled; // INFINI
  }
  else if (LIKELY(!agent->tvlAgencyPCC().empty()))
  {
    return baggageExchangeActivationTnEnabled; // TN
  }
  else
  {
    return baggageExchangeActivationAirlineEnabled; // Other Airline
  }
}

bool
TrxUtil::isBaggage302CarryOnActivated(const PricingTrx& trx)
{
  if (trx.billing() == nullptr || trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return isBaggage302CarryOnActivatedEnabled(trx).isValid(&trx);
}

bool
TrxUtil::isBaggage302ExchangeActivated(const PricingTrx& trx)
{
  if (UNLIKELY(trx.getRequest()->ticketingAgent() == nullptr))
    return false;

  return isBaggage302ExchangeActivatedEnabled(trx).isValid(&trx);
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggage302EmbargoesActivatedEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->abacusUser())
  {
    return baggageEmbargoesActivationAbEnabled; // ABACUS
  }
  else if (agent->axessUser())
  {
    return baggageEmbargoesActivationAxEnabled; // AXESS
  }
  else if (agent->infiniUser())
  {
    return baggageEmbargoesActivationInEnabled; // INFINI
  }
  else if (!agent->tvlAgencyPCC().empty())
  {
    return baggageEmbargoesActivationTnEnabled; // TN
  }
  else
  {
    if (trx.billing()->partitionID() == SPECIAL_CARRIER_AA)
    {
      return baggageEmbargoesActivationAAEnabled; // AA
    }
    else
    {
      return baggageEmbargoesActivationAirlineEnabled; // Other Airline
    }
  }
}

bool
TrxUtil::isBaggage302EmbargoesActivated(const PricingTrx& trx)
{
  if (trx.billing() == nullptr || trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return isBaggage302EmbargoesActivatedEnabled(trx).isValid(&trx);
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggage302GlobalDisclosureActivatedEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->abacusUser())
  {
    return baggageGlobalDisclosureActivationAbEnabled; // ABACUS
  }
  else if (agent->axessUser())
  {
    return baggageGlobalDisclosureActivationAxEnabled; // AXESS
  }
  else if (agent->infiniUser())
  {
    return baggageGlobalDisclosureActivationInEnabled; // INFINI
  }
  else if (!agent->tvlAgencyPCC().empty())
  {
    return baggageGlobalDisclosureActivationTnEnabled; // TN
  }
  else
  {
    if (trx.billing()->partitionID() == SPECIAL_CARRIER_AA)
    {
      return baggageGlobalDisclosureActivationAAEnabled; // AA
    }
    else
    {
      return baggageGlobalDisclosureActivationAirlineEnabled; // Other Airline
    }
  }
}

bool
TrxUtil::isBaggage302GlobalDisclosureActivated(const PricingTrx& trx)
{
  if (trx.billing() == nullptr || trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return isBaggage302GlobalDisclosureActivatedEnabled(trx).isValid(&trx) &&
         checkSchemaVersionForGlobalDisclosure(trx);
}

DynamicConfigurableEffectiveDate&
TrxUtil::getBaggage302DotActivationDate(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();
  if (agent->tvlAgencyPCC().empty())
  {
    return baggage302DotActivationCarrierDate;
  }
  else if (agent->abacusUser())
  {
    return baggage302DotActivationAbDate;
  }
  else if (agent->axessUser())
  {
    return baggage302DotActivationAxDate;
  }
  else if (agent->infiniUser())
  {
    return baggage302DotActivationInDate;
  }
  return baggage302DotActivationDate;
}

bool
TrxUtil::isBaggage302DotActivated(const PricingTrx& trx)
{
  if (trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return getBaggage302DotActivationDate(trx).isValid(&trx);
}

bool
TrxUtil::isBaggage302NewUSDotMethodActivated(const PricingTrx& trx)
{
  return baggage302NewUSDotMethodActivationDate.isValid(&trx);
}

bool
TrxUtil::isBaggageCTAMandateActivated(const PricingTrx& trx)
{
  return baggageCTAMandateActivationDate.isValid(&trx);
}

bool
TrxUtil::isIataReso302MandateActivated(const PricingTrx& trx)
{
  return iataReso302MandateActivationDate.isValid(&trx);
}

DynamicConfigurableFlagOn&
TrxUtil::getBaggageDotPhase2ADisplayEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->tvlAgencyPCC().empty())
  {
    return baggageDotPhase2ADisplayCarriersEnabled;
  }
  else if (agent->abacusUser())
  {
    return baggageDotPhase2ADisplayAbEnabled;
  }
  else if (agent->axessUser())
  {
    return baggageDotPhase2ADisplayAxEnabled;
  }
  else if (agent->infiniUser())
  {
    return baggageDotPhase2ADisplayInEnabled;
  }
  return baggageDotPhase2ADisplayTnEnabled;
}

bool
TrxUtil::isBaggageDotPhase2ADisplayEnabled(const PricingTrx& trx)
{
  if (trx.getRequest()->ticketingAgent() == nullptr)
    return false;

  return getBaggageDotPhase2ADisplayEnabled(trx).isValid(&trx);
}

bool
TrxUtil::isBaggageBTAActivated(const PricingTrx& trx)
{
  if (UNLIKELY(trx.billing() == nullptr || trx.getRequest()->ticketingAgent() == nullptr))
    return false;

  return isBaggageBTAActivationEnabled(trx).isValid(&trx);
}

DynamicConfigurableFlagOn&
TrxUtil::isBaggageBTAActivationEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest()->ticketingAgent();

  if (agent->abacusUser())
  {
    return baggageBTAActivationAbEnabled; // ABACUS
  }
  if (UNLIKELY(agent->axessUser()))
  {
    return baggageBTAActivationAxEnabled; // AXESS
  }
  else if (agent->infiniUser())
  {
    return baggageBTAActivationInEnabled; // INFINI
  }
  else if (LIKELY(!agent->tvlAgencyPCC().empty()))
  {
    return baggageBTAActivationTnEnabled; // TN
  }
  else
  {
    if (trx.billing()->partitionID() == SPECIAL_CARRIER_AA)
    {
      return baggageBTAActivationAAEnabled; // AA
    }
    else
    {
      return baggageBTAActivationAirlineEnabled; // Other Airline
    }
  }
}

bool
TrxUtil::isProject331PlusDaysEnabled(const PricingTrx& trx)
{
  return project331PlusDaysEnabled.isValid(&trx);
}

bool
TrxUtil::isFqS8BrandedServiceActivated(const PricingTrx& trx)
{
  return fqS8BrandedServiceActivationDate.isValid(&trx);
}

bool
TrxUtil::isFullMapRoutingActivated(const PricingTrx& trx)
{
  return fullMapRoutingActivationDate.isValid(&trx);
}

bool
TrxUtil::isMultiTicketPricingEnabled(const PricingTrx& trx)
{
  return multiTicketPricingEnabled.isValid(&trx);
}

bool
TrxUtil::isSupplementChargeEnabled(const PricingTrx& trx)
{
  return supplementChargeEnabled.isValid(&trx);
}

bool
TrxUtil::isPricingInfiniCat05ValidationSkipActivated(const PricingTrx& trx)
{
  return pricingInfiniCat05ValidationSkipDate.isValid(&trx);
}

std::string
TrxUtil::supplementChargeCarrierListData(const PricingTrx& trx)
{
  return supplementChargeCarrierList.getValue(&trx);
}

std::string
TrxUtil::gtcCarrierListData(const PricingTrx& trx)
{
  return gtcCarrierList.getValue(&trx);
}

std::string
TrxUtil::amChargesTaxesListData(const PricingTrx& trx)
{
  return amChargesTaxesList.getValue(&trx);
}

std::string
TrxUtil::supplementHdrMsgFareDisplayData(const PricingTrx& trx)
{
  return supplementHdrMsgFareDisplay.getValue(&trx);
}

std::string
TrxUtil::subCodeDefinitionsData(const PricingTrx& trx)
{
  return subCodeDefinitions.getValue(&trx);
}

bool
TrxUtil::isAtpcoTTProjectHighRTEnabled(const PricingTrx& trx)
{
  return atpcoTTProjectHighRTEnabled.isValid(&trx);
}

bool
TrxUtil::isDnataEnabled(const PricingTrx& trx)
{
  return dnataEnabled.isValid(&trx);
}

bool
TrxUtil::isMarkupSecFilterEnhancementEnabled(const PricingTrx& trx)
{
  return markupSecFilterEnhancementEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesDisplayEnabled(const PricingTrx& trx)
{
  return atpcoTaxesDisplayEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnChangeFeeEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnChangeFeeEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnItinEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnItinEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnChangeFeeServiceFlagEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnChangeFeeServiceFlag.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnItinServiceFlagEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnItinServiceFlag.isValid(&trx);
}

bool
TrxUtil::isAtpcoBackingOutTaxesEnabled(const PricingTrx& trx)
{
  return atpcoBackingOutTaxesEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnOcEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnOcEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesOnBaggageEnabled(const PricingTrx& trx)
{
  return atpcoTaxesOnBaggageEnabled.isValid(&trx);
}

bool
TrxUtil::isAtpcoTaxesDefaultRoundingEnabled(const PricingTrx& trx)
{
  return atpcoTaxesDefaultRoundingEnabled.isValid(&trx);
}

bool
TrxUtil::isHpsDomGroupingEnabled(const PricingTrx& trx)
{
  return hpsTaxDomGroupingEnabled.isValid(&trx);
}

bool
TrxUtil::isHpsIntlGroupingEnabled(const PricingTrx& trx)
{
  return hpsTaxIntlGroupingEnabled.isValid(&trx);
}

bool
TrxUtil::useHPSFlightRanges(const PricingTrx& trx)
{
  return hpsTaxUseFlightRanges.isValid(&trx);
}

bool
TrxUtil::isTaxesNewHeaderASActive(const PricingTrx& trx)
{
  return taxesNewHeaderActivationASDate.isValid(&trx);
}

bool
TrxUtil::isTaxExemptionForChildActive(const PricingTrx& trx)
{
  return taxExemptionForChildActivationDate.isValid(&trx);
}

bool
TrxUtil::isSplitTaxesByFareComponentEnabled(const PricingTrx& trx)
{
  return splitTaxesByFareComponentEnabled.isValid(&trx);
}

bool
TrxUtil::isBaggageInPQEnabled(const PricingTrx& trx)
{
  if (trx.getTrxType() == PricingTrx::PRICING_TRX || trx.getTrxType() == PricingTrx::MIP_TRX)
    return baggageInPQEnabled.isValid(&trx);
  return false;
}

bool
TrxUtil::isFVOSurchargesEnabled()
{
  return FVOSurchargesEnabledConf.getValue();
}

uint32_t
TrxUtil::getConfigOBFeeOptionMaxLimit()
{
  return pricingOptionMaxLimit.getValue();
}

//
// Convert string to DateTime
// Valid Format Example : 2001-12-19, 2001-DEC-19, 2001/12/19
//
bool
TrxUtil::convertYYYYMMDDDate(std::string& dateStr, DateTime& outDate)
{
  try
  {
    std::string dateTimeStr = dateStr + " 00:00:00.000";
    DateTime convertedTime(dateTimeStr);
    outDate = convertedTime;
  }
  catch (...)
  {
    return false;
  }

  return true;
}

bool
TrxUtil::isUSAirAncillaryActivated(const PricingTrx& trx)
{
  return usAirAncillaryEnabled.isValid(&trx);
}

bool
TrxUtil::isPrepaidBaggageActivated(const PricingTrx& trx)
{
  return prepaidBaggageEnabled.isValid(&trx);
}

bool
TrxUtil::isBaggageChargesInMipActivated(const PricingTrx& trx)
{
  return baggageChargesInMip.isValid(&trx);
}

bool
TrxUtil::isEMDSPhase2Activated(const PricingTrx& trx)
{
  return eMDSPhase2Enabled.isValid(&trx);
}

bool
TrxUtil::cmdPricingTuningEnabled(const PricingTrx& trx)
{
  return cmdPricingTuningActivated.isValid(&trx);
}

bool
TrxUtil::optimusNetRemitEnabled(const PricingTrx& trx)
{
  const Agent* agent = trx.getRequest() ? trx.getRequest()->ticketingAgent() : nullptr;
  if (LIKELY(agent))
  {
    if (agent->abacusUser() || const_cast<PricingTrx&>(trx).isGNRAllowed())
      return true;
    return false;
  }
  return false;
}

uint32_t
TrxUtil::getMemCheckInterval(const Trx& trx)
{
  return memCheckIntervalConfig.getValue(&trx);
}

uint32_t
TrxUtil::getMemCheckTrxInterval(const Trx& trx)
{
  return memCheckTrxIntervalConfig.getValue(&trx);
}

uint32_t
TrxUtil::getShoppingMemCheckInterval(const Trx& trx)
{
  return memShoppingCheckInterval.getValue(&trx);
}

uint32_t
TrxUtil::getShoppingPercentageMemGrowthCheckInterval(const Trx& trx)
{
  return perMemGrowthShoppingCheckInterval.getValue(&trx);
}

uint32_t
TrxUtil::permCheckInterval(const PricingTrx& trx)
{
  return permCheckIntervalConfig.getValue(&trx);
}

uint32_t
TrxUtil::referenceRSS(const Trx& trx)
{
  return referenceRSSConfig.getValue(&trx);
}

uint32_t
TrxUtil::referenceVM(const Trx& trx)
{
  return referenceVMConfig.getValue(&trx);
}

uint32_t
TrxUtil::maxRSSGrowth(const Trx& trx)
{
  return maxRSSGrowthConfig.getValue(&trx);
}

uint32_t
TrxUtil::yqyrPrecalcLimit(const Trx& trx)
{
  return yqyrPrecalcLimitConfig.getValue(&trx);
}

uint32_t
TrxUtil::abortCheckInterval(const PricingTrx& trx)
{
  return abortCheckIntervalConfig.getValue(&trx);
}

uint32_t
TrxUtil::maxVMGrowth(const Trx& trx)
{
  return maxVMGrowthConfig.getValue(&trx);
}

uint32_t
TrxUtil::maxCat31SolutionCount(const PricingTrx& trx)
{
  return maxCat31SolutionCountConfig.getValue(&trx);
}

uint32_t
TrxUtil::getMaxNumberOfSolutions(const PricingTrx& trx)
{
  return maxNumberOfSolutions.getValue(&trx);
}
// @brief if test pcc and date check fails, check against users
bool
TrxUtil::isNetRemitEnabled(const PricingTrx& trx)
{
  if (isRexOrCsoTrx(trx) || (trx.getRequest() && trx.getRequest()->ticketingAgent() &&
                             trx.getRequest()->ticketingAgent()->isArcUser()))
    return false;

  if (!isBspUser(trx))
    return false;

  const Agent* agent = trx.getRequest() ? trx.getRequest()->ticketingAgent() : nullptr;
  if (agent)
  {
    // no need to check activation table for abacus and infini users
    if (agent->abacusUser() || agent->infiniUser())
      return true;

    return const_cast<PricingTrx&>(trx).isGNRAllowed();
  }
  return false;
}

bool
TrxUtil::isRexOrCsoTrx(const PricingTrx& trx)
{
  if (UNLIKELY(dynamic_cast<const RexBaseTrx*>(&trx) ||
               dynamic_cast<const BaseExchangeTrx*>(&trx) ||
               dynamic_cast<const CsoPricingTrx*>(&trx)))
    return true;
  return false;
}

bool
TrxUtil::isBspUser(const PricingTrx& trx)
{
  if ((!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
        && trx.isValidatingCxrGsaApplicable())
  {
    for (CountrySettlementPlanInfo* cspInfo : trx.countrySettlementPlanInfos())
    {
      if (cspInfo && cspInfo->getSettlementPlanTypeCode().equalToConst("BSP"))
        return true;
    }
    return false;
  }
  return true;
}

bool
TrxUtil::isTOCAllowed(PricingTrx& trx)
{
  return isAtpcoTaxesOnOcEnabled(trx) && trx.isCustomerActivatedByDate("TOC");
}

bool
TrxUtil::isTOBAllowed(PricingTrx& trx)
{
  return isAtpcoTaxesOnBaggageEnabled(trx) && trx.isCustomerActivatedByDate("TOB");
}

bool
TrxUtil::isBOTAllowed(PricingTrx& trx)
{
  return isAtpcoBackingOutTaxesEnabled(trx) && trx.isCustomerActivatedByDate("BOT");
}

bool
TrxUtil::isTOEAllowed(PricingTrx& trx)
{
  return (isAtpcoTaxesOnChangeFeeEnabled(trx) && trx.isCustomerActivatedByDate("TOE")) ||
         isAtpcoTaxesOnChangeFeeServiceFlagEnabled(trx);
}

bool
TrxUtil::isTOEAllowed(PricingTrx& trx, const DateTime& date)
{
  return (isAtpcoTaxesOnChangeFeeEnabled(trx) &&
          trx.isCustomerActivatedByDateAndFlag("TOE", date)) ||
         isAtpcoTaxesOnChangeFeeServiceFlagEnabled(trx);
}

bool
TrxUtil::isTOIAllowed(PricingTrx& trx)
{
  return (isAtpcoTaxesOnItinEnabled(trx) && trx.isCustomerActivatedByDate("TOI")) ||
         isAtpcoTaxesOnItinServiceFlagEnabled(trx);
}

bool
TrxUtil::isTOIAllowed(PricingTrx& trx, const DateTime& date)
{
  return (isAtpcoTaxesOnItinEnabled(trx) && trx.isCustomerActivatedByDateAndFlag("TOI", date)) ||
         isAtpcoTaxesOnItinServiceFlagEnabled(trx);
}

bool
TrxUtil::isTCHAllowed(PricingTrx& trx)
{
  return (trx.isCustomerActivatedByDate("TCH"));
}

bool
TrxUtil::needMetricsInResponse()
{
  if (!_recordingMetricsConfigured)
    getMetricsToRecordInfo();

  return (!_recordingMetricsDescription.empty());
}

bool
TrxUtil::needMetricsInResponse(const char* metricsDescription)
{
  if (!_recordingMetricsConfigured)
    getMetricsToRecordInfo();

  for (std::string& descript : _recordingMetricsDescription)
  {
    size_t descSz = descript.size();
    if (descript.c_str()[descSz - 1] == '*')
    {
      if (strncmp(descript.c_str(), metricsDescription, descSz - 1) == 0)
        return true;
    }
    else if (strcmp(descript.c_str(), metricsDescription) == 0)
      return true;
  }
  return false;
}

void
TrxUtil::getMetricsToRecordInfo()
{
  boost::lock_guard<boost::mutex> guard(_mutex);

  if (_recordingMetricsConfigured)
    return;
  for (auto metric : metricsInResponse.getValue())
  {
    replace(metric.begin(), metric.end(), '_', ' ');
    _recordingMetricsDescription.push_back(metric);
  }

  _recordingMetricsConfigured = true;
}

bool
TrxUtil::isPricingTaxRequest(const PricingTrx* pricingTrx)
{
  const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(pricingTrx);

  if (taxTrx)
  {
    return !taxTrx->isShoppingPath();
  }

  return false;
}

bool
TrxUtil::isShoppingTaxRequest(const PricingTrx* pricingTrx)
{
  const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(pricingTrx);

  if (UNLIKELY(taxTrx))
  {
    return taxTrx->isShoppingPath();
  }

  return false;
}

bool
TrxUtil::getValidatingCxrFbcDisplayPref(const PricingTrx& trx, const FarePath& farePath)
{
  const PricingRequest* pReq = trx.getRequest();
  // if agent from hosted  partition return false
  if ((pReq->ticketingAgent()->agentTJR() == nullptr) && trx.billing() &&
      (!trx.billing()->partitionID().empty()) && (trx.billing()->aaaCity().size() < 4))
    return false;
  // accept req. only from 1S users
  if (!pReq->ticketingAgent()->sabre1SUser())
    return false;

  // look for validating cxr in itin
  CarrierCode validatingCxr = farePath.itin()->validatingCarrier();
  validatingCxr = MCPCarrierUtil::swapToPseudo(&trx, validatingCxr);

  // retrieve  valcxr pref from db
  const CarrierPreference* valCxrPref =
      trx.dataHandle().getCarrierPreference(validatingCxr, farePath.itin()->travelDate());
  if ((valCxrPref) && (valCxrPref->applyFBCinFC() == tse::YES))
    return true;
  else
    return false;
}

void
TrxUtil::setInfiniCat05BookingDTSkipFlagInItins(PricingTrx& priTrx)
{
  if (!isPricingInfiniCat05ValidationSkipActivated(priTrx))
    return;
  const Agent* agent = priTrx.getRequest()->ticketingAgent();
  Alpha3Char overrideCode = agent->agentTJR()->cat05OverrideCode();
  if (overrideCode == " ") // no override
    return;
  if (overrideCode == tse::ANY_CARRIER)
  {
    std::vector<Itin*>::iterator itinI = priTrx.itin().begin();
    std::vector<Itin*>::iterator itinE = priTrx.itin().end();
    for (; itinI != itinE; itinI++)
    {
      LOG4CXX_INFO(logger,
                   "Cat05OverrideCarrier:"
                       << "AllCarriers");
      (*itinI)->cat05BookingDateValidationSkip() = true;
    }
    return;
  }
  // read the cat05override table entry  for this pcc
  PseudoCityCode pcc = agent->tvlAgencyPCC();
  DataHandle dataHandle(priTrx.getRequest()->ticketingDT());
  const std::vector<Cat05OverrideCarrier*>& ovCxrList = dataHandle.getCat05OverrideCarrierInfo(pcc);
  if (ovCxrList.size())
  {
    const std::vector<CarrierCode>& cxrList = ovCxrList[0]->carrierList();
    std::vector<Itin*>::iterator itinI = priTrx.itin().begin();
    for (; itinI != priTrx.itin().end(); ++itinI)
    {
      (*itinI)->cat05BookingDateValidationSkip() =
          std::find(cxrList.begin(), cxrList.end(), (*itinI)->validatingCarrier()) != cxrList.end();
      if ((*itinI)->cat05BookingDateValidationSkip())
        LOG4CXX_INFO(logger, "Cat05OverrideCarrier:" << (*itinI)->validatingCarrier());
    }
  }
  return;
}

bool
TrxUtil::isCat35TFSFEnabled(const PricingTrx& priTrx)
{
  if (priTrx.getTrxType() == PricingTrx::IS_TRX ||
      (!isAutomatedRefundCat33Enabled(priTrx) &&
       priTrx.excTrxType() == PricingTrx::AF_EXC_TRX))
    return false;
  if (LIKELY(priTrx.getTrxType() == PricingTrx::MIP_TRX))
  {
    if (LIKELY(priTrx.excTrxType() != PricingTrx::AR_EXC_TRX))
      return false;
    if (!static_cast<const RexPricingOptions*>(priTrx.getOptions())->isNetSellingIndicator())
      return false;
  }
  const Agent* agent = priTrx.getRequest()->ticketingAgent();
  // not applicable airline pccs
  return !(agent && agent->tvlAgencyPCC().empty());
}

bool
TrxUtil::checkSchemaVersionForGlobalDisclosure(const PricingTrx& pricingTrx)
{
  if (!pricingTrx.isNotExchangeTrx())
    return true;

  const BaggageTrx* baggageTrx = dynamic_cast<const BaggageTrx*>(&pricingTrx);
  if (baggageTrx)
    return baggageTrx->getRequest() && baggageTrx->getRequest()->checkSchemaVersion(1, 0, 1);

  if (pricingTrx.altTrxType() == PricingTrx::WPA ||
      pricingTrx.altTrxType() == PricingTrx::WPA_NOMATCH)
    return pricingTrx.getRequest()->checkSchemaVersion(1, 0, 1);

  return pricingTrx.getRequest()->checkSchemaVersion(1, 1, 1);
}

bool
TrxUtil::isAutomaticPfcTaxExemptionEnabled(const PricingTrx& trx)
{
  if (trx.billing() && trx.billing()->partitionID() == "AA" && trx.getRequest() &&
      trx.getRequest()->ticketingAgent() &&
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
  {
    return true;
  }
  return false;
}

bool
TrxUtil::areUSTaxesOnYQYREnabled(const PricingTrx& trx)
{
  if (trx.billing() && trx.billing()->partitionID() == "AA" && trx.getRequest() &&
      trx.getRequest()->ticketingAgent() &&
      trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty())
    return true;
  else
    return false;
}

CurrencyCode
TrxUtil::getEquivCurrencyCode(const PricingTrx& trx)
{
  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
  {
    return trx.getOptions()->currencyOverride();
  }

  return trx.getRequest()->ticketingAgent()->currencyCodeAgent();
}

bool
TrxUtil::isSpecialOpenJawActivated(const PricingTrx& trx)
{
  return specialOpenJawActivationDate.isValid(&trx);
}

bool
TrxUtil::isdiffCntyNMLOpenJawActivated(const PricingTrx& trx)
{
  return diffCntyNMLOpenJawActivationDate.isValid(&trx);
}

bool
TrxUtil::isCosExceptionFixActivated(const PricingTrx& trx)
{
  return trx.isCosExceptionFixEnabled();
}

bool
TrxUtil::isSimpleTripForMIP(const PricingTrx& trx)
{
  bool rc = false;

  if ((trx.getTrxType() == PricingTrx::MIP_TRX) && (trx.legPreferredCabinClass().size() <= 2))
    rc = true;

  return rc;
}

bool
TrxUtil::isHistorical(const PricingTrx& trx)
{
  DateTime localTime = DateTime::localTime();
  const Loc* hdqLoc = trx.dataHandle().getLoc(RuleConst::HDQ_CITY, localTime);
  const Loc* pccLoc = trx.getRequest()->ticketingAgent()->agentLocation();

  // check if we have locations
  if (UNLIKELY((nullptr == hdqLoc) || (nullptr == pccLoc)))
    return false;

  short utcOffset = 0;
  // get time difference between agent time and local time
  if (LIKELY(LocUtil::getUtcOffsetDifference(
          *pccLoc, *hdqLoc, utcOffset, trx.dataHandle(), localTime, localTime)))
  {
    // adjust the time to agent time
    localTime = localTime.addSeconds(utcOffset * 60);
    return trx.ticketingDate() < localTime;
  }
  return false;
}

OBFeeSubType
TrxUtil::getOBFeeSubType(const ServiceSubTypeCode code)
{
  if (code.equalToConst("FCA") || code.equalToConst("FDA"))
    return OBFeeSubType::OB_F_TYPE;

  switch (code[0])
  {
  case 'T':
    return OBFeeSubType::OB_T_TYPE;
    break;
  case 'R':
    return OBFeeSubType::OB_R_TYPE;
    break;
  default:
    break;
  }
  return OBFeeSubType::OB_UNKNOWN;
}

bool
TrxUtil::isTotalPriceEnabled(PricingTrx& trx)
{
  if (UNLIKELY(!totalPriceAllowed.isValid(&trx)))
    return false;

  if (UNLIKELY(trx.excTrxType() != PricingTrx::NOT_EXC_TRX))
    return TrxUtil::isTotalPriceEnabledForREX(trx);

  if (trx.getTrxType() != PricingTrx::MIP_TRX &&
      trx.getTrxType() !=
          PricingTrx::PRICING_TRX) // Other transaction types are unsupported as of now.
    return false;

  // Alt Dates controlled separately
  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.isAltDates())
    return !fallback::yqyrADTotalPricing(&trx);

  return totalPriceEnabled.isValid(
             &trx) || // If the flag in config is turned on, it takes precedence over CACT.
         trx.isCustomerActivatedByDate("TPC");
}

bool
TrxUtil::isTotalPriceEnabledForREX(PricingTrx& trx)
{
  // Non Cat31/33 requests often provide fare basis codes in the request. If this is the case,
  // there's no need to use Total Price
  // because the fare basis codes to be used are forced.
  // Cat31/33 provide fare basis codes for the "old" itinerary.
  Itin* itin = trx.itin().front();
  bool allDummy = true;
  for (std::vector<TravelSeg*>::const_iterator it = itin->travelSeg().begin();
       it != itin->travelSeg().end();
       ++it)
  {
    const AirSeg* airSeg = (*it)->toAirSeg();
    if (airSeg && airSeg->fareBasisCode().empty())
    {
      allDummy = false;
      break;
    }
  }
  // This part will also disable Total Price for the REPRICE_EXCITIN_PHASE of Cat31 and Cat33.
  if (allDummy)
    return false;

  // If the flag in config is turned on, it takes precedence over CACT.
  if (totalPriceEnabled.isValid(&trx))
    return true;

  if (fallback::ssdsp1788SanitizerErrorFix(&trx))
  {
    const BaseExchangeTrx& beTrx = static_cast<const BaseExchangeTrx&>(trx);

    LOG4CXX_DEBUG(logger,
                  "Exchange TotalPriceConcept activation for exchange type: "
                      << trx.excTrxType() << " asked with D07/currentTicketingDT = "
                      << beTrx.currentTicketingDT().toIsoExtendedString());

    if (!beTrx.currentTicketingDT().isEmptyDate())
      return trx.isCustomerActivatedByDateAndFlag("TPC", beTrx.currentTicketingDT());

    else
      LOG4CXX_ERROR(logger, "EmptyDate used - disabling exchange TotalPriceConcept !");
  }
  else
  {
    const BaseExchangeTrx* beTrx = nullptr;
    if (trx.excTrxType() != PricingTrx::CSO_EXC_TRX)
      beTrx = static_cast<const BaseExchangeTrx*>(&trx);
    else
      beTrx = static_cast<const BaseExchangeTrx*>(trx.getParentTrx());

    LOG4CXX_DEBUG(logger,
                  "Exchange TotalPriceConcept activation for exchange type: "
                      << trx.excTrxType() << " asked with D07/currentTicketingDT = "
                      << beTrx->currentTicketingDT().toIsoExtendedString());

    if (!beTrx->currentTicketingDT().isEmptyDate())
      return trx.isCustomerActivatedByDateAndFlag("TPC", beTrx->currentTicketingDT());
    else
      LOG4CXX_ERROR(logger, "EmptyDate used - disabling exchange TotalPriceConcept !");
  }

  return false;
}

bool
TrxUtil::isRequestFromAS(const PricingTrx& trx)
{
  if (trx.getRequest()->ticketingAgent()->agentTJR() == nullptr && trx.billing() &&
      !trx.billing()->partitionID().empty() && trx.billing()->aaaCity().size() < 4)
    return true;

  return false;
}

bool
TrxUtil::isPerformOACCheck(const PricingTrx& trx)
{
  return isRequestFromAS(trx);
}

bool
TrxUtil::isDynamicConfigOverrideEnabled(const Trx& trx)
{
  return dynamicConfigOverrideEnabled.isValid(&trx);
}

bool
TrxUtil::isDynamicConfigOverridePermament(const Trx& trx)
{
  return dynamicConfigOverridePermament.isValid(&trx);
}

bool
TrxUtil::isJumpCabinLogicDisableCompletely(const Trx& trx)
{
  return jumpCabinLogicDisableCompletely.isValid(&trx);
}

bool
TrxUtil::isFqFareRetailerEnabled(const Trx& trx)
{
  return fqFareRetailerEnabled.isValid(&trx);
}

size_t
TrxUtil::getMinAvailableMemoryTrx(const Trx& trx)
{
  return minAvailableMemoryTrx.getValue(&trx);
}

size_t
TrxUtil::getMaxRSSPercentageMemoryTrx(const Trx& trx)
{
  return maxRSSPercentageMemoryTrx.getValue(&trx);
}

bool
TrxUtil::isEnoughAvailableMemory(const size_t minAvailableMemory, const size_t maxRSSPercentage)
{
  const size_t available(MemoryUsage::getAvailableMemory() / 1024);
  if (UNLIKELY(available < minAvailableMemory))
  {
    if (maxRSSPercentage < 100)
    {
      static const size_t total(MemoryUsage::getTotalMemory() / 1024);
      const size_t rss(MemoryUsage::getResidentMemorySize() / (1024 * 1024));
      const size_t percentage(rss * 100u / total);
      if (percentage < maxRSSPercentage)
        return true;
      LOG4CXX_ERROR(logger,
                    "Memory Total=" << total << " RSS=" << rss << " Percentage=" << percentage
                                    << " MaxRSSPercentage=" << maxRSSPercentage);
    }
    LOG4CXX_ERROR(logger,
                  "Low memory level dropping transactions. Available="
                      << available << " MinAvailableMemory=" << minAvailableMemory);
    return false;
  }
  return true;
}

static void
throwMemoryError(PricingTrx& trx)
{
  const bool isShopping =
      (trx.getTrxType() == PricingTrx::MIP_TRX && trx.excTrxType() != PricingTrx::AR_EXC_TRX) ||
      trx.getTrxType() == PricingTrx::IS_TRX || trx.getTrxType() == PricingTrx::ESV_TRX ||
      trx.getTrxType() == PricingTrx::FF_TRX;

  if (isShopping)
    throw ErrorResponseException(ErrorResponseException::SHOPPING_MAX_NUMBER_COMBOS_EXCEEDED);
  else
    throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
}

//TODO Remove with unifyMemoryAborter.
void
TrxUtil::checkTrxMemoryFlag(PricingTrx& trx)
{
  if (UNLIKELY(trx.getMemoryManager()->isOutOfMemory()))
    throwMemoryError(trx);
}

void
TrxUtil::checkTrxMemoryLimits(PricingTrx& trx)
{
  const size_t minimum(TrxUtil::getMinAvailableMemoryTrx(trx));
  const size_t maxRSSPercentage = TrxUtil::getMaxRSSPercentageMemoryTrx(trx);

  if (LIKELY(TrxUtil::isEnoughAvailableMemory(minimum, maxRSSPercentage)))
    return;

  LOG4CXX_ERROR(logger,
                "TXN:" << trx.transactionId()
                       << " CONSUMED TOO MUCH MEMORY - TOO LOW AVAILABLE MEMORY");
  if (!fallback::logRequestWhenLowMemory(&trx))
    LOG4CXX_ERROR(loggerLowMemory,
                  "TXN:" << trx.transactionId() << " XML REQUEST: " << trx.rawRequest());
  throwMemoryError(trx);
}

void
TrxUtil::checkTrxMemoryGrowth(PricingTrx& trx)
{
  if (!trx.memGrowthExceeded())
    return;

  LOG4CXX_ERROR(logger, "TXN:" << trx.transactionId() << " CONSUMED TOO MUCH MEMORY");
  throwMemoryError(trx);
}

void
TrxUtil::checkTrxMemoryManager(PricingTrx& trx)
{
  if (!Memory::GlobalManager::instance() ||
      LIKELY(!Memory::GlobalManager::instance()->isCriticalCondition()))
    return;

  LOG4CXX_ERROR(logger, "TXN:" << trx.transactionId() << " CONSUMED TOO MUCH MEMORY (MMAN)");
  throwMemoryError(trx);
}

void
TrxUtil::checkTrxMemoryAborted(PricingTrx& trx,
                               const uint32_t count,
                               const uint32_t memGrowCheckInterval,
                               const uint32_t memAvailCheckInterval)
{
  if (!Memory::changesFallback)
  {
    Memory::GlobalManager::instance()->checkTrxMemoryLimits(trx, count);
  }
  else
  {
    TrxUtil::checkTrxMemoryManager(trx);

    if (memAvailCheckInterval > 0 && count % memAvailCheckInterval == 0)
      TrxUtil::checkTrxMemoryLimits(trx);

    if (memGrowCheckInterval > 0 && count % memGrowCheckInterval == 0)
      TrxUtil::checkTrxMemoryGrowth(trx);
  }
}

bool
TrxUtil::isAAAwardPricing(const PricingTrx& trx)
{
  if (LIKELY(!trx.awardRequest() || !trx.billing() || trx.billing()->partitionID() != "AA"))
    return false;

  switch (trx.getTrxType())
  {
  case PricingTrx::PRICING_TRX:
  case PricingTrx::REPRICING_TRX:
    break;
  default:
    return false;
  }

  return true;
}

bool
TrxUtil::isIcerActivated(const PricingTrx& trx)
{
  return icerActivationDate.isValid(&trx);
}

bool
TrxUtil::isIcerActivated(const Trx& trx, const DateTime& dateChk)
{
  return icerActivationDate.isValid(&trx, dateChk);
}

bool
TrxUtil::isJcbCarrier(const CarrierCode* carrier)
{
  if (!carrier)
    return false;

  return alg::contains(isJcbCarrierCfg.getValue(), *carrier);
}

bool
TrxUtil::isFrequentFlyerTierActive(const Trx& trx)
{
  return ffTierActivationDate.isValid(&trx);
}

bool
TrxUtil::isFrequentFlyerTierOBActive(const Trx& trx)
{
  return ffTierOBActivationDate.isValid(&trx);
}

bool
TrxUtil::isEmdValidationForM70Active(const Trx& trx)
{
  return emdValidationForM70.isValid(&trx);
}

bool
TrxUtil::isEmdValidationOnReservationPathForAB240Active(const Trx& trx)
{
  return emdValidationOnReservationPathForAB240.isValid(&trx);
}

bool
TrxUtil::isEmdValidationFlightRelatedOnCheckingPathForAB240Active(const Trx& trx)
{
  return emdValidationFlightRelatedOnCheckingPathForAB240.isValid(&trx);
}

bool
TrxUtil::isEmdValidationFlightRelatedServiceAndPrepaidBaggageActive(const Trx& trx)
{
  return emdValidationFlightRelatedServiceAndPrepaidBaggage.isValid(&trx);
}

bool
TrxUtil::isEmdValidationInterlineChargeChecksActivated(const Trx& trx)
{
  return emdValidationInterlineChargeChecks.isValid(&trx);
}

bool
TrxUtil::isEmdValidationRelaxedActivated(const Trx& trx)
{
  return emdValidationRelaxed.isValid(&trx);
}

bool
TrxUtil::usesPrecalculatedTaxes(const PricingTrx& trx)
{
  switch (trx.getTrxType())
  {
  case PricingTrx::MIP_TRX:
    return trx.isAltDates();
  case PricingTrx::IS_TRX:
    return !static_cast<const ShoppingTrx&>(trx).isSumOfLocalsProcessingEnabled();
  default:
    return false;
  }
}

const TravelSeg*
TrxUtil::getInterlineAvailNextTravelSeg(const TravelSeg* currTvlSeg,
                                        const std::vector<TravelSeg*>& tsVec)
{
  const TravelSeg* prevTS = 0;
  const TravelSeg* nextTS = 0;

  if (tsVec.size() <= 1)
    return nextTS; // which is null

  // from this point on, the travel segments must be at least 2.
  std::vector<TravelSeg*>::const_iterator itr = tsVec.begin();
  for (; itr != tsVec.end(); itr++)
  {
    if (*itr == currTvlSeg)
    {
      if (itr == tsVec.end() - 1)
      // the currTvlSeg is the last segment,
      {
        if (prevTS && (prevTS->legId() == (*itr)->legId()))
        {
          // the last leg has 2+ travel segs
          return prevTS;
        }
        else // the last leg has only 1 travel seg
          return 0;
      }
      else
      { // the currTvlSeg isn't the last segment of the itin
        nextTS = *(itr + 1);
        if (nextTS->legId() == (*itr)->legId())
          return nextTS;
        else
        {
          if (prevTS && (prevTS->legId() == (*itr)->legId()))
            return prevTS;
          else // this leg has only 1 travel seg
            return 0;
        }
      }
    }

    prevTS = *itr;
  }

  // currTvlSeg not found
  return 0;
}

bool
TrxUtil::newDiscountLogic(const PricingTrx& trx)
{
  if (trx.getRequest() == nullptr)
    return false;

  return newDiscountLogic(*trx.getRequest(), trx);
}

bool
TrxUtil::newDiscountLogic(const PricingRequest& request, const Trx& trx)
{
  if (!fallback::azPlusUp(&trx))
      return true;

  return false;
}

std::string
TrxUtil::getOverrideBsResponse(const PricingTrx& pricingTrx)
{
  return dynamicConfigOverrideBSResponse.getValue(&pricingTrx);
}

uint32_t
TrxUtil::getMaxPenaltyFailedFaresThreshold(const Trx& trx)
{
  return maxPenaltyFailedFaresThreshold.getValue(&trx);
}

uint32_t
TrxUtil::getRec2Cat35SegCountLimit(const Trx& trx)
{
  return rec2Cat35SegCountLimitConfig.getValue(&trx);
}

int16_t
TrxUtil::getASBaseTaxEquivTotalLength(const Trx& trx)
{
  return ASBaseTaxEquivTotalLengthConfig.getValue(&trx);
}

bool
TrxUtil::isAutomatedRefundCat33Enabled(const PricingTrx& trx)
{
  return automatedRefundCat33Enabled.isValid(&trx);
}

bool
TrxUtil::isAbacusEndorsementHierarchyAtpcoFaresActive(const PricingTrx& trx)
{
  return abacusEndorsementHierarchyAtpcoFares.isValid(&trx);
}

bool
TrxUtil::isInfiniEndorsementHierarchyAtpcoFaresActive(const PricingTrx& trx)
{
  return infiniEndorsementHierarchyAtpcoFares.isValid(&trx);
}

bool
TrxUtil::isDateAdjustmentIndicatorActive(const PricingTrx& trx)
{
  return dateAdjustmentIndicatorActive.isValid(&trx);
}

bool
TrxUtil::isControlConnectIndicatorActive(const PricingTrx& trx)
{
  return controlConnectIndicatorActive.isValid(&trx);
}
} // tse
