#include "Pricing/Shopping/Diversity/SwapperBasicValidationStrategy.h"

#include "Common/Assert.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

SwapperBasicValidationStrategy::SwapperBasicValidationStrategy(ShoppingTrx& trx,
                                                               ItinStatistic& stats,
                                                               const DiversityModel* model)
  : _trx(trx), _stats(stats), _model(model), _newSolution(nullptr), _bucket(Diversity::BUCKET_COUNT)
{
  // We can't remove solutions which are required by carrier ...
  addValidator(std::tr1::bind(
      &SwapperBasicValidationStrategy::checkCarrierDiversity, this, std::tr1::placeholders::_1));
  // or non-stop diversity requirements
  addValidator(std::tr1::bind(
      &SwapperBasicValidationStrategy::checkNonStopDiversity, this, std::tr1::placeholders::_1));

  addValidator(std::tr1::bind(
      &SwapperBasicValidationStrategy::checkLastMinPricedItin, this, std::tr1::placeholders::_2));

  addValidator(std::tr1::bind(
      &SwapperBasicValidationStrategy::checkCustomSolution, this, std::tr1::placeholders::_1));
}

SwapperEvaluationResult
SwapperBasicValidationStrategy::checkCarrierDiversity(const SopIdVec& oldCombination) const
{
  TSE_ASSERT(_newSolution);

  // TODO: Ask model about carrier requirements
  const CarrierCode& oldCarrier = SopCombinationUtil::detectCarrier(_trx, oldCombination);

  if (oldCarrier != Diversity::INTERLINE_CARRIER && oldCarrier != _newSolution->carrier &&
      _stats.getNumOfItinsForCarrier(oldCarrier) <=
          _trx.diversity().getOptionsPerCarrier(oldCarrier))
    return SwapperEvaluationResult::CARRIERS;

  return SwapperEvaluationResult::SELECTED;
}

SwapperEvaluationResult
SwapperBasicValidationStrategy::checkNonStopDiversity(const SopIdVec& oldCombination) const
{
  TSE_ASSERT(_newSolution);

  bool isOldNonStop = (SopCombinationUtil::detectNonStop(_trx, oldCombination) &
                       SopCombinationUtil::ONLINE_NON_STOP);

  // We can't remove non-stops on HDNS-markets
  if (_trx.diversity().isHighDensityMarket() && isOldNonStop)
    return SwapperEvaluationResult::NON_STOPS;

  if (UNLIKELY((isOldNonStop && !_newSolution->isNonStop) && _model->isNonStopOptionNeeded()))
    return SwapperEvaluationResult::NON_STOPS;

  return SwapperEvaluationResult::SELECTED;
}

SwapperEvaluationResult
SwapperBasicValidationStrategy::checkLastMinPricedItin(MoneyAmount oldPrice) const
{
  TSE_ASSERT(_bucket != Diversity::BUCKET_COUNT);

  if (_bucket != Diversity::GOLD && _bucket != Diversity::UGLY)
    return SwapperEvaluationResult::SELECTED;

  if (fabs(oldPrice - _stats.getMinPrice(_bucket)) > 0.01)
    return SwapperEvaluationResult::SELECTED;

  if (_stats.getMinPricedItinCount(_bucket) != 1)
    return SwapperEvaluationResult::SELECTED;

  return SwapperEvaluationResult::LAST_MIN_PRICED;
}

SwapperEvaluationResult
SwapperBasicValidationStrategy::checkCustomSolution(const SopIdVec& oldCombination) const
{
  if (UNLIKELY(_trx.diversity().getNumOfCustomSolutions() &&
      ShoppingUtil::isCustomSolution(_trx, oldCombination)))
    return SwapperEvaluationResult::CUSTOM_SOLUTION;

  return SwapperEvaluationResult::SELECTED;
}
}
