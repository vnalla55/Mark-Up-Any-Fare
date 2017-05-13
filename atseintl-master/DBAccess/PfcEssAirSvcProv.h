//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef PFCESSAIRSVCPROV_H
#define PFCESSAIRSVCPROV_H

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PfcEssAirSvcProv
{
public:
  PfcEssAirSvcProv() : _orderNo(0), _flt1(0), _flt2(0) {}

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  CarrierCode& easCarrier() { return _easCarrier; }
  const CarrierCode& easCarrier() const { return _easCarrier; }

  FlightNumber& flt1() { return _flt1; }
  const FlightNumber& flt1() const { return _flt1; }

  FlightNumber& flt2() { return _flt2; }
  const FlightNumber& flt2() const { return _flt2; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  bool operator==(const PfcEssAirSvcProv& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_easCarrier == rhs._easCarrier) &&
            (_flt1 == rhs._flt1) && (_flt2 == rhs._flt2) && (_vendor == rhs._vendor));
  }

  static void dummyData(PfcEssAirSvcProv& obj)
  {
    obj._orderNo = 1;
    obj._easCarrier = "ABC";
    obj._flt1 = 2222;
    obj._flt2 = 3333;
    obj._vendor = "DEFG";
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
  CarrierCode _easCarrier;
  FlightNumber _flt1;
  FlightNumber _flt2;
  VendorCode _vendor;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _easCarrier);
    FLATTENIZE(archive, _flt1);
    FLATTENIZE(archive, _flt2);
    FLATTENIZE(archive, _vendor);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_orderNo
           & ptr->_easCarrier
           & ptr->_flt1
           & ptr->_flt2
           & ptr->_vendor;
  }
};
}
#endif
