//-------------------------------------------------------------------
//
//  File:        BrandProgram.cpp
//  Created:     November 2015
//  Authors:
//
//  Description:
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

#include "BrandedFares/BrandProgram.h"

#include "DataModel/FareMarket.h"
#include "Diagnostic/Diag894Collector.h"

#include <set>

namespace tse
{

bool BrandProgram::calculateDirectionality(const FareMarket& fareMarket,
                                           Direction& direction,
                                           Diag894Collector* diag894) const
{
  if (fareMarket.brandProgramIndexVec().empty()) //no brands on this FM
  {
    direction = Direction::BOTHWAYS;
    return true;
  }

  bool isOriginal = false;
  bool isReversed = false;

  auto predOrigin = [&](const LocCode& loc) {
      return (originsRequested().find(loc) !=
              originsRequested().end());
  };
  auto predDest = [&](const LocCode& loc) {
      return (destinationsRequested().find(loc) !=
              destinationsRequested().end());
  };
  auto hasOriginRequested = [&](const std::set<LocCode>& locsRequested) {
    return std::any_of(locsRequested.begin(), locsRequested.end(), predOrigin);
  };
  auto hasDestinationRequested = [&](const std::set<LocCode>& locsRequested) {
    return std::any_of(locsRequested.begin(), locsRequested.end(), predDest);
  };

  // we want information if program is the same direction as fare market (ORIGINAL)
  // or not (REVERSED).
  isOriginal = hasOriginRequested(fareMarket.getOriginsRequested());
  isReversed = hasDestinationRequested(fareMarket.getDestinationsRequested());

  direction = Direction::BOTHWAYS;
  if (isOriginal && isReversed)
    direction = Direction::BOTHWAYS;
  else if (isOriginal)
    direction = Direction::ORIGINAL;
  else if (isReversed)
    direction = Direction::REVERSED;
  else
    return false;

  if (UNLIKELY(diag894 != nullptr))
  {
    if (diag894->isDDINFO())
      diag894->printProgramDirectionCalculationDetails(
        *this, fareMarket, isOriginal, isReversed, direction);
  }

  return true;
}

} // tse

