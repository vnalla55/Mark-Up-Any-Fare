#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

struct FareRetailerRuleLookupId
{
  FareRetailerRuleLookupId(uint64_t fareRetailerRuleId = 0,
                           uint64_t ruleSeqNo = 0)
    : _fareRetailerRuleId(fareRetailerRuleId),
      _ruleSeqNo(ruleSeqNo)
  {
  }
  uint64_t _fareRetailerRuleId;
  uint64_t _ruleSeqNo;

  bool operator==(const FareRetailerRuleLookupId& rhs) const
  {
    return _fareRetailerRuleId == rhs._fareRetailerRuleId
           && _ruleSeqNo == rhs._ruleSeqNo;
  }
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareRetailerRuleId);
    FLATTENIZE(archive, _ruleSeqNo);
  }
};

class FareRetailerRuleLookupInfo
{
 public:
  FareRetailerRuleLookupInfo()
    : _applicationType(' ')
  {
  }
  Indicator& applicationType() { return _applicationType; }
  const Indicator applicationType() const { return _applicationType; }

  PseudoCityCode& sourcePcc() { return _sourcePcc; }
  const PseudoCityCode& sourcePcc() const { return _sourcePcc; }

  PseudoCityCode& pcc() { return _pcc; }
  const PseudoCityCode& pcc() const { return _pcc; }

  std::vector<FareRetailerRuleLookupId>& fareRetailerRuleLookupIds() { return _fareRetailerRuleLookupIds; }
  const std::vector<FareRetailerRuleLookupId>& fareRetailerRuleLookupIds() const { return _fareRetailerRuleLookupIds; }

  bool operator==(const FareRetailerRuleLookupInfo& rhs) const
  {
    return _applicationType ==rhs._applicationType
           && _sourcePcc == rhs._sourcePcc
           && _pcc == rhs._pcc
           && _fareRetailerRuleLookupIds == rhs._fareRetailerRuleLookupIds;
  }

  static void dummyData(FareRetailerRuleLookupInfo& obj)
  {
    obj._applicationType = 'N';
    obj._sourcePcc = "15X0";
    obj._pcc = "B4T0";

    FareRetailerRuleLookupId frrl(12,23);
    obj._fareRetailerRuleLookupIds.push_back(frrl);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _applicationType);
    FLATTENIZE(archive, _sourcePcc);
    FLATTENIZE(archive, _pcc);
    FLATTENIZE(archive, _fareRetailerRuleLookupIds);
  }

 private:
  Indicator _applicationType;
  PseudoCityCode _sourcePcc;
  PseudoCityCode _pcc;
  std::vector<FareRetailerRuleLookupId> _fareRetailerRuleLookupIds;
};

}// tse

