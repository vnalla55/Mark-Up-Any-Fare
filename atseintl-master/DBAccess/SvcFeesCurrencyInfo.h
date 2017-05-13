//----------------------------------------------------------------------------
//   2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//   ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class SvcFeesCurrencyInfo
{
public:
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  LocKey& posLoc() { return _posLoc; }
  const LocKey& posLoc() const { return _posLoc; }

  CurrencyCode& currency() { return _currency; }
  const CurrencyCode& currency() const { return _currency; }

  MoneyAmount& feeAmount() { return _feeAmount; }
  const MoneyAmount& feeAmount() const { return _feeAmount; }

  CurrencyNoDec& noDec() { return _noDec; }
  const CurrencyNoDec& noDec() const { return _noDec; }

  bool operator==(const SvcFeesCurrencyInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_posLoc == rhs._posLoc) && (_currency == rhs._currency) &&
            (_feeAmount == rhs._feeAmount) && (_noDec == rhs._noDec));
  }

  static void dummyData(SvcFeesCurrencyInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 1;
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    LocKey::dummyData(obj._posLoc);
    obj._currency = "USD";
    obj._feeAmount = 100.00;
    obj._noDec = 2;
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  VendorCode _vendor;
  int _itemNo = 0;
  int _seqNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  LocKey _posLoc;
  CurrencyCode _currency;
  MoneyAmount _feeAmount = 0;
  CurrencyNoDec _noDec = 0;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _currency);
    FLATTENIZE(archive, _feeAmount);
    FLATTENIZE(archive, _noDec);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_seqNo
           & ptr->_expireDate
           & ptr->_createDate
           & ptr->_posLoc
           & ptr->_currency
           & ptr->_feeAmount
           & ptr->_noDec;
  }
};
}
