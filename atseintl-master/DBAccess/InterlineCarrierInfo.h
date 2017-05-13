//-------------------------------------------------------------------------------
// Copyright 2015, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class InterlineCarrierInfo
{
private:
  CarrierCode _carrier;
  std::vector<CarrierCode> _partners;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _partners);
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  std::vector<CarrierCode>& partners() { return _partners; }
  const std::vector<CarrierCode>& partners() const { return _partners; }

  bool operator==(const InterlineCarrierInfo& rhs) const
  {
    bool eq((_carrier == rhs._carrier) && (_partners.size() == rhs._partners.size()));

    for (size_t i = 0; (eq && (i < _partners.size())); ++i)
    {
      eq = ((_partners[i]) == (rhs._partners[i]));
    }

    return eq;
  }

  static void dummyData(InterlineCarrierInfo& obj)
  {
    obj._carrier = "ABCD";
    obj._partners.push_back(obj._carrier);
  }
};
}

