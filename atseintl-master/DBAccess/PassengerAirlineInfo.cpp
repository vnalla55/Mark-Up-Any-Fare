//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/PassengerAirlineInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

PassengerAirlineInfo::PassengerAirlineInfo()
  : _airlineCode(""),
    _airlineName("")
{
}

void
PassengerAirlineInfo::dummyData(PassengerAirlineInfo& info)
{
  info.setAirlineCode("CC");
  info.setAirlineName("Airline Name");
  const DateTime dt(time(nullptr));
  info.setEffDate(dt);
  info.setDiscDate(dt);
  info.setCreateDate(dt);
  info.setExpireDate(dt);
}

void
PassengerAirlineInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _airlineCode);
  FLATTENIZE(archive, _airlineName);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _expireDate);
}

bool
PassengerAirlineInfo::operator==(const PassengerAirlineInfo& rhs) const
{
  return ((getAirlineCode() == rhs.getAirlineCode()) &&
          (getAirlineName() == rhs.getAirlineName()) &&
          (effDate() == rhs.effDate()) &&
          (discDate() == rhs.discDate()) &&
          (createDate() == rhs.createDate()) &&
          (expireDate() == rhs.expireDate()));
}
}
