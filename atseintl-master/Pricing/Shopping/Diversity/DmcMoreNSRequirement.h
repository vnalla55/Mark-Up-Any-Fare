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

#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"

namespace tse
{

class DmcMoreNSRequirement : public DmcRequirement
{
  friend class DmcMoreNSRequirementTest;

public:
  DmcMoreNSRequirement(DmcRequirementsSharedContext& sharedCtx);

  Value getStatus() const;

  Value getCouldSatisfyAdjustment(Value allInOneRequirements) const;

private:
  DmcRequirementsSharedContext& _sharedCtx;
};

} // ns tse

