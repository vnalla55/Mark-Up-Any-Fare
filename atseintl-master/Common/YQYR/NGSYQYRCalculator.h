// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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

#include "Common/YQYR/ShoppingYQYRCalculator.h"

#include <boost/functional/hash.hpp>

namespace tse
{
class NGSYQYRCalculatorTest;

struct LocationHasher
{
  size_t operator()(const Loc* location) const { return hash_value(location->loc()); }
};

namespace YQYR
{
class NGSYQYRCalculator : public ShoppingYQYRCalculator
{
  friend class ::tse::NGSYQYRCalculatorTest;

public:
  NGSYQYRCalculator(ShoppingTrx& trx) : ShoppingYQYRCalculator(trx) {}
  void init();

  void initializeFareMarketCarrierStorage(FareMarket* fareMarket,
                                          const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                                          const std::vector<PaxTypeFare*>& applicableFares);

  bool calculateYQYRs(const CarrierCode validatingCarrier,
                      const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                      const PaxTypeFare* fare,
                      FareMarket* fareMarket,
                      FarePath* farePath,
                      const uint32_t sopIndex,
                      YQYRCalculator::YQYRFeesApplicationVec& results);

private:
  typedef std::pair<uint32_t, uint16_t> SopAndLegPair;
  typedef std::unordered_map<const Loc*,
                             std::unordered_set<SopAndLegPair, boost::hash<SopAndLegPair>>,
                             LocationHasher> FurthestPointToSOPsMap; // all sops applicable for
                                                                     // this furthest point

  const Loc* determineFurthestPoints(FurthestPointToSOPsMap& result);
  template <class IteratorType>
  const Loc* determineFurthestPoint(IteratorType begin,
                                    IteratorType end,
                                    const AirSeg* journeyOrigin,
                                    const Itin* itin,
                                    const bool isOutbound) const;
  const AirSeg* determineJourneyOrigin(const std::vector<TravelSeg*>& travelSegs);

  void createBuckets(const FurthestPointToSOPsMap& map, std::vector<YQYRBucket>& buckets);

  void printDiagHeader(const FurthestPointToSOPsMap& map);

  YQYRCalculator::Directions determineDirection(const FareMarket& fm, const Loc* furthestLoc) const;
  bool useGlobalReturnsToOrigin(const FareMarket& fm, const FarePath& fp) const;

private:
  typedef std::unordered_map<SopAndLegPair, uint16_t, boost::hash<SopAndLegPair>>
  SopToBucketIndexMap;
  SopToBucketIndexMap _sopToBucketIndex;
  bool _globalReturnsToOrigin;
};
}
}
