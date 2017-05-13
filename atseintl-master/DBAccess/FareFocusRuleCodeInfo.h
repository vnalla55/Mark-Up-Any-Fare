#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusRuleCodeInfo
{
 public:

  FareFocusRuleCodeInfo()
    : _ruleCdItemNo(0)
  {
  }

  uint64_t& ruleCdItemNo() { return _ruleCdItemNo; }
  uint64_t ruleCdItemNo() const { return _ruleCdItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<RuleNumber>& ruleCd() { return _ruleCd; }
  const std::vector<RuleNumber>& ruleCd() const { return _ruleCd; }

  bool operator==(const FareFocusRuleCodeInfo& rhs) const
  {
    return _ruleCdItemNo == rhs._ruleCdItemNo
           && _createDate == rhs._createDate
           && _expireDate == rhs._expireDate
           && _ruleCd == rhs._ruleCd;
  }

  static void dummyData(FareFocusRuleCodeInfo& obj)
  {
    obj._ruleCdItemNo = 12345;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._ruleCd.push_back("AB12");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _ruleCdItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _ruleCd);
  }

 private:
  uint64_t _ruleCdItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<RuleNumber> _ruleCd;
};

}// tse


