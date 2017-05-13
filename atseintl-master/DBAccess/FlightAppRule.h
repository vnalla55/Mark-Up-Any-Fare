//----------------------------------------------------------------------------
//	   FlightAppRule.h
//	   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//-----------------------------------------------------------------------------------------

#pragma once
#include "DBAccess/Flattenizable.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class FlightAppRule : public RuleItemInfo
{
public:
  FlightAppRule()
    : _carrierFltTblItemNo1(0),
      _carrierFltTblItemNo2(0),
      _geoTblItemNoBetwVia(0),
      _geoTblItemNoAndVia(0),
      _geoTblItemNoVia(0),
      _fltAppl(' '),
      _unavailtag(' '),
      _flt1(0),
      _fltRelational1(' '),
      _flt2(0),
      _fltRelational2(' '),
      _flt3(0),
      _inOutInd(' '),
      _geoAppl(' '),
      _locAppl(' '),
      _viaInd(' '),
      _hidden(' '),
      _fltNonStop(' '),
      _fltDirect(' '),
      _fltMultiStop(' '),
      _fltOneStop(' '),
      _fltOnline(' '),
      _fltInterline(' '),
      _fltSame(' '),
      _equipAppl(' ')
  {
  }
  bool operator==(const FlightAppRule& rhs) const
  {
    return (
        (RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
        (_expireDate == rhs._expireDate) && (_carrierFltTblItemNo1 == rhs._carrierFltTblItemNo1) &&
        (_carrierFltTblItemNo2 == rhs._carrierFltTblItemNo2) &&
        (_geoTblItemNoBetwVia == rhs._geoTblItemNoBetwVia) &&
        (_geoTblItemNoAndVia == rhs._geoTblItemNoAndVia) &&
        (_geoTblItemNoVia == rhs._geoTblItemNoVia) && (_fltAppl == rhs._fltAppl) &&
        (_unavailtag == rhs._unavailtag) && (_flt1 == rhs._flt1) && (_carrier1 == rhs._carrier1) &&
        (_fltRelational1 == rhs._fltRelational1) && (_flt2 == rhs._flt2) &&
        (_carrier2 == rhs._carrier2) && (_fltRelational2 == rhs._fltRelational2) &&
        (_flt3 == rhs._flt3) && (_carrier3 == rhs._carrier3) && (_dow == rhs._dow) &&
        (_inOutInd == rhs._inOutInd) && (_geoAppl == rhs._geoAppl) && (_locAppl == rhs._locAppl) &&
        (_viaInd == rhs._viaInd) && (_hidden == rhs._hidden) && (_fltNonStop == rhs._fltNonStop) &&
        (_fltDirect == rhs._fltDirect) && (_fltMultiStop == rhs._fltMultiStop) &&
        (_fltOneStop == rhs._fltOneStop) && (_fltOnline == rhs._fltOnline) &&
        (_fltInterline == rhs._fltInterline) && (_fltSame == rhs._fltSame) &&
        (_equipAppl == rhs._equipAppl) && (_equipType == rhs._equipType) &&
        (_lastModDate == rhs._lastModDate));
  }

  static void dummyData(FlightAppRule& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._carrierFltTblItemNo1 = 1;
    obj._carrierFltTblItemNo2 = 2;
    obj._geoTblItemNoBetwVia = 3;
    obj._geoTblItemNoAndVia = 4;
    obj._geoTblItemNoVia = 5;
    obj._fltAppl = 'A';
    obj._unavailtag = 'B';
    obj._flt1 = 6666;
    obj._carrier1 = "CDE";
    obj._fltRelational1 = 'F';
    obj._flt2 = 7777;
    obj._carrier2 = "GHI";
    obj._fltRelational2 = 'J';
    obj._flt3 = 8888;
    obj._carrier3 = "KLM";
    obj._dow = "aaaaaaa";
    obj._inOutInd = 'N';
    obj._geoAppl = 'O';
    obj._locAppl = 'P';
    obj._viaInd = 'Q';
    obj._hidden = 'R';
    obj._fltNonStop = 'S';
    obj._fltDirect = 'T';
    obj._fltMultiStop = 'U';
    obj._fltOneStop = 'V';
    obj._fltOnline = 'W';
    obj._fltInterline = 'X';
    obj._fltSame = 'Y';
    obj._equipAppl = 'Z';
    obj._equipType = "abc";
    obj._lastModDate = time(nullptr);
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
  int _carrierFltTblItemNo1;
  int _carrierFltTblItemNo2;
  int _geoTblItemNoBetwVia;
  int _geoTblItemNoAndVia;
  int _geoTblItemNoVia;
  Indicator _fltAppl;
  Indicator _unavailtag;
  FlightNumber _flt1;
  CarrierCode _carrier1;
  Indicator _fltRelational1;
  FlightNumber _flt2;
  CarrierCode _carrier2;
  Indicator _fltRelational2;
  FlightNumber _flt3;
  CarrierCode _carrier3;
  DayOfWeekCode _dow;
  Indicator _inOutInd;
  Indicator _geoAppl;
  Indicator _locAppl;
  Indicator _viaInd;
  Indicator _hidden;
  Indicator _fltNonStop;
  Indicator _fltDirect;
  Indicator _fltMultiStop;
  Indicator _fltOneStop;
  Indicator _fltOnline;
  Indicator _fltInterline;
  Indicator _fltSame;
  Indicator _equipAppl;
  EquipmentType _equipType;
  DateTime _lastModDate;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _carrierFltTblItemNo1);
    FLATTENIZE(archive, _carrierFltTblItemNo2);
    FLATTENIZE(archive, _geoTblItemNoBetwVia);
    FLATTENIZE(archive, _geoTblItemNoAndVia);
    FLATTENIZE(archive, _geoTblItemNoVia);
    FLATTENIZE(archive, _fltAppl);
    FLATTENIZE(archive, _unavailtag);
    FLATTENIZE(archive, _flt1);
    FLATTENIZE(archive, _carrier1);
    FLATTENIZE(archive, _fltRelational1);
    FLATTENIZE(archive, _flt2);
    FLATTENIZE(archive, _carrier2);
    FLATTENIZE(archive, _fltRelational2);
    FLATTENIZE(archive, _flt3);
    FLATTENIZE(archive, _carrier3);
    FLATTENIZE(archive, _dow);
    FLATTENIZE(archive, _inOutInd);
    FLATTENIZE(archive, _geoAppl);
    FLATTENIZE(archive, _locAppl);
    FLATTENIZE(archive, _viaInd);
    FLATTENIZE(archive, _hidden);
    FLATTENIZE(archive, _fltNonStop);
    FLATTENIZE(archive, _fltDirect);
    FLATTENIZE(archive, _fltMultiStop);
    FLATTENIZE(archive, _fltOneStop);
    FLATTENIZE(archive, _fltOnline);
    FLATTENIZE(archive, _fltInterline);
    FLATTENIZE(archive, _fltSame);
    FLATTENIZE(archive, _equipAppl);
    FLATTENIZE(archive, _equipType);
    FLATTENIZE(archive, _lastModDate);
  }

protected:
public:
  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  int& carrierFltTblItemNo1() { return _carrierFltTblItemNo1; }
  const int& carrierFltTblItemNo1() const { return _carrierFltTblItemNo1; }

  int& carrierFltTblItemNo2() { return _carrierFltTblItemNo2; }
  const int& carrierFltTblItemNo2() const { return _carrierFltTblItemNo2; }

  int& geoTblItemNoBetwVia() { return _geoTblItemNoBetwVia; }
  const int& geoTblItemNoBetwVia() const { return _geoTblItemNoBetwVia; }

  int& geoTblItemNoAndVia() { return _geoTblItemNoAndVia; }
  const int& geoTblItemNoAndVia() const { return _geoTblItemNoAndVia; }

  int& geoTblItemNoVia() { return _geoTblItemNoVia; }
  const int& geoTblItemNoVia() const { return _geoTblItemNoVia; }

  Indicator& fltAppl() { return _fltAppl; }
  const Indicator& fltAppl() const { return _fltAppl; }

  FlightNumber& flt1() { return _flt1; }
  const FlightNumber& flt1() const { return _flt1; }

  CarrierCode& carrier1() { return _carrier1; }
  const CarrierCode& carrier1() const { return _carrier1; }

  Indicator& fltRelational1() { return _fltRelational1; }
  const Indicator& fltRelational1() const { return _fltRelational1; }

  FlightNumber& flt2() { return _flt2; }
  const FlightNumber& flt2() const { return _flt2; }

  CarrierCode& carrier2() { return _carrier2; }
  const CarrierCode& carrier2() const { return _carrier2; }

  Indicator& fltRelational2() { return _fltRelational2; }
  const Indicator& fltRelational2() const { return _fltRelational2; }

  FlightNumber& flt3() { return _flt3; }
  const FlightNumber& flt3() const { return _flt3; }

  CarrierCode& carrier3() { return _carrier3; }
  const CarrierCode& carrier3() const { return _carrier3; }

  DayOfWeekCode& dow() { return _dow; }
  const DayOfWeekCode& dow() const { return _dow; }

  Indicator& inOutInd() { return _inOutInd; }
  const Indicator& inOutInd() const { return _inOutInd; }

  Indicator& geoAppl() { return _geoAppl; }
  const Indicator& geoAppl() const { return _geoAppl; }

  Indicator& locAppl() { return _locAppl; }
  const Indicator& locAppl() const { return _locAppl; }

  Indicator& viaInd() { return _viaInd; }
  const Indicator& viaInd() const { return _viaInd; }

  Indicator& hidden() { return _hidden; }
  const Indicator& hidden() const { return _hidden; }

  Indicator& fltNonStop() { return _fltNonStop; }
  const Indicator& fltNonStop() const { return _fltNonStop; }

  Indicator& fltDirect() { return _fltDirect; }
  const Indicator& fltDirect() const { return _fltDirect; }

  Indicator& fltMultiStop() { return _fltMultiStop; }
  const Indicator& fltMultiStop() const { return _fltMultiStop; }

  Indicator& fltOneStop() { return _fltOneStop; }
  const Indicator& fltOneStop() const { return _fltOneStop; }

  Indicator& fltOnline() { return _fltOnline; }
  const Indicator& fltOnline() const { return _fltOnline; }

  Indicator& fltInterline() { return _fltInterline; }
  const Indicator& fltInterline() const { return _fltInterline; }

  Indicator& fltSame() { return _fltSame; }
  const Indicator& fltSame() const { return _fltSame; }

  Indicator& equipAppl() { return _equipAppl; }
  const Indicator& equipAppl() const { return _equipAppl; }

  EquipmentType& equipType() { return _equipType; }
  const EquipmentType& equipType() const { return _equipType; }

  DateTime& lastModDate() { return _lastModDate; }
  const DateTime& lastModDate() const { return _lastModDate; }

  Indicator& unavailtag() { return _unavailtag; }
  const Indicator& unavailtag() const { return _unavailtag; }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_carrierFltTblItemNo1
           & ptr->_carrierFltTblItemNo2
           & ptr->_geoTblItemNoBetwVia
           & ptr->_geoTblItemNoAndVia
           & ptr->_geoTblItemNoVia
           & ptr->_fltAppl
           & ptr->_unavailtag
           & ptr->_flt1
           & ptr->_carrier1
           & ptr->_fltRelational1
           & ptr->_flt2
           & ptr->_carrier2
           & ptr->_fltRelational2
           & ptr->_flt3
           & ptr->_carrier3
           & ptr->_dow
           & ptr->_inOutInd
           & ptr->_geoAppl
           & ptr->_locAppl
           & ptr->_viaInd
           & ptr->_hidden
           & ptr->_fltNonStop
           & ptr->_fltDirect
           & ptr->_fltMultiStop
           & ptr->_fltOneStop
           & ptr->_fltOnline
           & ptr->_fltInterline
           & ptr->_fltSame
           & ptr->_equipAppl
           & ptr->_equipType
           & ptr->_lastModDate;
  }

};
}

