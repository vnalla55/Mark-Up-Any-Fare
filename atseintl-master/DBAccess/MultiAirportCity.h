//----------------------------------------------------------------------------
//     © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#ifndef MULTIAIRPORTCITY_H
#define MULTIAIRPORTCITY_H

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class MultiAirportCity
{
public:
  bool operator==(const MultiAirportCity& rhs) const
  {
    return ((_airportCode == rhs._airportCode) && (_city == rhs._city));
  }

  static void dummyData(MultiAirportCity& obj)
  {
    obj._airportCode = "aaaaaaaa";
    obj._city = "bbbbbbbb";
  }

protected:
  LocCode _airportCode;
  LocCode _city;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _airportCode);
    FLATTENIZE(archive, _city);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
public:
  LocCode& airportCode() { return _airportCode; }
  const LocCode& airportCode() const { return _airportCode; }

  LocCode& city() { return _city; }
  const LocCode& city() const { return _city; }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_airportCode
           & ptr->_city;
  }
};
}

#endif
