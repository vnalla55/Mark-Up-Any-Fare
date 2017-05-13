//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class StopoversInfoSeg
{
public:
  StopoversInfoSeg()
    : _orderNo(0), _carrierInd(' '), _stopoverGeoAppl(' '), _stopoverInOutInd(' '), _chargeInd(' ')
  {
  }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  Indicator& carrierInd() { return _carrierInd; }
  const Indicator& carrierInd() const { return _carrierInd; }

  std::string& noStops() { return _noStops; }
  const std::string& noStops() const { return _noStops; }

  CarrierCode& carrierIn() { return _carrierIn; }
  const CarrierCode& carrierIn() const { return _carrierIn; }

  Indicator& stopoverGeoAppl() { return _stopoverGeoAppl; }
  const Indicator& stopoverGeoAppl() const { return _stopoverGeoAppl; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  CarrierCode& carrierOut() { return _carrierOut; }
  const CarrierCode& carrierOut() const { return _carrierOut; }

  Indicator& stopoverInOutInd() { return _stopoverInOutInd; }
  const Indicator& stopoverInOutInd() const { return _stopoverInOutInd; }

  Indicator& chargeInd() { return _chargeInd; }
  const Indicator& chargeInd() const { return _chargeInd; }

  bool operator==(const StopoversInfoSeg& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_carrierInd == rhs._carrierInd) &&
            (_noStops == rhs._noStops) && (_carrierIn == rhs._carrierIn) &&
            (_stopoverGeoAppl == rhs._stopoverGeoAppl) && (_loc1 == rhs._loc1) &&
            (_loc2 == rhs._loc2) && (_carrierOut == rhs._carrierOut) &&
            (_stopoverInOutInd == rhs._stopoverInOutInd) && (_chargeInd == rhs._chargeInd));
  }

  static void dummyData(StopoversInfoSeg& obj)
  {
    obj._orderNo = 1;
    obj._carrierInd = 'A';
    obj._noStops = "aaaaaaaa";
    obj._carrierIn = "BCD";
    obj._stopoverGeoAppl = 'E';

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._carrierOut = "FGH";
    obj._stopoverInOutInd = 'I';
    obj._chargeInd = 'J';
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
  Indicator _carrierInd;
  std::string _noStops;
  CarrierCode _carrierIn;
  Indicator _stopoverGeoAppl;
  LocKey _loc1;
  LocKey _loc2;
  CarrierCode _carrierOut;
  Indicator _stopoverInOutInd;
  Indicator _chargeInd;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _carrierInd);
    FLATTENIZE(archive, _noStops);
    FLATTENIZE(archive, _carrierIn);
    FLATTENIZE(archive, _stopoverGeoAppl);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _carrierOut);
    FLATTENIZE(archive, _stopoverInOutInd);
    FLATTENIZE(archive, _chargeInd);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_orderNo
           & ptr->_carrierInd
           & ptr->_noStops
           & ptr->_carrierIn
           & ptr->_stopoverGeoAppl
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_carrierOut
           & ptr->_stopoverInOutInd
           & ptr->_chargeInd;
  }
};
}

