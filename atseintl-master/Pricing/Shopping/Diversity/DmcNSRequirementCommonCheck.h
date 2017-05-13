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
#include "DataModel/ItinIndex.h"
#include "Pricing/Shopping/Diversity/DiversityModel.h"
#include "Pricing/Shopping/Diversity/DmcRequirement.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

namespace tse
{
struct SOPInfo;

namespace shpq
{
class SoloPQItem;
}

class DmcNSRequirementCommonCheck
{
public:
  DmcNSRequirementCommonCheck(DmcRequirementsSharedContext& sharedCtx) : _sharedCtx(sharedCtx) {}

  /**
   * The most lightweight prevalidation check for any non-stop operation, must be called the first
   */
  bool canPQProduceMoreNonStop() const;
  bool getPQItemCouldSatisfy(const shpq::SoloPQItem* pqItem) const;
  bool getCombinationCouldSatisfy(shpq::SopIdxVecArg comb, DmcRequirement::Value) const;

  bool getSopCouldSatisfy(unsigned legId,
                          const SOPInfo& sopInfo,
                          const DmcRequirement::SopInfosStatistics& statistics) const;

private:
  bool couldHaveMore(const shpq::SoloPQItem* pqItem) const;
  bool couldHaveMore(CarrierCode cxr) const;

  DmcRequirementsSharedContext& _sharedCtx;
};

} // ns tse

