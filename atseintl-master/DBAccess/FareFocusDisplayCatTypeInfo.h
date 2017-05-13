#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusDisplayCatTypeInfo
{
 public:
  FareFocusDisplayCatTypeInfo()
    : _displayCatTypeItemNo(0)
  {
  }

  uint64_t& displayCatTypeItemNo() { return _displayCatTypeItemNo; }
  uint64_t displayCatTypeItemNo() const { return _displayCatTypeItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<Indicator>& displayCatType() { return _displayCatType; }
  const std::vector<Indicator>& displayCatType() const { return _displayCatType; }

  bool operator==(const FareFocusDisplayCatTypeInfo& rhs) const
  {
    return _displayCatTypeItemNo == rhs._displayCatTypeItemNo
               && _createDate == rhs._createDate
               && _expireDate == rhs._expireDate
               && _displayCatType == rhs._displayCatType;
  }

  static void dummyData(FareFocusDisplayCatTypeInfo& obj)
  {
    obj._displayCatTypeItemNo = 113333;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._displayCatType.push_back('A');
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _displayCatTypeItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _displayCatType);
  }

 private:
  uint64_t _displayCatTypeItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<Indicator> _displayCatType;
};

}// tse

