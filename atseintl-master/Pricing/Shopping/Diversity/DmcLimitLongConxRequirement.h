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

#include "Diagnostic/Diag941Collector.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{

class DmcLimitLongConxRequirement : public DmcRequirement
{
public:
  DmcLimitLongConxRequirement(DmcRequirementsSharedContext& sharedCtx)
    : _sharedCtx(sharedCtx), _dc(nullptr)
  {
    if (sharedCtx._dc != nullptr)
      _dc = dynamic_cast<Diag941Collector*>(sharedCtx._dc);
  }

  bool getThrowAwayCombination(shpq::SopIdxVecArg comb);
  bool getThrowAwaySop(std::size_t legId, int sopId) const;

private:
  DmcRequirementsSharedContext& _sharedCtx;
  Diag941Collector* _dc;
};

} // ns tse

