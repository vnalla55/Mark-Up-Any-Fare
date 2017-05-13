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

#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"
#include "Pricing/Shopping/PQ/SOPInfo.h"

#include <boost/intrusive/list_hook.hpp>

namespace tse
{
class ShoppingTrx;

namespace shpq
{

struct SOPCombination : public boost::intrusive::list_base_hook<
                            boost::intrusive::link_mode<boost::intrusive::normal_link> >
{
  friend class SOPCombinationList;
  friend class SOPCombinationListTest;

  SOPCombination(const SoloPQItem& pqItem) : spId(pqItem.getSolPattern()->getSPId()) {}

  SOPCombination(const ShoppingTrx& trx,
                 const SoloPQItem& pqItem,
                 const std::vector<SOPInfo>& combination);

  SOPCombination() = default;

  bool operator==(const SOPCombination& rhs) const { return oSopVec == rhs.oSopVec; }

  void updateForAltDates(const ShoppingTrx& trx, const std::vector<SOPInfo>& combination);

public:
  SopIdxVec oSopVec;
  char status = 0;
  const DatePair* datePair = nullptr;
  uint64_t duration = 0;
  SolutionPattern::SPEnumType spId = SolutionPattern::SPEnumType::SP10;

private:
  bool _isLinked = false;
  SOPCombination* _next[MAX_NUMBER_LEGS];
};
}
} // ns tse::shpq

