//----------------------------------------------------------------------------
//
//      File: PricingResponseFormatter.h
//      Description: Class to format Pricing responses back to sending client
//      Created: February 17, 2005
//      Authors: Mike Carroll
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

#include "Common/DateTime.h"
#include "Common/Message.h"
#include "Common/Money.h"
#include "Common/MultiTicketUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/XMLConstruct.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"
#include "FareCalc/FcMessage.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "Xform/ResponseFormatter.h"

#include <functional>
#include <vector>

namespace tse
{
class BaggageResponseBuilder;
class BaggageTravel;
class FareUsage;
class MaxPenaltyResponse;
class MinFarePlusUpItem;
class NegPaxTypeFareRuleData;
class OCFees;
class OCFeesUsage;
class PaxTypeFareRuleData;
class PricingTrx;
class RefundPermutation;
class RexBaseTrx;
struct PaxOCFees;
struct PaxOCFeesUsages;

using FuFcIdMap = std::map<FareUsage*, uint16_t>;
class PricingResponseFormatter : public ResponseFormatter
{
  friend class PricingResponseFormatterTest;

public:
  PricingResponseFormatter();

  // depricated, should be removed with fallback EXTRACT_REISSUE_EXCHANGE_FORMATTER
  using PricingUnits = std::vector<PricingUnit*>;

  struct BaggageSizeWeightDescription
  {
    BaggageSizeWeightDescription(const ServiceGroupDescription& serviceDescription,
                                 float baggageSizeWeightLimit,
                                 const char& baggageSizeWeightUnitType,
                                 const char& baggageSizeWeightLimitType)
    {
      _serviceDescription = serviceDescription, _baggageSizeWeightLimit = baggageSizeWeightLimit;
      _baggageSizeWeightUnitType = baggageSizeWeightUnitType;
      _baggageSizeWeightLimitType = baggageSizeWeightLimitType;
    }

    ServiceGroupDescription _serviceDescription;
    float _baggageSizeWeightLimit;
    char _baggageSizeWeightUnitType;
    char _baggageSizeWeightLimitType;
  };

  virtual std::string formatResponse(
      const std::string& responseString,
      bool displayOnly,
      PricingTrx& pricingTrx,
      FareCalcCollector* fareCalcCollector,
      ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);

  static void
  formatResponse(const ErrorResponseException& ere, bool diplayOnly, std::string& response);
  static void formatRexResponse(const ErrorResponseException& ere, std::string& response);
  static std::string getDirectionality(const PaxTypeFareRuleData* ptfRuleData);

  virtual void buildValidatingCarrierList(const PricingTrx& pricingTrx,
                                          XMLConstruct& construct,
                                          const FarePath& farePath);
  void buildValidatingCarrier(XMLConstruct& construct,
                              const Itin& itin,
                              const CarrierCode& vcxr,
                              bool isDefaultValidatingCxr = false);

  virtual void prepareValidatingCarrierLists(PricingTrx& trx,
                                             XMLConstruct& construct,
                                             const FarePath& farePath);

  virtual void buildValidatingCarrierList(XMLConstruct& construct,
                                          const FarePath& farePath,
                                          const SettlementPlanType& sp,
                                          const CarrierCode& defaultValCxr,
                                          const std::string& inhibitPq);

  virtual void getGlobalDirection(const PricingTrx* trx,
                                  DateTime travelDate,
                                  const std::vector<TravelSeg*>& tvlSegs,
                                  GlobalDirection& globalDir) const;

  void buildValidatingCarrier(XMLConstruct& construct,
                              const CarrierCode& vcxr,
                              const vcx::ValidatingCxrData& vcxrData,
                              bool isDefaultValidatingCxr = false);

  bool isAnyDefaultVcxrEmpty(const FarePath& farePath) const;

  void formatElementsInCAL(PricingTrx& pricingTrx,
                           CalcTotals& calcTotals,
                           const FareUsage& fareUsage,
                           const PricingUnit& pricingUnit,
                           const CurrencyNoDec& noDecCalc,
                           const FarePath& farePath,
                           uint16_t& segmentOrder,
                           XMLConstruct& construct,
                           std::vector<const FareUsage*>& fuPlusUpsShown,
                           CalcTotals::FareCompInfo& fareCompInfo);

  size_t numOfValidCalcTotals(PricingTrx& pricingTrx,
                              FareCalcCollector& fareCalcCollector);

protected:
  struct SubCodeRestrictionsComparator
      : std::unary_function<const BaggageSizeWeightDescription*, bool>
  {
  public:
    SubCodeRestrictionsComparator(const char unit,
                                  const char alternativeUnit,
                                  const char limitType);

    bool operator()(const BaggageSizeWeightDescription* baggageSizeWeight) const;

  private:
    const char _unit;
    const char _alternativeUnit;
    const char _limitType;
  };

  void adjustDiagnosticResponseSize(const Diagnostic& diag);

  void formatProperResponseWhetherMTActive(ErrorResponseException::ErrorResponseCode tktErrCode,
                                           const std::string& responseString,
                                           bool displayOnly,
                                           PricingTrx& pricingTrx,
                                           XMLConstruct& construct,
                                           FareCalcCollector* fareCalcCollector);

  virtual void formatPricingResponse(XMLConstruct& construct,
                                     const std::string& responseString,
                                     bool displayOnly,
                                     PricingTrx& pricingTrx,
                                     FareCalcCollector* fareCalcCollector,
                                     ErrorResponseException::ErrorResponseCode tktErrCode,
                                     bool prepareAgentAndBilling = true);

  virtual void formatPricingResponseMTActive(XMLConstruct& construct,
                                             const std::string& responseString,
                                             bool displayOnly,
                                             PricingTrx& pricingTrx,
                                             FareCalcCollector* fareCalcCollector,
                                             ErrorResponseException::ErrorResponseCode tktErrCode);

  virtual void formatPricingResponseMTOffered(XMLConstruct& construct,
                                              const std::vector<std::string>& respMsgVec,
                                              bool displayOnly,
                                              PricingTrx& pricingTrx,
                                              FareCalcCollector* fareCalcCollector,
                                              ErrorResponseException::ErrorResponseCode tktErrCode,
                                              MultiTicketUtil::TicketSolution ts);

  virtual void formatOCFeesResponse(XMLConstruct& construct, PricingTrx& pricingTrx, ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger = nullptr);

  virtual void formatResponseIfRedirected(XMLConstruct& construct,
                                          const std::string& responseString,
                                          RexBaseTrx& rexTrx,
                                          FareCalcCollector* fareCalcCollector,
                                          ErrorResponseException::ErrorResponseCode tktErrCode);

  void prepareSummaryForTaxInfo(RexBaseTrx& trx, XMLConstruct& construct) const;

  static constexpr int exchangeRatePrecision = 13;

  virtual void prepareAgent(PricingTrx& pricingTrx, XMLConstruct& construct);

  virtual void prepareBilling(PricingTrx& pricingTrx,
                              FareCalcCollector& fareCalcCollector,
                              XMLConstruct& construct);
  void addBillingElements(PricingTrx& pricingTrx, const Billing& billing, XMLConstruct& construct);
  void prepareChangeFee(RexPricingTrx& pricingTrx, XMLConstruct& construct, CalcTotals& calcTotals);

  void addReissueExchangeSectionForRefund(const PricingTrx& trx,
                                          XMLConstruct& construct,
                                          const RefundPermutation& winnerPerm,
                                          const PricingUnits& pricingUnits);

  void addChangeFeesForRefund(const PricingTrx& trx,
                              XMLConstruct& construct,
                              const RefundPermutation& winnerPerm);

  //remove with fallback taxRexPricingTxType
  virtual void formatResponseForFullRefund(XMLConstruct& construct,
                                           const std::string& responseString,
                                           RefundPricingTrx& trx,
                                           ErrorResponseException::ErrorResponseCode tktErrCode);

  virtual void formatResponseForFullRefund(XMLConstruct& construct,
                                           const std::string& responseString,
                                           RexBaseTrx& rexTrx,
                                           ErrorResponseException::ErrorResponseCode tktErrCode,
                                           bool taxInfoRequest);

  void prepareAgentForFullRefund(RexBaseTrx& trx, XMLConstruct& construct);
  void prepareBillingForFullRefund(RexBaseTrx& trx, XMLConstruct& construct);
  void prepareSummaryForFullRefund(RefundPricingTrx& trx, XMLConstruct& construct);
  void preparePassengerForFullRefund(RefundPricingTrx& trx, XMLConstruct& construct);

  void
  addZeroFee(const PricingTrx& trx, XMLConstruct& construct, const RefundPermutation& winnerPerm);
  void addNonZeroFee(const PricingTrx& trx,
                     XMLConstruct& construct,
                     const Money& fee,
                     bool highestFee,
                     bool nonRefundable);

  void prepareValidatingCarrierAttr(const PricingTrx& pricingTrx, XMLConstruct& construct);

  void prepareCommonSummaryAttrs(PricingTrx& pricingTrx,
                                 FareCalcCollector& fareCalcCollector,
                                 XMLConstruct& construct,
                                 CalcTotals* calcTotals = nullptr);

  virtual void prepareSummary(PricingTrx& pricingTrx,
                              FareCalcCollector& fareCalcCollector,
                              XMLConstruct& construct,
                              const char* itinNumber = nullptr);

  virtual void
  prepareRexSummary(RexPricingTrx* rexTrx, XMLConstruct& construct, FareCalcCollector& originalFCC);

  void callPrepareSummaryForRex(RexPricingTrx* rexTrx,
                                FarePath& farePath,
                                const FareCalcConfig& fcConfig,
                                FareCalcCollector& originalFCC,
                                XMLConstruct& construct,
                                const char* itinNumber);

  virtual void preparePassengers(PricingTrx& pricingTrx,
                                 FareCalcCollector& fareCalcCollector,
                                 XMLConstruct& construct);

  void buildPartcipatingCarrier(XMLConstruct& construct, const vcx::ParticipatingCxr& pcx);

  void prepareMessages(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void prepareEndorsementMessages(PricingTrx& pricingTrx,
                                  const FarePath* farePath,
                                  XMLConstruct& construct);

  void prepareFOPwarningMessages(PricingTrx& pricingTrx,
                                 CalcTotals& calcTotals,
                                 XMLConstruct& construct);

  void prepareGIMessage(PricingTrx& pricingTrx, XMLConstruct& construct);

  void prepareTaxes(PricingTrx& pricingTrx,
                    CalcTotals& calcTotals,
                    XMLConstruct& construct,
                    bool& taxExemptProcessed);

  void
  prepareTaxesOnChangeFee(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void prepareExemptTaxes(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void prepareTaxBSR(PricingTrx& pricingTrx,
                     CalcTotals& calcTotals,
                     const CurrencyNoDec& noDec,
                     XMLConstruct& construct);

  void prepareFareIATARate(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void prepareFareBSR(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void prepareFareCalcs(PricingTrx& pricingTrx,
                        CalcTotals& calcTotals,
                        const FareUsage& fareUsage,
                        const PricingUnit& pricingUnit,
                        const CurrencyNoDec& noDecCalc,
                        const FarePath& farePath,
                        bool stopoverFlag,
                        uint16_t puCount,
                        uint16_t fcCount,
                        uint16_t& segmentOrder,
                        XMLConstruct& construct,
                        std::vector<const FareUsage*>& fusAlreadyShown,
                        std::vector<MoneyAmount>& faeValues,
                        std::vector<MoneyAmount>& ftsValues,
                        const FuFcIdMap& fuFcIdCol);

  void addFareBrandDetails(const PricingTrx& pricingTrx,
                           const PaxTypeFare& pax,
                           XMLConstruct& construct,
                           Direction fareUsageDirection) const;

  void addSearchForBrandsPricingFareBrandDetails(PricingTrx& pricingTrx,
                                                 CalcTotals& calcTotals,
                                                 const FareUsage& fareUsage,
                                                 XMLConstruct& construct) const;

  void prepareSegments(PricingTrx& pricingTrx,
                       CalcTotals& calcTotals,
                       const FareUsage& fareUsage,
                       const FarePath& farePath,
                       const PricingUnit& pricingUnit,
                       uint16_t& segmentOrder,
                       const CurrencyNoDec& noDecCalc,
                       XMLConstruct& construct);

  void prepareHPUForNet(PricingTrx& pricingTrx,
                  const FareUsage& fareUsage,
                  XMLConstruct& construct);

  void prepareHPUForAdjusted(PricingTrx& pricingTrx,
                  const FareUsage& fareUsage,
                  XMLConstruct& construct);

  void prepareArunk(PricingTrx& pricingTrx,
                    const FarePath& farePath,
                    const FareUsage& fareUsage,
                    const PricingUnit& pricingUnit,
                    const Itin* itin,
                    uint16_t& segmentOrder,
                    XMLConstruct& construct);

  void prepareSegmentSideTrips(PricingTrx& pricingTrx,
                               const FarePath& farePath,
                               const PricingUnit& pricingUnit,
                               const Itin* itin,
                               const TravelSeg& tvlSeg,
                               uint16_t& segmentOrder,
                               XMLConstruct& construct);

  void
  prepareScheduleInfo(PricingTrx& pricingTrx, const TravelSeg& tvlSeg, XMLConstruct& construct,
                                     const  FareUsage& fUsage, bool isRebooked = false);

  bool generateSegmentSideTripAttributes(PricingTrx& pricingTrx,
                                         const FarePath& farePath,
                                         const TravelSeg* travelSeg,
                                         XMLConstruct& construct);

  void addOAndDTags(PricingTrx& pricingTrx,
                    const FarePath& farePath,
                    const Itin* itin,
                    const TravelSeg& tvlSeg,
                    const FareUsage& fareUsage,
                    const bool rebooked,
                    XMLConstruct& construct);

  void prepareSegmentsCity(PricingTrx& pricingTrx,
                           const FarePath& farePath,
                           const Itin* itin,
                           const TravelSeg& tvlSeg,
                           const FareUsage& fareUsage,
                           const bool rebooked,
                           XMLConstruct& construct);

  void prepareSurcharges(const TravelSeg& tvlSeg,
                         CalcTotals& calcTotals,
                         const CurrencyNoDec& noDecCalc,
                         XMLConstruct& construct);

  void prepareMileage(const TravelSeg& tvlSeg, CalcTotals& calcTotals, XMLConstruct& construct);

  bool displayTfdPsc(const PricingTrx& pricingTrx, const FareUsage& fareUsage) const;

  void prepareTfdpsc(PricingTrx& pricingTrx,
                     const TravelSeg* tvlSeg,
                     const FareUsage& fareUsage,
                     XMLConstruct& construct);

  void prepareNetRemitTrailerMsg(PricingTrx& pricingTrx,
                                 const FarePath& farePath,
                                 const FareUsage& fareUsage,
                                 XMLConstruct& construct,
                                 CalcTotals& calcTotals);

  void prepareFareVendorSource(const FareUsage& fareUsage,
                               const NegPaxTypeFareRuleData* ruleData,
                               XMLConstruct& construct);

  void prepareNegotiatedFbc(PricingTrx& pricingTrx,
                            const FareUsage& fareUsage,
                            XMLConstruct& construct,
                            CalcTotals& calcTotals);

  void prepareCountryCodes(const TravelSeg& tvlSeg, XMLConstruct& construct);

  void prepareTransferCharges(const TravelSeg& tvlSeg,
                              CalcTotals& calcTotals,
                              const FareUsage& fareUsage,
                              const CurrencyNoDec& noDecCalc,
                              XMLConstruct& construct);

  void prepareFarePathPlusUps(const FarePath& farePath,
                              const CurrencyNoDec& noDecCalc,
                              XMLConstruct& construct);

  void preparePricingUnitPlusUps(const PricingUnit& pricingUnit,
                                 const CurrencyNoDec& noDecCalc,
                                 PricingTrx& pricingTrx,
                                 XMLConstruct& construct);

  void prepareFareUsagePlusUps(const FareUsage& fareUsage,
                               const CurrencyNoDec& noDecCalc,
                               XMLConstruct& construct,
                               std::vector<const FareUsage*>& fusAlreadyShown);

  void preparePupElement(const std::string& payload,
                         const MinFarePlusUpItem& plusUp,
                         const CurrencyNoDec& noDecCalc,
                         const NationCode& countryOfPmt,
                         XMLConstruct& construct);

  bool isCategory25Applies(uint16_t category, const PaxTypeFare& ptFare) const;

  void prepareRuleCategoryIndicator(const PricingTrx& pricingTrx,
                                    const FareUsage& fareUsage,
                                    XMLConstruct& construct);

  void prepareDifferential(PricingTrx& pricingTrx,
                           const FareUsage& fareUsage,
                           const uint16_t noDecCalc,
                           XMLConstruct& construct);

  void prepareStopoverSegment(const PricingTrx& pricingTrx,
                              const TravelSeg& tvlS,
                              CalcTotals& calcTotals,
                              XMLConstruct& construct);

  bool prepareCat27TourCode(const FarePath& farePath, uint16_t paxNumber, XMLConstruct& construct);

  void prepareCat35Ticketing(PricingTrx& pricingTrx,
                             const FarePath& farePath,
                             uint16_t paxNumber,
                             const tse::CurrencyNoDec& noDecCalc,
                             const tse::CurrencyNoDec& baseNoDecCalc,
                             bool cat27TourCode,
                             XMLConstruct& construct,
                             bool netSellTkt,
                             bool& isUnableToOverrideNegDataForComm);

  virtual void prepareResponseText(const std::string& responseString,
                                   XMLConstruct& construct,
                                   bool noSizeLImit = false,
                                   void (PricingResponseFormatter::*)(XMLConstruct& construct,
                                                                      const char msgType,
                                                                      const uint16_t msgCode,
                                                                      const std::string& msgText)
                                   const = &PricingResponseFormatter::prepareMessage,
                                   const char MessageType = Message::TYPE_GENERAL) const;

  virtual void prepareErrorMessage(PricingTrx& pricingTrx,
                                   XMLConstruct& construct,
                                   ErrorResponseException::ErrorResponseCode errCode,
                                   const std::string& msgText);

  void prepareErrorMessageWithErrorCode(PricingTrx& pricingTrx,
                                        XMLConstruct& construct,
                                        ErrorResponseException::ErrorResponseCode errCode,
                                        const std::string& msgText);

  void prepareSimpleMessage(XMLConstruct& construct,
                            const char msgType,
                            const uint16_t msgCode,
                            const std::string& msgText) const;

  void prepareMessage(XMLConstruct& construct,
                      const char msgType,
                      const uint16_t msgCode,
                      const std::string& msgText) const;

  void prepareMessage(XMLConstruct& construct,
                      const char msgType,
                      const std::vector<std::string>& msgTextVec);

  void prepareMessage(XMLConstruct& construct,
                      const char msgType,
                      const uint16_t msgCode,
                      const std::string& airlineCode,
                      const std::string& msgText);

  void prepareMessage(PricingTrx& pricingTrx, XMLConstruct& construct, const FcMessage& message);

  void getSaleLoc(PricingTrx& pricingTrx, LocCode& locCode) const;
  void getTicketLoc(PricingTrx& pricingTrx, LocCode& locCode) const;
  const PaxTypeCode getPaxType(const FareCalcConfig& fcConfig, CalcTotals& calcTotals);

  void preparePassengerInfo(PricingTrx& pricingTrx,
                            const FareCalcConfig& fcConfig,
                            CalcTotals& calcTotals,
                            uint16_t paxNumber,
                            XMLConstruct& construct);

  void prepareMarkupAndCommissionAmount(PricingTrx& pricingTrx,
                                        XMLConstruct& construct,
                                        const FarePath& fp,
                                        const CurrencyNoDec noDec) const;

  void prepareMarkupAndCommissionAmountOld(XMLConstruct& construct,
                                           const FarePath& fp,
                                           const CurrencyNoDec noDec) const;

  void addPassengerInfoTag(const FareCalcConfig& fcConfig,
                           CalcTotals& calcTotals,
                           XMLConstruct& construct) const;

  void prepareCommissionForValidatingCarriers(
      PricingTrx& trx,
      XMLConstruct& construct,
      const FarePath& fp,
      FuFcIdMap& fuFcIdCol,
      const CurrencyNoDec noDec) const;

  void constructElementVCC(PricingTrx& trx,
                           XMLConstruct& xml,
                           const FarePath& fp,
                           const CarrierCode& cxrs,
                           const MoneyAmount& commAmount,
                           const CurrencyNoDec noDec,
                           FuFcIdMap& fuFcIdCol,
                           uint16_t& fcInd) const;

  void constructElementVCCForCat35(
      PricingTrx& trx,
      XMLConstruct& construct,
      const FarePath& fp,
      const CurrencyNoDec noDec) const;

  void constructElementFCB(const amc::FcCommissionData& fcCommInfo,
                           uint16_t q6d,
                           const CurrencyNoDec noDec,
                           XMLConstruct& xml) const;

  void prepareHPSItems(PricingTrx& trx, XMLConstruct& xml, CalcTotals& calcTotals) const;

  void prepareAdjustedCalcTotal(PricingTrx& pricingTrx, XMLConstruct& xml, CalcTotals& calcTotals);

  void prepareMaxPenaltyResponse(PricingTrx& pricingTrx,
                                 const FarePath& farePath,
                                 const MaxPenaltyResponse& maxPenaltyResponse,
                                 const DateTime& ticketingDate,
                                 XMLConstruct& construct);

  void electronicTicketInd(const Indicator& electronicTicketIndicator, XMLConstruct& construct);

  virtual void addAdditionalPaxInfo(PricingTrx& pricingTrx,
                                    CalcTotals& calcTotals,
                                    uint16_t paxNumber,
                                    XMLConstruct& construct);

  virtual void prepareFarePath(PricingTrx& pricingTrx,
                               CalcTotals& calcTotals,
                               const CurrencyNoDec& noDecCalc,
                               const CurrencyNoDec& noDecEquiv,
                               uint16_t paxNumber,
                               bool stopoverFlag,
                               const FuFcIdMap& fuFcIdCol,
                               XMLConstruct& construct);

  void traverseTravelSegs(PricingTrx& pricingTrx,
                          CalcTotals& calcTotals,
                          const CurrencyNoDec& noDecCalc,
                          const CurrencyNoDec& noDecEquiv,
                          const FarePath& farePath,
                          uint16_t paxNumber,
                          bool stopoverFlag,
                          const FuFcIdMap& fuFcIdCol,
                          XMLConstruct& construct);

  virtual void scanTotalsItin(CalcTotals& calcTotals,
                              bool& fpFound,
                              bool& infantMessage,
                              char& nonRefundable,
                              MoneyAmount& moneyAmountAbsorbtion);

  void scanFarePath(const FarePath& farePath,
                    bool& infantMessage,
                    char& nonRefundable,
                    MoneyAmount& moneyAmountAbsorbtion);

  const CarrierCode getValidatingCarrier(const PricingTrx& pricingTrx) const;

  void addBillingInfo(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);

  void adjustSize(std::string& attr, const uint16_t len);

  void prepareHostPortInfo(PricingTrx& pricingTrx, XMLConstruct& construct);

  virtual void prepareUnflownItinPriceInfo(RexPricingTrx& rexTrx, XMLConstruct& construct);

  void setBuildPlusUp(PricingTrx& pricingTrx);

  void
  isStartOrEndOfSideTrip(const FarePath& fp, const TravelSeg* tvlSeg, bool& isStart, bool& isEnd);

  void prepareMinMaxTaxInfo(XMLConstruct& construct, const TaxItem& taxItem);

  void checkLimitOBFees(PricingTrx& pricingTrx, FareCalcCollector& fareCalcCollector);
  const FarePath* clearAllFeesAndGetLastPTC(const std::vector<CalcTotals*>& calcTotals) const;

  const TicketingFeesInfo* mockOBFeeInPaymentCurrency(PricingTrx& pricingTrx,
                                                      const TicketingFeesInfo& sourceFee,
                                                      const MoneyAmount& feeAmount,
                                                      const CurrencyCode& paymentCurrency) const;

  std::pair<const TicketingFeesInfo*, MoneyAmount>
  computeMaximumOBFeesPercent(PricingTrx& pricingTrx, const CalcTotals& calcTotals);
  MoneyAmount calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                                const CalcTotals& calcTotals,
                                                const TicketingFeesInfo* feeInfo) const;
  MoneyAmount calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                                    const CalcTotals& calcTotals,
                                                    const TicketingFeesInfo* feeInfo) const;
  bool checkForZeroMaximum(PricingTrx& pricingTrx, const CalcTotals& calcTotals) const;
  void prepareOBFees(PricingTrx& pricingTrx, CalcTotals& calcTotals, XMLConstruct& construct);
  void prepareAllOBFees(PricingTrx& pricingTrx,
                        CalcTotals& calcTotals,
                        XMLConstruct& construct,
                        size_t maxOBFeesOptions,
                        const std::vector<TicketingFeesInfo*>& collectedOBFees);

  void prepareOBFee(PricingTrx& pricingTrx,
                    CalcTotals& calcTotals,
                    XMLConstruct& construct,
                    const TicketingFeesInfo* feeInfo,
                    MoneyAmount feeAmt = 0.0,
                    MoneyAmount fareAmtWith2CCFee = 0.0) const;

  void prepareOBFee(PricingTrx& pricingTrx,
                    CalcTotals& calcTotals,
                    XMLConstruct& construct,
                    TicketingFeesInfo* feeInfo,
                    const FopBinNumber& fopBin,
                    const MoneyAmount& chargeAmount,
                    const CurrencyNoDec& numDec,
                    const MoneyAmount& feeAmt,
                    const MoneyAmount& fareAmtWith2CCFee) const;

  void prepare2CardsOBFee(PricingTrx& pricingTrx,
                          CalcTotals& calcTotals,
                          XMLConstruct& construct,
                          const std::vector<TicketingFeesInfo*>& collectedOBFees);

  void calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                      CalcTotals& calcTotals,
                                      XMLConstruct& construct,
                                      const TicketingFeesInfo* feeInfo,
                                      MoneyAmount& totalFeeFareAmount,
                                      bool fillXML = true) const;

  void calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                      CalcTotals& calcTotals,
                                      const TicketingFeesInfo* feeInfo,
                                      MoneyAmount& totalFeeFareAmount,
                                      CurrencyNoDec& noDec) const;

  void calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          XMLConstruct& construct,
                                          const TicketingFeesInfo* feeInfo,
                                          MoneyAmount& totalFeeFareAmount,
                                          const MoneyAmount& chargeAmount = 0.0) const;

  void calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          const TicketingFeesInfo* feeInfo,
                                          Money& targetMoney,
                                          MoneyAmount& totalFeeFareAmount,
                                          const MoneyAmount& chargeAmount = 0.0) const;

  void calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                          CalcTotals& calcTotals,
                                          const TicketingFeesInfo* feeInfo,
                                          MoneyAmount& totalFeeFareAmount,
                                          CurrencyNoDec& noDec,
                                          const MoneyAmount& chargeAmount = 0.0) const;

  void calculateObFeeAmount(PricingTrx& pricingTrx,
                            CalcTotals& calcTotals,
                            XMLConstruct& construct,
                            const TicketingFeesInfo* feeInfo,
                            MoneyAmount& totalFeeFareAmount,
                            const MoneyAmount& chargeAmount = 0.0) const;

  void calculateObFeeAmount(PricingTrx& pricingTrx,
                            CalcTotals& calcTotals,
                            const TicketingFeesInfo* feeInfo,
                            MoneyAmount& totalFeeFareAmount,
                            CurrencyNoDec& noDec,
                            const MoneyAmount& chargeAmount = 0.0) const;

  MoneyAmount calculateResidualObFeeAmount(PricingTrx& pricingTrx,
                                           const MoneyAmount& totalPaxAmount,
                                           const TicketingFeesInfo* feeInfo) const;

  MoneyAmount getLowestObFeeAmount(const CurrencyCode& maxFeeCur,
                                   const MoneyAmount& calcAmount,
                                   const MoneyAmount& maxAmount) const;

  void roundOBFeeCurrency(PricingTrx& pricingTrx, Money& targetMoney) const;

  void prepareLatencyDataInResponse(Trx& trx, XMLConstruct& construct);

  Money convertOBFeeCurrency(PricingTrx& pricingTrx,
                             const CalcTotals& calcTotals,
                             const TicketingFeesInfo* feeInfo) const;
  void
  convertOBFeeCurrency(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney) const;

  bool getFeeRounding(PricingTrx& pricingTrx,
                      const CurrencyCode& currencyCode,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule) const;

  Indicator cabinChar(Indicator& cabin) const;

  const std::string& logXmlData(const XMLConstruct& construct) const;
  std::string formatRexPricingResponse(const std::string& responseString,
                                       RexBaseTrx& trx,
                                       FareCalcCollector* fareCalcCollector,
                                       ErrorResponseException::ErrorResponseCode tktErrCode);
  void addDateAttr(const DateTime& date, std::string attr, XMLConstruct& construct);

  void formatOCHeaderMsg(XMLConstruct& construct);

  virtual bool
  isGenericTrailer(PricingTrx& pricingTrx, XMLConstruct& construct, const bool prefixInd);

  bool builTrailerOCF(PricingTrx& pricingTrx,
                      XMLConstruct& construct,
                      std::string msg,
                      const bool prefixInd);

  virtual void createOCGSection(PricingTrx& pricingTrx, XMLConstruct& construct);

  virtual bool checkIfAnyGroupValid(PricingTrx& pricingTrx);

  void replaceSpecialCharInDisplay(std::string& srvcGrpDesc);

  virtual bool addPrefixWarningForOCTrailer(PricingTrx& pricingTrx, bool warning);

  void formatOCFees(PricingTrx& pricingTrx, XMLConstruct& construct, const bool timeOutMax = false, ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger = nullptr);

  const char getFootnoteByHirarchyOrder(const OCFees& fee) const;

  void
  formatOCFeesGroups(PricingTrx& pricingTrx,
                     XMLConstruct& construct,
                     const std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFees>>>&
                         groupFeesVector,
                     const bool allFeesDisplayOnly,
                     const bool a = false);
  //
  void formatOCFeesGroups(
      PricingTrx& pricingTrx,
      XMLConstruct& construct,
      const std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>>&
          groupFeesVector,
      const bool allFeesDisplayOnly,
      const bool timeOutMax = false);

  const char getFootnoteByHirarchyOrder(const OCFeesUsage& fee) const;

  void formatOCFeesLine(PricingTrx& pricingTrx,
                        XMLConstruct& construct,
                        const PaxOCFeesUsages& paxOcFees,
                        const uint16_t index,
                        const char& indicator);

  void buildPNRData(const PricingTrx& pricingTrx,
                    XMLConstruct& construct,
                    const PaxOCFeesUsages& paxOcFees,
                    const uint16_t index,
                    const Money& EquivPrice);

  void buildPNRDataForNonDisplayFees(const PricingTrx& pricingTrx,
                                     XMLConstruct& construct,
                                     const PaxOCFeesUsages& paxOcFees);
  void processPNRSegmentNumber(const PricingTrx& pricingTrx,
                               XMLConstruct& construct,
                               const PaxOCFeesUsages& paxOcFees);
  void processTicketEffDate(const PricingTrx& pricingTrx,
                            XMLConstruct& construct,
                            const PaxOCFeesUsages& paxOcFees);

  void processTicketDiscDate(const PricingTrx& pricingTrx,
                             XMLConstruct& construct,
                             const PaxOCFeesUsages& paxOcFees);

  void buildOcTaxData(XMLConstruct& construct,
                      const OCFeesUsage* ocFees,
                      const bool isExemptAllTaxes = false,
                      const bool isExemptSpecificTaxes = false,
                      const std::vector<std::string>& taxIdExempted = std::vector<std::string>());

  //

  void buildST1data(std::string& st1Ind, const char* indicator);

  bool isTaxExempted(const std::string& taxCode,
                     const bool isExemptAllTaxes = false,
                     const bool isExemptSpecificTaxes = false,
                     const std::vector<std::string>& taxIdExempted = std::vector<std::string>());

  virtual DateTime calculatePurchaseByDate(const PricingTrx& pricingTrx);

  void checkPNRSegmentNumberLogic(const PricingTrx& pricingTrx,
                                  XMLConstruct& construct,
                                  int trvlStart,
                                  int trvlEnd);

  void formatOCTrailer(XMLConstruct& construct, const std::string& statusCode);
  void formatOCGenericMsg(XMLConstruct& construct, const std::string& msg);

  bool isOcFeesTrxDisplayOnly(const PricingTrx& pricingTrx);

  void isTimeOutBeforeStartOCFees(PricingTrx& trx, XMLConstruct& construct, const bool prefixInd);

  bool anyTimeOutMaxCharCountIssue(PricingTrx& trx, XMLConstruct& construct, const bool prefixInd);

  void timeOutMaxCharCountNoOCFeesReturned(PricingTrx& trx,
                                           XMLConstruct& construct,
                                           const bool prefixInd);

  void timeOutMaxCharCountRequestedOCFeesReturned(PricingTrx& trx,
                                                  XMLConstruct& construct,
                                                  const bool prefixInd);

  bool
  buildOCFeesFullResponse(PricingTrx& pricingTrx, XMLConstruct& construct, const bool prefixInd, ServiceFeeUtil::OcFeesUsagesMerger* ocFeesMerger = nullptr);

  bool isR7TuningAndWP(PricingTrx& pricingTrx, XMLConstruct& construct, const bool prefixInd);

  bool isAncillaryNonGuarantee(PricingTrx& pricingTrx, const FarePath* farePath);

  std::string getCommercialName(const OCFeesUsage* ocFeesUsage);

  void buildBDI(PricingTrx& pricingTrx,
                XMLConstruct& construct,
                const OCFees* ocFees,
                const BaggageProvisionType& baggageProvision,
                const std::set<int> segmentNumbers);

  void buildCarryOnAllowanceBDI(PricingTrx& pricingTrx,
                                XMLConstruct& construct,
                                const std::vector<const BaggageTravel*>& baggageTravels);

  void buildCarryOnChargesBDI(PricingTrx& pricingTrx,
                              XMLConstruct& construct,
                              const std::vector<const BaggageTravel*>& baggageTravels);

  void buildEmbargoesBDI(PricingTrx& pricingTrx,
                         XMLConstruct& construct,
                         const std::vector<const BaggageTravel*>& baggageTravels);

  void buildITR(PricingTrx& pricingTrx,
                XMLConstruct& construct,
                const OCFees* ocFees,
                const BaggageProvisionType& baggageProvision);

  void buildPFF(PricingTrx& pricingTrx, XMLConstruct& construct, const OCFees* ocFees);

  bool checkFFStatus(const OCFees* ocFees);

  void buildQ00(PricingTrx& pricingTrx, XMLConstruct& construct, int id);

  void populateSubCodeDefinitions(const PricingTrx& trx);
  void fetchSubCodeDefinitions(const PricingTrx& trx, std::vector<std::string>&);

  void formatBaggageResponse(PricingTrx& pricingTrx, XMLConstruct& construct);
  void buildOSC(PricingTrx& pricingTrx, const SubCodeInfo* subCode, XMLConstruct& construct) const;
  std::string buildExtendedSubCodeKey(const SubCodeInfo* subCodeInfo,
                                      const BaggageProvisionType& baggageProvision) const;
  void buildSubCodeDescription(PricingTrx& pricingTrx,
                               const SubCodeInfo* subCodeInfo,
                               XMLConstruct& construct) const;
  void buildSubCodeRestrictions(const SubCodeInfo* subCodeInfo, XMLConstruct& construct) const;
  void buildSubCodeRestrictionWithAlternativeUnits(
      const std::string& valueAtt,
      const std::string& unitAtt,
      const std::string& altValueAtt,
      const std::string& altUnitAtt,
      const SubCodeRestrictionsComparator& restrictionsComparator,
      const std::vector<const BaggageSizeWeightDescription*>& sizeWeightRestrictions,
      XMLConstruct& construct) const;
  void buildSubCodeRestriction(const std::string& valueAtt,
                               const float value,
                               const std::string& unitAtt,
                               const char unit,
                               XMLConstruct& construct) const;
  void getSubCodeRestrictions(const ServiceGroupDescription& description,
                              std::vector<const BaggageSizeWeightDescription*>& restrictions) const;

  bool insertBaggageData(XMLConstruct& construct,
                         BaggageResponseBuilder& baggageBuilder,
                         const std::string& baggageTag,
                         int& msgIndex,
                         PricingTrx& pricingTrx) const;
  void formatGreenScreenMsg(XMLConstruct& construct,
                            const std::string& responseString,
                            PricingTrx& pricingTrx,
                            const FareCalcCollector* fareCalcCollector) const;
  virtual bool validSchemaVersion(const PricingTrx& pricingTrx) const;
  void addUsDotItinIndicator(XMLConstruct& construct,
                             const Itin* itin,
                             const PricingTrx& pricingTrx) const;
  bool isOldPricingTrx(const PricingTrx& pricingTrx) const;
  bool checkForStructuredData(const PricingTrx& pricingTrx,
                              bool isBaggageXmlResponse,
                              bool isBaggageExchange,
                              bool isBaggageGlobalDisclosure) const;
  bool isXmlSizeLimit(XMLConstruct& construct, int msgIndext) const;
  bool insertPricingData(XMLConstruct& construct, int& msgIndex, const std::string& line) const;

  std::vector<BaggageSizeWeightDescription> _baggageSizeWeightRestrictions;

  static constexpr Indicator BLANK = ' ';
  static constexpr float NO_BAGGAGE_SIZE_WEIGHT = -1.0f;
  static const int32_t NO_BAGGAGE_PCS = -1;
  static constexpr char NO_BAGGAGE_SIZE_WEIGHT_UNIT = 0;
  static constexpr char NO_BAGGAGE_SIZE_WEIGHT_TYPE = 0;

  void buildSpanishDiscountIndicator(PricingTrx& pricingTrx,
                                     CalcTotals& calcTotals,
                                     XMLConstruct& construct) const;
  void setFopBinNumber(const FopBinNumber& formOfPayment,
                       XMLConstruct& construct,
                       std::string attributeName) const;
  void formatPricingCriteria(std::string&, PricingTrx&, uint16_t, XMLConstruct&);
  void buildTotalTTypeOBFeeAmount(PricingTrx& pricingTrx,
                                  CalcTotals& calcTotals,
                                  XMLConstruct& construct) const;

  Itin* getItin(const PricingTrx& trx) const;

  void appendCDataToResponse(Trx& trx, XMLConstruct& construct);

  void formatPricingResponseMTInactive(const std::string& responseString,
                                       bool displayOnly,
                                       ErrorResponseException::ErrorResponseCode tktErrCode,
                                       XMLConstruct& construct,
                                       PricingTrx& pricingTrx,
                                       FareCalcCollector* fareCalcCollector);

  Indicator getCommissionSourceIndicator(PricingTrx& pricingTrx,
                                         CalcTotals& calcTotals);

  CurrencyCode retrieveCurrencyCode(const CalcTotals* calc) const;
  bool shouldCalculateMaxOBFee(const uint32_t limit, const FareCalcCollector& collector) const;

protected:
  uint32_t _maxTotalBuffSize;
  DateTime _ticketingDate = DateTime::localTime();
  Itin* _itin = nullptr;

private:
  void processRuleCategoryIndicator(const PaxTypeFare& ptFare,
                                    const uint16_t category,
                                    bool& categoryFound,
                                    std::ostringstream& dataStream);

  bool isAltPricingTrxInVer(const PricingTrx& pricingTrx) const;

  bool isPricingTrxInVer(const PricingTrx& pricingTrx,
                         const short major,
                         const short minor,
                         const short revision) const;

  bool isTrxInProperVersion(const PricingTrx& pricingTrx,
                            const PricingTrx::AltTrxType trxType,
                            const short major,
                            const short minor,
                            const short revision) const;

  std::vector<OCFees::TaxItem>::const_iterator
  setEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems);

  void formatXMLTaxSplit(PricingTrx& pricingTrx,
                         CalcTotals& calcTotals,
                         const FareUsage& fareUsage,
                         XMLConstruct& construct) const;

  bool _buildPlusUp = true;
  bool _ancNonGuarantee = false;
  bool _limitMaxOBFees = false;

  std::set<const SubCodeInfo*> _subCodesForOSC;
  uint16_t _currentIndexForMT = 1;
}; // End class PricingResponseFormatter

} // End namespace tse
