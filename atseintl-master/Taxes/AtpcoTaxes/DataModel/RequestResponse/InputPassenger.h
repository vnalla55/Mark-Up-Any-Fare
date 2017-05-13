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
#include "DataModel/RequestResponse/InputPassengerStatus.h"

#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace tax
{

struct InputPassenger
{
  type::Index _id {0};
  type::PassengerCode _code {UninitializedCode};
  boost::gregorian::date _birthDate {boost::gregorian::not_a_date_time};
  type::StateProvinceCode _stateCode {""};
  InputPassengerStatus _passengerStatus;
};

} // namespace tax

