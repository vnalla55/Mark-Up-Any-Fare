#include "DataModel/CxrFareMarkets.h"

#include "DataModel/OwrtFareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataHandle.h"

#include <algorithm>

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

struct OWRTFareMarketLess
{
  bool operator()(OwrtFareMarketPtr lhp, OwrtFareMarketPtr rhp)
  {
    return lhp->lowerBound() < rhp->lowerBound();
  }
};
} // namespace

CxrFareMarketsPtr
CxrFareMarkets::create(Trx& trx, SolutionType solutionType)
{
  return CxrFareMarketsPtr(&trx.dataHandle().safe_create<CxrFareMarkets>(solutionType),
                           NullDeleter());
}

void
CxrFareMarkets::insert(OwrtFareMarketPtr owrtFM)
{
  if (LIKELY(owrtFM->getType() == _solutionType))
  {
    iterator it = std::lower_bound(begin(), end(), owrtFM, OWRTFareMarketLess());
    _owrtFareMarkets.insert(it, owrtFM);
  }
}

MoneyAmount
CxrFareMarkets::lowerBound() const
{
  return (*begin())->lowerBound();
}

const LocationPair&
CxrFareMarkets::getOriginLocation() const
{
  return (*begin())->getOriginLocation();
}
const LocationPair&
CxrFareMarkets::getDestinationLocation() const
{
  return (*begin())->getDestinationLocation();
}

const Fare*
CxrFareMarkets::getLowerBoundFare() const
{
  const const_iterator::value_type owrtFm(*begin());
  const OwrtFareMarket::const_iterator it(owrtFm->begin());

  return (it != owrtFm->end() && *it) ? (*it)->fare() : nullptr;
}
}
} // naemspace tse::shpq
