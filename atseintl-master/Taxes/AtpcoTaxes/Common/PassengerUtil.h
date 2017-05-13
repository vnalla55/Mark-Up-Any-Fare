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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/Nation.h"
#include "Common/Timestamp.h"

namespace tax
{

class Passenger;

class PassengerUtil
{
public:
  explicit PassengerUtil(const Passenger& passenger);

  type::PassengerCode getCode() const;
  type::Date getBirthDate() const;
  type::StateProvinceCode getStateCode() const;
  type::LocZoneText getLocZoneText() const;
  type::Nation getNation(const type::PassengerStatusTag& tag) const;

private:
  const Passenger& _passenger;

  PassengerUtil(const PassengerUtil&);
  PassengerUtil& operator=(const PassengerUtil&);
};

} // namespace tax
