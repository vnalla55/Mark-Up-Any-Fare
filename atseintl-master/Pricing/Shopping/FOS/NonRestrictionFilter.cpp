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
#include "Pricing/Shopping/FOS/NonRestrictionFilter.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/FOS/FosStatistic.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"

#include <algorithm>

namespace tse
{

namespace fos
{
NonRestrictionFilter::NonRestrictionFilter(const ShoppingTrx& trx, const FosTaskScope& task)
  : _trx(trx),
    _numTSPerLeg(0),
    _totalNumTS(0),
    _checkConnectingCities(false),
    _canProduceNonStops(true),
    _isNoPricedSolutions(!(_trx.flightMatrix().size() + _trx.estimateMatrix().size())),
    _isPqOverride(task.pqConditionOverride())
{
  if (trx.legs().size() == 2 && task.checkConnectingCities())
    _checkConnectingCities = true;
}

bool
NonRestrictionFilter::isApplicableSolution(const SopCombination& sopCombination) const
{
  if (UNLIKELY(_numTSPerLeg > 0 &&
      !FosCommonUtil::checkNumOfTravelSegsPerLeg(_trx, sopCombination, _numTSPerLeg)))
    return false;

  if (_totalNumTS > 0 &&
      !FosCommonUtil::checkTotalNumOfTravelSegs(_trx, sopCombination, _totalNumTS))
    return false;

  if (UNLIKELY(_checkConnectingCities && !FosCommonUtil::checkConnectingCitiesRT(_trx, sopCombination)))
    return false;

  return true;
}

bool
NonRestrictionFilter::isFilterToPop(const FosStatistic& stats) const
{
  return !FosCommonUtil::applyNonRestrictionFilter(
             _trx, stats, _isPqOverride, _isNoPricedSolutions);
}

void
NonRestrictionFilter::setNumTSRestrictions(const size_t totalNumTS, const size_t numTSPerLeg)
{
  _totalNumTS = totalNumTS;
  _numTSPerLeg = numTSPerLeg;
  _canProduceNonStops = (_numTSPerLeg < 1) && (_totalNumTS < _trx.legs().size());
}

} // fos
} // tse
