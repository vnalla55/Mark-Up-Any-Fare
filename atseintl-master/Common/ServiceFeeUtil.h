//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/CurrencyConversionRequest.h"
#include "Common/OcTypes.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/OcFeeGroupConfig.h"
#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
class Itin;
class PricingTrx;
class FarePath;
class FareUsage;
class TravelSeg;
class Loc;
class PaxOCFees;
class ServiceFeesGroup;
class PaxType;
class PaxTypeMatrix;
class PaxOCFees;
class PaxR7OCFees;
class FPOCFees;
class OCFees;
// Four classes below are for the OCFeesUsage development at Response build time
class PaxR7OCFeesUsages;
class FPOCFeesUsages;
class OCFeesUsage;

class OptionalServicesInfo;
class CalcTotals;
class Money;
class Currency;
class TaxNation;
class Nation;
class RequestedOcFeeGroup;
class SeatCabinCharacteristicInfo;
class ServiceFeeUtil;

struct PaxTypeBucketItem
{
  PaxType paxType;
  size_t numberOfItems = 0;
  double moneyAmount = 0.0;
  std::vector<OCFees*> ocFees;
  std::vector<OCFees*>* originalOcFees = nullptr;
};

class PaxBucketComparator
{
public:
  bool operator()(const PaxTypeBucketItem& lhs, const PaxTypeBucketItem& rhs) const;
};

class TravelSegmentOrderComparator
{
public:
  TravelSegmentOrderComparator(const Itin& itin) : _itin(itin) {}
  bool operator()(const TravelSeg* lhs, const TravelSeg* rhs) const;

private:
  const Itin& _itin;
};

struct FPOCFeesComparator : std::binary_function<const FPOCFees&, const FPOCFees&, bool>
{
  bool operator()(const FPOCFees& fee1, const FPOCFees& fee2) const;
};
struct FPOCFeesComparatorR7 : std::binary_function<const FPOCFees&, const FPOCFees&, bool>
{
  bool operator()(const FPOCFees& fee1, const FPOCFees& fee2) const;
  bool operator()(const PaxType::TktInfo* t1, const PaxType::TktInfo* t2) const;
};

// Two structs below are for the OCFeesUsage development at Response build time
struct FPOCFeesUsageComparator
    : std::binary_function<const FPOCFeesUsages&, const FPOCFeesUsages&, bool>
{
  bool operator()(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2) const;
};
struct FPOCFeesUsageComparatorR7
    : std::binary_function<const FPOCFeesUsages&, const FPOCFeesUsages&, bool>
{
  bool operator()(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2) const;
  bool operator()(const PaxType::TktInfo* t1, const PaxType::TktInfo* t2) const;
};
//

class TaxItemFeeRetriever
{
public:
  MoneyAmount operator()(const OCFees::TaxItem& taxItem) const { return taxItem.getTaxAmount(); }
};

class TaxItemFeeCurrencyConverter
{
public:
  TaxItemFeeCurrencyConverter(ServiceFeeUtil& util);

public:
  MoneyAmount operator()(const OCFees::TaxItem& taxItem) const;

private:
  ServiceFeeUtil& _util;
};

template <typename T>
struct StateRestorer
{
  StateRestorer<T>(T& obj, T& set, T& restore) : _obj(obj), _restore(restore) { _obj = set; }
  ~StateRestorer<T>() { _obj = _restore; }

private:
  T& _obj;
  T& _restore;
};

class ServiceFeeUtil
{
private:
  ServiceFeeUtil(const ServiceFeeUtil&);
  ServiceFeeUtil& operator=(const ServiceFeeUtil&);

public:
  friend class ServiceFeeUtilTest;
  typedef std::vector<tse::TravelSeg*>::const_iterator TravelSegVecIC;

  ServiceFeeUtil(PricingTrx& trx);
  virtual ~ServiceFeeUtil() {}

  static void fill(std::set<BookingCode>& bookingCode, const SvcFeesResBkgDesigInfo* padis);

  static bool isInternationalJourneyType(const Itin& itin);
  bool isRoundTripJourneyType(const Itin& itin,
                              const CarrierCode& validatingCarrier,
                              bool isInternational) const;

  TravelSegVecIC getJourneyDestination(const FarePath& farePath,
                                       const Loc*& journeyDestination,
                                       bool isRT,
                                       bool isIntl) const;

  static bool checkIsDateBetween(const DateTime& startDate,
                                 const DateTime& endDate,
                                 const DateTime& betweenDate);

  static bool isStartDateSpecified(const OptionalServicesInfo& optSrvInfo);
  static bool isStopDateSpecified(const OptionalServicesInfo& optSrvInfo);

  bool matchPaxType(const CarrierCode& validatingCarrier,
                    const PaxType& farePathPaxType,
                    const PaxTypeCode& paxTypeCode) const;

  static std::vector<PaxOCFees> getSortedFees(const ServiceFeesGroup& sfg,
                                              const std::vector<PaxType*>& reqPaxTypes,
                                              bool removeDuplicates = true);

  static std::vector<PaxOCFees> getFees(const ServiceFeesGroup& sfg,
                                        const std::vector<PaxType*>& reqPaxTypes,
                                        bool removeDuplicates = true);

  static std::vector<PaxR7OCFees>
  getSortedFeesForR7(const std::vector<const ServiceFeesGroup*>& sfgs,
                     const std::vector<PaxType*>& reqPaxTypes,
                     bool removeDuplicates = true);
  // @@@ The chages below till "//" are for the OCFeesUsages
  static std::vector<PaxOCFeesUsages> getSortedFeesUsages(const ServiceFeesGroup& sfg,
                                                          const std::vector<PaxType*>& reqPaxTypes,
                                                          bool removeDuplicates = true);

  // Merges the OCs which are available for all passengers, keeps track of
  // the maximum OCFees count, assigns a unique ID to each returned OCFee
  // and groups the returned OCFees per ServiceFeesGroup and then per FarePath.
  class OcFeesUsagesMerger
  {
  public:
    OcFeesUsagesMerger(const std::vector<PaxType*>& reqPaxTypes, unsigned maxFeesCount = 0);

    // Sorts all the OCs for a given group, merges similar OCs, which are available for all passengers
    // into one with passenger type "ALL".
    // Side effects:
    //   * Adds the returned PaxOCFeesUsages to the vector of all OCs
    //   * Adds the IDs (indexes in the vector of all OCs) to the appropriate SFG=>set<ID> map
    std::vector<PaxOCFeesUsages> mergeFeesUsagesForSfg(const ServiceFeesGroup& sfg);

    // Returns a reference to a map of ServiceFeesGroupCode=>set<OCFeeID> for the specified FarePath
    const std::map<std::string, std::set<int>>& getGroupedOCFeeIdsForFarePath(const FarePath* fp);

    // Returns a reference to a vector of all the unique PaxOCFeesUsages seen by the merger
    const std::vector<PaxOCFeesUsages>& getAllOCFees() { return _ocs; }

    // True if the merger had to drop some OCFees to maintain the requested maximum OCFees count
    bool isMaxFeesCountReached() { return _maxFeesCountReached; }

    // Returns a string representation of the internal state of the merger
    std::string toString();

  private:
    struct SimilarOcFeesUsages;
    std::vector<SimilarOcFeesUsages> getFeesGroupedBySimilarity(const ServiceFeesGroup& sfg);
    FPOCFeesUsages* findFirstUsageWithPaxTypeAndAddAllSimilarsToMapping(PaxType* paxType,
                                                                        SimilarOcFeesUsages& group,
                                                                        const ServiceGroup& groupCode);

    const std::vector<PaxType*>& _reqPaxTypes;
    unsigned _maxFeesCount;
    bool _maxFeesCountReached;
    std::map<const FarePath*, std::map<std::string, std::set<int>>> _ocFeesIdBySfgByFarePath;
    std::vector<PaxOCFeesUsages> _ocs;
  };

  static std::vector<PaxOCFeesUsages> getFeesUsages(const ServiceFeesGroup& sfg,
                                                    const std::vector<PaxType*>& reqPaxTypes,
                                                    bool removeDuplicates = true);

  static std::vector<PaxR7OCFeesUsages>
  getSortedFeesForR7Usages(const std::vector<const ServiceFeesGroup*>& sfgs,
                           const std::vector<PaxType*>& reqPaxTypes,
                           bool removeDuplicates = true);
  //

  void convertServiceFeesGroupCurrency(ServiceFeesGroup*);

  Money convertOCFeeCurrency(const CalcTotals& calcTotals, const OCFees* fee);
  Money convertOCFeeCurrency(const OCFees* fee);
  Money convertBaggageFeeCurrency(const OCFees* fee);
  Money convertBaggageFeeCurrency(const OCFeesUsage& ocFeeUsage);
  Money convertBaggageFeeCurrency(const MoneyAmount sourceAmount,
                                  const CurrencyCode& sourceCurrency,
                                  const CurrencyCode& targetCurrency);
  Money convertOCFeeCurrencyWithTaxes(const CurrencyCode sellingCurrency, const OCFees* fee);
  Money convertOCFeeCurrencyWithTaxes(const CurrencyCode sellingCurrency, const OCFeesUsage* fee);
  Money convertOCFeeCurrency(const OCFeesUsage& ocFeeUsage);

  static const RequestedOcFeeGroup*
  getRequestedOcFeeGroupData(const std::vector<RequestedOcFeeGroup>& serviceGroupsVec,
                             const ServiceGroup& group);

  static const OcFeeGroupConfig*
  getGroupConfigurationForCode(const std::vector<OcFeeGroupConfig>& groupSummaryConfigVec,
                               const ServiceGroup& group);

  Money getOCFeesSummary(const Itin* itin);

  CurrencyCode getSellingCurrency() const;

  static CurrencyCode getSellingCurrency(const PricingTrx& trx);
  bool getFeeRounding_old(const CurrencyCode& currencyCode,
                      RoundingFactor& roundingFactor,
                      CurrencyNoDec& roundingNoDec,
                      RoundingRule& roundingRule);  // To remove with ocFeesAmountRoundingRefactoring

  // external services overrides
  virtual bool convertCurrency(
      Money& target,
      const Money& source,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::TAXES) const;

  static bool isAdultPaxType(const PaxType& paxType);

  static bool feesFoundForItin(const Itin* itin);

  static void fareStatToString(const PaxTypeFare& paxTypeFare, std::string& outStr);
  static void setFareStat(PaxTypeFare& ptf, const OCFareTypeCode& fareType);

  static void setFareIndicator(PaxTypeFare& ptf, uint16_t fareTypeInd);

  static void collectSegmentStatus(const FarePath& farePath, Ts2ss& ts2ss);

  static bool isDiscounted19_22(const PaxTypeFare& ptf);

  static bool checkServiceGroupForAcs(const ServiceGroup& serviceGroup);

  static bool isServiceGroupInvalidForAcs(const ServiceGroup& serviceGroup);

  static bool isFeeFarePercentage(const OptionalServicesInfo& info);

  static void
  createOCFeesUsages(const ServiceFeesGroup& sg, PricingTrx& trx, bool ignorePadis = false);
  static void createOCFeesUsagesforR7(const std::vector<ServiceFeesGroup*>& sfgs, PricingTrx& trx);
  static void
  addSeatCabinCharacteristic(PricingTrx& trx, OCFeesUsage& ocfUsage, SvcFeesResBkgDesigInfo& padis);

  static std::string getTranslatedPadisDescription(PricingTrx& trx,
                                                   const CarrierCode& carrier,
                                                   const DateTime& travelDate,
                                                   const SvcFeesResBkgDesigInfo& padis);

  static void
  getTranslatedPadisDescription(PricingTrx& pricingTrx,
                                const CarrierCode& carrier,
                                const DateTime& travelDate,
                                const SvcFeesResBkgDesigInfo& padis,
                                std::map<BookingCode, std::string>& codeDescriptionMap,
                                std::map<BookingCode, std::string>& abbreviatedDescriptionMap);

  static void createOCFeesUsageForSingleS7(PricingTrx& pricingTrx,
                                           OCFees& ocfee,
                                           size_t i,
                                           bool ignorePadis = false);
  static void fillOutOCFeeUsageByPadisDescription(PricingTrx& pricingTrx,
                                                  OCFees& ocfee,
                                                  size_t i,
                                                  SvcFeesResBkgDesigInfo& padis);

  static bool isPortionOfTravelSelected(PricingTrx& trx, TravelSeg* first, TravelSeg* last);
  Money convertMoney(
      const MoneyAmount sourceAmount,
      const CurrencyCode& sourceCurrency,
      const CurrencyCode& targetCurrency,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::TAXES);

  static std::string getFareBasisRoot(PricingTrx& trx, std::string fareBasis);

  static bool isPerformEMDCheck(const PricingTrx& trx, const SubCodeInfo& subCode);
  static bool isPerformEMDCheck_old(const PricingTrx& trx);

  static bool isRequestFromTN(const PricingTrx& trx);

protected:
  Money convertFeeWithTaxes(
      const MoneyAmount& fee,
      const std::vector<tse::OCFees::TaxItem>& taxes,
      const CurrencyCode& sourceCurrency,
      const CurrencyCode& targetCurrency,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::TAXES);

  static void getSortedFees(const ServiceFeesGroup& sfg, std::vector<FPOCFees>& sortedFees);
  // @@@ The chage below for the OCFeesUsages
  static void getSortedFees(const ServiceFeesGroup& sfg, std::vector<FPOCFeesUsages>& sortedFees);

  TravelSegVecIC getMlgTurnAround_old(const FarePath& farePath,
                                  std::map<uint32_t, int, std::greater<uint32_t> >& mlgMap) const;

  TravelSegVecIC getMlgTurnAround(const FarePath& farePath,
                                  std::map<uint32_t, int, std::greater<uint32_t> >& mlgMap) const;

  void getStopoversAndFareBreaks_old(const FarePath& farePath, std::set<TravelSeg*>& result) const;

  void getStopoversAndFareBreaks(const FarePath& farePath, std::set<TravelSeg*, TravelSegmentOrderComparator>& result) const;

  bool isStopOverPoint(const TravelSeg* travelSeg,
                       const TravelSeg* travelSegTo,
                       const FareUsage* fareUsage) const;
  bool matchSabrePaxList(const PaxTypeCode& farePathPtc, const PaxTypeCode& svcFeePtc) const;
  bool paxTypeMappingMatch(const CarrierCode& validatingCarrier,
                           const PaxType& paxChk,
                           const PaxTypeCode& paxRef) const;
  bool paxTypeMappingMatch(const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxTypes,
                           const CarrierCode& carrier,
                           const PaxTypeCode& paxRef) const;

  // Database overrides
  virtual bool checkMultiTransport(const LocCode& locCode1,
                                   const LocCode& locCode2,
                                   const CarrierCode& carrierCode,
                                   GeoTravelType tvlType) const;
  virtual bool isStopOver(const TravelSeg* travelSeg,
                          const TravelSeg* travelSegTo,
                          const FareUsage* fareUsage) const;
  virtual uint32_t getTPM(const Loc& market1,
                          const Loc& market2,
                          const std::vector<TravelSeg*>& travelSegs,
                          const FarePath& farePath) const;
  virtual const std::vector<const PaxTypeMatrix*>&
  getSabrePaxTypes(const PaxTypeCode& farePathPtc) const;

  virtual const Currency* getCurrency_old(const CurrencyCode& currencyCode) const; // To remove with ocFeesAmountRoundingRefactoring
  virtual const TaxNation* getTaxNation_old(const NationCode& nationCode) const; // To remove with ocFeesAmountRoundingRefactoring

  template <class T>
  static std::vector<T> groupPaxTypes(std::vector<FPOCFees>& sortedFees,
                                      const std::vector<PaxType*>& reqPaxTypes,
                                      bool removeDuplicates = true);

  template <class T>
  static void reducePaxTypes(std::vector<FPOCFees>::const_iterator begin,
                             std::vector<FPOCFees>::const_iterator end,
                             const std::vector<PaxType*>& reqPaxTypes,
                             std::vector<T>& groupedFees);

  static bool isSimilar(const FPOCFees& fee1, const FPOCFees& fee2);
  static bool isSimilarAndSamePaxType(const FPOCFees& fee1, const FPOCFees& fee2);

  // @@@ The chages below till "//" are for the OCFeesUsages
  template <class T>
  static std::vector<T> groupPaxTypesU(std::vector<FPOCFeesUsages>& sortedFees,
                                       const std::vector<PaxType*>& reqPaxTypes,
                                       bool removeDuplicates = true);

  template <class T>
  static void reducePaxTypesU(std::vector<FPOCFeesUsages>::const_iterator begin,
                              std::vector<FPOCFeesUsages>::const_iterator end,
                              const std::vector<PaxType*>& reqPaxTypes,
                              std::vector<T>& groupedFees);

  static bool isSimilarU(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2);
  static bool isSimilarAndSamePaxTypeU(const FPOCFeesUsages& fee1, const FPOCFeesUsages& fee2);
  static bool isSimilarOCFeesUsage(const OCFeesUsage& fee1, const OCFeesUsage& fee2);
  //

  Money convertOCFeeCurrency(const CurrencyCode& sellingCurrency, const OCFees* fee);

  void removeRestrictedSubCodes(const OcFeeGroupConfig* feeGroupConfig,
                                std::vector<OCFees*>& outFeesVec);

  Money getFeeDependOnSubCodesOrder(const Itin* itin,
                                    const OcFeeGroupConfig* feeGroupConfig,
                                    const CurrencyCode& sellingCurrency,
                                    size_t numberOfItems,
                                    std::vector<OCFees*>& feesVec);

  Money getLowestFee(const Itin* itin,
                     const CurrencyCode& sellingCurrency,
                     std::vector<OCFees*>& feesVec);

  OCFees* getFeeForSubCode(const Itin* itin,
                           TravelSeg* travelSeg,
                           std::vector<OCFees*>& feesVec,
                           const ServiceSubTypeCode& subTypeCode);

  bool feeValidForTravelSeg(const Itin* itin, OCFees* ocFees, TravelSeg* travelSeg);

  bool addFeeToMap(OCFees* ocFees,
                   TravelSeg* travelSeg,
                   std::map<TravelSeg*, std::vector<OCFees*> >& feesForSegmentMap);

  Money getSummaryFromMap(const std::map<TravelSeg*, std::vector<OCFees*> >& feesForSegmentMap,
                          const CurrencyCode& sellingCurrency,
                          std::vector<OCFees*>& feesVec);

  void clearExpensivePaxOCFees(ServiceFeesGroup* sfg, const PaxTypeCode& paxType);

  void removeFeesForRemainingPaxTypes(ServiceFeesGroup* sfg,
                                      std::vector<PaxType>& paxTypeRemVec,
                                      std::vector<bool>& paxTypeFeesInitialized);

  std::vector<PaxTypeBucketItem> constructPaxTypeBuckets(ServiceFeesGroup* sfg,
                                                         const OcFeeGroupConfig* feeGroupConfig,
                                                         size_t noOfItemsRequested,
                                                         const Itin* itin,
                                                         const CurrencyCode& sellingCurrency);

  std::vector<PaxTypeBucketItem> getPaxBuckets(size_t numberOfItems, ServiceFeesGroup* sfg);

  Money getMoneyFromVec(const std::vector<OCFees*>& ocFeesVec, const CurrencyCode& sellingCurrency);

  bool pickBucketItem(PaxType& remPaxType,
                      PaxTypeBucketItem& paxTypeBucketItem,
                      Money& outMoney,
                      size_t& feesRem,
                      bool& paxTypeFeesCopied);

  void getOCFeesSummary(const Itin* itin,
                        size_t numberOfItems,
                        const OcFeeGroupConfig* feeGroupConfig,
                        std::vector<OCFees*>& ocFeesVec,
                        const CurrencyCode& sellingCurrency);

  void recalculateOCFeesVector(std::vector<OCFees*>& ocFeesVec, size_t noOfItemsRequested);

  size_t getMaxNoOfFeesAllowed(size_t feesRemaining, size_t noOfPaxRemaining);

  static void
  getPadisDescriptions(const std::set<BookingCode>& bookingCodeSet,
                       const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
                       const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll,
                       std::map<BookingCode, std::string>& codeDescriptionMap,
                       std::map<BookingCode, std::string>& abbreviatedDescriptionMap);

  static void
  getPadisDisplayDescriptions(const std::set<BookingCode>& bookingCodeSet,
                              const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
                              const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll,
                              std::map<BookingCode, std::string>& displayDescriptionMap,
                              std::map<BookingCode, std::string>& abbreviatedDescriptionMap);

  static std::string
  getPadisDescriptions(PricingTrx& pricingTrx,
                       const std::set<BookingCode>& bookingCodeSet,
                       const std::vector<SeatCabinCharacteristicInfo*>& seatCabinForCarrier,
                       const std::vector<SeatCabinCharacteristicInfo*>& seatCabinAll);

  static void cleanupOcFeesMap(std::map<const FarePath*, std::vector<OCFees*> >& map);

  enum
  {
    PTF_Normal = 0x00,
    PTF_Discounted = 0x01,
    PTF_FareByRule = 0x02,
    PTF_Negotiated = 0x04,
    PTF_TariffCatPrivate = 0x08,
  };

  // Data members
protected:
  PricingTrx& _trx;
};

} // namespace tse

