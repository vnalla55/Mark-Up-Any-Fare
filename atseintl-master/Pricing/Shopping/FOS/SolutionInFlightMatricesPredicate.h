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
#pragma once

#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

namespace tse
{
class ShoppingTrx;

namespace fos
{

class SolutionInFlightMatricesPredicate : public tse::utils::IPredicate<SopCombination>
{
public:
  typedef std::set<SopCombination> CombinationSet;
  typedef std::vector<SopCombination> CombinationVector;

  explicit SolutionInFlightMatricesPredicate(const ShoppingTrx& trx) { initCombinationSet(trx); }

  bool operator()(const SopCombination& sopIds) override
  {
    return _combinationSet.find(sopIds) == _combinationSet.end();
  }

protected:
  void initCombinationSet(const ShoppingTrx& trx);

private:
  CombinationSet _combinationSet;
};

} // namespace fos
} // namespace tse
