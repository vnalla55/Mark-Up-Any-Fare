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
#include "Common/PassengerUtil.h"
#include "DomainDataObjects/Passenger.h"

namespace tax
{

PassengerUtil::PassengerUtil(const Passenger& passenger) : _passenger(passenger) {}

type::PassengerCode
PassengerUtil::getCode() const
{
  return _passenger._code;
}

type::Date
PassengerUtil::getBirthDate() const
{
  return _passenger._birthDate;
}

type::StateProvinceCode
PassengerUtil::getStateCode() const
{
  return _passenger._stateCode;
}


type::LocZoneText
PassengerUtil::getLocZoneText() const
{
  return _passenger._passengerStatus._residency;
}

type::Nation
PassengerUtil::getNation(const type::PassengerStatusTag& tag) const
{
  if (!_passenger._stateCode.empty())
    return type::Nation("US");

  if (tag == type::PassengerStatusTag::Employee)
    return _passenger._passengerStatus._employment;

  if (tag == type::PassengerStatusTag::National)
    return _passenger._passengerStatus._nationality;

  return type::Nation(UninitializedCode);
}

} // namespace tax
