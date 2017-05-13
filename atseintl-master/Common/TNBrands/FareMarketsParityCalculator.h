//-------------------------------------------------------------------
//
//  Authors:     Andrzej Fediuk
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/FallbackUtil.h"
#include "Common/TNBrands/BrandedFareMarket.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "Common/TNBrands/TNBrandsInterfaces.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "DataModel/TNBrandsTypes.h"

#include <boost/scoped_ptr.hpp>

#include <map>
#include <memory>
#include <set>

namespace tse
{
FALLBACK_DECL(fallbackIbfParityThruMarketsFix)

namespace skipper
{
// Recursively traverse through all possible fare market paths (using dynamic
// programming algorithm) and calculate possible brands parity.
template <typename ItinGeometryCalculatorT = ItinGeometryCalculator,
          typename BrandedFareMarketFactoryT = Factory<BrandedFareMarket<> > >
class FareMarketsParityCalculator : public IFareMarketsParityCalculator
{
public:
  // itinGeometryCalculator corresponds to the itin in which we are
  // calculating the parity. Only fare markets from this itn will be used.
  // selfCalculator and factory are needed only for testing/mock purposes.
  FareMarketsParityCalculator(
      const ItinGeometryCalculatorT& itinGeometryCalculator,
      IFareMarketsParityCalculator* selfCalculator = nullptr,
      BrandedFareMarketFactoryT* factory = 0) :
        _itinGeometryCalculator(itinGeometryCalculator)
  {
    if (selfCalculator == nullptr)
    {
      _selfCalculator = this;
    }
    else
    {
      _selfCalculator = selfCalculator;
    }

    assignValidObject(factory, _factory);
    _segmentCount = itinGeometryCalculator.getSegmentCount();
  }

  // Creates BrandedFareMarket object (based on itinGeometryCalculator given
  // in constructor and fareMarket) and stores it in internal structures.
  void addFareMarket(const FareMarket& fareMarket) override
  {
    const PricingTrx& trx = _itinGeometryCalculator.getTrxGeometryCalculator().getTrx();
    if (!fallback::fallbackIbfParityThruMarketsFix(&trx) &&
        _itinGeometryCalculator.isThruFareOnlyItin() &&
        !_itinGeometryCalculator.isThruFareMarket(fareMarket))
    {
      return;
    }
    BrandedFareMarketPtr brandedFareMarketPtr(
        _factory->create(_itinGeometryCalculator, fareMarket));
    _fmsPerSegment[brandedFareMarketPtr->getStartSegmentIndex()]
                   .insert(brandedFareMarketPtr);
  }

  // Recursively called method which traverse all possible fare market paths
  // (using dynamic programming algorithm) to calculating brand parity.
  // The brand information itself is not enough, we need to also return fare
  // markets leading to this solution. For this reason result of this call
  // contains map with brands fulfilling parity requirement as a keys and
  // set of fare markets which, in various combinations, can be used to create
  // fare market path for this brand.
  //
  // startSegmentIndex indicates the index of travel segment in which we want
  // to find brands matching parity requirements. endSegmentIndex defines how
  // "far" we want to look for parity.
  // If local cache contains solution for this startSegmentIndex it is returned
  // immediately.
  // If not, all fare markets starting in the considered segment of travel are
  // considered. For each such fare market verify if it "saturates" the whole
  // travel and if so store all of its brands as an answer and proceed to the
  // next fare market. If the fare market does not end the travel, call this
  // function recursively with the index of the last segment covered by this
  // fare market, requesting all brands fulfilling the parity. Then intersect
  // the response with fare market's brands and add the result to the answer.
  // After all possible fare markets were check store the result in the local
  // cache and return it.
  // NOTE: When index of the fare market's end segment is considered,
  // additional check is made if the next segment after this fare market is an
  // arunk. If so, this segment is skipped.
  FareMarketsPerBrandCode possibleBrands(size_t startSegmentIndex, size_t endSegmentIndex) override;

  // default use of possibleBrands - looks for possible brands from the given
  // startSegmentIndex until the end of the journey (until _segmentCount)
  FareMarketsPerBrandCode possibleBrands(size_t startSegmentIndex) override
  {
    return _selfCalculator->possibleBrands(startSegmentIndex, _segmentCount);
  }

private:
  typedef typename BrandedFareMarketFactoryT::Type BrandedFareMarket;
  typedef std::shared_ptr<BrandedFareMarket> BrandedFareMarketPtr;

  typedef std::set<BrandedFareMarketPtr> BrandedFareMarketSet;
  typedef std::map<size_t, BrandedFareMarketSet> FareMarketsPerSegment;

  typedef std::map<size_t, FareMarketsPerBrandCode> CacheMap;
  typedef std::map<size_t,
      std::pair<FareMarketsPerBrandCode, bool> > CacheMapWithFixedLegs;

  const ItinGeometryCalculatorT& _itinGeometryCalculator;
  boost::scoped_ptr<BrandedFareMarketFactoryT> _factory;
  IFareMarketsParityCalculator* _selfCalculator;

  FareMarketsPerSegment _fmsPerSegment;
  CacheMap _cache;
  size_t _segmentCount;
};

template <typename ItinGeometryCalculatorT, typename FareMarketsPropertiesT>
FareMarketsPerBrandCode FareMarketsParityCalculator<ItinGeometryCalculatorT,
  FareMarketsPropertiesT>::possibleBrands(size_t startSegmentIndex, size_t endSegmentIndex)
{
  TSE_ASSERT(endSegmentIndex <= _segmentCount);

  // check if this part of problem is already known (dynamic programming)
  // if so, return solution
  CacheMap::const_iterator cacheFound = _cache.find(startSegmentIndex);
  if (cacheFound != _cache.end())
  {
    return cacheFound->second;
  }

  FareMarketsPerBrandCode parityBrands;
  typename FareMarketsPerSegment::const_iterator newSegmentFound
    = _fmsPerSegment.find(startSegmentIndex);

  if (newSegmentFound == _fmsPerSegment.end())
  {
    // There are no fare markets starting at this segment. An empty brand set will be returned.
    // We can still calculate brand parity at this segment based on thru markets that include it.
    _cache[startSegmentIndex] = parityBrands;
    return parityBrands;
  }

  for (const BrandedFareMarketPtr& fm : _fmsPerSegment[startSegmentIndex])
  {
    //move to the next travel part after end of this fare market
    size_t nextStartingSegment = fm->getEndSegmentIndex() + 1;
    nextStartingSegment =
        _itinGeometryCalculator.getNextTravelSegmentIfCurrentArunk(nextStartingSegment);
    const UnorderedBrandCodes fmBrands = fm->getBrands();
    if (nextStartingSegment >= endSegmentIndex)
    {
      //parity calculation is done - we are at the end of the fare market path
      //just store all available brands - parity will be calculated on the
      //"higher" level (higher recursion levels).
      UnorderedBrandCodes::const_iterator iter = fmBrands.begin();
      for (; iter != fmBrands.end(); ++iter)
      {
        parityBrands[*iter].insert(fm->getFareMarket());
      }
    }
    else
    {
      //not the end of the fare market path - get brands matching parity on
      //the rest of the path (or rather all possible paths) using recursive call
      FareMarketsPerBrandCode brandsAndFareMarketsFromEndSegment =
          _selfCalculator->possibleBrands(nextStartingSegment, endSegmentIndex);
      UnorderedBrandCodes brandsFromEndSegment;
      FareMarketsPerBrandCode::const_iterator iter =
          brandsAndFareMarketsFromEndSegment.begin();
      for (; iter != brandsAndFareMarketsFromEndSegment.end(); ++iter)
      {
        brandsFromEndSegment.insert(iter->first);
      }
      //calculate parity -> intersection
      UnorderedBrandCodes intersection;
      std::set_intersection(
          brandsFromEndSegment.begin(),
          brandsFromEndSegment.end(),
          fmBrands.begin(),
          fmBrands.end(),
          std::inserter(intersection, intersection.begin()));
      if (intersection.size() > 0)
      {
        UnorderedBrandCodes::const_iterator iter = intersection.begin();
        for (; iter != intersection.end(); ++iter)
        {
          parityBrands[*iter].insert(
              brandsAndFareMarketsFromEndSegment[*iter].begin(),
              brandsAndFareMarketsFromEndSegment[*iter].end());
          parityBrands[*iter].insert(fm->getFareMarket());
        }
      }
    }
  }
  // store calculated solution for future reference (dynamic programming)
  _cache[startSegmentIndex] = parityBrands;
  return parityBrands;
}

} // namespace skipper

} // namespace tse

