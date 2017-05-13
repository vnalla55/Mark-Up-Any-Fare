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

namespace tax
{
class InputFlight;

struct InputFlightUsage
{
  InputFlightUsage()
    : _flightRefId(0),
      _connectionDateShift(0),
      _openSegmentIndicator(type::OpenSegmentIndicator::Fixed),
      _forcedConnection(type::ForcedConnection::Blank)
  {
  }

  type::Index _flightRefId;
  int16_t _connectionDateShift;
  std::string _bookingCodeType;
  type::OpenSegmentIndicator _openSegmentIndicator;
  type::ForcedConnection _forcedConnection;
};
}

