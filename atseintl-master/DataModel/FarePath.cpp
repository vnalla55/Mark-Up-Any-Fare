//--------------------------------------------------------------------
//-------------------------------------------------------------------
//
//  File:        FarePath.cpp
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

#include "DataModel/FarePath.h"

#include "Common/Assert.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ExchangeUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MaxStayRestriction.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/RuleItemInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/FarePathFactoryStorage.h"
#include "Pricing/FarePathUtils.h"
#include "Pricing/PUPQItem.h"
#include "Pricing/PUPath.h"
#include "Rules/RuleConst.h"
#include "Rules/TicketingEndorsement.h"

#include <algorithm>
#include <functional>
#include <numeric>

namespace tse
{
FALLBACK_DECL(conversionDateSSDSP1154);

using namespace std;

namespace
{
Logger logger("atseintl.DataModel.FarePath");
}

const std::string
FarePath::T225("T225");
const std::string
FarePath::T226("T226");

const CurrencyCode&
FarePath::getCalculationCurrency() const
{
  return calculationCurrency().empty() && itin() ? itin()->calculationCurrency()
                                                 : calculationCurrency();
}

void
FarePath::determineMostRestrTktDT(PricingTrx& trx,
                                  DateTime& latestTktDT,
                                  DateTime& earliestTktDT,
                                  bool& simultaneousResTkt,
                                  DiagCollector* diagPtr) const
{
  DiagCollector& diag = *diagPtr;

  if (diagPtr != nullptr)
  {
    diag.printLine();
    diag << "FARE PATH RES/TKT RULE VALIDATION DIAGNOSTICS" << endl;
    diag << " FARE PATH NUC AMOUNT: " << this->getTotalNUCAmount()
         << " REQUESTED PAXTYPE: " << this->paxType()->paxType() << endl;
  }

  earliestTktDT = DateTime::openDate();
  latestTktDT = DateTime::openDate();

  for (const PricingUnit* const pu : _pricingUnit)
  {
    const DateTime& puEarliestDT = pu->earliestTktDT();
    const DateTime& puLatestDT = pu->latestTktDT();

    if (!puLatestDT.isOpenDate())
    {
      latestTktDT.setWithEarlier(puLatestDT);
    }

    if (!puEarliestDT.isOpenDate())
    {
      earliestTktDT.setWithLater(puEarliestDT);
    }

    if (diagPtr != nullptr)
    {
      diag << "---------------------------------------" << endl;
      diag << *pu;

      diag << " CAT5 LAST TKT DT: " << puLatestDT.toIsoExtendedString() << endl
           << " CAT5 FIRST TKT DT: " << puEarliestDT.toIsoExtendedString() << endl;
    }

    // go throuth the Fare Usages (check latestTktDT from cat15)
    const std::vector<FareUsage*>& puFu = pu->fareUsage();
    for (const FareUsage* const fu : puFu)
    {
      const PaxTypeFare& pFare = *fu->paxTypeFare();
      const DateTime& fuLatestDT = pFare.fare()->latestTktDT();
      if (!fuLatestDT.isOpenDate())
      {
        latestTktDT.setWithEarlier(fuLatestDT);

        if (diagPtr != nullptr)
        {
          diag << " CAT15 LAST TKT DT: " << fuLatestDT.toIsoExtendedString() << endl;
        }
      }

      if (pFare.isFareByRule())
      {
        try
        {
          // check the Cat15 latestTktDate for calculated FBR from the base fare
          const FareByRuleItemInfo* fbrItemInfo =
              dynamic_cast<const FareByRuleItemInfo*>(pFare.getFbrRuleData()->ruleItemInfo());
          if (fbrItemInfo != nullptr && !pFare.isSpecifiedFare())
          {
            PaxTypeFare* bFare = pFare.baseFare(25);
            if (bFare)
            {
              if (!bFare->fare()->latestTktDTFootN().isOpenDate())
              {
                latestTktDT.setWithEarlier(bFare->fare()->latestTktDTFootN());
              }

              Indicator i = fbrItemInfo->ovrdcat15();
              if (i == 'B' || i == ' ')
              {
                // check FareRule/GenRule levels
                if (!bFare->fare()->latestTktDTFareR().isOpenDate())
                {
                  latestTktDT.setWithEarlier(bFare->fare()->latestTktDTFareR());
                }
                if (!bFare->fare()->latestTktDTGenR().isOpenDate())
                {
                  latestTktDT.setWithEarlier(bFare->fare()->latestTktDTGenR());
                }
              }
            }
          }
        }
        catch (...) {}
      }

      simultaneousResTkt |= fu->simultaneousResTkt();
    } // FU
  } // PU

  if (!latestTktDT.isOpenDate())
  {
    const LocCode& tvlLocT = this->itin()->travelSeg().front()->origAirport();
    DateTime localTime = DateTime::localTime();
    const Loc* saleLoc;
    short utcOffsetRES = 0;

    if (trx.getRequest()->PricingRequest::salePointOverride().size())
      saleLoc =
          trx.dataHandle().getLoc(trx.getRequest()->PricingRequest::salePointOverride(), localTime);
    else
      saleLoc = trx.getRequest()->ticketingAgent()->agentLocation();

    const Loc* tvlLoc = trx.dataHandle().getLoc(tvlLocT, localTime);
    if (saleLoc && tvlLoc)
    {
      if (!LocUtil::getUtcOffsetDifference(
              *tvlLoc, *saleLoc, utcOffsetRES, trx.dataHandle(), localTime, localTime))
        utcOffsetRES = 0;
    }
    DateTime travelDate = this->itin()->travelDate().subtractSeconds(utcOffsetRES * 60);

    if (latestTktDT > travelDate)
    {
      if (diagPtr != nullptr)
      {
        diag << " CAT15 LATEST TKT DT IS LATER THAN ITIN DEPART - RESET IT" << endl;
      }
      latestTktDT = DateTime::openDate();
    }
  }

  if (diagPtr != nullptr)
  {
    diag << "---------------------------------------" << endl;

    diag << endl << "FARE PATH "
         << " LAST TKT DT: " << latestTktDT.toIsoExtendedString() << endl
         << "           FIRST TKT DT: " << earliestTktDT.toIsoExtendedString() << endl;
  }
}

MinFarePlusUpItem*
FarePath::minFarePlusUp(MinimumFareModule module, int16_t startSeg, int16_t endSeg) const
{
  std::vector<FarePath::PlusUpInfo*>::const_iterator iter;
  if ((iter = std::find_if(_plusUpInfoList.begin(),
                           _plusUpInfoList.end(),
                           FarePath::PlusUpInfo::KeyEquals(module, startSeg, endSeg))) !=
      _plusUpInfoList.end())
  {
    if (*iter)
      return (*iter)->minFarePlusUp();
  }

  return nullptr;
}

bool
FarePath::isTravelSegPartOfFarePath(const TravelSeg* tvlSeg) const
{
  return std::any_of(_pricingUnit.cbegin(),
                     _pricingUnit.cend(),
                     [tvlSeg](const PricingUnit* pu)
                     { return pu->isTravelSegPartOfPricingUnit(tvlSeg); });
}

const std::string
FarePath::tktFareVendor(const PricingTrx& trx) const
{
  VendorCode fareVendor = "";
  bool isMixedVendor = false;
  bool vendorOverridesOther = false;

  // Identify Fare Vendor/Source for Ticketing
  for (const PricingUnit* const pu : _pricingUnit)
  {
    for (const FareUsage* const fareUsage : pu->fareUsage())
    {
      std::string vendor = getVendorCode(trx, *fareUsage->paxTypeFare(), vendorOverridesOther);

      if (fareVendor.empty())
      {
        if (vendorOverridesOther)
          return vendor;

        fareVendor = vendor;
        continue;
      }

      if (vendor != fareVendor)
      {
        isMixedVendor = true;

        // SMF override ATPCO or SITA
        if (!TrxUtil::optimusNetRemitEnabled(trx) && vendor != ATPCO_VENDOR_CODE &&
            vendor != SITA_VENDOR_CODE &&
            (fareVendor == ATPCO_VENDOR_CODE || fareVendor == SITA_VENDOR_CODE))
        {
          fareVendor = vendor;
        }
      }
    }
  }

  std::string retVendor = fareVendor;
  if (retVendor == ATPCO_VENDOR_CODE) // ATP
    retVendor += "C";

  if (isMixedVendor)
    retVendor += "X";

  return retVendor; // Examples: ATPC, ATPCX, SITA, SITAX, C548, C548X
}

std::string
FarePath::getVendorCode(const PricingTrx& trx, const PaxTypeFare& ptf, bool& overrides)
{
  overrides = false;

  if (ptf.isNegotiated())
  {
    const NegPaxTypeFareRuleData* negPaxTypeFareRuleData = ptf.getNegRuleData();
    if (negPaxTypeFareRuleData)
    {
      if (TrxUtil::optimusNetRemitEnabled(trx) && ptf.vendor() == SMF_ABACUS_CARRIER_VENDOR_CODE &&
          negPaxTypeFareRuleData->fareProperties())
      {
        overrides = true;
        return negPaxTypeFareRuleData->fareProperties()->fareSource();
      }
      else if (!negPaxTypeFareRuleData->creatorPCC().empty())
      {
        return negPaxTypeFareRuleData->creatorPCC();
      }
    }
  }

  return ptf.vendor();
}

bool
FarePath::isAnyPuCmdPricedWithWarning() const
{
  return std::any_of(_pricingUnit.cbegin(),
                     _pricingUnit.cend(),
                     [](const PricingUnit* const pu)
                     { return pu->cmdPricedWFail(); });
}

bool
FarePath::cmdPrcWithWarning()
{
  if (_cmdPrcStat != CP_UNKNOWN)
    return (_cmdPrcStat == CP_W_WARNING);

  if (isAnyPuCmdPricedWithWarning())
  {
    _cmdPrcStat = CP_W_WARNING;
    return true;
  }
  else
  {
    _cmdPrcStat = CP_NO_WARNING;
    return false;
  }
}

FarePath*
FarePath::clone(DataHandle& dataHandle) const
{
  FarePath* res = &dataHandle.safe_create<FarePath>();
  *res = *this;

  return res;
}

void
FarePath::copyBaggageDataFrom(const FarePath& fp)
{
  _baggageAllowance = fp.baggageAllowance();
  _baggageTravels = fp.baggageTravels();
  _baggageTravelsPerSector = fp.baggageTravelsPerSector();
  _baggageResponse = fp.baggageResponse();
  _greenScreenBaggageResponse = fp.greenScreenBaggageResponse();
  _baggageEmbargoesResponse = fp.baggageEmbargoesResponse();
}

void
FarePath::collectBookingCode(const PricingTrx& trx, const FareUsage* fu)
{
  if (fu->travelSeg().size() != fu->segmentStatus().size())
  {
    LOG4CXX_DEBUG(logger, "FarePath skip collect Booking codes");
    return;
  }

  unsigned int index = 0;
  typedef std::vector<PaxTypeFare::SegmentStatus>::const_iterator SegmentStatusVecCI;

  SegmentStatusVecCI cSt = fu->segmentStatus().begin();

  for (std::vector<TravelSeg*>::const_iterator t = fu->travelSeg().begin(),
                                               tend = fu->travelSeg().end();
       t != tend;
       ++t, ++cSt)
  {
    index = (_itin->segmentOrder(*t)) - 1;
    const PaxTypeFare::SegmentStatus& segStat = *cSt;

    if (!bookingCodeRebook[index].empty())
      continue;

    if (trx.getRequest()->isLowFareNoAvailability() && segStat._bkgCodeReBook.empty())
    {
      bookingCodeRebook[index] = (*t)->getBookingCode();
      LOG4CXX_DEBUG(logger, "FarePath BKG rebook " << index << " " << bookingCodeRebook[index]);
    }
    else
    {
      bookingCodeRebook[index] = segStat._bkgCodeReBook;
      LOG4CXX_DEBUG(logger, "FarePath BKG rebook " << index << " " << bookingCodeRebook[index]);
    }
  }
  return;
}

MoneyAmount
FarePath::convertToBaseCurrency(const PricingTrx& trx, MoneyAmount mA, const CurrencyCode& curr)
    const
{
  return convertBetweenCurrencies(trx, mA, curr, itin()->originationCurrency());
}

MoneyAmount
FarePath::convertBetweenCurrencies(const PricingTrx& trx,
                                   MoneyAmount mA,
                                   const CurrencyCode& from,
                                   const CurrencyCode& to) const
{
  if (from == to)
    return mA;
  MoneyAmount baseAmount = 0;
  const Money sourceMoney(mA, from);
  Money targetMoney(baseAmount, to);
  CurrencyConversionFacade ccf;
  ccf.convert(targetMoney, sourceMoney, const_cast<PricingTrx&>(trx), true);
  return targetMoney.value();
}

namespace
{

struct NonRefundableCountingAcc
{
  int operator()(const FareUsage& fu) const { return fu.isNonRefundable() ? 1 : 0; }
  typedef const FareUsage argument_type;
};

struct NonRefundableAmountAcc
{
  NonRefundableAmountAcc(const CurrencyCode& calculationCurrency,
                         const PricingTrx& trx,
                         bool useInternationalRounding,
                         bool useNonRefundable = true)
    : _calculationCurrency(calculationCurrency),
      _trx(trx),
      _useInternationalRounding(useInternationalRounding),
      _useNonRefundable(useNonRefundable)
  {
  }

  MoneyAmount operator()(const FareUsage& fu)
  {
    bool includeFU = _useNonRefundable ? fu.isNonRefundable() : !fu.isNonRefundable();
    return includeFU ? fu.getNonRefundableAmt(_calculationCurrency, _trx, _useInternationalRounding)
                           .value()
                     : 0.0;
  }

private:
  const CurrencyCode _calculationCurrency;
  const PricingTrx& _trx;
  const bool _useInternationalRounding;
  const bool _useNonRefundable;
};

template <typename ResultType, typename Function>
ResultType
accumulateForAllFareUsages(const FarePath& farePath, ResultType acc, Function function)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      acc += function(*fu);
    }
  }
  return acc;
}

} // namespace

MoneyAmount
FarePath::getNonrefundableAmountInNUC(PricingTrx& trx) const
{
  return ExchangeUtil::convertCurrency(
             trx, getNonrefundableAmount(trx), NUC, itin()->useInternationalRounding()).value();
}

Money
FarePath::getNonrefundableAmount(PricingTrx& trx) const
{
  std::shared_ptr<ExchangeUtil::RaiiProcessingDate> dateSetter;

  if (dynamic_cast<RexBaseTrx*>(&trx))
  {
    bool fallback = fallback::conversionDateSSDSP1154(&trx);
    dateSetter = std::make_shared<ExchangeUtil::RaiiProcessingDate>(static_cast<RexBaseTrx&>(trx),
                                                                    *this, fallback);
  }

  const CurrencyCode& calculationCurrency = getCalculationCurrency();
  MoneyAmount amt = accumulateForAllFareUsages(
      *this,
      0.0,
      NonRefundableAmountAcc(calculationCurrency, trx, itin()->useInternationalRounding()));

  return Money(amt, calculationCurrency);
}

void
FarePath::calculateNonrefundableAmount(const FarePath& excFarePath,
                                       RexPricingTrx& trx)
{
  std::shared_ptr<ExchangeUtil::RaiiProcessingDate> dateSetter;

  if (dynamic_cast<RexBaseTrx*>(&trx))
  {
    bool fallback = fallback::conversionDateSSDSP1154(&trx);
    dateSetter = std::make_shared<ExchangeUtil::RaiiProcessingDate>(trx, *this, fallback);
  }
  _nonrefundableAmount = _isExcTicketHigher ?
          trx.exchangeItin().front()->getNonRefAmount() :
          getNonrefundableAmount(trx);

  if (_nonrefundableAmount.isZeroAmount() || _nonrefundableAmount.code() == _baseFareCurrency)
    return;

  _nonrefundableAmount = trx.convertCurrency(_nonrefundableAmount, _baseFareCurrency,
                                             itin()->useInternationalRounding());
}

Money
FarePath::getNonrefundableAmountFromCat16(const PricingTrx& trx) const
{
  Money nonRefundable(0,trx.getRequest()->ticketingAgent()->currencyCodeAgent());

  for (PricingUnit* pu : pricingUnit())
  {
    for(FareUsage* fu : pu->fareUsage())
    {
      nonRefundable = nonRefundable + fu->getNonRefundableAmount();
    }
  }
  return nonRefundable;

}

std::string
FarePath::getNonrefundableMessage() const
{
  std::ostringstream os;
  if (_nonrefundableAmount.value() > EPSILON)
  {
    os << _nonrefundableAmount.toString() << " NONREFUNDABLE";
  }
  return os.str();
}

std::string
FarePath::getNonrefundableMessage2() const
{
  std::ostringstream os;
  if (_nonrefundableAmount.value() > EPSILON)
  {
    if (_isExcTicketHigher)
      os << "EXCHANGE";
    else
      os << "NEW";
    os << " TKT HIGHER NONREFUNDABLE AMOUNT: " << _nonrefundableAmount.toString();
  }
  return os.str();
}

void
FarePath::updateTktEndorsement()
{
  const ProcessTagPermutation* leastExpensivePermutation = lowestFee31Perm();

  if (!leastExpensivePermutation)
    return;

  Indicator endorsmentByte = leastExpensivePermutation->getEndorsementByte();

  if (endorsmentByte == ProcessTagPermutation::ENDORSEMENT_W ||
      endorsmentByte == ProcessTagPermutation::ENDORSEMENT_X)
  {
    _tktEndorsement.clear();
  }

  if (endorsmentByte != ProcessTagPermutation::ENDORSEMENT_Y)
  {
    TicketEndorseItem eItem;
    eItem.priorityCode = 0; // the highest priority; the first line in msg box
    eItem.endorsementTxt = getNonrefundableMessage();
    if (!eItem.endorsementTxt.empty())
      _tktEndorsement.push_back(eItem);
  }
}

Indicator
FarePath::residualPenaltyIndicator(const RexPricingTrx& trx) const
{
  if (!perm._31LowestPerm)
    return ProcessTagPermutation::RESIDUAL_BLANK;

  uint16_t itinIndex = trx.getItinPos(_itin);
  if (trx.processTagPermutations(itinIndex).empty())
    return ProcessTagPermutation::RESIDUAL_BLANK;

  Indicator orgInd = perm._31LowestPerm->getResidualPenaltyByte();

  if (!trx.isSubscriberTrx())
    return orgInd;

  if (orgInd != ProcessTagPermutation::RESIDUAL_N &&
      orgInd != ProcessTagPermutation::RESIDUAL_BLANK)
    return orgInd;

  if (trx.exchangeItin().front()->farePath().empty())
    return orgInd;
  if (!trx.exchangeItin().front()->farePath().front())
    return orgInd;

  FarePath& excFarePath = *trx.exchangeItin().front()->farePath().front();

  int nonRefFU = accumulateForAllFareUsages(excFarePath, 0, NonRefundableCountingAcc());
  if (nonRefFU == 0)
    return ProcessTagPermutation::RESIDUAL_S;

  int refFU = accumulateForAllFareUsages(excFarePath, 0, std::not1(NonRefundableCountingAcc()));
  if (refFU == 0)
    return ProcessTagPermutation::RESIDUAL_I;

  if (orgInd == ProcessTagPermutation::RESIDUAL_BLANK)
  {
    bool internationalRounding = itin()->useInternationalRounding();
    MoneyAmount nonRefAmount = accumulateForAllFareUsages(
                    excFarePath,
                    0.0,
                    NonRefundableAmountAcc(getCalculationCurrency(), trx, internationalRounding)),
                refAmount = accumulateForAllFareUsages(
                    excFarePath,
                    0.0,
                    NonRefundableAmountAcc(
                        getCalculationCurrency(), trx, internationalRounding, false));
    return (refAmount > (nonRefAmount + EPSILON)) ? ProcessTagPermutation::RESIDUAL_S
                                                  : ProcessTagPermutation::RESIDUAL_I;
  }

  return orgInd;
}

bool
FarePath::isAnyFareUsageAcrossTurnaroundPoint() const
{
  return std::any_of(_pricingUnit.cbegin(),
                     _pricingUnit.cend(),
                     [](const PricingUnit* const pu)
                     { return pu->isAnyFareUsageAcrossTurnaroundPoint(); });
}

bool
FarePath::applyNonIATARounding(const PricingTrx& trx) const
{
  return _applyNonIATARounding == YES;
}

bool
FarePath::applyNonIATARounding(const PricingTrx& trx)
{
  if (_applyNonIATARounding != BLANK)
    return _applyNonIATARounding == YES;

  for (const auto pricingUnit : _pricingUnit)
  {
    for (FareUsage* const fareUsage : pricingUnit->fareUsage())
    {
      if (!fareUsage->paxTypeFare()->applyNonIATARounding(trx))
      {
        _applyNonIATARounding = NO;
        return false;
      }
    }
  }

  _applyNonIATARounding = YES;
  return true;
}

bool
FarePath::hasMultipleCurrency() const
{
  bool isFirstCurrencyFound = false;
  for (const auto priceableUnit : _pricingUnit)
  {
    CurrencyCode paxTypeFarecurr;

    if (!priceableUnit)
      continue;

    if (!isFirstCurrencyFound)
    {
      paxTypeFarecurr = priceableUnit->getFirstPaxTypeFareCurrency();
      isFirstCurrencyFound = true;
    }

    if (priceableUnit->hasMultipleCurrency(paxTypeFarecurr))
      return true;
  }
  return false;
}

FarePath*
FarePath::findTaggedFarePath(const CarrierCode& cxr) const
{
  for (FarePath* fp : _gsaClonedFarePaths)
  {
    std::vector<CarrierCode>& valCxrs = fp->validatingCarriers();
    if (std::find(valCxrs.begin(), valCxrs.end(), cxr) != valCxrs.end())
      return fp;
  }
  return nullptr;
}

bool
FarePath::forbidCreditCardFOP() const
{
  for (const PricingUnit* pu : _pricingUnit)
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fu->forbiddenFop().isSet(Fare::FOP_CREDIT))
        return true;
      if (UNLIKELY(fu->paxTypeFare()->fare()->forbiddenFop().isSet(Fare::FOP_CREDIT)))
        return true;
    }
  }
  return false;
}

void
FarePath::clearGsaClonedFarePaths()
{
  if (!_gsaClonedFarePaths.empty())
  {
    const CarrierCode& firstTaggedValCxr =
      _gsaClonedFarePaths.front()->_validatingCarriers.front();

    std::vector<CarrierCode>::iterator it =
      std::find(_validatingCarriers.begin(), _validatingCarriers.end(), firstTaggedValCxr);

    if (it != _validatingCarriers.end())
      _validatingCarriers.erase(it, _validatingCarriers.end());

    _gsaClonedFarePaths.clear();
  }
}

void FarePath::propagateFinalBooking()
{
  for (FarePath* clonedFarePath : _gsaClonedFarePaths)
  {
    clonedFarePath->mutableFinalBooking() = _finalBooking;
  }
}

std::pair<const FareUsage*, const uint16_t>
FarePath::findFUWithPUNumberWithFirstTravelSeg(const TravelSeg* travelSeg) const
{
  uint16_t pricingUnitNumber = 1;

  for (const PricingUnit* pricingUnit : _pricingUnit)
  {
    const auto fareUsage = pricingUnit->getFareUsageWithFirstTravelSeg(travelSeg);
    if (fareUsage != nullptr)
      return std::make_pair(fareUsage, pricingUnitNumber);

    const FareUsage* fu(nullptr);
    for (const PricingUnit* sideTripPu : pricingUnit->sideTripPUs())
    {
      if ((fu = sideTripPu->getFareUsageWithFirstTravelSeg(travelSeg)))
      {
        auto puIt = std::find(_pricingUnit.begin(), _pricingUnit.end(), sideTripPu);
        const uint16_t puNumber = std::distance(_pricingUnit.begin(), puIt) + 1;
        return std::make_pair(fu, puNumber);
      }
    }

    ++pricingUnitNumber;
  }
  return std::make_pair(nullptr, 0);
}

MoneyAmount
FarePath::getTaxMoneyAmount(const CarrierCode& cxr)
{
  Money taxTotal(NUC);
  ValCxrTaxResponseMap::iterator it = _valCxrTaxResponseMap.find(cxr);
  if (it != _valCxrTaxResponseMap.end())
    it->second->getTaxRecordTotal(taxTotal);
  else
  {
    for (FarePath* cloneFp : _gsaClonedFarePaths)
    {
      ValCxrTaxResponseMap& taxMap = cloneFp->valCxrTaxResponseMap();
      it = taxMap.find(cxr);
      if (it != taxMap.end())
      {
        it->second->getTaxRecordTotal(taxTotal);
        break;
      }
    }
  }
  return taxTotal.value();
}

bool
FarePath::needRecalculateCat12() const
{
  return std::any_of(_pricingUnit.begin(),
                     _pricingUnit.end(),
                     [](const PricingUnit* pu)
                     { return pu->needRecalculateCat12(); });
}

void
FarePath::reuseSurchargeData() const
{
  for (const auto pu : _pricingUnit)
    pu->reuseSurchargeData();
}

bool
FarePath::isEqualAmountComponents(const FarePath& rhs) const
{
  return _pricingUnit.size() == rhs._pricingUnit.size() &&
         std::equal(_pricingUnit.cbegin(),
                    _pricingUnit.cend(),
                    rhs._pricingUnit.cbegin(),
                    [](const PricingUnit* const lhs, const PricingUnit* const rhs)
                    { return lhs->isEqualAmountComponents(*rhs); });
}

FarePath*
FarePath::clone(PUPath* puPath, FarePathFactoryStorage& storage)
{
  FarePath* newFp = &storage.constructFarePath();
  *newFp = *this;
  newFp->validatingCarriers().clear();
  newFp->pricingUnit().clear();

  for (PricingUnit* i : _pricingUnit)
  {
    // PricingUnit from mem pool
    PricingUnit* newPu = &storage.constructPricingUnit();
    *newPu = *i;

    newPu->fareUsage().clear();
    for (FareUsage* j : i->fareUsage())
    {
      // PricingUnit from mem pool
      FareUsage* newFu = &storage.constructFareUsage();
      *newFu = *j;
      newFu->surchargeData() = j->surchargeData();
      newPu->fareUsage().push_back(newFu);
    }
    newFp->pricingUnit().push_back(newPu);
  }

  farepathutils::clearMainTripSideTripLink(*newFp);
  farepathutils::addMainTripSideTripLink(*newFp, puPath);
  farepathutils::copyPUPathEOEInfo(*newFp, puPath);

  return newFp;
}

/* Return true when there is difference in commissions
 * A difference exists if a validating carrier exists but we failed
 * to collect commission for it
 */
bool
FarePath::doesValCarriersHaveDiffComm(const CarrierCode& dcx) const
{
  if (!_valCxrCommAmtCol.empty())
  {
    bool noCommForDcx =
      !dcx.empty() &&
      _valCxrCommAmtCol.find(dcx) == _valCxrCommAmtCol.end();

    if (noCommForDcx)
      return true;

    bool noCommForAcx = false;
    if (!_validatingCarriers.empty())
    {
      for (const CarrierCode& cxr : _validatingCarriers)
      {
        if (_valCxrCommAmtCol.find(cxr) == _valCxrCommAmtCol.end())
        {
          noCommForAcx = true;
          break;
        }
      }
    }

    if (noCommForAcx)
      return true;

    if (_valCxrCommAmtCol.size() > 1)
    {
      MoneyAmount commAmt = _valCxrCommAmtCol.begin()->second;
      bool diffCommAmt = std::find_if(_valCxrCommAmtCol.begin(), _valCxrCommAmtCol.end(),
          [commAmt] (const std::pair<CarrierCode, MoneyAmount>& p)->bool
          { return abs(p.second-commAmt) > EPSILON; }) != _valCxrCommAmtCol.end();

      if (diffCommAmt)
        return true;
    }
  }
  return false;
}

MoneyAmount
FarePath::getDynamicPriceDeviationForLeg(int16_t legId) const
{
  auto op = [legId](MoneyAmount sum, const PricingUnit* pu)
  { return sum += pu->getDynamicPriceDeviationForLeg(legId); };

  return std::accumulate(_pricingUnit.cbegin(), _pricingUnit.cend(), MoneyAmount(0), op);
}

CarrierCode
FarePath::getValidatingCarrier() const
{
  return _validatingCarriers.empty()
             ? _itin->validatingCarrier()
             : _validatingCarriers.front();
}
} // tse namespace
