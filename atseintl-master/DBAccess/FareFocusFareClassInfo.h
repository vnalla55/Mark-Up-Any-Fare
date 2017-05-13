#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FareFocusFareClassInfo
{
 public:

  FareFocusFareClassInfo()
    : _fareClassItemNo(0)
  {
  }

  long& fareClassItemNo() { return _fareClassItemNo; }
  long fareClassItemNo() const { return _fareClassItemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<FareClassCodeC>& fareClass() { return _fareClass; }
  const std::vector<FareClassCodeC>& fareClass() const { return _fareClass; }

  bool operator==(const FareFocusFareClassInfo& rhs) const
  {
    return _fareClassItemNo == rhs._fareClassItemNo
           && _createDate == rhs._createDate
           && _expireDate == rhs._expireDate
           && _fareClass == rhs._fareClass;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(FareFocusFareClassInfo& obj)
  {
    obj._fareClassItemNo = 111111;
    obj._createDate = ::time(nullptr);
    obj._expireDate = ::time(nullptr);
    obj._fareClass.push_back("ABCDEFGI");
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareClassItemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fareClass);
  }

 private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_fareClassItemNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_fareClass;
  }

  long _fareClassItemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<FareClassCodeC> _fareClass;
};

}// tse

