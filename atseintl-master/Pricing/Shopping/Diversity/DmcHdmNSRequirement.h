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
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{

class DmcHdmNSRequirement : public DmcRequirement
{
public:
  DmcHdmNSRequirement(DmcRequirementsSharedContext& sharedCtx)
    : _enabled(sharedCtx._diversity.isHighDensityMarket() ? NEED_NONSTOPS : 0)
  {
  }

  Value getStatus() const { return _enabled; }

  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const { return _enabled; }

  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price) const
  {
    return _enabled;
  }

private:
  Value _enabled;
};

} // ns tse

