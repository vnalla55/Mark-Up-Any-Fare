//-------------------------------------------------------------------
//
//  File:        RtgKey.h
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class Routing;

class RtgKey
{
public:
  // Routing Key Accessors a.k.a. methods from hell

  const VendorCode& vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }

  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  const RoutingNumber& routingNumber() const { return _routingNumber; }
  RoutingNumber& routingNumber() { return _routingNumber; }

  const TariffNumber& routingTariff() const { return _routingTariff; }
  TariffNumber& routingTariff() { return _routingTariff; }

  const RoutingNumber& addOnRouting1() const { return _addOnRouting1; }
  RoutingNumber& addOnRouting1() { return _addOnRouting1; }

  const RoutingNumber& addOnRouting2() const { return _addOnRouting2; }
  RoutingNumber& addOnRouting2() { return _addOnRouting2; }

  const bool& dirOutbound() const { return _dirOutbound; }
  bool& dirOutbound() { return _dirOutbound; }

  bool operator<(const RtgKey& key) const
  {
    if (this->_routingNumber < key._routingNumber)
      return true;
    if (this->_routingNumber > key._routingNumber)
      return false;
    if (this->_routingTariff < key._routingTariff)
      return true;
    if (this->_routingTariff > key._routingTariff)
      return false;
    if (this->_addOnRouting1 < key._addOnRouting1)
      return true;
    if (this->_addOnRouting1 > key._addOnRouting1)
      return false;
    if (this->_addOnRouting2 < key._addOnRouting2)
      return true;
    if (this->_addOnRouting2 > key._addOnRouting2)
      return false;
    if (this->_vendor < key._vendor)
      return true;
    if (this->_vendor > key._vendor)
      return false;
    if (UNLIKELY(this->_carrier < key._carrier))
      return true;
    if (this->_carrier > key._carrier)
      return false;
    return (this->_dirOutbound < key._dirOutbound);
  }

private:
  RoutingNumber _routingNumber; // for base routing number.
  TariffNumber _routingTariff = 0; // for base routing number.
  RoutingNumber _addOnRouting1; // for add on routing number.
  RoutingNumber _addOnRouting2; // for add on routing number.
  VendorCode _vendor;
  CarrierCode _carrier;
  bool _dirOutbound = false;
};
}
