//-------------------------------------------------------------------
//
//  File:        XMLShoppingResponse.h
//  Created:     January 20, 2005
//  Authors:     David White, Siriwan Chootongchai
//
//  Description: Implements generation of XML responses for shopping requests
//
//  Copyright Sabre 2005
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

#pragma once

#include "Common/AirlineShoppingUtils.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/Message.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMap.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/SOLItinGroups.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcMessage.h"
#include "Taxes/Common/AbstractTaxSplitter.h"
#include "Xform/ShoppingResponseQ4QGrouping.h"
#include "Xform/XMLBaggageResponse.h"
#include "Xform/XMLWriter.h"

#include <boost/function.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>

namespace tse
{
class PricingTrx;
class ShoppingTrx;
class RexPricingTrx;
class GroupFarePath;
class FarePath;
class FareUsage;
class Itin;
class TaxResponse;
class CalcTotals;
class ErrorResponseException;
class MinFarePlusUpItem;
class ProcessTagPermutation;
class OCFees;
class OCFeesUsage;
class InterlineTicketCarrier;
class Money;
using FuFcIdMap = std::map<const FareUsage*, uint16_t>;

namespace shpq
{
class SoloGroupFarePath;
}

class XMLShoppingResponse
{
public:
  static bool generateXML(PricingTrx& trx, std::string& res);
  static bool generateXML(PricingTrx& trx, ErrorResponseException& ere, std::string& res);
  static bool generateXML(ErrorResponseException& ere, std::string& res);

protected:
  struct Monetary
  {
    MoneyAmount amount = 0;
    CurrencyCode currency;
    CurrencyNoDec noDec;

    Monetary();
    Monetary(const MoneyAmount&, const CurrencyCode&, const CurrencyNoDec&);
    void add(const Monetary&, XMLShoppingResponse& resp);
  };

  struct ItineraryTotals
  {
    Monetary construction;
    Monetary base;
    Monetary equivalent;
    Monetary tax;

    MoneyAmount tTypeFees = 0;
    MoneyAmount equivalentAndTax = 0;

    boost::optional<int> psgMileage;
    boost::optional<MoneyAmount> feesAmount;

    bool operator<(const ItineraryTotals& rhs) const;
    void add(const ItineraryTotals&, XMLShoppingResponse& resp);
  };

public:
  typedef XMLWriter::Node Node;
  typedef std::pair<SopIdVec, GroupFarePath*> ISSolution;
  typedef std::pair<int, FareUsage*> OrderedFareUsage;
  typedef std::vector<std::vector<OrderedFareUsage> > FareUsagesPerLeg;
  typedef std::map<uint16_t, ProgramID> LegToProgramIdMap;
  typedef std::map<uint16_t, Direction> LegToDirectionMap;

  explicit XMLShoppingResponse(PricingTrx& trx, ErrorResponseException* ere = nullptr);
  XMLShoppingResponse(const XMLShoppingResponse&);
  virtual ~XMLShoppingResponse() {};
  void getXML(std::string& res);
  virtual void generateResponse();
  static void generateHostname(Node&);
  void generateResponseDiagnostic();
  void generateError();

  void generateITN(const SopIdVec& sops,
                   const GroupFarePath* path,
                   const std::vector<ISSolution>& paths,
                   int* q4q,
                   int* q5q,
                   std::string* q6q);

  void generateITN(const SopIdVec& sops,
                   const MoneyAmount totalNucAmount,
                   const MoneyAmount totalNucBaseFareAmount,
                   const GroupFarePath* path, // siriwan
                   const int* q4q = nullptr,
                   const int* q5q = nullptr,
                   const std::string* q6q = nullptr);

  void generateTOT(const MoneyAmount totalNUCAmount,
                   const MoneyAmount totalNUCBaseFareAmount,
                   const CurrencyCode& currency);

  typedef std::multimap<MoneyAmount, std::string> GenerateItinResultsContainerType;

  std::string getDirectionality(const FareUsage& fareUsage);

  std::string addFareBrandDetails(Node& nodeFDC, const PaxTypeFare& ptf,
    const BrandCode& brandCodeFromOuterSpace, Direction fareUsageDirection) const;

  void generateFDC(const FareUsage& fareUsage,
                   CalcTotals& calcTotal,
                   const Itin* itin,
                   uint16_t* segmentOrder,
                   uint16_t* index,
                   PricingUnit& pu,
                   bool firstSideTrip,
                   bool* preAvl,
                   uint16_t paxProcOrder,
                   bool& minMaxCatFound,
                   const boost::optional<FareCalc::SplitTaxInfo>& taxes,
                   uint16_t& fcId);

  void addVendorCodeToFDCNode(Node& nodeFDC, const VendorCode& vendorCode);

  void generateFDS(const FareUsage& fareUsage,
                   CalcTotals& calcTotal,
                   const Itin& Itin,
                   uint16_t* segmentOrder,
                   uint16_t* index,
                   PricingUnit& pu,
                   bool firstSideTrip,
                   bool* preAvl);

  void generatePEN(const MaxPenaltyResponse& maxPenaltyResponse,
                   const DateTime& ticketingDate,
                   const FarePath& farePath);

  void generateTAD(const TaxResponse* tax, CalcTotals& calcTotal);
  void generateTADForSplittedTaxes(CalcTotals& calcTotal, const FareCalc::SplitTaxInfo& group);

  template <typename TaxItems, typename TaxRecords>
  void generateTaxBreakDown(const TaxItems& taxItems,
                            const TaxRecords& taxRecords,
                            CurrencyNoDec taxNoDec);
  template <typename TaxRecords>
  void generateTaxBreakDownItem(const TaxItem& taxItem,
                                std::string& taxCodes,
                                const TaxRecords& taxRecords,
                                CurrencyNoDec taxNoDec);
  template <typename PfcItems>
  void generateTaxPfc(const PfcItems& pfcItems, const CurrencyCode& taxCurrencyCode)
  {
    for (const PfcItem* item : pfcItems)
    {
      generateTaxPfcItem(*item, taxCurrencyCode);
    }
  }

  void generateTaxPfcItem(const PfcItem& pfcItem, const CurrencyCode& taxCurrencyCode);

  void generateTaxOverrides(const CurrencyCode& currency);
  void generateTaxExchange(CalcTotals& calcTotal);
  void generateTaxInformation(CalcTotals& calcTotal);

  template <typename TaxRecords, typename PfcItems>
  void generateTaxInformation(const TaxRecords& taxRecords,
                              const PfcItems& pfcItems,
                              CurrencyNoDec taxNoDec)
  {
    for (const TaxRecord* taxRecord : taxRecords)
    {
      generateTaxInformationRecord(*taxRecord, pfcItems, taxNoDec);
    }
  }

  template <typename PfcItems>
  void generateTaxInformationRecord(const TaxRecord& taxRecord,
                                    const PfcItems& pfcItems,
                                    CurrencyNoDec taxNoDec);

  template <typename TaxExempts>
  void generateTaxExempt(const TaxExempts& taxExempts);

  void generateTaxInfoPerLeg(CalcTotals& calcTotal);

  void generateOBG(CalcTotals& calcTotal);

  void generateValidatingCxrList(const FarePath farePath);
  void generateValidatingCxrList(const FarePath& fp, const SettlementPlanType& sp, const CarrierCode& defaultValCxr);
  void generateValidatingCarrier(const CarrierCode& vcxr, bool isDefaultValidatingCxr);
  void generateValidatingCarrier(const CarrierCode& vcxr, const std::vector<NationCode>& interlineValidCountries, bool isDefaultValidatingCxr);
  void prepareValidatingCarrierLists(const FarePath& fp);

  bool getFeeRounding(PricingTrx& pricingTrx,
                      const CurrencyCode& currencyCode,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule);

  void generateP0N(const Itin* itin, Node& nodeITN);
  void generatePTF(const GroupFarePath* path, Node& nodeITN);
  void generateTFO(const shpq::SoloGroupFarePath* path, Node& nodeITN);

  void generateP0J(PricingTrx& trx, const Itin* itin, Node& nodeITN);
  void generateVTA(PricingTrx& trx, const Itin* itin, Node& nodeITN);
  void generatePVTandFBR(const Itin* itin, Node& nodeITN);
  void generatePVTandFBR(const GroupFarePath* path, Node& nodeITN);
  void generatePVTandFBRImpl(const FarePath farePath, bool& isPrivate, bool& isFBR);
  void generateSTI(const Itin* itin, Node& nodeITN) const;

  void generateCPL(const Itin* itin, Node& nodeITN) const;
  std::string generateCPLValueString(const std::vector<CarrierCode>& gcPerLeg) const;

  void generateQ18(Node& nodeFDC, const uint16_t paxProcOrder, const FareUsage& fareUsage);

  void generatePSGAttr(Node& node, const FarePath& path);
  void generatePSGAttr(Node& node,
                       const uint16_t num,
                       const PaxTypeCode& paxTypeCode,
                       const MoneyAmount totalNuc);

  void generateBKKAttr(const GroupFarePath& path, const SopIdVec& sops);
  void collectFareUsagesPerLeg(const FarePath& farePath,
                               const SopIdVec& sops,
                               FareUsagesPerLeg& fareUsagesPerLeg) const;
  bool formatBKK(const Itin& itin, uint16_t legId, uint16_t sopId, const FareUsage& fu);
  bool formatBKKAso(const Itin& itin,
                    const SopIdVec& sops,
                    const FareMarket& fuFareMarket,
                    const FareUsage& fu,
                    std::size_t& outLastProcessedLeg);

  bool generateP27(const FarePath& path);

  void generateCCD(PricingTrx& trx, const CalcTotals& calcTotal);

  void prepareFareUsagePlusUps(const FareUsage& fareUsage, const tse::CurrencyNoDec& noDec);

  void preparePupElement(const std::string& payload,
                         const MinFarePlusUpItem& plusUp,
                         const tse::CurrencyNoDec& noDec,
                         const NationCode& countryOfPmt);

  void prepareRuleCategoryIndicator(const FareUsage& fareUsage,
                                    CurrencyNoDec noDec,
                                    bool& minMaxCatFound);

  void prepareDifferential(const FareUsage& fareUsage, const tse::CurrencyNoDec& noDec);

  void prepareHPUForNet(const FareUsage& fareUsage);
  void prepareHPUForAdjusted(const FareUsage& fareUsage);

  void preparePricingUnitPlusUps(const PricingUnit& pricingUnit,
                                 const tse::CurrencyNoDec& noDec,
                                 PricingTrx& pricingTrx);

  void prepareFarePathPlusUps(const FarePath& farePath, const tse::CurrencyNoDec& noDec);

  void generateP35(const FareCalcCollector& calc, Node& nodeITN);

  void generateP95(const SopIdVec& sops, Node& nodeITN);

  void generateFMT(const SopIdVec& sops, Node& nodeITN);

  void generateOND();

  void generateDFL();
  uint32_t getOriginalSopIndex(FlightFinderTrx& flightFinderTrx,
                               const uint32_t legId,
                               const uint32_t sopIndex);
  void truncateSolutionNumber();
  uint32_t countFFSolutionNumber();
  PaxTypeFare* getFrontPaxTypeFare(const FlightFinderTrx::FlightDataInfo& flightInfo);
  bool showFareBasisCode(FlightFinderTrx& flightFinderTrx, bool outbound);
  bool showSOPs(FlightFinderTrx& flightFinderTrx, bool outbound);

  void getTTypeFeePerPax(CalcTotals& calcTotal, const FarePath* path, MoneyAmount& tTypeFeeTotal);
  std::string generateN1U(const PaxTypeFare& paxTypeFare);

  void generatePrivateInd(const Itin& itin,
                          const FareCalcCollector& calc,
                          std::vector<std::string>& privateIndPerPax);

  void generateN1U(FareCalcCollector& calc, Itin* itin, Node& node);

  void formOfRefundInd(const ProcessTagPermutation& winnerPerm, Node& nodeREX);
  void generateREX(CalcTotals& calc);
  void generateCHG(RexPricingTrx& trx, CalcTotals& calcTotals);
  void generateCommissionInfo(Node& nodePSG,
                              const FarePath* farePath,
                              const CalcTotals& calcTotals,
                              FuFcIdMap& fuFcIdCol);
  Indicator getCommissionSourceIndicator(const CalcTotals& calcTotals);
  void constructElementVCCForCat35(const FarePath& fp, const CurrencyNoDec noDec);
  void prepareMarkupAndCommissionAmount(Node& nodePSG,const FarePath& fp,const CurrencyNoDec noDec);
  void prepareCommissionForValidatingCarriers(const FarePath* farePath,
                                              const CarrierCode& defValCxr,
                                              const CurrencyNoDec noDec,
                                              FuFcIdMap& fuFcIdCol);
  void constructElementVCC(const FarePath& fp,
                           const CarrierCode& cxrs,
                           const MoneyAmount& commAmount,
                           const CurrencyNoDec noDec,
                           FuFcIdMap& fuFcIdCol,
                           uint16_t& fcInd);
  void constructElementFCB(amc::FcCommissionData& fcCommInfo,
                           uint16_t q6d,
                           const CurrencyNoDec noDec);
  void electronicTicketInd(const Indicator& electronicTicketIndicator, Node& nodeREX);
  void prepareUnflownItinPriceInfo();
  //--------------------------------

  void
  generateITNContent(FareCalcCollector& calc,
                     const Itin* itin,
                     ItineraryTotals& itineraryTotals);
  void generateSEG(const std::vector<CalcTotals*>& calcTotals, const Itin* itin);
  void generatePSG(FareCalcCollector& calcCollector,
                   const Itin* itin,
                   const FarePath* path,
                   CalcTotals* totals,
                   bool& minMaxCatFound,
                   MoneyAmount& totalConstructAmount);

  void generateMainPSG(Node& node,
                       FareCalcCollector& calcCollector,
                       const Itin* itin,
                       const FarePath* origPath,
                       CalcTotals* total,
                       bool& minMaxCatFound,
                       MoneyAmount& totalConstructAmount);

  void generatePSGAttr(FareCalcCollector& calcCollector,
                       const Itin* itin,
                       const FarePath* path,
                       CalcTotals* totals,
                       Node& nodePSG,
                       MoneyAmount& totalConstructAmount);

  void generateSellingFareData(CalcTotals* total);
  void generateHPSAdjustedSellingData(CalcTotals* adjTotal);

  void
  callGenerateFDC(const Itin* itin, const FarePath* path, CalcTotals* totals, bool& minMaxCatFound);
  BookingCode
  calculateBookingCode(const PaxTypeFare::SegmentStatus& segStatus, const TravelSeg* travelSeg);
  const CabinType& calculateCabin(const PaxTypeFare::SegmentStatus& segStatus,
                                  const TravelSeg* travelSeg,
                                  const std::vector<ClassOfService*>* cosVec);

  const CabinType&
  calculateCabin(const PaxTypeFare::SegmentStatus& segStatus, const TravelSeg* travelSeg);

  void prepareSurcharges(const TravelSeg& tvlSeg, CalcTotals& calcTotal);

  void moveMileageToAmt(std::vector<ISSolution>& paths) const;

  void generateMSG(const Itin* itin, CalcTotals* totals);
  void addAdjMsg(CalcTotals* totals, const std::string& msg);
  void printAdjMsg(CalcTotals* totals);

  void generateSFI(const Itin* itin);

  void writeAltDatePairs(const Itin* itin);

  void generateSFG(const Itin* itin, const ServiceFeesGroup* sfg);

  void generateOCF(const Itin* itin, const ServiceFeesGroup* sfg);

  void generateTOT(FareCalcCollector& fareCalcCollector,
                   MoneyAmount totalConstructAmount,
                   Money constructionCurrency,
                   std::string constructCurrencyCode,
                   ItineraryTotals& itineraryTotals,
                   const uint16_t brandIndex = INVALID_BRAND_INDEX);

  void formatDiag931();
  void formatDiag988();

  void checkDuplicateTvlSeg(const FarePath& path);
  void checkDuplicateThruFM(std::vector<PricingTrx::OriginDestination>& odThruFM,
                            FareMarket& fareMarket);

  const CarrierCode getValidatingCarrier(PricingTrx& pricingTrx);
  XMLWriter _writer;

  PricingTrx& _trx;

  bool _atLeastOneDomItin = false;
  bool _atLeastOneInterItin = false;

  const ShoppingTrx* const _shoppingTrx;
  const FlightFinderTrx* const _ffTrx;
  RexPricingTrx* _rexTrx;
  const ErrorResponseException* const _error;

  VecMap<const FareUsage*, size_t> _fareUsageIds;

  bool isIS() const { return _shoppingTrx != nullptr; }
  bool isFF() const { return _ffTrx != nullptr; }
  bool isMIP() const { return _shoppingTrx == nullptr; }
  bool isMIPResponse() const
  {
    return (_shoppingTrx == nullptr ? true : ((_shoppingTrx->getTrxType() == ShoppingTrx::IS_TRX) &&
                                        (_shoppingTrx->startShortCutPricingItin() > 0)));
  }

  bool isInternationalItin(const SopIdVec& sops);

  void updateItinCosForSeatRemain(const Itin& itin, const FarePath& path);
  size_t numberOfSeatRemain(const FarePath& farePath,
                            const BookingCode& bkgCode,
                            const TravelSeg& travelSeg,
                            int16_t segOrder);
  ClassOfService* finalSegCos(const FarePath& farePath, int16_t segOrder);

  void
  saveClassOfServiceFromAvailMap(const std::vector<TravelSeg*>& travelSegVec, const Itin& itin);

  bool isDifferentialFound(const FareUsage& fareUsage,
                           Node& nodeBKC,
                           const TravelSeg& travelSeg,
                           BookingCode& bookingCode,
                           const FareBreakPointInfo& breakPoint,
                           bool displayFareBasisCode = true);
  PaxTypeFare::SegmentStatus*
  getDiffSegStatus(const FareUsage& fareUsage, const TravelSeg& travelSeg);

  bool isJcbItin(const GroupFarePath& path);
  MoneyAmount convertCurrency(const Money& money, const CurrencyCode& to);
  void generateNodeADSforCalendarOrAwardAltDates();
  void generateNodeForExchangeWithBrands();
  void generateNodeForSettlementTypes();
  void preparePrimeItins();
  void generateSnapITN(Itin* const primaryItin);

  void generateSID(const Itin* const estimateItin);

  typedef boost::function<void(const Itin*)> ItinIdentificationFunction;
  void generateITNbody(Itin* itin,
                       const std::string& tag,
                       const ItinIdentificationFunction& generateItinIdentificationElements,
                       ItineraryTotals& itineraryTotals,
                       const uint16_t ittIndex = 0,
                       const bool isCarnivalSolFlexfare = false);

  void generateGRIlastTicketingDay(Node& nodeGRI,
                                   const Itin* itin,
                                   FareCalcCollector* calc,
                                   const uint16_t brandIndex);

  void generateITNlowerBody(const Itin* itin,
                            const std::vector<CalcTotals*>& calcTotals,
                            FareCalcCollector* calc,
                            const std::vector<std::string>& privateIndPerPax,
                            const uint16_t brandIndex,
                            ItineraryTotals& itineraryTotals);

  void computeItineraryTotals(const Itin* itin,
                              FareCalcCollector* calc,
                              uint16_t brandIndex,
                              bool baseFareDiff,
                              const Monetary& construction,
                              ItineraryTotals& itineraryTotals);

  void generateTOTbody(const ItineraryTotals& total);

  void generateITT(Itin* const primaryItin, const int legId);

  void generateITNIdAttribute(Node& nodeITN, const Itin* itin);

  void generateITNFamilyAttribute(Node& nodeITN, const Itin& itin);

  std::vector<std::string> generateITNAttributes(
      tse::XMLWriter::Node& nodeITN,
      const tse::Itin*,
      /*FIXME: can be made const after fixing its interface*/ tse::FareCalcCollector* calc);

  std::string getSnapKey(const TaxItem& taxItem);

  size_t countMIPSolutions(const std::vector<Itin*>& itins) const;

  std::string formatAmount(const MoneyAmount amount, const CurrencyNoDec decimalPlaces) const;

  std::string formatAmount(const Money& money) const;

  MoneyAmount getMoneyAmountInCurrency(const MoneyAmount sourceAmount,
                                       const CurrencyCode& sourceCurrency,
                                       const CurrencyCode& targetCurrency);

  bool itinIsValidBrandSolution(const Itin* itin, const uint16_t brandIndex);
  void generateBrandError(const Itin* itin, const uint16_t brandIndex);
  void generateBrandCombinationError(Node& nodeGRI, Itin* itin, uint16_t poptSpaceIndex);

  bool isFlexFareGroupValidForItin(const Itin* itin, const uint16_t flexFareGroupId) const;
  void generateFlexFareError(const uint16_t flexFareGroupId);

  bool generateCommonContent(Node& nodeShoppingResponse);
  void generateMIPResponse();

  int computeFamilyNumber(const std::vector<ISSolution>& paths,
                          const std::vector<int>& sops,
                          bool jjBrazilDomestic,
                          std::map<std::pair<size_t, bool>, size_t>& q5qMapGTC,
                          std::map<std::pair<int, bool>, int>& q5qJJ,
                          const std::string& mapKey);

  void generateISResponse();
  void prepareAndGenerateItineraries();
  virtual void generateItineraries();
  void generateNodeADPifCutOffReached();

  void generateHurryLogicFlag(Node& nodeShoppingResponse);

  bool isItinApplicable(const Itin* itin) const;

  size_t getFareUsageId(const FareUsage* fu);

  void prepareLatencyInfo();

  std::string getCommercialName(const OCFeesUsage* ocFeesUsage);

  void calculateQ5Q(std::map<std::pair<int, bool>, int>& q5qJJ,
                    int* familyNumber,
                    const std::string& mapKey);

  size_t generateQ5QForGTC(const SopIdVec& sops,
                           std::map<std::pair<size_t, bool>, size_t>& q5qMap,
                           size_t originalFamilyNumber);

  MoneyAmount calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                             const TicketingFeesInfo* feeInfo,
                                             const CurrencyCode& paymentCur);
  void getAdjustedObFeeAmount(MoneyAmount& currentObFeeAmount);
  Money convertOBFeeCurrency(PricingTrx& pricingTrx,
                             const TicketingFeesInfo* feeInfo,
                             const CurrencyCode& paymentCur);
  void convertOBFeeCurrency(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney);
  MoneyAmount calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                                 CalcTotals& calcTotal,
                                                 const TicketingFeesInfo* feeInfo,
                                                 const CurrencyCode& paymentCur);
  void writeOutOBFee(const std::vector<TicketingFeesInfo*>& collectedOBFeeVect,
                     CalcTotals& calcTotal,
                     size_t maxOBFeesOptions,
                     CurrencyCode& targetCurrency);
  void getValidatingCarrierInItinOrder(const FarePath& fp, const SettlementPlanType* sp = nullptr);
private:
  bool performVTAValidation(const Itin& itin) const;
  void generateSpanishDiscountIndicator(Node& nodeITN, const Itin* itin);
  std::size_t updateTotalBrandSolutions();
  bool itinIsValidSolution(const Itin* itin) const;
  Node& generateStopoverSurcharge(const FareUsage::StopoverSurchargeMultiMap& stopoverSurcharges,
                                  const TravelSeg* const travelSeg,
                                  Node& node);

  bool fillLegToDirectionMap(const Itin* itin);

protected:
  CurrencyConversionFacade _currencyConverter;
  int _psgMileageTotal = 0;

  ShoppingResponseQ4QGrouping _q4qGrouping;

  bool _privateIndRetrieved = false;
  bool _privateIndAllowed = false;
  InterlineTicketCarrier* _interlineTicketCarrierData = nullptr;
  AbstractTaxSplitter* _taxSplitter = nullptr;
  XMLBaggageResponse _baggageResponse;
  bool _rbdByCabinVSinHouse = false;
  LegToDirectionMap _legToDirectionMap;
};
}

