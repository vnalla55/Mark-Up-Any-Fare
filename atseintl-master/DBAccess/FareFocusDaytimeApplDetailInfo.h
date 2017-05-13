#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{


class FareFocusDaytimeApplDetailInfo
{
 public:
  FareFocusDaytimeApplDetailInfo()
    : _orderno(0), _startTime(0), _stopTime(0)
  {
  }

  uint16_t& orderNo() { return _orderno; }
  const uint16_t& orderNo() const { return _orderno; }
  uint16_t& startTime() { return _startTime; }
  const uint16_t& startTime() const { return _startTime; }
  uint16_t& stopTime() { return _stopTime; }
  const uint16_t& stopTime() const { return _stopTime; }
  Indicator& todAppl() { return _todAppl; }
  const Indicator& todAppl() const { return _todAppl; }
  DowCode& dow() { return _dow; }
  const DowCode& dow() const { return _dow; }
  Indicator& dayTimeNeg() { return _dayTimeNeg; }
  const Indicator& dayTimeNeg() const { return _dayTimeNeg; }
  DateTime& startDate() { return _startDate; }
  const DateTime& startDate() const { return _startDate; }
  DateTime& stopDate() { return _stopDate; }
  const DateTime& stopDate() const { return _stopDate; }

  bool operator==(const FareFocusDaytimeApplDetailInfo& rhs) const
  {
    return _orderno == rhs._orderno
           && _startTime == rhs._startTime
           && _stopTime == rhs._stopTime
           && _todAppl == rhs._todAppl
           && _dow == rhs._dow
           && _dayTimeNeg == rhs._dayTimeNeg
           && _startDate == rhs._startDate
           && _stopDate == rhs._stopDate;
  }

  static void dummyData(FareFocusDaytimeApplDetailInfo& obj)
  {
    static DateTime current(::time(nullptr));
    obj._orderno = 12345;
    obj._startTime = 12345;
    obj._stopTime = 12345;
    obj._todAppl = 'A';
    obj._dow = "ABCD";
    obj._dayTimeNeg = 'C';
    obj._startDate = current;
    obj._stopDate = current;
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _orderno);
    FLATTENIZE(archive, _startTime);
    FLATTENIZE(archive, _stopTime);
    FLATTENIZE(archive, _todAppl);
    FLATTENIZE(archive, _dow);
    FLATTENIZE(archive, _dayTimeNeg);
    FLATTENIZE(archive, _startDate);
    FLATTENIZE(archive, _stopDate);
  }

 private:
  uint16_t _orderno;
  uint16_t _startTime;
  uint16_t _stopTime;
  Indicator _todAppl;
  DowCode _dow;
  Indicator _dayTimeNeg;
  DateTime _startDate;
  DateTime _stopDate;
};

}// tse

