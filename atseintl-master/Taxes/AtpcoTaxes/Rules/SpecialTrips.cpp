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

#include "DomainDataObjects/Flight.h"
#include "Rules/SpecialTrips.h"
#include "Rules/PaymentDetail.h"
#include "GeoUtils.h"
#include "TaxPointUtils.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace tax
{

SpecialTrips::SpecialTrips() {}
SpecialTrips::~SpecialTrips() {}

bool isTicketedPointInUS(type::Index index, const std::vector<Geo>& geos)
{
  return (geos[index].unticketedTransfer() == type::UnticketedTransfer::No &&
          GeoUtils::isUSTerritory(geos[index].getNation()));
}

void
SpecialTrips::findRoundTrips(PaymentDetail& paymentDetail,
                             GeoPath const& geoPath,
                             const std::vector<FlightUsage>& flightUsages,
                             std::vector<Trip>& roundTrips)
{
  // US Round Trip (Tax Code = AY only) – defined as a portion of travel that
  // - begins at the departure from the first air sector on the ticket and
  // - ends when that same city (may be different airport) or a coterminal point (as defined in the
  //   ATPCO PFC Product co-terminal record) is visited as a stopover or journey destination and
  // - includes at least one ticketed point in the US or US territory and
  // - includes at least one stopover point

  const std::vector<Geo>& geos = geoPath.geos();
  const size_t geosSize = geos.size();

  type::Index indexBegin = 0;
  do
  {
    const Flight* flight = geos[indexBegin].getFlight(flightUsages);

    if (flight == nullptr)
      return;

    if (!flight->isGroundTransport())
      break;

    indexBegin += 2;
  } while (indexBegin < geosSize);

  type::Index indexEnd = 0;
  const type::CityCode& cityCodeBegin = geos[indexBegin].cityCode();

  bool ticketedPointInUS = isTicketedPointInUS(indexBegin, geos);
  bool stopoverPresent = paymentDetail.isUSLimitStopover(indexBegin);
  for (type::Index i = indexBegin + 1; i < geosSize; ++i)
  {
    if (geos[i].isArrival() &&
        cityCodeBegin == geos[i].cityCode() &&
        paymentDetail.isUSLimitStopover(i))
    {
      indexEnd = i;
      break;
    }

    if (isTicketedPointInUS(i, geos))
      ticketedPointInUS = true;

    if (paymentDetail.isUSLimitStopover(i))
      stopoverPresent = true;
  }

  if ((indexEnd > indexBegin) && ticketedPointInUS && stopoverPresent)
    roundTrips.push_back(std::make_pair(indexBegin, indexEnd));
}

void
SpecialTrips::findContinuousJourneys(PaymentDetail& paymentDetail,
                                     GeoPath const& geoPath,
                                     std::vector<Trip>& journeys)
{
  TaxPointUtil tUtil(paymentDetail);
  std::vector<Geo> const& geos = geoPath.geos();
  for (unsigned i = 0; i < geos.size(); i += 2)
  {
    if (geos[i].getNation() == "CA" && tUtil.isStop(geos[i]))
    {
      std::map<type::CityCode, unsigned> cityCount;
      ++cityCount[geos[i].cityCode()];
      bool tripFound = false;
      for (unsigned j = i + 1; j < geos.size(); j += 2)
      {
        if (geos[j].getNation() == "CA" && tUtil.isStop(geos[j]) &&
            ++cityCount[geos[j].cityCode()] == 2)
        {
          journeys.push_back(Trip(i, j));
          i = j - 1;
          tripFound = true;
          break;
        }
      }
      if (!tripFound)
      {
        journeys.push_back(Trip(i, geos.size() - 1));
        break;
      }
    }
  }
}

void
SpecialTrips::findSingleJourneys(PaymentDetail& paymentDetail,
                                 GeoPath const& _geoPath,
                                 std::vector<Trip>& trips)
{

  bool found = false;
  type::Index indexBegin = 0;
  type::Index indexEnd = 1;

  while (indexBegin < _geoPath.geos().size() - 1)
  {

    found = false;
    const type::CityCode& cityCodeBegin = _geoPath.geos()[indexBegin].cityCode();

    while (indexEnd <= _geoPath.geos().size() - 1)
    {
      const type::CityCode& cityCodeEnd = _geoPath.geos()[indexEnd].cityCode();

      if ((cityCodeBegin == cityCodeEnd || indexEnd == _geoPath.geos().size() - 1) &&
          paymentDetail.isStopover(indexEnd))
      {
        trips.push_back(std::make_pair(indexBegin, indexEnd));
        indexBegin = indexEnd + 1;
        indexEnd = indexBegin + 1;
        found = true;
        break;
      }
      indexEnd = indexEnd + 2;
    }
    if (!found)
      indexBegin = indexBegin + 2;
  }
}

bool
SpecialTrips::isGeoInTrip(const std::vector<Trip>& trips, const Geo& tripGeo, const Geo& currentGeo)
{
  type::Index currentIndex = currentGeo.id();
  type::Index tripIndex = tripGeo.id();

  for(const Trip & trip : trips)
  {
    if (currentIndex >= trip.first && currentIndex <= trip.second && tripIndex >= trip.first &&
        tripIndex <= trip.second)
    {
      return true;
    }
  }
  return false;
}

void
SpecialTrips::removeOverlappedTrips(std::vector<Trip>& oneWayTrips,
                      std::vector<Trip>& roundTrips)
{
  std::vector<Trip> result;
  std::back_insert_iterator< std::vector<Trip> > resultIter (result);

  overlap_difference(oneWayTrips.begin(), oneWayTrips.end(),
                     roundTrips.begin(), roundTrips.end(),
                     resultIter, IsOverlapped());
  oneWayTrips.swap(result);
}
} // namespace tax
