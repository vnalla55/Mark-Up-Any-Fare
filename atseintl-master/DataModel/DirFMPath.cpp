#include "DataModel/DirFMPath.h"

#include "DataModel/CxrFareMarkets.h"
#include "DataModel/Fare.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"

namespace tse
{
namespace shpq
{

namespace
{

struct NullDeleter
{
  void operator()(void*) {}
};

struct LowerBoundCount
{
  void operator()(const CxrFareMarketsPtr cxrFM) { _lowerBound += cxrFM->lowerBound(); }
  MoneyAmount lowerBound() { return _lowerBound; }

private:
  MoneyAmount _lowerBound = 0;
};

} // namespace

DirFMPathPtr
DirFMPath::create(Trx& trx, CxrFareMarketsPtr firstSeg, CxrFareMarketsPtr secondSeg)
{
  return DirFMPathPtr(&trx.dataHandle().safe_create<DirFMPath>(firstSeg, secondSeg), NullDeleter());
}

MoneyAmount
DirFMPath::lowerBound() const
{
  return std::for_each(_segments.begin(), _segments.end(), LowerBoundCount()).lowerBound();
}

void
DirFMPath::setSolutionType(CxrFareMarketsPtr firstSeg, CxrFareMarketsPtr secondSeg /*default = 0*/)
{
  _solutionType = firstSeg->getType();
  if (secondSeg)
  {
    if (_solutionType == OW)
    {
      if (secondSeg->getType() == OW)
        _solutionType = OW_OW;
      else if (secondSeg->getType() == HRT)
        _solutionType = OW_HRT;
    }
    else if (LIKELY(_solutionType == HRT))
    {
      if (secondSeg->getType() == OW)
        _solutionType = HRT_OW;
      else if (secondSeg->getType() == HRT)
        _solutionType = HRT_HRT;
    }
  }
}

std::string
DirFMPath::str(const bool includeLowestFareInfo /* = false */) const
{
  std::string result;
  std::string separator("");

  for (const CxrFareMarketsPtr& seg : _segments)
  {
    result += separator;
    result += seg->getOriginLocation().toString();
    result += '-';
    result += seg->getDestinationLocation().toString();
    if (includeLowestFareInfo)
    {
      const Fare* const fare(seg->getLowerBoundFare());
      if (fare)
        result += '-' + fare->carrier() + '-' + fare->fareClass();
    }
    separator = "/";
  }
  return result;
}
}
} // namespcae tse::shpq
