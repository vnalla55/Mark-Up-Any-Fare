
#pragma once

#include "Pricing/Shopping/Diversity/SwapperValidationStrategy.h"

namespace tse
{

class ShoppingTrx;
class ItinStatistic;
class DiversityModel;

class SwapperBasicValidationStrategy : public SwapperValidationStrategy
{

public:
  SwapperBasicValidationStrategy(ShoppingTrx& trx,
                                 ItinStatistic& stats,
                                 const DiversityModel* model);

  virtual void setNewSolutionAttributes(const NewSolutionAttributes* newSolution) override
  {
    _newSolution = newSolution;
  }

  virtual void setCurrentBucket(const Diversity::BucketType bucket) override { _bucket = bucket; }

  SwapperEvaluationResult checkCarrierDiversity(const SopIdVec& oldCombination) const;
  SwapperEvaluationResult checkNonStopDiversity(const SopIdVec& oldCombination) const;
  SwapperEvaluationResult checkLastMinPricedItin(MoneyAmount oldPrice) const;
  SwapperEvaluationResult checkCustomSolution(const SopIdVec& oldCombination) const;

private:
  const ShoppingTrx& _trx;
  const ItinStatistic& _stats;
  const DiversityModel* _model;
  const NewSolutionAttributes* _newSolution;
  Diversity::BucketType _bucket;
};
}

