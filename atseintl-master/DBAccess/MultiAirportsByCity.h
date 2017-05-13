//----------------------------------------------------------------------------
//     � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class MultiAirportsByCity
{
public:
  LocCode& city() { return _city; }
  const LocCode& city() const { return _city; }

  LocCode& airportCode() { return _airportCode; }
  const LocCode& airportCode() const { return _airportCode; }

  bool operator==(const MultiAirportsByCity& rhs) const
  {
    return ((_city == rhs._city) && (_airportCode == rhs._airportCode));
  }

  static void dummyData(MultiAirportsByCity& obj)
  {
    obj._city = "ABCDEFGH";
    obj._airportCode = "IJKLMNOP";
  }

private:
  LocCode _city;
  LocCode _airportCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _city);
    FLATTENIZE(archive, _airportCode);
  }

};
}

