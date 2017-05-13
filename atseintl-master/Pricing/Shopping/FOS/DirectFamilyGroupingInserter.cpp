// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/FOS/DirectFamilyGroupingInserter.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Diagnostic/Diag910Collector.h"

namespace tse
{
namespace
{
ConfigurableValue<bool>
familyGroupingEnabledCfg("SHOPPING_DIVERSITY", "DIRECT_FOS_FAMILY_GROUPING", false);
}
namespace fos
{
static Logger
logger("atseintl.pricing.FOS.DirectFamilyGroupingInserter");

struct BaseSolutionFinder
{
  BaseSolutionFinder(const ShoppingTrx& trx, const SopIdVec& solution)
    : _trx(trx), _solution(solution)
  {
  }

  BaseSolutionFinder(const BaseSolutionFinder& baseSolutionFinder)
    : _trx(baseSolutionFinder._trx), _solution(baseSolutionFinder._solution)
  {
  }

  bool operator()(const SopIdVec& baseSolution) const
  {
    typedef ShoppingTrx::Leg Leg;
    typedef ShoppingTrx::SchedulingOption SOP;

    if (baseSolution.size() != _solution.size())
      return false;

    for (size_t legId = 0; legId < baseSolution.size(); legId++)
    {
      const Leg& leg = _trx.legs()[legId];
      const SOP& baseSop = leg.sop()[baseSolution[legId]];
      const SOP& sop = leg.sop()[_solution[legId]];

      if (!ShoppingUtil::isSameCxrAndCnxPointAndClassOfService(_trx, baseSop, sop))
        return false;
    }

    return true;
  }

private:
  const ShoppingTrx& _trx;
  const SopIdVec& _solution;
};

DirectFamilyGroupingInserter::DirectFamilyGroupingInserter(ShoppingTrx& trx,
                                                           Diag910Collector* dc910)
  : FosMatrixInserter(trx, dc910), _familyGroupingEnabled(false)
{
  _familyGroupingEnabled = familyGroupingEnabledCfg.getValue();
}

void
DirectFamilyGroupingInserter::addCombination(ValidatorBitMask validBitMask,
                                             const SopIdVec& combination)
{
  typedef ShoppingTrx::FlightMatrix FlightMatrix;
  typedef ShoppingTrx::EstimatedSolution EstimatedSolution;
  typedef ShoppingTrx::EstimateMatrix EstimateMatrix;

  if (!_familyGroupingEnabled || (validBitMask & validatorBitMask(VALIDATOR_NONSTOP)) == 0)
  {
    FosMatrixInserter::addCombination(validBitMask, combination);
    return;
  }

  BaseSolutionFinder baseSolutionFinder(_trx, combination);
  std::vector<SopIdVec>::iterator it =
      std::find_if(_baseSolutions.begin(), _baseSolutions.end(), baseSolutionFinder);

  if (it == _baseSolutions.end())
  {
    FlightMatrix::value_type flightMatrixItem(combination, nullptr);
    _trx.flightMatrix().insert(flightMatrixItem);
    _baseSolutions.push_back(combination);
    if (_dc910)
      _dc910->printFos(_trx, combination, SopIdVec(), validBitMask);
  }
  else
  {
    EstimatedSolution estimatedSolution(*it, nullptr);
    EstimateMatrix::value_type estimateMatrixItem(combination, estimatedSolution);
    _trx.estimateMatrix().insert(estimateMatrixItem);
    if (_dc910)
      _dc910->printFos(_trx, combination, estimatedSolution.first, validBitMask);
  }
}

} // fos
} // tse
