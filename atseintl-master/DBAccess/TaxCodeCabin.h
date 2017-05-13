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
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{
class RBuffer;
class WBuffer;

class TaxCodeCabin
{
public:
  Indicator& exceptInd() { return _exceptInd; }
  const Indicator& exceptInd() const { return _exceptInd; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Directionality& directionalInd() { return _directionalInd; }
  const Directionality& directionalInd() const { return _directionalInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  std::string& classOfService() { return _classOfService; }
  const std::string& classOfService() const { return _classOfService; }

  FlightNumber& flight1() { return _flight1; }
  const FlightNumber& flight1() const { return _flight1; }

  FlightNumber& flight2() { return _flight2; }
  const FlightNumber& flight2() const { return _flight2; }

  bool operator==(const TaxCodeCabin& rhs) const
  {
    return ((_exceptInd == rhs._exceptInd) && (_carrier == rhs._carrier) &&
            (_directionalInd == rhs._directionalInd) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_classOfService == rhs._classOfService) &&
            (_flight1 == rhs._flight1) && (_flight2 == rhs._flight2));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxCodeCabin& obj)
  {
    obj._exceptInd = 'A';
    obj._carrier = "BCD";
    obj._directionalInd = BOTH;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._classOfService = "aaaaaaaa";
    obj._flight1 = 1111;
    obj._flight2 = 2222;
  }

private:
  Indicator _exceptInd = ' ';
  CarrierCode _carrier;
  Directionality _directionalInd = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  std::string _classOfService;
  FlightNumber _flight1 = 0;
  FlightNumber _flight2 = 0;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _exceptInd);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _directionalInd);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _classOfService);
    FLATTENIZE(archive, _flight1);
    FLATTENIZE(archive, _flight2);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_exceptInd & ptr->_carrier & ptr->_directionalInd & ptr->_loc1 &
           ptr->_loc2 & ptr->_classOfService & ptr->_flight1 & ptr->_flight2;
  }
};
}
