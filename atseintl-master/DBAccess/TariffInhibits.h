//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TariffInhibits
{
public:
  TariffInhibits() : _tariffCrossRefType(' '), _fareTariff(0), _inhibit(' ') {}
  virtual ~TariffInhibits() {}

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& tariffCrossRefType() { return _tariffCrossRefType; }
  const Indicator& tariffCrossRefType() const { return _tariffCrossRefType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  bool operator==(const TariffInhibits& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_tariffCrossRefType == rhs._tariffCrossRefType) &&
            (_carrier == rhs._carrier) && (_fareTariff == rhs._fareTariff) &&
            (_ruleTariffCode == rhs._ruleTariffCode) && (_inhibit == rhs._inhibit) &&
            (_createDate == rhs._createDate));
  }

  static void dummyData(TariffInhibits& obj)
  {
    obj._vendor = "ABCD";
    obj._tariffCrossRefType = 'D';
    obj._carrier = "EFG";
    obj._fareTariff = 1;
    obj._ruleTariffCode = "HIJLMNO";
    obj._inhibit = 'P';
    obj._createDate = time(nullptr);
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
  Indicator _tariffCrossRefType;
  CarrierCode _carrier;
  TariffNumber _fareTariff;
  TariffCode _ruleTariffCode;
  Indicator _inhibit;
  DateTime _createDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _tariffCrossRefType);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _fareTariff);
    FLATTENIZE(archive, _ruleTariffCode);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _createDate);
  }

private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_tariffCrossRefType
           & ptr->_carrier
           & ptr->_fareTariff
           & ptr->_ruleTariffCode
           & ptr->_inhibit
           & ptr->_createDate;
  }
};
}
