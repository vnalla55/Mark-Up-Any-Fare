//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tse
{
class TaxText
{
public:
  TaxText() : _itemNo(0) {}

  ~TaxText() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::vector<std::string>& txtMsgs() { return _txtMsgs; }
  const std::vector<std::string>& txtMsgs() const { return _txtMsgs; }

  bool operator==(const TaxText& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_txtMsgs == rhs._txtMsgs));

    return eq;
  }

  static void dummyData(TaxText& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);

    obj._txtMsgs.push_back("TEXT");
    obj._txtMsgs.push_back("TEXT1");
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:
  VendorCode _vendor;
  int _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::vector<std::string> _txtMsgs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _txtMsgs);
  }

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_txtMsgs;
  }
};
}
