
#pragma once

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/Diversity/SwapperTypes.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"

#include <tr1/functional>

namespace tse
{

class SwapperValidationStrategy;

class SwapperView
{
public:
  typedef ItinStatistic::ScoredCombinations ScoredCombinations;
  typedef ScoredCombinations::const_iterator ScoredCombinationsCI;
  typedef ItinStatistic::Solution Solution;
  typedef std::list<Solution>::const_iterator SolutionCI;

  struct Iterator
  {
    ScoredCombinationsCI scoredCombsCI;
    ScoredCombinationsCI scoredCombsEnd;
    SolutionCI solutionCI;
  };

  SwapperView(const SwapperValidationStrategy& validationStrategy)
    : _validationStrategy(validationStrategy)
  {
  }

  Iterator initialize(const ScoredCombinations& scoredCombinations);

  bool getNextSolution(Iterator& it,
                       SwapperEvaluationResult& oldSolutionResult,
                       const Solution*& oldSolution);

private:
  typedef std::tr1::function<SwapperEvaluationResult(const SopIdVec&, MoneyAmount)>
  OldSolutionValidator;

  const std::list<Solution>& getSolutionList(const Iterator& it)
  {
    return it.scoredCombsCI->second;
  }

  bool findNext(Iterator& it);

  const SwapperValidationStrategy& _validationStrategy;
};
}

