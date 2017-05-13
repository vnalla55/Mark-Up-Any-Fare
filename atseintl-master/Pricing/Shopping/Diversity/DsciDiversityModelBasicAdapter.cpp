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

#include "Pricing/Shopping/Diversity/DsciDiversityModelBasicAdapter.h"

#include "DataModel/Diversity.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Diversity/DmcRequirementsFacade.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SopCombinationUtil.h"

namespace tse
{

int
DsciDiversityModelBasicAdapter::isBetterByRequirementsAndStatus(DmcRequirementsFacade* requirements,
                                                                double score,
                                                                const Operand& lhs,
                                                                const Operand& rhs)
{
  _lastObtainedCouldSatisfy = -1;

  if (lhs.status > rhs.status)
    return LESS;

  _lastObtainedCouldSatisfy = requirements->getCombinationCouldSatisfy(lhs.oSopVec, score);

  if (lhs.status < rhs.status && _lastObtainedCouldSatisfy)
  {
    _bestRequirements = _lastObtainedCouldSatisfy;
    return GREATER;
  }

  if (!_lastObtainedCouldSatisfy || _lastObtainedCouldSatisfy < _bestRequirements)
    return LESS;

  if (_lastObtainedCouldSatisfy > _bestRequirements)
  {
    _bestRequirements = _lastObtainedCouldSatisfy;
    return GREATER;
  }

  return EQUAL;
}

int
DsciDiversityModelBasicAdapter::isBetterByPairing(const Operand& lhs, const Operand& rhs)
{
  size_t lhsIBNeeded = _stats.getInboundOptionsNeeded(lhs.oSopVec[0]);
  size_t rhsIBNeeded = _stats.getInboundOptionsNeeded(rhs.oSopVec[0]);

  if (lhsIBNeeded > rhsIBNeeded)
    return GREATER;
  if (lhsIBNeeded < rhsIBNeeded)
    return LESS;

  return EQUAL;
}

int
DsciDiversityModelBasicAdapter::isBetterByNonStop(const Operand& lhs, const Operand& rhs)
{
  bool lhsNS =
      (SopCombinationUtil::detectNonStop(_trx, lhs.oSopVec) & SopCombinationUtil::ONLINE_NON_STOP);
  bool rhsNS =
      (SopCombinationUtil::detectNonStop(_trx, rhs.oSopVec) & SopCombinationUtil::ONLINE_NON_STOP);

  if (lhsNS == rhsNS)
  {
    return EQUAL;
  }
  return lhsNS > rhsNS ? GREATER : LESS;
}

int
DsciDiversityModelBasicAdapter::isBetterByTOD(const Operand& lhs, const Operand& rhs)
{
  const std::vector<std::pair<uint16_t, uint16_t> >& todRanges = _trx.diversity().getTODRanges();
  const Itin* lhsItin = _trx.legs()[0].sop()[lhs.oSopVec[0]].itin();
  const Itin* rhsItin = _trx.legs()[0].sop()[rhs.oSopVec[0]].itin();

  const std::size_t lhsSize = _stats.getOptionsNeededForTODBucket(lhsItin->getTODBucket(todRanges));
  const std::size_t rhsSize = _stats.getOptionsNeededForTODBucket(rhsItin->getTODBucket(todRanges));
  if (lhsSize > rhsSize)
  {
    return GREATER;
  }

  if (lhsSize < rhsSize)
  {
    return LESS;
  }

  return EQUAL;
}

} // tse
