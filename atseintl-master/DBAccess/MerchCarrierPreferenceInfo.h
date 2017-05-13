//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class MerchCarrierPreferenceInfo
{
public:
  MerchCarrierPreferenceInfo() : _altProcessInd(' '), _sectorPortionInd(' '), _concurrenceInd(' ')
  {
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  VendorCode& prefVendor() { return _prefVendor; }
  const VendorCode& prefVendor() const { return _prefVendor; }

  Indicator& altProcessInd() { return _altProcessInd; }
  const Indicator& altProcessInd() const { return _altProcessInd; }

  Indicator& sectorPortionInd() { return _sectorPortionInd; }
  const Indicator& sectorPortionInd() const { return _sectorPortionInd; }

  Indicator& concurrenceInd() { return _concurrenceInd; }
  const Indicator& concurrenceInd() const { return _concurrenceInd; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  bool operator==(const MerchCarrierPreferenceInfo& second) const
  {
    return (_carrier == second._carrier) && (_groupCode == second._groupCode) &&
           (_createDate == second._createDate) && (_prefVendor == second._prefVendor) &&
           (_altProcessInd == second._altProcessInd) &&
           (_sectorPortionInd == second._sectorPortionInd) &&
           (_concurrenceInd == second._concurrenceInd) && (_expireDate == second._expireDate) &&
           (_effDate == second._effDate) && (_discDate == second._discDate);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _groupCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _prefVendor);
    FLATTENIZE(archive, _altProcessInd);
    FLATTENIZE(archive, _sectorPortionInd);
    FLATTENIZE(archive, _concurrenceInd);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
  }

  static void dummyData(MerchCarrierPreferenceInfo& obj)
  {
    obj._carrier = "AAA";
    obj._groupCode = "EEE";
    obj._createDate = time(nullptr);
    obj._prefVendor = "ABCD";
    obj._altProcessInd = 'I';
    obj._sectorPortionInd = 'Q';
    obj._concurrenceInd = 'C';
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
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
  CarrierCode _carrier;
  ServiceGroup _groupCode;
  DateTime _createDate;
  VendorCode _prefVendor;
  Indicator _altProcessInd;
  Indicator _sectorPortionInd;
  Indicator _concurrenceInd;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;

  friend class SerializationTestBase;
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_carrier
           & ptr->_groupCode
           & ptr->_createDate
           & ptr->_prefVendor
           & ptr->_altProcessInd
           & ptr->_sectorPortionInd
           & ptr->_concurrenceInd
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate;
  }
};
}

