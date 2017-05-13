//----------------------------------------------------------------------------
//
//  File:        TrxUtil.h
//  Created:     4/12/2004
//  Authors:     JY
//
//  Description: Common functions required for ATSE shopping/pricing.
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
#pragma once

#include "Common/Config/DynamicConfigurableDate.h"
#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"

#include <boost/lexical_cast.hpp>

#include <atomic>
#include <vector>

namespace tse
{
class TravelSeg;
class PricingTrx;
class Loc;
class RepricingTrx;
class Itin;
class PricingUnit;
class PricingRequest;
class DiscountAmount;

class TrxUtil
{
  friend class TrxUtilTest;
  friend class RexPricingTrxTest;
  friend class ExcItinTest;
  friend class TestDbQuery;
  friend class TestJourney;
  friend class NegotiatedFareRuleUtilTest;
  friend class NegotiatedFareCombinationValidatorTest;
  friend class ItinUtilTest;
  friend class ItinAnalyzerServiceTest;

public:
  static FareMarket* getFareMarket(const PricingTrx& trx,
                                   const CarrierCode carrier,
                                   const std::vector<TravelSeg*>& travelSegs,
                                   const DateTime& date,
                                   const Itin* itin = nullptr);

  static void getFareMarket(const PricingTrx& trx,
                            const std::vector<TravelSeg*>& travelSegs,
                            const DateTime& date,
                            std::vector<FareMarket*>& retFareMarket,
                            const Itin* itin = nullptr);

  static bool buildFareMarket(PricingTrx& trx, const std::vector<TravelSeg*>& travelSeg);

  typedef const LocCode& (PricingRequest::*GetOverridenLocCode)() const;
  static const Loc* getOverridenLoc(const PricingTrx& trx, GetOverridenLocCode getOverridenLocCode);
  static const Loc* ticketingLoc(PricingTrx& trx);
  static const Loc* saleLoc(const PricingTrx& trx);

  static RepricingTrx* reprice(PricingTrx& trx,
                               const std::vector<TravelSeg*>& travelSegs,
                               FMDirection fmDirectionOverride = FMDirection::UNKNOWN,
                               bool skipRuleValidation = false,
                               const CarrierCode* carrierOverride = nullptr,
                               const GlobalDirection* globalDirectionOverride = nullptr,
                               const PaxTypeCode& extraPaxType = "",
                               bool retrieveFbrFares = false,
                               bool retrieveNegFares = false,
                               Indicator wpncsFlagIndicator = 'I',
                               const char optionsFareFamilyType = 0,
                               bool useCurrentDate = false,
                               bool privateFareCheck = false,
                               bool overrideTicketingAgent = false,
                               FareMarket::RetrievalInfo* retrievalInfo = nullptr);

  //--------------------------------------------------------------------------
  // @function Convenience::getAlternateCurrency
  //
  // Description: Get the alternate currency
  //
  // @param trx - a valid PricingTrx reference
  //--------------------------------------------------------------------------
  static const CurrencyCode& getAlternateCurrency(const PricingTrx& trx);

  //--------------------------------------------------------------------------
  // @function Convenience::getTicketingDT
  //
  // Description: Get the alternate currency
  //
  // @param trx - a valid PricingTrx reference
  //--------------------------------------------------------------------------
  static const DateTime& getTicketingDT(const PricingTrx& trx);
  static const DateTime& getTicketingDTCat5(const PricingTrx& trx);

  static const Percent*
  getDiscountPercentage(const FareMarket& fareMarket, const Discounts& discounts, const bool isMip);
  static const DiscountAmount*
  getDiscountAmount(const FareMarket& fareMarket, const Discounts& discounts);
  static const Percent* getDiscountPercentage(const PricingUnit& pricingUnit,
                                              const Discounts& discounts,
                                              const bool isMip);
  static const DiscountAmount*
  getDiscountAmount(const PricingUnit& pricingUnit, const Discounts& discounts);

  static bool validateDiscountOld(const PricingTrx& trx, const FareMarket& fareMarket);
  static bool validateDiscountOld(const PricingTrx& trx, const PricingUnit& pricingUnit);
  static bool validateDiscountNew(const PricingTrx& trx, const FareMarket& fareMarket);
  static bool validateDiscountNew(const PricingTrx& trx, const PricingUnit& pricingUnit);
  static bool
  validateDiscount(const FareMarket& fareMarket, const Discounts& discounts, const bool isMip);
  static bool
  validateDiscount(const PricingUnit& pricingUnit, const Discounts& discounts, const bool isMip);

  static bool getDiscountOld(const PricingTrx& trx,
                             const FareMarket& fareMarket,
                             Percent& discPercent,
                             const DiscountAmount*& discAmount);

  static bool getDiscountOld(const PricingTrx& trx,
                             const PricingUnit& pu,
                             Percent& discPercent,
                             const DiscountAmount*& discAmount);

  static const std::string& getPNR(const PricingTrx& trx)
  {
    const PricingOptions* options = trx.getOptions();

    return (options != nullptr ? options->pnr() : EMPTY_STRING());
  }

  static const std::string& getLineEntry(PricingTrx& trx)
  {
    const PricingOptions* options = trx.getOptions();

    return (options != nullptr ? options->lineEntry() : EMPTY_STRING());
  }

  // Couldnt return a const string& because of the Code<8> of agentCity
  static std::string getPCC(const PricingTrx& trx);

  static std::string getPCCFromReq(const PricingTrx& trx);

  static const std::string& getHostCarrier(const PricingTrx& trx);

  static const std::string& parentServiceName(const PricingTrx& trx);

  static const std::string getTransId(const PricingTrx& trx)
  {
    const Billing* billing = trx.billing();
    return (billing != nullptr ? boost::lexical_cast<std::string>(billing->transactionID())
                               : EMPTY_STRING());
  }

  static const std::string getClientTransId(const PricingTrx& trx)
  {
    const Billing* billing = trx.billing();
    return (billing != nullptr ? boost::lexical_cast<std::string>(billing->clientTransactionID())
                               : EMPTY_STRING());
  }

  static const std::string& getLNIATA(const PricingTrx& trx)
  {
    const Billing* billing = trx.billing();
    return (billing != nullptr ? billing->userSetAddress() : EMPTY_STRING());
  }

  static const std::string& getPartitionId(const PricingTrx& trx)
  {
    const Billing* billing = trx.billing();
    return (billing != nullptr ? billing->partitionID() : EMPTY_STRING());
  }

  static const std::string& getHomeAgencyIATA(PricingTrx& trx);

  static bool isAbacusEnabled() { return abacusEnabled; }

  // should only been set once in TseServer
  static void enableAbacus() { abacusEnabled = true; }
  static void disableAbacus() { abacusEnabled = false; }

  static bool determineIfNotCurrentRequest(const PricingTrx& trx);

  static bool journeyMileageApply(const CarrierCode& carrier);

  typedef std::map<CarrierCode, std::vector<CarrierCode>> InterMap;
  typedef std::map<std::string, std::vector<CarrierCode>> IntraMap;

  static bool interlineAvailabilityApply(const CarrierCode& carrier);
  static bool interlineAvailabilityApply(const TravelSeg* segment);
  static bool interlineAvailabilityApply(const PricingTrx& trx,
                                         const CarrierCode& currCarrier,
                                         const CarrierCode& nextCarrier);
  static bool interlineAvailabilityApply(const PricingTrx& trx,
                                         const TravelSeg* currSegment,
                                         const TravelSeg* nextSegment);
  static bool intralineAvailabilityApply(const PricingTrx& trx,
                                         const CarrierCode& current,
                                         const CarrierCode& next);
  static bool isIntralineAvailabilityCxr(const PricingTrx& trx, const CarrierCode& cxr);

  static void getIntralineAvailabilityCxrsNPartners(const PricingTrx&, IntraMap&);
  static void getInterlineAvailabilityCxrsNPartners(const PricingTrx& trx, InterMap&);

  static const TravelSeg*
  getInterlineAvailNextTravelSeg(const TravelSeg* currTvlSeg, const std::vector<TravelSeg*>& tsVec);

  static bool cat35LtypeEnabled(const PricingTrx& trx);

  static bool tfdNetRemitFareCombEnabled(const PricingTrx& trx);

  static bool optimusNetRemitEnabled(const PricingTrx& trx);

  static bool isJcbCarrier(const CarrierCode* carrier);

  static bool reuseFarePUResult();
  static bool isAdjustRexTravelDateEnabled();

  static bool isExchangeOrTicketing(PricingTrx& trx);

  static void createTrxAborter(Trx& trx);

  static bool isDisableYYForExcItin(const PricingTrx& trx);
  static bool taxEnhancementActivated(const DateTime& dateChk);

  static bool swsPoAtsePath(const PricingTrx& trx);
  static bool pssPoAtsePath(const PricingTrx& trx);
  static bool libPoAtsePath(const PricingTrx& trx);

  static DynamicConfigurableEffectiveDate& getIata302BaggageActivationDate(const PricingTrx& trx);
  static bool isIata302BaggageActivated(const PricingTrx& trx, const DateTime& trxDate);
  static bool isIata302BaggageActivated(const PricingTrx& trx);
  static DynamicConfigurableEffectiveDate& getBaggage302DotActivationDate(const PricingTrx& trx);
  static bool isBaggage302DotActivated(const PricingTrx& trx);
  static DynamicConfigurableFlagOn& isBaggage302CarryOnActivatedEnabled(const PricingTrx& trx);
  static DynamicConfigurableFlagOn& isBaggage302ExchangeActivatedEnabled(const PricingTrx& trx);
  static bool isBaggage302CarryOnActivated(const PricingTrx& trx);
  static bool isBaggage302ExchangeActivated(const PricingTrx& trx);
  static DynamicConfigurableFlagOn& isBaggageActivationXmlResponseEnabled(const PricingTrx& trx);
  static bool isBaggageActivationXmlResponse(const PricingTrx& trx);
  static DynamicConfigurableFlagOn& isBaggage302EmbargoesActivatedEnabled(const PricingTrx& trx);
  static bool isBaggage302EmbargoesActivated(const PricingTrx& trx);
  static DynamicConfigurableFlagOn&
  isBaggage302GlobalDisclosureActivatedEnabled(const PricingTrx& trx);
  static bool isBaggage302GlobalDisclosureActivated(const PricingTrx& trx);
  static bool isBaggage302NewUSDotMethodActivated(const PricingTrx& trx);
  static bool isBaggageCTAMandateActivated(const PricingTrx& trx);
  static bool isIataReso302MandateActivated(const PricingTrx& trx);
  static DynamicConfigurableFlagOn& getBaggageDotPhase2ADisplayEnabled(const PricingTrx& trx);
  static bool isBaggageDotPhase2ADisplayEnabled(const PricingTrx& trx);
  static bool isBaggageBTAActivated(const PricingTrx& trx);
  static bool isUSAirAncillaryActivated(const PricingTrx& trx);
  static bool isPrepaidBaggageActivated(const PricingTrx& trx);
  static bool isBaggageChargesInMipActivated(const PricingTrx& trx);
  static bool isIataFareSelectionActivated(const PricingTrx& trx);
  static bool isIataFareSelectionApplicable(PricingTrx* trx);
  static bool isAtpcoRbdByCabinAnswerTableActivated(const PricingTrx& trx);
  static bool isProject331PlusDaysEnabled(const PricingTrx& trx);
  static bool isFqS8BrandedServiceActivated(const PricingTrx& trx);
  static bool isPricingInfiniCat05ValidationSkipActivated(const PricingTrx& trx);
  static bool isFullMapRoutingActivated(const PricingTrx& trx);
  static bool isTaxesNewHeaderASActive(const PricingTrx& trx);
  static bool isTaxExemptionForChildActive(const PricingTrx& trx);
  static bool isSpecialOpenJawActivated(const PricingTrx& trx);
  static bool isCosExceptionFixActivated(const PricingTrx& trx);
  static bool isSplitTaxesByFareComponentEnabled(const PricingTrx& trx);
  static bool isBaggageInPQEnabled(const PricingTrx& trx);

  static bool isMultiTicketPricingEnabled(const PricingTrx& trx);
  static bool isSupplementChargeEnabled(const PricingTrx& trx);
  static bool isAtpcoTTProjectHighRTEnabled(const PricingTrx& trx);
  static bool cmdPricingTuningEnabled(const PricingTrx& trx);
  static uint32_t getConfigOBFeeOptionMaxLimit();
  static std::string supplementChargeCarrierListData(const PricingTrx& trx);
  static std::string supplementHdrMsgFareDisplayData(const PricingTrx& trx);
  static std::string subCodeDefinitionsData(const PricingTrx& trx);
  static std::string gtcCarrierListData(const PricingTrx& trx);
  static std::string amChargesTaxesListData(const PricingTrx& trx);

  static bool isDnataEnabled(const PricingTrx& trx);
  static bool isMarkupSecFilterEnhancementEnabled(const PricingTrx& trx);
  static bool isEMDSPhase2Activated(const PricingTrx& trx);
  static bool isFVOSurchargesEnabled();

  static uint32_t getMemCheckInterval(const Trx& trx);
  static uint32_t getMemCheckTrxInterval(const Trx& trx);
  static uint32_t getShoppingMemCheckInterval(const Trx& trx);
  static uint32_t getShoppingPercentageMemGrowthCheckInterval(const Trx& trx);
  static uint32_t permCheckInterval(const PricingTrx& trx);
  static uint32_t referenceRSS(const Trx& trx);
  static uint32_t referenceVM(const Trx& trx);
  static uint32_t maxRSSGrowth(const Trx& trx);
  static uint32_t maxVMGrowth(const Trx& trx);
  static uint32_t yqyrPrecalcLimit(const Trx& trx);
  static uint32_t abortCheckInterval(const PricingTrx& trx);
  static uint32_t maxCat31SolutionCount(const PricingTrx& trx);
  static uint32_t getMaxNumberOfSolutions(const PricingTrx& trx);
  static bool needMetricsInResponse();
  static bool needMetricsInResponse(const char* metricsDsc);

  static bool isNetRemitEnabled(const PricingTrx& trx);
  static bool isRexOrCsoTrx(const PricingTrx& trx);
  static bool isBspUser(const PricingTrx& trx);

  static bool isTOCAllowed(PricingTrx& trx);
  static bool isTOBAllowed(PricingTrx& trx);
  static bool isTOEAllowed(PricingTrx& trx);
  static bool isTOEAllowed(PricingTrx& trx, const DateTime& date);
  static bool isTOIAllowed(PricingTrx& trx);
  static bool isTOIAllowed(PricingTrx& trx, const DateTime& date);
  static bool isTCHAllowed(PricingTrx& trx);
  static bool isBOTAllowed(PricingTrx& trx);

  static bool isPricingTaxRequest(const PricingTrx* pricingTrx);
  static bool isShoppingTaxRequest(const PricingTrx* pricingTrx);

  static bool getValidatingCxrFbcDisplayPref(const PricingTrx& trx, const FarePath& farePath);
  static void setInfiniCat05BookingDTSkipFlagInItins(PricingTrx& trx);
  static bool isCat35TFSFEnabled(const PricingTrx& trx);
  static bool isHpsDomGroupingEnabled(const PricingTrx& trx);
  static bool isHpsIntlGroupingEnabled(const PricingTrx& trx);
  static bool useHPSFlightRanges(const PricingTrx& trx);
  static bool isFareSelectionForSubIata21Only(const PricingTrx& trx);
  static bool isFareSelectionForNGSThruFMOnly(const PricingTrx& trx);
  static bool isRbdByCabinValueInShopResponse(const PricingTrx& trx);
  static bool isRbdByCabinValueInPriceResponse(const PricingTrx& trx);
  static bool isAutomaticPfcTaxExemptionEnabled(const PricingTrx& trx);
  static bool areUSTaxesOnYQYREnabled(const PricingTrx& trx);
  static CurrencyCode getEquivCurrencyCode(const PricingTrx& trx);
  static bool isSimpleTripForMIP(const PricingTrx& trx);
  static bool isHistorical(const PricingTrx& trx);
  static bool isAtpcoBackingOutTaxesEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesDisplayEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnChangeFeeEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnItinEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnChangeFeeServiceFlagEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnItinServiceFlagEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnOcEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesOnBaggageEnabled(const PricingTrx& trx);
  static bool isAtpcoTaxesDefaultRoundingEnabled(const PricingTrx& trx);
  static bool isTotalPriceEnabled(PricingTrx& trx);
  static bool isTotalPriceEnabledForREX(PricingTrx& trx);
  static OBFeeSubType getOBFeeSubType(const ServiceSubTypeCode code);
  static bool isRequestFromAS(const PricingTrx& trx);
  static bool isPerformOACCheck(const PricingTrx& trx);
  static bool isDynamicConfigOverrideEnabled(const Trx& trx);
  static bool isDynamicConfigOverridePermament(const Trx& trx);
  static bool isJumpCabinLogicDisableCompletely(const Trx& trx);
  static bool isFqFareRetailerEnabled(const Trx& trx);
  static size_t getMinAvailableMemoryTrx(const Trx& trx);
  static size_t getMaxRSSPercentageMemoryTrx(const Trx& trx);

  static bool
  isEnoughAvailableMemory(const size_t minAvailableMemory, const size_t maxRSSPercentage);
  static void checkTrxMemoryFlag(PricingTrx& trx);
  static void checkTrxMemoryLimits(PricingTrx& trx);
  static void checkTrxMemoryGrowth(PricingTrx& trx);
  static void checkTrxMemoryManager(PricingTrx& trx);
  static void checkTrxMemoryAborted(PricingTrx& trx,
                                    const uint32_t count,
                                    const uint32_t memGrowCheckInterval,
                                    const uint32_t memAvailCheckInterval);

  static bool isAAAwardPricing(const PricingTrx& trx);
  static bool isIcerActivated(const PricingTrx& trx);
  static bool isIcerActivated(const Trx& trx, const DateTime& dateChk);
  static bool isFrequentFlyerTierActive(const Trx& trx);
  static bool isFrequentFlyerTierOBActive(const Trx& trx);

  static bool isEmdValidationForM70Active(const Trx& trx);
  static bool isEmdValidationOnReservationPathForAB240Active(const Trx& trx);
  static bool isEmdValidationFlightRelatedOnCheckingPathForAB240Active(const Trx& trx);
  static bool isEmdValidationFlightRelatedServiceAndPrepaidBaggageActive(const Trx& trx);
  static bool isEmdValidationInterlineChargeChecksActivated(const Trx& trx);
  static bool isEmdValidationRelaxedActivated(const Trx& trx);

  static bool usesPrecalculatedTaxes(const PricingTrx& trx);
  static bool isdiffCntyNMLOpenJawActivated(const PricingTrx& trx);

  static bool newDiscountLogic(const PricingRequest& request, const Trx& trx);
  static bool newDiscountLogic(const PricingTrx& trx);

  static std::string getOverrideBsResponse(const PricingTrx& pricingTrx);

  static uint32_t getMaxPenaltyFailedFaresThreshold(const Trx& trx);
  static uint32_t getRec2Cat35SegCountLimit(const Trx& trx);
  static bool isAutomatedRefundCat33Enabled(const PricingTrx& trx);
  static bool isAbacusEndorsementHierarchyAtpcoFaresActive(const PricingTrx& trx);
  static bool isInfiniEndorsementHierarchyAtpcoFaresActive(const PricingTrx& trx);
  static bool isDateAdjustmentIndicatorActive(const PricingTrx& trx);
  static bool isControlConnectIndicatorActive(const PricingTrx& trx);
  static int16_t getASBaseTaxEquivTotalLength(const Trx& trx);

protected:
  static void configureInterlineAvailabilityCxrsPartners(const PricingTrx&, InterMap&);

  static void configureIntralineAvailabilityCxrsPartners(const PricingTrx&, IntraMap&);

  static void configureTaxEnhancementActivationDate();

  static void getMetricsToRecordInfo();

  static void configureTamPriorityActivationDate();

  static bool convertYYYYMMDDDate(std::string& dateStr, DateTime& outDate);

  static bool checkSchemaVersionForGlobalDisclosure(const PricingTrx& pricingTrx);

  static DynamicConfigurableFlagOn& isBaggageBTAActivationEnabled(const PricingTrx& trx);

  static CurrencyCode errorCCode;
  static DateTime today;
  static bool abacusEnabled;

  static std::map<std::string, std::vector<CarrierCode>> intralineAvailabilityCxrs;
  static std::atomic<bool> intralineAvailabilityCxrsConfigured;

  static std::map<CarrierCode, std::vector<CarrierCode>> interlineAvailabilityCxrs;
  static std::atomic<bool> interlineAvailabilityCxrsConfigured;

  static DateTime _taxEnhancementActivationDate;
  static volatile bool _taxEnhancementActivationDateConfigured;

  static std::vector<std::string> _recordingMetricsDescription;
  static volatile bool _recordingMetricsConfigured;

private:
  static bool isRepricingForTaxShoppingDisabled(const PricingTrx* trx);

  // since it is used only once at begining of the server's life time
  // same mutex is used to synchronize intialization of 3 static data
  //
  static boost::mutex _mutex;
};
} // end tse namespace
