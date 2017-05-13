//-------------------------------------------------------------------
//
//  File:        ShoppingTrx.h
//  Created:     June 2, 2004
//  Design:      Mark Kasprowicz
//  Authors:
//
//  Description: Shopping Transaction object
//
//  Updates:
//    07/09/04 - Adrienne A. Stipe - adding TravelSeg and Diagnostic member
//                     vars
//      08/04/04 - Gerald LePage     - added ScheduleGroup map, pair, and
//                                     vector types
//      08/13/04 - Gerald LePage     - added ScheduleGroup vector type
//      09/07/04 - Gerald LePage     - changed schedule group keys to ints
//      09/29/04 - Adrienne A. Stipe - added globalDirectionVector
//      10/01/04 - Gerald LePage     - Removed global grouping typedefs and
//                                     set Leg to utilized ItinGroup class
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

#include "BookingCode/BCETuning.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Diversity.h"
#include "DataModel/ESVSolution.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "DataModel/VISOptions.h"
#include "Pricing/Shopping/IBF/IbfData.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace tse
{
class ClassOfService;
class FareMarket;
class GroupFarePath;
class ShoppingPQ;
class TravelSeg;

// Across stop over leg segment surface sector id
const uint32_t ASOLEG_SURFACE_SECTOR_ID = 0xffffffff;

class ShoppingTrx : public PricingTrx
{
public:

  enum INTERLINEONLINE
  {
    NOTDETERMINED,
    ONLINE,
    INTERLINE
  };

  class SchedulingOption
  {
  public:
    SchedulingOption(Itin* itin,
                     const uint32_t& originalSopId,
                     bool cabinClassValid = true,
                     bool combineSameCxr = false);

    Itin*& itin() { return _itin; }
    const Itin* itin() const { return _itin; }

    bool& cabinClassValid() { return _cabinClassValid; }
    bool cabinClassValid() const { return _cabinClassValid; }

    GlobalDirection& globalDirection() { return (_globalDirection); }
    const GlobalDirection& globalDirection() const { return (_globalDirection); }

    CarrierCode& governingCarrier() { return (_governingCarrier); }
    const CarrierCode& governingCarrier() const { return (_governingCarrier); }

    uint32_t& originalSopId() { return (_originalSopId); }
    const uint32_t& originalSopId() const { return (_originalSopId); }

    uint32_t& sopId() { return (_sopId); }
    const uint32_t& sopId() const { return (_sopId); }

    bool& combineSameCxr() { return _combineSameCxr; }
    bool combineSameCxr() const { return _combineSameCxr; }

    std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService()
    {
      return _thrufareClassOfService;
    }

    const std::vector<std::vector<ClassOfService*>*>& thrufareClassOfService() const
    {
      return _thrufareClassOfService;
    }

    bool getDummy() const { return _itin->isDummy(); }
    void setDummy(bool isDummy) { _itin->isDummy() = isDummy; }

    std::vector<uint32_t>& availabilityIds() { return _availabilityIds; }
    const std::vector<uint32_t>& availabilityIds() const { return _availabilityIds; }

    bool& domesticCnxTimeMoreThan4() { return _domesticCnxTimeMoreThan4; }
    const bool& domesticCnxTimeMoreThan4() const { return _domesticCnxTimeMoreThan4; }

    bool isInterline() const;

    bool& isLngCnxSop() { return _isLngCnxSop; }
    bool isLngCnxSop() const { return _isLngCnxSop; }

    bool isCustomSop() const { return _isCustomSop; }
    void setCustomSop(bool cus) { _isCustomSop = cus; }

    bool& isHighTPM() { return _isHighTPM; }
    bool isHighTPM() const { return _isHighTPM; }

  private:
    Itin* _itin;
    bool _cabinClassValid;
    GlobalDirection _globalDirection = GlobalDirection::ZZ;
    CarrierCode _governingCarrier;
    uint32_t _originalSopId; // Scheduling option Id from request
    uint32_t _sopId = 0; // Position of scheduling option in leg vector
    bool _combineSameCxr;
    std::vector<std::vector<ClassOfService*>*> _thrufareClassOfService;
    std::vector<uint32_t> _availabilityIds;
    // domestic connecting flight time
    bool _domesticCnxTimeMoreThan4 = false;
    bool _isLngCnxSop = false;
    bool _isCustomSop = false;
    bool _isHighTPM = false;
    mutable INTERLINEONLINE _interline = INTERLINEONLINE::NOTDETERMINED;

    INTERLINEONLINE checkInterline() const;
  };

  class Leg
  {
  public:
    enum SurfaceSectorLegType
    {
      SurfaceSectorLegNone = 0,
      SurfaceSectorEndAtLegOrig = 1,
      SurfaceSectorBeginAtLegDest = 2,
      SurfaceSectorEndAtLegOrigBeginAtLegDest = 3
    };

    Leg() { _preferredCabinClass.setEconomyClass(); }

    // Computes flight bitmap size for this leg, for the given governing
    // carrier key
    uint32_t getFlightBitmapSize(ShoppingTrx& trx, const ItinIndex::Key& itinRowKey);

    // These methods work for across stop over legs only
    void generateAcrossStopOverCombinations(ShoppingTrx& trx);

    // govCxrs[i] = governing carrier of SOP with index i
    // Create sopMap st.
    // sopMap[i] = index of i-th SOP with governing carrier indicated by cxr
    void createCxrSopMap(const std::vector<ItinIndex::Key>& govCxrs,
                         ItinIndex::Key cxr,
                         std::vector<uint32_t>& sopMap) const;

    bool segBeginsLeg(const TravelSeg* seg) const { return _beginTravelSegs.count(seg) != 0; }

    bool segEndsLeg(const TravelSeg* seg) const { return _endTravelSegs.count(seg) != 0; }

    void addSop(const SchedulingOption& newSop)
    {
      _sop.push_back(newSop);
      setSopTravelSegs(newSop);
    }

    void setSopTravelSegs(const SchedulingOption& newSop)
    {
      _beginTravelSegs.insert(newSop.itin()->travelSeg().front());
      _endTravelSegs.insert(newSop.itin()->travelSeg().back());
    }

    std::vector<SchedulingOption>& sop() { return _sop; }
    const std::vector<SchedulingOption>& sop() const { return _sop; }
    bool isSopEmpty() { return _sop.empty(); }

    uint32_t& requestSops() { return _requestSops; }
    uint32_t requestSops() const { return _requestSops; }

    CabinType& preferredCabinClass() { return _preferredCabinClass; }
    const CabinType& preferredCabinClass() const { return _preferredCabinClass; }

    // IB/OB directional indicator for the leg
    FMDirection& directionalIndicator() { return _directionalIndicator; }
    const FMDirection& directionalIndicator() const { return _directionalIndicator; }

    ItinIndex& carrierIndex() { return _carrierIndex; }
    const ItinIndex& carrierIndex() const { return _carrierIndex; }

    bool& stopOverLegFlag() { return _stopOverLegFlag; }
    const bool& stopOverLegFlag() const { return _stopOverLegFlag; }

    IndexPair& stopOverLegCombination() { return _stopOverLegCombination; }
    const IndexPair& stopOverLegCombination() const { return _stopOverLegCombination; }

    std::pair<int16_t, int16_t>& journeyIndexPair() { return _journeyIndexPair; }
    const std::pair<int16_t, int16_t>& journeyIndexPair() const { return _journeyIndexPair; }

    IndexVector& jumpedLegIndices() { return _jumpedLegIndices; }
    const IndexVector& jumpedLegIndices() const { return _jumpedLegIndices; }

    uint32_t& adoptedCrossedLegRefIndex() { return _adoptedCrossedLegRefIndex; }
    const uint32_t& adoptedCrossedLegRefIndex() const { return _adoptedCrossedLegRefIndex; }

    SurfaceSectorLegType& surfaceSectorLegType() { return _surfaceSectorLegType; }
    const SurfaceSectorLegType& surfaceSectorLegType() const { return _surfaceSectorLegType; }

    std::map<uint32_t, IndexPair>& surfaceSectorMap() { return _surfaceSectorMap; }
    const std::map<uint32_t, IndexPair>& surfaceSectorMap() const { return _surfaceSectorMap; }

    std::map<ItinIndex::Key, IndexVectors>& acrossStopOverCombinations()
    {
      return _acrossStopOverCombinations;
    }

    const std::map<ItinIndex::Key, IndexVectors>& acrossStopOverCombinations() const
    {
      return _acrossStopOverCombinations;
    }

    bool& dateChange() { return _dateChange; }
    const bool& dateChange() const { return _dateChange; }

    IntIndex& originalId() { return _originalId; }
    const IntIndex& originalId() const { return _originalId; }

    int getMinDurationSopIdx() const { return _minDurationSopIdx; }
    void setMinDurationSopIdx(int value) { _minDurationSopIdx = value; }

    bool isCustomLeg() const { return _isCustomLeg; }
    void setCustomLeg(bool cus) { _isCustomLeg = cus; }

    void setReturnAllFlights() { _returnAllFlights = true; }
    bool isReturnAllFlights() const { return _returnAllFlights; }

  private:
    ItinIndex _carrierIndex;
    std::vector<SchedulingOption> _sop;
    std::set<const TravelSeg*> _beginTravelSegs;
    std::set<const TravelSeg*> _endTravelSegs;
    uint32_t _requestSops = 0;
    // Beginning and end index for the stop over leg
    IndexPair _stopOverLegCombination;
    std::pair<int16_t, int16_t> _journeyIndexPair;
    IndexVector _jumpedLegIndices;
    std::map<ItinIndex::Key, IndexVectors> _acrossStopOverCombinations;
    std::map<ItinIndex::Key, uint32_t> _flightBitmapSizeMap;
    // Index of the primary international service leg we have crossed if
    // this is a "across" stop over leg
    uint32_t _adoptedCrossedLegRefIndex = 0;
    CabinType _preferredCabinClass;
    FMDirection _directionalIndicator = FMDirection::UNKNOWN;
    bool _stopOverLegFlag = false;
    SurfaceSectorLegType _surfaceSectorLegType = ShoppingTrx::Leg::SurfaceSectorLegNone;
    std::map<uint32_t, IndexPair> _surfaceSectorMap;
    bool _dateChange = false;
    // Q14 from request (-1 means that it's not set)
    IntIndex _originalId = INVALID_INT_INDEX;
    int _minDurationSopIdx = -1;
    bool _isCustomLeg = false;
    bool _returnAllFlights = false;
  };

  using FlightMatrix = std::map<SopIdVec, GroupFarePath*>;
  using EstimatedSolution = std::pair<SopIdVec, GroupFarePath*>;
  using EstimateMatrix = std::map<SopIdVec, EstimatedSolution>;

  struct CxrOnlinOptions
  {
    CarrierCode carrier;
    uint16_t numberOnlineOptions = 0;
    uint16_t maxPossibleCombinations = 0;
    std::vector<uint16_t> cxrSOPCount; // for each leg
  };

  struct PQDiversifierResult
  {
    std::vector<CarrierCode> onlineCarrierList;
    std::map<CarrierCode, CxrOnlinOptions> cxrOnlinOptions;
    uint32_t numberInterlineOptions = 0;
    uint32_t _numberOnlineOptionsPerCarrier = 0;
    uint32_t totalOnlineOptions = 0;
    uint32_t _minNumCustomSolutions = 0;

    std::atomic<int32_t> _currentLngCnxOptionCount{0};
    std::atomic<int32_t> _currentCustomOptionCount{0};

    bool validResults = false;

    void incrementCurrentLngCnxOptionCount()
    {
      _currentLngCnxOptionCount.fetch_add(1, std::memory_order_relaxed);
    }

    uint32_t currentLngCnxOptionCount()
    {
      return _currentLngCnxOptionCount.load(std::memory_order_relaxed);
    }

    void incrementCurrentCustomOptionCount()
    {
      _currentCustomOptionCount.fetch_add(1, std::memory_order_relaxed);
    }

    void decrementCurrentCustomOptionCount()
    {
      _currentCustomOptionCount.fetch_sub(1, std::memory_order_relaxed);
    }

    uint32_t currentCustomOptionCount()
    {
      return _currentCustomOptionCount.load(std::memory_order_relaxed);
    }
  };

  struct FareMarketRuleTuning
  {
    std::vector<std::vector<BCETuning>> _shoppingBCETuningData;
  };

  using FareMarketRuleMap = std::map<FareMarket*, FareMarketRuleTuning>;
  using PQPtr = std::shared_ptr<ShoppingPQ>;
  using ShoppingPQVector = std::vector<PQPtr>;

  struct FoundOptionInfo
  {
    MoneyAmount lowestOptionAmount = 10000000;
    MoneyAmount lowestOptionAmountForSnowman = 10000000;
  };

  using AltDateLowestAmount = std::map<DatePair, FoundOptionInfo*>;
  using ForcedConnectionSet = std::multiset<LocCode>;

  static constexpr char BLANK = ' ';
  static constexpr char GROUPBYCARRIER = '1';
  static constexpr char GROUPBYDATE = '2';
  static constexpr char GROUPBYCARRIERANDDATE = '3';

private:
  IbfData* _ibfData = nullptr;

protected:
  std::vector<Leg> _legs;

  Itin* _journeyItin = nullptr;
  FlightMatrix _flightMatrix;
  EstimateMatrix _estimateMatrix;

  std::vector<ESVSolution*> _flightMatrixESV;
  std::vector<ESVSolution*> _estimateMatrixESV;

  // AltDatePairs _altDatePairs;
  //  bool _altDates;
  AltDateLowestAmount _altDateLowestAmount;
  boost::mutex _altDateLowestAmountMutex;

  PQDiversifierResult _pqDiversifierResult;
  std::map<uint32_t, IndexPair> _externalSopToLegMap;

  RuleControllerWithChancelor<FareMarketRuleController> _fareMarketRuleController;
  RuleControllerWithChancelor<FareMarketRuleController> _ASOfareMarketRuleController;
  RuleControllerWithChancelor<FareMarketRuleController> _cat4RuleController;
  FareMarketRuleMap _fareMarketRuleMap;

  bool _interlineSolutionsOnly = false;
  bool _onlineSolutionsOnly = false;
  bool _noDiversity = false;
  int _interlineWeightFactor = 0;
  int _minDuration = -1;
  int _maxDuration = -1;
  DateTime _travelDate;
  DateTime _returnDate;
  char _groupMethodForMip = ShoppingTrx::BLANK;

  bool _noASOLegs = false;
  uint32_t _asoBitmapThreshold = 0;
  uint32_t _asoConxnPointLimit = 0;
  int32_t _asoMaxValidations = -1;

  int _requestedNumOfEstimatedSolutions = -1;
  uint32_t _maxNumOfLngCnxSolutions = 0;
  bool _altDateReuseOutBoundProcessed = false;
  bool _altDateReuseInBoundProcessed = false;

  bool _throughFarePrecedencePossible = false;
  bool _throughFarePrecedenceFoundInvalidFarePath = false;

  std::vector<PaxType*> _excludedPaxType; // IS excludes Multi-PaxType
  ESVOptions* _esvOptions = nullptr;
  VISOptions* _visOptions = nullptr;

  ShoppingPQVector _shoppingPQVector;
  bool _simpleTrip = false; // simple oneway or simpleRoundTrip
  bool _ruleTuningISProcess = false;
  ForcedConnectionSet _forcedConnectionRexShopping;
  Diversity* _diversity = nullptr;
  bool _solNoLocalInd = false;

  ShpBitValidationCollector _bitValidationCollector;

  std::set<const FareMarket*> _customSolutionFmSet;
  std::set<const FareMarket*> _spanishDiscountFmSet;

  int16_t _numOfCustomSolutions = 0;

  bool _noMoreLngConOptionsNeeded = false;

public:
  ShoppingTrx(Diversity* diversity = nullptr);

  virtual bool process(Service& srv) override { return srv.process(*this); }

  std::vector<Leg>& legs() { return _legs; }
  const std::vector<Leg>& legs() const { return _legs; }

  std::size_t numberOfLegs() const { return _legs.size(); }

  Itin*& journeyItin() { return _journeyItin; }
  const Itin* journeyItin() const { return _journeyItin; }

  FlightMatrix& flightMatrix() { return _flightMatrix; }
  const FlightMatrix& flightMatrix() const { return _flightMatrix; }

  EstimateMatrix& estimateMatrix() { return _estimateMatrix; }
  const EstimateMatrix& estimateMatrix() const { return _estimateMatrix; }

  std::vector<ESVSolution*>& flightMatrixESV() { return _flightMatrixESV; }
  const std::vector<ESVSolution*>& flightMatrixESV() const { return _flightMatrixESV; }

  std::vector<ESVSolution*>& estimateMatrixESV() { return _estimateMatrixESV; }
  const std::vector<ESVSolution*>& estimateMatrixESV() const { return _estimateMatrixESV; }

  // AltDatePairs& altDatePairs() { return _altDatePairs; }
  // const AltDatePairs& altDatePairs() const { return _altDatePairs; }
  AltDateLowestAmount& altDateLowestAmount() { return _altDateLowestAmount; }
  const AltDateLowestAmount& altDateLowestAmount() const { return _altDateLowestAmount; }
  boost::mutex& altDateLowestAmountMutex() { return _altDateLowestAmountMutex; }
  const boost::mutex& altDateLowestAmountMutex() const { return _altDateLowestAmountMutex; }

  PQDiversifierResult& pqDiversifierResult() { return (_pqDiversifierResult); }
  const PQDiversifierResult& pqDiversifierResult() const { return (_pqDiversifierResult); }

  FareMarketRuleController& fareMarketRuleController() { return (_fareMarketRuleController); }
  const FareMarketRuleController& fareMarketRuleController() const
  {
    return (_fareMarketRuleController);
  }

  FareMarketRuleController& ASOfareMarketRuleController() { return (_ASOfareMarketRuleController); }
  const FareMarketRuleController& ASOfareMarketRuleController() const
  {
    return (_ASOfareMarketRuleController);
  }

  FareMarketRuleController& cat4RuleController() { return (_cat4RuleController); }
  const FareMarketRuleController& cat4RuleController() const { return (_cat4RuleController); }

  std::map<uint32_t, IndexPair>& externalSopToLegMap() { return (_externalSopToLegMap); }
  const std::map<uint32_t, IndexPair>& externalSopToLegMap() const
  {
    return (_externalSopToLegMap);
  }

  bool& onlineSolutionsOnly() { return _onlineSolutionsOnly; }
  const bool& onlineSolutionsOnly() const { return _onlineSolutionsOnly; }

  bool& interlineSolutionsOnly() { return _interlineSolutionsOnly; }
  const bool& interlineSolutionsOnly() const { return _interlineSolutionsOnly; }

  bool& noDiversity() { return _noDiversity; }

  bool noDiversity() const { return _noDiversity; }

  int& interlineWeightFactor() { return _interlineWeightFactor; }
  const int& interlineWeightFactor() const { return _interlineWeightFactor; }

  int32_t& minDuration() { return _minDuration; }
  const int32_t& minDuration() const { return _minDuration; }

  int32_t& maxDuration() { return _maxDuration; }
  const int32_t& maxDuration() const { return _maxDuration; }

  void setTravelDate(DateTime travelDate) { _travelDate = travelDate; }
  const DateTime getTravelDate() const { return _travelDate; }
  void setReturnDate(DateTime returnDate) { _returnDate = returnDate; }
  const DateTime getReturnDate() const { return _returnDate; }
  char& groupMethodForMip() { return _groupMethodForMip; }
  const char& groupMethodForMip() const { return _groupMethodForMip; }

  // For disabling ASO legs on a transactional basis
  bool& noASOLegs() { return (_noASOLegs); }
  const bool& noASOLegs() const { return (_noASOLegs); }

  uint32_t& asoBitmapThreshold() { return (_asoBitmapThreshold); }
  const uint32_t& asoBitmapThreshold() const { return (_asoBitmapThreshold); }

  uint32_t& asoConxnPointLimit() { return (_asoConxnPointLimit); }
  const uint32_t& asoConxnPointLimit() const { return (_asoConxnPointLimit); }

  int32_t& asoMaxValidations() { return _asoMaxValidations; }
  const int32_t& asoMaxValidations() const { return _asoMaxValidations; }

  void setRequestedNumOfEstimatedSolutions(int value) { _requestedNumOfEstimatedSolutions = value; }
  int getRequestedNumOfEstimatedSolutions() const { return _requestedNumOfEstimatedSolutions; }

  uint32_t& maxNumOfLngCnxSolutions() { return _maxNumOfLngCnxSolutions; }
  const uint32_t& maxNumOfLngCnxSolutions() const { return _maxNumOfLngCnxSolutions; }

  FareMarketRuleMap& fareMarketRuleMap() { return _fareMarketRuleMap; }
  const FareMarketRuleMap& fareMarketRuleMap() const { return _fareMarketRuleMap; }

  bool& altDateReuseOutBoundProcessed() { return _altDateReuseOutBoundProcessed; }
  const bool& altDateReuseOutBoundProcessed() const { return _altDateReuseOutBoundProcessed; }

  bool& altDateReuseInBoundProcessed() { return _altDateReuseInBoundProcessed; }
  const bool& altDateReuseInBoundProcessed() const { return _altDateReuseInBoundProcessed; }

  std::vector<PaxType*>& excludedPaxType() { return _excludedPaxType; }
  const std::vector<PaxType*>& excludedPaxType() const { return _excludedPaxType; }

  ESVOptions*& esvOptions() { return _esvOptions; }
  const ESVOptions& esvOptions() const { return *_esvOptions; }

  VISOptions*& visOptions() { return _visOptions; }
  const VISOptions* visOptions() const { return _visOptions; }

  ShoppingPQVector& shoppingPQVector() { return _shoppingPQVector; }
  const ShoppingPQVector& shoppingPQVector() const { return _shoppingPQVector; }

  ShoppingPQ* getCxrShoppingPQ(CarrierCode* cxr);

  bool isSimpleTrip() { return _simpleTrip; }
  void setSimpleTrip(bool simpleTrip) { _simpleTrip = simpleTrip; }
  bool isRuleTuningISProcess() { return _ruleTuningISProcess; }
  void setRuleTuningISProcess(bool ruleTuningISProcess)
  {
    _ruleTuningISProcess = ruleTuningISProcess;
  }

  ForcedConnectionSet& forcedConnection() { return _forcedConnectionRexShopping; }
  const ForcedConnectionSet& forcedConnection() const { return _forcedConnectionRexShopping; }

  void addCat(uint16_t category) { _fareMarketRuleController.addCat(category); }

  bool isSumOfLocalsProcessingEnabled() const;
  bool isLngCnxProcessingEnabled() const;

  const Diversity& diversity() const { return *_diversity; }
  Diversity& diversity() { return *_diversity; }

  bool getSolNoLocalInd() const { return _solNoLocalInd; }
  void setSolNoLocalInd(bool value) { _solNoLocalInd = value; }

  ShpBitValidationCollector& getBitValidationCollector() { return _bitValidationCollector; }

  int16_t getNumOfCustomSolutions() const { return _numOfCustomSolutions; }
  void setNumOfCustomSolutions(int16_t cus) { _numOfCustomSolutions = cus; }

  void setCustomSolutionFM(const FareMarket* fm);
  bool isCustomSolutionFM(const FareMarket* fm) const;

  void setSpanishDiscountFM(const FareMarket* fm);
  bool isSpanishDiscountFM(const FareMarket* fm) const;

  bool isThroughFarePrecedencePossible() const { return _throughFarePrecedencePossible; }
  void setThroughFarePrecedencePossible(bool t) { _throughFarePrecedencePossible = t; }
  bool isThroughFarePrecedenceFoundInvalidFarePath() const
  {
    return _throughFarePrecedenceFoundInvalidFarePath;
  }
  void setThroughFarePrecedenceFoundInvalidFarePath(bool t)
  {
    _throughFarePrecedenceFoundInvalidFarePath = t;
  }

  bool isNoMoreLngConOptionsNeeded() const { return _noMoreLngConOptionsNeeded; }
  void setNoMoreLngConOptionsNeeded(bool value) { _noMoreLngConOptionsNeeded = value; }

  void setIbfData(IbfData* data);
  IbfData& getIbfData() const;
};

using LegVec = std::vector<ShoppingTrx::Leg>;
using SopVec = std::vector<ShoppingTrx::SchedulingOption>;

inline ShoppingTrx::ShoppingTrx(Diversity* diversity)
  : _fareMarketRuleController(ShoppingComponentWithFlightsValidation),
    _ASOfareMarketRuleController(ShoppingASOComponentWithFlightsValidation),
    _cat4RuleController(ShoppingComponentValidateQualifiedCat4),
    _travelDate(DateTime::emptyDate()),
    _returnDate(DateTime::emptyDate()),
    _diversity(diversity)
{
  if (!_diversity)
    _dataHandle.get(_diversity);
}

inline ShoppingTrx::SchedulingOption::SchedulingOption(Itin* itin,
                                                       const uint32_t& originalSopId,
                                                       bool cabinClassValid,
                                                       bool combineSameCxr)
  : _itin(itin),
    _cabinClassValid(cabinClassValid),
    _originalSopId(originalSopId),
    _combineSameCxr(combineSameCxr)
{
}
} // tse namespace
