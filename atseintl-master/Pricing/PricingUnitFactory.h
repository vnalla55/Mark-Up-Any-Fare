//-------------------------------------------------------------------
// File:    PricingUnitFactory.h
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

#include "Common/ArrayVector.h"
#include "Common/TseObjectPool.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/PricingUnitRequesterData.h"
#include "Pricing/PUPQItem.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

#include <boost/pool/object_pool.hpp>
#include <boost/heap/priority_queue.hpp>

#include <deque>
#include <queue>
#include <stack>
#include <vector>

namespace tse
{
class PaxType;
class PricingTrx;
class DiagCollector;
class Combinations;
class FarePUResultContainer;

typedef TseObjectPool<FareUsage> FareUsagePool;
typedef TseObjectPool<PUPQItem> PUPQItemPool;
typedef TseObjectPool<PricingUnit> PricingUnitPool;


// brief RoundTripCheck is a uility class.
// It has new logic to check whether a trip is RT or CT.
// It had methods to checks Cat2 (dowType), Cat3 (seasonalType) Record 1, Record 2 and
// Record 3 (itemNo, relationalInd).
//
// Please read FRD Highest RT Check.doc for more details.
struct RoundTripCheck
{
  enum CompareResult : uint8_t
  { BLANK_IND,
    SAME_IND,
    DIFFERENT_IND,
    NOT_NULL };

  template <typename T>
  static CompareResult checkNullPtr(const T* a, const T* b)
  {
    if (!a && !b)
      return SAME_IND;
    else if ((a && !b) || (!a && b))
      return DIFFERENT_IND;
    return NOT_NULL;
  }

  static CompareResult compareIndicators(Indicator xType, Indicator yType)
  {
    if (xType == ' ' && yType == ' ')
      return BLANK_IND;
    else if (xType == yType)
      return SAME_IND;

    return DIFFERENT_IND;
  }

  // It check Rec1 for Cat2 and Cat3. It compares dowType and seasonTypes
  static CompareResult checkRec1Indicator(const PaxTypeFare& paxTypeFare1,
                                          const PaxTypeFare& paxTypeFare2,
                                          uint16_t catNum);

  // It check Rec2 for Cat2 and Cat3. It compares dowType and seasonTypes
  static CompareResult
  checkRec2Indicator(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2, uint16_t catNum);

  // It check Rec3 itemNo and relationalInd.
  static CompareResult
  checkRec3ItemNo(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2, uint16_t catNum);

  static CompareResult checkRec2Indicator(const GeneralFareRuleInfo* gfrInfo1,
                                          const GeneralFareRuleInfo* gfrInfo2,
                                          uint16_t catNum);

  static CompareResult checkItemInfo(const std::vector<CategoryRuleItemInfoSet*>& criiSetVec1,
                                     const std::vector<CategoryRuleItemInfoSet*>& criiSetVec2);

private:
  RoundTripCheck();
  RoundTripCheck(const RoundTripCheck&);
  void operator=(const RoundTripCheck&);
};

class PricingUnitFactory
{
  friend class PricingUnitFactoryTest;
  friend class PricingUnitFactory_isFareValidTest;
  friend class PricingUnitFactory_isPricingUnitValidTest;
  friend class PricingUnitFactory_buildFareUsageTest;

public:
  typedef boost::heap::priority_queue<PUPQItem*,
    boost::heap::compare<PUPQItem::Greater<PUPQItem::GreaterFare> > > PUPQ;
  typedef boost::heap::priority_queue<PUPQItem*,
    boost::heap::compare<PUPQItem::GreaterLowToHigh<PUPQItem::GreaterFare> > > LowToHighPUPQ;

  PricingUnitFactory()
  {
    _stopTime = time(nullptr);
  }

  PricingUnitFactory(const PricingUnitFactory& puf) = delete;
  PricingUnitFactory& operator=(const PricingUnitFactory& rhs) = delete;

  virtual ~PricingUnitFactory() = default;

  bool initPricingUnitPQ(DiagCollector& diag);

  PUPQItem* getPUPQItem(uint32_t idx, DiagCollector& diag);
  unsigned getPUPQItemCounter() const { return _getPUPQItemCounter; }

  PUPQItem* getAlreadyBuiltPUPQItem(uint32_t idx)
  {
    if (idx < _puCount)
    {
      return _validPUPQItem[idx];
    }
    return nullptr;
  }

  uint32_t sizeAlreadyBuiltPUPQItem() const { return _puCount; }

  PUPQItem*
  getSameFareBasisPUPQItem(const PricingUnit& primaryPU, uint16_t& puIdx, DiagCollector& diag);

  PUPQItem* getNextCxrFarePUPQItem(PUPQItem& pupqItem, bool isXPoint,
                                   const CarrierCode& valCxr,
                                   DiagCollector& diag);

  PUPQItem* getNextCxrFarePUPQItemImpl(PUPQItem& prevPUPQItem,
                                       bool isXPoint,
                                       const std::string& puCxrFareType,
                                       const CarrierCode& valCxr,
                                       const std::deque<bool>& cxrFareRest,
                                       DiagCollector& diag);

  // If we need to fall back
  PUPQItem* getNextCxrFarePUPQItemOld(PUPQItem& pupqItem, DiagCollector& diag);

  const PricingTrx* trx() const { return _trx; }
  PricingTrx*& trx() { return _trx; }

  const Itin* itin() const { return _itin; }
  Itin*& itin() { return _itin; }

  DataHandle& dataHandle() { return _dataHandle; }

  const PU* pu() const { return _pu; }
  PU*& pu() { return _pu; }

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  const bool& done() const { return _done; }
  bool& done() { return _done; }

  const bool& useCxrPreference() const { return _useCxrPreference; }
  bool& useCxrPreference() { return _useCxrPreference; }

  const PUPQ& puPQ() const { return _puPQ; }
  PUPQ& puPQ() { return _puPQ; }

  void startTiming() { _stopTime = time(nullptr); }
  time_t stopTime() { return _stopTime; }

  const double& shortCktTimeOut() const { return _shortCktTimeOut; }
  double& shortCktTimeOut() { return _shortCktTimeOut; }

  const uint16_t& fcCount() const { return _fcCount; }
  uint16_t& fcCount() { return _fcCount; }

  const Combinations* combinations() const { return _combinations; }
  Combinations*& combinations() { return _combinations; }

  const bool& isISPhase() const { return _isISPhase; }
  bool& isISPhase() { return _isISPhase; }

  const bool& puScopeValidationEnabled() const { return _puScopeValidationEnabled; }
  bool& puScopeValidationEnabled() { return _puScopeValidationEnabled; }

  const bool& delayPUValidationMIP() const { return _delayPUValidationMIP; }
  bool& delayPUValidationMIP() { return _delayPUValidationMIP; }

  const bool& initStage() const { return _initStage; }
  bool& initStage() { return _initStage; }

  const bool& initializedForDelayXpn() const { return _initializedForDelayXpn; }
  bool& initializedForDelayXpn() { return _initializedForDelayXpn; }

  const uint16_t& puScopeValMaxNum() const { return _puScopeValMaxNum; }
  uint16_t& puScopeValMaxNum() { return _puScopeValMaxNum; }

  void setPUScopeCat10Status(const uint32_t puIdx, PUPQItem::PUValidationStatus status);
  PUPQItem::PUValidationStatus getPUScopeCat10Status(const uint32_t puIdx);

  void setPUScopeRuleReValStatus(const uint32_t puIdx, PUPQItem::PUValidationStatus status);
  PUPQItem::PUValidationStatus getPUScopeRuleReValStatus(const uint32_t puIdx, PricingUnit& pu);

  const std::set<const PaxTypeFare*>& failedFareSet() const { return _failedFareSet; }
  std::set<const PaxTypeFare*>& failedFareSet() { return _failedFareSet; }

  const uint16_t& maxSPCTFareCompCount() const { return _maxSPCTFareCompCount; }
  uint16_t& maxSPCTFareCompCount() { return _maxSPCTFareCompCount; }

  const uint32_t& maxSPCTPUCount() const { return _maxSPCTPUCount; }
  uint32_t& maxSPCTPUCount() { return _maxSPCTPUCount; }

  const MoneyAmount& delta() const { return _delta; }
  MoneyAmount& delta() { return _delta; }

  const bool& searchAlwaysLowToHigh() const { return _searchAlwaysLowToHigh; }
  bool& searchAlwaysLowToHigh() { return _searchAlwaysLowToHigh; }

  const bool& enableCxrFareSearchTuning() const { return _enableCxrFareSearchTuning; }
  bool& enableCxrFareSearchTuning() { return _enableCxrFareSearchTuning; }

  const uint32_t& maxNbrCombMsgThreshold() const { return _maxNbrCombMsgThreshold; }
  uint32_t& maxNbrCombMsgThreshold() { return _maxNbrCombMsgThreshold; }

  const bool& taxPerFareBreak() const { return _taxPerFareBreak; }
  bool& taxPerFareBreak() { return _taxPerFareBreak; }

  bool isOnlyOwFares() const { return _onlyOwFares; }
  void setOnlyOwFares(bool onlyOwFares) { _onlyOwFares = onlyOwFares; }

  bool isOnlyNonStops() const { return _onlyNonStops; }
  void setOnlyNonStops(bool v) { _onlyNonStops = v; }

  void clearFactoryForRex(DiagCollector& diag);

  class JourneyPULowerBound
  {
  public:
    JourneyPULowerBound() : _journeyPULowerBound(-1) {}

    MoneyAmount journeyPULowerBound()
    {
      const boost::lock_guard<boost::mutex> g(_mutex);
      return _journeyPULowerBound;
    }

    void setJourneyPULowerBound(MoneyAmount lbAmt);

  private:
    MoneyAmount _journeyPULowerBound;
    boost::mutex _mutex;
  };

  const JourneyPULowerBound* journeyPULowerBound() const { return _journeyPULowerBound; }
  JourneyPULowerBound*& journeyPULowerBound() { return _journeyPULowerBound; }

  void saveCat10FailedFare(const FareUsage* fareUsage1, const FareUsage* fareUsage2);

  void saveISFailedFare(const PaxTypeFare* paxTypeFare) { _failedFareSet.insert(paxTypeFare); }

  void saveISFailedFare(const PaxTypeFare* paxTypeFare, const DatePair& datePair);

  // This clear is needed for 2nd call for NetRemitPricing
  void clear();

  PricingUnitRequesterData& requesterData() { return _requesterData; }
  const PricingUnitRequesterData& requesterData() const { return _requesterData; }

protected:
  bool pqEmpty()
  {
    if (LIKELY(_searchAlwaysLowToHigh))
      return _lthPUPQ.empty();

    return _puPQ.empty();
  }

  void pqPop()
  {
    if (LIKELY(_searchAlwaysLowToHigh))
    {
      if (LIKELY(!_lthPUPQ.empty()))
        _lthPUPQ.pop();

      return;
    }

    if (!_puPQ.empty())
      _puPQ.pop();
  }

  PUPQItem* pqTop()
  {
    if (LIKELY(_searchAlwaysLowToHigh))
    {
      if (LIKELY(!_lthPUPQ.empty()))
        return _lthPUPQ.top();
    }
    else
    {
      if (!_puPQ.empty())
        return _puPQ.top();
    }

    return nullptr;
  }

  void pqPush(PUPQItem* pupqItem)
  {
    // also set lower bound
    if (LIKELY(_searchAlwaysLowToHigh))
    {
      _lthPUPQ.push(pupqItem);
    }
    else
    {
      _puPQ.push(pupqItem);
    }
  }

  size_t pqSize()
  {
    if (_searchAlwaysLowToHigh)
      return _lthPUPQ.size();

    return _puPQ.size();
  }

  bool getInitCxrFareIndices(PUPQItem& pupqItem, ArrayVector<uint16_t>& fareIndices);
  bool getPUValCxr(PUPQItem& pupqItem, std::vector<CarrierCode>& puValCxr);

  bool getCxrFareRestriction(PUPQItem& pupqItem,
                             std::deque<bool>& cxrFareRest,
                             const CarrierCode& valCxr,
                             std::string& puCxrFareType);
  bool getCxrFareRestriction_old(PUPQItem& pupqItem,
                             std::deque<bool>& cxrFareRest,
                             std::string& puCxrFareType);

  void buildNextCxrFarePricingUnitSet(PUPQ& cxrFarePUPQ,
                                      const PUPQItem& prevPUPQItem,
                                      const std::deque<bool>& cxrFareRest,
                                      DiagCollector& diag);

  bool buildCxrFarePricingUnit(PUPQ& cxrFarePUPQ,
                               const bool initStage,
                               const PUPQItem& prevPUPQItem,
                               const ArrayVector<uint16_t>& fareIndices,
                               const std::deque<bool>& cxrFareRest,
                               const uint16_t xPoint,
                               DiagCollector& diag);

  virtual PUPQItem* getNextCxrFarePricinUnit(PUPQ& cxrFarePUPQ,
                                             const std::deque<bool>& cxrFareRest,
                                             DiagCollector& diag);

  virtual PUPQItem* getNextCxrFarePricinUnit_old(PUPQ& cxrFarePUPQ, DiagCollector& diag);

  bool buildNextLevelPricinUnit(DiagCollector& diag);

  bool validatePausedPricingUnit(DiagCollector& diag);

  void buildNextPricingUnitSet(const PUPQItem& pupqItem, DiagCollector& diag);

  bool buildPricingUnit(const PUPQItem* prevPUPQItem,
                        const ArrayVector<uint16_t>& fareIndices,
                        const uint16_t xPoint,
                        DiagCollector& diag);

  virtual bool buildFareUsage(const PaxTypeFare* primaryPaxTypeFare,
                              const uint16_t mktIdx,
                              uint16_t fareIdx,
                              bool& fareFound,
                              PUPQItem& pupqItem,
                              const PaxTypeFare::SegmentStatusVec* segStatus,
                              bool cxrFarePhase,
                              const FareType& fareType,
                              DiagCollector& diag,
                              bool fxCnException = false,
                              bool rexCheckSameFareDate = false);

  bool buildFareUsageOld(const PaxTypeFare* primaryPaxTypeFare,
                         const uint16_t mktIdx,
                         uint16_t fareIdx,
                         bool& fareFound,
                         PUPQItem& pupqItem,
                         const PaxTypeFare::SegmentStatusVec* segStatus,
                         bool cxrFarePhase,
                         const FareType& fareType,
                         DiagCollector& diag,
                         bool fxCnException = false,
                         bool rexCheckSameFareDate = false);

  bool sameBookingCodes(const PaxTypeFare::SegmentStatusVec& segStatus1,
                        const PaxTypeFare::SegmentStatusVec& segStatus2);

  void setUpNewFUforCat31(FareUsage& fu, PricingUnit& pu, const PaxTypeFare& ptf) const;

  bool checkSOJFareRestriction(const PricingUnit& prU) const;

  void copyPUTemplateInfo(PUPQItem& pupqItem);

  void copyPUCat5Result(PricingUnit& toPU, const PricingUnit& fromPU);

  bool getFareMktPaxTypeBucket(MergedFareMarket& mfm,
                               const uint16_t fareIdx,
                               uint16_t& relativeIdx,
                               FareMarket*& fm,
                               PaxTypeBucket*& paxTypeCortege);

  bool isFareValid(PaxTypeFare& paxTypeFare,
                      const PaxTypeBucket& paxTypeCortege,
                      const FareMarket& fm,
                      const uint16_t mktIdx,
                      bool& prevRuleFailed,
                      DiagCollector& diag,
                      bool fxCnException = false);

  bool matchPrimaryPaxTypeFareForINF(const PaxTypeFare* primaryPaxTypeFare,
                                     const PaxTypeFare* paxTypeFare) const;

  bool isFakeFareMarket(const FareMarket* fm) const;

  bool checkIS(const PaxTypeFare& paxTypeFare) const;
  bool checkAltDatesIS(const PaxTypeFare& paxTypeFare) const;
  bool checkAltDatesIS(const PricingUnit& pu) const;
  bool checkAltDatesMIPCutOff(const PUPQItem& pupqItem);

  bool checkRec2Cat10Indicator(const uint16_t mktIdx,
                               const PaxTypeFare& paxTypeFare,
                               std::string& failReason) const;

  bool checkCTRec2Cat10Indicator(const CombinabilityRuleInfo& rec2Cat10,
                                 const PaxTypeFare& paxTypeFare,
                                 std::string& failReason) const;

  bool checkRTRec2Cat10Indicator(const CombinabilityRuleInfo& rec2Cat10,
                                 const PaxTypeFare& paxTypeFare,
                                 std::string& failReason) const;

  bool checkOJRec2Cat10Indicator(const uint16_t mktIdx,
                                 const CombinabilityRuleInfo& rec2Cat10,
                                 const PaxTypeFare& paxTypeFare,
                                 std::string& failReason) const;

  virtual bool isSamePointApplicable(const uint16_t mktIdx,
                                     const CombinabilityRuleInfo& rec2Cat10,
                                     const DateTime& travelDate) const;

  bool isValidFareTypeGroup(const PaxTypeFare& ptf) const;

  bool checkCTFareRestriction(const PricingUnit& pu);

  bool hasBrandParity(const PricingUnit& pricingUnit) const;
  bool isValidForIbf(const PricingUnit& pricingUnit, DiagCollector& diag);

  virtual bool
  isPricingUnitValid(PUPQItem& pupqItem, const bool allowDelayedValidation, DiagCollector& diag);

  void setPriority(const MergedFareMarket& mfm,
                   PUPQItem& pupqItem,
                   const PaxTypeFare& paxTypeFare,
                   const Directionality puDir,
                   bool  cxrFarePhase,
                   bool& lowerPriorityFare);

  virtual bool usePrevFareUsage(const uint16_t mktIdx,
                                const PUPQItem& prevPUPQItem,
                                bool  cxrFarePhase,
                                PUPQItem& pupqItem,
                                DiagCollector& diag,
                                bool& hasPrevFailedFareUsage,
                                bool rexCheckSameFareDate = false);

  bool releasePUPQItem(PUPQItem* pupqItem);
  PUPQItem* constructPUPQItem();
  FareUsage* constructFareUsage();

  bool stopBuildingPU();

  virtual bool checkPULevelCombinability(PricingUnit& prU, DiagCollector& diag);

  bool checkOJSurfaceRestriction(PricingUnit& prU, DiagCollector& diag);

  bool performPULevelCombinabilityAndRuleValidation(PUPQItem& pupqItem,
                                                    bool allowDelayedValidation,
                                                    DiagCollector& diag);

  bool checkMileagePercentage(const PaxTypeFare& paxTypeFare1,
                              const PaxTypeFare& paxTypeFare2,
                              DiagCollector& diag);

  bool checkCarrierApplySameNUC(const FareMarket& fm1, const FareMarket& fm2);

  bool isPUWithInUSCA(const FareMarket& fm1, const FareMarket& fm2);

  bool sameNUCAmount(const PaxTypeFare& paxTypeFare1, const PaxTypeFare& paxTypeFare2);

  bool checkFaresCombOfSPAndNL(const PaxTypeFare& paxTypeFare1,
                               const PaxTypeFare& paxTypeFare2,
                               DiagCollector& diag);

  void displayConversionMessage(DiagCollector& diag,
                                const PricingUnit& prU,
                                const std::string& itins = "");

  bool processRTCTIATAExceptions(PricingUnit& prU, DiagCollector& diag);

  bool checkRTCTIATAExceptionsCommon(const PricingUnit& prU,
                                     const PaxTypeFare& paxTypeFare1,
                                     const PaxTypeFare& paxTypeFare2,
                                     DiagCollector& diag);

  // Apply new logic to decide whether trip type is CT or RT
  bool isPuRT(const PaxTypeFare& paxTypeFare1,
              const PaxTypeFare& paxTypeFare2,
              DiagCollector& diag,
              uint16_t catNum);

  const GeneralFareRuleInfo* getRecord2(const PaxTypeFare& ptf, uint16_t catNum) const;

  virtual bool getMileage(const FareMarket& fm,
                          const DateTime& travelDate,
                          uint32_t& miles,
                          DiagCollector& diag);

  virtual bool getMileage(const LocCode& city1,
                          const LocCode& city2,
                          GlobalDirection& gd,
                          const DateTime& travelDate,
                          uint32_t& miles,
                          DiagCollector& diag);

  uint32_t puCountDiagParam();
  void
  display601DiagHeader(const MergedFareMarket& mfm, const uint16_t fareIdx, DiagCollector& diag);

  void display601DiagMsg(const PaxTypeFare& paxTypeFare,
                         const PaxTypeBucket& paxTypeCortege,
                         const FareMarket& fm,
                         const uint16_t mktIdx,
                         const bool valid,
                         const bool ctNLSPFareRestPass,
                         const std::string& prevRuleStatus,
                         DiagCollector& diag) const;
  void displayPricingUnit(const PricingUnit& pu, DiagCollector& diag) const;
  void displayPrevFailedFC(const PricingUnit& pu, DiagCollector& diag) const;
  void displayPartiallyBuiltFailedPU(const PricingUnit& pu, DiagCollector& diag) const;

  bool isValidCarrierForOpenJawPU(const PU* pu, const PaxTypeFare& paxTypeFare) const;
  bool isValidFareForCxrSpecificOpenJaw(const PaxTypeFare& paxTypeFare) const;

  bool isValidForXORequest(const PaxTypeFare& paxTypeFare,
                           const PaxTypeBucket& paxTypeCortege,
                           DiagCollector& diag) const;

  virtual bool performPULevelRuleValidation(PricingUnit& prU, DiagCollector& diag);

  bool continuePUScopeValidation(PUPQItem& pupqItem) const;

  void setJourneyPULowerBound(const PUPQItem& pupqItem)
  {
    if (_pu->isCompleteJourney() && _initStage)
      _journeyPULowerBound->setJourneyPULowerBound(pupqItem.pricingUnit()->getTotalPuNucAmount());
  }

  bool checkPrevValidationResult(const PaxTypeFare* paxTypeFare) const
  {
    if (_pu->fcCount() > 1)
    {
      return _failedFareSet.count(paxTypeFare) == 0;
    }
    return true;
  }

  bool checkPrevValidationResult(PricingUnit& prUnit, DiagCollector& diag) const;

  bool hasFailedFare(const PUPQItem& pupqItem, bool hasPrevFailedFareUsage,
                     uint16_t mktIdx, uint16_t xPoint) const;

  bool checkFareTag(const PaxTypeFare& paxTypeFare,
                    GeoTravelType geoTravelType,
                    bool fxCnException,
                    DiagCollector& diag,
                    std::string& failReason) const;

  bool checkIndustryFareValidity(const PaxTypeFare& paxTypeFare) const;

  bool checkJLCNXFareSelection(const PaxTypeFare& paxTypeFare, const FareMarket& fareMarket) const;

  // bool getRebookedClassesStatus(PricingUnit& pu);
  // uint32_t getRebookedClassesStatus(const PaxTypeFare& paxTypeFare);

  bool checkRebookedClassesForRex(PUPQItem& pupqItem) const;
  bool checkRebookedClassesForRex(const PaxTypeFare& paxTypeFare) const;

  void setNegotiatedFarePriority(PUPQItem* pupqItem);

  uint32_t getCat35Record3ItemNo(const PaxTypeFare& paxTypeFare);

  virtual bool useCxrInCxrFltTbl(const std::vector<TravelSeg*>& tvlSegs,
                                 const VendorCode& vendor,
                                 int carrierFltTblItemNo,
                                 const DateTime& ticketingDate) const;

  virtual bool
  getGlobalDirection(DateTime travelDate, TravelSeg& tvlSeg, GlobalDirection& globalDir) const;

  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  DataHandle _dataHandle;
  Combinations* _combinations = nullptr;

  PU* _pu = nullptr;
  PaxType* _paxType = nullptr;
  bool _done = false;
  bool _useCxrPreference = true;

  static constexpr char RT_INDICATOR = 'R';
  static constexpr char OW_INDICATOR = 'O';

  static const uint16_t SHORT_CKT_COMB_COUNT = 100;

  // During Init, try at least INIT_COMB_VAL_COUNT times even
  // if another  complete journey PU has already been priced to
  // get a PU validated in PU-Factory
  //
  static const uint16_t INIT_COMB_VAL_COUNT = 10;

  // After Init stage, don't try to get a PU validated more
  // than MAX_CURR_COMB_VAL_COUNT time at one stretch/request
  //
  static const uint16_t MAX_CURR_COMB_VAL_COUNT = 50;

  uint32_t _puCount = 0; // num of valid pu to be used/reused
  uint32_t _puCombTried = 0; // num of pu combination tried
  uint32_t _curCombTried = 0; // num of pu combination tried with this request
  uint16_t _puScopeValMaxNum = 500; // num of pu to validate in pu factory
  MoneyAmount _delta = -1; // validate PU up to this delta

  JourneyPULowerBound* _journeyPULowerBound = nullptr;
  bool _initStage = true;
  bool _initializedForDelayXpn = false;

  uint32_t _reqPUCount = 0;

  PUPQ _puPQ;
  std::vector<PUPQItem*> _validPUPQItem;
  PUPQItem* _pausedPUPQItem = nullptr;

  bool _searchAlwaysLowToHigh = false;
  LowToHighPUPQ _lthPUPQ;

  PUPQ _cxrFarePUPQ;

  // CxrFare combo for each Cxr-FareType combo
  class CXRFareCombo
  {
  public:
    CXRFareCombo() : _done(false) {}

    std::vector<PUPQItem*>& validCxrPUPQItem() { return _validCxrPUPQItem; }
    const std::vector<PUPQItem*>& validCxrPUPQItem() const { return _validCxrPUPQItem; }

    PUPQ& cxrFarePUPQ() { return _cxrFarePUPQ; }
    const PUPQ& cxrFarePUPQ() const { return _cxrFarePUPQ; }

    bool& done() { return _done; }
    const bool& done() const { return _done; }

  private:
    PUPQ _cxrFarePUPQ;
    std::vector<PUPQItem*> _validCxrPUPQItem;
    bool _done;
  };

  bool _enableCxrFareSearchTuning = true;
  std::map<std::string, CXRFareCombo> _cxrFareComboMap;

  // need more testing for Cxr-Fare combo search tuning
  // std::set<std::vector<uint16_t> > _failedCxrFareIndices;

  std::set<const PaxTypeFare*> _failedFareSet;
  std::map<const PaxTypeFare*, std::set<const PaxTypeFare*> > _cat10FailedFare;
  std::map<const PaxTypeFare*, std::set<DatePair> > _altDateISFailedFare;

  uint16_t _fcCount = 1; // num of fare component
  double _shortCktTimeOut = 60; // after 60sec, stop building large CT/OJ
  bool _shutdownFactory = false;
  time_t _stopTime;

  bool _isISPhase = false;
  bool _puScopeValidationEnabled = true;
  bool _delayPUValidationMIP = false;

  uint16_t _maxSPCTFareCompCount = 2; // max num of FC allowed for SP-CT
  uint32_t _maxSPCTPUCount = 2000; // max num of SP-CT PU we should try
  uint32_t _spCTPUTried = 0; // num of sp-ct pu tried
  bool _multiCurrencyPricing = false;
  bool _altCurrencyRequest = false;
  CurrencyCode _altCurrencyCode;

  uint32_t _maxNbrCombMsgThreshold = 5000;
  bool _maxNbrCombMsgSet = false;

  FareUsagePool _fuPool;
  PricingUnitPool _puPool;
  PUPQItemPool _pupqPool;

  bool _clearingProcessedForRex = false;
  bool _taxPerFareBreak = false;
  bool _onlyOwFares = false;
  bool _onlyNonStops = false;
  FarePUResultContainer* _farePUResultContainer = nullptr;

  PricingUnitRequesterData _requesterData;
  unsigned _getPUPQItemCounter = 0;

private:
  static const BookingCode ALL_BOOKING_CODE;

  bool checkSingleCurrency(const PaxTypeFare& paxTypeFare,
                           const PaxTypeBucket& paxTypeCortege,
                           const FareMarket& fm,
                           const Directionality dir) const;

  bool checkMultiCurrency(const PaxTypeFare& paxTypeFare,
                          const PaxTypeBucket& paxTypeCortege,
                          const FareMarket& fm,
                          const Directionality dir) const;

  bool checkCurrency(const PaxTypeFare& paxTypeFare,
                     const PaxTypeBucket& paxTypeCortege,
                     const FareMarket& fm,
                     const Directionality dir,
                     bool isMultiCurrency) const;

  bool checkNonDirectionalFare(const PaxTypeFare& paxTypeFare,
                               const PaxTypeBucket& paxTypeCortege,
                               const CurrencyCode& currency,
                               GeoTravelType geoTravleType) const;

  bool checkAseanCurrencies(const std::vector<CurrencyCode>& aseanCurrencies,
                            const CurrencyCode& currency) const;
  bool checkISIsValidFare(PaxTypeFare& paxTypeFare) const;
  bool isNonStopFare(const PaxTypeFare& ptf) const;
  bool isNonStopPU(const PricingUnit& pu, DiagCollector& dc) const;
  bool isValidPUForValidatingCxr(PricingUnit& pu, DiagCollector& diag);
  bool isFareValidForTraditionalVC(const PaxTypeFare& paxTypeFare,
                                   const std::set<CarrierCode>& vcs) const
  {
    return std::any_of(paxTypeFare.validatingCarriers().cbegin(),
                       paxTypeFare.validatingCarriers().cend(),
                       [vcs] (const CarrierCode& cxr)
                       { return vcs.find(cxr) != vcs.end(); });
  }
};

} // tse namespace

