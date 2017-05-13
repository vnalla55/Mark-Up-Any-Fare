//----------------------------------------------------------------------------
//
//      File:           EndOnEndSegment.h
//      Description:    Cat104 segment processing data
//    Created:        3/29/2004
//    Authors:        Roger Kelly
//
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class EndOnEndSegment
{
public:
  bool operator==(const EndOnEndSegment& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_tvltsi == rhs._tvltsi) && (_tvlAppl == rhs._tvlAppl) &&
            (_tvloverwaterInd == rhs._tvloverwaterInd) && (_tvlNonstopInd == rhs._tvlNonstopInd) &&
            (_tvlFareType == rhs._tvlFareType) && (_tvlFareInd == rhs._tvlFareInd) &&
            (_tvldir == rhs._tvldir) && (_tvlLoc1Type == rhs._tvlLoc1Type) &&
            (_tvlLoc1 == rhs._tvlLoc1) && (_tvlLoc2Type == rhs._tvlLoc2Type) &&
            (_tvlLoc2 == rhs._tvlLoc2));
  }

  static void dummyData(EndOnEndSegment& obj)
  {
    obj._orderNo = 1;
    obj._tvltsi = 2;
    obj._tvlAppl = 'A';
    obj._tvloverwaterInd = 'B';
    obj._tvlNonstopInd = 'C';
    obj._tvlFareType = "DEFGHIJK";
    obj._tvlFareInd = 'L';
    obj._tvldir = 'M';
    obj._tvlLoc1Type = NATION;
    obj._tvlLoc1 = "aaaaaaaa";
    obj._tvlLoc2Type = MARKET;
    obj._tvlLoc2 = "bbbbbbbb";
  }

private:
  int _orderNo = 0;
  TSICode _tvltsi = 0;
  Indicator _tvlAppl = ' ';
  Indicator _tvloverwaterInd = ' ';
  Indicator _tvlNonstopInd = ' ';
  FareType _tvlFareType;
  Indicator _tvlFareInd = ' ';
  Indicator _tvldir = ' ';
  LocType _tvlLoc1Type = LocType::UNKNOWN_LOC;
  LocCode _tvlLoc1;
  LocType _tvlLoc2Type = LocType::UNKNOWN_LOC;
  LocCode _tvlLoc2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _tvltsi);
    FLATTENIZE(archive, _tvlAppl);
    FLATTENIZE(archive, _tvloverwaterInd);
    FLATTENIZE(archive, _tvlNonstopInd);
    FLATTENIZE(archive, _tvlFareType);
    FLATTENIZE(archive, _tvlFareInd);
    FLATTENIZE(archive, _tvldir);
    FLATTENIZE(archive, _tvlLoc1Type);
    FLATTENIZE(archive, _tvlLoc1);
    FLATTENIZE(archive, _tvlLoc2Type);
    FLATTENIZE(archive, _tvlLoc2);
  }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  TSICode& tvltsi() { return _tvltsi; }
  const TSICode& tvltsi() const { return _tvltsi; }

  Indicator& tvlAppl() { return _tvlAppl; }
  const Indicator& tvlAppl() const { return _tvlAppl; }

  Indicator& tvloverwaterInd() { return _tvloverwaterInd; }
  const Indicator& tvloverwaterInd() const { return _tvloverwaterInd; }

  Indicator& tvlNonstopInd() { return _tvlNonstopInd; }
  const Indicator& tvlNonstopInd() const { return _tvlNonstopInd; }

  FareType& tvlFareType() { return _tvlFareType; }
  const FareType& tvlFareType() const { return _tvlFareType; }

  Indicator& tvlFareInd() { return _tvlFareInd; }
  const Indicator& tvlFareInd() const { return _tvlFareInd; }

  Indicator& tvldir() { return _tvldir; }
  const Indicator& tvldir() const { return _tvldir; }

  LocType& tvlLoc1Type() { return _tvlLoc1Type; }
  const LocType& tvlLoc1Type() const { return _tvlLoc1Type; }

  LocCode& tvlLoc1() { return _tvlLoc1; }
  const LocCode& tvlLoc1() const { return _tvlLoc1; }

  LocType& tvlLoc2Type() { return _tvlLoc2Type; }
  const LocType& tvlLoc2Type() const { return _tvlLoc2Type; }

  LocCode& tvlLoc2() { return _tvlLoc2; }
  const LocCode& tvlLoc2() const { return _tvlLoc2; }
};
}
