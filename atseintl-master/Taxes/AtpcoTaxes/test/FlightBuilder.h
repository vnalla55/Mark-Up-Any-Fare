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

#include "DomainDataObjects/Flight.h"

namespace tax
{
class FlightBuilder
{
  Flight* _flight;

public:
  FlightBuilder() : _flight(new Flight) {}

  FlightBuilder& setDepartureTime(type::Time time)
  {
    _flight->departureTime() = time;
    return *this;
  }

  FlightBuilder& setArrivalTime(type::Time time)
  {
    _flight->arrivalTime() = time;
    return *this;
  }

  FlightBuilder& setArrivalDateShift(int16_t shift)
  {
    _flight->arrivalDateShift() = shift;
    return *this;
  }

  FlightBuilder& setMarketingCarrier(type::CarrierCode carrierCode)
  {
    _flight->marketingCarrier() = carrierCode;
    return *this;
  }

  FlightBuilder& setMarketingCarrierFlightNumber(type::FlightNumber flightNumber)
  {
    _flight->marketingCarrierFlightNumber() = flightNumber;
    return *this;
  }

  Flight* build() { return _flight; }
};
}
