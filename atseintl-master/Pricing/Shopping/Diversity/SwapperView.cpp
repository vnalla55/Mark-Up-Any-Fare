#include "Pricing/Shopping/Diversity/SwapperView.h"

#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/Diversity/SwapperValidationStrategy.h"

namespace tse
{

SwapperView::Iterator
SwapperView::initialize(const ScoredCombinations& scoredCombinations)
{
  Iterator it;
  it.scoredCombsCI = scoredCombinations.begin();
  it.scoredCombsEnd = scoredCombinations.end();
  if (LIKELY(it.scoredCombsCI != it.scoredCombsEnd))
  {
    it.solutionCI = getSolutionList(it).begin();
  }
  return it;
}

bool
SwapperView::getNextSolution(Iterator& it,
                             SwapperEvaluationResult& oldSolutionResult,
                             const Solution*& oldSolution)
{
  if (!findNext(it))
    return false;
  const Solution& oldSolutionRef = *it.solutionCI;
  oldSolution = &oldSolutionRef;
  oldSolutionResult =
      _validationStrategy.validate(oldSolution->first, oldSolution->second->getTotalNUCAmount());
  ++it.solutionCI;
  return true;
}

bool
SwapperView::findNext(Iterator& it)
{
  if (UNLIKELY(it.scoredCombsCI == it.scoredCombsEnd))
    return false;

  while (1)
  {
    if (it.solutionCI != getSolutionList(it).end())
      return true;

    if (LIKELY(++it.scoredCombsCI == it.scoredCombsEnd))
      return false;

    it.solutionCI = getSolutionList(it).begin();
  }

  return false;
}
}
