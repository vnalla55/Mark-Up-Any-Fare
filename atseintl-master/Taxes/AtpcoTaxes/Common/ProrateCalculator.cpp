// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/ProrateCalculator.h"
#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/MileageGetter.h"

namespace tax
{


ProrateCalculator::ProrateCalculator(const MileageGetter& mileageGetter)
  : _mileageGetter(mileageGetter)
{
}

type::MoneyAmount
ProrateCalculator::getProratedAmount(const Range& baseRange,
                                     const Range& taxRange,
                                     type::MoneyAmount amount,
                                     const GeoPath& geoPath,
                                     bool skipHiddenPoints /* = true */,
                                     const std::vector<Map>* maps /* = nullptr */) const
{
  Range commonRange = baseRange & taxRange;

  if (maps != nullptr)
  {
    // Fare proration, fares can be non-continuous, for correct result need to use maps vector
    type::Index start = 0;
    type::Index end = start + 1;

    type::Miles baseMiles = 0;
    type::Miles commonMiles = 0;
    while (end < maps->size())
    {
      if (skipHiddenPoints)
      {
        while (geoPath.geos()[maps->at(end).index()].isUnticketed())
          end += 2;
      }
      Range farePartRange{maps->at(start).index(), maps->at(end).index()};
      type::Miles dist = _mileageGetter.getSingleDistance(farePartRange.begin, farePartRange.end);

      baseMiles += dist;
      if (!(commonRange & farePartRange).empty)
      {
        commonMiles += dist;
      }

      start = end + 1;
      end = start + 1;
    }

    if (baseMiles == 0)
      return 0;

    return amount * commonMiles / baseMiles;
  }

  // maps == nullptr, proration of Yq/Yrs
  if (commonRange == baseRange)
    return amount;

  type::Miles baseMiles = 0;
  type::Index start = baseRange.begin;
  type::Index end = start + 1;
  while (end <= baseRange.end)
  {
    if (skipHiddenPoints)
    {
      while (geoPath.geos()[end].isUnticketed())
        end += 2;
    }
    baseMiles += _mileageGetter.getSingleDistance(start, end);
    start = end + 1;
    end = start + 1;
  }

  if (baseMiles == 0)
    return 0;

  type::Miles commonMiles = 0;
  start = commonRange.begin;
  end = start + 1;
  while (end <= commonRange.end)
  {
    if (skipHiddenPoints)
    {
      while (geoPath.geos()[end].isUnticketed())
        end += 2;
    }
    commonMiles += _mileageGetter.getSingleDistance(start, end);
    start = end + 1;
    end = start + 1;
  }

  return amount * commonMiles / baseMiles;
}

}
