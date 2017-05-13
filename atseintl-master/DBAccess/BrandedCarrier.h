//----------------------------------------------------------------------------
// � 2008, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class BrandedCarrier
{

public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  BFVersion& bfvCode() { return _bfvCode; }
  const BFVersion& bfvCode() const { return _bfvCode; }

  bool operator==(const BrandedCarrier& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_description == rhs._description) &&
            (_bfvCode == rhs._bfvCode));
  }

  static void dummyData(BrandedCarrier& obj)
  {
    obj._carrier = "zzz";
    obj._description = "Description";
    obj._bfvCode = "abcde";
  }

private:
  CarrierCode _carrier;
  std::string _description;
  BFVersion _bfvCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _bfvCode);
  }
};
}

