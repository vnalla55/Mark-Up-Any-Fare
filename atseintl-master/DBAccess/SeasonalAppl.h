//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class SeasonalAppl : public RuleItemInfo
{
public:
  SeasonalAppl()
    : _validityInd(' '),
      _inhibit(' '),
      _unavailtag(' '),
      _geoTblItemNo(0),
      _seasonDateAppl(' '),
      _tvlstartyear(0),
      _tvlstartmonth(0),
      _tvlstartDay(0),
      _tvlStopyear(0),
      _tvlStopmonth(0),
      _tvlStopDay(0),
      _assumptionOverride(' ')
  {
  }

  virtual ~SeasonalAppl() {}

  virtual bool operator==(const SeasonalAppl& rhs) const
  {
    return ((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_validityInd == rhs._validityInd) &&
            (_inhibit == rhs._inhibit) && (_unavailtag == rhs._unavailtag) &&
            (_geoTblItemNo == rhs._geoTblItemNo) && (_seasonDateAppl == rhs._seasonDateAppl) &&
            (_tvlstartyear == rhs._tvlstartyear) && (_tvlstartmonth == rhs._tvlstartmonth) &&
            (_tvlstartDay == rhs._tvlstartDay) && (_tvlStopyear == rhs._tvlStopyear) &&
            (_tvlStopmonth == rhs._tvlStopmonth) && (_tvlStopDay == rhs._tvlStopDay) &&
            (_assumptionOverride == rhs._assumptionOverride));
  }

  static void dummyData(SeasonalAppl& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._validityInd = 'A';
    obj._inhibit = 'B';
    obj._unavailtag = 'C';
    obj._geoTblItemNo = 1;
    obj._seasonDateAppl = 'D';
    obj._tvlstartyear = 2;
    obj._tvlstartmonth = 3;
    obj._tvlstartDay = 4;
    obj._tvlStopyear = 5;
    obj._tvlStopmonth = 6;
    obj._tvlStopDay = 7;
    obj._assumptionOverride = 'E';
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
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _unavailtag;
  int _geoTblItemNo;
  Indicator _seasonDateAppl;
  int _tvlstartyear;
  int _tvlstartmonth;
  int _tvlstartDay;
  int _tvlStopyear;
  int _tvlStopmonth;
  int _tvlStopDay;
  Indicator _assumptionOverride;

public:
  void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _geoTblItemNo);
    FLATTENIZE(archive, _seasonDateAppl);
    FLATTENIZE(archive, _tvlstartyear);
    FLATTENIZE(archive, _tvlstartmonth);
    FLATTENIZE(archive, _tvlstartDay);
    FLATTENIZE(archive, _tvlStopyear);
    FLATTENIZE(archive, _tvlStopmonth);
    FLATTENIZE(archive, _tvlStopDay);
    FLATTENIZE(archive, _assumptionOverride);
  }

protected:
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

  int& geoTblItemNo() { return _geoTblItemNo; }
  const int& geoTblItemNo() const { return _geoTblItemNo; }

  Indicator& seasonDateAppl() { return _seasonDateAppl; }
  const Indicator& seasonDateAppl() const { return _seasonDateAppl; }

  int& tvlstartyear() { return _tvlstartyear; }
  const int& tvlstartyear() const { return _tvlstartyear; }

  int& tvlstartmonth() { return _tvlstartmonth; }
  const int& tvlstartmonth() const { return _tvlstartmonth; }

  int& tvlstartDay() { return _tvlstartDay; }
  const int& tvlstartDay() const { return _tvlstartDay; }

  int& tvlStopyear() { return _tvlStopyear; }
  const int& tvlStopyear() const { return _tvlStopyear; }

  int& tvlStopmonth() { return _tvlStopmonth; }
  const int& tvlStopmonth() const { return _tvlStopmonth; }

  int& tvlStopDay() { return _tvlStopDay; }
  const int& tvlStopDay() const { return _tvlStopDay; }

  Indicator& assumptionOverride() { return _assumptionOverride; }
  const Indicator& assumptionOverride() const { return _assumptionOverride; }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_validityInd
           & ptr->_inhibit
           & ptr->_unavailtag
           & ptr->_geoTblItemNo
           & ptr->_seasonDateAppl
           & ptr->_tvlstartyear
           & ptr->_tvlstartmonth
           & ptr->_tvlstartDay
           & ptr->_tvlStopyear
           & ptr->_tvlStopmonth
           & ptr->_tvlStopDay
           & ptr->_assumptionOverride;
  }

};

}// tse

