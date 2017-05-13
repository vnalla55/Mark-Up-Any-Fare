//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class BlackoutInfo : public RuleItemInfo
{

public:
  BlackoutInfo()
    : _unavailtag(' '),
      _geoTblItemNoBetween(0),
      _geoTblItemNoAnd(0),
      _blackoutAppl(' '),
      _tvlStartYear(0),
      _tvlStartMonth(0),
      _tvlStartDay(0),
      _tvlStopYear(0),
      _tvlStopMonth(0),
      _tvlStopDay(0),
      _intlRest(' '),
      _inhibit(' ')
  {
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  int& geoTblItemNoBetween() { return _geoTblItemNoBetween; }
  const int& geoTblItemNoBetween() const { return _geoTblItemNoBetween; }

  int& geoTblItemNoAnd() { return _geoTblItemNoAnd; }
  const int& geoTblItemNoAnd() const { return _geoTblItemNoAnd; }

  Indicator& blackoutAppl() { return _blackoutAppl; }
  const Indicator& blackoutAppl() const { return _blackoutAppl; }

  int& tvlStartYear() { return _tvlStartYear; }
  const int& tvlStartYear() const { return _tvlStartYear; }

  int& tvlStartMonth() { return _tvlStartMonth; }
  const int& tvlStartMonth() const { return _tvlStartMonth; }

  int& tvlStartDay() { return _tvlStartDay; }
  const int& tvlStartDay() const { return _tvlStartDay; }

  int& tvlStopYear() { return _tvlStopYear; }
  const int& tvlStopYear() const { return _tvlStopYear; }

  int& tvlStopMonth() { return _tvlStopMonth; }
  const int& tvlStopMonth() const { return _tvlStopMonth; }

  int& tvlStopDay() { return _tvlStopDay; }
  const int& tvlStopDay() const { return _tvlStopDay; }

  Indicator& intlRest() { return _intlRest; }
  const Indicator& intlRest() const { return _intlRest; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  virtual bool operator==(const BlackoutInfo& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_unavailtag == rhs._unavailtag) &&
            (_geoTblItemNoBetween == rhs._geoTblItemNoBetween) &&
            (_geoTblItemNoAnd == rhs._geoTblItemNoAnd) && (_blackoutAppl == rhs._blackoutAppl) &&
            (_tvlStartYear == rhs._tvlStartYear) && (_tvlStartMonth == rhs._tvlStartMonth) &&
            (_tvlStartDay == rhs._tvlStartDay) && (_tvlStopYear == rhs._tvlStopYear) &&
            (_tvlStopMonth == rhs._tvlStopMonth) && (_tvlStopDay == rhs._tvlStopDay) &&
            (_intlRest == rhs._intlRest) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(BlackoutInfo& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._unavailtag = 'A';
    obj._geoTblItemNoBetween = 1;
    obj._geoTblItemNoAnd = 2;
    obj._blackoutAppl = 'B';
    obj._tvlStartYear = 3;
    obj._tvlStartMonth = 4;
    obj._tvlStartDay = 5;
    obj._tvlStopYear = 6;
    obj._tvlStopMonth = 7;
    obj._tvlStopDay = 8;
    obj._intlRest = 'C';
    obj._inhibit = 'D';
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
  int _geoTblItemNoBetween;
  int _geoTblItemNoAnd;
  Indicator _blackoutAppl;
  int _tvlStartYear;
  int _tvlStartMonth;
  int _tvlStartDay;
  int _tvlStopYear;
  int _tvlStopMonth;
  int _tvlStopDay;
  Indicator _intlRest;
  Indicator _inhibit;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _geoTblItemNoBetween);
    FLATTENIZE(archive, _geoTblItemNoAnd);
    FLATTENIZE(archive, _blackoutAppl);
    FLATTENIZE(archive, _tvlStartYear);
    FLATTENIZE(archive, _tvlStartMonth);
    FLATTENIZE(archive, _tvlStartDay);
    FLATTENIZE(archive, _tvlStopYear);
    FLATTENIZE(archive, _tvlStopMonth);
    FLATTENIZE(archive, _tvlStopDay);
    FLATTENIZE(archive, _intlRest);
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
           & ptr->_geoTblItemNoBetween
           & ptr->_geoTblItemNoAnd
           & ptr->_blackoutAppl
           & ptr->_tvlStartYear
           & ptr->_tvlStartMonth
           & ptr->_tvlStartDay
           & ptr->_tvlStopYear
           & ptr->_tvlStopMonth
           & ptr->_tvlStopDay
           & ptr->_intlRest
           & ptr->_inhibit;
  }
};
}

