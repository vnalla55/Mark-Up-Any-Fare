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

#include "Pricing/Shopping/FOS/SolutionInFlightMatricesPredicate.h"

#include "DataModel/ShoppingTrx.h"
#include "Pricing/ShoppingPQ.h"

namespace tse
{
namespace fos
{

void
SolutionInFlightMatricesPredicate::initCombinationSet(const ShoppingTrx& trx)
{
  size_t combinationsCount = 0u;

  for (const ShoppingTrx::PQPtr& pq : trx.shoppingPQVector())
  {
    combinationsCount += pq->getFlightMatrix().size() + pq->getEstimateMatrix().size();
  }

  CombinationVector combinations(combinationsCount);

  for (const ShoppingTrx::PQPtr& pq : trx.shoppingPQVector())
  {
    for (const ShoppingTrx::FlightMatrix::value_type& comb : pq->getFlightMatrix())
    {
      combinations.push_back(comb.first);
    }

    for (const ShoppingTrx::EstimateMatrix::value_type& comb : pq->getEstimateMatrix())
    {
      combinations.push_back(comb.first);
    }
  }

  _combinationSet.clear();
  _combinationSet.insert(combinations.begin(), combinations.end());
}

} // namespace fos
} // namespace tse
