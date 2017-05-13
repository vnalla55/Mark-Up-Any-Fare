//----------------------------------------------------------------------------
//       � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class LimitFareCxrLoc
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  bool operator==(const LimitFareCxrLoc& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2));
  }

  static void dummyData(LimitFareCxrLoc& obj)
  {
    obj._carrier = "ABC";
    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);
  }

private:
  CarrierCode _carrier;
  LocKey _loc1;
  LocKey _loc2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
  }

};
}

