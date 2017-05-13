//----------------------------------------------------------------------------
//    � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//    and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//    or transfer of this software/documentation, in any medium, or incorporation of this
//    software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxCodeGenText
{
public:
  TaxCodeGenText() : _messageDisplayCat(' '), _messageOrderNo(0), _itemNo(0) {}

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  Indicator& messageDisplayCat() { return _messageDisplayCat; }
  const Indicator& messageDisplayCat() const { return _messageDisplayCat; }

  int& messageOrderNo() { return _messageOrderNo; }
  const int& messageOrderNo() const { return _messageOrderNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  std::string& keyWord1() { return _keyWord1; }
  const std::string& keyWord1() const { return _keyWord1; }

  std::string& keyWord2() { return _keyWord2; }
  const std::string& keyWord2() const { return _keyWord2; }

  std::vector<std::string>& txtMsgs() { return _txtMsgs; }
  const std::vector<std::string>& txtMsgs() const { return _txtMsgs; }

  bool operator==(const TaxCodeGenText& rhs) const
  {
    return ((_taxCode == rhs._taxCode) && (_messageDisplayCat == rhs._messageDisplayCat) &&
            (_messageOrderNo == rhs._messageOrderNo) && (_createDate == rhs._createDate) &&
            (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
            (_expireDate == rhs._expireDate) && (_itemNo == rhs._itemNo) &&
            (_keyWord1 == rhs._keyWord1) && (_keyWord2 == rhs._keyWord2) &&
            (_txtMsgs == rhs._txtMsgs));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxCodeGenText& obj)
  {
    obj._taxCode = "ABC";
    obj._messageDisplayCat = 'D';
    obj._messageOrderNo = 1;
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._itemNo = 2;
    obj._keyWord1 = "aaaaaaaa";
    obj._keyWord2 = "bbbbbbbb";

    obj._txtMsgs.push_back("cccccccc");
    obj._txtMsgs.push_back("dddddddd");
  }

private:
  TaxCode _taxCode;
  Indicator _messageDisplayCat;
  int _messageOrderNo;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  int _itemNo;
  std::string _keyWord1;
  std::string _keyWord2;
  std::vector<std::string> _txtMsgs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _messageDisplayCat);
    FLATTENIZE(archive, _messageOrderNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _keyWord1);
    FLATTENIZE(archive, _keyWord2);
    FLATTENIZE(archive, _txtMsgs);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_taxCode & ptr->_messageDisplayCat & ptr->_messageOrderNo &
           ptr->_createDate & ptr->_effDate & ptr->_discDate & ptr->_expireDate & ptr->_itemNo &
           ptr->_keyWord1 & ptr->_keyWord2 & ptr->_txtMsgs;
  }
};
}

