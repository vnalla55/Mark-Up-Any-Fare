//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class RoutingMap
{
public:
  RoutingMap()
    : _routingTariff(0), _lnkmapsequence(0), _loc1No(0), _loctag(' '), _nextLocNo(0), _altLocNo(0)
  {
  }

  bool operator==(const RoutingMap& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_routingTariff == rhs._routingTariff) && (_routing == rhs._routing) &&
            (_effDate == rhs._effDate) && (_lnkmapsequence == rhs._lnkmapsequence) &&
            (_loc1No == rhs._loc1No) && (_loctag == rhs._loctag) &&
            (_nextLocNo == rhs._nextLocNo) && (_altLocNo == rhs._altLocNo) &&
            (_loc1 == rhs._loc1) && (_localRouting == rhs._localRouting) &&
            (_nation == rhs._nation));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(RoutingMap& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._routingTariff = 1;
    obj._routing = "HIJK";
    obj._effDate = time(nullptr);
    obj._lnkmapsequence = 2;
    obj._loc1No = 3;
    obj._loctag = 'L';
    obj._nextLocNo = 4;
    obj._altLocNo = 5;

    LocKey::dummyData(obj._loc1);

    obj._localRouting = "MNOPQRST";
    obj._nation = "UVWX";
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _routingTariff;
  RoutingNumber _routing;
  DateTime _effDate;
  int _lnkmapsequence;
  int _loc1No;
  Indicator _loctag;
  int _nextLocNo;
  int _altLocNo;
  LocKey _loc1;
  LocCode _localRouting;
  NationCode _nation;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _routingTariff);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _lnkmapsequence);
    FLATTENIZE(archive, _loc1No);
    FLATTENIZE(archive, _loctag);
    FLATTENIZE(archive, _nextLocNo);
    FLATTENIZE(archive, _altLocNo);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _localRouting);
    FLATTENIZE(archive, _nation);
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& routingTariff() { return _routingTariff; }
  const TariffNumber& routingTariff() const { return _routingTariff; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  int& lnkmapsequence() { return _lnkmapsequence; }
  const int& lnkmapsequence() const { return _lnkmapsequence; }

  int& loc1No() { return _loc1No; }
  const int& loc1No() const { return _loc1No; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  Indicator& loctag() { return _loctag; }
  const Indicator& loctag() const { return _loctag; }

  int& nextLocNo() { return _nextLocNo; }
  const int& nextLocNo() const { return _nextLocNo; }

  int& altLocNo() { return _altLocNo; }
  const int& altLocNo() const { return _altLocNo; }

  LocCode& localRouting() { return _localRouting; }
  const LocCode& localRouting() const { return _localRouting; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_carrier & ptr->_routingTariff & ptr->_routing &
           ptr->_effDate & ptr->_lnkmapsequence & ptr->_loc1No & ptr->_loctag & ptr->_nextLocNo &
           ptr->_altLocNo & ptr->_loc1 & ptr->_localRouting & ptr->_nation;
  }
};
}

