
#pragma once

#include "Pricing/Shopping/Diversity/SwapperValidationStrategy.h"

namespace tse
{

class ShoppingTrx;
class ItinStatistic;

class SwapperIBFValidationStrategy : public SwapperValidationStrategy
{
public:
  SwapperIBFValidationStrategy(ShoppingTrx& trx, ItinStatistic& stats);

  SwapperEvaluationResult checkNonStopSolution(const SopIdVec& oldCombination) const;
  SwapperEvaluationResult checkAllSopsRequirement(const SopIdVec& oldCombination) const;
  SwapperEvaluationResult checkRCOnlineRequirement(const SopIdVec& oldCombination) const;

private:
  const ShoppingTrx& _trx;
  const ItinStatistic& _stats;
};
}

