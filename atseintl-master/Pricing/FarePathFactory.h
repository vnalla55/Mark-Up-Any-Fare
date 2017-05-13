//-------------------------------------------------------------------
// File:    FarePathFactory.h
// Created: July 2004
// Authors: Mohammad Hossan
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

#include "Common/Assert.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "DataModel/FarePath.h"
#include "Pricing/FactoriesConfig.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FarePathFactoryFailedPricingUnits.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/PricingUnitRequester.h"
#include "Pricing/PUPath.h"
#include "Rules/RuleControllerWithChancelor.h"

#include <boost/heap/priority_queue.hpp>

#include <queue>
#include <vector>

namespace tse
{
class BaggageCalculator;
class Combinations;
class DiagCollector;
class FarePathValidator;
class FareMarket;
class FarePathFactoryCreator;
class Itin;
class MixedClassController;
class PU;
class PUPQItem;
class PaxFPFBaseData;
class PaxFarePathFactory;
class PaxType;
class PricingTrx;
class PricingUnit;
class PricingUnitFactory;
class PricingUnitFactoryBucket;
class YQYRCalculator;
struct FarePathSettings;
struct SavedFPPQItem;

const RuleNumber Rule_T225 = "T225";
const RuleNumber Rule_T226 = "T226";
const RuleNumber Rule_T227 = "T227";
const TariffNumber Tariff_8 = 8;

class FarePathFactory
{
  enum RexClearingStage
  { RCS_NOT_PROCESSED,
    RCS_NEED_PROCESS,
    RCS_ALREADY_PROCESSED };

  enum BrandParityResult
  { PASSED,
    FAILED_COMMON_BRANDS,
    FAILED_STATUS_CHECK };

public:
  using FarePathPQ =
      boost::heap::priority_queue<FPPQItem*,
                                  boost::heap::compare<FPPQItem::Greater<FPPQItem::GreaterFare>>>;
  using LowToHighFarePathPQ = boost::heap::priority_queue<
      FPPQItem*,
      boost::heap::compare<FPPQItem::GreaterLowToHigh<FPPQItem::GreaterFare>>>;
  using FarePathASEPQ =
      boost::heap::priority_queue<FPPQItem*,
                                  boost::heap::compare<FPPQItem::Greater<FPPQItem::LowerFare>>>;

  struct GETCXRPUThrArg : public TseCallableTrxTask
  {
    GETCXRPUThrArg() { desc("GET PU TASK"); }

    void performTask() override;
    bool getAlreadyBuiltPUPQItem();

    uint32_t idx = 0;
    bool isValid = true;
    PricingUnitFactory* puf = nullptr;
    bool getNextCxr = false;
    PUPQItem* prevPUPQItem = nullptr;
    PUPQItem* pupqItem = nullptr;
    DiagCollector* diag = nullptr;
    ErrorResponseException::ErrorResponseCode errResponseCode =
        ErrorResponseException::ErrorResponseCode::NO_ERROR;
    std::string errResponseMsg;
    bool done = false;
    bool isXPoint = false;
    CarrierCode valCxr;

  private:
    bool isSameFarePathValidForRebook(FarePath& fp);
    bool checkEOECombinability(FPPQItem& fppqItem, DiagCollector& diag);
  };

  template <typename T>
  class Greater
  {
  public:
    bool operator()(FarePathFactory* lfact, FarePathFactory* rfact) const
    {
      const FPPQItem* lhs = lfact->lowerBoundFPPQItem();
      const FPPQItem* rhs = rfact->lowerBoundFPPQItem();

      const uint16_t lhsTfpRank = lfact->puPath()->fareMarketPath()->throughFarePrecedenceRank();
      const uint16_t rhsTfpRank = rfact->puPath()->fareMarketPath()->throughFarePrecedenceRank();

      if (UNLIKELY(lhsTfpRank != rhsTfpRank))
        return lhsTfpRank > rhsTfpRank;

      FPPQItem::GreaterLowToHigh<T> greater;
      return greater(lhs, rhs);
    }
  };

  explicit FarePathFactory(const FactoriesConfig& factoriesConfig)
    : _factoriesConfig(factoriesConfig)
  {
  }

  FarePathFactory(const FarePathFactory&) = delete;
  FarePathFactory& operator=(const FarePathFactory&) = delete;

  virtual ~FarePathFactory() = default;

  static FarePathFactory*
  createFarePathFactory(const FarePathFactoryCreator& creator,
                        PUPath* puPath,
                        PricingUnitFactoryBucket* const puFactoryBucket,
                        Itin* itin,
                        PaxFPFBaseData* paxFPFBaseData,
                        std::vector<SavedFPPQItem>* savedFPPQItems = nullptr);

  int keepValidItems();

  bool initFarePathFactory(DiagCollector& diag);
  void getAllPricingUnitFactory();

  void setSavedFPPQItems(std::vector<SavedFPPQItem>* savedFPPQItems)
  {
    _savedFPPQItems = savedFPPQItems;
  }

  FPPQItem* getNextFPPQItem(DiagCollector& diag);
  FPPQItem* getNextFPPQItemFromQueue();

  void pushBack(FPPQItem* fppqItem) { pqPush(fppqItem); }

  FPPQItem* getSameFareBasisFPPQItem(const FPPQItem& primaryFPPQItem,
                                     DiagCollector& diag); // for INF paxType only

  const PricingTrx* trx() const { return _trx; }
  PricingTrx*& trx() { return _trx; }

  const Itin* itin() const { return _itin; }
  Itin*& itin() { return _itin; }

  std::vector<PricingUnitFactory*>& allPUF() { return _allPUF; }

  const Combinations* combinations() const { return _combinations; }
  Combinations*& combinations() { return _combinations; }

  const PUPath* puPath() const { return _puPath; }
  PUPath*& puPath() { return _puPath; }

  const PricingUnitFactoryBucket* puFactoryBucket() const { return _puFactoryBucket; }
  PricingUnitFactoryBucket*& puFactoryBucket() { return _puFactoryBucket; }

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  const int32_t& multiPaxShortCktTimeOut() const { return _multiPaxShortCktTimeOut; }
  int32_t& multiPaxShortCktTimeOut() { return _multiPaxShortCktTimeOut; }

  MoneyAmount lowerBoundFPAmount() const { return _lowerBoundFPAmount; }

  const FPPQItem* lowerBoundFPPQItem() { return pqTop(); }

  MoneyAmount& externalLowerBoundAmount() { return _externalLowerBoundAmount; }
  const MoneyAmount& externalLowerBoundAmount() const { return _externalLowerBoundAmount; }

  bool reachedPushBackLimit();
  void incrementPushBackCount();

  uint32_t fpCombTried() { return _fpCombTried; }

  bool& shutdown() { return _shutdown; }
  const bool& shutdown() const { return _shutdown; }

  void saveISFailedFare(const uint16_t pufIdx, const PaxTypeFare* paxTypeFare)
  {
    _allPUF[pufIdx]->saveISFailedFare(paxTypeFare);
  }

  void
  saveISFailedFare(const uint16_t pufIdx, const PaxTypeFare* paxTypeFare, const DatePair& datePair)
  {
    _allPUF[pufIdx]->saveISFailedFare(paxTypeFare, datePair);
  }

  const bool pricingAxess() const { return _pricingAxess; }
  bool& pricingAxess() { return _pricingAxess; }

  // This clear is needed for 2nd call for NetRemitPricing
  void clear();
  void releaseMemory();
  void clearAndReleaseMemory();

  const bool multiPaxShortCktStarted() const { return _multiPaxShortCktStarted; }
  bool& multiPaxShortCktStarted() { return _multiPaxShortCktStarted; }

  void clearFactoryForRex(DiagCollector& diag);

  bool isEoeCombinabilityEnabled() const { return _eoeCombinabilityEnabled; }
  void setEoeCombinabilityEnabled(bool eoeCombinabilityEnabled)
  {
    _eoeCombinabilityEnabled = eoeCombinabilityEnabled;
  }

  void releaseFPPQItem(FPPQItem* fppqItem);
  void saveEOEFailedFare(const PaxTypeFare* paxTypeFare1, const PaxTypeFare* paxTypeFare2);

  PUPQItem::PUValidationStatus
  getPUScopeCat10Status(const FPPQItem& fppqItem, const uint16_t puFactIdx);

protected:
  bool checkEOECombinability(FPPQItem& fppqItem,
                             FareUsage*& failedSourceFareUsage,
                             FareUsage*& failedTargetFareUsage,
                             DiagCollector& diag);

  void buildNextFarePathSet(FPPQItem& fppqItem, DiagCollector& diagnostic);
  void requestForNewPricingUnits(uint16_t xPoint,
                                 const std::vector<uint32_t>& puIndices,
                                 const MoneyAmount puDelta,
                                 unsigned expandStep,
                                 DiagCollector& diagnostic);

  bool buildInitialFarePath(DiagCollector& diagnostic);

  FPPQItem* buildPausedFarePath(FPPQItem& fppqItem, DiagCollector& diag);

  bool buildFarePath(bool initStage,
                     const std::vector<uint32_t>& puIndices,
                     const unsigned xPoint,
                     DiagCollector& diagnostic);

  void processSameFareDate(FPPQItem& fppqItem);

  PUPQItem* findEOEValidPUPair(FarePath* tmpFPath,
                               const int32_t puFactIdx1,
                               const int32_t puFactIdx2,
                               uint32_t& idx,
                               PricingUnitFactory* puf,
                               const FareUsage* const failedEOETargetFareUsage,
                               FPPQItem::EOEValidationStatus& eoeValStatus,
                               DiagCollector& diagIn);

  void replaceWithNewPU(FPPQItem& fppqItem,
                        PUPQItem* pupqItem,
                        const int32_t puFactIdx,
                        const uint32_t idx);

  void recalculatePriority(FPPQItem& fppqItem);

  bool checkEarlyEOECombinability(const bool initStage,
                                  FPPQItem& fppqItem,
                                  const uint16_t xPoint,
                                  DiagCollector& diagIn);

  bool checkCxrFPPQItemExists(const std::string& fpFareType,
                              const FPPQItem& fppqItem,
                              std::vector<CarrierCode>& valCxr,
                              DiagCollector& diag);
  bool checkCxrFPPQItemExists(const std::string& fpFareType,
                              const FPPQItem& fppqItem,
                              const CarrierCode& valCxr,
                              const std::deque<bool>& cxrFareRest,
                              std::vector<CarrierCode>& passingValCxr,
                              DiagCollector& diag);
  void
  recordCxrFPPQItemExistance(const std::string& fpFareType, std::vector<CarrierCode>& cxrFPvalCxr);

  bool passCxrFarePreference(FPPQItem& fppqItem, DiagCollector& diag);

  bool checkCxrFPPQItemFoundBefore(FPPQItem& fppqItem, std::string& fpFareType);

  bool bypassCxrFareCombo(FPPQItem& fppqItem, DiagCollector& diag);
  bool checkCxrFareComboCount(FPPQItem& fppqItem);
  bool openUpYYFareCombo();

  void incrementCxrFareTypeCount(FPPQItem& fppqItem);
  void decrementCxrFareTypeCount(FPPQItem& fppqItem);
  void getCxrFareType(const FPPQItem& fppqItem, std::string& fpFareType);

  bool getCxrFareRestriction(const FPPQItem& fppqItem,
                             std::map<CarrierCode, std::deque<bool>>& cxrFareRest)
  {
    std::vector<CarrierCode> tmp;
    return getCxrFareRestriction(fppqItem, cxrFareRest, tmp);
  }
  bool getCxrFareRestriction(const FPPQItem& fppqItem,
                             std::map<CarrierCode, std::deque<bool>>& cxrFareRest,
                             std::vector<CarrierCode>& alreadyPassingValCxrList);
  bool getCxrFareRestriction(const FPPQItem& fppqItem,
                             const CarrierCode& valCxr,
                             std::deque<bool>& cxrFareRest,
                             std::vector<CarrierCode>& alreadyPassingValCxrList);

  bool getFPValCxrFromPTF(const FPPQItem& fppqItem, std::vector<CarrierCode>& fpValCxr);

  using CxrFareRestToItnIdxMap_t = std::map<std::deque<bool>, std::set<uint16_t>>;

  void buildNextCxrFarePathSet(const bool initStage,
                               const FPPQItem& prevFPPQItem,
                               const CarrierCode& valCxr,
                               const std::deque<bool>& cxrFareRest,
                               DiagCollector& diag);

  bool buildCxrFarePath(const bool initStage,
                        const FPPQItem& prevFPPQItem,
                        const std::vector<uint32_t>& puIndices,
                        const CarrierCode& valCxr,
                        const std::deque<bool>& cxrFareRest,
                        const uint16_t xPoint,
                        DiagCollector& diag);

  void resetPriority(const PUPQItem& pupqItem, FPPQItem& fppqItem);

  inline MoneyAmount getPUDelta(const FPPQItem& prevFPpqItem);

  virtual bool
  isFarePathValid(FPPQItem& fppqItem, DiagCollector& diag, std::list<FPPQItem*>& clonedFPPQItems);

  bool isFarePathValidForFlexFareGroup(FPPQItem& fppqItem, FarePathValidator& farePathValidator);
  bool isFarePathValidForCorpIDFare(FPPQItem& fppqItem, flexFares::GroupId groupID);
  bool isFarePathValidForXOFare(FPPQItem& fppqItem, const flexFares::GroupId groupID);

  void saveFPPQItem(const FPPQItem& fppqItem, const uint16_t category);

  bool checkEOEFailedFare(FPPQItem& fppqItem)
  {
    FareUsage* failedSourceFareUsage = nullptr;
    FareUsage* failedEOETargetFareUsage = nullptr;
    return checkEOEFailedFare(
        *fppqItem.farePath(), failedSourceFareUsage, failedEOETargetFareUsage);
  }

  bool checkEOEFailedFare(FarePath& fpath,
                          FareUsage*& failedSourceFareUsage,
                          FareUsage*& failedEOETargetFareUsage);

  void resetLowerBound();

  inline void setLowerBound()
  {
    if (LIKELY(_factoriesConfig.searchAlwaysLowToHigh()))
    {
      if (!_lthFarePathPQ.empty())
      {
        FarePath* fp = _lthFarePathPQ.top()->farePath();
        _lowerBoundFPAmount = fp->getNUCAmountScore();
      }
      else
      {
        _lowerBoundFPAmount = -1;
      }
    }
    else
    {
      if (!_farePathPQ.empty())
      {
        FarePath* fp = _farePathPQ.top()->farePath();
        _lowerBoundFPAmount = fp->getNUCAmountScore();
      }
      else
      {
        _lowerBoundFPAmount = -1;
      }
    }
  }

  inline void pqPop()
  {
    if (LIKELY(_factoriesConfig.searchAlwaysLowToHigh()))
    {
      if (LIKELY(!_lthFarePathPQ.empty()))
      {
        _lthFarePathPQ.pop();
      }
    }
    else
    {
      if (!_farePathPQ.empty())
      {
        _farePathPQ.pop();
      }
    }
    setLowerBound();
  }

  inline FPPQItem* pqTop()
  {
    if (LIKELY(_factoriesConfig.searchAlwaysLowToHigh()))
    {
      if (!_lthFarePathPQ.empty())
      {
        return _lthFarePathPQ.top();
      }
    }
    else
    {
      if (!_farePathPQ.empty())
      {
        return _farePathPQ.top();
      }
    }
    return nullptr;
  }

  inline void pqPush(FPPQItem* fppqItem)
  {
    // also set lower bound
    if (LIKELY(_factoriesConfig.searchAlwaysLowToHigh()))
    {
      _lthFarePathPQ.push(fppqItem);
      FarePath* fp = _lthFarePathPQ.top()->farePath();
      _lowerBoundFPAmount = fp->getNUCAmountScore();
    }
    else
    {
      _farePathPQ.push(fppqItem);
      FarePath* fp = _farePathPQ.top()->farePath();
      _lowerBoundFPAmount = fp->getNUCAmountScore();
    }
  }

  inline size_t pqSize()
  {
    if (_factoriesConfig.searchAlwaysLowToHigh())
      return _lthFarePathPQ.size();

    return _farePathPQ.size();
  }

  inline bool pqEmpty()
  {
    if (LIKELY(_factoriesConfig.searchAlwaysLowToHigh()))
      return _lthFarePathPQ.empty();

    return _farePathPQ.empty();
  }

  // these are for FP level PU validation status saving and checking

  bool checkCarrierPreference(const FarePath& fpath, DiagCollector& diag);

  void startTimer() { _startTime = time(nullptr); }
  bool startMultiPaxShortCkt();

  FarePath* constructFarePath() { return &_storage.constructFarePath(); }

  FPPQItem* constructFPPQItem()
  {
    FPPQItem& item = _storage.constructFPPQItem();
    item.farePathFactory() = this;
    return &item;
  }

  bool hasBrandParity(const FarePath& farePath, DiagCollector& diag) const;
  BrandParityResult hasBrandParity_impl(const FarePath& farePath) const;
  bool hasHardPassBrandOnEachLeg(const FarePath& farePath,
                                 const std::set<BrandCode>& parityBrands) const;

  void updateFarePathAmounts(FarePath& fpath,
                             const PricingUnit& prU,
                             FPPQItem* fppqItem = nullptr,
                             int16_t xPoint = -1);

  void updateFarePath(FarePath& fpath, const FarePath& inFpath);

  void processFarePathClones(FPPQItem*& fppqItem, std::list<FPPQItem*>& clonedFPPQItems);

  FPPQItem* cloneFPPQItem(FPPQItem& fppqItem, std::vector<CarrierCode>& valCxrVect);
  void removeValidatingCarriers(FPPQItem& fppqItem, std::vector<CarrierCode>& valCxrsToRemove);

  template <class RevalidatorT>
  bool checkFBR(const FPPQItem& fppqItem,
                const uint16_t numSeatsRequired,
                const RevalidatorT& revalidator = RevalidatorT()) const
  {
    for (const PricingUnit* pu : fppqItem.farePath()->pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        const PaxTypeFare& ptf = *fu->paxTypeFare();
        if (!ptf.isFareByRule())
          continue;
        const FareMarket& fm = *ptf.fareMarket();
        if (fm.classOfServiceVec().empty())
        {
          std::vector<std::vector<ClassOfService*>*> cosVec;
          for (TravelSeg* const segment : fm.travelSeg())
            cosVec.push_back(&segment->classOfService());

          if (!revalidator.checkFBR(&ptf, numSeatsRequired, cosVec, fm.travelSeg()))
            return false;
        }
        else if (!revalidator.checkFBR(
                     &ptf, numSeatsRequired, fm.classOfServiceVec(), fm.travelSeg()))
          return false;
      }
    }
    return true;
  }
  bool checkFBR(const FPPQItem&) const;

  const FactoriesConfig& _factoriesConfig;
  FarePathFactoryStorage _storage;

  PricingTrx* _trx = nullptr;
  Combinations* _combinations = nullptr;

  Itin* _itin = nullptr;

  PUPath* _puPath = nullptr;
  PricingUnitFactoryBucket* _puFactoryBucket = nullptr;

  std::vector<SavedFPPQItem>* _savedFPPQItems = nullptr;
  PaxFPFBaseData* _paxFPFBaseData = nullptr;
  std::vector<PricingUnitFactory*> _allPUF; // collection of PU-factory of main trip and side Trip
  // PU
  PaxType* _paxType = nullptr;

  bool _shutdown = false; // Short-Ckt logic of PaxFarePathFactory can shutdown to
  // try other FarePathFactory and to stop looping for ever

  uint32_t _fpCount = 0;
  uint32_t _fpCombTried = 0;

  FarePathPQ _farePathPQ;
  MoneyAmount _lowerBoundFPAmount = -1;
  MoneyAmount _externalLowerBoundAmount = -1;

  LowToHighFarePathPQ _lthFarePathPQ;

  FarePathPQ _cxrFarePathPQ;
  std::map<std::string, uint16_t> _fpFareTypeCountMap;
  std::map<std::string, std::map<CarrierCode, uint16_t>> _fpFareTypeCountByValCxrMap;
  std::multimap<std::string, FPPQItem*> _processedYYFarePaths;

  std::map<std::string, std::vector<CarrierCode>> _valCxrCxrFarePathExistanceMap;
  bool _openUpYYFarePhase = false;

  //  idx to _allPUF     set of failed PrU idx
  FarePathFactoryFailedPricingUnits _failedPricingUnits;
  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*>> _eoeFailedFare;

  bool _groupFarePathRequired = true;

  bool _maxNbrCombMsgSet = false;

  bool _pricingAxess = false;

  // followings are to tune FP PushBack for PlusUp issue of INF
  int32_t _plusUpPushBackCount = 0;

  int32_t _multiPaxShortCktTimeOut = 30;
  time_t _startTime = 0;
  bool _multiPaxShortCktStarted = false;

  RexClearingStage _clearingStageForRex = RexClearingStage::RCS_NOT_PROCESSED;

  std::set<std::string> _searchedPrimaryFareBasis; // for INF PaxType Tuning

  uint16_t _eoeValidPUPairSearchLoopMax = 5;

  bool _eoeCombinabilityEnabled = true;
  bool _fpInitStage = true;
  bool _isRexTrxAndNewItin = false;
  bool _ignoreAlternateVCs = false;
  PricingUnitRequester _pricingUnitRequester;
  YQYRCalculator* _yqyrPreCalc = nullptr;
  BaggageCalculator* _bagCalculator = nullptr;
  RuleControllerWithChancelor<PricingUnitRuleController>* _ruleController = nullptr;
  FarePathSettings* _farePathSettings = nullptr;

private:
  friend class FarePathFactoryTest;
};

class FarePathFactoryCreator
{
public:
  virtual ~FarePathFactoryCreator() = default;
  virtual FarePathFactory*
  create(const FactoriesConfig& factoriesConfig, const PricingTrx& trx) const;
};
} // tse namespace
