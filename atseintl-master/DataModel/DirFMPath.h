// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Common/ShpqTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <memory>
#include <vector>

namespace tse
{

class Trx;

namespace shpq
{

class CxrFareMarkets;
class DirFMPath;
typedef std::shared_ptr<DirFMPath> DirFMPathPtr;

class DirFMPath
{
  friend class CxrFMCollectorTest;

  typedef std::shared_ptr<CxrFareMarkets> CxrFareMarketsPtr;
  typedef std::vector<CxrFareMarketsPtr> CxrFMVector;

public:
  typedef CxrFMVector::iterator iterator;
  typedef CxrFMVector::const_iterator const_iterator;

  static DirFMPathPtr
  create(Trx&, CxrFareMarketsPtr, CxrFareMarketsPtr secondSeg = CxrFareMarketsPtr());
  // use create() instead of constructor
  explicit DirFMPath(CxrFareMarketsPtr firstSeg, CxrFareMarketsPtr secondSeg = CxrFareMarketsPtr())
  {
    _segments.push_back(firstSeg);
    if (secondSeg)
      _segments.push_back(secondSeg);
    setSolutionType(firstSeg, secondSeg);
  }

  iterator begin() { return _segments.begin(); }
  iterator end() { return _segments.end(); }

  const_iterator begin() const { return _segments.begin(); }
  const_iterator end() const { return _segments.end(); }

  MoneyAmount lowerBound() const;
  SolutionType getType() const { return _solutionType; }

  std::string str(const bool includeLowestFareInfo = false) const;

private:
  void
  setSolutionType(CxrFareMarketsPtr firstSeg, CxrFareMarketsPtr secondSeg = CxrFareMarketsPtr());

private:
  CxrFMVector _segments; // up to 2
  SolutionType _solutionType = SolutionType::NONE;
};
}
} // namespace tse::shpq

