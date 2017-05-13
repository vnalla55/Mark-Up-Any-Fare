//----------------------------------------------------------------------------
//
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxExemptionCarrier
{
public:
  TaxExemptionCarrier() : _direction(' '), _flight1(0), _flight2(0) {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  LocCode& airport1() { return _airport1; }
  const LocCode& airport1() const { return _airport1; }

  LocCode& airport2() { return _airport2; }
  const LocCode& airport2() const { return _airport2; }

  Indicator& direction() { return _direction; }
  const Indicator& direction() const { return _direction; }

  FlightNumber& flight1() { return _flight1; }
  const FlightNumber& flight1() const { return _flight1; }

  FlightNumber& flight2() { return _flight2; }
  const FlightNumber& flight2() const { return _flight2; }

  bool operator==(const TaxExemptionCarrier& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_airport1 == rhs._airport1) &&
            (_airport2 == rhs._airport2) && (_direction == rhs._direction) &&
            (_flight1 == rhs._flight1) && (_flight2 == rhs._flight2));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TaxExemptionCarrier& obj)
  {
    obj._carrier = "ABC";
    obj._airport1 = "DEFGH";
    obj._airport2 = "IJKLM";
    obj._direction = 'N';
    obj._flight1 = 1111;
    obj._flight2 = 2222;
  }

private:
  CarrierCode _carrier;
  LocCode _airport1;
  LocCode _airport2;
  Indicator _direction;
  FlightNumber _flight1;
  FlightNumber _flight2;

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_carrier & ptr->_airport1 & ptr->_airport2 & ptr->_direction &
           ptr->_flight1 & ptr->_flight2;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _airport1);
    FLATTENIZE(archive, _airport2);
    FLATTENIZE(archive, _direction);
    FLATTENIZE(archive, _flight1);
    FLATTENIZE(archive, _flight2);
  }
};

template<> struct cdu_pod_traits<TaxExemptionCarrier>: std::true_type{};

}
