//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "DataModel/FareUsage.h"

#include "Common/Assert.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/TravelSeg.h"

#include <algorithm>

namespace tse
{
class PricingTrx;

FIXEDFALLBACK_DECL(validateAllCat16Records);
FIXEDFALLBACK_DECL(keepFareCombinationsFixed);

//TODO remove with validateAllCat16Records
FareUsage::FareUsage()
  : _nonRefundable(!fallback::fixed::validateAllCat16Records())
{
}

FareUsage::FareUsage(const FareUsage& ref)
{
  *this = ref;
}

FareUsage&
FareUsage::
operator=(const FareUsage& rhs)
{
  if (UNLIKELY(this == &rhs))
    return *this;

  _status = rhs._status;
  _ruleSoftPassStatus = rhs._ruleSoftPassStatus;
  _paxTypeFare = rhs._paxTypeFare;
  _brandCode = rhs._brandCode;
  _adjustedPaxTypeFare = rhs._adjustedPaxTypeFare;
  _tktNetRemitFare = rhs._tktNetRemitFare;
  _tktNetRemitFare2 = rhs._tktNetRemitFare2;
  _netRemitPscResults = rhs._netRemitPscResults;

  _travelSeg.insert(_travelSeg.end(), rhs._travelSeg.begin(), rhs._travelSeg.end());

  _tktEndorsement.insert(
      _tktEndorsement.end(), rhs._tktEndorsement.begin(), rhs._tktEndorsement.end());

  _stopoversMatchingVCTR = rhs._stopoversMatchingVCTR;
  _stopoversMatchingGeneralRuleVCTR = rhs._stopoversMatchingGeneralRuleVCTR;
  _transfersMatchingVCTR = rhs._transfersMatchingVCTR;
  _transfersMatchingGeneralRuleVCTR = rhs._transfersMatchingGeneralRuleVCTR;

  _stopovers.insert(rhs._stopovers.begin(), rhs._stopovers.end());

  _transfers.insert(rhs._transfers.begin(), rhs._transfers.end());

  _stopoverSurcharges.insert(rhs._stopoverSurcharges.begin(), rhs._stopoverSurcharges.end());

  _infoSegsDirCharge.insert(rhs._infoSegsDirCharge.begin(), rhs._infoSegsDirCharge.end());

  _transferSurcharges.insert(rhs._transferSurcharges.begin(), rhs._transferSurcharges.end());

  _rec2Cat10 = rhs._rec2Cat10;

  if (rhs._structuredRuleData != nullptr)
    _structuredRuleData =
        std::unique_ptr<StructuredRuleData>(new StructuredRuleData(*rhs._structuredRuleData));

  _surchargeAmt = rhs._surchargeAmt;
  _transferAmt = rhs._transferAmt;
  _stopOverAmt = rhs._stopOverAmt;

  _absorptionAdjustment = rhs._absorptionAdjustment;

  _adjustedStopOvers = rhs._adjustedStopOvers;

  _differentialAmt = rhs._differentialAmt;
  _minFarePlusUpAmt = rhs._minFarePlusUpAmt;
  _netCat35NucAmount = rhs._netCat35NucAmount;
  _discAmount = rhs._discAmount;
  _spanishResidentDiscountAmt = rhs._spanishResidentDiscountAmt;
  _isRoundTrip = rhs._isRoundTrip;

  _hasSideTrip = rhs._hasSideTrip;
  _inbound = rhs._inbound;
  _dirChangeFromOutbound = rhs._dirChangeFromOutbound;
  _highRT = rhs._highRT;

  _penaltyRestInd = rhs._penaltyRestInd;
  _appendageCode = rhs._appendageCode;
  _sameMinMaxInd = rhs._sameMinMaxInd;
  _nonRefundable = rhs._nonRefundable;
  _nonRefundableAmount = rhs._nonRefundableAmount;
  _changePenaltyApply = rhs._changePenaltyApply;

  _minStayDate = rhs._minStayDate;
  _startNVBTravelSeg = rhs._startNVBTravelSeg;
  _NVAData = rhs._NVAData;

  _maxStayDate = rhs._maxStayDate;

  _bookingCodeStatus = rhs._bookingCodeStatus;

  _segmentStatus.insert(_segmentStatus.end(), rhs._segmentStatus.begin(), rhs._segmentStatus.end());

  _segmentStatusRule2.insert(
      _segmentStatusRule2.end(), rhs._segmentStatusRule2.begin(), rhs._segmentStatusRule2.end());

  _differentialPlusUp.insert(
      _differentialPlusUp.end(), rhs._differentialPlusUp.begin(), rhs._differentialPlusUp.end());

  _endOnEndRequired = rhs._endOnEndRequired;
  if (_endOnEndRequired)
    _eoeRules.insert(_eoeRules.end(), rhs._eoeRules.begin(), rhs._eoeRules.end());

  _selectedPTFs.insert(_selectedPTFs.end(), rhs._selectedPTFs.begin(), rhs._selectedPTFs.end());

  _isKeepFare = rhs._isKeepFare;
  _ruleFailed = rhs._ruleFailed;

  _simultaneousResTkt = rhs._simultaneousResTkt;
  _failedCat5InAnotherFu = rhs._failedCat5InAnotherFu;
  _forbiddenFop = rhs._forbiddenFop;

  _flexFaresGroupId = rhs._flexFaresGroupId;
  _ignorePTFCmdPrcFailedFlag = rhs._ignorePTFCmdPrcFailedFlag;

  if (!fallback::fixed::keepFareCombinationsFixed())
    _combinationFailedButSoftPassForKeepFare = rhs._combinationFailedButSoftPassForKeepFare;

  return *this;
}

std::string
FareUsage::getFopTrailerMsg() const
{
  static constexpr auto fopNames = {std::make_pair(Fare::FOP_CASH, "CASH"),
                                    std::make_pair(Fare::FOP_CHECK, "CK"),
                                    std::make_pair(Fare::FOP_CREDIT, "CC"),
                                    std::make_pair(Fare::FOP_GTR, "GTR")};

  std::string msg;

  for (const auto fopName : fopNames)
  {
    if (!_forbiddenFop.isSet(fopName.first))
      continue;
    if (!msg.empty())
      msg += "/";
    msg += fopName.second;
  }

  if (!msg.empty())
    msg = "WHEN TICKETING FOP MUST NOT BE " + msg;

  return msg;
}

const MoneyAmount
FareUsage::totalFareAmount() const
{
  MoneyAmount amount = _surchargeAmt;
  amount += _transferAmt;
  amount += _stopOverAmt;
  amount += _differentialAmt;
  amount += _minFarePlusUpAmt;
  amount += _paxTypeFare->mileageSurchargeAmt();
  amount -= getSpanishResidentDiscountAmt();
  if (UNLIKELY(isNetCat35NucUsed()))
  {
    amount += _netCat35NucAmount;
  }
  else
  {
    if (UNLIKELY(_adjustedPaxTypeFare != nullptr)) // Currency Adjustment processing
    {
      amount += _adjustedPaxTypeFare->nucFareAmount();
    }
    else // Normal way
    {
      amount += isRoundTrip() ? (_paxTypeFare->nucFareAmount() * 2) : _paxTypeFare->nucFareAmount();
    }
    // Account for the DA/DP discount:
    amount -= getDiscAmount();
  }

  return amount;
}

class NonRefAmount
{
public:
  NonRefAmount(CurrencyCode calculationCurrency,
               const PricingTrx& trx,
               bool useInternationalRounding)
    : _calculationCurrency(calculationCurrency),
      _trx(trx),
      _useInternationalRounding(useInternationalRounding)
  {
  }

  MoneyAmount getNonRefAmountOld(const FareUsage& fu) const
  {
    Money amtFromNUC = ExchangeUtil::convertCurrency(_trx,
                                                     Money(getAmtAvailableOnlyInNUC(fu), NUC),
                                                     _calculationCurrency,
                                                     _useInternationalRounding);
    return fu.paxTypeFare()->nucFareAmount() +
        amtFromNUC.value() + surchargeAmt(fu) +
        unconvertedAmountSum(fu.stopoverSurcharges()) +
        unconvertedAmountSum(fu.transferSurcharges());
  }

  Money getNonRefAmount(const FareUsage& fu) const
  {
    MoneyAmount amount = fu.paxTypeFare()->nucFareAmount() +
                         fu.surchargeAmt() +
                         fu.stopOverAmt() +
                         fu.transferAmt();

    Money amtFromNUC = Money(getAmtAvailableOnlyInNUC(fu), NUC);
    if (_calculationCurrency != NUC)
    {
      amtFromNUC = ExchangeUtil::convertCurrency(_trx,
                                                 amtFromNUC,
                                                 _calculationCurrency,
                                                 _useInternationalRounding);
    }
    else
    {
      amount += fu.differentialAmt();
    }

    amount += amtFromNUC.value();
    amount -= fu.getSpanishResidentDiscountAmt();
    amount -= fu.getDiscAmount();

    return Money(amount, _calculationCurrency);
  }

private:
  MoneyAmount getAmtAvailableOnlyInNUC(const FareUsage& fu) const
  {
    return fu.paxTypeFare()->mileageSurchargeAmt() + fu.minFarePlusUp().getTotalSum();
  }

  MoneyAmount surchargeAmt(const FareUsage& fu) const
  {
    MoneyAmount amt = 0.0;
    for (const auto surchargeData : fu.surchargeData())
      amt += surchargeData->amountSelected();
    return amt;
  }

  template <typename T>
  MoneyAmount unconvertedAmountSum(const T& cont) const
  {
    MoneyAmount amt = 0.0;
    for (const auto& elem : cont)
      amt += elem.second->unconvertedAmount();
    return amt;
  }

  const CurrencyCode _calculationCurrency;
  const PricingTrx& _trx;
  const bool _useInternationalRounding;
};

Money
FareUsage::getNonRefundableAmt(CurrencyCode calculationCurrency,
                               const PricingTrx& trx,
                               bool useInternationalRounding) const
{
    NonRefAmount nonRefCalculator(calculationCurrency, trx, useInternationalRounding);
    return nonRefCalculator.getNonRefAmount(*this);
}

void
FareUsage::addSurcharge(const StopoverSurcharge* surcharge, const MoneyAmount nucAmount)
{
  if (surcharge && stopoverSurcharges().find(surcharge->travelSeg()) == stopoverSurcharges().end())
    addStopoverSurcharge(*surcharge, nucAmount);
}

void
FareUsage::addStopoverSurcharge(const StopoverSurcharge& surcharge, const MoneyAmount& nucAmount)
{
  StopoverSurchargeMultiMap::value_type value(surcharge.travelSeg(), &surcharge);
  stopoverSurcharges().insert(value);
  stopOverAmt() += nucAmount;
}

void
FareUsage::addSurcharge(const TransferSurcharge* surcharge, const MoneyAmount nucAmount)
{
  if (LIKELY(surcharge))
  {
    if (transferSurcharges().find(surcharge->travelSeg()) == transferSurcharges().end())
    {
      TransferSurchargeMultiMap::value_type value(surcharge->travelSeg(), surcharge);
      transferSurcharges().insert(value);

      transferAmt() += nucAmount;
    }
  }
}

bool
FareUsage::hasTravelSeg(const TravelSeg* tvlSeg) const
{
  return std::find(_travelSeg.cbegin(), _travelSeg.cend(), tvlSeg) != _travelSeg.cend();
}

void
FareUsage::addNVAData(DataHandle& dataHandle, uint16_t segOrder, const DateTime* nvaDate)
{
  if (!_NVAData)
  {
    dataHandle.get(_NVAData);
  }

  if (UNLIKELY(!_NVAData))
    return;

  (*_NVAData)[segOrder] = nvaDate;
}

const TravelSeg*
FareUsage::getTktNetRemitTravelSeg(const TravelSeg* travelSeg) const
{
  const TravelSeg* tktNetRemitTravelSeg = nullptr;

  if ((travelSeg == nullptr) || (_tktNetRemitFare == nullptr))
    return tktNetRemitTravelSeg;

  if (_tktNetRemitFare2 != nullptr) // Recurring segments
  {
    if (_travelSeg.front() == travelSeg)
      tktNetRemitTravelSeg = _tktNetRemitFare->fareMarket()->travelSeg().front();
    else if (_travelSeg.back() == travelSeg)
      tktNetRemitTravelSeg = _tktNetRemitFare2->fareMarket()->travelSeg().front();
  }
  else
  {
    if (_paxTypeFare->fareMarket() == _tktNetRemitFare->fareMarket())
      tktNetRemitTravelSeg = travelSeg;
    else
    {
      if (_travelSeg.back() == travelSeg)
        tktNetRemitTravelSeg = _tktNetRemitFare->fareMarket()->travelSeg().front();
    }
  }

  return tktNetRemitTravelSeg;
}

FareUsage*
FareUsage::clone(DataHandle& dataHandle) const
{
  FareUsage* res = dataHandle.create<FareUsage>();
  *res = *this;

  // Copy Surcharges since it is not done in operator =.
  // Shopping uses this clone function.
  res->surchargeData() = _surchargeData;
  res->minFarePlusUp() = _minFarePlusUp;

  return res;
}

void
FareUsage::getSoftPassedCategories(std::vector<uint16_t>& categorySequence)
{
  if (_ruleSoftPassStatus.isSet(RS_Cat01))
    categorySequence.push_back(1);
  else if (_ruleSoftPassStatus.isSet(RS_Cat02))
    categorySequence.push_back(2);
  else if (_ruleSoftPassStatus.isSet(RS_Cat03))
    categorySequence.push_back(3);
  else if (_ruleSoftPassStatus.isSet(RS_Cat04))
    categorySequence.push_back(4);
  else if (_ruleSoftPassStatus.isSet(RS_Cat11))
    categorySequence.push_back(11);
  else if (_ruleSoftPassStatus.isSet(RS_Cat16))
    categorySequence.push_back(16);
  else if (_ruleSoftPassStatus.isSet(RS_Cat19))
    categorySequence.push_back(19);
  else if (_ruleSoftPassStatus.isSet(RS_Cat20))
    categorySequence.push_back(20);
  else if (_ruleSoftPassStatus.isSet(RS_Cat21))
    categorySequence.push_back(21);
  else if (_ruleSoftPassStatus.isSet(RS_Cat22))
    categorySequence.push_back(22);
}

bool
FareUsage::isAcrossTurnaroundPoint() const
{
  int legId = -1;
  for (const TravelSeg* seg : travelSeg())
  {
    if (legId >= 0 && seg->legId() != legId)
      return true;
    legId = seg->legId();
  }
  return false;
}

MoneyAmount
FareUsage::calculateFareAmount() const
{
  MoneyAmount amount;

  if (LIKELY(adjustedPaxTypeFare() == nullptr))
  {
    amount = paxTypeFare()->totalFareAmount();
  }
  else
  {
    amount = adjustedPaxTypeFare()->totalFareAmount();
  }

  return amount - getDiscAmount() - absorptionAdjustment() + minFarePlusUp().getSum(HIP)
      - getSpanishResidentDiscountAmt();
}

bool
FareUsage::isEqualAmountComponents(const FareUsage& rhs) const
{
  return (fabs(_surchargeAmt - rhs.surchargeAmt()) < EPSILON &&
          fabs(_surchargeAmtUnconverted - rhs.surchargeAmtUnconverted()) < EPSILON &&
          fabs(_transferAmt - rhs.transferAmt()) < EPSILON &&
          fabs(_transferAmtUnconverted - rhs.transferAmtUnconverted()) < EPSILON &&
          fabs(_stopOverAmt - rhs.stopOverAmt()) < EPSILON &&
          fabs(_stopOverAmtUnconverted - rhs.stopOverAmtUnconverted()) < EPSILON &&
          fabs(_absorptionAdjustment - rhs.absorptionAdjustment()) < EPSILON &&
          fabs(_differentialAmt - rhs.differentialAmt()) < EPSILON &&
          fabs(_minFarePlusUpAmt - rhs.minFarePlusUpAmt()) < EPSILON &&
          fabs(_netCat35NucAmount - rhs.netCat35NucAmount()) < EPSILON &&
          fabs(_discAmount - rhs.getDiscAmount()) < EPSILON &&
          fabs(_spanishResidentDiscountAmt - rhs.getSpanishResidentDiscountAmt()) < EPSILON);
}
void
FareUsage::copyBkgStatusFromPaxTypeFare()
{
  bookingCodeStatus() = paxTypeFare()->bookingCodeStatus();

  if (segmentStatus().empty())
    segmentStatus().insert(segmentStatus().end(),
                           paxTypeFare()->segmentStatus().begin(),
                           paxTypeFare()->segmentStatus().end());
  if (segmentStatusRule2().empty())
    segmentStatusRule2().insert(segmentStatusRule2().end(),
                                paxTypeFare()->segmentStatusRule2().begin(),
                                paxTypeFare()->segmentStatusRule2().end());
}

bool
FareUsage::needRecalculateCat12() const
{
  TSE_ASSERT(_paxTypeFare);
  return (_paxTypeFare->needRecalculateCat12() ||
          _paxTypeFare->isCategorySoftPassed(RuleConst::SURCHARGE_RULE));
}

void
FareUsage::reuseSurchargeData()
{
  TSE_ASSERT(_paxTypeFare);
  if (!_paxTypeFare->needRecalculateCat12())
    if (!_paxTypeFare->surchargeData().empty())
      _surchargeData = _paxTypeFare->surchargeData();
}

bool
FareUsage::isADDatePassValidation(const DatePair& altDatePair) const
{
  return _paxTypeFare->getAltDatePass(altDatePair);
}

void
FareUsage::clearReusedFareUsage()
{
  _penaltyRestInd = ' ';
  _sameMinMaxInd = ' ';
  _appendageCode.clear();
  _nonRefundable = !fallback::fixed::validateAllCat16Records();;
  _netRemitPscResults.clear();
}

} // namespace tse
