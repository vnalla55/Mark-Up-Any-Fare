//-------------------------------------------------------------------
//
//  File:        DiscountPricing.h
//  Created:     April 14, 2005
//  Design:
//  Authors:     Quan Ta
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"

namespace tse
{
class FareUsage;
class MinFarePlusUpItem;
class PricingTrx;
class PricingUnit;

static Percent calcDiscountPercentage(MoneyAmount discAmount, MoneyAmount totalAmount)
{
  if (std::abs(totalAmount) < EPSILON)
    return 0.0;

  return discAmount/totalAmount;
}

class DiscountPricing
{
  friend class DiscountPricingTest;

public:
  DiscountPricing(const PricingTrx& trx, FarePath& farePath);

  void processOld() const;
  void process() const;

private:
  enum class DiscEntry
  { amount,
    percentage };

  class ApplyFareDiscount
  {
  public:
    ApplyFareDiscount(FarePath& farePath, MoneyAmount discAmount, MoneyAmount totalAmount, const DiscountPricing& discountPricing)
      : _farePath(farePath), _discPercent(calcDiscountPercentage(discAmount, totalAmount) ), _discEntry(DiscEntry::amount), _discountPricing(discountPricing)
    {
    }
    ApplyFareDiscount(FarePath& farePath, const Percent discPercent, const DiscountPricing& discountPricing)
      : _farePath(farePath), _discPercent(discPercent / 100.0), _discEntry(DiscEntry::percentage), _discountPricing(discountPricing)
    {
    }

    void operator()(FareUsage* fareUsage);

  protected:
    void discountNUCs(FareUsage* fareUsage);

  private:
    FarePath& _farePath;
    Percent _discPercent;
    DiscEntry _discEntry;
    const DiscountPricing& _discountPricing;
  };

  using DiscAmounts = std::vector<DiscountAmount>;
  using SegmentOrder = int16_t;
  using DiscPercentages = std::map<SegmentOrder, Percent>;

  void processDA(const DiscAmounts& discAmounts) const;
  void processDP() const;
  void processDAorPA() const;
  void processDPorPP() const;

  MoneyAmount calculateDiffAmount(DiscountAmount discount) const;
  CurrencyCode retrieveAAACurrencyCode() const;
  Money convertToCurrency(const Money& source, const CurrencyCode& targetCurrencyCode) const;

  void discountPricingUnitPlusUp(const PricingUnit& pu, const Percent& discPercent) const;
  void discountFarePathPlusUp() const;

  Percent getDiscountOld(const FarePath::RscPlusUp& rscPlusUp) const;
  Percent getDiscountOld(const FarePath::PlusUpInfo& plusUpInfo) const;
  const Percent* getDiscountNew(const MinFarePlusUpItem& minFarePlusUpItem) const;

  //
  // Apply discount percent to the given amount, and update the total amount
  // accordingly.
  //
  void applyDiscount(MinFarePlusUpItem&, const Percent& percent) const;
  //
  // Collect the fare components effected by this discount command
  //
  MoneyAmount
  collectDiscFareComps(const DiscountAmount& discAmount, std::vector<FareUsage*>& fcList) const;

  MoneyAmount calcApplicableDiscAmount(const DiscountAmount& discAmount) const;

private:
  const PricingTrx& _trx;
  FarePath& _farePath;
  const bool _isCETrx;
};
} // end of tse namespace
