#include "Pricing/Shopping/Diversity/SwapperIBFValidationStrategy.h"

#include "Common/ShoppingUtil.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

SwapperIBFValidationStrategy::SwapperIBFValidationStrategy(ShoppingTrx& trx, ItinStatistic& stats)
  : _trx(trx), _stats(stats)
{
  // We cannot remove direct flights generated as FOSes before PQ
  addValidator(std::tr1::bind(
      &SwapperIBFValidationStrategy::checkNonStopSolution, this, std::tr1::placeholders::_1));
  // Every flight has to be represented
  addValidator(std::tr1::bind(
      &SwapperIBFValidationStrategy::checkAllSopsRequirement, this, std::tr1::placeholders::_1));
  // Online Options should be preferred before processing goes to interline
  addValidator(std::tr1::bind(
      &SwapperIBFValidationStrategy::checkRCOnlineRequirement, this, std::tr1::placeholders::_1));
}

SwapperEvaluationResult
SwapperIBFValidationStrategy::checkNonStopSolution(const SopIdVec& oldCombination) const
{
  if (SopCombinationUtil::detectNonStop(_trx, oldCombination))
  {
    if ( !_trx.getRequest()->isAllFlightsRepresented())
      return SwapperEvaluationResult::NON_STOPS;

    if (_stats.getMissingDirectOptionsCount() >= 0)
      return SwapperEvaluationResult::NON_STOPS;
  }

  return SwapperEvaluationResult::SELECTED;
}

SwapperEvaluationResult
SwapperIBFValidationStrategy::checkAllSopsRequirement(const SopIdVec& oldCombination) const
{
  if (_stats.getSopPairing(0, oldCombination[0]) == 1)
    return SwapperEvaluationResult::ALL_SOPS;

  if (oldCombination.size() > 1 && _stats.getSopPairing(1, oldCombination[1]) == 1)
    return SwapperEvaluationResult::ALL_SOPS;

  return SwapperEvaluationResult::SELECTED;
}

SwapperEvaluationResult
SwapperIBFValidationStrategy::checkRCOnlineRequirement(const SopIdVec& oldCombination) const
{
  if (ShoppingUtil::isOnlineOptionForCarrier(_trx, oldCombination, _stats.getRequestingCarrier()) &&
      _stats.getMissingRCOnlineOptionsCount() >= 0)
    return SwapperEvaluationResult::RC_ONLINES;

  return SwapperEvaluationResult::SELECTED;
}
}
