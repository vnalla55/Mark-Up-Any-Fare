//-------------------------------------------------------------------
//
//  File:        FarePath.h
//  Created:     March 8, 2004
//  Authors:
//
//  Description:
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

#include "Common/Money.h"
#include "Common/SmallBitSet.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/CopyablePtr.h"
#include "DataModel/Itin.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/StructuredRuleData.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TicketingFeesInfo.h"
#include "Routing/RoutingConsts.h"
#include "Rules/TicketingEndorsement.h"

#include <utility>

namespace tse
{
class Loc;
class PaxType;
class PricingUnit;
class ClassOfService;
class CollectedNegFareData;
class DiagCollector;
class AirSeg;
class DataHandle;
class PaxTypeFare;
class PricingTrx;
class ProcessTagPermutation;
class RefundPermutation;
class RexPricingTrx;
class ReissueCharges;
class BaggageTravel;
class NetRemitFarePath;
class NetFarePath;
class YQYRCalculator;
class MaxPenaltyResponse;
class PUPath;
class FarePathFactoryStorage;

class FarePath
{
  friend class FarePathTest;
  friend class FarePathTestWithRexTrx;

public:
  // map: validating carrier -> cloned FarePath
  using TSVec = std::vector<TravelSeg*>;
  using ItinTSVec = std::vector<TSVec>;

  // FarePathKey: PaxType,
  //             Outbound Brand Id,
  //             Inbound Brand Id,
  //             Outbound Itinerary pointer,
  //             Inbound Itinerary pointer
  using FarePathKey = HashKey<const PaxType*, uint16_t, uint16_t, const Itin*, const Itin*>;

  static FarePathKey buildKey(const PricingTrx& trx, const FarePath* farePath)
  {
    FarePath::FarePathKey farePathKey(farePath->paxType(),
                                      farePath->brandIndexPair().first,
                                      farePath->brandIndexPair().second,
                                      farePath->parentItinPair().first,
                                      farePath->parentItinPair().second);

    return farePathKey;
  }

  class OscPlusUp : public MinFarePlusUpItem
  {
  public:
    const PaxTypeFare* thruFare = nullptr;
    std::vector<PricingUnit*> pus;
  };

  class RscPlusUp : public MinFarePlusUpItem
  {
  public:
    LocCode inboundBoardPoint;
    LocCode inboundOffPoint;
    LocCode constructPoint2;
  };

  //////////////////////////////////////////////////////////////////////////
  //
  // TODO: Pending approval.

  class PlusUpInfo final
  {
  public:
    PlusUpInfo() = default;

    PlusUpInfo(MinimumFareModule module,
               MinFarePlusUpItem* minFarePlusUp,
               int16_t startSeg = 0,
               int16_t endSeg = 0)
      : _module(module), _startSeg(startSeg), _endSeg(endSeg), _minFarePlusUp(minFarePlusUp)
    {
    }

    MinimumFareModule module() const { return _module; }
    MinimumFareModule& module() { return _module; }

    const MinFarePlusUpItem* minFarePlusUp() const { return _minFarePlusUp; }
    MinFarePlusUpItem*& minFarePlusUp() { return _minFarePlusUp; }

    const int16_t startSeg() const { return _startSeg; }
    int16_t& startSeg() { return _startSeg; }
    const int16_t endSeg() const { return _endSeg; }
    int16_t& endSeg() { return _endSeg; }

    class KeyEquals
    {
    public:
      KeyEquals(MinimumFareModule module, int16_t startSeg, int16_t endSeg)
        : _module(module), _startSeg(startSeg), _endSeg(endSeg)
      {
      }

      bool operator()(const PlusUpInfo* plusUpInfo)
      {
        return (_module == plusUpInfo->_module && _startSeg == plusUpInfo->_startSeg &&
                _endSeg == plusUpInfo->_endSeg);
      }

    private:
      MinimumFareModule _module;
      int16_t _startSeg;
      int16_t _endSeg;
    };

  private:
    MinimumFareModule _module = MAX_FARE_MODULE_IND;
    int16_t _startSeg = 0;
    int16_t _endSeg = 0;
    MinFarePlusUpItem* _minFarePlusUp = nullptr;
  }; // class PlusUpInfo

  void storeCommissionForValidatingCarrier(const CarrierCode& valCxr, MoneyAmount amt)
  {
    _valCxrCommAmtCol.insert(std::pair<CarrierCode, MoneyAmount>(valCxr, amt));
  }

  FarePath* clone(PUPath* puPath, FarePathFactoryStorage& storage);

  bool doesValCarriersHaveDiffComm(const CarrierCode& dcx) const;

private:
  std::vector<PlusUpInfo*> _plusUpInfoList;

  CurrencyCode _baseFareCurrency;
  CurrencyCode _calculationCurrency;

public:
  const std::vector<PlusUpInfo*>& plusUpInfoList() const { return _plusUpInfoList; }
  std::vector<PlusUpInfo*>& plusUpInfoList() { return _plusUpInfoList; }

  MinFarePlusUpItem*
  minFarePlusUp(MinimumFareModule module, int16_t startSeg, int16_t endSeg) const;

  enum RexStatus : uint8_t
  { REX_NOT_PROCESSED = 0,
    REX_PASSED,
    REX_FAILED };

  virtual ~FarePath() = default;

  const bool& processed() const { return _processed; }
  bool& processed() { return _processed; }

  const bool& isAdjustedSellingFarePath() const { return _isAdjustedSellingFarePath; }
  void setAdjustedSellingFarePath() { _isAdjustedSellingFarePath = true; }

  const MoneyAmount aslMslDiffAmount() const { return _aslMslDiffAmount; }
  MoneyAmount& aslMslDiffAmount() { return _aslMslDiffAmount; }

  const bool& intlSurfaceTvlLimit() const { return _intlSurfaceTvlLimit; }
  bool& intlSurfaceTvlLimit() { return _intlSurfaceTvlLimit; }

  const bool ignoreSurfaceTPM() const { return _ignoreSurfaceTPM; }
  bool& ignoreSurfaceTPM() { return _ignoreSurfaceTPM; }

  const bool& plusUpFlag() const { return _plusUpFlag; }
  bool& plusUpFlag() { return _plusUpFlag; }

  const bool& selectedNetRemitFareCombo() const { return _selectedNetRemitFareCombo; }
  bool& selectedNetRemitFareCombo() { return _selectedNetRemitFareCombo; }

  const bool& rebookClassesExists() const { return _rebookClassesExists; }
  bool& rebookClassesExists() { return _rebookClassesExists; }

  const bool& bookingCodeFailButSoftPassForKeepFare() const
  {
    return _bookingCodeFailButSoftPassForKeepFare;
  }
  bool& bookingCodeFailButSoftPassForKeepFare() { return _bookingCodeFailButSoftPassForKeepFare; }

  const bool multipleTourCodeWarning() const { return _multipleTourCodeWarning; }
  void setMultipleTourCodeWarning(Record3ReturnTypes validationResult, bool isCmdPricing)
  {
    _multipleTourCodeWarning = validationResult == FAIL && isCmdPricing;
  }

  bool isTravelSegPartOfFarePath(const TravelSeg* tvlSeg) const;

  MoneyAmount rexChangeFee() const
  {
    return _rexChangeFee;
  }

  void setRexChangeFee(MoneyAmount fee) { _rexChangeFee = fee; }

  MoneyAmount yqyrNUCAmount() const { return _yqyrNUCAmount; }
  void setYqyrNUCAmount(MoneyAmount a) { _yqyrNUCAmount = a; }

  MoneyAmount bagChargeNUCAmount() const { return _bagChargeNUCAmount; }
  void setBagChargeNUCAmount(MoneyAmount a) { _bagChargeNUCAmount = a; }

  const MoneyAmount plusUpAmount() const { return _plusUpAmount; }
  MoneyAmount& plusUpAmount() { return _plusUpAmount; }

  const MoneyAmount getTotalNUCAmount() const { return _totalNUCAmount; }
  void setTotalNUCAmount(MoneyAmount amt) { _totalNUCAmount = amt; }
  void increaseTotalNUCAmount(MoneyAmount amt) { _totalNUCAmount += amt; }
  void decreaseTotalNUCAmount(MoneyAmount amt) { _totalNUCAmount -= amt; }

  MoneyAmount getTotalNUCMarkupAmount() const { return _totalNUCMarkupAmount; }
  void setTotalNUCMarkupAmount(MoneyAmount totalNucMarkupAmount) { _totalNUCMarkupAmount = totalNucMarkupAmount; }

  MoneyAmount getDynamicPriceDeviationAmount() const { return _dynamicPriceDeviationAmount; }
  void resetTotalNUCAmount(MoneyAmount amt);
  void rollbackDynamicPriceDeviation();
  void accumulatePriceDeviationAmount(MoneyAmount amt);
  MoneyAmount getUndeviatedTotalNUCAmount() const { return _totalNUCAmount - _dynamicPriceDeviationAmount; }
  MoneyAmount getDynamicPriceDeviationForLeg(int16_t legId) const;

  MoneyAmount getSpanishResidentUpperBoundDiscAmt() const
  {
    return _spanishResidentUpperBoundDiscAmt;
  }
  void setSpanishResidentUpperBoundDiscAmt(MoneyAmount amt)
  {
    _spanishResidentUpperBoundDiscAmt = amt;
  }

  const MoneyAmount getNUCAdditionalFees() const
  {
    return rexChangeFee() + yqyrNUCAmount() + bagChargeNUCAmount();
  }

  const MoneyAmount getNUCAmountScore() const
  {
    return getUndeviatedTotalNUCAmount() + getNUCAdditionalFees();
  }

  int& mileage() { return _mileage; }
  int mileage() const { return _mileage; }

  const MoneyAmount& unroundedTotalNUCAmount() const { return _unroundedTotalNUCAmount; }
  MoneyAmount& unroundedTotalNUCAmount() { return _unroundedTotalNUCAmount; }

  const std::vector<TicketEndorseItem>& tktEndorsement() const { return _tktEndorsement; }
  std::vector<TicketEndorseItem>& tktEndorsement() { return _tktEndorsement; }

  const Itin* itin() const { return _itin; }
  Itin*& itin() { return _itin; }

  const PaxType* paxType() const { return _paxType; }
  PaxType*& paxType() { return _paxType; }

  const CollectedNegFareData* collectedNegFareData() const { return _collectedNegFareData; }
  CollectedNegFareData*& collectedNegFareData() { return _collectedNegFareData; }

  const NetRemitFarePath* netRemitFarePath() const { return _netRemitFarePath; }
  NetRemitFarePath*& netRemitFarePath() { return _netRemitFarePath; }

  const std::string& cat27TourCode() const { return _cat27TourCode; }
  std::string& cat27TourCode() { return _cat27TourCode; }

  const FarePath* originalFarePathAxess() const { return _originalFarePathAxess; }
  FarePath*& originalFarePathAxess() { return _originalFarePathAxess; }

  const FarePath* axessFarePath() const { return _axessFarePath; }
  FarePath*& axessFarePath() { return _axessFarePath; }

  const FarePath* adjustedSellingFarePath() const { return _adjustedSellingFarePath; }
  FarePath*& adjustedSellingFarePath() { return _adjustedSellingFarePath; }

  const NetFarePath* netFarePath() const { return _netFarePath; }
  NetFarePath*& netFarePath() { return _netFarePath; }

  const ProcessTagPermutation* lowestFee31Perm() const { return perm._31LowestPerm; }
  void setLowestFee31Perm(const ProcessTagPermutation* p) { perm._31LowestPerm = p; }

  const RefundPermutation* lowestFee33Perm() const { return perm._33LowestPerm; }
  void setLowestFee33Perm(const RefundPermutation* p) { perm._33LowestPerm = p; }

  bool& ignoreReissueCharges() { return _ignoreReissueCharges; }
  const bool ignoreReissueCharges() const { return _ignoreReissueCharges; }

  bool& flownOWFaresCollected() { return _flownOWFaresCollected; }
  const bool flownOWFaresCollected() const { return _flownOWFaresCollected; }

  std::vector<FareUsage*>& flownOWFares() { return _flownOWFares; }
  const std::vector<FareUsage*>& flownOWFares() const { return _flownOWFares; }

  const std::vector<PricingUnit*>& pricingUnit() const { return _pricingUnit; }
  std::vector<PricingUnit*>& pricingUnit() { return _pricingUnit; }

  const std::vector<OscPlusUp*>& oscPlusUp() const { return _oscPlusUp; }
  std::vector<OscPlusUp*>& oscPlusUp() { return _oscPlusUp; }

  const std::vector<RscPlusUp*>& rscPlusUp() const { return _rscPlusUp; }
  std::vector<RscPlusUp*>& rscPlusUp() { return _rscPlusUp; }

  MostRestrictiveJourneySFRData& getMostRestrictiveJourneySFRData() const
  {
    TSE_ASSERT(_mostRestrictiveJourneyData);
    return *_mostRestrictiveJourneyData;
  }
  void createMostRestrictiveJourneySFRData()
  {
    _mostRestrictiveJourneyData.reset(new MostRestrictiveJourneySFRData);
  }
  bool hasMostRestrictiveJourneySFRData() const { return _mostRestrictiveJourneyData != nullptr; }

  void rollBackSurcharges() { _rollBackSurcharges = true; }
  const bool isRollBackSurcharges() const { return _rollBackSurcharges; }

  const MoneyAmount& commissionAmount() const { return _commissionAmount; }
  MoneyAmount& commissionAmount() { return _commissionAmount; }

  const Percent& commissionPercent() const { return _commissionPercent; }
  Percent& commissionPercent() { return _commissionPercent; }

  std::string& trailerCurrAdjMsg() { return _trailerCurrAdjMsg; }
  const std::string trailerCurrAdjMsg() const { return _trailerCurrAdjMsg; }

  std::string& paperTktSurcharge() { return _paperTktSurcharge; }
  const std::string paperTktSurcharge() const { return _paperTktSurcharge; }

  const bool& tktRestricted() const { return _tktRestricted; }
  bool& tktRestricted() { return _tktRestricted; }

  const bool& tfrRestricted() const { return _tfrRestricted; }
  bool& tfrRestricted() { return _tfrRestricted; }

  const bool& regularNet() const { return _regularNet; }
  bool& regularNet() { return _regularNet; }

  const std::string tktFareVendor(const PricingTrx& trx) const;

  bool cmdPrcWithWarning();

  bool needRecalculateCat12() const;

  void reuseSurchargeData() const;

  bool isEqualAmountComponents(const FarePath& rhs) const;

  void collectBookingCode(const PricingTrx& trx, const FareUsage* fu);

  std::vector<std::string> bookingCodeRebook;

  const bool& noMatchOption() const { return _noMatchOption; }
  bool& noMatchOption() { return _noMatchOption; }

  bool duplicate() const { return _duplicate; }
  bool& duplicate() { return _duplicate; }

  struct isDuplicate : public std::unary_function<const FarePath*, bool>
  {
    bool operator()(const FarePath* farePath) const { return farePath->duplicate(); }
  };
  bool& forceCat5Validation() { return _forceCat5Validation; }
  bool forceCat5Validation() const { return _forceCat5Validation; }

  void updateTktEndorsement();
  virtual std::string getNonrefundableMessage() const;
  virtual std::string getNonrefundableMessage2() const;
  bool& isExcTicketHigher() { return _isExcTicketHigher; }
  void calculateNonrefundableAmount(const FarePath& excFarePath, RexPricingTrx& trx);
  void setHigherNonrefundableAmountInBaseCurr(const MoneyAmount& amt)
  {
    _nonrefundableAmount = Money(amt, baseFareCurrency());
  }
  const Money& getHigherNonRefundableAmount() const { return _nonrefundableAmount; }

  Indicator residualPenaltyIndicator(const RexPricingTrx& trx) const;

  ReissueCharges*& reissueCharges() { return _reissueCharges; }
  const ReissueCharges* reissueCharges() const { return _reissueCharges; }

  const uint16_t& getFlexFaresGroupId() const { return _flexFaresGroupId; }
  void setFlexFaresGroupId(const uint16_t& id) { _flexFaresGroupId = id; }

  // MIP only: final booking codes and seat counts.
  std::vector<ClassOfService*>& mutableFinalBooking() { return _finalBooking; }
  const std::vector<ClassOfService*>& finalBooking() const { return _finalBooking; }

  // ******************************** MIP/ASE ********************************* //
  uint16_t& brandIndex() { return _brandIndex; }
  const uint16_t& brandIndex() const { return _brandIndex; }

  const BrandCode& getBrandCode() const { return _brandCode; }
  void setBrandCode(BrandCode brandCode) { _brandCode = brandCode; }

  std::pair<uint16_t, uint16_t>& brandIndexPair() { return _brandIndexPair; }
  const std::pair<uint16_t, uint16_t>& brandIndexPair() const { return _brandIndexPair; }

  std::pair<Itin*, Itin*>& parentItinPair() { return _parentItinPair; }
  const std::pair<Itin*, Itin*>& parentItinPair() const { return _parentItinPair; }
  // ******************************** MIP/ASE ********************************* //

  // **** QREX/BSP  **** //
  Indicator& exchangeReissue() { return _reissueVsExchange; }
  const Indicator& exchangeReissue() const { return _reissueVsExchange; }
  const bool isExchange() const { return _reissueVsExchange == EXCHANGE; }
  const bool isReissue() const { return _reissueVsExchange == REISSUE; }
  const bool useSecondRoeDate() const { return _useSecondRoeDate; }
  bool& useSecondRoeDate() { return _useSecondRoeDate; }
  bool& minFareCheckDone() { return _minFareCheckDone; }
  bool minFareCheckDone() const { return _minFareCheckDone; }

  // **** QREX/BSP **** //

  // This function object is needed for FarePath Priority Queue
  class Greater
  {
  public:
    bool operator()(FarePath& lhs, FarePath& rhs)
    {
      return lhs.getNUCAmountScore() > rhs.getNUCAmountScore();
    }

    bool operator()(FarePath* lhs, FarePath* rhs)
    {
      return lhs->getNUCAmountScore() > rhs->getNUCAmountScore();
    }
  };

  class Less
  {
  public:
    bool operator()(FarePath& lhs, FarePath& rhs)
    {
      return lhs.getNUCAmountScore() < rhs.getNUCAmountScore();
    }

    bool operator()(FarePath* lhs, FarePath* rhs)
    {
      return lhs->getNUCAmountScore() < rhs->getNUCAmountScore();
    }
  };

  /**
  * @method determineMostRestrTktDT
  *
  * Description: Determines most restricted ticketing date decided by
  *  advance reservation/ticketing (cat5) rules
  *
  * @param (OUT) DateTime& latestTktDT - ticketing must be done by
  *      this date, set as openDate
  *      if no rules apply
  *  (OUT) DateTime& earliestTktDT - ticket can not be issued
  *      before this date, set as
  *      openDate if no rules apply
  *  (IN) DiagCollector* diagPtr - Optional, when not 0, require
  *      this function to print out
  *      diagnostic
  *
  * @return void
  */
  void determineMostRestrTktDT(PricingTrx& trx,
                               DateTime& latestTktDT,
                               DateTime& earliestTktDT,
                               bool& simultaneousResTkt,
                               DiagCollector* diagPtr = nullptr) const;

  static const uint16_t PENALTIES_RULE = 16;
  static const uint16_t MIN_STAY_RULE = 6;
  static const uint16_t MAX_STAY_RULE = 7;
  static const std::string T225;
  static const std::string T226;

  Itin::ISICode intlSaleIndicator() const { return _intlSaleIndicator; }
  Itin::ISICode& intlSaleIndicator() { return _intlSaleIndicator; }

  CurrencyCode& baseFareCurrency() { return _baseFareCurrency; }
  const CurrencyCode& baseFareCurrency() const { return _baseFareCurrency; }

  CurrencyCode& calculationCurrency() { return _calculationCurrency; }
  const CurrencyCode& calculationCurrency() const { return _calculationCurrency; }
  const CurrencyCode& getCalculationCurrency() const;

  const bool fuelSurchargeIgnored() const { return _fuelSurchargeIgnored; }
  bool& fuelSurchargeIgnored() { return _fuelSurchargeIgnored; }

  RexStatus rexStatus() const { return _rexStatus; }
  RexStatus& rexStatus() { return _rexStatus; }

  class CompareAirSegs
  {
  public:
    bool operator()(const AirSeg* airSeg1, const AirSeg* airSeg2) const
    {
      return (airSeg1->segmentOrder() < airSeg2->segmentOrder());
    }

    bool operator()(const AirSeg& airSeg1, const AirSeg& airSeg2) const
    {
      return (airSeg1.segmentOrder() < airSeg2.segmentOrder());
    }
  };

  using SegToAllowanceTextMap = std::map<const AirSeg*, std::string, CompareAirSegs>;

  SegToAllowanceTextMap& mutableBaggageAllowance() { return _baggageAllowance; }
  const SegToAllowanceTextMap& baggageAllowance() const { return _baggageAllowance; }

  std::vector<const BaggageTravel*>& baggageTravels() { return _baggageTravels; }
  const std::vector<const BaggageTravel*>& baggageTravels() const { return _baggageTravels; }

  std::vector<const BaggageTravel*>& baggageTravelsPerSector() { return _baggageTravelsPerSector; }
  const std::vector<const BaggageTravel*>& baggageTravelsPerSector() const
  {
    return _baggageTravelsPerSector;
  }

  CarrierCode& defaultValidatingCarrier() { return _defaultValidatingCarrier; }
  const CarrierCode& defaultValidatingCarrier() const { return _defaultValidatingCarrier; }

  CarrierCode& marketingCxrForDefaultValCxr() { return _marketingCxrForDefaultValCxr; }
  const CarrierCode& marketingCxrForDefaultValCxr() const { return _marketingCxrForDefaultValCxr; }

  std::map<SettlementPlanType, CarrierCode>& marketingCxrForDefaultValCxrPerSp()
  {
    return _mktCxrForDefaultValCxrPerSp;
  }
  const std::map<SettlementPlanType, CarrierCode>& marketingCxrForDefaultValCxrPerSp() const
  {
    return _mktCxrForDefaultValCxrPerSp;
  }

  std::vector<CarrierCode>& validatingCarriers() { return _validatingCarriers; }
  const std::vector<CarrierCode>& validatingCarriers() const { return _validatingCarriers; }

  using ValCxrTaxResponseMap = std::map<CarrierCode, TaxResponse*>;
  ValCxrTaxResponseMap& valCxrTaxResponseMap() { return _valCxrTaxResponseMap; }
  const ValCxrTaxResponseMap& valCxrTaxResponseMap() const { return _valCxrTaxResponseMap; }

  void setValCxrTaxResponse(const CarrierCode& valCxr, TaxResponse* taxResponse)
  {
    _valCxrTaxResponseMap.insert(ValCxrTaxResponseMap::value_type(valCxr, taxResponse));
  }

  using ValCxrTaxResponsesMap = std::map<const CarrierCode, std::vector<TaxResponse*>>;
  ValCxrTaxResponsesMap& valCxrTaxResponses() { return _valCxrTaxResponses; }
  const ValCxrTaxResponsesMap& valCxrTaxResponses() const { return _valCxrTaxResponses; }

  void setValCxrTaxResponses(const CarrierCode& valCxr, TaxResponse* taxResponse)
  {
    _valCxrTaxResponses[valCxr].push_back(taxResponse);
  }

  using DefaultValCxrPerSp = std::map<SettlementPlanType, CarrierCode>;
  DefaultValCxrPerSp& defaultValCxrPerSp() { return _defaultValCxrPerSp; }
  const DefaultValCxrPerSp& defaultValCxrPerSp() const { return _defaultValCxrPerSp; }

  using ValCxrCommissionAmount = std::map<CarrierCode, MoneyAmount>;
  const ValCxrCommissionAmount& valCxrCommissionAmount() const { return _valCxrCommAmtCol; }
  ValCxrCommissionAmount& valCxrCommissionAmount() { return _valCxrCommAmtCol; }

  SettlementPlanValCxrsMap& settlementPlanValidatingCxrs() { return _settlementPlanValCxrs; }
  const SettlementPlanValCxrsMap& settlementPlanValidatingCxrs() const
  {
    return _settlementPlanValCxrs;
  }

  MoneyAmount getTaxMoneyAmount(const CarrierCode& cxr);
  std::string& baggageResponse() { return _baggageResponse; }
  const std::string& baggageResponse() const { return _baggageResponse; }

  std::string& greenScreenBaggageResponse() { return _greenScreenBaggageResponse; }
  const std::string& greenScreenBaggageResponse() const { return _greenScreenBaggageResponse; }

  std::string& baggageEmbargoesResponse() { return _baggageEmbargoesResponse; }
  const std::string& baggageEmbargoesResponse() const { return _baggageEmbargoesResponse; }

  JourneyUtil::SegOAndDMap& segmentOAndDMarket() { return _segmentOAndDMarket; }
  const JourneyUtil::SegOAndDMap& segmentOAndDMarket() const { return _segmentOAndDMarket; }

  std::vector<OAndDMarket*>& oAndDMarkets() { return _OAndDMarkets; }
  const std::vector<OAndDMarket*>& oAndDMarkets() const { return _OAndDMarkets; }

  virtual FarePath* clone(DataHandle& dataHandle) const;

  void copyBaggageDataFrom(const FarePath& fp);

  virtual MoneyAmount getNonrefundableAmountInNUC(PricingTrx& trx) const;
  virtual Money getNonrefundableAmount(PricingTrx& trx) const;

  Money getNonrefundableAmountFromCat16(const PricingTrx& trx) const;

  const std::vector<TicketingFeesInfo*>& collectedTktOBFees() const { return _collectedTktOBFees; }
  std::vector<TicketingFeesInfo*>& collectedTktOBFees() { return _collectedTktOBFees; }

  const std::vector<TicketingFeesInfo*>& collectedRTypeOBFee() const
  {
    return _collectedRTypeOBFee;
  }
  std::vector<TicketingFeesInfo*>& collectedRTypeOBFee() { return _collectedRTypeOBFee; }

  const std::vector<TicketingFeesInfo*>& collectedTTypeOBFee() const
  {
    return _collectedTTypeOBFee;
  }
  std::vector<TicketingFeesInfo*>& collectedTTypeOBFee() { return _collectedTTypeOBFee; }

  const TicketingFeesInfo* maximumObFee() const { return _maximumObFee; }
  const TicketingFeesInfo*& maximumObFee() { return _maximumObFee; }

  const bool& endorsementsCollected() const { return _endorsementsCollected; }
  bool& endorsementsCollected() { return _endorsementsCollected; }

  bool isAnyFareUsageAcrossTurnaroundPoint() const;

  bool applyNonIATARounding(const PricingTrx& trx) const;
  bool applyNonIATARounding(const PricingTrx& trx);

  void deactivateNonIATARounding() { _applyNonIATARounding = NO; }
  bool hasMultipleCurrency() const;

  MoneyAmount
  convertToBaseCurrency(const PricingTrx& trx, MoneyAmount mA, const CurrencyCode& curr) const;

  YQYRCalculator* yqyrCalculator() { return _yqyrCalculator; }
  const YQYRCalculator* yqyrCalculator() const { return _yqyrCalculator; }
  void setYqyrCalculator(YQYRCalculator* yqyrCalculator) { _yqyrCalculator = yqyrCalculator; }

  TaxResponse::TaxItemVector& getMutableExternalTaxes() { return _externalTaxes; }
  const TaxResponse::TaxItemVector& getExternalTaxes() const { return _externalTaxes; }
  const std::vector<FarePath*>& gsaClonedFarePaths() const { return _gsaClonedFarePaths; }
  std::vector<FarePath*>& gsaClonedFarePaths() { return _gsaClonedFarePaths; }
  void clearGsaClonedFarePaths();
  FarePath* findTaggedFarePath(const CarrierCode& cxr) const;
  bool forbidCreditCardFOP() const;

  // Copy finalBooking to the cloned FarePaths.
  void propagateFinalBooking();

  const MaxPenaltyResponse* maxPenaltyResponse() const { return _maxPenaltyResponse; }
  MaxPenaltyResponse*& maxPenaltyResponse() { return _maxPenaltyResponse; }

  bool& isAgencyCommissionQualifies() { return _isAgencyCommissionQualifies; }
  const bool& isAgencyCommissionQualifies() const { return _isAgencyCommissionQualifies; }

  std::pair<const FareUsage*, const uint16_t>
  findFUWithPUNumberWithFirstTravelSeg(const TravelSeg* travelSeg) const;

  CarrierCode getValidatingCarrier() const;

protected:
  static std::string getVendorCode(const PricingTrx& trx, const PaxTypeFare& ptf, bool& overrides);

  enum CmdPrcStat : uint8_t
  { CP_UNKNOWN = 0,
    CP_W_WARNING,
    CP_NO_WARNING };

  Itin* _itin = nullptr;
  PaxType* _paxType = nullptr;
  CollectedNegFareData* _collectedNegFareData = nullptr;
  NetRemitFarePath* _netRemitFarePath = nullptr;
  NetFarePath* _netFarePath = nullptr; // for CWT Cat35 NetFare ticketing (NETSELL)
  FarePath* _originalFarePathAxess = nullptr; // for Jal Axess
  FarePath* _axessFarePath = nullptr; // for Jal Axess
  FarePath* _adjustedSellingFarePath = nullptr;
  std::vector<FarePath*> _gsaClonedFarePaths;
  std::string _cat27TourCode;

  std::vector<PricingUnit*> _pricingUnit;

  bool _processed = false;
  bool _intlSurfaceTvlLimit = false;
  bool _plusUpFlag = false;
  bool _selectedNetRemitFareCombo = false;
  bool _tktRestricted = false;
  bool _tfrRestricted = false;
  bool _flownOWFaresCollected = false;
  bool _multipleTourCodeWarning = false;
  bool _isAdjustedSellingFarePath = false;
  bool _regularNet = false;

  union Perm
  {
    Perm() : _31LowestPerm(nullptr) {}
    const ProcessTagPermutation* _31LowestPerm;
    const RefundPermutation* _33LowestPerm;
  } perm;

  MoneyAmount _rexChangeFee = 0.0; // change fee in calculation currency
  MoneyAmount _yqyrNUCAmount = 0.0; // Total YQ/YR amount in calculation currency (usually NUC)
  MoneyAmount _bagChargeNUCAmount = 0.0;

  MoneyAmount _plusUpAmount = 0.0; // aggregate plus-up amount
  mutable MoneyAmount _totalNUCAmount = 0.0; // total amount in NUC's
  MoneyAmount _totalNUCMarkupAmount = 0.0;
  MoneyAmount _unroundedTotalNUCAmount = 0.0; // total amount in NUC's
  MoneyAmount _dynamicPriceDeviationAmount = 0.0;

  MoneyAmount _spanishResidentUpperBoundDiscAmt = 0.0;

  MoneyAmount _commissionAmount = 0.0; // commission amount in payment currency
  Percent _commissionPercent = 0.0; // commission % for cat35

  MoneyAmount _aslMslDiffAmount = 0.0;

  std::vector<TicketEndorseItem> _tktEndorsement; // ticket endorsement

  std::vector<OscPlusUp*> _oscPlusUp;
  std::vector<RscPlusUp*> _rscPlusUp;
  std::vector<FareUsage*> _flownOWFares;

  // struct travelSegsEqual;

  // Currency Adjustment warning MSG
  // ====== ======
  std::string _trailerCurrAdjMsg;

  // Paper Tkt surcharge warning MSG
  // ====== ======
  std::string _paperTktSurcharge;
  bool _ignoreReissueCharges = false;
  bool _rollBackSurcharges = false; // surcharges rolled back to filed fare currency
  Itin::ISICode _intlSaleIndicator = Itin::ISICode::UNKNOWN;

  CmdPrcStat _cmdPrcStat = CmdPrcStat::CP_UNKNOWN;

  int _mileage = 0;
  bool _endorsementsCollected = false;
  bool _isExcTicketHigher = false;
  Indicator _applyNonIATARounding = BLANK;

  Money _nonrefundableAmount{NUC};

  std::vector<TicketingFeesInfo*> _collectedTktOBFees;
  const TicketingFeesInfo* _maximumObFee = nullptr;

  std::vector<TicketingFeesInfo*> _collectedRTypeOBFee;
  std::vector<TicketingFeesInfo*> _collectedTTypeOBFee;

  YQYRCalculator* _yqyrCalculator = nullptr;

private:
  MoneyAmount convertBetweenCurrencies(const PricingTrx& trx,
                                       MoneyAmount ma,
                                       const CurrencyCode& from,
                                       const CurrencyCode& to) const;

  bool isAnyPuCmdPricedWithWarning() const;

  SegToAllowanceTextMap _baggageAllowance;
  std::vector<const BaggageTravel*> _baggageTravels;
  std::vector<const BaggageTravel*> _baggageTravelsPerSector;
  CarrierCode _defaultValidatingCarrier;
  CarrierCode _marketingCxrForDefaultValCxr;
  std::map<SettlementPlanType, CarrierCode> _mktCxrForDefaultValCxrPerSp;
  std::vector<CarrierCode> _validatingCarriers;
  ValCxrTaxResponseMap _valCxrTaxResponseMap;
  ValCxrTaxResponsesMap _valCxrTaxResponses;
  DefaultValCxrPerSp _defaultValCxrPerSp;
  ValCxrCommissionAmount _valCxrCommAmtCol;
  bool _isAgencyCommissionQualifies = false;
  SettlementPlanValCxrsMap _settlementPlanValCxrs;
  std::string _baggageResponse;
  std::string _greenScreenBaggageResponse;
  std::string _baggageEmbargoesResponse;
  JourneyUtil::SegOAndDMap _segmentOAndDMarket;
  std::vector<OAndDMarket*> _OAndDMarkets;
  std::vector<ClassOfService*> _finalBooking;
  bool _fuelSurchargeIgnored = false;
  RexStatus _rexStatus = RexStatus::REX_NOT_PROCESSED;
  CopyablePtr<MostRestrictiveJourneySFRData> _mostRestrictiveJourneyData;

  bool _noMatchOption = false; // WPA CWT project (1S customer)
  bool _rebookClassesExists = false;
  bool _bookingCodeFailButSoftPassForKeepFare = false;
  bool _duplicate = false;
  bool _ignoreSurfaceTPM = false;
  bool _forceCat5Validation = false;

  ReissueCharges* _reissueCharges = nullptr;
  Indicator _reissueVsExchange = BLANK; // QREX/BSP  value '1' - reissue
  // value '2' - exchange
  bool _useSecondRoeDate = false;
  bool _minFareCheckDone = false; // QREX/BSP

  // ******************************** MIP/ASE ********************************* //
  uint16_t _brandIndex = INVALID_BRAND_INDEX;
  BrandCode _brandCode;
  uint16_t _flexFaresGroupId = false;
  std::pair<uint16_t, uint16_t> _brandIndexPair{INVALID_BRAND_INDEX, INVALID_BRAND_INDEX};

  std::pair<Itin*, Itin*> _parentItinPair{nullptr, nullptr};
  TaxResponse::TaxItemVector _externalTaxes;

  MaxPenaltyResponse* _maxPenaltyResponse = nullptr;
};

void inline FarePath::resetTotalNUCAmount(MoneyAmount amt)
{
  _totalNUCAmount = amt;
  _dynamicPriceDeviationAmount = 0;
}

void inline FarePath::rollbackDynamicPriceDeviation()
{
  decreaseTotalNUCAmount(_dynamicPriceDeviationAmount);
  _dynamicPriceDeviationAmount = 0;
}

void inline FarePath::accumulatePriceDeviationAmount(MoneyAmount amt)
{
  increaseTotalNUCAmount(amt);
  _dynamicPriceDeviationAmount += amt;
}
} // tse namespace
