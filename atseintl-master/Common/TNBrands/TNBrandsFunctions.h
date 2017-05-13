//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//               Andrzej Fediuk
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

#include "BrandedFares/BrandedFaresComparator.h"
#include "Common/Assert.h"
#include "Common/BrandingUtil.h"
#include "DataModel/TNBrandsTypes.h"
#include "Diagnostic/Diag892Collector.h"

namespace tse
{

namespace skipper
{

class TNBrandsFunctions
{
public:

  static const size_t ALL_SEGMENTS;

  // Returns segment-oriented information about brands belonging to carriers
  // for an itinerary represented by itinCalculator, taking into account
  // fare markets' starting segments.

  // Iterates over all fare markets in the itin. For each fare market, only
  // brands requested for the itin are considered (or all of them if no specific
  // brands are requested). Brands per carrier are stored in result on the
  // position corresponding to fare market's starting segment index in itin.
  // It is possible that brand set for some carrier is empty (e.g. there were
  // no brands in the market).
  template <typename ItinGeometryCalculatorT>
  static void calculateSegmentOrientedBrandCodesPerCarrier(
    const ItinGeometryCalculatorT& itinCalculator,
    const SegmentOrientedBrandCodesPerCarrier* brandsValidForCabin,
    SegmentOrientedBrandCodesPerCarrier& result);

  // Sorts data that comes from calculateSegmentOrientedBrandCodesPerCarrier.

  // Returns a vector of mappings from carrier code to a vector of brands.
  // Brands are sorted within each segment and carrier using
  // the given comparator.
  template <typename BrandedFaresComparatorT>
  static void sortSegmentOrientedBrandCodesPerCarrier(
    const SegmentOrientedBrandCodesPerCarrier& input,
    BrandedFaresComparatorT& comparator,
    SegmentOrientedBrandCodeArraysPerCarrier& result);

  // Accepts a vector of mappings from carrier code to a vector of brands.
  // Returns the maximal length of brand vector across the whole input.
  // By default calculates maximum on all segments but value for specific
  // segment can be calculated using argument onSegment.
  static size_t calculateMaxBrandsCountPerCarrier(
      const SegmentOrientedBrandCodeArraysPerCarrier& input,
      size_t onSegment = ALL_SEGMENTS);

  // Assume that you want to place all unique elements from
  // a shorter array into a longer array in a "uniform" manner,
  // i.e. all elements occupy more or less the same number of positions,
  // e.g.
  // [A B C D E] -> [A A B C C D E]
  // for inputSetSize = 5 and outputSetSize = 7.
  // This function calculates the index in the input array that
  // corresponds to a given outputIndex in the output array.
  static size_t calculateProportionalIndex(
      size_t outputIndex, size_t inputSetSize, size_t outputSetSize);

  // Assume that you want to place all unique elements from
  // a shorter array into a longer array, but you prefer bottom elements
  // more than top ones, and additionally all elements should occupy more
  // or less the same number of positions (+/-1).
  // e.g.
  // [A B C D E] -> [A B C D D E E]
  // for inputSetSize = 5 and outputSetSize = 7.
  // This function calculates the index in the input array that
  // corresponds to a given outputIndex in the output array.
  static size_t calculateBottomPreferredIndex(
      size_t outputIndex, size_t inputSetSize, size_t outputSetSize);

  // Assume that you want to place all unique elements from
  // a shorter array into a longer array, but you prefer top elements
  // more than bottom ones, and additionally all elements should occupy more
  // or less the same number of positions (+/-1).
  // e.g.
  // [A B C D E] -> [A A B B C D E]
  // for inputSetSize = 5 and outputSetSize = 7.
  // This function calculates the index in the input array that
  // corresponds to a given outputIndex in the output array.
  static size_t calculateTopPreferredIndex(
      size_t outputIndex, size_t inputSetSize, size_t outputSetSize);

  // Accepts a vector of mappings from carrier code to a vector of brands.
  // Assures that each vector of brands is not empty - in case it is
  // a special indicator NO_BRAND will be placed in the array.
  static void fillEmptyBrandsArraysPerCarrier(
      SegmentOrientedBrandCodeArraysPerCarrier& brandInfo);

  // Fills the "reserved" non-branded BrandingOptionSpace at index 0
  // with NO_BRAND indicators for all governing carriers in each segment.
  // Expects that the branded BrandingOptionSpaces have already been
  // calculated.
  static void updateReservedBrandingOptionSpace(
    BrandingOptionSpaces& brandingSpaces);

  // For all itins runs calculateBrandingOptionSpaces() with brand filtering
  // by cabin if cabin higher than economy is requested
  static void buildBrandingOptionSpacesForAllItins(PricingTrx& trx,
                                                   Logger& logger,
                                                   Diag892Collector* diag892);
  // builds SegmentOrientedBrandCodesPerCarrier structure holding brands
  // that have fares in a Cabin that was requested
  static void selectBrandsValidForReqCabin(const PricingTrx& trx,
                                           const Itin* itin,
                                           SegmentOrientedBrandCodesPerCarrier& filteredBrands,
                                           Diag892Collector* diag892,
                                           bool stayInCabin);

  // build SegmentOrientedBrandCodesPerCarrierInCabin to get information
  // about brands in specific cabins. Only brands matching minimalCabinIndex or
  // higher are considered valid.
  static void selectBrandsPerCabin(const PricingTrx& trx,
                                   const Itin* itin,
                                   size_t minimalCabinIndex,
                                   SegmentOrientedBrandCodesPerCarrierInCabin& brandsPerCabin,
                                   Diag892Collector* diag892,
                                   bool stayInCabin);

  // Fill brandInfoArraysForThisCabin with segment/carrier/brand information
  // valid for requested cabin. Cabin is requested by limiting available brands
  // to brands valid only in this cabin by use of brandsPerCarrierInCabin.
  // validBrandsInOrderPerSegment is used as a general filter - it contains only
  // validated brands with additional ordering applied.
  // brandDeduplicationPerSegment is a helper variable which at the beginning
  // (before first call) should contain all brands for all carriers (a copy of
  // validBrandsInOrderPerSegment). At each call (each cabin filtering) used
  // brands are removed from it to assure each brand is used only once.
  static bool filterSortedSegmentOrientedBrandCodesPerCarrierByCabin(
    const SegmentOrientedBrandCodesPerCarrier& brandsPerCarrierInCabin,
    const SegmentOrientedBrandCodeArraysPerCarrier& validBrandsInOrderPerSegment,
    SegmentOrientedBrandCodeArraysPerCarrier& brandDeduplicationPerSegment,
    SegmentOrientedBrandCodeArraysPerCarrier& brandInfoArraysForThisCabin,
    bool useDirectionality);

  // Helper function for filterSortedSegmentOrientedBrandCodesPerCarrierByCabin
  // implementing its logic at specific segment.
  // brandsPerCarrierInCabinAtSegment contains all brands available for all
  // carriers at specific segment.
  static bool filterCarrierBrandCodesByCabinAtSegment(
    const BrandCodesPerCarrier& brandsPerCarrierInCabinAtSegment,
    const CarrierDirection& validCarrierWithDirection,
    BrandCodeArraysPerCarrier& brandDeduplicationAtSegment,
    BrandCodeArraysPerCarrier& brandInfoArraysForThisCabinAtSegment,
    bool useDirectionality);

  // Helper function for splitCarrierBrandCodesByCabinAtSegment implementing
  // it logic for single carrier
  static bool filterAndDeduplicateCarrierBrandCodes(
    const UnorderedBrandCodes& availableBrandsForCarrier,
    OrderedBrandCodes& validDeduplicatedBrandsForCarrier,
    OrderedBrandCodes& brandInfoForCarrier);

  // returns true if any segment from the input set has leg id that is on the list
  // of fixed legs passed as second attribute
  static bool isAnySegmentOnFixedLeg(const std::vector<TravelSeg*>& segments,
                                     const std::vector<bool>& fixedLegs);

  // returns true if any segment from the input set has leg id that is not on the
  // list of fixed legs passed as second attribute
  static bool isAnySegmentOnNonFixedLeg(const std::vector<TravelSeg*>& segments,
                                        const std::vector<bool>& fixedLegs);


  // returns true if any segment has leg id of the first not fixed leg
  static bool isAnySegmentOnCurrentlyShoppedLeg(const std::vector<TravelSeg*>& segments,
                                                const std::vector<bool>& fixedLegs);

  // add NO_BRAND to space block for given direction. if NO_BRAND is present
  // in this block in opposite direction, remove it and add one in BOTHWAYS
  static void setUpdateNoBrand(CarrierBrandPairs& spaceBlock,
                               const CarrierDirection& carrierWithDirection);
};

template <typename ItinGeometryCalculatorT>
void TNBrandsFunctions::calculateSegmentOrientedBrandCodesPerCarrier(
    const ItinGeometryCalculatorT& itinCalculator,
    const SegmentOrientedBrandCodesPerCarrier* brandsValidForCabin,
    SegmentOrientedBrandCodesPerCarrier& result)
{
  // Make sure that the result size equals
  // the number of segments in the itin.
  result.clear();
  result.resize(itinCalculator.getSegmentCount());

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(
    itinCalculator.getTrxGeometryCalculator().getTrx());

  for (typename ItinGeometryCalculatorT::FareMarketT* fm : itinCalculator.getFareMarkets())
  {
    TSE_ASSERT(fm != nullptr);
    const size_t start = itinCalculator.getFareMarketStartSegmentIndex(*fm);
    BrandCodesPerCarrier& crxBrandCodes = result[start];
    if (!useDirectionality)
    {
      const UnorderedBrandCodes brandCodes = itinCalculator.getItinSpecificBrandCodes(*fm);
      const CarrierCode carrier = fm->governingCarrier();
      CarrierDirection cxDirection = CarrierDirection(carrier, Direction::BOTHWAYS);
      if (brandsValidForCabin != nullptr)
      {
        // Filtering Brands by Cabin requested. Checking if brand is present in filter
        TSE_ASSERT(start < brandsValidForCabin->size());
        const BrandCodesPerCarrier& validCarrierBrands = brandsValidForCabin->at(start);
        BrandCodesPerCarrier::const_iterator brandsForCarrier = validCarrierBrands.find(cxDirection);
        if (brandsForCarrier == validCarrierBrands.end())
        {
          crxBrandCodes[cxDirection];
          continue;
        }

        const UnorderedBrandCodes& validBrands = brandsForCarrier->second;

        std::set_intersection(brandCodes.begin(), brandCodes.end(),
                              validBrands.begin(), validBrands.end(),
                              std::inserter(crxBrandCodes[cxDirection],
                                            crxBrandCodes[cxDirection].begin()));
      }
      else
      {
        crxBrandCodes[cxDirection].insert(brandCodes.begin(), brandCodes.end());
      }
    }
    else
    {
      const QualifiedBrandIndices brandCodeIndices =
          itinCalculator.getItinSpecificBrandCodesIndices(*fm);
      const std::vector<QualifiedBrand>& allBrands =
          itinCalculator.getTrxGeometryCalculator().getQualifiedBrands();
      const CarrierCode carrier = fm->governingCarrier();
      // place holders = NO_BRAND, will be replaced by brands or merged into BOTHWAYS
      crxBrandCodes[CarrierDirection(carrier, Direction::ORIGINAL)];
      crxBrandCodes[CarrierDirection(carrier, Direction::REVERSED)];

      for (int index: brandCodeIndices)
      {
        Direction direction;
        if (!itinCalculator.getProgramDirection(allBrands[index].first, *fm, direction))
          continue; //skip brand if direction cannot be calculated
        CarrierDirection cxDirection = CarrierDirection(carrier, direction);

        if (brandsValidForCabin != nullptr)
        {
          // Filtering Brands by Cabin requested. Checking if brand is present in filter
          TSE_ASSERT(start < brandsValidForCabin->size());
          const BrandCodesPerCarrier& validCarrierBrands = brandsValidForCabin->at(start);
          BrandCodesPerCarrier::const_iterator brandsForCarrier = validCarrierBrands.find(cxDirection);
          if (brandsForCarrier == validCarrierBrands.end())
          {
            continue;
          }
          const UnorderedBrandCodes& validBrands = brandsForCarrier->second;
          if (validBrands.find(allBrands[index].second->brandCode()) != validBrands.end())
          {
            crxBrandCodes[cxDirection].insert(allBrands[index].second->brandCode());
          }
        }
        else
        {
          crxBrandCodes[cxDirection].insert(allBrands[index].second->brandCode());
        }
      }
    }
  }
  if (useDirectionality)
  {
    for (auto& segment: result)
    {
      std::set<CarrierCode> carriersWithOriginalEmpty;
      std::set<CarrierCode> carriersWithReversedEmpty;

      for (auto& carrierBrands: segment)
      {
        if (carrierBrands.second.empty())
        {
          if (carrierBrands.first.direction == Direction::ORIGINAL)
            carriersWithOriginalEmpty.insert(carrierBrands.first.carrier);
          else if (carrierBrands.first.direction == Direction::REVERSED)
            carriersWithReversedEmpty.insert(carrierBrands.first.carrier);
        }
      }
      std::set<CarrierCode> carriersWithDirectionsEmpty;
      std::set_intersection(carriersWithOriginalEmpty.begin(), carriersWithOriginalEmpty.end(),
                            carriersWithReversedEmpty.begin(), carriersWithReversedEmpty.end(),
                            std::inserter(carriersWithDirectionsEmpty,
                                          carriersWithDirectionsEmpty.begin()));
      // add BOTHWAYS:NOBRAND which in the next step will reduce ORIGIN/DESTINATION
      for (const CarrierCode& carrier: carriersWithDirectionsEmpty)
        segment[CarrierDirection(carrier, Direction::BOTHWAYS)];

      std::set<CarrierCode> carriersWithBothways;
      for (auto& carrierBrands: segment)
      {
        if (carrierBrands.first.direction == Direction::BOTHWAYS)
          carriersWithBothways.insert(carrierBrands.first.carrier);
      }
      // BOTHWAYS override origin/destination
      for (const CarrierCode& carrier: carriersWithBothways)
      {
        segment.erase(CarrierDirection(carrier, Direction::ORIGINAL));
        segment.erase(CarrierDirection(carrier, Direction::REVERSED));
      }
    }
  }
}

template <typename BrandedFaresComparatorT>
void TNBrandsFunctions::sortSegmentOrientedBrandCodesPerCarrier(
    const SegmentOrientedBrandCodesPerCarrier& input,
    BrandedFaresComparatorT& comparator,
    SegmentOrientedBrandCodeArraysPerCarrier& result)
{
  for (const BrandCodesPerCarrier& crxBrandCodes : input)
  {
    BrandCodeArraysPerCarrier arraysPerCarrier;
    for (const auto& crxBrandCode : crxBrandCodes)
    {
      OrderedBrandCodes& arrayOfBrands = arraysPerCarrier[crxBrandCode.first];
      arrayOfBrands.insert(
          arrayOfBrands.end(), crxBrandCode.second.begin(), crxBrandCode.second.end());
      std::sort(arrayOfBrands.begin(), arrayOfBrands.end(), comparator);
    }
    result.push_back(arraysPerCarrier);
  }
}

} // namespace skipper

} /* namespace tse */

