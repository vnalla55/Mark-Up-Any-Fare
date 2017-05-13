// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include <set>
#include <vector>

namespace tse
{
class TravelSeg;
class ShoppingTrx;
class Loc;
class Logger;

namespace shpq
{

class ASOCandidateChecker
{
  friend class ASOCandidateCheckerDerived;

public:
  ASOCandidateChecker(ShoppingTrx& trx);

  // TODO lowpri: fix by-val std::vector<int> to shpq::SopIdxVecArg
  bool match(std::vector<int> sops) const;

private:
  uint32_t getMileage(const Loc& loc1, const Loc& loc2) const;
  std::set<LocCode> collectIntermediatePoints(std::vector<TravelSeg*>& segments) const;

  bool isDiamond(std::vector<int> sops) const;
  bool checkDistance(const Loc& loc1, const Loc& loc2) const;
  bool checkLegsOverlap(std::vector<int> sops) const;
  bool isLimitHintingWithinZ021ForDomCncxTurnAround(const std::vector<int>& sops) const;

  ShoppingTrx& _trx;
  uint32_t _asoCandidateMileage;
  static Logger _logger;
};
}
} // namespace tse::shpq

