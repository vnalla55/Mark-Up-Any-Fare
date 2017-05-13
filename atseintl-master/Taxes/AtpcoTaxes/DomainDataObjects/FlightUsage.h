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
#include "Common/Timestamp.h"

namespace tax
{
class Flight;

class FlightUsage
{
public:
  FlightUsage();

  void setId(type::Index id) { _id = id; }
  type::Index getId() const { return _id; }
  type::Index& flightRefId() { return _flightRefId; }
  type::Index flightRefId() const { return _flightRefId; }
  int16_t& connectionDateShift() { return _connectionDateShift; }
  int16_t connectionDateShift() const { return _connectionDateShift; }
  type::OpenSegmentIndicator& openSegmentIndicator() { return _openSegmentIndicator; }
  type::OpenSegmentIndicator openSegmentIndicator() const { return _openSegmentIndicator; }
  type::ForcedConnection& forcedConnection() { return _forcedConnection; }
  type::ForcedConnection forcedConnection() const { return _forcedConnection; }
  const Flight*& flight() { return _flight; }
  const Flight* flight() const { return _flight; }
  type::Date departureDate() const { return _departureDate; }
  type::Date arrivalDate() const { return _arrivalDate; }

  type::Time departureTime() const;
  type::Time arrivalTime() const;

  type::Date markDepartureDate(type::Date reference);

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _id;
  type::Index _flightRefId;
  int16_t _connectionDateShift;
  std::string _bookingCodeType;
  type::OpenSegmentIndicator _openSegmentIndicator;
  const Flight* _flight;
  type::Date _departureDate;
  type::Date _arrivalDate;
  type::ForcedConnection _forcedConnection;
};
}
