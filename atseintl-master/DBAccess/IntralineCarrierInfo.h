//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
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

class IntralineCarrierInfo
{
private:
  std::string _name;
  std::vector<CarrierCode> _partners;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _name);
    FLATTENIZE(archive, _partners);
  }

  std::string& name() { return _name; }
  const std::string& name() const { return _name; }

  std::vector<CarrierCode>& partners() { return _partners; }
  const std::vector<CarrierCode>& partners() const { return _partners; }

  bool operator==(const IntralineCarrierInfo& rhs) const
  {
    bool eq((_name == rhs._name) && (_partners.size() == rhs._partners.size()));

    for (size_t i = 0; (eq && (i < _partners.size())); ++i)
    {
      eq = ((_partners[i]) == (rhs._partners[i]));
    }

    return eq;
  }

  static void dummyData(IntralineCarrierInfo& obj)
  {
    obj._name = "ABCD";
    obj._partners.push_back(obj._name);
  }
};
}

