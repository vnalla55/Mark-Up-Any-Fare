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
#include "DomainDataObjects/Request.h"

namespace tax
{

std::ostream&
Request::print(std::ostream& out) const
{
  out << "POINTSOFSALE\n";
  for (const PointOfSale& pointOfSale : _pointsOfSale)
  {
    pointOfSale.print(out, 1);
  }

  out << "POSTAXPOINTS\n";
  for (const Geo& posTaxPoint : _posTaxPoints)
  {
    posTaxPoint.print(out, 1);
  }

  out << "GEOPATHS\n";
  for (const GeoPath& geoPath : _geoPaths)
  {
    geoPath.print(out, 1);
  }

  out << "FAREPATHS\n";
  for (const FarePath& farePath : _farePaths)
  {
    farePath.print(out, 1);
  }

  out << "YQYRPATHS\n";
  for (const YqYrPath& yqYrPath : _yqYrPaths)
  {
    yqYrPath.print(out, 1);
  }

  out << "GEOPATHMAPPINGS\n";
  for (const GeoPathMapping& geoPathMapping : _geoPathMappings)
  {
    geoPathMapping.print(out, 1);
  }

  out << "PASSENGERS\n";
  for (const Passenger& passenger: _passengers)
  {
    passenger.print(out, 1);
  }

  out << "ITINS\n";
  for (const Itin& itin : _allItins)
  {
    itin.print(out, 1);
  }

  out << "FLIGHTS\n";
  for (const Flight& flight : _flights)
  {
    flight.print(out, 1);
  }

  out << "FARES\n";
  for (const Fare& fare :_fares)
  {
    fare.print(out, 1);
  }

  out << "YQYRS\n";
  for (const YqYr& yqYr : _yqYrs)
  {
    yqYr.print(out, 1);
  }

  out << "CHANGEFEES\n";
  for (const ChangeFee& changeFee : _changeFees)
  {
    changeFee.print(out, 1);
  }

  out << "PROCESSINGOPTIONS\n";
  _procOpts.print(out, 1);
  out << "TICKETINGOPTIONS\n";
  _ticketingOptions.print(out, 1);

  out << "DIAGNOSTIC\n";
  _diagnostic.print(out, 1);

  out << "PREVTICKETGEOPATH\n";
  _prevTicketGeoPath.print(out, 1);

  out << "\n";
  return out;
}

std::pair<Request::ContainersValue, bool>
Request::setRulesContainers(const type::Nation& nation,
                            const type::TaxPointTag& taxPointTag,
                            ContainersValue containers)
{
  ContainersKey key(nation, taxPointTag);
  std::pair<ContainersMap::iterator, bool> insertResult =
      _containersMap.insert(ContainersMap::value_type(key, containers));

  return std::make_pair(insertResult.first->second, insertResult.second);
}

const GeoPath&
Request::getGeoPath(type::Index itinIndex) const
{
  const Itin& itin = allItins()[itinIndex];
  const type::Index& geoPathRefId = itin.geoPathRefId();
  return geoPaths()[geoPathRefId];
}

const std::vector<FlightUsage>
Request::getFlightUsages(type::Index itinIndex) const
{
  const Itin& itin = allItins()[itinIndex];
  return itin.flightUsages();
}

}
