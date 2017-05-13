// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxExemption
{
public:

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  PseudoCityCode& channelId() { return _channelId; }
  const PseudoCityCode& channelId() const { return _channelId; }

  ChannelType& channelType() { return _channelType; }
  const ChannelType& channelType() const { return _channelType; }

  OfficeStationCode& officeStationCode() { return _officeStationCode; }
  const OfficeStationCode& officeStationCode() const { return _officeStationCode; }

  std::vector<CarrierCode>& validationCxr() { return _validationCxr; }
  const std::vector<CarrierCode>& validationCxr() const { return _validationCxr; }

  long long int& exemptionSegId() { return _exemptionSegId; }
  const long long int& exemptionSegId() const { return _exemptionSegId; }

  bool operator==(const TaxExemption& rhs) const
  {
    return ((_vendor == rhs._vendor) &&
        (_taxCode == rhs._taxCode) &&
        (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) &&
        (_discDate == rhs._discDate) &&
        (_createDate == rhs._createDate) &&
        (_channelId == rhs._channelId) &&
        (_channelType == rhs._channelType) &&
        (_officeStationCode == rhs._officeStationCode) &&
        (_validationCxr == rhs._validationCxr) &&
        (_exemptionSegId == rhs._exemptionSegId));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxExemption& obj)
  {
    obj._vendor = "STRX";
    obj._taxCode = "ABC";
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._channelId = "DEF";
    obj._channelType = "GHI";
    obj._officeStationCode = "123456";

    obj._validationCxr.push_back("OPQ");
    obj._validationCxr.push_back("RST");

    obj._exemptionSegId = 3456;
  }

private:
  VendorCode _vendor;
  TaxCode _taxCode;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _createDate;
  PseudoCityCode _channelId;
  ChannelType _channelType;
  OfficeStationCode _officeStationCode;
  std::vector<CarrierCode> _validationCxr;
  long long int _exemptionSegId;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer
        & ptr->_vendor
        & ptr->_taxCode
        & ptr->_expireDate
        & ptr->_effDate
        & ptr->_discDate
        & ptr->_createDate
        & ptr->_channelId
        & ptr->_channelType
        & ptr->_officeStationCode
        & ptr->_validationCxr
        & ptr->_exemptionSegId;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _taxCode);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _channelId);
    FLATTENIZE(archive, _channelType);
    FLATTENIZE(archive, _officeStationCode);
    FLATTENIZE(archive, _validationCxr);
    FLATTENIZE(archive, _exemptionSegId);
  }
};

}//namespace tse
