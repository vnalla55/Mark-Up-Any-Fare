//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TSEDateInterval.h"

#include <vector>

namespace tse
{
class WBuffer;
class RBuffer;

class AddonZoneInfo
{
public:
  AddonZoneInfo() : _fareTariff(0), _inclExclInd(' '), _zone(0) {}

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  const DateTime& discDate() const { return _effInterval.discDate(); }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& inclExclInd() { return _inclExclInd; }
  const Indicator& inclExclInd() const { return _inclExclInd; }

  LocKey& market() { return _market; }
  const LocKey& market() const { return _market; }

  AddonZone& zone() { return _zone; }
  const AddonZone& zone() const { return _zone; }

  bool operator==(const AddonZoneInfo& rhs) const;

  WBuffer& write(WBuffer& os) const;

  RBuffer& read(RBuffer& is);

  static void dummyData(AddonZoneInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

private:
  TSEDateInterval _effInterval;
  VendorCode _vendor;
  TariffNumber _fareTariff;
  CarrierCode _carrier;
  Indicator _inclExclInd;
  LocKey _market;
  AddonZone _zone;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer
           & ptr->_effInterval
           & ptr->_vendor
           & ptr->_fareTariff
           & ptr->_carrier
           & ptr->_inclExclInd
           & ptr->_market
           & ptr->_zone;
  }
};

} // tse

