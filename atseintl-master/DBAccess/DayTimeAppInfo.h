//----------------------------------------------------------------------------
// 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class DayTimeAppInfo : public RuleItemInfo
{
public:
  DayTimeAppInfo()
    : _unavailtag(' '),
      _geoTblItemNo(0),
      _startTime(0),
      _stopTime(0),
      _todAppl(' '),
      _dowSame(' '),
      _dowOccur(0),
      _dayTimeNeg(' '),
      _dayTimeAppl(' '),
      _inhibit(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  int& startTime() { return _startTime; }
  const int& startTime() const { return _startTime; }

  int& stopTime() { return _stopTime; }
  const int& stopTime() const { return _stopTime; }

  Indicator& todAppl() { return _todAppl; }
  const Indicator& todAppl() const { return _todAppl; }

  DayOfWeekCode& dow() { return _dow; }
  const DayOfWeekCode& dow() const { return _dow; }

  Indicator& dowSame() { return _dowSame; }
  const Indicator& dowSame() const { return _dowSame; }

  int& dowOccur() { return _dowOccur; }
  const int& dowOccur() const { return _dowOccur; }

  Indicator& dayTimeNeg() { return _dayTimeNeg; }
  const Indicator& dayTimeNeg() const { return _dayTimeNeg; }

  Indicator& dayTimeAppl() { return _dayTimeAppl; }
  const Indicator& dayTimeAppl() const { return _dayTimeAppl; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const DayTimeAppInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailtag == rhs._unavailtag) &&
            (_geoTblItemNo == rhs._geoTblItemNo) && (_startTime == rhs._startTime) &&
            (_stopTime == rhs._stopTime) && (_todAppl == rhs._todAppl) && (_dow == rhs._dow) &&
            (_dowSame == rhs._dowSame) && (_dowOccur == rhs._dowOccur) &&
            (_dayTimeNeg == rhs._dayTimeNeg) && (_dayTimeAppl == rhs._dayTimeAppl) &&
            (_inhibit == rhs._inhibit));
  }

  static void dummyData(DayTimeAppInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailtag = 'A';
    obj._geoTblItemNo = 1;
    obj._startTime = 2;
    obj._stopTime = 3;
    obj._todAppl = 'B';
    obj._dow = "aaaaaaa";
    obj._dowSame = 'C';
    obj._dowOccur = 4;
    obj._dayTimeNeg = 'D';
    obj._dayTimeAppl = 'E';
    obj._inhibit = 'F';
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
  DateTime _createDate;
  DateTime _expireDate;
  Indicator _unavailtag;
  int _geoTblItemNo;
  int _startTime;
  int _stopTime;
  Indicator _todAppl;
  DayOfWeekCode _dow;
  Indicator _dowSame;
  int _dowOccur;
  Indicator _dayTimeNeg;
  Indicator _dayTimeAppl;
  Indicator _inhibit;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _startTime);
    FLATTENIZE(archive, _stopTime);
    FLATTENIZE(archive, _todAppl);
    FLATTENIZE(archive, _dow);
    FLATTENIZE(archive, _dowSame);
    FLATTENIZE(archive, _dowOccur);
    FLATTENIZE(archive, _dayTimeNeg);
    FLATTENIZE(archive, _dayTimeAppl);
    FLATTENIZE(archive, _inhibit);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_unavailtag
           & ptr->_geoTblItemNo
           & ptr->_startTime
           & ptr->_stopTime
           & ptr->_todAppl
           & ptr->_dow
           & ptr->_dowSame
           & ptr->_dowOccur
           & ptr->_dayTimeNeg
           & ptr->_dayTimeAppl
           & ptr->_inhibit;
  }
};

} // namespace tse
