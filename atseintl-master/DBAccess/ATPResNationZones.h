//----------------------------------------------------------------------------
// ATPResNationZones.h
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class ATPResNationZones
{
public:
  ATPResNationZones() {}
  ~ATPResNationZones() {}

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  std::vector<std::string>& zones() { return _zones; }
  const std::vector<std::string>& zones() const { return _zones; }

  bool operator==(const ATPResNationZones& rhs) const
  {
    return ((_nation == rhs._nation) && (_zones == rhs._zones));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(ATPResNationZones& obj)
  {
    obj._nation = "ABCD";
    obj._zones.push_back("EFGH");
    obj._zones.push_back("IJKL");
  }

private:
  NationCode _nation;
  std::vector<std::string> _zones;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_nation & ptr->_zones;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _zones);
  }

};
}
