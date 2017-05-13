//-------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "DBAccess/Flattenizable.h"

namespace tse
{

class State
{
private:
  NationCode _nation;
  StateCode _state;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _expireDate;
  DateTime _discDate;
  IATASubAreaCode _subArea;
  IATAAreaCode _area;
  Description _description;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _nation);
    FLATTENIZE(archive, _state);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _subArea);
    FLATTENIZE(archive, _area);
    FLATTENIZE(archive, _description);
  }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  StateCode& state() { return _state; }
  const StateCode& state() const { return _state; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  IATASubAreaCode& subArea() { return _subArea; }
  const IATASubAreaCode& subArea() const { return _subArea; }

  IATAAreaCode& area() { return _area; }
  const IATAAreaCode& area() const { return _area; }

  Description& description() { return _description; }
  const Description& description() const { return _description; }

  bool operator==(const State& rhs) const
  {
    return ((_nation == rhs._nation) && (_state == rhs._state) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_expireDate == rhs._expireDate) && (_discDate == rhs._discDate) &&
            (_subArea == rhs._subArea) && (_area == rhs._area) &&
            (_description == rhs._description));
  }

  static void dummyData(State& obj)
  {
    obj._nation = "ABCD";
    obj._state = "EFGH";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._subArea = "IJ";
    obj._area = "KLMN";
    obj._description = "aaaaaaaa";
  }
};
}

