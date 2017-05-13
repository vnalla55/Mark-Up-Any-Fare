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

#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CompactOptionalIO.h"
#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/FarePath.h"
#include "DomainDataObjects/YqYrPath.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Factories/FlightUsageFactory.h"
#include "Rules/TurnaroundCalculator.h"

namespace tax
{

type::Date
Itin::dateAtTaxPoint(type::Index taxPoint) const
{
  type::Index flightRefId = taxPoint / 2;
  bool isArrival = (taxPoint % 2) == 1;
  const FlightUsage* fu = &_flightUsages[flightRefId];
  if (fu->openSegmentIndicator() != type::OpenSegmentIndicator::Open)
    return isArrival ? fu->arrivalDate() : fu->departureDate();
  return type::Date::blank_date();
}

type::Date
Itin::firstDateAtTaxPoint(type::Index taxPoint) const
{
  type::Index flightRefId = taxPoint / 2;
  bool isArrival = (taxPoint % 2) == 1;
  const FlightUsage& fu = _flightUsages[flightRefId];
  if (fu.openSegmentIndicator() != type::OpenSegmentIndicator::Open)
    return isArrival ? fu.arrivalDate() : fu.departureDate();
  bool found = false;
  while (flightRefId > 0 && !found)
  {
    flightRefId--;
    if (_flightUsages[flightRefId].openSegmentIndicator() != type::OpenSegmentIndicator::Open)
    {
      found = true;
      break;
    }
  }

  if (found)
    return _flightUsages[flightRefId].arrivalDate();
  else
    return type::Date::blank_date();
}

type::Date
Itin::lastDateAtTaxPoint(type::Index taxPoint) const
{
  type::Index flightRefId = taxPoint / 2;
  bool isArrival = (taxPoint % 2) == 1;
  const FlightUsage* fu = &_flightUsages[flightRefId];
  if (fu->openSegmentIndicator() != type::OpenSegmentIndicator::Open)
  {
    return isArrival ? fu->arrivalDate() : fu->departureDate();
  }
  while (flightRefId < _flightUsages.size() &&
         _flightUsages[flightRefId].openSegmentIndicator() == type::OpenSegmentIndicator::Open)
    flightRefId++;
  if (flightRefId < _flightUsages.size())
    return _flightUsages[flightRefId].departureDate();
  else
    return type::Date::blank_date();
}

void
Itin::computeTimeline()
{
  type::Date curr = _travelOriginDate;
  for (FlightUsage& fu : _flightUsages)
  {
    curr = curr.advance(fu.connectionDateShift());
    curr = fu.markDepartureDate(curr);
  }
}

const Geo*
Itin::getTurnaround(const TurnaroundCalculator& turnaroundCalculator) const
{
  if (!_turnaroundCalculated)
  {
    _turnaroundPoint = turnaroundCalculator.getTurnaroundPoint(*this);

    if (!turnaroundCalculator.turnaroundLogicOverride())
      _turnaroundCalculated = true;

    return _turnaroundPoint;
  }
  else
  {
    if (!turnaroundCalculator.turnaroundLogicOverride())
      return _turnaroundPoint;

    return turnaroundCalculator.getTurnaroundPoint(*this);
  }
}

std::ostream&
Itin::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  const std::string indent(indentLevel, indentChar);

  out << indent << "ID: " << _id << "\n" << indent << "GEOPATHREFID: " << _geoPathRefId << "\n"
      << indent << "FAREPATHREFID: " << _farePathRefId << "\n" << indent
      << "YQYRPATHREFID: " << _yqYrPathRefId << "\n" << indent
      << "OPTIONALSERVICEPATHREFID: " << _optionalServicePathRefId << "\n" << indent
      << "PASSENGERREFID: " << _passengerRefId << "\n" << indent
      << "POINTOFSALEREFID: " << _pointOfSaleRefId << "\n" << indent
      << "CHANGEFEEREFID: " << _changeFeeRefId << "\n" << indent
      << "FAREPATHGEOPATHMAPPINGREFID: " << _farePathGeoPathMappingRefId << "\n" << indent
      << "YQYRPATHGEOPATHMAPPINGREFID: " << _yqYrPathGeoPathMappingRefId << "\n" << indent
      << "OPTIONALSERVICEPATHGEOPATHMAPPINGREFID: " << _optionalServicePathGeoPathMappingRefId
      << "\n" << indent << "TRAVELORIGINDATE: " << _travelOriginDate << "\n";

  out << indent << "GEOPATH\n";
  if (_geoPath != nullptr)
    _geoPath->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  out << indent << "FAREPATH\n";
  if (_farePath != nullptr)
    _farePath->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  out << indent << "YQYRPATH\n";
  if (_yqYrPath != nullptr)
    _yqYrPath->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  out << indent << "GEOPATHMAPPING\n";
  if (_geoPathMapping != nullptr)
    _geoPathMapping->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  out << indent << "FLIGHTUSAGES\n";
  for (FlightUsage const& flightUsage : _flightUsages)
  {
    flightUsage.print(out, indentLevel + 1);
  }

  out << indent << "LABEL: " << _label << "\n";

  return out;
}

} // namespace tax
