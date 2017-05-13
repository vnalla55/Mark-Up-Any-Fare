//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//   ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class SvcFeesSecurityInfo
{
public:
  SvcFeesSecurityInfo() : _itemNo(0), _seqNo(0), _travelAgencyInd(' '), _viewBookTktInd(' ') {}
  ~SvcFeesSecurityInfo() {}

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

  Indicator& travelAgencyInd() { return _travelAgencyInd; }
  const Indicator& travelAgencyInd() const { return _travelAgencyInd; }

  CarrierCode& carrierGdsCode() { return _carrierGdsCode; }
  const CarrierCode& carrierGdsCode() const { return _carrierGdsCode; }

  AgencyDutyCode& dutyFunctionCode() { return _dutyFunctionCode; }
  const AgencyDutyCode& dutyFunctionCode() const { return _dutyFunctionCode; }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  LocKey& code() { return _code; }
  const LocKey& code() const { return _code; }

  Indicator& viewBookTktInd() { return _viewBookTktInd; }
  const Indicator& viewBookTktInd() const { return _viewBookTktInd; }

  bool operator==(const SvcFeesSecurityInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_travelAgencyInd == rhs._travelAgencyInd) &&
            (_carrierGdsCode == rhs._carrierGdsCode) &&
            (_dutyFunctionCode == rhs._dutyFunctionCode) && (_loc == rhs._loc) &&
            (_code == rhs._code) && (_viewBookTktInd == rhs._viewBookTktInd));
  }

  static void dummyData(SvcFeesSecurityInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 1;
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._travelAgencyInd = ' ';
    obj._carrierGdsCode = "1S";
    obj._dutyFunctionCode = "AB";
    LocKey::dummyData(obj._loc);
    LocKey::dummyData(obj._code);
    obj._viewBookTktInd = ' ';
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
  Indicator _travelAgencyInd;
  CarrierCode _carrierGdsCode;
  AgencyDutyCode _dutyFunctionCode;
  LocKey _loc;
  LocKey _code;
  Indicator _viewBookTktInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _travelAgencyInd);
    FLATTENIZE(archive, _carrierGdsCode);
    FLATTENIZE(archive, _dutyFunctionCode);
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _code);
    FLATTENIZE(archive, _viewBookTktInd);
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
           & ptr->_travelAgencyInd
           & ptr->_carrierGdsCode
           & ptr->_dutyFunctionCode
           & ptr->_loc
           & ptr->_code
           & ptr->_viewBookTktInd;
  }
};
}
