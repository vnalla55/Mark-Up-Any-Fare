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

#include "Pricing/Shopping/FOS/DetailedSop.h"
#include "Pricing/Shopping/FOS/FosFilter.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

#include <queue>

namespace tse
{
namespace fos
{

class FosFilterComposite : public FosFilter
{
public:
  FosFilterComposite(const FosStatistic& stat) : _fosStatistic(stat) {}

  FilterType getType() const override { return FILTER_COMPOSITE; }

  ValidatorBitMask getAffectedValidators() const override
  {
    return _fosFilter.empty() ? ~ValidatorBitMask(0) : _fosFilter.front()->getAffectedValidators();
  }

  void addFilter(FosFilter& filter) { _fosFilter.push(&filter); }
  void pop()
  {
    if (!_fosFilter.empty())
      _fosFilter.pop();
  }
  bool empty() const { return _fosFilter.empty(); }
  bool isLastFilter() const { return _fosFilter.size() == 1; }

  bool isApplicableSolution(const SopCombination& sopCombination) const override
  {
    return _fosFilter.empty() || _fosFilter.front()->isApplicableSolution(sopCombination);
  }

  bool isApplicableSop() const override
  {
    return _fosFilter.empty() || _fosFilter.front()->isApplicableSop();
  }

  bool isCombinationForReuse(const SopCombination& sopCombination) const override
  {
    return _fosFilter.size() > 1 && _fosFilter.front()->isCombinationForReuse(sopCombination);
  }

  const SopDetailsPtrVec& getFilteredSopDetails(const DetailedSop& orginal) override
  {
    return _fosFilter.empty() ? orginal.getSopDetailsVec()
                              : _fosFilter.front()->getFilteredSopDetails(orginal);
  }

  bool isFilterToPop()
  {
    return (!_fosFilter.empty() && _fosFilter.front()->isFilterToPop(_fosStatistic));
  }

private:
  std::queue<FosFilter*> _fosFilter;

  const FosStatistic& _fosStatistic;
};

} // fos
} // tse
