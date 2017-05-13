
#pragma once

#include "Pricing/Shopping/Diversity/SwapperTypes.h"

#include <vector>

#include <tr1/functional>

namespace tse
{

class SwapperValidationStrategy
{
public:
  virtual ~SwapperValidationStrategy() {}

  virtual void setNewSolutionAttributes(const NewSolutionAttributes*) {}
  virtual void setCurrentBucket(const Diversity::BucketType) {}

  SwapperEvaluationResult validate(const SopIdVec& oldCombination, MoneyAmount oldPrice) const
  {
    for (const auto& validator : _validationPath)
    {
      SwapperEvaluationResult res = validator(oldCombination, oldPrice);
      if (res != SwapperEvaluationResult::SELECTED)
        return res;
    }
    return SwapperEvaluationResult::SELECTED;
  }

protected:
  typedef std::tr1::function<SwapperEvaluationResult(const SopIdVec&, MoneyAmount)>
  OldSolutionValidator;

  void addValidator(const OldSolutionValidator& validator) { _validationPath.push_back(validator); }

  std::vector<OldSolutionValidator> _validationPath;
};
}

