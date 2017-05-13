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
#include "DataModel/RequestResponse/InputFlightUsage.h"
#include "DomainDataObjects/FlightUsage.h"
#include "DomainDataObjects/Flight.h"

namespace tax
{
FlightUsage::FlightUsage()
  : _id(0),
    _flightRefId(0),
    _connectionDateShift(0),
    _bookingCodeType(""),
    _openSegmentIndicator(type::OpenSegmentIndicator::Fixed),
    _flight(nullptr),
    _departureDate(type::Date::blank_date()),
    _arrivalDate(type::Date::blank_date()),
    _forcedConnection(type::ForcedConnection::Blank)
{
}

type::Date
FlightUsage::markDepartureDate(type::Date reference)
{
  _departureDate = reference;
  _arrivalDate = reference.advance(_flight->arrivalDateShift());
  return _arrivalDate;
}

type::Time
FlightUsage::departureTime() const
{
  return _flight->departureTime();
}

type::Time
FlightUsage::arrivalTime() const
{
  return _flight->arrivalTime();
}

std::ostream&
FlightUsage::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "FLIGHTREFID: " << _flightRefId << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "CONNECTIONDATESHIFT: " << _connectionDateShift << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "BOOKINGCODETYPE: " << _bookingCodeType << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "OPENSEGMENTINDICATOR: " << _openSegmentIndicator << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "FLIGHT\n";
  if (_flight != nullptr)
    _flight->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "DEPARTUREDATE: " << _departureDate << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ARRIVALDATE: " << _arrivalDate << "\n";

  return out;
}

} // namespace tax
