//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Pricing/RequiredNonStopsCalculator.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/ShoppingPQ.h"

#include <algorithm>

namespace tse
{
namespace
{
Logger
logger("atseintl.Pricing.RequiredNonStopsCalculator");

struct IsValidNonStop
{
  const ShoppingTrx& _trx;

  IsValidNonStop(const ShoppingTrx& trx) : _trx(trx) {}

  template <class T>
  bool operator()(const T& solution) const
  {
    if (_trx.onlineSolutionsOnly() && !ShoppingUtil::isOnlineSolution(_trx, solution.first))
      return false;
    if (_trx.interlineSolutionsOnly() && ShoppingUtil::isOnlineSolution(_trx, solution.first))
      return false;
    return ShoppingUtil::isDirectFlightSolution(_trx, solution.first);
  }
};
}

void
RequiredNonStopsCalculator::init(const ShoppingTrx& trx)
{
  const Diversity& div = trx.diversity();

  if (trx.onlineSolutionsOnly())
    _maxNumberOfNS = div.getMaxOnlineNonStopCount();
  else if (trx.interlineSolutionsOnly())
    _maxNumberOfNS = div.getMaxInterlineNonStopCount();
  else
    _maxNumberOfNS = div.getMaxNonStopCount();
}

void
RequiredNonStopsCalculator::countAlreadyGeneratedNS(const ShoppingTrx& trx,
                                                    const ShoppingTrx::FlightMatrix& fm)
{
  if (_maxNumberOfNS == 0)
    return;
  _numAlreadyGeneratedNS += std::count_if(fm.begin(), fm.end(), IsValidNonStop(trx));
}

void
RequiredNonStopsCalculator::countAlreadyGeneratedNS(const ShoppingTrx& trx,
                                                    const ShoppingTrx::EstimateMatrix& em)
{
  if (_maxNumberOfNS == 0)
    return;
  _numAlreadyGeneratedNS += std::count_if(em.begin(), em.end(), IsValidNonStop(trx));
}

void
RequiredNonStopsCalculator::countAlreadyGeneratedNS(const ShoppingTrx& trx)
{
  if (_maxNumberOfNS == 0)
    return;

  for (const ShoppingTrx::PQPtr& pqItem : trx.shoppingPQVector())
  {
    countAlreadyGeneratedNS(trx, pqItem->flightMatrix());
    countAlreadyGeneratedNS(trx, pqItem->estimateMatrix());
  }
}

std::size_t
RequiredNonStopsCalculator::calcRequiredNSCount(const ShoppingTrx& trx)
{
  if (_numAlreadyGeneratedNS > _maxNumberOfNS)
  {
    LOG4CXX_ERROR(logger, "Number of already generated non-stops is greater than maximum");
    return 0u;
  }

  std::size_t requestedNSCount = trx.diversity().getNonStopOptionsCount();
  std::size_t possibleNSCount = _maxNumberOfNS - _numAlreadyGeneratedNS;

  return std::min(possibleNSCount, requestedNSCount);
}

void
RequiredNonStopsCalculator::countAlreadyGeneratedNSPerCarrier(const ShoppingTrx& trx)
{
  for (const ShoppingTrx::PQPtr& pq : trx.shoppingPQVector())
  {
    countAlreadyGeneratedNSPerCarrier(trx, pq->flightMatrix());
    countAlreadyGeneratedNSPerCarrier(trx, pq->estimateMatrix());
  }
}

void
RequiredNonStopsCalculator::calcRequiredNSCountPerCarrier(
    const ShoppingTrx& trx,
    uint32_t totalOptionsRequired,
    fos::FosCommonUtil::CarrierMap& requiredMap)
{
  std::vector<fos::FosCommonUtil::CarrierMapElem> availableVec;
  availableVec.reserve(trx.diversity().getMaxNonStopCountPerCarrier().size());

  for (const fos::FosCommonUtil::CarrierMapElem& cxrElem :
       trx.diversity().getMaxNonStopCountPerCarrier())
  {
    availableVec.push_back(fos::FosCommonUtil::CarrierMapElem(
        cxrElem.first, cxrElem.second - _numAlreadyGeneratedNSPerCarrier[cxrElem.first]));
  }

  fos::FosCommonUtil::diversifyOptionsNumPerCarrier(
      totalOptionsRequired, availableVec, requiredMap);
}

} // tse
