#include "DataModel/OwrtFareMarket.h"

#include "DataModel/PaxTypeFare.h"
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
}

OwrtFareMarketPtr
OwrtFareMarket::create(Trx& trx,
                       SolutionType solutionType,
                       FareMarket* fareMarket,
                       ItinIndex::Key cxrIndexKey)
{
  return OwrtFareMarketPtr(
      &trx.dataHandle().safe_create<OwrtFareMarket>(solutionType, fareMarket, cxrIndexKey),
      NullDeleter());
}

MoneyAmount
OwrtFareMarket::lowerBound() const
{
  return (*begin())->nucFareAmount();
}

const Fare*
OwrtFareMarket::getLowerBoundFare() const
{
  return (*begin())->fare();
}

std::string
OwrtFareMarket::str() const
{
  const std::string separator("-");
  return getOriginLocation().toString() + separator + getDestinationLocation().toString();
}

void
OwrtFareMarket::analyzeValidFares()
{
  for (const auto elem : _paxTypeFares)
  {
    _hasNormal = _hasNormal || elem->isNormal();
    _hasSpecial = _hasSpecial || elem->isSpecial();
    _hasTag1 = _hasTag1 || (elem->owrt() == ONE_WAY_MAY_BE_DOUBLED);
    _hasTag2 = _hasTag2 || (elem->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED);
    _hasTag3 = _hasTag3 || (elem->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED);
  }
}
}
} // namespace tse::shpq
