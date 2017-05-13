//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class SubCodeInfo
{
public:
  SubCodeInfo()
    : _fltTktMerchInd(' '),
      _industryCarrierInd(' '),
      _concur(' '),
      _rfiCode(' '),
      _ssimCode(' '),
      _emdType(' '),
      _taxTextTblItemNo(0),
      _pictureNo(0),
      _consumptionInd(' ')
  {
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  ServiceTypeCode& serviceTypeCode() { return _serviceTypeCode; }
  const ServiceTypeCode& serviceTypeCode() const { return _serviceTypeCode; }

  ServiceSubTypeCode& serviceSubTypeCode() { return _serviceSubTypeCode; }
  const ServiceSubTypeCode& serviceSubTypeCode() const { return _serviceSubTypeCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& fltTktMerchInd() { return _fltTktMerchInd; }
  const Indicator& fltTktMerchInd() const { return _fltTktMerchInd; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& industryCarrierInd() { return _industryCarrierInd; }
  const Indicator& industryCarrierInd() const { return _industryCarrierInd; }

  ServiceGroup& serviceGroup() { return _serviceGroup; }
  const ServiceGroup& serviceGroup() const { return _serviceGroup; }

  ServiceGroup& serviceSubGroup() { return _serviceSubGroup; }
  const ServiceGroup& serviceSubGroup() const { return _serviceSubGroup; }

  ServiceGroupDescription& description1() { return _description1; }
  const ServiceGroupDescription& description1() const { return _description1; }

  ServiceGroupDescription& description2() { return _description2; }
  const ServiceGroupDescription& description2() const { return _description2; }

  Indicator& concur() { return _concur; }
  const Indicator& concur() const { return _concur; }

  Indicator& rfiCode() { return _rfiCode; }
  const Indicator& rfiCode() const { return _rfiCode; }

  SubCodeSSR& ssrCode() { return _ssrCode; }
  const SubCodeSSR& ssrCode() const { return _ssrCode; }

  ServiceDisplayInd& displayCat() { return _displayCat; }
  const ServiceDisplayInd& displayCat() const { return _displayCat; }

  Indicator& ssimCode() { return _ssimCode; }
  const Indicator& ssimCode() const { return _ssimCode; }

  Indicator& emdType() { return _emdType; }
  const Indicator& emdType() const { return _emdType; }

  std::string& commercialName() { return _commercialName; }
  const std::string& commercialName() const { return _commercialName; }

  uint32_t& taxTextTblItemNo() { return _taxTextTblItemNo; }
  const uint32_t& taxTextTblItemNo() const { return _taxTextTblItemNo; }

  uint32_t& pictureNo() { return _pictureNo; }
  const uint32_t& pictureNo() const { return _pictureNo; }

  ServiceBookingInd& bookingInd() { return _bookingInd; }
  const ServiceBookingInd& bookingInd() const { return _bookingInd; }

  Indicator& consumptionInd() { return _consumptionInd; }
  Indicator consumptionInd() const { return _consumptionInd; }

  bool operator==(const SubCodeInfo& second) const
  {
    return (_vendor == second._vendor) && (_carrier == second._carrier) &&
           (_serviceTypeCode == second._serviceTypeCode) &&
           (_serviceSubTypeCode == second._serviceSubTypeCode) &&
           (_createDate == second._createDate) && (_expireDate == second._expireDate) &&
           (_fltTktMerchInd == second._fltTktMerchInd) && (_effDate == second._effDate) &&
           (_discDate == second._discDate) && (_industryCarrierInd == second._industryCarrierInd) &&
           (_serviceGroup == second._serviceGroup) &&
           (_serviceSubGroup == second._serviceSubGroup) &&
           (_description1 == second._description1) && (_description2 == second._description2) &&
           (_concur == second._concur) && (_rfiCode == second._rfiCode) &&
           (_ssrCode == second._ssrCode) && (_displayCat == second._displayCat) &&
           (_ssimCode == second._ssimCode) && (_emdType == second._emdType) &&
           (_commercialName == second._commercialName) &&
           (_taxTextTblItemNo == second._taxTextTblItemNo) && (_pictureNo == second._pictureNo) &&
           (_bookingInd == second._bookingInd) && (_consumptionInd == second._consumptionInd);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _serviceTypeCode);
    FLATTENIZE(archive, _serviceSubTypeCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _fltTktMerchInd);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _industryCarrierInd);
    FLATTENIZE(archive, _serviceGroup);
    FLATTENIZE(archive, _serviceSubGroup);
    FLATTENIZE(archive, _description1);
    FLATTENIZE(archive, _description2);
    FLATTENIZE(archive, _concur);
    FLATTENIZE(archive, _rfiCode);
    FLATTENIZE(archive, _ssrCode);
    FLATTENIZE(archive, _displayCat);
    FLATTENIZE(archive, _ssimCode);
    FLATTENIZE(archive, _emdType);
    FLATTENIZE(archive, _commercialName);
    FLATTENIZE(archive, _taxTextTblItemNo);
    FLATTENIZE(archive, _pictureNo);
    FLATTENIZE(archive, _bookingInd);
    FLATTENIZE(archive, _consumptionInd);
  }

  static void dummyData(SubCodeInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EF";
    obj._serviceTypeCode = "GH";
    obj._serviceSubTypeCode = "IJK";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._fltTktMerchInd = 'L';
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._industryCarrierInd = 'M';
    obj._serviceGroup = "NOP";
    obj._serviceSubGroup = "QRS";
    obj._description1 = "TU";
    obj._description2 = "WV";
    obj._concur = 'X';
    obj._rfiCode = 'Y';
    obj._ssrCode = "ZABC";
    obj._displayCat = "DE";
    obj._ssimCode = 'F';
    obj._emdType = 'G';
    obj._commercialName = "123456789012345678901234567890";
    obj._taxTextTblItemNo = 1;
    obj._pictureNo = 2;
    obj._bookingInd = "HI";
    obj._consumptionInd = 'T';
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
  CarrierCode _carrier;
  ServiceTypeCode _serviceTypeCode;
  ServiceSubTypeCode _serviceSubTypeCode;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _fltTktMerchInd;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _industryCarrierInd;
  ServiceGroup _serviceGroup;
  ServiceGroup _serviceSubGroup;
  ServiceGroupDescription _description1;
  ServiceGroupDescription _description2;
  Indicator _concur;
  Indicator _rfiCode;
  SubCodeSSR _ssrCode;
  ServiceDisplayInd _displayCat;
  Indicator _ssimCode;
  Indicator _emdType;
  std::string _commercialName;
  uint32_t _taxTextTblItemNo;
  uint32_t _pictureNo;
  ServiceBookingInd _bookingInd;
  Indicator _consumptionInd;

  friend class SerializationTestBase;
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_serviceTypeCode
           & ptr->_serviceSubTypeCode
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_fltTktMerchInd
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_industryCarrierInd
           & ptr->_serviceGroup
           & ptr->_serviceSubGroup
           & ptr->_description1
           & ptr->_description2
           & ptr->_concur
           & ptr->_rfiCode
           & ptr->_ssrCode
           & ptr->_displayCat
           & ptr->_ssimCode
           & ptr->_emdType
           & ptr->_commercialName
           & ptr->_taxTextTblItemNo
           & ptr->_pictureNo
           & ptr->_bookingInd
           & ptr->_consumptionInd;
  }
};
} // tse
