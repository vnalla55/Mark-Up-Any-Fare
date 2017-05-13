//----------------------------------------------------------------------------
//	    2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class SvcFeesCxrResultingFCLInfo
{
public:
  SvcFeesCxrResultingFCLInfo() : _itemNo(0), _seqNo(0) {}

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

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  FareClassCode& resultingFCL() { return _resultingFCL; }
  const FareClassCode& resultingFCL() const { return _resultingFCL; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  bool operator==(const SvcFeesCxrResultingFCLInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_carrier == rhs._carrier) && (_resultingFCL == rhs._resultingFCL) &&
            (_fareType == rhs._fareType));
  }

  static void dummyData(SvcFeesCxrResultingFCLInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 1;
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._carrier = "AA";
    obj._resultingFCL = "L-3LGPN";
    obj._fareType = "ER";
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
  int _itemNo;
  int _seqNo;
  DateTime _expireDate;
  DateTime _createDate;
  CarrierCode _carrier;
  FareClassCode _resultingFCL;
  FareType _fareType;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _resultingFCL);
    FLATTENIZE(archive, _fareType);
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
           & ptr->_carrier
           & ptr->_resultingFCL
           & ptr->_fareType;
  }
};
}
