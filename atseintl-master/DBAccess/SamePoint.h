//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef SAME_POINT_H
#define SAME_POINT_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class SamePoint
{
public:
  SamePoint() : _itemNo(0), _seqNo(0), _inhibit(' '), _validityInd(' ') {}

  bool operator==(const SamePoint& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_inhibit == rhs._inhibit) && (_validityInd == rhs._validityInd) &&
            (_mkt1 == rhs._mkt1) && (_mkt2 == rhs._mkt2));
  }

  static void dummyData(SamePoint& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'E';
    obj._validityInd = 'F';
    obj._mkt1 = "GHIJKLMN";
    obj._mkt2 = "OPQRSTUV";
  }

private:
  VendorCode _vendor;
  int _itemNo;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _inhibit;
  Indicator _validityInd;
  LocCode _mkt1;
  LocCode _mkt2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _mkt1);
    FLATTENIZE(archive, _mkt2);
  }

public:
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& itemNo() { return _itemNo; }
  const int& itemNo() const { return _itemNo; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  LocCode& mkt1() { return _mkt1; }
  const LocCode& mkt1() const { return _mkt1; }

  LocCode& mkt2() { return _mkt2; }
  const LocCode& mkt2() const { return _mkt2; }
};
}

#endif
