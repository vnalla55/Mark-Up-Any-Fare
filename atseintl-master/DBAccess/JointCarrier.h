//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef JOINT_CARRIER_H
#define JOINT_CARRIER_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class JointCarrier
{
public:
  JointCarrier() : _itemNo(0), _seqNo(0), _inhibit(' '), _validityInd(' ') {}
  bool operator==(const JointCarrier& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_inhibit == rhs._inhibit) && (_validityInd == rhs._validityInd) &&
            (_carrier1 == rhs._carrier1) && (_carrier2 == rhs._carrier2) &&
            (_carrier3 == rhs._carrier3));
  }

  static void dummyData(JointCarrier& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._seqNo = 2;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._inhibit = 'E';
    obj._validityInd = 'F';
    obj._carrier1 = "GHI";
    obj._carrier2 = "JKL";
    obj._carrier3 = "MNO";
  }

private:
  VendorCode _vendor;
  int _itemNo;
  int _seqNo;
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _inhibit;
  Indicator _validityInd;
  CarrierCode _carrier1;
  CarrierCode _carrier2;
  CarrierCode _carrier3;

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
    FLATTENIZE(archive, _carrier1);
    FLATTENIZE(archive, _carrier2);
    FLATTENIZE(archive, _carrier3);
  }

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

  CarrierCode& carrier1() { return _carrier1; }
  const CarrierCode& carrier1() const { return _carrier1; }

  CarrierCode& carrier2() { return _carrier2; }
  const CarrierCode& carrier2() const { return _carrier2; }

  CarrierCode& carrier3() { return _carrier3; }
  const CarrierCode& carrier3() const { return _carrier3; }
};
}

#endif
