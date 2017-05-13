#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

// extract SECURITYITEMNO for matching FareFocusRule
// SECURITYITEMNO match FAREFOCUSRULE.SECURITYITEMNO
// PSEUDOCITY - AGENT's PCC
// PSEUDOCITYTYPE = ['T' (branch) 'H' (home)]
// Is PCC allowed for the rule

class FareFocusSecurityDetailInfo
{
 public:
  FareFocusSecurityDetailInfo()
    : _pseudoCityType(' ')
  {
  }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  Indicator& pseudoCityType() { return _pseudoCityType; }
  Indicator pseudoCityType() const { return _pseudoCityType; }

  bool operator==(const FareFocusSecurityDetailInfo& rhs) const
  {
    return _pseudoCity == rhs._pseudoCity
           && _pseudoCityType == rhs._pseudoCityType;
  }

  static void dummyData(FareFocusSecurityDetailInfo& obj)
  {
    obj._pseudoCity = "ABCDE";
    obj._pseudoCityType = 'T';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _pseudoCityType);
  }

 private:
  PseudoCityCode _pseudoCity;
  Indicator _pseudoCityType;
};

}// tse

