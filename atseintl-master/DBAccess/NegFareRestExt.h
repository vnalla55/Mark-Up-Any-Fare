//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class NegFareRestExt
{
public:
  NegFareRestExt()
    : _itemNo(0),
      _staticValueCodeCombInd(' '),
      _tourCodeCombInd(' '),
      _fareBasisAmtInd(' '),
      _tktFareDataSegExistInd(' ')
  {
  }

  virtual ~NegFareRestExt() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint32_t& itemNo() { return _itemNo; }
  const uint32_t& itemNo() const { return _itemNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::string& staticValueCode() { return _staticValueCode; }
  const std::string& staticValueCode() const { return _staticValueCode; }

  Indicator& staticValueCodeCombInd() { return _staticValueCodeCombInd; }
  const Indicator& staticValueCodeCombInd() const { return _staticValueCodeCombInd; }

  Indicator& tourCodeCombInd() { return _tourCodeCombInd; }
  const Indicator& tourCodeCombInd() const { return _tourCodeCombInd; }

  Indicator& fareBasisAmtInd() { return _fareBasisAmtInd; }
  const Indicator& fareBasisAmtInd() const { return _fareBasisAmtInd; }

  Indicator& tktFareDataSegExistInd() { return _tktFareDataSegExistInd; }
  const Indicator& tktFareDataSegExistInd() const { return _tktFareDataSegExistInd; }

  virtual bool operator==(const NegFareRestExt& rhs) const
  {
    return (
        (_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_staticValueCode == rhs._staticValueCode) &&
        (_staticValueCodeCombInd == rhs._staticValueCodeCombInd) &&
        (_tourCodeCombInd == rhs._tourCodeCombInd) && (_fareBasisAmtInd == rhs._fareBasisAmtInd) &&
        (_tktFareDataSegExistInd == rhs._tktFareDataSegExistInd));
  }

  static void dummyData(NegFareRestExt& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 123;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._staticValueCode = "ABC";
    obj._staticValueCodeCombInd = '1';
    obj._tourCodeCombInd = '2';
    obj._fareBasisAmtInd = 'F';
    obj._tktFareDataSegExistInd = 'x';
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
  uint32_t _itemNo;
  DateTime _createDate;
  DateTime _expireDate;
  std::string _staticValueCode;
  Indicator _staticValueCodeCombInd;
  Indicator _tourCodeCombInd;
  Indicator _fareBasisAmtInd;
  Indicator _tktFareDataSegExistInd;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _staticValueCode);
    FLATTENIZE(archive, _staticValueCodeCombInd);
    FLATTENIZE(archive, _tourCodeCombInd);
    FLATTENIZE(archive, _fareBasisAmtInd);
    FLATTENIZE(archive, _tktFareDataSegExistInd);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_staticValueCode
           & ptr->_staticValueCodeCombInd
           & ptr->_tourCodeCombInd
           & ptr->_fareBasisAmtInd
           & ptr->_tktFareDataSegExistInd;
  }
};
}
