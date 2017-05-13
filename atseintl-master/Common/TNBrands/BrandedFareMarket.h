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

#include "Common/Assert.h"
#include "Common/TNBrands/ItinGeometryCalculator.h"
#include "DataModel/TNBrandsTypes.h"

namespace tse
{

namespace skipper
{

// A class exposing fare market-related data/functionality of a specific
// fare market in a convenient way, in context of a particular itin object
// (given by itinGeometryCalculator).
template <typename ItinGeometryCalculatorT = ItinGeometryCalculator>
class BrandedFareMarket
{
public:
  BrandedFareMarket(
      const ItinGeometryCalculatorT& itinGeometryCalculator,
      const FareMarket& fareMarket):
    _itinGeometryCalculator(itinGeometryCalculator), _fareMarket(fareMarket) {}

  // Returns fare market-specific brand codes.
  // These are brands specified in the request for a given itinerary or all
  // brands associated with the fare market if no such brands were specified.
  // It is possible that the result set is empty (if no brands are
  // available for fare market or all such brands were filtered out).
  // The same functionality as in
  // ItinGeometryCalculator::getItinSpecificBrandCodes()
  const UnorderedBrandCodes getBrands() const
  {
    return _itinGeometryCalculator.getItinSpecificBrandCodes(_fareMarket);
  }

  // Returns fare market first segment's index in itin (passed indirectly
  // via itinCalculator). The same functionality as in
  // ItinGeometryCalculator::getFareMarketStartSegmentIndex()
  size_t getStartSegmentIndex() const
  {
    return _itinGeometryCalculator.getFareMarketStartSegmentIndex(_fareMarket);
  }

  // Returns fare market last segment's index in itin (passed indirectly
  // via itinCalculator). The same functionality as in
  // ItinGeometryCalculator::getFareMarketEndSegmentIndex()
  size_t getEndSegmentIndex() const
  {
    return _itinGeometryCalculator.getFareMarketEndSegmentIndex(_fareMarket);
  }

  // Returns fare market corresponding to this object
  const FareMarket* getFareMarket() const
  {
    return &_fareMarket;
  }


private:
  const ItinGeometryCalculatorT& _itinGeometryCalculator;
  const FareMarket& _fareMarket;
};


} // namespace skipper

} // namespace tse


