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

#include <set>
#include "DataModel/Common/Types.h"
#include "Common/Timestamp.h"

namespace tax
{

class Flight
{
public:
  Flight();

  type::Time& departureTime() { return _departureTime; }
  type::Time departureTime() const { return _departureTime; }
  type::Time& arrivalTime() { return _arrivalTime; }
  type::Time arrivalTime() const { return _arrivalTime; }
  int16_t& arrivalDateShift() { return _arrivalDateShift; }
  int16_t arrivalDateShift() const { return _arrivalDateShift; }
  type::FlightNumber& marketingCarrierFlightNumber() { return _marketingCarrierFlightNumber; }
  type::FlightNumber marketingCarrierFlightNumber() const { return _marketingCarrierFlightNumber; }

  type::CarrierCode& marketingCarrier() { return _marketingCarrier; }

  const type::CarrierCode& marketingCarrier() const { return _marketingCarrier; }

  type::CarrierCode& operatingCarrier() { return _operatingCarrier; }

  const type::CarrierCode& operatingCarrier() const { return _operatingCarrier; }

  type::EquipmentCode& equipment() { return _equipment; }

  const type::EquipmentCode& equipment() const { return _equipment; }

  bool isGroundTransport() const
  {
    return groundEquipments.find(_equipment) != groundEquipments.end();
  }

  type::CabinCode& cabinCode() { return _cabinCode; }

  const type::CabinCode& cabinCode() const { return _cabinCode; }

  type::ReservationDesignatorCode& reservationDesignatorCode() { return _reservationDesignatorCode; }

  const type::ReservationDesignatorCode& reservationDesignatorCode() const { return _reservationDesignatorCode; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Time _departureTime;
  type::Time _arrivalTime;
  int16_t _arrivalDateShift;
  type::FlightNumber _marketingCarrierFlightNumber;
  type::CarrierCode _marketingCarrier;
  type::CarrierCode _operatingCarrier;
  type::EquipmentCode _equipment;
  static const std::set<type::EquipmentCode> groundEquipments;
  type::CabinCode _cabinCode;
  type::ReservationDesignatorCode _reservationDesignatorCode;
};

} // namespace tax
