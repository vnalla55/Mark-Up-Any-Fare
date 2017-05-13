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
#include "DomainDataObjects/PassengerStatus.h"

namespace tax
{

struct Passenger
{
  type::PassengerCode _code {UninitializedCode};
  type::Date _birthDate {type::Date::invalid_date()};
  type::StateProvinceCode _stateCode {""};
  PassengerStatus _passengerStatus {};

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const
  {
    const std::string indent(indentLevel, indentChar);
    out << indent << "CODE: " << _code.asString() << "\n";
    out << indent << "BIRTHDATE: " << _birthDate << "\n";
    out << indent << "STATEPROVINCECODE: " << _stateCode << "\n";
    out << indent << "NATIONALITY: " << _passengerStatus._nationality.asString() << "\n";
    out << indent << "RESIDENCY: " << _passengerStatus._residency.asString() << "\n";
    out << indent << "EMPLOYMENT: " << _passengerStatus._employment.asString() << "\n";
    return out;
  }
};

} // namespace tax

