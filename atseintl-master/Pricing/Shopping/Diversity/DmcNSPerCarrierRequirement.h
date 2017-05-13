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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Diversity.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{
namespace shpq
{
class SoloPQItem;
}

class DmcNSPerCarrierRequirement : public DmcRequirement
{
public:
  friend class DmcNSPerCarrierRequirementTest;

  DmcNSPerCarrierRequirement(DmcRequirementsSharedContext& sharedCtx);

  bool isEffective() const
  {
    return (_isEnabled && _sharedCtx._diversity.isDirectCarriersCapable());
  }

  Value getStatus() const;
  Value getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;
  Value getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, MoneyAmount price) const;

  bool isOptNeededForNSCarrier(CarrierCode cxr) const;
  void print() const;

private:
  bool isOptNeededForNSCarrier() const;

  const bool _isEnabled;
  DmcRequirementsSharedContext& _sharedCtx;
};

} // ns tse

