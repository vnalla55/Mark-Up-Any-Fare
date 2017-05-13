// ----------------------------------------------------------------
//
//   Author: Michal Mlynek
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

class DmcBrandedFaresTakeAllRequirement : public DmcRequirement
{
public:
  DmcBrandedFaresTakeAllRequirement(DmcRequirementsSharedContext& sharedCtx);

  Value getStatus() const;
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;
  Value getCombinationCouldSatisfy(const shpq::SopIdxVecArg comb, MoneyAmount price) const;

private:
  DmcRequirementsSharedContext& _sharedCtx;
  bool _enabled;
  unsigned int _optionsRequested;
};

} // ns tse

