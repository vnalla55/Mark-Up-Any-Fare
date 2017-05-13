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
#include "DataModel/Common/CodeIO.h"
#include "DataModel/RequestResponse/InputFlight.h"
#include "DomainDataObjects/Flight.h"

namespace tax
{

const std::set<type::EquipmentCode> Flight::groundEquipments{"BUS", "ICE", "LCH", "TAT", "TCM",
                                                             "TEE", "TGV", "THS", "THT", "TIC",
                                                             "TRN", "TSL"};

Flight::Flight()
  : _departureTime(type::Time::blank_time()),
    _arrivalTime(type::Time::blank_time()),
    _arrivalDateShift(0),
    _marketingCarrierFlightNumber(0),
    _marketingCarrier(UninitializedCode),
    _operatingCarrier(UninitializedCode),
    _equipment(""),
    _cabinCode(type::CabinCode::Blank)
{
}

std::ostream&
Flight::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "DEPARTURETIME: " << _departureTime << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ARRIVALTIME: " << _arrivalTime << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ARRIVALDATESHIFT: " << _arrivalDateShift << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "MARKETINGCARRIERFLIGHTNUMBER: " << _marketingCarrierFlightNumber << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "MARKETINGCARRIER: " << _marketingCarrier << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "OPERATINGCARRIER: " << _operatingCarrier << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "EQUIPMENT: " << _equipment << "\n";

  return out;
}

} // namespace tax
