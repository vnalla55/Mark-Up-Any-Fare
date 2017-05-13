#pragma once

#include <tr1/unordered_set>
#include <cstddef>

namespace tse
{
class ConstructedFareInfo;
class PricingTrx;

class ConstructedFareInfoResponse
{
public:
  using HashFunc = size_t(*)(const ConstructedFareInfo*);
  using EqualFunc = bool(*)(const ConstructedFareInfo*, const ConstructedFareInfo*);
  using ConstructedFareInfoHashSet = std::tr1::unordered_set<ConstructedFareInfo*, HashFunc, EqualFunc>;

  ConstructedFareInfoHashSet& responseHashSet() { return _responseHashSet; }
  const ConstructedFareInfoHashSet& responseHashSet() const { return _responseHashSet; }

  ConstructedFareInfoResponse(const PricingTrx& trx);

private:
  ConstructedFareInfoHashSet _responseHashSet;
};
}
