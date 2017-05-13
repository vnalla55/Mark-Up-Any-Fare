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

#include "DataModel/Common/TaxPointProperties.h"
#include "ServiceInterfaces/MileageService.h"

#include <functional>
#include <vector>

namespace tax
{
class Geo;
class GeoPath;
class Itin;
class MileageService;

class TurnaroundCalculator
{
  typedef std::vector<MileageService::GeoIdMile> GeoIdMiles;
  typedef std::vector<bool> Stopovers;
  typedef std::function<bool(type::Index geoId, const TaxPointProperties& tpp)> IsAllowedFunc;
public:
  TurnaroundCalculator(const GeoPath& geoPath,
                       const MileageService& mileageService,
                       const TaxPointsProperties& taxPointsProperties,
                       bool specUS_RTOJLogic,
                       bool alternateTurnaroundDeterminationLogic);

  const Geo* getTurnaroundPoint(const Itin& itin) const;
  bool turnaroundLogicOverride() const
  {
    return _specUS_RTOJLogic || _alternateTurnaroundDeterminationLogic;
  }
private:
  const Geo* findTurnaround(const GeoIdMiles& geoIdMiles,
                            IsAllowedFunc isAllowed) const;
  bool journeyHasConnectionAtFareBreakAndStopover(Stopovers international24hStopovers,
                                                  Stopovers domesticUS4hStopovers) const;

  const GeoPath& _geoPath;
  const MileageService& _mileageService;
  const TaxPointsProperties& _taxPointsProperties;
  bool _specUS_RTOJLogic;
  bool _alternateTurnaroundDeterminationLogic;
};
}

