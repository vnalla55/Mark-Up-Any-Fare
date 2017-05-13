// ---------------------------------------------------------------------------
//  File:         BookingCodeExceptionSegment.h
//          ****** This is NOT a C++ Exception *****

// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <stdlib.h>

namespace tse
{

/**
BookingCodeExceptionSegment contains the data base data from the
BookingCodeExceptionSegment table.  Each segment belongs to a
vector in BookingCodeExceptionSequence.
*/

class BookingCodeExceptionSegment
{
public:
  BookingCodeExceptionSegment()
    : _segNo(0),
      _primarySecondary(' '),
      _fltRangeAppl(' '),
      _flight1(0),
      _flight2(0),
      _tsi(0),
      _directionInd(' '),
      _loc1Type(' '),
      _loc2Type(' '),
      _posTsi(0),
      _posLocType(' '),
      _soldInOutInd(' '),
      _tvlEffYear(0),
      _tvlEffMonth(0),
      _tvlEffDay(0),
      _tvlDiscYear(0),
      _tvlDiscMonth(0),
      _tvlDiscDay(0),
      _tvlStartTime(0),
      _tvlEndTime(0),
      _fareclassType(' '),
      _restrictionTag(' '),
      _sellTktInd(' ')
  {
  }
  virtual ~BookingCodeExceptionSegment() {};

  const uint16_t& segNo() const { return _segNo; }
  uint16_t& segNo() { return _segNo; }

  const CarrierCode& viaCarrier() const
  {
    return _viaCarrier;
  };
  CarrierCode& viaCarrier()
  {
    return _viaCarrier;
  };

  const char& primarySecondary() const { return _primarySecondary; }
  char& primarySecondary() { return _primarySecondary; }

  const char& fltRangeAppl() const { return _fltRangeAppl; }
  char& fltRangeAppl() { return _fltRangeAppl; }

  const FlightNumber& flight1() const { return _flight1; }
  FlightNumber& flight1() { return _flight1; }

  const FlightNumber& flight2() const { return _flight2; }
  FlightNumber& flight2() { return _flight2; }

  const EquipmentType& equipType() const { return _equipType; }
  EquipmentType& equipType() { return _equipType; }

  const std::string& tvlPortion() const { return _tvlPortion; }
  std::string& tvlPortion() { return _tvlPortion; }

  const TSICode& tsi() const { return _tsi; }
  TSICode& tsi() { return _tsi; }

  const char& directionInd() const { return _directionInd; }
  char& directionInd() { return _directionInd; }

  const char& loc1Type() const { return _loc1Type; }
  char& loc1Type() { return _loc1Type; }

  const LocCode& loc1() const { return _loc1; }
  LocCode& loc1() { return _loc1; }

  const char& loc2Type() const { return _loc2Type; }
  char& loc2Type() { return _loc2Type; }

  const LocCode& loc2() const { return _loc2; }
  LocCode& loc2() { return _loc2; }

  const TSICode& posTsi() const { return _posTsi; }
  TSICode& posTsi() { return _posTsi; }

  const char& posLocType() const { return _posLocType; }
  char& posLocType() { return _posLocType; }

  const LocCode& posLoc() const { return _posLoc; }
  LocCode& posLoc() { return _posLoc; }

  const char& soldInOutInd() const { return _soldInOutInd; }
  char& soldInOutInd() { return _soldInOutInd; }

  const uint16_t& tvlEffYear() const { return _tvlEffYear; }
  uint16_t& tvlEffYear() { return _tvlEffYear; }

  const uint16_t& tvlEffMonth() const { return _tvlEffMonth; }
  uint16_t& tvlEffMonth() { return _tvlEffMonth; }

  const uint16_t& tvlEffDay() const { return _tvlEffDay; }
  uint16_t& tvlEffDay() { return _tvlEffDay; }

  const uint16_t& tvlDiscYear() const { return _tvlDiscYear; }
  uint16_t& tvlDiscYear() { return _tvlDiscYear; }

  const uint16_t& tvlDiscMonth() const { return _tvlDiscMonth; }
  uint16_t& tvlDiscMonth() { return _tvlDiscMonth; }

  const uint16_t& tvlDiscDay() const { return _tvlDiscDay; }
  uint16_t& tvlDiscDay() { return _tvlDiscDay; }

  const std::string& daysOfWeek() const { return _daysOfWeek; }
  std::string& daysOfWeek() { return _daysOfWeek; }

  const uint16_t& tvlStartTime() const { return _tvlStartTime; }
  uint16_t& tvlStartTime() { return _tvlStartTime; }

  const uint16_t& tvlEndTime() const { return _tvlEndTime; }
  uint16_t& tvlEndTime() { return _tvlEndTime; }

  const char& fareclassType() const { return _fareclassType; }
  char& fareclassType() { return _fareclassType; }

  const FareClassCode& fareclass() const { return _fareclass; }
  FareClassCode& fareclass() { return _fareclass; }

  const char& restrictionTag() const { return _restrictionTag; }
  char& restrictionTag() { return _restrictionTag; }

  const BookingCode& bookingCode1() const { return _bookingCode1; }
  BookingCode& bookingCode1() { return _bookingCode1; }

  const BookingCode& bookingCode2() const { return _bookingCode2; }
  BookingCode& bookingCode2() { return _bookingCode2; }

  const Indicator& sellTktInd() const { return _sellTktInd; }
  Indicator& sellTktInd() { return _sellTktInd; }

  const std::string& arbZoneNo() const { return _arbZoneNo; }
  std::string& arbZoneNo() { return _arbZoneNo; }

  virtual bool operator==(const BookingCodeExceptionSegment& rhs) const
  {
    return ((_segNo == rhs._segNo) && (_viaCarrier == rhs._viaCarrier) &&
            (_primarySecondary == rhs._primarySecondary) && (_fltRangeAppl == rhs._fltRangeAppl) &&
            (_flight1 == rhs._flight1) && (_flight2 == rhs._flight2) &&
            (_equipType == rhs._equipType) && (_tvlPortion == rhs._tvlPortion) &&
            (_tsi == rhs._tsi) && (_directionInd == rhs._directionInd) &&
            (_loc1Type == rhs._loc1Type) && (_loc1 == rhs._loc1) && (_loc2Type == rhs._loc2Type) &&
            (_loc2 == rhs._loc2) && (_posTsi == rhs._posTsi) && (_posLocType == rhs._posLocType) &&
            (_posLoc == rhs._posLoc) && (_soldInOutInd == rhs._soldInOutInd) &&
            (_tvlEffYear == rhs._tvlEffYear) && (_tvlEffMonth == rhs._tvlEffMonth) &&
            (_tvlEffDay == rhs._tvlEffDay) && (_tvlDiscYear == rhs._tvlDiscYear) &&
            (_tvlDiscMonth == rhs._tvlDiscMonth) && (_tvlDiscDay == rhs._tvlDiscDay) &&
            (_daysOfWeek == rhs._daysOfWeek) && (_tvlStartTime == rhs._tvlStartTime) &&
            (_tvlEndTime == rhs._tvlEndTime) && (_fareclassType == rhs._fareclassType) &&
            (_fareclass == rhs._fareclass) && (_restrictionTag == rhs._restrictionTag) &&
            (_bookingCode1 == rhs._bookingCode1) && (_bookingCode2 == rhs._bookingCode2) &&
            (_sellTktInd == rhs._sellTktInd) && (_arbZoneNo == rhs._arbZoneNo));
  }

  static void dummyData(BookingCodeExceptionSegment& obj)
  {
    obj._segNo = 1;
    obj._viaCarrier = "ABC";
    obj._primarySecondary = 'D';
    obj._fltRangeAppl = 'E';
    obj._flight1 = 2222;
    obj._flight2 = 3333;
    obj._equipType = "FGH";
    obj._tvlPortion = "aaaaaaaa";
    obj._tsi = 4;
    obj._directionInd = 'I';
    obj._loc1Type = 'J';
    obj._loc1 = "KLMNO";
    obj._loc2Type = 'P';
    obj._loc2 = "QRSTU";
    obj._posTsi = 5;
    obj._posLocType = 'V';
    obj._posLoc = "WXYZa";
    obj._soldInOutInd = 'b';
    obj._tvlEffYear = 6;
    obj._tvlEffMonth = 7;
    obj._tvlEffDay = 8;
    obj._tvlDiscYear = 9;
    obj._tvlDiscMonth = 10;
    obj._tvlDiscDay = 11;
    obj._daysOfWeek = "bbbbbbbb";
    obj._tvlStartTime = 12;
    obj._tvlEndTime = 13;
    obj._fareclassType = 'c';
    obj._fareclass = "defghijk";
    obj._restrictionTag = 'l';
    obj._bookingCode1 = "mn";
    obj._bookingCode2 = "op";
    obj._sellTktInd = 'q';
    obj._arbZoneNo = "cccccccc";
  }

protected:
  uint16_t _segNo;
  CarrierCode _viaCarrier;
  char _primarySecondary;
  char _fltRangeAppl;
  FlightNumber _flight1;
  FlightNumber _flight2;
  EquipmentType _equipType;
  std::string _tvlPortion;
  TSICode _tsi;
  char _directionInd;
  char _loc1Type;
  LocCode _loc1; // This can be up to 5 characters -- its not a 3 char airport code
  char _loc2Type;
  LocCode _loc2;
  TSICode _posTsi; // Point Of Sale
  char _posLocType;
  LocCode _posLoc;
  char _soldInOutInd;
  uint16_t _tvlEffYear;
  uint16_t _tvlEffMonth;
  uint16_t _tvlEffDay;
  uint16_t _tvlDiscYear;
  uint16_t _tvlDiscMonth;
  uint16_t _tvlDiscDay;
  std::string _daysOfWeek;
  uint16_t _tvlStartTime;
  uint16_t _tvlEndTime;
  char _fareclassType;
  FareClassCode _fareclass;
  char _restrictionTag;
  BookingCode _bookingCode1;
  BookingCode _bookingCode2;
  Indicator _sellTktInd;
  std::string _arbZoneNo;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _segNo);
    FLATTENIZE(archive, _viaCarrier);
    FLATTENIZE(archive, _primarySecondary);
    FLATTENIZE(archive, _fltRangeAppl);
    FLATTENIZE(archive, _flight1);
    FLATTENIZE(archive, _flight2);
    FLATTENIZE(archive, _equipType);
    FLATTENIZE(archive, _tvlPortion);
    FLATTENIZE(archive, _tsi);
    FLATTENIZE(archive, _directionInd);
    FLATTENIZE(archive, _loc1Type);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2Type);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _posTsi);
    FLATTENIZE(archive, _posLocType);
    FLATTENIZE(archive, _posLoc);
    FLATTENIZE(archive, _soldInOutInd);
    FLATTENIZE(archive, _tvlEffYear);
    FLATTENIZE(archive, _tvlEffMonth);
    FLATTENIZE(archive, _tvlEffDay);
    FLATTENIZE(archive, _tvlDiscYear);
    FLATTENIZE(archive, _tvlDiscMonth);
    FLATTENIZE(archive, _tvlDiscDay);
    FLATTENIZE(archive, _daysOfWeek);
    FLATTENIZE(archive, _tvlStartTime);
    FLATTENIZE(archive, _tvlEndTime);
    FLATTENIZE(archive, _fareclassType);
    FLATTENIZE(archive, _fareclass);
    FLATTENIZE(archive, _restrictionTag);
    FLATTENIZE(archive, _bookingCode1);
    FLATTENIZE(archive, _bookingCode2);
    FLATTENIZE(archive, _sellTktInd);
    FLATTENIZE(archive, _arbZoneNo);
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

protected:
private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_segNo & ptr->_viaCarrier & ptr->_primarySecondary & ptr->_fltRangeAppl &
           ptr->_flight1 & ptr->_flight2 & ptr->_equipType & ptr->_tvlPortion & ptr->_tsi &
           ptr->_directionInd & ptr->_loc1Type & ptr->_loc1 & ptr->_loc2Type & ptr->_loc2 &
           ptr->_posTsi & ptr->_posLocType & ptr->_posLoc & ptr->_soldInOutInd & ptr->_tvlEffYear &
           ptr->_tvlEffMonth & ptr->_tvlEffDay & ptr->_tvlDiscYear & ptr->_tvlDiscMonth &
           ptr->_tvlDiscDay & ptr->_daysOfWeek & ptr->_tvlStartTime & ptr->_tvlEndTime &
           ptr->_fareclassType & ptr->_fareclass & ptr->_restrictionTag & ptr->_bookingCode1 &
           ptr->_bookingCode2 & ptr->_sellTktInd & ptr->_arbZoneNo;
  }

}; // end BookingCodeExceptionSegment

typedef std::vector<BookingCodeExceptionSegment*> BookingCodeExceptionSegmentVector;
typedef BookingCodeExceptionSegmentVector::iterator BookingCodeExceptionSegmentVectorI;
typedef BookingCodeExceptionSegmentVector::const_iterator BookingCodeExceptionSegmentVectorCI;
} // end of tse
