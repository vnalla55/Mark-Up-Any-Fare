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

#include <boost/date_time/posix_time/posix_time.hpp>
#include "DataModel/Common/Types.h"

namespace tax
{

struct InputFlight
{
public:
  InputFlight()
    : _id(0),
      _departureTime(boost::posix_time::not_a_date_time),
      _arrivalTime(boost::posix_time::not_a_date_time),
      _arrivalDateShift(0),
      _marketingCarrierFlightNumber(0),
      _marketingCarrier(UninitializedCode),
      _operatingCarrier(UninitializedCode),
      _equipment(""),
      _cabinCode(type::CabinCode::Blank),
      _reservationDesignator("") {};

  type::Index _id;
  boost::posix_time::time_duration _departureTime;
  boost::posix_time::time_duration _arrivalTime;
  int16_t _arrivalDateShift;
  type::FlightNumber _marketingCarrierFlightNumber;
  type::CarrierCode _marketingCarrier;
  type::CarrierCode _operatingCarrier;
  type::EquipmentCode _equipment;
  type::CabinCode _cabinCode;
  type::ReservationDesignatorCode _reservationDesignator;
};

} // namespace tax
