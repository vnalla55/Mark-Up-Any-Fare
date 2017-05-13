
//----------------------------------------------------------------------------
// (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{
// typedef std::vector<tse::CarrierCode> CarrierList;
class Cat05OverrideCarrier
{
public:
  Cat05OverrideCarrier() {}
  virtual ~Cat05OverrideCarrier() {}

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  std::vector<CarrierCode>& carrierList() { return _carrierList; }
  const std::vector<CarrierCode>& carrierList() const { return _carrierList; }

  bool operator==(const Cat05OverrideCarrier& rhs) const
  {
    bool eq(_pseudoCity == rhs._pseudoCity);
    eq = eq && (_carrierList.size() == rhs._carrierList.size());
    for (size_t i = 0; i < _carrierList.size() && eq; ++i)
    {
      eq = (_carrierList[i] == rhs._carrierList[i]);
    }
    return eq;
  }

  static void dummyData(Cat05OverrideCarrier& obj)
  {
    obj._pseudoCity = "8HF6";
    obj._carrierList.push_back("JL");
    obj._carrierList.push_back("AA");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _carrierList);
  }

private:
  PseudoCityCode _pseudoCity;
  std::vector<CarrierCode> _carrierList;
};

} // tse

