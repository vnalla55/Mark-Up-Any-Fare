//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class RoutingKeyInfo
{
public:
  RoutingKeyInfo() : _routingTariff(0) {}
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& routingTariff() { return _routingTariff; }
  const TariffNumber& routingTariff() const { return _routingTariff; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  TariffCode& routingCode() { return _routingCode; }
  const TariffCode& routingCode() const { return _routingCode; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  bool operator==(const RoutingKeyInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_routingTariff == rhs._routingTariff) && (_routing == rhs._routing) &&
            (_routingCode == rhs._routingCode) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate));
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

  void static dummyData(RoutingKeyInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._routingTariff = 1;
    obj._routing = "HIJK";
    obj._routingCode = "LMNOPQR";
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
  }

protected:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _routingTariff;
  RoutingNumber _routing;
  TariffCode _routingCode;
  DateTime _createDate;
  DateTime _expireDate;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _routingTariff);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _routingCode);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_routingTariff
           & ptr->_routing
           & ptr->_routingCode
           & ptr->_createDate
           & ptr->_expireDate;
  }

};
}
