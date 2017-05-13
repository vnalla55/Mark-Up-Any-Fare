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

#include "Common/Assert.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TNBrands/TNBrandsUtils.h"
#include "DataModel/TNBrandsTypes.h"

namespace tse
{

namespace skipper
{

template <typename TNBrandsFunctionsT = TNBrandsFunctions>
class SpaceBlockCalculator
{
public:
  SpaceBlockCalculator(TNBrandsFunctionsT* functions = 0)
  {
    assignValidObject(functions, _functions);
  }

  // Calculates a set of pricing alternatives for a single segment
  // (carrier - brand pairs which can be used on this segment).
  // Such pricing alternatives are called space block since a vector
  // of such blocks (corresponding to segments) builds a whole pricing
  // option space (pricing alternatives for building a whole option).
  //
  // segmentInfo contains vectors of brands belonging to various carriers.
  // Returned block contains one brand for each carrier which reflects a
  // possible choice between various carriers' brands on the segment.
  //
  // totalNumberOfSpaces is the total number of pricing option spaces we
  // generate per itin. Since the carrier's number of brands is usually
  // smaller than that, we must reuse brands in different pricing spaces.
  // Since we want to do it "uniformly" the spaceIndex is passed. Using it
  // we can determine the source brand which will be used in the currently
  // built space, for example:
  // brand1   space1
  //          space2
  // brand2   space3
  //          space4
  // brand3   space5
  //
  // maxBrandsOnThisSegment is used to calculate how to reuse NO BRAND case.
  // The algorithm will calculate indexes as for maxBrandsOnThisSegment available
  // but will puth NO BRAND only where the first ("the cheapest") brand whould
  // be used (index 0 returned), for example:
  // number of spaces: 5
  // max number of brands for any carrier on this segment: 3
  // current carrier has only NO BRAND on this segment:
  //        how maxBrands would be distributed     who we use it for NO BRAND
  // space1             brand 1                            NO_BRAND
  // space2             brand 1                            NO_BRAND
  // space3             brand 2
  // space4             brand 2
  // space5             brand 3
  //
  // isFirstCabin informs if the space block will belong to the cheapest
  // requested cabin. If not, we handle carriers with NO_BRAND a bit different,
  // that is, we avoid using NO_BRAND if any other brand is available on
  // segment.
  CarrierBrandPairs calculateSpaceBlock(
      const BrandCodeArraysPerCarrier& segmentInfo,
      size_t totalNumberOfSpaces,
      size_t spaceIndex,
      size_t maxBrandsOnThisSegment,
      bool isFirstCabin = true) const;

private:
  boost::scoped_ptr<TNBrandsFunctionsT> _functions;
};

template <typename TNBrandsFunctionsT>
CarrierBrandPairs SpaceBlockCalculator<TNBrandsFunctionsT>::calculateSpaceBlock(
    const BrandCodeArraysPerCarrier& segmentInfo,
    size_t totalNumberOfSpaces,
    size_t spaceIndex,
    size_t maxBrandsOnThisSegment,
    bool isFirstCabin) const
{
  CarrierBrandPairs spaceBlock;

  bool isAnyBrandPresent = false;
  if (isFirstCabin == false)
  {
    // not the first cabin, we need additional info if any of carriers has
    // brands on this segment
    for (const auto& elem : segmentInfo)
    {
      if (elem.second.front() != NO_BRAND)
      {
        isAnyBrandPresent = true;
        break;
      }
    }
  }

  for (const auto& elem : segmentInfo)
  {
    // If carrier with NO_BRAND, calculate it as if it was the "cheapest"
    // (first) brand of the carrier with the most brands on this segment.
    // This way NO_BRAND will be applied for this carrier only on the first few
    // spaces.
    if (elem.second.size() == 1 && elem.second.front() == NO_BRAND)
    {
      // If this is the first cabin or in case of any other cabin no brands
      // are available on this segment calculate where to put the NO_BRAND
      // If this is not the first cabin and any of the carriers has brands here
      // ignore the carrier without brands.
      if (isFirstCabin || !isAnyBrandPresent)
      {
        const size_t brandIndex = _functions->calculateProportionalIndex(
            spaceIndex, maxBrandsOnThisSegment, totalNumberOfSpaces);
        if (brandIndex == 0)
          _functions->setUpdateNoBrand(spaceBlock, elem.first);
      }
    }
    else
    {
      const size_t brandIndex = _functions->calculateProportionalIndex(
          spaceIndex, elem.second.size(), totalNumberOfSpaces);
      spaceBlock[elem.first] = elem.second[brandIndex];
    }
  }
  return spaceBlock;
}


// Generates pricing option spaces for brandInfo (typically
// gathered for a single itin).

// A single space consists of pricing alternatives (sets of
// carrier-brand possibilities) organized as a vector in
// per-segment manner. An alternative of form (carrier-no brand)
// is permitted and represented as a carrier, NO_BRAND pair.
//
// Example output may look like this:
// segment 1  segment 2  segment 3
// (AA: X)    (AA: Z)    (BB: --)
// (BB: Y)               (CC: P)
// --------------------------------
// (AA: X)    (AA: Z)    (BB: --)
// (BB: Z)               (CC: Q)
//
// (AA: X)(BB: Y) represents pricing alternatives for the segment 1 in the
// first pricing option space.
// Consequently, the first part above the dashed line represents the first
// pricing option space.
template <typename TNBrandsFunctionsT = TNBrandsFunctions,
          typename SpaceBlockCalculatorT = SpaceBlockCalculator<> >
class BrandingOptionSpacesCalculator
{
public:
  BrandingOptionSpacesCalculator(TNBrandsFunctionsT* functions = 0,
                                SpaceBlockCalculatorT* blockCalculator = 0)
  {
    assignValidObject(functions, _functions);
    assignValidObject(blockCalculator, _blockCalculator);
  }

  // Generates pricing option spaces for brandInfo which contains vectors
  // of brands belonging to various carriers in itin in per-segment manner.
  //
  // We assume that brands in vectors are sorted incrementally,
  // according to the "class of service" (e.g. economy brands come first,
  // business brands come last).
  // The number of spaces equals to the maximum number of brands
  // for any carrier, in any segment but can be limited (limit contains
  // number of options spaces to generate including space reserved for the
  // cheapest option; this implicates limit, if set, should be at least 2).
  // If the number of brands for a carrier on a segment is less then the
  // total number of spaces, the brand are reused "uniformly" across all
  // pricing option spaces (see SpaceBlockCalculator class for details).
  // isFirstCabin informs if this block of spaces should contain reserved
  // space for the cheapest option. In case of spaces by cabin we want the
  // cheapest option only in the first requested cabin.
  void calculateSpaces(
    const SegmentOrientedBrandCodeArraysPerCarrier& brandInfo,
    BrandingOptionSpaces& output,
    size_t limit,
    bool isFirstCabin = true) const;

private:
  boost::scoped_ptr<TNBrandsFunctionsT> _functions;
  boost::scoped_ptr<SpaceBlockCalculatorT> _blockCalculator;
};

template <typename TNBrandsFunctionsT, typename SpaceBlockCalculatorT>
void BrandingOptionSpacesCalculator<TNBrandsFunctionsT,
                                   SpaceBlockCalculatorT>::calculateSpaces(
    const SegmentOrientedBrandCodeArraysPerCarrier& brandInfo,
    BrandingOptionSpaces& output,
    size_t limit,
    bool isFirstCabin) const
{
  output.clear();

  if (isFirstCabin) // add reserved space for the first cabin
  {
    TSE_ASSERT(limit != 1);
    BrandingOptionSpace reservedSpace;
    output.push_back(reservedSpace);
  }

  size_t totalNumberOfSpaces = _functions->calculateMaxBrandsCountPerCarrier(brandInfo);

  if (limit == 0 || limit > totalNumberOfSpaces)
    limit = totalNumberOfSpaces;

  for (size_t spaceIndex = 0; spaceIndex < limit; ++spaceIndex)
  {
    BrandingOptionSpace space;
    for (size_t segmentIndex = 0; segmentIndex < brandInfo.size(); ++segmentIndex)
    {
      space.push_back(_blockCalculator->calculateSpaceBlock(
        brandInfo[segmentIndex], totalNumberOfSpaces, spaceIndex,
        _functions->calculateMaxBrandsCountPerCarrier(brandInfo, segmentIndex),
        isFirstCabin));
    }
    output.push_back(space);
  }

  // in case we are preparing spaces for more expensive cabins avoid situation
  // when there is only NO_BRAND solution (on all segments for all carriers)
  if (!isFirstCabin && output.size() == 1)
  {
    bool isAnyBrandAvailable = false;
    for (size_t segmentIndex = 0;
         segmentIndex < brandInfo.size() && isAnyBrandAvailable == false;
         ++segmentIndex)
    {
      for (CarrierBrandPairs::const_iterator iter = output.front()[segmentIndex].begin();
           iter != output.front()[segmentIndex].end();
           ++iter)
      {
        if (iter->second != NO_BRAND)
        {
          isAnyBrandAvailable = true;
          break; // breaks only the inner loop, outer loop ended isAnyBrandAvailable
        }
      }
    }

    if (!isAnyBrandAvailable)
    {
      // no solution should be returned
      output.clear();
    }
  }
}

} // namespace skipper

} /* namespace tse */

