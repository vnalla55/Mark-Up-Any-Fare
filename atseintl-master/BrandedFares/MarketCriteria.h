//-------------------------------------------------------------------
//
//  File:        MarketCriteria.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class MarketCriteria : boost::noncopyable
{
public:
  AlphaCode& direction() { return _direction; }
  const AlphaCode direction() const { return _direction; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  GlobalDirection globalDirection() const { return _globalDirection; }

  std::vector<PaxTypeCode>& paxType() { return _paxType; }
  const std::vector<PaxTypeCode>& paxType() const { return _paxType; }

  // need to pass in YYYY-MM-DD format
  DateTime& departureDate() { return _departureDate; }
  const DateTime& departureDate() const { return _departureDate; }

  LocCode& departureAirportCode() { return _departureAirportCode; }
  const LocCode& departureAirportCode() const { return _departureAirportCode; }

  LocCode& arrivalAirportCode() { return _arrivalAirportCode; }
  const LocCode& arrivalAirportCode() const { return _arrivalAirportCode; }

private:
  AlphaCode _direction;
  GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
  std::vector<PaxTypeCode> _paxType;
  DateTime _departureDate;
  LocCode _departureAirportCode;
  LocCode _arrivalAirportCode;
};
} // tse

