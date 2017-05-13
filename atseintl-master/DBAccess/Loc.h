//-------------------------------------------------------------------
//
//  File:        Loc.h
//  Created:     March 8, 2004
//  Authors:     Roger Kelly
//
//  Description: Location.
//
//  Updates:
//          03/08/04 - VN - file created.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class Loc
{
protected:
  LocCode _loc;
  DateTime _createDate;
  DateTime _effDate = 0;
  DateTime _expireDate = 0;
  DateTime _discDate;
  LatLong _latdeg = 0;
  LatLong _latmin = 0;
  LatLong _latsec = 0;
  LatLong _lngdeg = 0;
  LatLong _lngmin = 0;
  LatLong _lngsec = 0;
  char _lathem = 0;
  char _lnghem = 0;
  bool _cityInd = false;
  bool _multitransind = false;
  bool _ruralarpind = false;
  Indicator _transtype = '\0';
  LocCode _city;
  NationCode _nation;
  StateCode _state;
  IATAAreaCode _area;
  IATASubAreaCode _subarea;
  AlaskaZoneCode _alaskazone = 0;
  bool _bufferZoneInd = false;
  DSTGrpCode _dstgrp;
  LocDescription _description;
  bool _faresind = false;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _latdeg);
    FLATTENIZE(archive, _latmin);
    FLATTENIZE(archive, _latsec);
    FLATTENIZE(archive, _lngdeg);
    FLATTENIZE(archive, _lngmin);
    FLATTENIZE(archive, _lngsec);
    FLATTENIZE(archive, _lathem);
    FLATTENIZE(archive, _lnghem);
    FLATTENIZE(archive, _cityInd);
    FLATTENIZE(archive, _multitransind);
    FLATTENIZE(archive, _ruralarpind);
    FLATTENIZE(archive, _transtype);
    FLATTENIZE(archive, _city);
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _state);
    FLATTENIZE(archive, _area);
    FLATTENIZE(archive, _subarea);
    FLATTENIZE(archive, _alaskazone);
    FLATTENIZE(archive, _bufferZoneInd);
    FLATTENIZE(archive, _dstgrp);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _faresind);
  }

  virtual bool operator==(const Loc& rhs) const
  {
    return ((_loc == rhs._loc) && (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_expireDate == rhs._expireDate) && (_discDate == rhs._discDate) &&
            (_latdeg == rhs._latdeg) && (_latmin == rhs._latmin) && (_latsec == rhs._latsec) &&
            (_lngdeg == rhs._lngdeg) && (_lngmin == rhs._lngmin) && (_lngsec == rhs._lngsec) &&
            (_lathem == rhs._lathem) && (_lnghem == rhs._lnghem) && (_cityInd == rhs._cityInd) &&
            (_multitransind == rhs._multitransind) && (_ruralarpind == rhs._ruralarpind) &&
            (_transtype == rhs._transtype) && (_city == rhs._city) && (_nation == rhs._nation) &&
            (_state == rhs._state) && (_area == rhs._area) && (_subarea == rhs._subarea) &&
            (_alaskazone == rhs._alaskazone) && (_bufferZoneInd == rhs._bufferZoneInd) &&
            (_dstgrp == rhs._dstgrp) && (_description == rhs._description) &&
            (_faresind == rhs._faresind));
  }

  virtual bool operator<(const Loc& rhs) const
  {
    return ((_loc < rhs._loc) && (_createDate < rhs._createDate) && (_effDate < rhs._effDate) &&
            (_expireDate < rhs._expireDate) && (_discDate < rhs._discDate) &&
            (_latdeg < rhs._latdeg) && (_latmin < rhs._latmin) && (_latsec < rhs._latsec) &&
            (_lngdeg < rhs._lngdeg) && (_lngmin < rhs._lngmin) && (_lngsec < rhs._lngsec) &&
            (_lathem < rhs._lathem) && (_lnghem < rhs._lnghem) && (_cityInd < rhs._cityInd) &&
            (_multitransind < rhs._multitransind) && (_ruralarpind < rhs._ruralarpind) &&
            (_transtype < rhs._transtype) && (_city < rhs._city) && (_nation < rhs._nation) &&
            (_state < rhs._state) && (_area < rhs._area) && (_subarea < rhs._subarea) &&
            (_alaskazone < rhs._alaskazone) && (_bufferZoneInd < rhs._bufferZoneInd) &&
            (_dstgrp < rhs._dstgrp) && (_description < rhs._description) &&
            (_faresind < rhs._faresind));
  }

  static void dummyData(Loc& obj)
  {
    obj.loc() = "ABCDEFGH";
    obj.createDate() = time(nullptr);
    obj.effDate() = time(nullptr);
    obj.expireDate() = time(nullptr);
    obj.discDate() = time(nullptr);
    obj.latdeg() = 1;
    obj.latmin() = 2;
    obj.latsec() = 3;
    obj.lngdeg() = 4;
    obj.lngmin() = 5;
    obj.lngsec() = 6;
    obj.lathem() = 'I';
    obj.lnghem() = 'J';
    obj.cityInd() = true;
    obj.multitransind() = false;
    obj.ruralarpind() = true;
    obj.transtype() = 'K';
    obj.city() = "LMNOPQRS";
    obj.nation() = "TU";
    obj.state() = "XYZa";
    obj.area() = "bcde";
    obj.subarea() = "fg";
    obj.alaskazone() = 'h';
    obj.bufferZoneInd() = false;
    obj.dstgrp() = "ijkl";
    obj.description() = "mnopqrstuv";
    obj.faresind() = true;
  }

  Loc() = default;
  Loc(const Loc& oldLoc, const LocCode& newCity)
  {
    *this = oldLoc; // default copy is enough for Loc
    this->_city = newCity;
  }

  virtual ~Loc() = default;

  // Access

  LocCode& loc() { return _loc; }
  const LocCode& loc() const { return _loc; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  IATASubAreaCode& subarea() { return _subarea; }
  const IATASubAreaCode& subarea() const { return _subarea; }

  IATAAreaCode& area() { return _area; }
  const IATAAreaCode& area() const { return _area; }

  Indicator& transtype() { return _transtype; }
  const Indicator& transtype() const { return _transtype; }

  LocCode& city() { return _city; }
  const LocCode& city() const { return _city; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  StateCode& state() { return _state; }
  const StateCode& state() const { return _state; }

  DSTGrpCode& dstgrp() { return _dstgrp; }
  const DSTGrpCode& dstgrp() const { return _dstgrp; }

  AlaskaZoneCode& alaskazone() { return _alaskazone; }
  const AlaskaZoneCode& alaskazone() const { return _alaskazone; }

  LocDescription& description() { return _description; }
  const LocDescription& description() const { return _description; }

  LatLong& latsec() { return _latsec; }
  const LatLong& latsec() const { return _latsec; }

  LatLong& latdeg() { return _latdeg; }
  const LatLong& latdeg() const { return _latdeg; }

  LatLong& latmin() { return _latmin; }
  const LatLong& latmin() const { return _latmin; }

  LatLong& lngmin() { return _lngmin; }
  const LatLong& lngmin() const { return _lngmin; }

  LatLong& lngsec() { return _lngsec; }
  const LatLong& lngsec() const { return _lngsec; }

  LatLong& lngdeg() { return _lngdeg; }
  const LatLong& lngdeg() const { return _lngdeg; }

  char& lathem() { return _lathem; }
  const char& lathem() const { return _lathem; }

  char& lnghem() { return _lnghem; }
  const char& lnghem() const { return _lnghem; }

  bool& cityInd() { return _cityInd; }
  const bool& cityInd() const { return _cityInd; }

  bool& bufferZoneInd() { return _bufferZoneInd; }
  const bool& bufferZoneInd() const { return _bufferZoneInd; }

  bool& ruralarpind() { return _ruralarpind; }
  const bool& ruralarpind() const { return _ruralarpind; }

  bool& multitransind() { return _multitransind; }
  const bool& multitransind() const { return _multitransind; }

  bool& faresind() { return _faresind; }
  const bool& faresind() const { return _faresind; }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_loc
           & ptr->_createDate
           & ptr->_effDate
           & ptr->_expireDate
           & ptr->_discDate
           & ptr->_latdeg
           & ptr->_latmin
           & ptr->_latsec
           & ptr->_lngdeg
           & ptr->_lngmin
           & ptr->_lngsec
           & ptr->_lathem
           & ptr->_lnghem
           & ptr->_cityInd
           & ptr->_multitransind
           & ptr->_ruralarpind
           & ptr->_transtype
           & ptr->_city
           & ptr->_nation
           & ptr->_state
           & ptr->_area
           & ptr->_subarea
           & ptr->_alaskazone
           & ptr->_bufferZoneInd
           & ptr->_dstgrp
           & ptr->_description
           & ptr->_faresind;
  }

};
} // tse namespace
