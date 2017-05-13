//----------------------------------------------------------------------------
//       © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#ifndef PFCEQUIPTYPEEXEMPT_H
#define PFCEQUIPTYPEEXEMPT_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class PfcEquipTypeExempt
{
public:
  PfcEquipTypeExempt() : _noSeats(0), _inhibit(' ') {}
  EquipmentType& equip() { return _equip; }
  const EquipmentType& equip() const { return _equip; }

  StateCode& state() { return _state; }
  const StateCode& state() const { return _state; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  std::string& equipName() { return _equipName; }
  const std::string& equipName() const { return _equipName; }

  int& noSeats() { return _noSeats; }
  const int& noSeats() const { return _noSeats; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  bool operator==(const PfcEquipTypeExempt& rhs) const
  {
    return ((_equip == rhs._equip) && (_state == rhs._state) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_equipName == rhs._equipName) &&
            (_noSeats == rhs._noSeats) && (_vendor == rhs._vendor) && (_inhibit == rhs._inhibit));
  }

  static void dummyData(PfcEquipTypeExempt& obj)
  {
    obj._equip = "ABC";
    obj._state = "DEFG";
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._equipName = "aaaaaaaa";
    obj._noSeats = 1;
    obj._vendor = "HIJK";
    obj._inhibit = 'L';
  }

private:
  EquipmentType _equip;
  StateCode _state;
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  std::string _equipName;
  int _noSeats;
  VendorCode _vendor;
  Indicator _inhibit;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _equip);
    FLATTENIZE(archive, _state);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _equipName);
    FLATTENIZE(archive, _noSeats);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _inhibit);
  }
};
}

#endif
