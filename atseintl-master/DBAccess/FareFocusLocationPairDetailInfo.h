#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{


class FareFocusLocationPairDetailInfo
{
 public:
  FareFocusLocationPairDetailInfo()
  {
  }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }
  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  bool operator==(const FareFocusLocationPairDetailInfo& rhs) const
  {
    return _loc1 == rhs._loc1
           && _loc2 == rhs._loc2;
  }

  static void dummyData(FareFocusLocationPairDetailInfo& obj)
  {
    obj._loc1.loc() = "DFW";
    obj._loc1.locType() = 'C';
    obj._loc1.loc() = "LAX";
    obj._loc1.locType() = 'C';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
  }

 private:
  LocKey _loc1;
  LocKey _loc2;
};

}// tse

