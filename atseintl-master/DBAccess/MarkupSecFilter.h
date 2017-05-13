//----------------------------------------------------------------------------
//       � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class MarkupSecFilter
{
public:
  MarkupSecFilter() : _ruleTariff(0), _pseudoCityInd(' '), _pseudoCityType(' ') {}
  virtual ~MarkupSecFilter() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  Indicator& pseudoCityInd() { return _pseudoCityInd; }
  const Indicator& pseudoCityInd() const { return _pseudoCityInd; }

  Indicator& pseudoCityType() { return _pseudoCityType; }
  const Indicator& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  AgencyIATA& iataTvlAgencyNo() { return _iataTvlAgencyNo; }
  const AgencyIATA& iataTvlAgencyNo() const { return _iataTvlAgencyNo; }

  CarrierCode& carrierCrs() { return _carrierCrs; }
  const CarrierCode& carrierCrs() const { return _carrierCrs; }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  bool operator==(const MarkupSecFilter& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_ruleTariff == rhs._ruleTariff) && (_rule == rhs._rule) &&
            (_pseudoCityInd == rhs._pseudoCityInd) && (_pseudoCityType == rhs._pseudoCityType) &&
            (_pseudoCity == rhs._pseudoCity) && (_iataTvlAgencyNo == rhs._iataTvlAgencyNo) &&
            (_carrierCrs == rhs._carrierCrs) && (_loc == rhs._loc));
  }

  static void dummyData(MarkupSecFilter& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._ruleTariff = 1;
    obj._rule = "HIJK";
    obj._pseudoCityInd = 'L';
    obj._pseudoCityType = 'M';
    obj._pseudoCity = "NOPQR";
    obj._iataTvlAgencyNo = "STUVWXYZ";
    obj._carrierCrs = "abc";

    LocKey::dummyData(obj._loc);
  }

  virtual WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  VendorCode _vendor;
  CarrierCode _carrier;
  DateTime _createDate;
  DateTime _expireDate;
  TariffNumber _ruleTariff;
  RuleNumber _rule;
  Indicator _pseudoCityInd;
  Indicator _pseudoCityType;
  PseudoCityCode _pseudoCity;
  AgencyIATA _iataTvlAgencyNo;
  CarrierCode _carrierCrs;
  LocKey _loc;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _pseudoCityInd);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _iataTvlAgencyNo);
    FLATTENIZE(archive, _carrierCrs);
    FLATTENIZE(archive, _loc);
  }

private:

  template <typename B, typename T> static B& convert(B& buffer, T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_ruleTariff
           & ptr->_rule
           & ptr->_pseudoCityInd
           & ptr->_pseudoCityType
           & ptr->_pseudoCity
           & ptr->_iataTvlAgencyNo
           & ptr->_carrierCrs
           & ptr->_loc;
  }
};
}
