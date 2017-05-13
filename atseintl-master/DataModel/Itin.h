//-------------------------------------------------------------------
//
//  File:        Itin.h
//  Created:     March 8, 2004
//  Authors:
//
//  Description: The Itinerary class object represents one
//               itinerary from incoming shopping/pricing request.
//
//  Updates:
//          03/08/04 - VN - file created.
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
#pragma once

#include "AncillaryOptions/AncillaryIdentifier.h"
#include "AncillaryOptions/AncillaryPriceModifier.h"
#include "Common/Assert.h"
#include "Common/BaggageTripType.h"
#include "Common/ErrorResponseException.h"
#include "Common/JourneyUtil.h"
#include "Common/MultiTicketUtil.h"
#include "Common/SmallBitSet.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/IbfAvailabilityTracker.h"
#include "DataModel/SimilarItinData.h"
#include "DataModel/ValidatingCxrGSAData.h"
#include "Routing/RoutingConsts.h"

#include <map>
#include <utility>
#include <vector>

namespace tse
{
namespace Memory
{
class LocalManager;
}

class Agent;
class CarrierPreference;
class ClassOfService;
class ConsolidatorPlusUp;
class DataHandle;
class ESVOptions;
class ESVSopWrapper;
class FareCompInfo;
class FareMarket;
class FareMarketPathMatrix;
class FarePath;
class PaxType;
class PaxTypeFare;
class ServiceFeesGroup;
class SOPFareList;
class SOPFarePath;
class TaxResponse;
class TravelSeg;
class YQYRLBFactory;

using TravelSegPtrVec = std::vector<TravelSeg*>;

namespace skipper
{
class ItinBranding;
}

namespace PrecalcBaggage
{
class ItinData;
class PaxData;
}

typedef std::shared_ptr<FareMarketPathMatrix> FmpMatrixPtr;
typedef std::shared_ptr<YQYRLBFactory> YQYRLBFactoryPtr;
typedef std::vector<FarePath*> FarePaths;
typedef std::shared_ptr<FarePaths> FarePathsPtr;
typedef std::map<BrandCode, std::set<ProgramID> >BrandFilterMap;
typedef std::vector<TravelSegPtrVec> ItinLegs;

struct MaxPenaltyStats
{
  struct Filter
  {
    bool _queryFailed;

    MoneyAmount _minPenalty;
  };

  Filter _changeFilter;

  Filter _refundFilter;

  unsigned int _failedFares;
};

class Itin
{
  enum FamilyLogicType : uint8_t
  {
    NO_FAMILY,
    HEAD_OF_FAMILY,
    SIMILAR_ITIN,
  };

public:
  enum WQDateType : uint8_t
  {
    NoDate, // None of the travel segments has any date.
    PartialDate, // Some of the travel segments has a date.
    FullDate // All travel segments has a date.
  };

  enum ISICode : uint8_t
  {
    UNKNOWN = 0,
    SITI, // Sold inside, ticketed inside
    SITO, // Sold inside, ticketed outside
    SOTI, // Sold outside, ticketed inside
    SOTO // Sold outside, ticketed outside
  };

  enum TripCharacteristics : uint16_t
  {
    OneWay = 0x01,
    RoundTrip = 0x02,
    OriginatesUS = 0x04,
    TerminatesUS = 0x08,
    OriginatesCanadaMaritime = 0x10,
    USOnly = 0x20,
    CanadaOnly = 0x40,
    TurnaroundPointForTax = 0x80,
    RussiaOnly = 0x100,
    RW_SFC = 0x200,
    CT_SFC = 0x400,
    EuropeOnly = 0x800
  };

  typedef std::map<TravelSeg*, std::vector<ClassOfService*>*> InterlineJourneyInfoMap;
  typedef std::map<TravelSeg*, std::pair<TravelSeg*, TravelSeg*> > InterlineJourneyMarketMap;
  typedef std::set<std::string> ProgramSet;
  typedef std::map<std::string, ProgramSet> ProgramsForBrandMap;
  typedef std::map<AncillaryIdentifier, std::vector<AncillaryPriceModifier>> AncillaryPriceModifiersMap;

  virtual ~Itin() = default;

  void duplicate(const Itin& itin, DataHandle& dataHandle);
  void duplicate(const Itin& itin, const PaxTypeFare& ptf, DataHandle& dataHandle);

  // Access

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType geoTravelType() const { return _geoTravelType; }

  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }
  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }

  std::vector<FareMarket*>& fareMarket() { return _fareMarket; }
  const std::vector<FareMarket*>& fareMarket() const { return _fareMarket; }

  std::vector<FareMarket*>& flowFareMarket() { return _flowFareMarket; }
  const std::vector<FareMarket*>& flowFareMarket() const { return _flowFareMarket; }

  std::vector<FarePath*>& farePath() { return _farePath; }
  const std::vector<FarePath*>& farePath() const { return _farePath; }

  const std::vector<TaxResponse*>& getTaxResponses() const { return _taxResponse; }
  std::vector<TaxResponse*>& mutableTaxResponses() { return _taxResponse; }

  const std::vector<TaxResponse*>& getAllValCxrTaxResponses() const { return _valCxrTaxResponse; }
  std::vector<TaxResponse*>& valCxrTaxResponses() { return _valCxrTaxResponse; }

  // Only used for child itins!
  std::vector<ClassOfService*>& origBooking() { return _origBooking; }
  const std::vector<ClassOfService*>& origBooking() const { return _origBooking; }

  JourneyUtil::SegOAndDMap& segmentOAndDMarket() { return _segmentOAndDMarket; }
  const JourneyUtil::SegOAndDMap& segmentOAndDMarket() const { return _segmentOAndDMarket; }

  FmpMatrixPtr& fmpMatrix() { return _fmpMatrix; }
  const FmpMatrixPtr& fmpMatrix() const { return _fmpMatrix; }

  std::map<BrandCode, FmpMatrixPtr>& brandFmpMatrices() { return _brandFmpMatrices; }
  const std::map<BrandCode, FmpMatrixPtr>& brandFmpMatrices() const { return _brandFmpMatrices; }

  std::map<flexFares::GroupId, FmpMatrixPtr>& groupsFmpMatrices() { return _groupsFmpMatrices; }
  const std::map<flexFares::GroupId, FmpMatrixPtr>& groupsFmpMatrices() const
  {
    return _groupsFmpMatrices;
  }

  void invalidateAllPaxTypeFaresForRetailer(PricingTrx& trx);

  std::set<uint16_t> getPricedBrandCombinationIndexes() const;

  const IbfAvailabilityTracker& getIbfAvailabilityTracker() const
  {
    return _ibfAvailabilityTracker;
  }

  IbfAvailabilityTracker& getMutableIbfAvailabilityTracker() { return _ibfAvailabilityTracker; }

  std::vector<OAndDMarket*>& oAndDMarkets() { return _OAndDMarkets; }
  const std::vector<OAndDMarket*>& oAndDMarkets() const { return _OAndDMarkets; }

  CarrierCode& ticketingCarrier() { return _ticketingCarrier; }
  const CarrierCode& ticketingCarrier() const { return _ticketingCarrier; }

  CarrierCode& validatingCarrier() { return _validatingCarrier; }
  const CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  CarrierCode& traditionalValidatingCxr() { return _traditionalValidatingCxr; }
  const CarrierCode&  traditionalValidatingCxr() const { return _traditionalValidatingCxr; }

  CarrierPreference*& ticketingCarrierPref() { return _ticketingCarrierPref; }
  const CarrierPreference* ticketingCarrierPref() const { return _ticketingCarrierPref; }

  ValidatingCxrGSAData*& validatingCxrGsaData() { return _validatingCxrGsaData; }
  const ValidatingCxrGSAData* validatingCxrGsaData() const { return _validatingCxrGsaData; }

  bool isValidatingCxrGsaDataForMultiSp() const;

  SpValidatingCxrGSADataMap*& spValidatingCxrGsaDataMap() { return _spValidatingCxrGsaDataMap; }
  const SpValidatingCxrGSADataMap* spValidatingCxrGsaDataMap() const { return _spValidatingCxrGsaDataMap; }

  const GSASwapMap& gsaSwapMap() const { return _validatingCxrGsaData->gsaSwapMap(); }
  const GSASwapMap& gsaSwapMap(const SettlementPlanType& sp) const;

  bool getSwapCarriers(const CarrierCode& carrier, std::set<CarrierCode>& result) const
  { return _validatingCxrGsaData->getSwapCarriers(carrier, result); }

  bool getSwapCarriers(const CarrierCode& carrier,
      std::set<CarrierCode>& result, const SettlementPlanType& sp) const;

  void removeNonDefaultValidatingCarriers(CarrierCode & vcr) const;
  void removeAlternateValidatingCarriers();

  bool hasNeutralValidatingCarrier() const { return _validatingCxrGsaData->isNeutralValCxr(); }
  bool hasNeutralValidatingCarrier(const SettlementPlanType& sp) const;

  void getValidatingCarriers(const PricingTrx& trx, std::vector<CarrierCode>& result) const;
  void getValidatingCarriers(const ValidatingCxrGSAData& valCxrGsaData,
                             std::vector<CarrierCode>& result) const;

  bool hasFareMarket(const FareMarket* fareMarket) const;

  ISICode& intlSalesIndicator() { return _intlSalesIndicator; }
  const ISICode& intlSalesIndicator() const { return _intlSalesIndicator; }

  SmallBitSet<uint16_t, TripCharacteristics>& tripCharacteristics() { return _tripCharacteristics; }
  const SmallBitSet<uint16_t, TripCharacteristics>& tripCharacteristics() const
  {
    return _tripCharacteristics;
  }

  CurrencyCode& calculationCurrency() { return _calculationCurrency; }
  const CurrencyCode& calculationCurrency() const { return _calculationCurrency; }

  CurrencyCode& originationCurrency() { return _originationCurrency; }
  const CurrencyCode& originationCurrency() const { return _originationCurrency; }

  CurrencyCode& calcCurrencyOverride() { return _calcCurrencyOverride; }
  const CurrencyCode& calcCurrencyOverride() const { return _calcCurrencyOverride; }

  std::vector<PaxType*>& paxGroup() { return _paxGroup; }
  const std::vector<PaxType*>& paxGroup() const { return _paxGroup; }

  uint32_t& sequenceNumber() { return _sequenceNumber; }
  const uint32_t& sequenceNumber() const { return _sequenceNumber; }

  int16_t& furthestPointSegmentOrder() { return _furthestPointSegmentOrder; }
  const int16_t& furthestPointSegmentOrder() const { return _furthestPointSegmentOrder; }

  bool& salesNationRestr() { return _salesNationRestr; }
  const bool& salesNationRestr() const { return _salesNationRestr; }

  bool isSRFEApplicable() const { return _SRFEApplicable; }
  void setSRFEApplicable(bool val) { _SRFEApplicable = val; }

  // function to return a segment's pposition in the Itin.
  // will return -1 if the segment doesn't exist in the Itin at all.
  int16_t segmentOrder(const TravelSeg* segment) const;

  int16_t segmentPnrOrder(const TravelSeg* segment) const;

  bool hasTvlSegInFlowFareMarkets(const TravelSeg* segment) const;

  // function which returns true iff the segment is present in the Itin
  bool segmentExists(const TravelSeg* segment) const { return segmentOrder(segment) != -1; }

  // function which returns true iff 'a' comes immediately after 'b'
  // in this Itin
  bool segmentFollows(const TravelSeg* a, const TravelSeg* b) const;

  // function which returns true iff after 'a'there is an arunk segment,
  // and after the arunk segment is 'b'
  bool segmentFollowsAfterArunk(const TravelSeg* a, const TravelSeg* b) const;

  // functions which return true iff the given segment is the first/last
  // segment in the Itin
  bool isFirstSegment(const TravelSeg* segment) const
  {
    return travelSeg().empty() == false && travelSeg().front() == segment;
  }

  bool isLastSegment(const TravelSeg* segment) const
  {
    return travelSeg().empty() == false && travelSeg().back() == segment;
  }

  bool& simpleTrip() { return _simpleTrip; }
  const bool& simpleTrip() const { return _simpleTrip; }

  bool& inboundSop() { return _inboundSop; }
  const bool& inboundSop() const { return _inboundSop; }

  void setTravelDate(const DateTime& travelDate) { _travelDate = travelDate; }
  virtual const DateTime& travelDate() const { return _travelDate; }

  DateTime& bookingDate() { return _bookingDate; }
  const DateTime& bookingDate() const { return _bookingDate; }

  std::vector<std::pair<int, int> >& legID() { return _legID; }
  const std::vector<std::pair<int, int> >& legID() const { return _legID; }

  ErrorResponseException::ErrorResponseCode& errResponseCode() { return _errResponseCode; }
  const ErrorResponseException::ErrorResponseCode& errResponseCode() const
  {
    return _errResponseCode;
  }

  std::string& errResponseMsg() { return _errResponseMsg; }
  const std::string& errResponseMsg() const { return _errResponseMsg; }

  bool& useInternationalRounding() { return _useInternationalRounding; }
  const bool& useInternationalRounding() const { return _useInternationalRounding; }

  /**
  *  @method: csTextMessages
  *  Description: Get/Set methods for Currency Selection Text Messages
  *
  *  @return: const/non-const reference to vector of strings
  */
  std::vector<std::string>& csTextMessages() { return _csTextMessages; }
  const std::vector<std::string>& csTextMessages() const { return _csTextMessages; }

  const std::vector<SimilarItinData>& getSimilarItins() const { return _similarItinsData; }
  void addSimilarItin(Itin* itin);
  void addSimilarItins(std::vector<SimilarItinData>::const_iterator begin,
                       std::vector<SimilarItinData>::const_iterator end);
  bool eraseSimilarItin(Itin* itin);
  bool rotateHeadOfSimilarItins(Itin* newHead);
  std::vector<SimilarItinData> clearSimilarItins();
  void swapSimilarItins(std::vector<SimilarItinData>& itins);

  const SimilarItinData* getSimilarItinData(const Itin& similarItin) const;

  const std::pair<DateTime, DateTime>* datePair() const { return _datePair; }
  std::pair<DateTime, DateTime>*& datePair() { return _datePair; }

  bool& dcaSecondCall() { return _dcaSecondCall; }
  const bool& dcaSecondCall() const { return _dcaSecondCall; }

  // For PU pricing
  ConsolidatorPlusUp*& consolidatorPlusUp() { return _consolidatorPlusUp; }
  const ConsolidatorPlusUp* consolidatorPlusUp() const { return _consolidatorPlusUp; }
  bool isPlusUpPricing() { return _consolidatorPlusUp != nullptr; }
  const bool isPlusUpPricing() const { return _consolidatorPlusUp != nullptr; }

  const MoneyAmount& estimatedTax() const { return _estimatedTax; }

  void setEstimatedTax(MoneyAmount tax) { _estimatedTax = tax; }

  WQDateType& dateType() { return _dateType; }
  const WQDateType& dateType() const { return _dateType; }

  bool& isDummy() { return _isDummy; }
  const bool& isDummy() const { return _isDummy; }

  void setHasSideTrip(bool hasSideTrip) { _hasSideTrip = hasSideTrip; }
  const bool& hasSideTrip() const { return _hasSideTrip; }

  BrandCodeSet& brandCodes() { return _brandCodes; }
  const BrandCodeSet& brandCodes() const { return _brandCodes; }

  BrandFilterMap& brandFilterMap() { return _brandFilterMap; }
  const BrandFilterMap& brandFilterMap() const { return _brandFilterMap; }

  ProgramsForBrandMap& getProgramsForBrandMap() { return _programsForBrandMap; }
  const ProgramsForBrandMap& getProgramsForBrandMap() const { return _programsForBrandMap; }

  // Fails assertion if not initialized
  skipper::ItinBranding& getItinBranding()
  {
    TSE_ASSERT(_itinBranding != nullptr);
    return *_itinBranding;
  }

  // Fails assertion if not initialized
  const skipper::ItinBranding& getItinBranding() const
  {
    TSE_ASSERT(_itinBranding != nullptr);
    return *_itinBranding;
  }

  bool isItinBrandingSet() const
  {
    return _itinBranding != nullptr;
  }

  // Should be called exactly once
  void setItinBranding(skipper::ItinBranding* itinBranding);

  ItinLegs& itinLegs() { return _itinLegs; }
  const ItinLegs& itinLegs() const { return _itinLegs; }

  InterlineJourneyInfoMap& interlineJourneyInfo() { return _interlineJourneyInfo; }
  const InterlineJourneyInfoMap& interlineJourneyInfo() const { return _interlineJourneyInfo; }

  InterlineJourneyMarketMap& interlineJourneyMarket() { return _interlineJourneyMarket; }
  const InterlineJourneyMarketMap& interlineJourneyMarket() const
  {
    return _interlineJourneyMarket;
  }

  size_t getTODBucket(const std::vector<std::pair<uint16_t, uint16_t> >& todRanges) const;

  void calculateMileage(PricingTrx* trx);
  uint32_t getMileage() const { return _mileage; }

  // ******************************** ESV/VIS  ******************************** //
  void updateTotalPenalty(const ESVOptions* esvOptions, const DateTime& reqDepDateTime);
  void calculateFlightTimeMinutes(const int32_t& hoursODOffset, const int32_t& legIdx);

  const TravelSeg* firstTravelSeg() const { return travelSeg()[0]; }
  const TravelSeg* lastTravelSeg() const { return travelSeg()[travelSeg().size() - 1]; }

  const DateTime& getDepartTime() const;
  const DateTime& getArrivalTime() const;

  bool& isJcb() { return _isJcb; }
  const bool& isJcb() const { return _isJcb; }

  const MoneyAmount& totalPenalty() const { return _totalPenalty; }

  CarrierCode& onlineCarrier() { return _onlineCarrier; }
  const CarrierCode& onlineCarrier() const { return _onlineCarrier; }

  std::map<PaxType*, SOPFareList>& paxTypeSOPFareListMap() { return _paxTypeSOPFareListMap; }
  const std::map<PaxType*, SOPFareList>& paxTypeSOPFareListMap() const
  {
    return _paxTypeSOPFareListMap;
  }

  bool& dominatedFlight() { return _dominatedFlight; }
  const bool& dominatedFlight() const { return _dominatedFlight; }

  const int32_t& getFlightTimeMinutes() const { return _flightTimeMinutes; }

  std::vector<ESVSopWrapper*>& sopWrapperVec() { return _sopWrapperVec; }
  const std::vector<ESVSopWrapper*>& sopWrapperVec() const { return _sopWrapperVec; }

  // ******************************** ESV/VIS  ******************************** //

  IntIndex& itinNum() { return _itinNum; }
  const IntIndex& itinNum() const { return _itinNum; }

  // ******************************** MIP ******************************** //
  const uint16_t& getItinIndex() const { return _itinIndex; }
  // void setExcItinIndex(uint16_t index,size_t maxIndex = 1 );
  virtual void setItinIndex(uint16_t index) { _itinIndex = index; }
  // ******************************** EXC/MIP  ******************************** //

  int getItinFamily() const { return _itinFamily; }
  void setItinFamily(int family) { _itinFamily = family; }

  void markAsHeadOfFamily() { _familyLogicType = HEAD_OF_FAMILY; }
  bool isHeadOfFamily() const { return _familyLogicType == HEAD_OF_FAMILY; }

  void markAsSimilarItin() { _familyLogicType = SIMILAR_ITIN; }
  bool isSimilarItin() const { return _familyLogicType == SIMILAR_ITIN; }

  // ******************************** QREX/BSP  ******************************** //
  Indicator& exchangeReissue() { return _reissueVsExchange; }
  const Indicator& exchangeReissue() const { return _reissueVsExchange; }
  // ******************************** QREX/BSP  ******************************** //

  // ******************************** OC FEES  ******************************** //
  std::vector<ServiceFeesGroup*>& ocFeesGroup() { return _serviceFeesGroups; }
  const std::vector<ServiceFeesGroup*>& ocFeesGroup() const { return _serviceFeesGroups; }

  std::vector<ServiceFeesGroup*>& ocFeesGroupsFreeBag() { return _serviceFeesGroupsFreeBag; }
  const std::vector<ServiceFeesGroup*>& ocFeesGroupsFreeBag() const { return _serviceFeesGroupsFreeBag; }

  bool& moreFeesAvailable() { return _moreFeesAvailable; }
  const bool& moreFeesAvailable() const { return _moreFeesAvailable; }

  bool& allSegsConfirmed() { return _allSegsConfirmed; }
  const bool allSegsConfirmed() const { return _allSegsConfirmed; }

  bool& allSegsUnconfirmed() { return _allSegsUnconfirmed; }
  const bool allSegsUnconfirmed() const { return _allSegsUnconfirmed; }

  bool& timeOutForExceeded() { return _timeOutForExceeded; }
  const bool& timeOutForExceeded() const { return _timeOutForExceeded; }

  bool& timeOutForExceededSFGpresent() { return _timeOutForExceededSFGpresent; }
  const bool& timeOutForExceededSFGpresent() const { return _timeOutForExceededSFGpresent; }

  bool& timeOutOCFForWP() { return _timeOutOCFForWP; }
  const bool& timeOutOCFForWP() const { return _timeOutOCFForWP; }

  const uint16_t& getItinOrderNum() const { return _itinOrderNum; }
  void setItinOrderNum(uint16_t index) { _itinOrderNum = index; }

  const BaggageTripType& getBaggageTripType() const { return _baggageTripType; }
  void setBaggageTripType(const BaggageTripType& btt) { _baggageTripType = btt; }

  // ******************************** OC FEES  ******************************** //
  // **********  Multi ticket project *****************
  const uint16_t& getMultiTktItinOrderNum() const { return _itinMultiTktOrderNum; }
  void setMultiTktItinOrderNum(uint16_t index) { _itinMultiTktOrderNum = index; }
  MultiTicketUtil::TicketSolution& ticketSolution() { return _ticketSolution; }
  const MultiTicketUtil::TicketSolution& ticketSolution() const { return _ticketSolution; }
  void setTicketSolution(MultiTicketUtil::TicketSolution solution) { _ticketSolution = solution; }
  // **********  Multi ticket project *****************

  bool isThroughFarePrecedence() const { return _throughFarePrecedence; }
  void setThroughFarePrecedence(bool t) { _throughFarePrecedence = t; }

  std::string& anciliaryServiceCode() { return _anciliaryServiceCode; }
  const std::string& anciliaryServiceCode() const { return _anciliaryServiceCode; }

  // ******************************** TAX SERVICE  ******************************** //

  Agent*& agentPCCOverride() { return _agentPCCOverride; }
  const Agent* agentPCCOverride() const { return _agentPCCOverride; }

  const DateTime& getMaxDepartureDT() const { return _maxDepartureDT; }
  void setMaxDepartureDT(const DateTime& maxDepartDT) { _maxDepartureDT = maxDepartDT; }

  void setThruFareOnly(bool thruFareOnly) { _thruFareOnly = thruFareOnly; }
  bool isThruFareOnly() const { return _thruFareOnly; }

  void addFareMarketJustForRexPlusUps(const FareMarket* fm);
  bool isFareMarketJustForRexPlusUps(const FareMarket* fm) const;

  bool& cat05BookingDateValidationSkip() { return _cat05BookingDateValidationSkip; }
  const bool& cat05BookingDateValidationSkip() const { return _cat05BookingDateValidationSkip; }
  YQYRLBFactoryPtr& yqyrLBFactory() { return _yqyrLBFactory; }
  const YQYRLBFactoryPtr& yqyrLBFactory() const { return _yqyrLBFactory; }

  bool isOcFeesFound() const { return _isOcFeesFound; }
  void setOcFeesFound(bool value) { _isOcFeesFound = value; }

  uint8_t ticketNumber() const { return _ticketNumber; }
  void setTicketNumber(uint8_t tkn) { _ticketNumber = tkn; }

  void setMemoryManager(Memory::LocalManager* manager) { _memoryManager = manager; }
  Memory::LocalManager* getMemoryManager() const { return _memoryManager; }

  std::vector<FareCompInfo*>& fareComponent() { return _fareComponent; }
  const std::vector<FareCompInfo*>& fareComponent() const { return _fareComponent; }

  bool isGeoConsistent(const Itin& itinToCompare) const;

  void setPrecalcBagData(PrecalcBaggage::ItinData* pbd) { _precalcBaggageData = pbd; }
  const PrecalcBaggage::ItinData* getPrecalcBagData() const { return _precalcBaggageData; }
  const PrecalcBaggage::PaxData& getPrecalcBagPaxData(const PaxType& pt) const;

  const MaxPenaltyStats& maxPenaltyStats() const { return _maxPenaltyStats; }
  MaxPenaltyStats& maxPenaltyStats() { return _maxPenaltyStats; }

  size_t countPaxTypeInFarePaths(const PaxType&) const;
  bool hasAtLeastOneEmptyFMForGovCxr(CarrierCode cxr) const;

  std::map<ClassOfService*, size_t>& getFamilyAggregatedAvlMap()
  {
    return _familyAggregatedAvlMap;
  }

  void addAncillaryPriceModifier(const AncillaryIdentifier& ancillaryIdentifier, const AncillaryPriceModifier& ancillaryPriceModifier)
  {
    _ancillaryPriceModifiers[ancillaryIdentifier].push_back(ancillaryPriceModifier);
  }

  const AncillaryPriceModifiersMap& getAncillaryPriceModifiers() const
  {
    return _ancillaryPriceModifiers;
  }

protected:
  DateTime _travelDate;

private:
  Memory::LocalManager* _memoryManager = nullptr;
  ValidatingCxrGSAData* _validatingCxrGsaData = nullptr;
  SpValidatingCxrGSADataMap* _spValidatingCxrGsaDataMap = nullptr;
  CarrierPreference* _ticketingCarrierPref = nullptr;
  CarrierCode _ticketingCarrier{"XX"};
  CarrierCode _validatingCarrier;
  CarrierCode _traditionalValidatingCxr;
  uint32_t _sequenceNumber = 0;

  int16_t _furthestPointSegmentOrder = 0;
  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;
  bool _salesNationRestr = false;

  bool _SRFEApplicable = false;

  ErrorResponseException::ErrorResponseCode _errResponseCode =
      ErrorResponseException::ErrorResponseCode::NO_ERROR;
  std::string _errResponseMsg;

  std::vector<TravelSeg*> _travelSeg;
  std::vector<FareMarket*> _fareMarket;
  std::vector<FareMarket*> _flowFareMarket;
  std::vector<FarePath*> _farePath;
  std::vector<TaxResponse*> _taxResponse;
  std::vector<TaxResponse*> _valCxrTaxResponse;
  std::vector<ClassOfService*> _origBooking;
  std::vector<OAndDMarket*> _OAndDMarkets;

  JourneyUtil::SegOAndDMap _segmentOAndDMarket;
  FmpMatrixPtr _fmpMatrix;
  std::map<BrandCode, FmpMatrixPtr> _brandFmpMatrices;
  std::map<flexFares::GroupId, FmpMatrixPtr> _groupsFmpMatrices;

  SmallBitSet<uint16_t, TripCharacteristics> _tripCharacteristics;

  CurrencyCode _calculationCurrency;
  CurrencyCode _originationCurrency;
  CurrencyCode _calcCurrencyOverride;

  ISICode _intlSalesIndicator = ISICode::UNKNOWN;

  bool _simpleTrip = false;
  bool _inboundSop = false;
  bool _useInternationalRounding = false;

  std::vector<PaxType*> _paxGroup;
  DateTime _bookingDate;
  DateTime _maxDepartureDT{0}; // to set SOFTPASS status in validation cat6 for SOL

  // leg/option ID, required for MIP
  std::vector<std::pair<int, int> > _legID;
  std::vector<std::string> _csTextMessages;

  // DatePair pointer
  DatePair* _datePair = nullptr;

  // similar itins to this itin. Used by Shopping/MIP to share
  // results between itins.
  std::vector<SimilarItinData> _similarItinsData;

  // parsed from request for MIP/Solo Carnival diagnostics
  int _itinFamily = INVALID_INT_INDEX;
  FamilyLogicType _familyLogicType = NO_FAMILY;
  bool _dcaSecondCall = false;
  bool _isDummy = false;
  WQDateType _dateType = Itin::WQDateType::NoDate;

  // for Plus Up Pricing
  ConsolidatorPlusUp* _consolidatorPlusUp = nullptr;

  MoneyAmount _estimatedTax = 0;

  std::string _anciliaryServiceCode;
  IntIndex _itinNum = INVALID_INT_INDEX; // ISELL-MIP optimization
  bool _thruFareOnly = false;
  bool _hasSideTrip = false;
  uint32_t _mileage = 0;

  InterlineJourneyInfoMap _interlineJourneyInfo;
  InterlineJourneyMarketMap _interlineJourneyMarket; // diag 199, 990

  BrandCodeSet _brandCodes;
  BrandFilterMap _brandFilterMap;
  ProgramsForBrandMap _programsForBrandMap;
  // convenience object for TN Shopping calculations
  skipper::ItinBranding* _itinBranding = nullptr;

  IbfAvailabilityTracker _ibfAvailabilityTracker;

  ItinLegs _itinLegs;

  // ******************************** ESV/VIS  ******************************** //

  MoneyAmount _totalPenalty = 0;
  CarrierCode _onlineCarrier;
  int32_t _flightTimeMinutes = 0;
  std::map<PaxType*, SOPFareList> _paxTypeSOPFareListMap;
  std::vector<ESVSopWrapper*> _sopWrapperVec;
  bool _isJcb = false;
  bool _dominatedFlight = false;
  // ******************************** ESV/VIS  ******************************** //

  // ******************************** EXC/MIP  ******************************** //
  uint16_t _itinIndex = 0; // to keep info which itin is compared to
  // ******************************** EXC/MIP  ******************************** //

  // ******************************** OC FEES  ******************************** //
  bool _moreFeesAvailable = false;
  bool _allSegsConfirmed = true;
  bool _allSegsUnconfirmed = false;
  bool _timeOutForExceeded = false; // time out, no OC requested, no OC returned
  bool _timeOutForExceededSFGpresent = false; // time out, OC requested and returned
  bool _timeOutOCFForWP = false; // time out on OC for WP, no OC returned
  uint16_t _itinOrderNum = 0; // itinerary number needs for Ancillary and AA Baggage
  BaggageTripType _baggageTripType;
  std::vector<ServiceFeesGroup*> _serviceFeesGroups; // OC Fees
  std::vector<ServiceFeesGroup*> _serviceFeesGroupsFreeBag; // OC Fees for FREE BAG service
  PrecalcBaggage::ItinData* _precalcBaggageData = nullptr;
  AncillaryPriceModifiersMap _ancillaryPriceModifiers;
  // ******************************** OC FEES  ******************************** //

  // ******************************** QREX/BSP  ******************************** //
  Indicator _reissueVsExchange = BLANK; // based on the changes to the first flight sector
  // value '2' - exchange, value '1' - reissue
  // ******************************** QREX/BSP  ******************************** //

  // **********  Multi ticket project ******************
  bool _throughFarePrecedence = false;
  uint16_t _itinMultiTktOrderNum = 0;
  MultiTicketUtil::TicketSolution _ticketSolution =
      MultiTicketUtil::TicketSolution::NO_SOLUTION_FOUND;
  // ******************************** TAX SERVICE  ******************************** //
  Agent* _agentPCCOverride = nullptr;
  YQYRLBFactoryPtr _yqyrLBFactory;
  void duplicateBase(const Itin& itin, DataHandle& dataHandle);
  //************INFINI CAT05 booking date validation*********//
  bool _cat05BookingDateValidationSkip = false;

  std::vector<FareCompInfo*> _fareComponent;
  std::map<ClassOfService*, size_t> _familyAggregatedAvlMap;

  std::set<const FareMarket*> _fmsJustForRexPlusUps;
  bool _isOcFeesFound = false;
  uint8_t _ticketNumber = 0;
  MaxPenaltyStats _maxPenaltyStats = {{false, 0.0}, {false, 0.0}, 0};
};

class SOPFarePath
{
public:
  enum CombinationType
  {
    NOT_INITIALIZED,
    R,
    X,
    RR,
    RX,
    XR,
    XX
  };

  std::vector<PaxTypeFare*>& paxTypeFareVec() { return _paxTypeFareVec; }
  const std::vector<PaxTypeFare*>& paxTypeFareVec() const { return _paxTypeFareVec; }

  std::vector<FareMarket*>& fareMarketPath() { return _fareMarketPath; }
  const std::vector<FareMarket*>& fareMarketPath() const { return _fareMarketPath; }

  MoneyAmount& totalAmount() { return _totalAmount; }
  const MoneyAmount& totalAmount() const { return _totalAmount; }

  bool& haveCat10Restrictions() { return _haveCat10Restrictions; }
  const bool& haveCat10Restrictions() const { return _haveCat10Restrictions; }

  CombinationType& combinationType() { return _combinationType; }
  const CombinationType& combinationType() const { return _combinationType; }

  static bool compare(SOPFarePath* sopFarePath1, SOPFarePath* sopFarePath2)
  {
    return sopFarePath1->totalAmount() < sopFarePath2->totalAmount();
  }

private:
  std::vector<PaxTypeFare*> _paxTypeFareVec;
  std::vector<FareMarket*> _fareMarketPath;
  MoneyAmount _totalAmount = 0;
  bool _haveCat10Restrictions = true;
  CombinationType _combinationType = CombinationType::NOT_INITIALIZED;
};

class SOPFareList
{
public:
  std::vector<SOPFarePath*>& owSopFarePaths() { return _owSopFarePaths; }
  const std::vector<SOPFarePath*>& owSopFarePaths() const { return _owSopFarePaths; }

  std::vector<SOPFarePath*>& rtSopFarePaths() { return _rtSopFarePaths; }
  const std::vector<SOPFarePath*>& rtSopFarePaths() const { return _rtSopFarePaths; }

  std::vector<SOPFarePath*>& ctSopFarePaths() { return _ctSopFarePaths; }
  const std::vector<SOPFarePath*>& ctSopFarePaths() const { return _ctSopFarePaths; }

  std::vector<SOPFarePath*>& ojSopFarePaths() { return _ojSopFarePaths; }
  const std::vector<SOPFarePath*>& ojSopFarePaths() const { return _ojSopFarePaths; }

private:
  std::vector<SOPFarePath*> _owSopFarePaths;
  std::vector<SOPFarePath*> _rtSopFarePaths;
  std::vector<SOPFarePath*> _ctSopFarePaths;
  std::vector<SOPFarePath*> _ojSopFarePaths;
};
} // tse namespace
