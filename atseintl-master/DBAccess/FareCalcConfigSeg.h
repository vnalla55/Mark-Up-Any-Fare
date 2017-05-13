//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareCalcConfigSeg
{
public:
  LocCode& marketLoc() { return _marketLoc; }
  const LocCode& marketLoc() const { return _marketLoc; }

  LocCode& displayLoc() { return _displayLoc; }
  const LocCode& displayLoc() const { return _displayLoc; }

  bool operator==(const FareCalcConfigSeg& rhs) const
  {
    return ((_marketLoc == rhs._marketLoc) && (_displayLoc == rhs._displayLoc));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(FareCalcConfigSeg& obj)
  {
    obj._marketLoc = "aaaaaaaa";
    obj._displayLoc = "bbbbbbbb";
  }

private:
  LocCode _marketLoc;
  LocCode _displayLoc;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_marketLoc & ptr->_displayLoc;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _marketLoc);
    FLATTENIZE(archive, _displayLoc);
  }

};
}

