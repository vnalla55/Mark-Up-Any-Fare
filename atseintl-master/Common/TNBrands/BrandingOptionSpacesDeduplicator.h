//-------------------------------------------------------------------
//
//  Authors:     Artur de Sousa Rocha
//
//  Copyright Sabre 2015
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

#include "Common/TNBrands/TNBrandsUtils.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/PricingUnit.h"

#include <boost/scoped_ptr.hpp>

namespace tse
{
class PricingTrx;

namespace skipper
{

const uint16_t CHEAPEST_OPTION_INDEX = 0;

// Helper class for FarePathBrandKeyBuilder.
class FareUsageBrandFinder
{
public:

  // Returns a pair (governing carrier, brand code) for the brand used by the fare
  // in the specified FareUsage.
  GoverningCarrierBrand findBrand(const PricingTrx& trx,
                                  const Itin& itin,
                                  const FarePath& farePath,
                                  const FareUsage& fu) const;
};


// Helper class for building fare path keys that will be compared between
// different price options.
template <typename FareUsageBrandFinderT = FareUsageBrandFinder>
class FarePathBrandKeyBuilder
{
public:
  using FarePathBrandKey = std::vector<GoverningCarrierBrand>;

  FarePathBrandKeyBuilder(FareUsageBrandFinderT* fuBrandFinder = nullptr)
  {
    assignValidObject(fuBrandFinder, _fuBrandFinder);
  }

  // Returns the key consisting of (carrier, brand) pairs for every air segment
  // in the fare path.
  //
  // Brand codes are retrieved from fares used in the fare path.
  FarePathBrandKey buildKey(const PricingTrx& trx, const Itin& itin, const FarePath& farePath) const
  {
    FarePathBrandKey result(itin.travelSeg().size());

    for (const PricingUnit* pu : farePath.pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        const GoverningCarrierBrand brand = _fuBrandFinder->findBrand(trx, itin, farePath, *fu);
        for (const TravelSeg* seg : fu->travelSeg())
        {
          if (seg->isAir())
          {
            // Depending on how PU path was built, FareUsages retrieved by PU don't have to be
            // in order and therefore the segments retrieved from FareUsages would not be in order
            // either.
            //
            // For a snowman construction:
            //
            //   PU1   PU2
            //
            //   FU1   FU2
            //  ----- -----
            // A     B     C
            //  ----- -----
            //   FU4   FU3
            //
            // If this is a 4-segment itinerary, the order of retrieved segments is (1, 4, 2, 3).
            const int16_t segmentIndex = itin.segmentOrder(seg) - 1;
            TSE_ASSERT(segmentIndex >= 0 && size_t(segmentIndex) < result.size());
            result[segmentIndex] = brand;
          }
        }
      }
    }

    return result;
  }

private:
  boost::scoped_ptr<FareUsageBrandFinderT> _fuBrandFinder;
};

// Helper class for building fare path keys that will be compared between
// different price options.
template <typename FareUsageBrandFinderT = FareUsageBrandFinder>
class FarePathBrandKeyWithPaxTypeBuilder
{
public:
  using FarePathBrandKey = std::pair<const PaxType*, std::vector<GoverningCarrierBrand>>;

  FarePathBrandKeyWithPaxTypeBuilder(FareUsageBrandFinderT* fuBrandFinder = nullptr)
    : farePathBrandKeyBuilder_(fuBrandFinder)
  {
  }

  // Returns the key consisting of pair (paxType, vec<SegmentPair>)
  // where SegmentPair is a pair (carrier, brand) for every air segment
  // in the fare path.
  //
  // Brand codes are retrieved from fares used in the fare path.
  FarePathBrandKey buildKey(const PricingTrx& trx,
                            const Itin& itin,
                            const FarePath& farePath) const
  {
    return FarePathBrandKey(farePath.paxType(),
                            farePathBrandKeyBuilder_.buildKey(trx, itin, farePath));
  }

private:
  FarePathBrandKeyBuilder<FareUsageBrandFinderT> farePathBrandKeyBuilder_;
};

// Class for detecting if the cheapest "use whatever brand" price option
// is a duplicate of an option from BrandingOptionSpaces.
//
// We expect that the cheapest option will often result in exactly the same
// brand combination as one of the BrandingOptionSpaces. We don't want to
// show the same brand combination twice for an itinerary.
template <typename FarePathBrandKeyBuilderT = FarePathBrandKeyBuilder<> >
class BrandingOptionSpacesDeduplicatorTemplate
{
public:
  using AllPaxBrandKey = std::map<const PaxType*,
                                  typename FarePathBrandKeyBuilderT::FarePathBrandKey>;
  using PaxBrandsMap = std::map<uint16_t, AllPaxBrandKey>;
  using KeyBrandsMap = std::map<typename FarePathBrandKeyBuilderT::FarePathBrandKey,
                                std::set<uint16_t>>;

  BrandingOptionSpacesDeduplicatorTemplate(const PricingTrx& trx,
                                           FarePathBrandKeyBuilderT* keyBuilder = nullptr) :
    _trx(trx)
  {
    assignValidObject(keyBuilder, _keyBuilder);
  }

  // Calculates which farePaths (corresponding to branding spaces) are priced
  // for the same brand combination.
  void calculateDeduplicationMap(const Itin& itin, KeyBrandsMap& deduplicationMap)
  {
    for (const FarePath* farePath : itin.farePath())
    {
      deduplicationMap[_keyBuilder->buildKey(_trx, itin, *farePath)].insert(farePath->brandIndex());
    }
  }

  std::set<uint16_t> getBrandOptionIndexesToDeduplicate(const Itin& itin, const KeyBrandsMap& deduplicationMap) const
  {
    std::set<uint16_t> pricedBrandCombIdxs;

    for (auto deduplicationItem : deduplicationMap)
    {
      if (deduplicationItem.second.size() > 1)
        pricedBrandCombIdxs.insert(++deduplicationItem.second.begin(), deduplicationItem.second.end());
    }

    return pricedBrandCombIdxs;
  }

  // Checks if the cheapest price option regardless of brands uses the same
  // brand combination for every pax type as any other option in this itinerary.
  //
  // If yes, returns the brand index of the other option; it will not be serialized
  // in the response.
  //
  // If no, returns INVALID_BRAND_INDEX.
  uint16_t findDuplicateOptionIndex(const Itin& itin)
  {
    PaxBrandsMap paxBrandsMap;
    for (const FarePath* farePath : itin.farePath())
    {
      paxBrandsMap[farePath->brandIndex()][farePath->paxType()] =
        _keyBuilder->buildKey(_trx, itin, *farePath);
    }

    // Cheapest option wasn't priced. Nothing to deduplicate.
    // (Effectively this happens for itineraries that weren't priced.)
    if (paxBrandsMap.count(CHEAPEST_OPTION_INDEX) == 0)
      return INVALID_BRAND_INDEX;

    const AllPaxBrandKey catchAllKey = paxBrandsMap[CHEAPEST_OPTION_INDEX];

    for (const typename PaxBrandsMap::value_type& brandKey : paxBrandsMap)
    {
      if (brandKey.first == CHEAPEST_OPTION_INDEX)
        continue;

      if (brandKey.second == catchAllKey)
        return brandKey.first;
    }

    return INVALID_BRAND_INDEX;
  }

private:
  const PricingTrx& _trx;
  boost::scoped_ptr<FarePathBrandKeyBuilderT> _keyBuilder;
};


typedef BrandingOptionSpacesDeduplicatorTemplate<> BrandingOptionSpacesDeduplicator;

} /* namespace skipper */

} /* namespace tse */

