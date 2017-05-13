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

#include "DataModel/Common/Types.h"
#include "DataModel/Common/CompactOptional.h"
#include "DomainDataObjects/ChangeFee.h"
#include "DomainDataObjects/Flight.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Passenger.h"
#include "DomainDataObjects/PointOfSale.h"
#include "DomainDataObjects/TicketingFee.h"

#include <limits>
#include <vector>

namespace tax
{
class FarePath;
class Geo;
class GeoPath;
class YqYrPath;
class OptionalServicePath;
class GeoPathMapping;
class TurnaroundCalculator;

class Itin
{
public:
  typedef CompactOptional<type::Index> OptionalIndex;

  const type::Index& id() const { return _id; }

  type::Index& id() { return _id; }

  const type::Index& geoPathRefId() const { return _geoPathRefId; };

  type::Index& geoPathRefId() { return _geoPathRefId; };

  const type::Index& farePathRefId() const { return _farePathRefId; };

  type::Index& farePathRefId() { return _farePathRefId; };

  const OptionalIndex& farePathGeoPathMappingRefId() const { return _farePathGeoPathMappingRefId; };

  OptionalIndex& farePathGeoPathMappingRefId() { return _farePathGeoPathMappingRefId; };

  const OptionalIndex& yqYrPathRefId() const { return _yqYrPathRefId; };

  OptionalIndex& yqYrPathRefId() { return _yqYrPathRefId; };

  const OptionalIndex& yqYrPathGeoPathMappingRefId() const { return _yqYrPathGeoPathMappingRefId; };

  OptionalIndex& yqYrPathGeoPathMappingRefId() { return _yqYrPathGeoPathMappingRefId; };

  const OptionalIndex& optionalServicePathRefId() const { return _optionalServicePathRefId; };

  OptionalIndex& optionalServicePathRefId() { return _optionalServicePathRefId; };

  const OptionalIndex& optionalServicePathGeoPathMappingRefId() const
  {
    return _optionalServicePathGeoPathMappingRefId;
  };

  OptionalIndex& optionalServicePathGeoPathMappingRefId()
  {
    return _optionalServicePathGeoPathMappingRefId;
  };

  type::Date& travelOriginDate() { return _travelOriginDate; }

  const type::Date& travelOriginDate() const { return _travelOriginDate; }

  std::string& label() { return _label; }
  const std::string& label() const { return _label; }

  const GeoPath*& geoPath() { return _geoPath; }
  const GeoPath* geoPath() const { return _geoPath; }

  const FarePath*& farePath() { return _farePath; }
  const FarePath* farePath() const { return _farePath; }

  const YqYrPath*& yqYrPath() { return _yqYrPath; }
  const YqYrPath* yqYrPath() const { return _yqYrPath; }

  const OptionalServicePath*& optionalServicePath() { return _optionalServicePath; }
  const OptionalServicePath* optionalServicePath() const { return _optionalServicePath; }

  const Passenger*& passenger() { return _passenger; }
  const Passenger* passenger() const { return _passenger; }

  type::Index& passengerRefId() { return _passengerRefId; }
  const type::Index& passengerRefId() const { return _passengerRefId; }

  type::Index& pointOfSaleRefId() { return _pointOfSaleRefId; }
  const type::Index& pointOfSaleRefId() const { return _pointOfSaleRefId; }

  OptionalIndex& changeFeeRefId() { return _changeFeeRefId; }
  const OptionalIndex& changeFeeRefId() const { return _changeFeeRefId; }

  const PointOfSale*& pointOfSale() { return _pointOfSale; }
  const PointOfSale* pointOfSale() const { return _pointOfSale; }

  std::vector<FlightUsage>& flightUsages() { return _flightUsages; }
  const std::vector<FlightUsage>& flightUsages() const { return _flightUsages; }

  const GeoPathMapping*& geoPathMapping() { return _geoPathMapping; }
  const GeoPathMapping* geoPathMapping() const { return _geoPathMapping; }

  const GeoPathMapping*& yqYrPathGeoPathMapping() { return _yqYrPathGeoPathMapping; }
  const GeoPathMapping* yqYrPathGeoPathMapping() const { return _yqYrPathGeoPathMapping; }

  const GeoPathMapping*& optionalServicePathGeoPathMapping()
  {
    return _optionalServicePathGeoPathMapping;
  }
  const GeoPathMapping* optionalServicePathGeoPathMapping() const
  {
    return _optionalServicePathGeoPathMapping;
  }

  std::vector<TicketingFee>& ticketingFees() { return _ticketingFees; }
  const std::vector<TicketingFee>& ticketingFees() const { return _ticketingFees; }

  void computeTimeline();

  type::Date dateAtTaxPoint(type::Index taxPoint) const;
  type::Date firstDateAtTaxPoint(type::Index taxPoint) const;
  type::Date lastDateAtTaxPoint(type::Index taxPoint) const;

  const Geo* getTurnaround(const TurnaroundCalculator& turnaroundCalculator) const;

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _id{0};
  type::Index _geoPathRefId{std::numeric_limits<type::Index>::max()};
  type::Index _farePathRefId{std::numeric_limits<type::Index>::max()};
  OptionalIndex _yqYrPathRefId;
  OptionalIndex _optionalServicePathRefId;
  type::Index _passengerRefId{std::numeric_limits<type::Index>::max()};
  type::Index _pointOfSaleRefId{0}; // default POS
  OptionalIndex _changeFeeRefId;

  OptionalIndex _farePathGeoPathMappingRefId;
  OptionalIndex _yqYrPathGeoPathMappingRefId;
  OptionalIndex _optionalServicePathGeoPathMappingRefId;

  type::Date _travelOriginDate;
  std::string _label;

  const GeoPath* _geoPath{nullptr};
  const FarePath* _farePath{nullptr};
  const YqYrPath* _yqYrPath{nullptr};
  const OptionalServicePath* _optionalServicePath{nullptr};
  const Passenger* _passenger{nullptr};
  const PointOfSale* _pointOfSale{nullptr};

  const GeoPathMapping* _geoPathMapping{nullptr};
  const GeoPathMapping* _yqYrPathGeoPathMapping{nullptr};
  const GeoPathMapping* _optionalServicePathGeoPathMapping{nullptr};
  std::vector<FlightUsage> _flightUsages;
  std::vector<TicketingFee> _ticketingFees;

protected:
  mutable bool _turnaroundCalculated{false};
  mutable const Geo* _turnaroundPoint{nullptr};
};
} // namespace tax
