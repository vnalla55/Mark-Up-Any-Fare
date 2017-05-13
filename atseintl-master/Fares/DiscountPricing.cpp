//-------------------------------------------------------------------
//
//  File:        DiscountPricing.cpp
//  Created:     April 14, 2005
//  Design:      Quan Ta
//  Authors:
//
//  Description: DA and DP Discount Processing
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein is the
//          property of Sabre. The program(s) may be used and/or
//          copied only with the written permission of Sabre or in
//          accordance with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have been
//          supplied.
//
//-------------------------------------------------------------------
#include "Fares/DiscountPricing.h"

#include "Common/Assert.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "Util/BranchPrediction.h"

#include <cmath>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.Fares.DiscountPricing");

using OscPlusUp = FarePath::OscPlusUp;
using OscPlusUpList = std::vector<OscPlusUp*>;
using RscPlusUp = FarePath::RscPlusUp;
using RscPlusUpList = std::vector<RscPlusUp*>;
using PlusUpInfo = FarePath::PlusUpInfo;
using PlusUpInfoList = std::vector<PlusUpInfo*>;
using PricingUnitList = std::vector<PricingUnit*>;

DiscountPricing::DiscountPricing(const PricingTrx& trx, FarePath& farePath)
  : _trx(trx),
    _farePath(farePath),
    _isCETrx(_trx.excTrxType() == PricingTrx::PORT_EXC_TRX &&
             ((static_cast<const ExchangePricingTrx&>(_trx)).reqType() == TAG_10_EXCHANGE))
{
}

void
DiscountPricing::processOld() const
{
  const PricingRequest& request = *_trx.getRequest();

  bool isDAEntry = request.isDAEntry();
  bool isDPEntry = request.isDPEntry();

  if (LIKELY(!isDAEntry && !isDPEntry))
    return;

  processDP();
  const DiscAmounts& discAmounts = request.getDiscountAmounts();
  if (!discAmounts.empty())
    processDA(discAmounts);
}

void
DiscountPricing::process() const
{
  _farePath.rollbackDynamicPriceDeviation();
  processDPorPP();
  processDAorPA();
}

void
DiscountPricing::processDA(const DiscAmounts& discAmounts) const
{
  for (const auto& discAmount : discAmounts)
  {
    // collect the effected fare components:
    std::vector<FareUsage*> fcList;
    MoneyAmount totalAmount = collectDiscFareComps(discAmount, fcList);

    // calculate the applicable discount amount, i.e. after apply all required
    // currency conversion.
    MoneyAmount applDiscAmount = calcApplicableDiscAmount(discAmount);

    LOG4CXX_DEBUG(logger,
                  "DA Amount: " << discAmount.amount << ", APPL Amount: " << applDiscAmount);

    MoneyAmount diff = (applDiscAmount - totalAmount);

    if ((diff > EPSILON))
    {
      LOG4CXX_ERROR(logger,
                    "Disc. amount too large: " << applDiscAmount
                                               << " (Total Amount: " << totalAmount << ")")

      throw ErrorResponseException(
          ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED,
          "DISCOUNT EXCEED FARE COMPONENT TOTAL");
    }

    // apply the fare discount - the discount will be stored on the fare usage
    std::for_each(
        fcList.begin(), fcList.end(), ApplyFareDiscount(_farePath, applDiscAmount, totalAmount, *this));
  }
}

void
DiscountPricing::processDP() const
{
  const PricingRequest& request = *_trx.getRequest();

  for (const PricingUnit* const pricingUnit : _farePath.pricingUnit())
  {
    for (FareUsage* const fareUsage : pricingUnit->fareUsage())
    {
      if (_isCETrx && fareUsage->paxTypeFare()->isDummyFare())
        continue; // ignore DP/DA for unchanged FC in CE

      const TravelSeg* firstNoArunkSeg = TravelSegUtil::firstNoArunkSeg(fareUsage->travelSeg());
      if (!firstNoArunkSeg)
        continue;

      const Percent discPercent = request.discountPercentage(firstNoArunkSeg->segmentOrder());

      if (discPercent > 100.0)
      {
        LOG4CXX_ERROR(logger, "Disc. percentage too large: " << discPercent);
        throw ErrorResponseException(
            ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED,
            "VERIFY DISCOUNT PERCENT");
      }

      if (discPercent <= 0.0)
        continue;

      ApplyFareDiscount applyFareDiscount(_farePath, discPercent, *this);
      applyFareDiscount(fareUsage);
    }

    Percent discPercent = 0.0;
    const DiscountAmount* discAmount = nullptr;

    // Apply DP discount on PricingUnit's PlusUp's - Note the DA discount is ignored
    TrxUtil::getDiscountOld(_trx, *pricingUnit, discPercent, discAmount);
    if (discPercent > 0.0)
      discountPricingUnitPlusUp(*pricingUnit, discPercent);
  }

  // Apply DP discount on FarePath's PlusUp's
  discountFarePathPlusUp();
}

void
DiscountPricing::processDAorPA() const
{
  const DiscAmounts& discAmounts = _trx.getRequest()->getDiscountAmountsNew();

  for (const auto& discAmount : discAmounts)
  {
    std::vector<FareUsage*> fcList;
    const MoneyAmount totalAmount = collectDiscFareComps(discAmount, fcList);

    const MoneyAmount diffAmount = calculateDiffAmount(discAmount);

    if (discAmount.amount < 0)
    {
      LOG4CXX_DEBUG(logger,
                    "PA Amount: " << -discAmount.amount << ", APPL Amount: " << diffAmount);
    }
    else
    {
      LOG4CXX_DEBUG(logger,
                    "DA Amount: " << discAmount.amount << ", APPL Amount: " << -diffAmount);
    }

    if ((totalAmount + diffAmount) < -EPSILON)
    {
      LOG4CXX_ERROR(logger,
                    "Disc. amount too large: " << -diffAmount
                                               << " (Total Amount: " << totalAmount << ")")

      throw ErrorResponseException(
          ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED,
          "DISCOUNT EXCEED FARE COMPONENT TOTAL");
    }

    std::for_each(
        fcList.begin(), fcList.end(), ApplyFareDiscount(_farePath, -diffAmount, totalAmount, *this));
  }
}

void
DiscountPricing::processDPorPP() const
{
  const PricingRequest& request = *_trx.getRequest();

  if (request.getDiscountPercentagesNew().empty())
    return;

  for (const PricingUnit* const pricingUnit : _farePath.pricingUnit())
  {
    for (FareUsage* const fareUsage : pricingUnit->fareUsage())
    {
      if (_isCETrx && fareUsage->paxTypeFare()->isDummyFare())
        continue; // ignore DP/DA for unchanged FC in CE

      const TravelSeg* firstNoArunkSeg = TravelSegUtil::firstNoArunkSeg(fareUsage->travelSeg());
      if (!firstNoArunkSeg)
        continue;

      const Percent* discPercent =
          request.discountPercentageNew(firstNoArunkSeg->segmentOrder(), _trx);

      if (discPercent == nullptr)
        continue;

      if (*discPercent > 100.0)
      {
        LOG4CXX_ERROR(logger, "Disc. percentage too large: " << *discPercent);
        throw ErrorResponseException(
            ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED,
            "VERIFY DISCOUNT PERCENT");
      }

      ApplyFareDiscount applyFareDiscount(_farePath, *discPercent, *this);
      applyFareDiscount(fareUsage);
    }

    const Percent* discPercentNew = _trx.getDiscountPercentageNew(*pricingUnit);

    if (discPercentNew != nullptr)
      discountPricingUnitPlusUp(*pricingUnit, *discPercentNew);
  }

  // Apply DP discount on FarePath's PlusUp's
  discountFarePathPlusUp();
}

MoneyAmount
DiscountPricing::collectDiscFareComps(const DiscountAmount& discAmount,
                                      std::vector<FareUsage*>& fcList) const
{
  MoneyAmount totalAmount = 0;
  for (const PricingUnit* const pricingUnit : _farePath.pricingUnit())
  {
    for (FareUsage* const fareUsage : pricingUnit->fareUsage())
    {
      if (_isCETrx && fareUsage->paxTypeFare()->isDummyFare())
        continue; // no discount on unchanged FC in CE

      std::vector<TravelSeg*> tvlSeg = fareUsage->travelSeg();
      TSE_ASSERT(!tvlSeg.empty());
      if (tvlSeg.front()->segmentOrder() >= discAmount.startSegmentOrder &&
          tvlSeg.back()->segmentOrder() <= discAmount.endSegmentOrder)
      {
        fcList.push_back(fareUsage);

        // @FIXME: any currency conversion required here in respect to the
        // origin currency?
        totalAmount += fareUsage->paxTypeFare()->totalFareAmount() + fareUsage->minFarePlusUpAmt();
      }
    }
  }

  return totalAmount;
}

MoneyAmount
DiscountPricing::calculateDiffAmount(DiscountAmount discount) const
{
  if (discount.currencyCode.empty())
  {
    discount.currencyCode = retrieveAAACurrencyCode();
    if (discount.currencyCode.empty())
      return 0.0;
  }

  const CurrencyCode& baseFareCurrency = _farePath.baseFareCurrency();
  const CurrencyCode& calculationCurrency = _farePath.calculationCurrency();

  Money moneyDifference(fabs(discount.amount), discount.currencyCode);

  if (baseFareCurrency != discount.currencyCode)
    moneyDifference = convertToCurrency(Money(fabs(discount.amount), discount.currencyCode), baseFareCurrency);

  if (calculationCurrency == NUC)
    moneyDifference = convertToCurrency(moneyDifference, NUC);

  return discount.amount < 0 ? moneyDifference.value() : -moneyDifference.value();
}

CurrencyCode
DiscountPricing::retrieveAAACurrencyCode() const
{
  const Loc* aaaLoc = TrxUtil::saleLoc(_trx);

  if (aaaLoc)
  {
    CurrencyCode discountCurrency;
    if (CurrencyUtil::getNationCurrency(aaaLoc->nation(), discountCurrency, _trx.ticketingDate()))
      return discountCurrency;
    else
      LOG4CXX_ERROR(logger, "Cannot retrieve the AAA currency - use NUC");
  }
  else
    LOG4CXX_ERROR(logger, "Cannot retrieve AAA city info - No discount.");

  return "";
}

Money
DiscountPricing::convertToCurrency(const Money& source, const CurrencyCode& targetCurrencyCode) const
{
  Money target(targetCurrencyCode);

  CurrencyConversionFacade currencyConverter;

  if (!currencyConverter.convert(target, source, _trx))
    LOG4CXX_ERROR(logger, "Currency conversion from " << source.code() << " to " << targetCurrencyCode << " failed");

  return target;
}

//
// Calculate the applicable discount amount after all currency conversion
// took place
//
// @return the applicable discount amount
//
MoneyAmount
DiscountPricing::calcApplicableDiscAmount(const DiscountAmount& discAmount) const
{
  // If no currency is specified, assume the currency of AAA
  //
  CurrencyCode currencyCode = discAmount.currencyCode;
  if (currencyCode.empty())
  {
    const Loc* aaaLoc = TrxUtil::saleLoc(_trx);
    if (!aaaLoc)
    {
      LOG4CXX_ERROR(logger, "Cannot retrieve AAA city info - No discount.");
      return 0.0;
    }

    // FIXME: getNationCurrency is not const-correct.
    NationCode aaaNation = aaaLoc->nation();
    if (!CurrencyUtil::getNationCurrency(aaaNation, currencyCode, _trx.ticketingDate()))
    {
      LOG4CXX_ERROR(logger, "Cannot retrieve the AAA currency - use NUC");
      return 0.0;
    }
  }

  // If the discount currency is different than the origin currency, convert
  // discount amount to the origin currency and discount the fares based on
  // the converted amount.  Even if the M<currency code> qualifier is used,
  // convert the discount amount directly to the origin currency (do not
  // go through the currency requested in M<currency code>)
  //

  if (_farePath.baseFareCurrency() != currencyCode)
  {
    Money source(discAmount.amount, currencyCode);

    Money target(_farePath.baseFareCurrency());

    CurrencyConversionFacade currencyConverter;

    if (currencyConverter.convert(target, source, _trx))
    {
      if (_farePath.calculationCurrency() == NUC)
      {
        Money nucTarget(NUC);

        if (currencyConverter.convert(nucTarget, target, _trx))
          return nucTarget.value();
        else
        {
          LOG4CXX_ERROR(logger, "NUC conversion failed for currency: " << target.code());
          return 0.0;
        }
      }

      return target.value();
    }
    else
    {
      LOG4CXX_ERROR(logger,
                    "BSR Currency conversion failed From currency "
                        << source.code() << " TO currency " << target.code());
      return 0.0;
    }
  }
  else if (_farePath.calculationCurrency() == NUC)
  {
    Money source(discAmount.amount, currencyCode);

    Money target(NUC);

    CurrencyConversionFacade currencyConverter;

    if (currencyConverter.convert(target, source, _trx))
    {
      return target.value();
    }
    else
    {
      LOG4CXX_ERROR(logger, "NUC Currency conversion failed for " << currencyCode);
      return 0.0;
    }

  }

  return discAmount.amount;
}

void
DiscountPricing::discountPricingUnitPlusUp(const PricingUnit& pu, const Percent& discPercent) const
{
  for (const auto& minFarePlusUpElem : pu.minFarePlusUp())
    applyDiscount(*minFarePlusUpElem.second, discPercent);

  for (const FareUsage* const fu : pu.fareUsage())
    for (const auto& minFarePlusUpElem : fu->minFarePlusUp())
      applyDiscount(*minFarePlusUpElem.second, discPercent);
}

void
DiscountPricing::discountFarePathPlusUp() const
{
  // OSC PLUS UP:
  for (OscPlusUp* const oscPlusUp : _farePath.oscPlusUp())
  {
    const FareMarket& fareMarket = *oscPlusUp->thruFare->fareMarket();
    Percent discPercent = 0.0;
    const Percent* discPercentNew = nullptr;
    const DiscountAmount* discAmount = nullptr;

    if (TrxUtil::newDiscountLogic(_trx))
    {
      discPercentNew = _trx.getDiscountPercentageNew(fareMarket);

      if (discPercentNew != nullptr)
        applyDiscount(*oscPlusUp, *discPercentNew);
    }
    else
    {
      TrxUtil::getDiscountOld(_trx, fareMarket, discPercent, discAmount); // lint !e530

      if (discPercent > 0.0)
        applyDiscount(*oscPlusUp, discPercent);
    }

  }

  // RSC PLUS UP:
  for (RscPlusUp* const rscPlusUp : _farePath.rscPlusUp())
  {
    if (TrxUtil::newDiscountLogic(_trx))
    {
      const Percent* discPercent = getDiscountNew(*rscPlusUp);
      if (discPercent != nullptr)
        applyDiscount(*rscPlusUp, *discPercent);
    }
    else
    {
      Percent discPercent = getDiscountOld(*rscPlusUp);
      if (discPercent > 0.0)
        applyDiscount(*rscPlusUp, discPercent);
    }
  }

  // COM/DMC PLUS UP:
  for (PlusUpInfo* const plusUpInfo : _farePath.plusUpInfoList())
  {
    if (TrxUtil::newDiscountLogic(_trx))
    {
      const Percent* discPercent = getDiscountNew(*plusUpInfo->minFarePlusUp());
      if (discPercent != nullptr)
        applyDiscount(*plusUpInfo->minFarePlusUp(), *discPercent);
    }
    else
    {
      Percent discPercent = getDiscountOld(*plusUpInfo);
      if (discPercent > 0.0)
        applyDiscount(*plusUpInfo->minFarePlusUp(), discPercent);
    }
  }
}

Percent
DiscountPricing::getDiscountOld(const FarePath::RscPlusUp& rscPlusUp) const
{
  Percent discPercent = 0.0;
  const DiscountAmount* discAmount = nullptr;

  for (const PricingUnit* const pu : _farePath.pricingUnit())
  {
    for (const TravelSeg* const travelSeg : pu->travelSeg())
    {
      if (travelSeg->origin()->loc() == rscPlusUp.boardPoint)
      {
        TrxUtil::getDiscountOld(_trx, *pu, discPercent, discAmount);
        if (discPercent > 0.0)
          return discPercent;
      }
    }
  }

  return 0;
}

Percent
DiscountPricing::getDiscountOld(const FarePath::PlusUpInfo& plusUpInfo) const
{
  Percent discPercent = 0.0;
  const DiscountAmount* discAmount = nullptr;

  for (const PricingUnit* const pu : _farePath.pricingUnit())
  {
    for (const TravelSeg* const travelSeg : pu->travelSeg())
    {
      if (travelSeg->origin()->loc() == plusUpInfo.minFarePlusUp()->boardPoint)
      {
        TrxUtil::getDiscountOld(_trx, *pu, discPercent, discAmount);
        if (discPercent > 0.0)
          return discPercent;
      }
    }
  }

  return 0;
}

const Percent*
DiscountPricing::getDiscountNew(const MinFarePlusUpItem& minFarePlusUpItem) const
{
  for (const PricingUnit* const pu : _farePath.pricingUnit())
    for (const TravelSeg* const travelSeg : pu->travelSeg())
      if (travelSeg->origin()->loc() == minFarePlusUpItem.boardPoint)
      {
        const Percent* discPercent = _trx.getDiscountPercentageNew(*pu);
        if (discPercent != nullptr)
          return discPercent;
      }

  return nullptr;
}

void
DiscountPricing::ApplyFareDiscount::
operator()(FareUsage* fareUsage)
{
  if (TrxUtil::newDiscountLogic(_discountPricing._trx))
  {
    if (_discPercent - EPSILON > 1.0)
    {
      LOG4CXX_ERROR(logger, "Invalid Disc. percent: " << _discPercent);
      return;
    }
  }
  else
  {
    if (_discPercent < 0.0 || (_discPercent - EPSILON) > 1.0)
    {
      LOG4CXX_ERROR(logger, "Invalid Disc. percent: " << _discPercent);
      return;
    }
  }

  if (_farePath.calculationCurrency() == NUC)
  {
    discountNUCs(fareUsage);
    return;
  }

  // NOTE: the PaxTypeFare::totalFareAmount is the sum of mileage surcharge
  //       and Fare::nucFareAmount
  //
  // 5.3: Apply mileage surcharge before any discount amount.
  MoneyAmount fareAmount = fareUsage->paxTypeFare()->totalFareAmount();
  fareAmount -= fareUsage->getSpanishResidentDiscountAmt();
  if (_discEntry == DiscEntry::amount)
  {
    fareAmount += fareUsage->minFarePlusUpAmt();
  }

  fareUsage->setDiscAmount(fareAmount * _discPercent);

  // This is to guard agains display -0.0000123 as -0.00, it's zero.
  if (fareUsage->getDiscAmount() > fareAmount)
    fareUsage->setDiscAmount(fareAmount);

  LOG4CXX_DEBUG(logger,
                "\n\tfareAmount: " << fareAmount << ", discPercent: " << _discPercent
                                   << ", fu.discAmount: " << fareUsage->getDiscAmount());

  // Update the farePath total amount to include the discount
  _farePath.accumulatePriceDeviationAmount(-fareUsage->getDiscAmount());
  if (std::abs(_farePath.getTotalNUCAmount()) <= EPSILON)
    _farePath.setTotalNUCAmount(0);
}

void
DiscountPricing::ApplyFareDiscount::discountNUCs(FareUsage* fareUsage)
{
  MoneyAmount fareAmount = _farePath.useSecondRoeDate()
                               ? fareUsage->paxTypeFare()->fare()->rexSecondNucOriginalFareAmount()
                               : fareUsage->paxTypeFare()->nucOriginalFareAmount();
  MoneyAmount totalAmount = fareUsage->paxTypeFare()->totalFareAmount();

  fareAmount *= (1 - _discPercent);
  CurrencyUtil::truncateNUCAmount(fareAmount);

  if (fareUsage->paxTypeFare()->isRoundTrip()) // calculate for HR
    CurrencyUtil::halveNUCAmount(fareAmount);

  // mileage surcharge
  fareAmount *= (100.0 + fareUsage->paxTypeFare()->mileageSurchargePctg()) / 100.0;
  fareAmount -= fareUsage->getSpanishResidentDiscountAmt();
  CurrencyUtil::truncateNUCAmount(fareAmount);

  // Plus Up
  if (_discEntry == DiscEntry::amount)
  {
    MoneyAmount plusUp = fareUsage->minFarePlusUpAmt() * (1 - _discPercent);
    CurrencyUtil::truncateNUCAmount(plusUp);
    fareAmount += plusUp;
    totalAmount += fareUsage->minFarePlusUpAmt();
  }

  fareUsage->setDiscAmount(totalAmount - fareAmount);

  _farePath.accumulatePriceDeviationAmount(-fareUsage->getDiscAmount());
  if (std::abs(_farePath.getTotalNUCAmount()) <= EPSILON)
    _farePath.setTotalNUCAmount(0);
}

void
DiscountPricing::applyDiscount(MinFarePlusUpItem& minFarePlusUp, const Percent& discount) const
{
  minFarePlusUp.applyDynamicPriceDeviation(_farePath, -discount);
}
}
