//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VCTR.h"
#include "DataModel/BaseExchangeTrx.h"

#include <map>
#include <vector>

namespace tse
{
class Itin;
class MinFarePlusUpItem;
class ProcessTagInfo;
class ReissueSequence;
class RexBaseTrx;
class VoluntaryChangesInfo;

enum RepriceFareValidationResult
{
  REPRICE_PASS,
  RULE_INDICATOR,
  CARRIER_APPLICATION_TBL,
  TARIFF_NUMBER,
  RULE_NUMBER,
  EXCLUDE_PRIVATE,
  FARE_CLASS_CODE,
  FARE_TYPE_CODE,
  FARE_TYPE_TABLE,
  FARE_AMOUNT,
  NORMAL_SPECIAL,
  OWRT_INDICATOR,
  SAME_INDICATOR
};

enum SameAirportValidationStatus
{
  SAME_AIRPORT_NOT_CHECKED,
  SAME_AIRPORT_PASS,
  SAME_AIRPORT_FAIL
};

enum SkippedValidations
{
  svNone = 0x00,
  svOriginallyScheduledFlight = 0x01,
  svCarrierRestrictions = 0x02,
  svCoupon = 0x04,
  svAgencyRestrictions = 0x08
};

class PartialFareBreakLimitationValidation final
{
public:
  void setUp();
  void add(const ProcessTagInfo& pti) { _processTags.push_back(&pti); }
  size_t size() const { return _processTags.size(); }
  void setNewSegments(const std::vector<TravelSeg*>& tss, const Itin& itin);
  bool doNotUseForPricing(const FareMarket& fm, const Itin& itin);

private:
  bool _forceFareBreak = true;
  bool _forceKeepFare = true;

  std::vector<const ProcessTagInfo*> _processTags;
  std::vector<int16_t> _newSegments;
};

class FareCompInfo
{
  friend class FareComponentTest;
  friend class BaseExchangeTrxTest;

public:
  class MultiNewItinData final
  {
  public:
    typedef std::map<const PaxTypeFare*, bool> FailByPrevReissued;
    typedef SmallBitSet<uint8_t, SkippedValidations> SkippedValidationsSet;
    typedef std::pair<uint16_t, SkippedValidationsSet> OverridingIntlFcData;
    typedef std::map<const VoluntaryChangesInfo*, OverridingIntlFcData> OverridingIntlFcCache;
    typedef std::pair<const ReissueSequence*, const PaxTypeFare*>
    RepriceFareValidationResultCacheKey;
    typedef std::map<RepriceFareValidationResultCacheKey, RepriceFareValidationResult>
    RepriceFareValidationResultCache;
    typedef std::pair<const ReissueSequence*, const FareMarket*>
    RepriceFullyFlownValidationResultCacheKey;
    typedef std::map<RepriceFullyFlownValidationResultCacheKey, Indicator>
    RepriceFullyFlownValidationResultCache;

  private:
    FailByPrevReissued _failByPrevReissued;
    OverridingIntlFcCache _overridingFcs;
    RepriceFareValidationResultCache _repriceValidationCache;
    RepriceFullyFlownValidationResultCache _repriceFFValidationCache;

  public:
    FailByPrevReissued& failByPrevReissued() { return _failByPrevReissued; }
    const FailByPrevReissued& failByPrevReissued() const { return _failByPrevReissued; }

    OverridingIntlFcCache& overridingFcs() { return _overridingFcs; }
    const OverridingIntlFcCache& overridingFcs() const { return _overridingFcs; }

    RepriceFareValidationResultCache& repriceValidationCache() { return _repriceValidationCache; }
    const RepriceFareValidationResultCache& repriceValidationCache() const
    {
      return _repriceValidationCache;
    }

    RepriceFullyFlownValidationResultCache& repriceFFValidationCache()
    {
      return _repriceFFValidationCache;
    }
    const RepriceFullyFlownValidationResultCache& repriceFFValidationCache() const
    {
      return _repriceFFValidationCache;
    }
  };

  class MatchedFare
  {
  public:
    MatchedFare(PaxTypeFare* ptf, double variance = -1.0) : _ptf(ptf), _variance(variance) {}
    const PaxTypeFare* get() const { return _ptf; }
    PaxTypeFare* get() { return _ptf; }
    double getVariance() const { return _variance; }

  protected:
    PaxTypeFare* _ptf;
    double _variance;
  };

  typedef MultiNewItinData::RepriceFareValidationResultCache RepriceFareValidationResultCache;
  typedef MultiNewItinData::RepriceFareValidationResultCacheKey RepriceFareValidationResultCacheKey;
  typedef MultiNewItinData::RepriceFullyFlownValidationResultCache
  RepriceFullyFlownValidationResultCache;
  typedef MultiNewItinData::RepriceFullyFlownValidationResultCacheKey
  RepriceFullyFlownValidationResultCacheKey;
  typedef MultiNewItinData::SkippedValidationsSet SkippedValidationsSet;
  typedef MultiNewItinData::OverridingIntlFcData OverridingIntlFcData;
  typedef MultiNewItinData::OverridingIntlFcCache OverridingIntlFcCache;
  typedef MultiNewItinData::FailByPrevReissued FailByPrevReissued;
  typedef std::map<uint16_t, MultiNewItinData> MultiNewItinDataMap;
  typedef std::vector<MatchedFare> MatchedFaresVec;

  virtual ~FareCompInfo() = default;

  uint16_t& fareCompNumber() { return _fareCompNumber; }
  const uint16_t& fareCompNumber() const { return _fareCompNumber; }

  MoneyAmount& fareCalcFareAmt() { return _fareCalcFareAmt; }
  const MoneyAmount fareCalcFareAmt() const { return _fareCalcFareAmt; }

  MoneyAmount& tktFareCalcFareAmt() { return _tktFareCalcFareAmt; }
  const MoneyAmount tktFareCalcFareAmt() const { return _tktFareCalcFareAmt; }

  FareClassCode& fareBasisCode() { return _fareBasisCode; }
  const FareClassCode& fareBasisCode() const { return _fareBasisCode; }

  const MinFarePlusUpItem*& hip() { return _hip; }
  const MinFarePlusUpItem* hip() const { return _hip; }

  bool hipIncluded() const { return (_hip != nullptr); }

  FareMarket*& fareMarket() { return _fareMarket; }
  const FareMarket* fareMarket() const { return _fareMarket; }

  FareMarket*& secondaryFareMarket() { return _secondaryFareMarket; }
  const FareMarket* secondaryFareMarket() const { return _secondaryFareMarket; }

  uint16_t& mileageSurchargePctg() { return _mileageSurchargePctg; }
  const uint16_t mileageSurchargePctg() const { return _mileageSurchargePctg; }

  LocCode& mileageSurchargeCity() { return _mileageSurchargeCity; }
  const LocCode mileageSurchargeCity() const { return _mileageSurchargeCity; }

  uint16_t& fareMatchingPhase() { return _fareMatchingPhase; }
  const uint16_t fareMatchingPhase() const { return _fareMatchingPhase; }

  bool& secondaryFMprocessed() { return _secondaryFMprocessed; }
  bool secondaryFMprocessed() const { return _secondaryFMprocessed; }

  RepriceFareValidationResultCache& repriceValidationCache()
  {
    return getMultiNewItinData().repriceValidationCache();
  }

  const RepriceFareValidationResultCache& repriceValidationCache() const
  {
    return getMultiNewItinData().repriceValidationCache();
  }

  RepriceFareValidationResultCache& repriceValidationCache(const uint16_t itinIndex)
  {
    return getMultiNewItinData(itinIndex).repriceValidationCache();
  }

  const RepriceFareValidationResultCache& repriceValidationCache(const uint16_t itinIndex) const
  {
    return getMultiNewItinData(itinIndex).repriceValidationCache();
  }

  RepriceFullyFlownValidationResultCache& repriceFFValidationCache()
  {
    return getMultiNewItinData().repriceFFValidationCache();
  }

  const RepriceFullyFlownValidationResultCache& repriceFFValidationCache() const
  {
    return getMultiNewItinData().repriceFFValidationCache();
  }

  RepriceFullyFlownValidationResultCache& repriceFFValidationCache(const uint16_t itinIndex)
  {
    return getMultiNewItinData(itinIndex).repriceFFValidationCache();
  }

  const RepriceFullyFlownValidationResultCache&
  repriceFFValidationCache(const uint16_t itinIndex) const
  {
    return getMultiNewItinData(itinIndex).repriceFFValidationCache();
  }

  SameAirportValidationStatus& sameAirportResult() { return _sameAirportResult; }
  const SameAirportValidationStatus sameAirportResult() const { return _sameAirportResult; }

  std::map<uint16_t, MoneyAmount>& stopoverSurcharges() { return _stopoverSurcharges; }
  const std::map<uint16_t, MoneyAmount>& stopoverSurcharges() const { return _stopoverSurcharges; }

  OverridingIntlFcCache& overridingFcs() { return getMultiNewItinData().overridingFcs(); }
  const OverridingIntlFcCache& overridingFcs() const
  {
    return getMultiNewItinData().overridingFcs();
  }

  OverridingIntlFcData* findOverridingData(const VoluntaryChangesInfo* vcRec3);

  PaxTypeBucket* getPaxTypeBucket(const RexBaseTrx& trx, bool secondaryMarket = false) const;

  void updateFareMarket(const RexBaseTrx& trx);
  void loadOtherFares(RexPricingTrx& ntrx);

  static uint16_t getOverridingFc(const OverridingIntlFcData* od)
  {
    return od ? od->first : uint16_t(0);
  }
  static SkippedValidationsSet* getSkippedValidationsSet(OverridingIntlFcData* od)
  {
    return od ? &od->second : nullptr;
  }

  bool discounted() const { return _discounted; }
  bool& discounted() { return _discounted; }

  bool hasVCTR() const { return _hasVCTR; }
  bool& hasVCTR() { return _hasVCTR; }

  const tse::VCTR& VCTR() const { return _VCTR; }
  tse::VCTR& VCTR() { return _VCTR; }

  MoneyAmount getTktBaseFareCalcAmt() const;

  FailByPrevReissued& failByPrevReissued() { return getMultiNewItinData().failByPrevReissued(); }
  const FailByPrevReissued& failByPrevReissued() const
  {
    return getMultiNewItinData().failByPrevReissued();
  }

  FailByPrevReissued& failByPrevReissued(const uint16_t itinIndex)
  {
    return getMultiNewItinData(itinIndex).failByPrevReissued();
  }

  const FailByPrevReissued& failByPrevReissued(const uint16_t itinIndex) const
  {
    return getMultiNewItinData(itinIndex).failByPrevReissued();
  }

  void setItinIndex(uint16_t itinIndex) { _itinIndex = itinIndex; }

  MatchedFaresVec& matchedFares() { return _matchedFares; }
  const MatchedFaresVec& getMatchedFares() const { return _matchedFares; }

  void updateFMMapping(Itin* itin);

  std::set<FareMarket*>& getMappedFCs() { return _mappedFC; }
  const std::set<FareMarket*>& getMappedFCs() const { return _mappedFC; }

  PartialFareBreakLimitationValidation& partialFareBreakLimitationValidation() { return _pfblv; }

  const bool isDscProcessed() const { return _isDscProcessed; }
  void setIsDscProcessed() { _isDscProcessed = true; }

protected:
  MultiNewItinData& getMultiNewItinData();
  const MultiNewItinData& getMultiNewItinData() const;
  MultiNewItinData& getMultiNewItinData(uint16_t itinIndex);
  const MultiNewItinData& getMultiNewItinData(uint16_t itinIndex) const;

  uint16_t _fareCompNumber = 0;
  MoneyAmount _fareCalcFareAmt = 0;
  MoneyAmount _tktFareCalcFareAmt = 0;
  FareClassCode _fareBasisCode;
  const MinFarePlusUpItem* _hip = nullptr;
  uint16_t _mileageSurchargePctg = 0;
  LocCode _mileageSurchargeCity;
  std::map<uint16_t, MoneyAmount> _stopoverSurcharges;
  FareMarket* _fareMarket = nullptr;
  FareMarket* _secondaryFareMarket = nullptr; // For multicarrier
  // and foreign domestic fare components
  bool _secondaryFMprocessed = false;

  uint16_t _fareMatchingPhase = 0;

  bool _discounted = false;

  bool _hasVCTR = false;
  tse::VCTR _VCTR;

  bool _isDscProcessed = false;

  SameAirportValidationStatus _sameAirportResult =
      SameAirportValidationStatus::SAME_AIRPORT_NOT_CHECKED;

  MatchedFaresVec _matchedFares;

  std::set<FareMarket*> _mappedFC;
  std::vector<PaxTypeFare*> _otherFares;

  PartialFareBreakLimitationValidation _pfblv;

  uint16_t _itinIndex = 0; // Exchange schopping - to get current new itin
  MultiNewItinDataMap _multiNewItinData; // Exchange schopping - data diff for each new itin
};

inline MoneyAmount
FareCompInfo::getTktBaseFareCalcAmt() const
{
  return (_mileageSurchargePctg > EPSILON)
             ? _tktFareCalcFareAmt * HUNDRED / (_mileageSurchargePctg + HUNDRED)
             : _tktFareCalcFareAmt;
}
} // namespace tse
