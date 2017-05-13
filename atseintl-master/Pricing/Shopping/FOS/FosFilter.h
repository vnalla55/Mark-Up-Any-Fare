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
#include "Pricing/Shopping/FOS/FosTypes.h"

namespace tse
{
namespace fos
{
class FosStatistic;

class FosFilter
{
public:
  virtual ~FosFilter() {}

  virtual FilterType getType() const = 0;
  // which validators are possible to accept any solution returned by this filter
  virtual ValidatorBitMask getAffectedValidators() const { return ~ValidatorBitMask(0); }
  virtual bool isApplicableSolution(const SopCombination& sopCombination) const = 0;
  virtual bool isApplicableSop() const { return true; }

  virtual const SopDetailsPtrVec& getFilteredSopDetails(const DetailedSop& orginal)
  {
    return orginal.getSopDetailsVec();
  }

  virtual bool isCombinationForReuse(const SopCombination& sopCombination) const { return false; }

  virtual bool isFilterToPop(const FosStatistic&) const { return false; }
};

} // fos
} // tse
