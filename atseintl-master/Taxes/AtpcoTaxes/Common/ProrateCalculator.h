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

#pragma once

#include "Common/RangeUtils.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Map.h"

#include <vector>

namespace tax
{
class MileageGetter;
class GeoPath;

class ProrateCalculator
{
public:
  ProrateCalculator(const MileageGetter&);

  type::MoneyAmount
  getProratedAmount(const Range& baseRange,
                    const Range& taxRange,
                    type::MoneyAmount amount) const;

  type::MoneyAmount
  getProratedAmount(const Range& baseRange,
                    const Range& taxRange,
                    type::MoneyAmount amount,
                    const GeoPath& geoPath,
                    bool skipHiddenPoints = true,
                    const std::vector<Map>* maps = nullptr) const;

private:
  const MileageGetter& _mileageGetter;
};

}
