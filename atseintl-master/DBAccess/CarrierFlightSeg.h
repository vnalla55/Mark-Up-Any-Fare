//----------------------------------------------------------------------------
//     2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CarrierFlightSeg
{
public:
  CarrierFlightSeg() : _orderNo(0), _flt1(0), _flt2(0) {}

  virtual ~CarrierFlightSeg() {}

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  CarrierCode& marketingCarrier() { return _marketingCarrier; }
  const CarrierCode& marketingCarrier() const { return _marketingCarrier; }

  CarrierCode& operatingCarrier() { return _operatingCarrier; }
  const CarrierCode& operatingCarrier() const { return _operatingCarrier; }

  FlightNumber& flt1() { return _flt1; }
  const FlightNumber& flt1() const { return _flt1; }

  FlightNumber& flt2() { return _flt2; }
  const FlightNumber& flt2() const { return _flt2; }

  virtual bool operator==(const CarrierFlightSeg& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_marketingCarrier == rhs._marketingCarrier) &&
            (_operatingCarrier == rhs._operatingCarrier) && (_flt1 == rhs._flt1) &&
            (_flt2 == rhs._flt2));
  }

  static void dummyData(CarrierFlightSeg& obj)
  {
    obj._orderNo = 1;
    obj._marketingCarrier = "ABC";
    obj._operatingCarrier = "DEF";
    obj._flt1 = 2222;
    obj._flt2 = 3333;
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
  int _orderNo;
  CarrierCode _marketingCarrier;
  CarrierCode _operatingCarrier;
  FlightNumber _flt1;
  FlightNumber _flt2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _marketingCarrier);
    FLATTENIZE(archive, _operatingCarrier);
    FLATTENIZE(archive, _flt1);
    FLATTENIZE(archive, _flt2);
  }

protected:
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_orderNo
           & ptr->_marketingCarrier
           & ptr->_operatingCarrier
           & ptr->_flt1
           & ptr->_flt2;
  }

};
}
