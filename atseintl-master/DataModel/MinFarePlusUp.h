//-------------------------------------------------------------------
//
//  File:        PlusUp.h
//  Created:     April 11, 2005
//  Design:      Jeff Hoffman
//  Authors:
//
//  Description: Aggregate of all PlusUp data
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/VecMultiMap.h"

#include <algorithm>
#include <functional>
#include <numeric>

namespace tse
{
class FarePath;

class MinFarePlusUpItem
{
public:
  virtual ~MinFarePlusUpItem() = default;

  MoneyAmount plusUpAmount = 0.0;
  MoneyAmount baseAmount = 0.0;
  LocCode boardPoint;
  LocCode offPoint;
  LocCode constructPoint;

  CurrencyCode currency; // Currency code of the published charge
  // Please note that the baseAmount, and plusUpAmount
  // is not in this currency, it is in NUC

  void copyMinFarePlusUpItem(const MinFarePlusUpItem* minFarePlusUpItem);
  void applyDynamicPriceDeviation(FarePath& fp, Percent deviation);

private:
  void accumulateDynamicPriceDeviation(MoneyAmount amt);
  void rollbackDynamicPriceDeviation();
  MoneyAmount _dynamicPriceDeviation = 0;
};

class BhcPlusUpItem : public MinFarePlusUpItem
{
public:
  LocCode fareBoardPoint;
  LocCode fareOffPoint;
};

class MinFarePlusUp : public VecMultiMap<MinimumFareModule, MinFarePlusUpItem*>
{
public:
  MoneyAmount getSum(MinimumFareModule module) const
  {
    if (empty())
      return 0.0;

    return std::accumulate(lower_bound(module), upper_bound(module), 0.0, sumPlusUpAmounts);
  }

  MoneyAmount getTotalSum() const
  {
    if (empty())
      return 0.0;

    return std::accumulate(begin(), end(), 0.0, sumPlusUpAmounts);
  }

  void addItem(MinimumFareModule module, MinFarePlusUpItem* item)
  {
    insert(std::make_pair(module, item));
  }

  MinFarePlusUpItem* getPlusUp(MinimumFareModule module) const
  {
    const_iterator lowerPos = lower_bound(module);
    if (lowerPos != upper_bound(module))
      return lowerPos->second;
    return nullptr;
  }

private:
  static MoneyAmount sumPlusUpAmounts(MoneyAmount amount, const KeyValuePair& pair)
  {
    return amount + pair.second->plusUpAmount;
  }
};

void inline MinFarePlusUpItem::copyMinFarePlusUpItem(const MinFarePlusUpItem* minFarePlusUpItem)
{
  plusUpAmount = minFarePlusUpItem->plusUpAmount;
  baseAmount = minFarePlusUpItem->baseAmount;
  boardPoint = minFarePlusUpItem->boardPoint;
  offPoint = minFarePlusUpItem->offPoint;
  constructPoint = minFarePlusUpItem->constructPoint;
  currency = minFarePlusUpItem->currency;
  _dynamicPriceDeviation = minFarePlusUpItem->_dynamicPriceDeviation;
}

void inline MinFarePlusUpItem::rollbackDynamicPriceDeviation()
{
  plusUpAmount -= _dynamicPriceDeviation;
  _dynamicPriceDeviation = 0;
}

void inline MinFarePlusUpItem::accumulateDynamicPriceDeviation(MoneyAmount amt)
{
  _dynamicPriceDeviation += amt;
  plusUpAmount += amt;
}
}
