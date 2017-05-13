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
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{
namespace shpq
{
class SoloPQItem;
}

class DmcBrandedFaresAllSopsRequirement : public DmcRequirement
{
public:
  DmcBrandedFaresAllSopsRequirement(DmcRequirementsSharedContext& sharedCtx);

  Value getStatus() const;
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;
  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price) const;

private:
  bool isSopIdUnused(size_t legIdx, size_t sopId) const;

  DmcRequirementsSharedContext& _sharedCtx;
  bool _isOneWay;
  bool _enabled;
};

} // ns tse

