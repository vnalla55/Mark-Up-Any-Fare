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
#include "DBAccess/LocKey.h"

namespace tse
{

class TpdPsrViaCxrLoc
{
public:
  CarrierCode& viaCarrier() { return _viaCarrier; }
  const CarrierCode& viaCarrier() const { return _viaCarrier; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  bool operator==(const TpdPsrViaCxrLoc& rhs) const
  {
    return ((_viaCarrier == rhs._viaCarrier) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TpdPsrViaCxrLoc& obj)
  {
    obj._viaCarrier = "ABC";
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
  }

private:
  CarrierCode _viaCarrier;
  LocKey _loc1;
  LocKey _loc2;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_viaCarrier & ptr->_loc1 & ptr->_loc2;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _viaCarrier);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
  }
};
}

