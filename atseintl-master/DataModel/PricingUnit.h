//-------------------------------------------------------------------
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseDateTimeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/VecMap.h"
#include "Common/VecMultiMap.h"
#include "DataModel/CopyablePtr.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/StructuredRuleData.h"

#include <memory>
#include <ostream>

namespace tse
{
class AdvResOverride;
class FareUsage;
class PU;
class TravelSeg;

class PricingUnit
{
public:
  enum class Type : uint8_t
  {
    UNKNOWN,
    OPENJAW,
    ROUNDTRIP,
    CIRCLETRIP,
    ONEWAY,
    ROUNDTHEWORLD_SFC,
    CIRCLETRIP_SFC
  };

  enum PUSubType : uint8_t
  {
    UNKNOWN_SUBTYPE = 0,
    DEST_OPENJAW,
    ORIG_OPENJAW,
    DOUBLE_OPENJAW
  };

  enum PUFareType : uint8_t
  {
    NL = 1,
    SP
  };

  enum PUIsCmdPricing : uint8_t
  {
    CP_UNKNOWN,
    NO_CMD_PRICING,
    IS_CMD_PRICING
  };

  enum OJSurfaceStatus : uint8_t
  {
    NOT_CHECKED,
    SURFACE_SHORTEST, // surface mileage is shorter than both the legs
    SURFACE_NOT_SHORTEST, // surface is shorter than the longer leg
    SURFACE_125LARGER, // but is less than 1.25 times of the largest flown leg, CR330
    SURFACE_VERYLARGE // not within 1.25 times of the largest flown leg
  };

  PricingUnit();

  bool needRecalculateCat12() const;
  void reuseSurchargeData() const;
  bool isEqualAmountComponents(const PricingUnit& rhs) const;
  bool isPUWithSameFares(const PricingUnit& rhs) const;
  void clearReusedFareUsage();
  bool areAllFaresNormal() const;

  // Access

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType& geoTravelType() const { return _geoTravelType; }

  Type& puType() { return _puType; }
  Type puType() const { return _puType; }

  PUSubType& puSubType() { return _puSubType; }
  const PUSubType& puSubType() const { return _puSubType; }

  OJSurfaceStatus& ojSurfaceStatus() { return _ojSurfaceStatus; }
  const OJSurfaceStatus& ojSurfaceStatus() const { return _ojSurfaceStatus; }

  bool& sameNationOJ() { return _sameNationOJ; }
  const bool& sameNationOJ() const { return _sameNationOJ; }

  PUFareType& puFareType() { return _puFareType; }
  const PUFareType& puFareType() const { return _puFareType; }

  const std::vector<FareUsage*>& fareUsage() const { return _fareUsages; }
  std::vector<FareUsage*>& fareUsage() { return _fareUsages; }

  const std::vector<FareUsage*>& reusedResultFUs() const { return _reusedResultFUs; }
  std::vector<FareUsage*>& reusedResultFUs() { return _reusedResultFUs; }

  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }
  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers; }
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers; }

  PseudoCityCode& frrCodeSourcePCC() { return _frrCodeSourcePCC; }
  const PseudoCityCode& frrCodeSourcePCC() const { return _frrCodeSourcePCC; }

  const TravelSeg* turnAroundPoint() const { return _turnAroundPoint; }
  const TravelSeg*& turnAroundPoint() { return _turnAroundPoint; }

  DateTime& earliestTktDT() { return _earliestTktDT; }
  const DateTime& earliestTktDT() const { return _earliestTktDT; }

  DateTime& latestTktDT() { return _latestTktDT; }
  const DateTime& latestTktDT() const { return _latestTktDT; }

  void setTotalPuNucAmount(const MoneyAmount& totalPuNUCAmount)
  {
    _totalPuNUCAmount = totalPuNUCAmount;
  }
  const MoneyAmount& getTotalPuNucAmount() const { return _totalPuNUCAmount; }

  MoneyAmount& taxAmount() { return _taxAmount; }
  const MoneyAmount& taxAmount() const { return _taxAmount; }

  void accumulateBaggageLowerBound(MoneyAmount lb);
  void rollbackBaggageLowerBound();

  void setBaggageLowerBound(MoneyAmount lb) { _baggageLowerBound = lb; }
  MoneyAmount baggageLowerBound() const { return _baggageLowerBound; }

  const bool& isOutBoundYQFTaxCharged() const { return _isOutBoundYQFTaxCharged; }
  bool& isOutBoundYQFTaxCharged() { return _isOutBoundYQFTaxCharged; }

  const bool& isInBoundYQFTaxCharged() const { return _isInBoundYQFTaxCharged; }
  bool& isInBoundYQFTaxCharged() { return _isInBoundYQFTaxCharged; }

  const bool& isOutBoundYQITaxCharged() const { return _isOutBoundYQITaxCharged; }
  bool& isOutBoundYQITaxCharged() { return _isOutBoundYQITaxCharged; }

  const bool& isInBoundYQITaxCharged() const { return _isInBoundYQITaxCharged; }
  bool& isInBoundYQITaxCharged() { return _isInBoundYQITaxCharged; }

  const uint8_t& sideTripNumber() const { return _sideTripNumber; }
  uint8_t& sideTripNumber() { return _sideTripNumber; }

  const bool& isSideTripPU() const { return _isSideTripPU; }
  bool& isSideTripPU() { return _isSideTripPU; }

  const bool& hasSideTrip() const { return _hasSideTrip; }
  bool& hasSideTrip() { return _hasSideTrip; }

  const bool& noPUToEOE() const { return _noPUToEOE; }
  bool& noPUToEOE() { return _noPUToEOE; }

  const std::vector<PricingUnit*>& sideTripPUs() const { return _sideTripPUs; }
  std::vector<PricingUnit*>& sideTripPUs() { return _sideTripPUs; }

  bool hasMultipleCurrency() const;
  bool hasMultipleCurrency(CurrencyCode& paxTypeFarecurr) const;
  CurrencyCode getFirstPaxTypeFareCurrency() const;

  MinFarePlusUp& minFarePlusUp() { return _minFarePlusUp; }
  const MinFarePlusUp& minFarePlusUp() const { return _minFarePlusUp; }

  const MinFarePlusUpItem* hrtojNetPlusUp() const { return _hrtojNetPlusUp; }
  MinFarePlusUpItem*& hrtojNetPlusUp() { return _hrtojNetPlusUp; }

  const MinFarePlusUpItem* hrtcNetPlusUp() const { return _hrtcNetPlusUp; }
  MinFarePlusUpItem*& hrtcNetPlusUp() { return _hrtcNetPlusUp; }

  const VecMultiMap<MinimumFareModule, std::string>& minFareUnchecked() const
  {
    return _minFareUnchecked;
  }
  VecMultiMap<MinimumFareModule, std::string>& minFareUnchecked() { return _minFareUnchecked; }

  bool isCmdPricing();
  bool setCmdPrcFailedFlag(const unsigned int category, const bool isFailed = true);

  bool cmdPricedWFail() const { return _cpFailedStatus.isSet(PaxTypeFare::PTFF_ALL); }

  PaxTypeFare::PaxTypeFareCPFailedStatus& cpFailedStatus() { return _cpFailedStatus; }
  const PaxTypeFare::PaxTypeFareCPFailedStatus& cpFailedStatus() const { return _cpFailedStatus; }

  const bool& exemptMinFare() const { return _exemptMinFare; }
  bool& exemptMinFare() { return _exemptMinFare; }

  const bool& exemptOpenJawRoundTripCheck() const { return _exemptOpenJawRoundTripCheck; }
  bool& exemptOpenJawRoundTripCheck() { return _exemptOpenJawRoundTripCheck; }

  bool nigeriaCurrencyAdjustment() const { return _nigeriaCurrencyAdjustment; }
  bool& nigeriaCurrencyAdjustment() { return _nigeriaCurrencyAdjustment; }

  const VecMap<int16_t, DateTime>& tvlSegNVA() const { return _tvlSegNVA; }
  VecMap<int16_t, DateTime>& tvlSegNVA() { return _tvlSegNVA; }

  const VecMap<int16_t, DateTime>& tvlSegNVB() const { return _tvlSegNVB; }
  VecMap<int16_t, DateTime>& tvlSegNVB() { return _tvlSegNVB; }

  const VecMap<int16_t, bool>& tvlSegNVABDone() const { return _tvlSegNVABDone; }
  VecMap<int16_t, bool>& tvlSegNVABDone() { return _tvlSegNVABDone; }

  bool& fareDirectionReversed() { return _fareDirectionReversed; }
  bool fareDirectionReversed() const { return _fareDirectionReversed; }

  bool isMirrorImage() const;

  PricingUnit* clone(DataHandle& dataHandle) const;

  typedef VecMap<int16_t, DateTime>::const_iterator TvlSegDTMapI;

  AdvResOverride*& volChangesAdvResOverride() { return _volChangesAdvResOverride; }
  const AdvResOverride* volChangesAdvResOverride() const { return _volChangesAdvResOverride; }

  const bool& hasKeepFare() const { return _hasKeepFare; }
  bool& hasKeepFare() { return _hasKeepFare; }

  const bool& ruleFailedButSoftPassForKeepFare() const { return _ruleFailedButSoftPassForKeepFare; }
  bool& ruleFailedButSoftPassForKeepFare() { return _ruleFailedButSoftPassForKeepFare; }

  const bool& combinationFailedButSoftPassForKeepFare() const
  {
    return _combinationFailedButSoftPassForKeepFare;
  }
  bool& combinationFailedButSoftPassForKeepFare()
  {
    return _combinationFailedButSoftPassForKeepFare;
  }

  bool dynamicValidationPhase() const { return _dynamicValidationPhase; }
  bool& dynamicValidationPhase() { return _dynamicValidationPhase; }

  int16_t& mostRestrictiveMaxStop() { return _mostRestrictiveMaxStop; }
  int16_t mostRestrictiveMaxStop() const { return _mostRestrictiveMaxStop; }

  int16_t& mostRestrictiveMaxTransfer() { return _mostRestrictiveMaxTransfer; }
  int16_t mostRestrictiveMaxTransfer() const { return _mostRestrictiveMaxTransfer; }

  bool hasTransferFCscope() const { return _hasTransferFCscope; }
  bool& hasTransferFCscope() { return _hasTransferFCscope; }

  bool isRebookedClassesStatus();

  int16_t& totalTransfers() { return _totalTransfers; }
  int16_t totalTransfers() const { return _totalTransfers; }

  int& mileage() { return _mileage; }
  int mileage() const { return _mileage; }

  const bool& itinWithinScandinavia() const { return _itinWithinScandinavia; }
  bool& itinWithinScandinavia() { return _itinWithinScandinavia; }

  const bool isOverrideCxrCat05TktAftRes() const { return _isOverrideCxrCat05TktAftRes; }
  bool& isOverrideCxrCat05TktAftRes() { return _isOverrideCxrCat05TktAftRes; }

  // Return true if any fare component covers travel segments from different legs.
  bool isAnyFareUsageAcrossTurnaroundPoint() const;

  const std::set<PU*>& intlOJToOW() const { return _intlOJToOW; }
  std::set<PU*>& intlOJToOW() { return _intlOJToOW; }

  const uint16_t& getFlexFaresGroupId() const { return _flexFaresGroupId; }
  void setFlexFaresGroupId(const uint16_t& id) { _flexFaresGroupId = id; }

  void addChangedFareCat10overrideCat25(PaxTypeFare* target, PaxTypeFare* source)
  {
    _targetToSourceFareCat10overrideCat25.insert(std::make_pair(target, source));
  }

  bool isFareChangedCat10overrideCat25(const PaxTypeFare* fare) const
  {
    return _targetToSourceFareCat10overrideCat25.find(fare) !=
        _targetToSourceFareCat10overrideCat25.end();
  }

  PaxTypeFare* getOldFareCat10overrideCat25(PaxTypeFare* fare) const
  {
    std::map<const PaxTypeFare*, PaxTypeFare*>::const_iterator it =
        _targetToSourceFareCat10overrideCat25.find(fare);
    if(it != _targetToSourceFareCat10overrideCat25.end())
      return it->second;
    return fare;
  }

  const size_t getFaresMapSizeCat10overrideCat25() const {
    return _targetToSourceFareCat10overrideCat25.size(); }

  void clearFareMapCat10overrideCat25() { _targetToSourceFareCat10overrideCat25.clear(); }

  void copyBkgStatusForEachFareUsageFromPaxTypeFare();

  bool isADDatePassValidation(const DatePair& altDatePair) const;

  MostRestrictivePricingUnitSFRData& getMostRestrictivePricingUnitSFRData() const
  {
    TSE_ASSERT(_mostRestrictiveStructuredRuleData != nullptr);
    return *_mostRestrictiveStructuredRuleData;
  }

  void createMostRestrictivePricingUnitSFRData()
  {
    _mostRestrictiveStructuredRuleData.reset(new MostRestrictivePricingUnitSFRData);
  }

  bool hasMostRestrictivePricingUnitSFRData() const
  {
    return _mostRestrictiveStructuredRuleData &&
           (!_mostRestrictiveStructuredRuleData->_minStayMap.empty() ||
            !_mostRestrictiveStructuredRuleData->_maxStayMap.empty());
  }

  bool isTravelSegPartOfPricingUnit(const TravelSeg* tvlSeg) const;

  const FareUsage* getFareUsageWithFirstTravelSeg(const TravelSeg* travelSeg) const;
  MoneyAmount getDynamicPriceDeviationForLeg(int16_t legId) const;

private:
  PaxType* _paxType = nullptr;
  Type _puType = Type::UNKNOWN;
  PUSubType _puSubType = PUSubType::UNKNOWN_SUBTYPE; // for OJ
  OJSurfaceStatus _ojSurfaceStatus = OJSurfaceStatus::NOT_CHECKED;

  PUIsCmdPricing _puIsCmdPricing = PUIsCmdPricing::CP_UNKNOWN; // for 3.5 command pricing
  bool _sameNationOJ = false; // for OJ
  PUFareType _puFareType = PUFareType::NL;
  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;
  uint8_t _sideTripNumber = 0; // non zero, when _isSideTripPU is true

  const TravelSeg* _turnAroundPoint = nullptr;

  bool _isSideTripPU = false;
  bool _hasSideTrip = false;
  bool _noPUToEOE = false; // to facilitate comb-validation
  bool _isOutBoundYQFTaxCharged = false;
  bool _isInBoundYQFTaxCharged = false;
  bool _isOutBoundYQITaxCharged = false;
  bool _isInBoundYQITaxCharged = false;

  std::vector<PricingUnit*> _sideTripPUs;

  std::vector<FareUsage*> _fareUsages;
  std::vector<TravelSeg*> _travelSeg;

  std::vector<CarrierCode> _validatingCarriers;

  PseudoCityCode _frrCodeSourcePCC;

  VecMap<int16_t, DateTime> _tvlSegNVA;
  VecMap<int16_t, DateTime> _tvlSegNVB;
  VecMap<int16_t, bool> _tvlSegNVABDone;

  MoneyAmount _totalPuNUCAmount = 0;
  MoneyAmount _taxAmount = 0;
  MoneyAmount _baggageLowerBound = 0;

  MinFarePlusUp _minFarePlusUp;
  VecMultiMap<MinimumFareModule, std::string> _minFareUnchecked;
  MinFarePlusUpItem* _hrtojNetPlusUp = nullptr;
  MinFarePlusUpItem* _hrtcNetPlusUp = nullptr;

  DateTime _earliestTktDT;
  DateTime _latestTktDT;

  PaxTypeFare::PaxTypeFareCPFailedStatus _cpFailedStatus =
      PaxTypeFare::CmdPricingFailedState::PTFF_NONE;
  AdvResOverride* _volChangesAdvResOverride = nullptr;

  bool _exemptMinFare = false;
  bool _exemptOpenJawRoundTripCheck = false;

  bool _nigeriaCurrencyAdjustment = false; // whether NG Adjustment should be applied.
  bool _fareDirectionReversed = false;

  // For Cat31 exchange rule validation
  bool _hasKeepFare = false;
  bool _ruleFailedButSoftPassForKeepFare = false;
  bool _combinationFailedButSoftPassForKeepFare = false;
  bool _hasTransferFCscope = false;

  // For WPQ Cat 8/9
  int16_t _mostRestrictiveMaxStop = -1;
  int16_t _mostRestrictiveMaxTransfer = -1;
  int16_t _totalTransfers = 0;

  bool _dynamicValidationPhase = false;
  bool _itinWithinScandinavia = false;
  int _mileage = 0;
  // for infini cat05 use PricingDT for ticketingtimelimit
  bool _isOverrideCxrCat05TktAftRes = false;

  uint16_t _flexFaresGroupId = 0;

  std::set<PU*> _intlOJToOW;
  std::vector<FareUsage*> _reusedResultFUs;
  std::map<const PaxTypeFare*, PaxTypeFare*> _targetToSourceFareCat10overrideCat25;
  CopyablePtr<MostRestrictivePricingUnitSFRData> _mostRestrictiveStructuredRuleData;
};

std::ostream&
operator<<(std::ostream& os, PricingUnit::Type puType);

} // tse namespace
