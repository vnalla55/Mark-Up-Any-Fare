//----------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#ifndef DISCOUNTSEGINFO_H
#define DISCOUNTSEGINFO_H

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class DiscountSegInfo
{
public:
  DiscountSegInfo()
    : _orderNo(0),
      _minNoPsg(0),
      _maxNoPsg(0),
      _minAge1(0),
      _maxAge1(0),
      _minAge2(0),
      _maxAge2(0),
      _minAge3(0),
      _maxAge3(0)
  {
  }

  bool operator==(const DiscountSegInfo& rhs) const
  {
    return ((_orderNo == rhs._orderNo) && (_minNoPsg == rhs._minNoPsg) &&
            (_maxNoPsg == rhs._maxNoPsg) && (_minAge1 == rhs._minAge1) &&
            (_maxAge1 == rhs._maxAge1) && (_minAge2 == rhs._minAge2) &&
            (_maxAge2 == rhs._maxAge2) && (_minAge3 == rhs._minAge3) &&
            (_maxAge3 == rhs._maxAge3) && (_accPsgType1 == rhs._accPsgType1) &&
            (_accPsgType2 == rhs._accPsgType2) && (_accPsgType3 == rhs._accPsgType3) &&
            (_fareClass == rhs._fareClass) && (_bookingCode == rhs._bookingCode));
  }

  static void dummyData(DiscountSegInfo& obj)
  {
    obj._orderNo = 1;
    obj._minNoPsg = 2;
    obj._maxNoPsg = 3;
    obj._minAge1 = 4;
    obj._maxAge1 = 5;
    obj._minAge2 = 6;
    obj._maxAge2 = 7;
    obj._minAge3 = 8;
    obj._maxAge3 = 9;
    obj._accPsgType1 = "ABC";
    obj._accPsgType2 = "DEF";
    obj._accPsgType3 = "GHI";
    obj._fareClass = "JKLMNOPQ";
    obj._bookingCode = "RS";
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
  int _minNoPsg;
  int _maxNoPsg;
  int _minAge1;
  int _maxAge1;
  int _minAge2;
  int _maxAge2;
  int _minAge3;
  int _maxAge3;
  PaxTypeCode _accPsgType1;
  PaxTypeCode _accPsgType2;
  PaxTypeCode _accPsgType3;
  FareClassCode _fareClass;
  BookingCode _bookingCode;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _minNoPsg);
    FLATTENIZE(archive, _maxNoPsg);
    FLATTENIZE(archive, _minAge1);
    FLATTENIZE(archive, _maxAge1);
    FLATTENIZE(archive, _minAge2);
    FLATTENIZE(archive, _maxAge2);
    FLATTENIZE(archive, _minAge3);
    FLATTENIZE(archive, _maxAge3);
    FLATTENIZE(archive, _accPsgType1);
    FLATTENIZE(archive, _accPsgType2);
    FLATTENIZE(archive, _accPsgType3);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _bookingCode);
  }

protected:
public:
  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  int& minNoPsg() { return _minNoPsg; }
  const int& minNoPsg() const { return _minNoPsg; }

  int& maxNoPsg() { return _maxNoPsg; }
  const int& maxNoPsg() const { return _maxNoPsg; }

  int& minAge1() { return _minAge1; }
  const int& minAge1() const { return _minAge1; }

  int& maxAge1() { return _maxAge1; }
  const int& maxAge1() const { return _maxAge1; }

  int& minAge2() { return _minAge2; }
  const int& minAge2() const { return _minAge2; }

  int& maxAge2() { return _maxAge2; }
  const int& maxAge2() const { return _maxAge2; }

  int& minAge3() { return _minAge3; }
  const int& minAge3() const { return _minAge3; }

  int& maxAge3() { return _maxAge3; }
  const int& maxAge3() const { return _maxAge3; }

  PaxTypeCode& accPsgType1() { return _accPsgType1; }
  const PaxTypeCode& accPsgType1() const { return _accPsgType1; }

  PaxTypeCode& accPsgType2() { return _accPsgType2; }
  const PaxTypeCode& accPsgType2() const { return _accPsgType2; }

  PaxTypeCode& accPsgType3() { return _accPsgType3; }
  const PaxTypeCode& accPsgType3() const { return _accPsgType3; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_orderNo
           & ptr->_minNoPsg
           & ptr->_maxNoPsg
           & ptr->_minAge1
           & ptr->_maxAge1
           & ptr->_minAge2
           & ptr->_maxAge2
           & ptr->_minAge3
           & ptr->_maxAge3
           & ptr->_accPsgType1
           & ptr->_accPsgType2
           & ptr->_accPsgType3
           & ptr->_fareClass
           & ptr->_bookingCode;
  }
};
}

#endif
