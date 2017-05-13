//----------------------------------------------------------------------------
//	    2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//    ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class SvcFeesResBkgDesigInfo
{
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

  Indicator& mkgOperInd() { return _mkgOperInd; }
  const Indicator& mkgOperInd() const { return _mkgOperInd; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  BookingCode& bookingCode1() { return _bookingCode1; }
  const BookingCode& bookingCode1() const { return _bookingCode1; }

  BookingCode& bookingCode2() { return _bookingCode2; }
  const BookingCode& bookingCode2() const { return _bookingCode2; }

  BookingCode& bookingCode3() { return _bookingCode3; }
  const BookingCode& bookingCode3() const { return _bookingCode3; }

  BookingCode& bookingCode4() { return _bookingCode4; }
  const BookingCode& bookingCode4() const { return _bookingCode4; }

  BookingCode& bookingCode5() { return _bookingCode5; }
  const BookingCode& bookingCode5() const { return _bookingCode5; }

  bool operator==(const SvcFeesResBkgDesigInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) && (_seqNo == rhs._seqNo) &&
            (_expireDate == rhs._expireDate) && (_createDate == rhs._createDate) &&
            (_mkgOperInd == rhs._mkgOperInd) && (_carrier == rhs._carrier) &&
            (_bookingCode1 == rhs._bookingCode1) && (_bookingCode2 == rhs._bookingCode2) &&
            (_bookingCode3 == rhs._bookingCode3) && (_bookingCode4 == rhs._bookingCode4) &&
            (_bookingCode5 == rhs._bookingCode5));
  }

  static void dummyData(SvcFeesResBkgDesigInfo& obj)
  {
    obj._vendor = "ATP";
    obj._itemNo = 1;
    obj._seqNo = 1;
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._mkgOperInd = 'E';
    obj._carrier = "AA";
    obj._bookingCode1 = "A";
    obj._bookingCode2 = "A";
    obj._bookingCode3 = "A";
    obj._bookingCode4 = "A";
    obj._bookingCode5 = "A";
  }

private:
  VendorCode _vendor;
  int _itemNo = 0;
  int _seqNo = 0;
  DateTime _expireDate;
  DateTime _createDate;
  Indicator _mkgOperInd = ' ';
  CarrierCode _carrier;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  BookingCode _bookingCode3;
  BookingCode _bookingCode4;
  BookingCode _bookingCode5;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _mkgOperInd);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _bookingCode3);
    FLATTENIZE(archive, _bookingCode4);
    FLATTENIZE(archive, _bookingCode5);
  }
};
}
