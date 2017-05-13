// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/TaxName.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRuleApplicator.h"

#include <vector>

namespace tax
{
typedef std::pair<type::Index, type::Index> Trip;

class PaymentDetail;

class SpecialTrips
{
public:
  SpecialTrips();
  ~SpecialTrips();

  static void findRoundTrips(PaymentDetail& paymentDetail,
                             GeoPath const& geoPath,
                             const std::vector<FlightUsage>& flightUsages,
                             std::vector<Trip>& roundTrips);

  static void findContinuousJourneys(PaymentDetail& paymentDetail,
                                     GeoPath const& geoPath,
                                     std::vector<Trip>& journeys);

  static void findSingleJourneys(PaymentDetail& paymentDetail,
                                 GeoPath const& geoPath,
                                 std::vector<Trip>& journeys);

  static bool
  isGeoInTrip(const std::vector<Trip>& trips, const Geo& tripGeo, const Geo& currentGeo);

  static void removeOverlappedTrips(std::vector<Trip>& oneWayTrips,
                                    std::vector<Trip>& roundTrips);
};

namespace
{
class IsOverlapped
{
public:
  bool operator() (const Trip& trip1, const Trip& trip2)
  {
    bool isNotOverlapped = ((trip1.second <= trip2.first) || (trip1.first >= trip2.second));
    return !isNotOverlapped;
  }
};

template<typename In, typename Out, typename Comparator>
Out overlap_difference(In first1, In last1, In first2, In last2, Out result, Comparator cmp)
{
  while(first1 != last1)
  {
    bool isOverlapped = false;
    for(In tmp = first2; tmp != last2; ++tmp)
      if (cmp(*tmp, *first1))
        isOverlapped = true;

    if (!isOverlapped)
      *result++ = *first1;

    ++first1;
  }
  return result;
}
}

} // namespace tax

