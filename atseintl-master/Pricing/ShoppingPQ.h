/*---------------------------------------------------------------------------
 *  File:    ShoppingPQ.h
 *  Created: Jan 20, 2005
 *  Authors: David White
 *
 *  Copyright Sabre 2003
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Pricing/FareMarketMerger.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/GroupFarePathFactory.h"
#include "Pricing/VITAValidator.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <boost/functional/hash.hpp>

#include <vector>

#include <tr1/unordered_map>
#include <tr1/unordered_set>

namespace tse
{
class PricingOrchestrator;
class FarePath;
class PricingUnitFactoryBucket;
class MergedFareMarket;
class PUPathMatrix;
class PaxFarePathFactory;
class FarePathFactory;
class BitmapOpOrderer;
class FarePathFlightsInfo;
class MultiAirportAgent;

class ShoppingPQ : public TseCallableTrxTask
{
public:
  struct SOPWrapper
  {
    SOPWrapper(SopId sopId) : _sopId(sopId) {}
    SopId queueRank() const { return _sopId; }
    SopId _sopId;
  };

  using SopIdWrapperVec = std::vector<SOPWrapper>;
  using LegWrapperVec = std::vector<SopIdWrapperVec>;

  ShoppingPQ(PricingOrchestrator& po,
             ShoppingTrx& trx,
             Itin* journeyItin,
             uint32_t noOfOptionsRequested,
             int nEstimatedOptions,
             const CarrierCode* carrier,
             BitmapOpOrderer& bitmapOpOrderer,
             const bool searchAlwaysLowToHigh,
             bool owFaresShoppingQueue = false,
             int16_t fareCombRepeatLimit = -1,
             bool nonStopShoppingQueue = false);

  Itin* journeyItin() { return _journeyItin; }
  FareMarketRuleController& fareMarketRuleController() { return _fareMarketRuleController; }
  FareMarketRuleController& asoRuleController() { return _asoRuleController; }
  FareMarketRuleController& cat4RuleController() { return _cat4RuleController; }
  uint32_t maxFlightsForRuleValidation() const { return _maxFlightsForRuleValidation; }
  void setEstimatedSolutionsRequested(int estimatedSolRequested)
  {
    _nEstimatedOptions = estimatedSolRequested;
  }
  int getEstimatedSolutionsRequested() const { return _nEstimatedOptions; }
  bool isInterline() const { return _carrier == nullptr; }
  bool isNonStopShoppingQueue() const { return _nonStopShoppingQueue; }
  bool checkProcessDoneWithCond() { return _doneHurryWithCond; }
  void setProcessDoneWithCond() { _doneHurryWithCond = true; }
  void getMoreEstimatedSolutions() { _getMoreEstimatedSolutions = true; }
  void getMoreEstimatedFlightOnlySolutions() { _getMoreEstimatedFlightOnlySolutions = true; }
  size_t numBadEstimates() const { return _numBadEstimates; }
  void setMinimumFamilySize(size_t minimum) { _minimumFamilySize = minimum; }
  bool foundHighestFarePath() { return _foundHighestFarePath; }
  MoneyAmount altDateHighestAmountAllow() { return _altDateHighestAmountAllow; }
  uint32_t altDateMaxPassedBit() const { return _altDateMaxPassedBit; }
  uint32_t maxFailedCellsToValidate() const { return _maxFailedCellsToValidate; }
  bool createMoreSolution() { return _createMoreSolution; }
  void setCreateMoreSolution(bool status) { _createMoreSolution = status; }
  GroupFarePath* getLastSolution() { return _lastSolution; }
  GroupFarePathFactory* getGroupFarePathFactory() { return &_groupFarePathFactory; }
  const CarrierCode* carrier() const { return _carrier; }
  PricingTrx::AltDatePairs& altDatePairsPQ() { return _altDatePairsPQ; }
  PricingOrchestrator* ppo() { return _po; }
  MoneyAmount taxAmount() { return _taxAmount; }
  void searchBeyondStateSet(bool value) { _searchBeyondActivated = value; }
  void setMultiAirportAgent(const MultiAirportAgent* const t) { _MultiAirportAgent = t; };
  uint32_t getNoOfOptionsRequested() const { return _noOfOptionsRequested; }
  void setNoOfOptionsRequested(uint32_t noOfOptionsRequested)
  {
    _noOfOptionsRequested = noOfOptionsRequested;
  }
  const ShoppingTrx::FlightMatrix& getFlightMatrix() const { return _flightMatrix; }
  void setFlightMatrix(const ShoppingTrx::FlightMatrix& flightMatrix)
  {
    _flightMatrix = flightMatrix;
  }
  ShoppingTrx::FlightMatrix& flightMatrix() { return _flightMatrix; }
  const ShoppingTrx::EstimateMatrix& getEstimateMatrix() const { return _estimateMatrix; }
  ShoppingTrx::EstimateMatrix& estimateMatrix() { return _estimateMatrix; }
  void resetProcessDoneWithCond() { _doneHurryWithCond = false; }

  const std::map<CarrierCode, size_t>& getOnlineV2Map() const { return _onlineCarrierOptionFound; }
  std::map<CarrierCode, size_t>& getOnlineV2Map() { return _onlineCarrierOptionFound; }
  void setOnlineCarrierListV2(const CarrierCode& cxr, size_t value)
  {
    _onlineCarrierOptionFound[cxr] = value;
  }

  // Main Processing methods
  void performTask() override;
  void getSolutions();
  bool getAdditionalSolutions(uint32_t noptions);
  void processSolution(GroupFarePath* path, bool extraFarePath = false);
  void groupMother();
  GroupFarePath* generateSolution(const MoneyAmount lastAmount = 0.0);

  // Generate FOS solutions
  void generateSolutionsWithNoFares(GroupFarePath* gfp = nullptr, bool directFltOnly = false);
  void generateCNXSolutionsWithNoFares(GroupFarePath* gfp,
                                       const bool needDirectFlt = false,
                                       const bool sameCxrReq = false);
  void generateEstimatedSolutions();
  void generateCustomSolutionsWithNoFares(GroupFarePath* gfp);

  // Helpers
  void propagateError();

  bool isValidMatrixCell(const std::vector<int>& pos);
  bool preValidateFarePath(FarePath& farePath);
  bool isFarePathValid(FarePath& farePath, const std::vector<int>& sopIndices);
  void farePathValidationResult(FarePath& farePath,
                                const char* result,
                                const char* resultFVO = "",
                                int fareIndex = 0);

  void incrementFarePathTried() { ++_farePathTried; }

  Itin* getJourneyItinAltDates(const DatePair& d) const;

  bool checkHurryCondOld();
  bool checkHurryCond();

  void removeBadEstimates();
  void removeHighAmountAltDates();

  bool foundNonStopOption();
  bool foundFarePathOption();

  CarrierCode* sameCxrFarePath(FarePath& farePath);

  bool foundOnlineOption();
  void makeOptionHigherPriority();
  void getSortedSolutionsGFP(std::vector<GroupFarePath*>& results);
  bool needMoreSBSolutions() { return lookForMoreSolutionsBeyond(); }

  bool hasLngCnxSop(const std::vector<int>& sops) const;
  bool checkNumOfLngCnx();

  bool needMoreCustomSolutions();
  void customSolutionStateSet(bool value) { _customSolutionSearchActivated = value; }

  bool processCustomSolutionStats(const std::vector<int>& sops, bool hasLngCnxAdded = false);
  bool sopsHighTPMSimilarityCheck(const ShoppingTrx::SchedulingOption& sop1,
                                  const ShoppingTrx::SchedulingOption& sop2) const;

private:
  using FPCounterKey = std::vector<std::string>;
  using FarePathRepeatCounter = std::map<FPCounterKey, int16_t>;
  using JourneyItinAltDatesMap = std::map<DatePair, Itin>;
  using SopFarePathPair = std::pair<std::vector<int>, FarePath*>;

  struct AltDateOptionInfo
  {
    AltDateOptionInfo() : cxrCode(""), lowestAmt(0) {}
    CarrierCode cxrCode;
    MoneyAmount lowestAmt;
  };

  // Accessors
  bool isOwFaresShoppingQueue() const { return _owFaresShoppingQueue; }
  bool checkHurryCondForSearchBeyond() const
  {
    return (_searchBeyondActivated && (((_farePathTried > _farePathForRuleValWithFltMaxForSB) &&
                                        (_fltCombTried > _fltCombMaxForSB)) ||
                                       (_farePathTried > _farePathForRuleValMaxForSB)));
  }

  bool checkHurryCondForCustomSearch() const
  {
    return (_customSolutionSearchActivated &&
            ((_farePathTried > _farePathForRuleValWithFltMaxForCustom &&
              _fltCombTried > _fltCombMaxForCustom) ||
             (_farePathTried > _farePathForRuleValMaxForCustom)));
  }

  bool lookForMoreSolutions(size_t wanted) const { return (getFlightMatrix().size() < wanted); }
  bool lookForMoreSolutionsBeyond() const
  {
    return ((_searchBeyondActivated && _carrier &&
             (/*_taxedCPoints.size()*/ _cPoints.size() < _noOfTaxCallsForDiffCnxPoints))
                ? true
                : false);
  }
  bool lookForMoreCustomSolutions()
  {
    return _customSolutionSearchActivated && needMoreCustomSolutions();
  }

  bool needsMoreSolutions(size_t wanted);

  ShoppingTrx& shoppingTrx() { return *_trx; }
  const ShoppingTrx& shoppingTrx() const { return *_trx; }

  // Processing methods
  void generateQueue();
  void processFarePathFlightsInfo(GroupFarePath* groupFPath,
                                  std::vector<FarePathFlightsInfo*>& info,
                                  bool extraFarepath = false);

  void generateEstimatedFlightOnlySolutions();
  bool generateSnowManSolutionsWithNoFares(GroupFarePath* gfp = nullptr);
  void createEstimatedFlightOnlySolution(const std::vector<int>& basedSolution,
                                         std::vector<std::vector<int>>& sopVecforAllLegs,
                                         int noEstimatedSolution);
  void generateEstimateMatrixItem(const std::vector<int>& flightMatrixPos,
                                  GroupFarePath* path,
                                  const std::vector<int>& sops);
  void generateCombinedEstimateMatrix(const std::vector<int>& flightMatrixPos,
                                      GroupFarePath* path,
                                      const std::vector<int>& sops);
  void findEstimatedSops(const std::vector<int>& flightMatrixPos, GroupFarePath* path);

  bool validateNonStopSops(const std::vector<int>& sops);
  bool validateInterlineTicketCarrierAgreement(const std::vector<int>& sops);
  bool checkValidatingCarrierForCat35(const PaxTypeFareRuleData& ptfRd, FarePath& farePath);
  MoneyAmount getTax(Itin& curItin, FarePath* farePath);

  bool canBeInFamily(const ShoppingTrx::FlightMatrix::value_type& solution) const;
  bool isIndustryFareUsed(const GroupFarePath& groupFarePath) const;
  void regroupIndustryFaresFamilies(ShoppingTrx::FlightMatrix& flightMatrix);

  // Helpers
  bool directFlightSolution(const std::vector<int>& sops);
  bool doneDatePairCxrWithFarePath(const CarrierCode& govCxr, std::vector<const DatePair*>& dates);
  bool doneOptionPerFarePath(const CarrierCode& govCxr, const DatePair* d);
  bool sopsInterline(const std::vector<int>& flightMatrixPos) const;
  bool isSopInTheList(int basedSolutionSopId, std::vector<int>& sopVecForEachLeg);
  bool isSopinTheMatrix(const std::vector<int>& sopVec);
  bool isSopinTheMatrix(const std::vector<int>& sopVec, CarrierCode* cxr);
  bool isFlightSnowman(const std::vector<int>& sops);
  bool checkIfSimilarOption(const std::vector<int>& sops);
  bool addToFlightMatrix(const ShoppingTrx::FlightMatrix::value_type& item,
                         const GroupFarePath* originalGfp = nullptr);
  bool isValidCxrRestriction(const std::vector<int>& sops) const;
  bool skipSpecialSolution(const std::vector<int>& sops);

  void checkNumberOfSolutions();

  CarrierCode* sopsOnline(const std::vector<int>& flightMatrixPos);
  FarePathRepeatCounter::key_type makeFarePathRepeatCounterKey(const GroupFarePath* gfp) const;

  std::string getSourceName(); // for GroupFarePath

  DiagCollector& diag();
  void printSopCombination(DiagCollector& diag, const std::vector<int>& sops);

  // this function should be used to duplicate GroupFarePaths within
  // the PQ, since it will record the 'base' fare path, so later
  // we can use it to access the validationResultCache
  GroupFarePath* duplicateGroupFarePath(GroupFarePath* path);
  FarePath* getBaseFarePath(FarePath* path);

  bool isSolutionProducedByInterlineQueue(const SopIdVec& sops);
  void insertCustomFamilyHead(const SopIdVec& sops);

private:
  DiagCollector* _diag;
  ErrorResponseException _error;
  ShoppingTrx* _trx;
  PricingOrchestrator* _po;
  Itin* _journeyItin;
  const CarrierCode* _carrier;
  RuleControllerWithChancelor<FareMarketRuleController> _fareMarketRuleController;
  RuleControllerWithChancelor<FareMarketRuleController> _asoRuleController;
  RuleControllerWithChancelor<FareMarketRuleController> _cat4RuleController;
  FarePathRepeatCounter _farePathRepeatCounter;
  ValidatingCarrierUpdater _validatingCarrier;
  VITAValidator _vitaValidator;
  std::vector<PricingUnitFactoryBucket*> _puFactoryBucketVect;
  std::vector<MergedFareMarket*> _mergedFareMarketVect;
  FareMarketPathMatrix _fmpMatrix;
  GroupFarePathFactory _groupFarePathFactory;
  FareMarketMerger _fmMerger;
  RuleControllerWithChancelor<PricingUnitRuleController> _flightIndependentRuleController;
  RuleControllerWithChancelor<PricingUnitRuleController> _ruleController;
  BitmapOpOrderer& _bitmapOpOrderer;

  const MultiAirportAgent* _MultiAirportAgent;

  ShoppingTrx::FlightMatrix _flightMatrix;
  ShoppingTrx::EstimateMatrix _estimateMatrix;

  bool _lngCnxAllowedInOnlineQ;
  bool _searchBeyondActivated;
  bool _customSolutionSearchActivated;
  bool _owFaresShoppingQueue;
  bool _nonStopShoppingQueue;
  bool _foundHighestFarePath;
  bool _doneHurryWithCond;
  // when this flag is set, we only add more solutions if
  // the solution isn't already an estimated solution. It is used
  // to prioritize number of solutions above quality of solutions.
  bool _getMoreEstimatedSolutions;
  // this flag will be set when all queues produce flight only solution
  bool _getMoreEstimatedFlightOnlySolutions;
  bool _createMoreSolution;
  bool _taxForISProcess;
  bool _combinedFamily;
  bool _cat12ForAltDates;
  bool _allowToRemoveCustomFamily;

  int _nEstimatedOptions;

  int16_t _fareCombRepeatLimit;

  uint32_t _optionsPerFarePath;
  uint32_t _fltCombMaxForSB;
  uint32_t _farePathForRuleValMaxForSB;
  uint32_t _farePathForRuleValWithFltMaxForSB;
  uint32_t _fltCombMaxForCustom;
  uint32_t _farePathForRuleValMaxForCustom;
  uint32_t _farePathForRuleValWithFltMaxForCustom;
  uint32_t _farePathForRuleValMax;
  uint32_t _farePathForRuleValWithFltMax;
  uint16_t _noOfTaxCallsForDiffCnxPoints;
  uint32_t _maxFlightsForRuleValidation;
  uint32_t _noOfOptionsRequested;
  uint32_t _farePathTried;
  uint32_t _fltCombTried;
  uint32_t _fltCombMax;
  uint32_t _multiLegFltCombMax;
  uint32_t _altDateDiversityDivider;
  uint32_t _interlineDiversityPercent;
  uint32_t _mustPriceInterlineSnowManPercent;
  uint32_t _altDateItinPriceJumpFactor;
  uint32_t _altDateMaxPassedBit;
  uint32_t _maxFailedCellsToValidate;
  uint32_t _maxFailedCellsForFltOnlySolution;
  uint32_t _altDateFareAmountWeightFactor;

  MoneyAmount _altDateHighestAmountAllow;
  MoneyAmount _altDateCutoffNucThreshold;
  MoneyAmount _taxAmount;

  size_t _numBadEstimates;
  size_t _minimumFamilySize;

  double _altDateMinimumWeightFactor;
  size_t _numFPAndMustPriceSnowManFltOnlyInterlineOption;

  std::set<int> _outboundSopSet;
  std::set<int> _inboundSopSet;
  std::set<std::vector<int>> _badEstimates;

  // keeps information if at least one of the members of family is custom
  // as we put there family head which has custom member
  std::set<SopIdVec> _customFamilyHeads;

  // we keep a cache of the results of calls to isFarePathValid()
  // Since we duplicate fare paths whenever a solution is found,
  // we must keep a mapping of what the 'base' fare path is, since
  // that is what is needed to look up the cache
  std::map<FarePath*, FarePath*> _baseFarePath;

  std::map<SopFarePathPair, bool> _validationResultCache;

  std::map<CarrierCode, size_t> _onlineCarrierOptionFound;

  PricingTrx::AltDatePairs _altDatePairsPQ;
  std::tr1::unordered_set<std::string, boost::hash<std::string>> _cPoints;

  mutable JourneyItinAltDatesMap _journeyItinAltDates;

  std::map<DatePair, AltDateOptionInfo> _cxrOptionPerDatesMap;

  PUPathMatrix* _puMatrix;
  std::vector<int> _flightMatrixDim;
  std::vector<PaxFarePathFactory*> _paxFarePathFactoryBucket;
  GroupFarePath* _lastSolution;

  std::vector<std::vector<uint32_t>> _sopCarriers;
  ItinIndex::Key _cxrCode;

  LegWrapperVec _legWrappedCustomSopsCollection;

  friend class ShoppingPQTest;
};
} // tse namespace
