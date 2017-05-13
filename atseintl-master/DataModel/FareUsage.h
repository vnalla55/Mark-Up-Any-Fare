//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Saber.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/Assert.h"
#include "Common/CommissionKeys.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/SmallBitSet.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VCTR.h"
#include "Common/VecMultiMap.h"
#include "Common/VecSet.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/StructuredRuleData.h"
#include "DBAccess/Record2Types.h"
#include "Rules/TicketingEndorsement.h"

#include <bitset>
#include <vector>
#include <memory>

namespace tse
{
class ChargeSODirect;
class DifferentialData;
class NegFareRestExtSeq;
class PricingUnit;
class StopoversInfoSeg;
class SurchargeData;
class TravelSeg;

class FareUsage
{
  friend class PricingResponseFormatterTest;

public:
  class Surcharge
  {
  public:
    Surcharge() = default;

    Surcharge(const MoneyAmount amount,
              const CurrencyCode currencyCode,
              const CurrencyNoDec noDecimals,
              const MoneyAmount unconvertedAmount,
              const CurrencyCode unconvertedCurrencyCode,
              const CurrencyNoDec unconvertedNoDecimals,
              const TravelSeg* travelSeg,
              const bool isSegmentSpecific,
              const uint32_t matchRuleItemNo,
              const VCTR& matchRuleVCTR,
              const bool isCharge1,
              const bool isFirstAmt)
      : _amount(amount),
        _currencyCode(currencyCode),
        _noDecimals(noDecimals),
        _unconvertedAmount(unconvertedAmount),
        _unconvertedCurrencyCode(unconvertedCurrencyCode),
        _unconvertedNoDecimals(unconvertedNoDecimals),
        _travelSeg(travelSeg),
        _isSegmentSpecific(isSegmentSpecific),
        _matchRuleItemNo(matchRuleItemNo),
        _matchRuleVCTR(matchRuleVCTR),
        _isCharge1(isCharge1),
        _isFirstAmt(isFirstAmt),
        _isFromOverride(false)
    {
    }

    virtual ~Surcharge() = default;

    const MoneyAmount& amount() const { return _amount; }
    MoneyAmount& amount() { return _amount; }

    const CurrencyCode& currencyCode() const { return _currencyCode; }
    CurrencyCode& currencyCode() { return _currencyCode; }

    const CurrencyNoDec& noDecimals() const { return _noDecimals; }
    CurrencyNoDec& noDecimals() { return _noDecimals; }

    const MoneyAmount& unconvertedAmount() const { return _unconvertedAmount; }
    MoneyAmount& unconvertedAmount() { return _unconvertedAmount; }

    const CurrencyCode& unconvertedCurrencyCode() const { return _unconvertedCurrencyCode; }
    CurrencyCode& unconvertedCurrencyCode() { return _unconvertedCurrencyCode; }

    const CurrencyNoDec& unconvertedNoDecimals() const { return _unconvertedNoDecimals; }
    CurrencyNoDec& unconvertedNoDecimals() { return _unconvertedNoDecimals; }

    const TravelSeg* travelSeg() const { return _travelSeg; }
    const TravelSeg*& travelSeg() { return _travelSeg; }

    const bool isSegmentSpecific() const { return _isSegmentSpecific; }
    bool& isSegmentSpecific() { return _isSegmentSpecific; }

    const uint32_t matchRuleItemNo() const { return _matchRuleItemNo; }
    uint32_t& matchRuleItemNo() { return _matchRuleItemNo; }

    const VCTR& matchRuleVCTR() const { return _matchRuleVCTR; }
    VCTR& matchRuleVCTR() { return _matchRuleVCTR; }

    const bool isCharge1() const { return _isCharge1; }
    bool& isCharge1() { return _isCharge1; }

    const bool isFirstAmt() const { return _isFirstAmt; }
    bool& isFirstAmt() { return _isFirstAmt; }

    const bool isFromOverride() const { return _isFromOverride; }
    bool& isFromOverride() { return _isFromOverride; }

  protected:
    MoneyAmount _amount = 0;
    CurrencyCode _currencyCode{NUC};
    CurrencyNoDec _noDecimals = 0;
    MoneyAmount _unconvertedAmount = 0;
    CurrencyCode _unconvertedCurrencyCode{NUC};
    CurrencyNoDec _unconvertedNoDecimals = 0;
    const TravelSeg* _travelSeg = nullptr;
    bool _isSegmentSpecific = false;
    uint32_t _matchRuleItemNo = 0;
    VCTR _matchRuleVCTR;
    bool _isCharge1 = true;
    bool _isFirstAmt = true;
    bool _isFromOverride = false;
  };

  class StopoverSurcharge : public Surcharge
  {
  public:
    StopoverSurcharge() = default;
    StopoverSurcharge(const MoneyAmount amount,
                      const CurrencyCode currencyCode,
                      const CurrencyNoDec noDecimals,
                      const MoneyAmount unconvertedAmount,
                      const CurrencyCode unconvertedCurrencyCode,
                      const CurrencyNoDec unconvertedNoDecimals,
                      const TravelSeg* travelSeg,
                      const bool isSegmentSpecific,
                      const uint32_t matchRuleItemNo,
                      const VCTR& matchRuleVCTR,
                      const bool isCharge1,
                      const bool isFirstAmt)
      : Surcharge(amount,
                  currencyCode,
                  noDecimals,
                  unconvertedAmount,
                  unconvertedCurrencyCode,
                  unconvertedNoDecimals,
                  travelSeg,
                  isSegmentSpecific,
                  matchRuleItemNo,
                  matchRuleVCTR,
                  isCharge1,
                  isFirstAmt)
    {
    }

    const bool chargeFromFirstInbound() const { return _chargeFromFirstInbound; }
    bool& chargeFromFirstInbound() { return _chargeFromFirstInbound; }

  protected:
    bool _chargeFromFirstInbound = false;
  };

  typedef VecMultiMap<const TravelSeg*, const StopoverSurcharge*> StopoverSurchargeMultiMap;
  typedef StopoverSurchargeMultiMap::const_iterator StopoverSurchargeMultiMapCI;

  typedef std::map<const StopoversInfoSeg*, ChargeSODirect*> StopoverInfoByDirectionMap;
  typedef StopoverInfoByDirectionMap::const_iterator StopoverInfoByDirectionMapCI;

  class TransferSurcharge : public Surcharge
  {
  public:
    TransferSurcharge() = default;
    TransferSurcharge(const MoneyAmount amount,
                      const CurrencyCode currencyCode,
                      const CurrencyNoDec noDecimals,
                      const MoneyAmount unconvertedAmount,
                      const CurrencyCode unconvertedCurrencyCode,
                      const CurrencyNoDec unconvertedNoDecimals,
                      const TravelSeg* travelSeg,
                      const bool isSegmentSpecific,
                      const uint32_t matchRuleItemNo,
                      const VCTR& matchRuleVCTR,
                      const bool isCharge1,
                      const bool isFirstAmt)
      : Surcharge(amount,
                  currencyCode,
                  noDecimals,
                  unconvertedAmount,
                  unconvertedCurrencyCode,
                  unconvertedNoDecimals,
                  travelSeg,
                  isSegmentSpecific,
                  matchRuleItemNo,
                  matchRuleVCTR,
                  isCharge1,
                  isFirstAmt)
    {
    }
  };

  struct TktNetRemitPscResult
  {
    TktNetRemitPscResult() = default;

    TktNetRemitPscResult(const TravelSeg* startTravelSeg,
                         const TravelSeg* endTravelSeg,
                         const NegFareRestExtSeq* tfdpscSeqNumber,
                         const PaxTypeFare* resultFare)
      : _startTravelSeg(startTravelSeg),
        _endTravelSeg(endTravelSeg),
        _tfdpscSeqNumber(tfdpscSeqNumber),
        _resultFare(resultFare)
    {
    }

    const TravelSeg* _startTravelSeg = nullptr;
    const TravelSeg* _endTravelSeg = nullptr;
    const NegFareRestExtSeq* _tfdpscSeqNumber = nullptr;
    const PaxTypeFare* _resultFare = nullptr;
  };

  // Addding for Validating Carrier Project for checking Fare Amount for
  // FarePath merge
  bool isEqualAmountComponents(const FareUsage& rhs) const;

protected:
  enum FareUsageStatus
  {
    Inbound = 0x0001,
    Outbound = 0x0002,
    NetCat35NucUsed = 0x0004,
    PerTktCharges = 0x0010,
    AppendNR = 0x0020,
    PaperTktSurchargeMayApply = 0x0040,
    PaperTktSurchargeIncluded = 0x0080
  };

public:
  typedef std::vector<TktNetRemitPscResult> TktNetRemitPscResultVec;

  typedef VecMultiMap<const TravelSeg*, const TransferSurcharge*> TransferSurchargeMultiMap;
  typedef TransferSurchargeMultiMap::const_iterator TransferSurchargeMultiMapCI;

  typedef Fare::FopStatus FopStatus;

  //FareUsage() = default;
  FareUsage();
  ~FareUsage() = default;

  bool isPaxTypeFareNormal() const
  {
    TSE_ASSERT(_paxTypeFare);
    return _paxTypeFare->isNormal();
  }

  // access to members

  const PaxTypeFare* paxTypeFare() const { return _paxTypeFare; }
  PaxTypeFare*& paxTypeFare() { return _paxTypeFare; }

  StructuredRuleData& getStructuredRuleData() const
  {
    TSE_ASSERT(_structuredRuleData);
    return *_structuredRuleData;
  }

  void createStructuredRuleDataIfNonexistent()
  {
    if (!_structuredRuleData)
      _structuredRuleData = std::unique_ptr<StructuredRuleData>(new StructuredRuleData);
  }

  bool hasStructuredRuleData() const { return _structuredRuleData != nullptr; }

  bool hasTravelSeg(const TravelSeg* tvlSeg) const;

  void setBrandCode(const BrandCode& value) { _brandCode = value; }
  const BrandCode& getBrandCode() const { return _brandCode; }

  const PaxTypeFare* adjustedPaxTypeFare() const { return _adjustedPaxTypeFare; }
  const PaxTypeFare*& adjustedPaxTypeFare() { return _adjustedPaxTypeFare; }

  const PaxTypeFare* tktNetRemitFare() const { return _tktNetRemitFare; }
  const PaxTypeFare*& tktNetRemitFare() { return _tktNetRemitFare; }

  const PaxTypeFare* tktNetRemitFare2() const { return _tktNetRemitFare2; }
  const PaxTypeFare*& tktNetRemitFare2() { return _tktNetRemitFare2; }

  const TktNetRemitPscResultVec& netRemitPscResults() const { return _netRemitPscResults; }
  TktNetRemitPscResultVec& netRemitPscResults() { return _netRemitPscResults; }

  const CombinabilityRuleInfo* rec2Cat10() const { return _rec2Cat10; }
  CombinabilityRuleInfo*& rec2Cat10() { return _rec2Cat10; }

  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }
  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }

  std::vector<TicketEndorseItem>& tktEndorsement() { return _tktEndorsement; }

  const std::vector<TicketEndorseItem>& tktEndorsement() const { return _tktEndorsement; }

  const std::vector<SurchargeData*>& surchargeData() const { return _surchargeData; }
  std::vector<SurchargeData*>& surchargeData() { return _surchargeData; }

  const VCTR& stopoversMatchingVCTR() const { return _stopoversMatchingVCTR; }
  VCTR& stopoversMatchingVCTR() { return _stopoversMatchingVCTR; }

  const VCTR& stopoversMatchingGeneralRuleVCTR() const { return _stopoversMatchingGeneralRuleVCTR; }
  VCTR& stopoversMatchingGeneralRuleVCTR() { return _stopoversMatchingGeneralRuleVCTR; }

  const VCTR& transfersMatchingVCTR() const { return _transfersMatchingVCTR; }
  VCTR& transfersMatchingVCTR() { return _transfersMatchingVCTR; }

  const VCTR& transfersMatchingGeneralRuleVCTR() const { return _transfersMatchingGeneralRuleVCTR; }
  VCTR& transfersMatchingGeneralRuleVCTR() { return _transfersMatchingGeneralRuleVCTR; }

  const VecSet<const TravelSeg*>& stopovers() const { return _stopovers; }
  VecSet<const TravelSeg*>& stopovers() { return _stopovers; }

  const StopoverSurchargeMultiMap& stopoverSurcharges() const { return _stopoverSurcharges; }
  StopoverSurchargeMultiMap& stopoverSurcharges() { return _stopoverSurcharges; }

  const StopoverInfoByDirectionMap& stopoverByDir() const { return _infoSegsDirCharge; }
  StopoverInfoByDirectionMap& stopoverByDir() { return _infoSegsDirCharge; }

  const VecSet<const TravelSeg*>& transfers() const { return _transfers; }
  VecSet<const TravelSeg*>& transfers() { return _transfers; }

  const TransferSurchargeMultiMap& transferSurcharges() const { return _transferSurcharges; }
  TransferSurchargeMultiMap& transferSurcharges() { return _transferSurcharges; }

  void addSurcharge(const StopoverSurcharge* surcharge, const MoneyAmount nucAmount);
  void addStopoverSurcharge(const StopoverSurcharge& surcharge, const MoneyAmount& nucAmount);
  void addSurcharge(const TransferSurcharge* surcharge, const MoneyAmount nucAmount);

  //TODO: remove - used only in getFareUsageDirection()
  bool isReverseDirection() const {
    return (isInbound() && !dirChangeFromOutbound());
  }

  //TODO: could be removed? used only in XMLShoppingResponse.cpp
  Direction getFareUsageDirection() const {
    return isReverseDirection() ?
        Direction::REVERSED : Direction::ORIGINAL;
  }

  bool isInbound() const { return _inbound; }
  bool isOutbound() const { return !_inbound; }
  bool& inbound() { return _inbound; }
  bool& dirChangeFromOutbound() { return _dirChangeFromOutbound; }
  bool dirChangeFromOutbound() const { return _dirChangeFromOutbound; }
  bool& ignorePTFCmdPrcFailedFlag() { return _ignorePTFCmdPrcFailedFlag; }
  bool ignorePTFCmdPrcFailedFlag() const { return _ignorePTFCmdPrcFailedFlag; }

  const bool& highRT() const { return _highRT; }
  bool& highRT() { return _highRT; }

  const bool isPerTktCharges() const { return _status.isSet(PerTktCharges); }
  void perTktCharges() { _status.set(PerTktCharges); }

  const bool isAppendNR() const { return _status.isSet(AppendNR); }

  const SmallBitSet<uint16_t, FareUsageStatus>& status() const { return _status; }
  SmallBitSet<uint16_t, FareUsageStatus>& status() { return _status; }

  // Currently PaperTktSurchargeMayApply is set without indexing only.
  // Change the implementation when support for indexing is added.
  // Reading is made by index already.

  bool setPaperTktSurchargeMayApply(bool paperTktSurchargeMayApply = true)
  {
    return _status.set(PaperTktSurchargeMayApply, paperTktSurchargeMayApply);
  }

  const bool isPaperTktSurchargeMayApply() const
  {
    return _status.isSet(PaperTktSurchargeMayApply);
  }

  const bool isPaperTktSurchargeIncluded() const
  {
    return _status.isSet(PaperTktSurchargeIncluded);
  }

  void paperTktSurchargeIncluded() { _status.set(PaperTktSurchargeIncluded); }

  const bool isNetCat35NucUsed() const { return _status.isSet(NetCat35NucUsed); }
  void netCat35NucUsed() { _status.set(NetCat35NucUsed); }

  MoneyAmount& surchargeAmt() { return _surchargeAmt; }
  const MoneyAmount surchargeAmt() const { return _surchargeAmt; }
  void accumulateSurchargeAmt(MoneyAmount _amt) { surchargeAmt() += _amt; }

  MoneyAmount& surchargeAmtUnconverted() { return _surchargeAmtUnconverted; }
  const MoneyAmount surchargeAmtUnconverted() const { return _surchargeAmtUnconverted; }
  void accumulateSurchargeAmtUnconverted(MoneyAmount _amt) { surchargeAmtUnconverted() += _amt; }

  MoneyAmount& transferAmt() { return _transferAmt; }
  const MoneyAmount transferAmt() const { return _transferAmt; }
  void accumulateTransferAmt(MoneyAmount _amt) { transferAmt() += _amt; }

  MoneyAmount& transferAmtUnconverted() { return _transferAmtUnconverted; }
  const MoneyAmount transferAmtUnconverted() const { return _transferAmtUnconverted; }
  void accumulateTransferAmtUnconverted(MoneyAmount _amt) { transferAmtUnconverted() += _amt; }

  MoneyAmount& stopOverAmt() { return _stopOverAmt; }
  const MoneyAmount stopOverAmt() const { return _stopOverAmt; }
  void accumulateStopOverAmt(MoneyAmount _amt) { stopOverAmt() += _amt; }

  MoneyAmount& stopOverAmtUnconverted() { return _stopOverAmtUnconverted; }
  const MoneyAmount stopOverAmtUnconverted() const { return _stopOverAmtUnconverted; }
  void accumulateStopOverAmtUnconverted(MoneyAmount _amt) { stopOverAmtUnconverted() += _amt; }

  MoneyAmount& absorptionAdjustment() { return _absorptionAdjustment; }
  const MoneyAmount& absorptionAdjustment() const { return _absorptionAdjustment; }

  MoneyAmount& differentialAmt() { return _differentialAmt; }
  const MoneyAmount differentialAmt() const { return _differentialAmt; }

  MoneyAmount& netCat35NucAmount() { return _netCat35NucAmount; }
  const MoneyAmount& netCat35NucAmount() const { return _netCat35NucAmount; }

  // Fare Discount Amount:
  void setDiscAmount(MoneyAmount amt) { _discAmount = amt; }
  const MoneyAmount getDiscAmount() const { return _discAmount; }

  void setSpanishResidentDiscountAmt(MoneyAmount amt) { _spanishResidentDiscountAmt = amt; }
  const MoneyAmount getSpanishResidentDiscountAmt() const { return _spanishResidentDiscountAmt; }

  bool& isRoundTrip() { return _isRoundTrip; }
  const bool& isRoundTrip() const { return _isRoundTrip; }

  const MoneyAmount totalFareAmount() const;

  AppendageCode& appendageCode() { return _appendageCode; }
  const AppendageCode& appendageCode() const { return _appendageCode; }

  Indicator& penaltyRestInd() { return _penaltyRestInd; }
  Indicator penaltyRestInd() const { return _penaltyRestInd; }

  Indicator& sameMinMaxInd() { return _sameMinMaxInd; }
  const Indicator& sameMinMaxInd() const { return _sameMinMaxInd; }

  bool& isNonRefundable() { return _nonRefundable; }
  bool isNonRefundable() const { return _nonRefundable; }

  const Money& getNonRefundableAmount() const
  {
    return _nonRefundableAmount;
  }

  void setNonRefundableAmount(Money nonRefundableAmount)
  {
    _nonRefundableAmount = nonRefundableAmount;
  }

  Money getNonRefundableAmt(CurrencyCode calculationCurrency,
                            const PricingTrx& trx,
                            bool useInternationalRounding) const;

  bool& changePenaltyApply() { return _changePenaltyApply; }
  bool changePenaltyApply() const { return _changePenaltyApply; }

  int16_t& adjustedStopOvers() { return _adjustedStopOvers; }
  const int16_t& adjustedStopOvers() const { return _adjustedStopOvers; }

  bool isFareBreakHasStopOver() const
  {
    return (!travelSeg().empty() && travelSeg().back()->stopOver());
  }

  const bool& hasSideTrip() const { return _hasSideTrip; }
  bool& hasSideTrip() { return _hasSideTrip; }

  const std::vector<PricingUnit*>& sideTripPUs() const { return _sideTripPUs; }
  std::vector<PricingUnit*>& sideTripPUs() { return _sideTripPUs; }

  // Calculated Maximum Stay date - 1 for all travel segs in PNR
  //
  const DateTime& minStayDate() const { return _minStayDate; }
  DateTime& minStayDate() { return _minStayDate; }

  // Starting travel seg to imprint NVB/NVA dates
  //
  int16_t& startNVBTravelSeg() { return _startNVBTravelSeg; }
  const int16_t& startNVBTravelSeg() const { return _startNVBTravelSeg; }

  // Calculated Maximum Stay date - 1 for all travel segs in PNR
  //
  const DateTime& maxStayDate() const { return _maxStayDate; }
  DateTime& maxStayDate() { return _maxStayDate; }

  const bool& endOnEndRequired() const { return _endOnEndRequired; }
  bool& endOnEndRequired() { return _endOnEndRequired; }

  const std::vector<const CombinabilityRuleItemInfo*>& eoeRules() const { return _eoeRules; }
  std::vector<const CombinabilityRuleItemInfo*>& eoeRules() { return _eoeRules; }

  // booking code & booking code segment status
  // ======= ==== = ======= ==== ======= ======

  bool isBookingCodePass() const { return _bookingCodeStatus.isSet(PaxTypeFare::BKS_PASS); }

  CarrierCode& diffCarrier() { return _diffCarrier; }
  const CarrierCode& diffCarrier() const { return _diffCarrier; }

  uint16_t& differSeqNumber() { return _differSeqNumber; }
  const uint16_t& differSeqNumber() const { return _differSeqNumber; }

  Indicator& calculationInd() { return _calculationInd; }
  const Indicator& calculationInd() const { return _calculationInd; }

  Indicator& hipExemptInd() { return _hipExemptInd; }
  const Indicator& hipExemptInd() const { return _hipExemptInd; }

  int& mixClassStatus() { return _mixClassStatus; }
  const int& mixClassStatus() const { return _mixClassStatus; }

  PaxTypeFare::BookingCodeStatus& bookingCodeStatus() { return _bookingCodeStatus; }
  const PaxTypeFare::BookingCodeStatus& bookingCodeStatus() const { return _bookingCodeStatus; }

  std::vector<PaxTypeFare::SegmentStatus>& segmentStatus() { return _segmentStatus; }
  const std::vector<PaxTypeFare::SegmentStatus>& segmentStatus() const { return _segmentStatus; }

  std::vector<PaxTypeFare::SegmentStatus>& segmentStatusRule2() { return _segmentStatusRule2; }
  const std::vector<PaxTypeFare::SegmentStatus>& segmentStatusRule2() const
  {
    return _segmentStatusRule2;
  }

  const std::vector<DifferentialData*>& differentialPlusUp() const { return _differentialPlusUp; }
  std::vector<DifferentialData*>& differentialPlusUp() { return _differentialPlusUp; }

  const MinFarePlusUp& minFarePlusUp() const { return _minFarePlusUp; }
  MinFarePlusUp& minFarePlusUp() { return _minFarePlusUp; }

  const VecMultiMap<MinimumFareModule, std::string>& minFareUnchecked() const
  {
    return _minFareUnchecked;
  }
  VecMultiMap<MinimumFareModule, std::string>& minFareUnchecked() { return _minFareUnchecked; }

  MoneyAmount& minFarePlusUpAmt() { return _minFarePlusUpAmt; }
  const MoneyAmount& minFarePlusUpAmt() const { return _minFarePlusUpAmt; }
  void accumulateMinFarePlusUpAmt(MoneyAmount _amt) { _minFarePlusUpAmt += _amt; }

  bool isHipProcessed() const { return _hipProcessed; }
  void setHipProcessed() { _hipProcessed = true; } // Should only be set from HIPMinimumFare

  bool& fareDirectionReversed() { return _fareDirectionReversed; }
  bool fareDirectionReversed() const { return _fareDirectionReversed; }

  // cat14 NVA restriction
  void addNVAData(DataHandle& dataHandle, uint16_t segOrder, const DateTime* nvaDate);
  std::map<uint16_t, const DateTime*>* getNVAData() const { return _NVAData; }

  // Return true if fare component covers travel segments from different legs.
  bool isAcrossTurnaroundPoint() const;

  //--- Update this when ever you add new data member----
  //
  FareUsage& operator=(const FareUsage& rhs);

  // Ref-Counting is added ONLY for using in PO so that one
  // FareUseged can be used by many PricingUnit in the PU-PQ
  // and avoid building again and again
  //
  bool decrementRefCount(DataHandle&)
  {
    return (--_refCount == 0);
  }

  void incrementRefCount() { ++_refCount; }

  const TravelSeg* getTktNetRemitTravelSeg(const TravelSeg* travelSeg) const;

  bool isSoftPassed() const { return _ruleSoftPassStatus.isSet(RS_AllCat); }
  void getSoftPassedCategories(std::vector<uint16_t>& categorySequence);

  FareUsage* clone(DataHandle& dataHandle) const;

  bool isFailedFound() const { return _failedFound; }
  void setFailedFound() { _failedFound = true; }
  void resetFailedFound() { _failedFound = false; }

  bool& simultaneousResTkt() { return _simultaneousResTkt; }
  bool simultaneousResTkt() const { return _simultaneousResTkt; }

  std::vector<PaxTypeFare*>& selectedPTFs() { return _selectedPTFs; }
  const std::vector<PaxTypeFare*>& selectedPTFs() const { return _selectedPTFs; }

  const bool& isKeepFare() const { return _isKeepFare; }
  bool& isKeepFare() { return _isKeepFare; }

  bool ruleFailedButSoftPassForKeepFare() const { return _isKeepFare && _ruleFailed; }

  const bool& ruleFailed() const { return _ruleFailed; }
  bool& ruleFailed() { return _ruleFailed; }

  const bool& combinationFailedButSoftPassForKeepFare() const
  {
    return _combinationFailedButSoftPassForKeepFare;
  }
  bool& combinationFailedButSoftPassForKeepFare()
  {
    return _combinationFailedButSoftPassForKeepFare;
  }

  std::set<uint16_t>& categoryIgnoredForKeepFare() { return _categoryIgnoredForKeepFare; }
  const std::set<uint16_t>& categoryIgnoredForKeepFare() const
  {
    return _categoryIgnoredForKeepFare;
  }

  bool& failedCat5InAnotherFu() { return _failedCat5InAnotherFu; }
  const bool failedCat5InAnotherFu() const { return _failedCat5InAnotherFu; }

  // _cat25Fare can be NULL. It will be temporary populated for some rules validation.
  const PaxTypeFare* cat25Fare() const { return _cat25Fare; }
  PaxTypeFare*& cat25Fare() { return _cat25Fare; }

  const bool& exemptMinFare() const { return _exemptMinFare; }
  bool& exemptMinFare() { return _exemptMinFare; }

  FopStatus& mutableForbiddenFop() { return _forbiddenFop; }
  const FopStatus& forbiddenFop() const { return _forbiddenFop; }
  std::string getFopTrailerMsg() const;

  MoneyAmount calculateFareAmount() const;

  TimeAndUnit& stopoverMinTime() { return _stopoverMinTime; }

  const TimeAndUnit& stopoverMinTime() const { return _stopoverMinTime; }

  const flexFares::GroupId& getFlexFaresGroupId() const { return _flexFaresGroupId; }
  void setFlexFaresGroupId(const flexFares::GroupId& groupId) { _flexFaresGroupId = groupId; }

  void copyBkgStatusFromPaxTypeFare();

  bool needRecalculateCat12() const;

  void reuseSurchargeData();

  bool isADDatePassValidation(const DatePair& altDatePair) const;

  bool isSamePaxTypeFare(const FareUsage& rhs) const { return _paxTypeFare == rhs._paxTypeFare; }

  void clearReusedFareUsage();

  std::map<CarrierCode, amc::FcCommissionData*>& fcCommInfoCol() { return _fcCommInfoCol; }
  const std::map<CarrierCode, amc::FcCommissionData*>& fcCommInfoCol() const { return _fcCommInfoCol; }

protected:
  uint32_t _refCount = 1;

  SmallBitSet<uint16_t, FareUsageStatus> _status;

  enum RuleState
  {
    RS_CatNotSupported = 0,
    RS_Cat01 = 0x00000001,
    RS_Cat02 = 0x00000002,
    RS_Cat03 = 0x00000004,
    RS_Cat04 = 0x00000008,
    RS_Cat11 = 0x00000010,
    RS_Cat16 = 0x00000020,
    RS_Cat19 = 0x00000040,
    RS_Cat20 = 0x00000080,
    RS_Cat21 = 0x00000100,
    RS_Cat22 = 0x00000200,
    RS_AllCat = 0x000003FF
  };
  typedef SmallBitSet<uint16_t, RuleState> RuleStatus;
  RuleStatus _ruleSoftPassStatus; // For any reason, categories that we
  // can not do validation at PU scope, we need revalidation at FP scope

  PaxTypeFare* _paxTypeFare = nullptr;
  BrandCode _brandCode;  // leg parity brand for ANY_BRAND_LEG_PARITY path
  const PaxTypeFare* _adjustedPaxTypeFare = nullptr;
  const PaxTypeFare* _tktNetRemitFare = nullptr;
  const PaxTypeFare* _tktNetRemitFare2 = nullptr;
  TktNetRemitPscResultVec _netRemitPscResults;

  std::vector<TravelSeg*> _travelSeg;

  std::vector<TicketEndorseItem> _tktEndorsement;
  std::vector<SurchargeData*> _surchargeData;

  CombinabilityRuleInfo* _rec2Cat10 = nullptr;
  std::unique_ptr<StructuredRuleData> _structuredRuleData;

  VCTR _stopoversMatchingVCTR;
  VCTR _stopoversMatchingGeneralRuleVCTR;
  VCTR _transfersMatchingVCTR;
  VCTR _transfersMatchingGeneralRuleVCTR;

  VecSet<const TravelSeg*> _stopovers;
  StopoverSurchargeMultiMap _stopoverSurcharges;
  StopoverInfoByDirectionMap _infoSegsDirCharge;

  VecSet<const TravelSeg*> _transfers;
  TransferSurchargeMultiMap _transferSurcharges;

  MoneyAmount _surchargeAmt = 0;
  MoneyAmount _surchargeAmtUnconverted = 0;
  MoneyAmount _transferAmt = 0;
  MoneyAmount _transferAmtUnconverted = 0;
  MoneyAmount _stopOverAmt = 0;
  MoneyAmount _stopOverAmtUnconverted = 0;
  MoneyAmount _absorptionAdjustment = 0;
  MoneyAmount _differentialAmt = 0;
  MoneyAmount _minFarePlusUpAmt = 0;
  MoneyAmount _netCat35NucAmount = 0;
  MoneyAmount _discAmount = 0;
  MoneyAmount _spanishResidentDiscountAmt = 0;
  bool _isRoundTrip = false;

  int16_t _adjustedStopOvers = 0;
  bool _hasSideTrip = false;
  std::vector<PricingUnit*> _sideTripPUs;
  bool _inbound = false;
  bool _dirChangeFromOutbound = false;
  bool _highRT = false;

  Indicator _penaltyRestInd{' '};
  AppendageCode _appendageCode;
  Indicator _sameMinMaxInd{' '};
  bool _nonRefundable = true;
  Money _nonRefundableAmount = Money(-1, NUC);
  bool _changePenaltyApply = false; // Set and used during NVA/NVB process

  DateTime _maxStayDate = DateTime::openDate();
  DateTime _minStayDate = DateTime::openDate();

  const RuleItemInfo* _dummy1 = nullptr;
  const RuleItemInfo* _dummy2 = nullptr;
  const RuleItemInfo* _dummy3 = nullptr;
  const RuleItemInfo* _dummy4 = nullptr;

  // Needed for NVA/NVB - Cat 10 Subcat 104
  bool _endOnEndRequired = false;
  std::vector<const CombinabilityRuleItemInfo*> _eoeRules;

  PaxTypeFare::BookingCodeStatus _bookingCodeStatus;
  std::vector<PaxTypeFare::SegmentStatus> _segmentStatus;
  std::vector<PaxTypeFare::SegmentStatus> _segmentStatusRule2;
  std::vector<DifferentialData*> _differentialPlusUp;
  std::vector<char*> _dummy;

  MinFarePlusUp _minFarePlusUp;
  VecMultiMap<MinimumFareModule, std::string> _minFareUnchecked;

  int16_t _startNVBTravelSeg = 0;

  bool _hipProcessed = false;
  int _mixClassStatus = 0;

  // booking code status for Differential
  // ======= ==== ====== === ============

  CarrierCode _diffCarrier;
  uint16_t _differSeqNumber = 0;
  Indicator _calculationInd{' '};
  Indicator _hipExemptInd{' '};

  std::map<uint16_t, const DateTime*>* _NVAData = nullptr;

  bool _failedFound = false;
  bool _simultaneousResTkt = false;

  // selected public PaxTypeFares for the Jal/Axess
  std::vector<PaxTypeFare*> _selectedPTFs;

  // _cat25Fare can be NULL. It will be temporary populated for some rules validation.
  PaxTypeFare* _cat25Fare = nullptr;

  bool _fareDirectionReversed = false;

  // For Cat31 exchange rule validation
  bool _isKeepFare = false;
  bool _ruleFailed = false;
  bool _combinationFailedButSoftPassForKeepFare = false;
  std::set<uint16_t> _categoryIgnoredForKeepFare;
  bool _failedCat5InAnotherFu = false;

  bool _exemptMinFare = false;
  FopStatus _forbiddenFop;

  flexFares::GroupId _flexFaresGroupId = 0;
  bool _ignorePTFCmdPrcFailedFlag = false;
  std::map<CarrierCode, amc::FcCommissionData*> _fcCommInfoCol;

private:
  // Overriddes default value 4h domestic/24h international. Te same value for all segments in FU.
  TimeAndUnit _stopoverMinTime;

  FareUsage(const FareUsage& ref);
};
} // namespace tse
