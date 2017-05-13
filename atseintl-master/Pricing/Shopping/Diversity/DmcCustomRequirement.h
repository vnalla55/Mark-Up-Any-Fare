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

#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Diversity.h"
#include "Diagnostic/Diag941Collector.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{
namespace shpq
{
class SoloPQItem;
}

class DmcCustomRequirement : public DmcRequirement
{
public:
  DmcCustomRequirement(DmcRequirementsSharedContext& sharedCtx)
    : _isFareCutoffReached(false), _sharedCtx(sharedCtx), _dc(nullptr)
  {
    if (sharedCtx._dc != nullptr)
      _dc = dynamic_cast<Diag941Collector*>(sharedCtx._dc);
  }

  bool getThrowAwayCombination(shpq::SopIdxVecArg comb);

  Value getStatus() const
  {
    return (_sharedCtx._stats.getCustomSolutionCount() <
            _sharedCtx._diversity.getMinimalNumCustomSolutions())
               ? NEED_CUSTOM
               : 0;
  }

  /**
   * To decrease false-positive results (thus improve performance by skipping not useful SoloPQ
   * items),
   * the same approach as in DmcNSRequirementCommonCheck::getPQItemCouldSatisfy can be used
   */
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;

  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb) const;

  void setFareCutoffReached() { _isFareCutoffReached = true; }

  void print();

private:
  bool _isFareCutoffReached;
  DmcRequirementsSharedContext& _sharedCtx;
  Diag941Collector* _dc;
};

} // ns tse

