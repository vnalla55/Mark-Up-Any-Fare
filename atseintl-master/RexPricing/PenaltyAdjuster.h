//-------------------------------------------------------------------
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

#include "DataModel/PricingUnit.h"

#include <cmath>
#include <numeric>

namespace tse
{

class PenaltyAdjuster
{
public:
  friend class PenaltyAdjusterTest;

  enum SumarizeStrategy
  {
    SUMARIZE_FC = 0,
    SUMARIZE_FU
  };

protected:
  class ComponentSumarizer
  {
  public:
    explicit ComponentSumarizer(const SumarizeStrategy strategy) : _strategy(strategy) {}

    MoneyAmount operator()(MoneyAmount total, const FareUsage* fu) const
    {
      if (_strategy == SUMARIZE_FC)
        return total + fu->surchargeAmt() + fu->stopOverAmt() + fu->differentialAmt() +
               fu->paxTypeFare()->fareMarket()->fareCompInfo()->fareCalcFareAmt();

      return total + fu->totalFareAmount();
    }

  private:
    const SumarizeStrategy _strategy;
  };

  class PlusUpSumarizer
  {
  public:
    MoneyAmount operator()(MoneyAmount total, const MinFarePlusUp::value_type& plus)
    {
      return total + plus.second->plusUpAmount;
    }
  };

  MoneyAmount _equalizer;
  MoneyAmount _adjustedPuAmt;
  const ComponentSumarizer _componentSumarizer;

public:
  PenaltyAdjuster(const PricingUnit& pu,
                  const SumarizeStrategy strategy,
                  const MoneyAmount& fpPlusUps = 0.0)
    : _componentSumarizer(strategy)
  {
    const MoneyAmount plusUpsSum = std::accumulate(
        pu.minFarePlusUp().begin(), pu.minFarePlusUp().end(), fpPlusUps, PlusUpSumarizer());

    const MoneyAmount puComponentsSum =
        std::accumulate(pu.fareUsage().begin(), pu.fareUsage().end(), 0.0, _componentSumarizer);

    _adjustedPuAmt = plusUpsSum + puComponentsSum;
    _equalizer = (std::fabs(puComponentsSum) < EPSILON ? 0.0 : plusUpsSum / puComponentsSum);
  }

  MoneyAmount adjustedFuAmt(const FareUsage& fu) const
  {
    const MoneyAmount fcSum = _componentSumarizer(0.0, &fu);
    return _equalizer * fcSum + fcSum;
  }

  MoneyAmount adjustedPuAmt() const { return _adjustedPuAmt; }
};
}

