//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef CARRIER_MIXED_CLASS_SEG_H
#define CARRIER_MIXED_CLASS_SEG_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CarrierMixedClassSeg
{
private:
  Indicator _hierarchy;
  BookingCode _bkgcd;

public:
  CarrierMixedClassSeg() : _hierarchy(' ') {}

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _hierarchy);
    FLATTENIZE(archive, _bkgcd);
  }

  bool operator==(const CarrierMixedClassSeg& rhs) const
  {
    return ((_hierarchy == rhs._hierarchy) && (_bkgcd == rhs._bkgcd));
  }

  static void dummyData(CarrierMixedClassSeg& obj)
  {
    obj._hierarchy = 'A';
    obj._bkgcd = "BC";
  }

  Indicator& hierarchy() { return _hierarchy; }
  const Indicator& hierarchy() const { return _hierarchy; }

  BookingCode& bkgcd() { return _bkgcd; }
  const BookingCode& bkgcd() const { return _bkgcd; }
};
}

#endif // CARRIER_MIXED_CLASS_SEG_H
