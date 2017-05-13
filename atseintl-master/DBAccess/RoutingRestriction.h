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

namespace tse
{

class RoutingRestriction
{
public:
  RoutingRestriction()
    : _restrSeqNo(0),
      _marketAppl(' '),
      _negViaAppl(' '),
      _viaType(' '),
      _nonStopDirectInd(' '),
      _airSurfaceInd(' '),
      _mpm(0),
      _market1type(' '),
      _market2type(' ')
  {
  }

  bool operator==(const RoutingRestriction& rhs) const
  {
    return ((_restrSeqNo == rhs._restrSeqNo) && (_restriction == rhs._restriction) &&
            (_marketAppl == rhs._marketAppl) && (_negViaAppl == rhs._negViaAppl) &&
            (_viaType == rhs._viaType) && (_nonStopDirectInd == rhs._nonStopDirectInd) &&
            (_airSurfaceInd == rhs._airSurfaceInd) && (_market1 == rhs._market1) &&
            (_market2 == rhs._market2) && (_viaMarket == rhs._viaMarket) &&
            (_viaCarrier == rhs._viaCarrier) && (_mpm == rhs._mpm)) &&
            (_market1type == rhs._market1type) && (_market2type == rhs._market2type);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(RoutingRestriction& obj)
  {
    obj._restrSeqNo = 1;
    obj._restriction = "AB";
    obj._marketAppl = 'C';
    obj._negViaAppl = 'D';
    obj._viaType = 'E';
    obj._nonStopDirectInd = 'F';
    obj._airSurfaceInd = 'G';
    obj._market1 = "HIJKLMNO";
    obj._market2 = "PQRSTUVW";
    obj._viaMarket = "XYZabcde";
    obj._viaCarrier = "fgh";
    obj._mpm = 1000;
    obj._market1type = 'C';
    obj._market2type = 'C';
  }

private:
  int _restrSeqNo;
  RestrictionNumber _restriction;
  Indicator _marketAppl;
  Indicator _negViaAppl;
  Indicator _viaType;
  Indicator _nonStopDirectInd;
  Indicator _airSurfaceInd;
  LocCode _market1;
  LocCode _market2;
  LocCode _viaMarket;
  CarrierCode _viaCarrier;
  int _mpm;
  Indicator _market1type;
  Indicator _market2type;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _restrSeqNo);
    FLATTENIZE(archive, _restriction);
    FLATTENIZE(archive, _marketAppl);
    FLATTENIZE(archive, _negViaAppl);
    FLATTENIZE(archive, _viaType);
    FLATTENIZE(archive, _nonStopDirectInd);
    FLATTENIZE(archive, _airSurfaceInd);
    FLATTENIZE(archive, _market1);
    FLATTENIZE(archive, _market2);
    FLATTENIZE(archive, _viaMarket);
    FLATTENIZE(archive, _viaCarrier);
    FLATTENIZE(archive, _mpm);
    FLATTENIZE(archive, _market1type);
    FLATTENIZE(archive, _market2type);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_restrSeqNo & ptr->_restriction & ptr->_marketAppl & ptr->_negViaAppl &
           ptr->_viaType & ptr->_nonStopDirectInd & ptr->_airSurfaceInd & ptr->_market1 &
           ptr->_market2 & ptr->_viaMarket & ptr->_viaCarrier & ptr->_mpm & ptr->_market1type &
           ptr->_market2type;
  }

public:
  static constexpr Indicator PERMITTED = 'P';
  static constexpr Indicator REQUIRED = 'R';
  static constexpr Indicator BETWEEN = 'B'; // Between City1 and City2
  static constexpr Indicator CITY = 'C';
  static constexpr Indicator NONSTOP = 'N';
  static constexpr Indicator DIRECT = 'D';
  static constexpr Indicator EITHER = 'E'; // Either Nonstop or Direct, Either Air or Surface
  static constexpr Indicator AIR = 'A';
  static constexpr Indicator SURFACE = 'S';

  int& restrSeqNo() { return _restrSeqNo; }
  const int& restrSeqNo() const { return _restrSeqNo; }

  RestrictionNumber& restriction() { return _restriction; }
  const RestrictionNumber& restriction() const { return _restriction; }

  Indicator& marketAppl() { return _marketAppl; }
  const Indicator& marketAppl() const { return _marketAppl; }

  Indicator& negViaAppl() { return _negViaAppl; }
  const Indicator& negViaAppl() const { return _negViaAppl; }

  Indicator& viaType() { return _viaType; }
  const Indicator& viaType() const { return _viaType; }

  Indicator& nonStopDirectInd() { return _nonStopDirectInd; }
  const Indicator& nonStopDirectInd() const { return _nonStopDirectInd; }

  Indicator& airSurfaceInd() { return _airSurfaceInd; }
  const Indicator& airSurfaceInd() const { return _airSurfaceInd; }

  LocCode& market1() { return _market1; }
  const LocCode& market1() const { return _market1; }

  LocCode& market2() { return _market2; }
  const LocCode& market2() const { return _market2; }

  LocCode& viaMarket() { return _viaMarket; }
  const LocCode& viaMarket() const { return _viaMarket; }

  CarrierCode& viaCarrier() { return _viaCarrier; }
  const CarrierCode& viaCarrier() const { return _viaCarrier; }

  int& mpm() { return _mpm; }
  const int& mpm() const { return _mpm; }

  Indicator& market1type() { return _market1type; }
  const Indicator& market1type() const { return _market1type; }

  Indicator& market2type() { return _market2type; }
  const Indicator& market2type() const { return _market2type; }
};
}

