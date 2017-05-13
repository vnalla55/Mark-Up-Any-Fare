#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusLookupInfo
{
 public:
  FareFocusLookupInfo()
  {
  }

  PseudoCityCode& pcc() { return _pcc; }
  const PseudoCityCode& pcc() const { return _pcc; }

  std::vector<uint64_t>& fareFocusRuleIds() { return _fareFocusRuleIds; }
  const std::vector<uint64_t>& fareFocusRuleIds() const { return _fareFocusRuleIds; }

  bool operator==(const FareFocusLookupInfo& rhs) const
  {
    return _pcc == rhs._pcc
           && _fareFocusRuleIds == rhs._fareFocusRuleIds;
  }

  static void dummyData(FareFocusLookupInfo& obj)
  {
    obj._pcc = "A1D0";
    obj._fareFocusRuleIds.push_back(111111);
    obj._fareFocusRuleIds.push_back(222);
    obj._fareFocusRuleIds.push_back(3333333333);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pcc);
    FLATTENIZE(archive, _fareFocusRuleIds);
  }

 private:
  PseudoCityCode _pcc;
  std::vector<uint64_t> _fareFocusRuleIds;
};

}// tse

