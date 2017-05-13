#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusPsgTypeInfo
{
 public:
  FareFocusPsgTypeInfo()
    : _psgTypeItemNo(0)
  {
  }

  uint64_t& psgTypeItemNo() { return _psgTypeItemNo; }
  uint64_t psgTypeItemNo() const { return _psgTypeItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<PaxTypeCode>& psgType() { return _psgType; }
  const std::vector<PaxTypeCode>& psgType() const { return _psgType; }

  bool operator==(const FareFocusPsgTypeInfo& rhs) const
  {
    return _psgTypeItemNo == rhs._psgTypeItemNo
               && _createDate == rhs._createDate
               && _expireDate == rhs._expireDate
               && _psgType == rhs._psgType;
  }

  static void dummyData(FareFocusPsgTypeInfo& obj)
  {
    obj._psgTypeItemNo = 113333;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._psgType.push_back("AAA");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _psgTypeItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _psgType);
  }

 private:
  uint64_t _psgTypeItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<PaxTypeCode> _psgType;
};

}// tse

